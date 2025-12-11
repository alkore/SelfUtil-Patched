#include "selfutil.h"
#include <filesystem>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <iostream>

using namespace std;

void print_usage()
{
    printf("selfutil [--input] [--output] [--dry-run] [--verbose] [--overwrite] "
           "[--align-size] [--not-patch-first-segment-duplicate] [--not-patch-version-segment]\n");
}

// Options globales
string input_file_path = "";
string output_file_path = "";
bool dry_run = false;
bool verbose = false;
bool overwrite = false;
bool align_size = false;
bool patch_first_segment_duplicate = true;
Elf64_Off patch_first_segment_safety_percentage = 2; // min amount of cells (in percentage)
bool patch_version_segment = true;

int first_min_offset = -1;

// Fonctions utilitaires
bool compare_u8_array(u8* first_array, u8* second_array, int size)
{
    for (int i = 0; i < size; i++)
        if (first_array[i] != second_array[i])
            return false;
    return true;
}

void set_u8_array(u8* arr, int value, int size)
{
    memset(arr, value, size);
}

// Alignement macOS
template<typename T>
T AlignUp(T val, T align)
{
    return (val + align - 1) & ~(align - 1);
}

// -------------------- MAIN --------------------
int main(int argc, char* argv[])
{
    if (argc < 2) {
        print_usage();
        return 0;
    }

    vector<string> args(argv + 1, argv + argc);
    bool error_found = false;

    for (size_t i = 0; i < args.size(); i++) {
        if (args[i] == "--input" && i + 1 < args.size()) input_file_path = args[++i];
        else if (args[i] == "--output" && i + 1 < args.size()) output_file_path = args[++i];
        else if (args[i] == "--dry-run") dry_run = true;
        else if (args[i] == "--verbose") verbose = true;
        else if (args[i] == "--overwrite") overwrite = true;
        else if (args[i] == "--align-size") align_size = true;
        else if (args[i] == "--not-patch-first-segment-duplicate") patch_first_segment_duplicate = false;
        else if (args[i] == "--not-patch-version-segment") patch_version_segment = false;
        else if (input_file_path.empty()) input_file_path = args[i];
        else error_found = true;
    }

    if (error_found || input_file_path.empty()) {
        print_usage();
        return 0;
    }

    string fixed_output_file_path = output_file_path.empty() ? input_file_path : output_file_path;

    if (!overwrite && output_file_path.empty()) {
        size_t newSize = input_file_path.rfind('.');
        if (newSize != string::npos) fixed_output_file_path = input_file_path.substr(0, newSize) + ".elf";
    }

    if (verbose) {
        printf("Input: %s\nOutput: %s\nDry Run: %d\nVerbose: %d\nOverwrite: %d\nAlign: %d\nPatch First: %d\nPatch Version: %d\n",
               input_file_path.c_str(), fixed_output_file_path.c_str(),
               dry_run, verbose, overwrite, align_size,
               patch_first_segment_duplicate, patch_version_segment);
    }

    SelfUtil util(input_file_path);
    if (!util.SaveToELF(fixed_output_file_path))
        cerr << "Error, Save to ELF failed!" << endl;

    return 0;
}

// -------------------- SelfUtil --------------------
SelfUtil::SelfUtil(string filePath) {
    if (!Load(filePath)) cerr << "Error, Load() failed!\n";
}

bool SelfUtil::Load(string filePath)
{
    if (!filesystem::exists(filePath)) {
        cerr << "Failed to find file: " << filePath << endl;
        return false;
    }

    size_t fileSize = filesystem::file_size(filePath);
    data.resize(fileSize);

    FILE* f = fopen(filePath.c_str(), "rb");
    if (!f) {
        cerr << "Failed to open file: " << filePath << endl;
        return false;
    }

    fread(data.data(), 1, fileSize, f);
    fclose(f);

    return Parse();
}

bool SelfUtil::Parse()
{
    seHead = reinterpret_cast<Self_Hdr*>(data.data());

    if (seHead->magic != PS4_SELF_MAGIC && seHead->magic != PS5_SELF_MAGIC) {
        cerr << "Invalid Magic! " << hex << seHead->magic << endl;
        return false;
    }

    entries.clear();
    for (unat seIdx = 0; seIdx < seHead->num_entries; seIdx++)
        entries.push_back(&reinterpret_cast<Self_Entry*>(data.data())[1 + seIdx]);

    elfHOffs = (1 + seHead->num_entries) * 0x20;
    eHead = reinterpret_cast<elf64_hdr*>(data.data() + elfHOffs);

    if (!TestIdent()) {
        cerr << "Elf e_ident invalid!\n";
        return false;
    }

    phdrs.clear();
    for (unat phIdx = 0; phIdx < eHead->e_phnum; phIdx++)
        phdrs.push_back(&reinterpret_cast<Elf64_Phdr*>(data.data() + elfHOffs + eHead->e_phoff)[phIdx]);

    return true;
}

bool SelfUtil::TestIdent()
{
    const u32 ELF_MAGIC = 0x464C457F;
    if (reinterpret_cast<u32*>(eHead->e_ident)[0] != ELF_MAGIC) return false;

    return (eHead->e_ident[EI_CLASS] == ELFCLASS64 &&
            eHead->e_ident[EI_DATA] == ELFDATA2LSB &&
            eHead->e_ident[EI_VERSION] == EV_CURRENT &&
            eHead->e_ident[EI_OSABI] == ELFOSABI_FREEBSD &&
            eHead->e_machine == EM_X86_64 &&
            eHead->e_version == EV_CURRENT);
}

bool SelfUtil::SaveToELF(string savePath)
{
    if (verbose) cout << "SaveToELF(" << savePath << ")\n";

    if (!eHead || phdrs.empty()) return false;

    Elf64_Off first = 0, last = 0;
    size_t saveSize = 0;
    Elf64_Phdr *ph_first = nullptr, *ph_last = nullptr, *ph_PT_SCE_VERSION = nullptr;
    bool patched_PT_SCE_VERSION = false;

    for (auto ph : phdrs) {
        if (ph->p_type == PT_SCE_VERSION) ph_PT_SCE_VERSION = ph;
        if (!ph_first || (ph->p_offset > 0 && ph->p_offset < ph_first->p_offset)) ph_first = ph;
        if (!ph_last || ph->p_offset > ph_last->p_offset) ph_last = ph;
    }

    first = ph_first ? ph_first->p_offset : 0;
    last = ph_last ? ph_last->p_offset : 0;
    saveSize = last ? static_cast<size_t>(ph_last->p_offset + ph_last->p_filesz) : 0;

    if (align_size) saveSize = AlignUp(saveSize, size_t(0x10));

    save.clear();
    save.resize(saveSize);
    memset(save.data(), 0, saveSize);

    memcpy(save.data(), eHead, first);
    u8* pd = save.data();

    for (auto ee : entries) {
        size_t phIdx = (ee->props >> 20) & 0xFFF;
        if (phIdx >= phdrs.size()) continue;

        Elf64_Phdr* ph = phdrs[phIdx];
        bool method_found = (ee->props & 0x800) != 0;

        if (!method_found && ph_PT_SCE_VERSION && ph == ph_PT_SCE_VERSION && patch_version_segment)
            method_found = true;

        if (method_found) {
            void* srcp = (ph_PT_SCE_VERSION && ph == ph_PT_SCE_VERSION) ?
                         data.data() + data.size() - ph->p_filesz :
                         data.data() + ee->offs;
            void* dstp = pd + ph->p_offset;
            size_t datasize = (ph_PT_SCE_VERSION && ph == ph_PT_SCE_VERSION) ? ph->p_filesz : ee->fileSz;

            memcpy(dstp, srcp, datasize);
            if (ph_PT_SCE_VERSION && ph == ph_PT_SCE_VERSION) patched_PT_SCE_VERSION = true;
        }
    }

    if (patch_first_segment_duplicate) {
        int first_min_offset_local = -1;
        for (auto e : entries) {
            int offset = static_cast<int>(e->offs - elfHOffs);
            if (offset >= 0 && offset < static_cast<int>(first))
                first_min_offset_local = max(first_min_offset_local, offset);
        }

        if (first_min_offset_local != -1)
            memset(pd + first_min_offset_local, 0, first - first_min_offset_local);
    }

    if (!dry_run) {
        FILE* f = fopen(savePath.c_str(), "wb");
        if (!f) {
            perror("Failed to save ELF");
            return false;
        }
        fwrite(pd, 1, saveSize, f);
        fclose(f);
    }

    return true;
}
