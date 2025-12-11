#include "pch.h"
#include "selfutil.h"
#include <filesystem>
#include "compat/elf.h"

// ===================
// Global options
// ===================
void print_usage() {
    printf("selfutil [--input] [--output] [--dry-run] [--verbose] [--overwrite] "
           "[--align-size] [--not-patch-first-segment-duplicate] [--not-patch-version-segment]\n");
}

string input_file_path = "";
string output_file_path = "";
bool dry_run = false;
bool verbose = false;
bool overwrite = false;
bool align_size = false;
bool patch_first_segment_duplicate = true;
Elf64_Off patch_first_segment_safety_percentage = 2;
bool patch_version_segment = true;

int first_min_offset = -1;

// ===================
// Main
// ===================
int main(int argc, char* argv[]) {
    vector<string> args;
    for (int i = 1; i < argc; i++)
        args.push_back(argv[i]);

    if (argc < 2) {
        print_usage();
        exit(0);
    }

    bool error_found = false;

    if (argc == 2)
        input_file_path = args[0];
    else {
        for (int i = 0; i < argc - 1; i++) {
            if (args[i] == "--input") input_file_path = args[++i];
            else if (args[i] == "--output") output_file_path = args[++i];
            else if (args[i] == "--dry-run") dry_run = true;
            else if (args[i] == "--verbose") verbose = true;
            else if (args[i] == "--overwrite") overwrite = true;
            else if (args[i] == "--align-size") align_size = true;
            else if (args[i] == "--not-patch-first-segment-duplicate") patch_first_segment_duplicate = false;
            else if (args[i] == "--not-patch-version-segment") patch_version_segment = false;
            else {
                error_found = true;
                break;
            }
        }
    }

    if (error_found || input_file_path.empty()) {
        print_usage();
        exit(0);
    }

    string fixed_output_file_path = output_file_path.empty() ? input_file_path : output_file_path;
    if (!overwrite && output_file_path.empty()) {
        size_t pos = input_file_path.rfind('.');
        if (pos != string::npos) {
            fixed_output_file_path = input_file_path.substr(0, pos) + ".elf";
        } else {
            fixed_output_file_path += ".elf";
        }
    }

    printf("%s: %s\n%s: %s\n%s: %s\n%s: %s\n%s: %s\n%s: %s\n%s: %s\n",
           "Input File Name", input_file_path.c_str(),
           "Output File Name", fixed_output_file_path.c_str(),
           "Dry Run", dry_run ? "True" : "False",
           "Verbose", verbose ? "True" : "False",
           "Overwrite", overwrite ? "True" : "False",
           "Align Size", align_size ? "True" : "False",
           "Patch First Segment Duplicate", patch_first_segment_duplicate ? "True" : "False",
           "Patch Version Segment", patch_version_segment ? "True" : "False");

    SelfUtil util(input_file_path);

    if (!util.SaveToELF(fixed_output_file_path))
        printf("Error, Save to ELF failed!\n");

    return 0;
}

// ===================
// Helper functions
// ===================
bool compare_u8_array(u8* a, u8* b, int size) {
    for (int i = 0; i < size; i++)
        if (a[i] != b[i]) return false;
    return true;
}

void set_u8_array(u8* arr, int value, int size) {
    for (int i = 0; i < size; i++) arr[i] = value;
}

// ===================
// SelfUtil Methods
// ===================
bool SelfUtil::Load(string filePath) {
    if (!filesystem::exists(filePath)) {
        printf("Failed to find file: \"%s\"\n", filePath.c_str());
        return false;
    }

    size_t fileSize = (size_t)filesystem::file_size(filePath);
    data.resize(fileSize);

    FILE* f = fopen(filePath.c_str(), "rb");
    if (!f) {
        printf("Failed to open file: \"%s\"\n", filePath.c_str());
        return false;
    }
    fread(&data[0], 1, fileSize, f);
    fclose(f);

    return Parse();
}

bool SelfUtil::Parse() {
    seHead = (Self_Hdr*)&data[0];

    if (seHead->magic != PS4_SELF_MAGIC && seHead->magic != PS5_SELF_MAGIC) {
        printf("Invalid Magic! (0x%08X)\n", seHead->magic);
        return false;
    }

    entries.clear();
    for (unat seIdx = 0; seIdx < seHead->num_entries; seIdx++)
        entries.push_back(&((Self_Entry*)&data[0])[1 + seIdx]);

    elfHOffs = (1 + seHead->num_entries) * 0x20;
    eHead = (Elf64_Ehdr*)(&data[0] + elfHOffs);

    if (!TestIdent()) {
        printf("Elf e_ident invalid!\n");
        return false;
    }

    for (unat phIdx = 0; phIdx < eHead->e_phnum; phIdx++)
        phdrs.push_back(&((Elf64_Phdr*)(&data[0] + elfHOffs + eHead->e_phoff))[phIdx]);

    return true;
}

bool SelfUtil::TestIdent() {
    if (ELF_MAGIC != ((u32*)eHead->e_ident)[0]) {
        printf("File is invalid! e_ident magic: %08X\n", ((u32*)eHead->e_ident)[0]);
        return false;
    }

    if (!((eHead->e_ident[EI_CLASS] == ELFCLASS64) &&
          (eHead->e_ident[EI_DATA] == ELFDATA2LSB) &&
          (eHead->e_ident[EI_VERSION] == EV_CURRENT) &&
          (eHead->e_ident[EI_OSABI] == ELFOSABI_FREEBSD)))
        return false;

    if ((eHead->e_type >> 8) != 0xFE)
        printf("Elf64::e_type: 0x%04X\n", eHead->e_type);

    if (!((eHead->e_machine == EM_X86_64) && (eHead->e_version == EV_CURRENT)))
        return false;

    return true;
}

bool SelfUtil::SaveToELF(string savePath) {
    if (verbose) printf("\nSaveToELF(\"%s\")\n", savePath.c_str());

    Elf64_Off first = 0, last = 0;
    size_t saveSize = 0;
    Elf64_Phdr* ph_first = nullptr;
    Elf64_Phdr* ph_last = nullptr;
    Elf64_Phdr* ph_PT_SCE_VERSION = nullptr;
    bool patched_PT_SCE_VERSION = false;

    for (auto ph : phdrs) {
        if (ph->p_type == PT_SCE_VERSION) ph_PT_SCE_VERSION = ph;
        if (!ph_first || (ph->p_offset > 0 && ph->p_offset < ph_first->p_offset)) ph_first = ph;
        if (!ph_last || ph->p_offset > ph_last->p_offset) ph_last = ph;
    }

    first = ph_first ? ph_first->p_offset : 0;
    if (ph_last) saveSize = ph_last->p_offset + ph_last->p_filesz;
    if (align_size) saveSize = AlignUp<size_t>(saveSize, 0x10);

    save.clear();
    save.resize(saveSize, 0);
    u8* pd = &save[0];

    memcpy(pd, eHead, first); // copy header to first segment

    // Patch segments
    for (auto ee : entries) {
        bool method_found = false;
        unat phIdx = (ee->props >> 20) & 0xFFF;
        Elf64_Phdr* ph = phdrs.at(phIdx);

        if ((ee->props & 0x800) == 0) {
            if (ph_PT_SCE_VERSION && ph == ph_PT_SCE_VERSION && patch_version_segment)
                method_found = true;
        } else method_found = true;

        if (method_found) {
            void* srcp = nullptr;
            void* dstp = pd + ph->p_offset;
            size_t datasize = 0;

            if (ph_PT_SCE_VERSION && ph == ph_PT_SCE_VERSION) {
                patched_PT_SCE_VERSION = true;
                srcp = &data[data.capacity() - ph->p_filesz];
                datasize = ph->p_filesz;
            } else {
                srcp = &data[ee->offs];
                datasize = ee->fileSz;
            }

            memcpy(dstp, srcp, datasize);
        }
    }

    if (!dry_run) {
        FILE* f = fopen(savePath.c_str(), "wb");
        if (!f) return false;
        fwrite(pd, 1, saveSize, f);
        fclose(f);
    }

    return true;
}
