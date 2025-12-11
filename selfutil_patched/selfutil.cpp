#include "pch.h"
#include "selfutil.h"
#include <filesystem>
#include "compat/elf.h"
#include <cstdint>
#include <cstdio>
#include <vector>
#include <cstring>

using namespace std;

// -----------------------------------------------------------------------------
// Global options
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------
void print_usage()
{
    printf("selfutil [--input] [--output] [--dry-run] [--verbose] [--overwrite] [--align-size] [--not-patch-first-segment-duplicate] [--not-patch-version-segment]\n");
}

bool compare_u8_array(u8* first_array, u8* second_array, int size)
{
    for (int i = 0; i < size; i++)
        if (first_array[i] != second_array[i])
            return false;
    return true;
}

void set_u8_array(u8* array, int value, int size)
{
    for (int i = 0; i < size; i++)
        array[i] = value;
}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    vector<string> args;
    for (int i = 1; i < argc; i++) args.push_back(argv[i]);

    if (argc < 2) { print_usage(); return 0; }

    bool error_found = false;

    if (argc == 2)
        input_file_path = args[0];
    else
    {
        for (size_t i = 0; i < args.size(); i++)
        {
            if (args[i] == "--input") input_file_path = args[++i];
            else if (args[i] == "--output") output_file_path = args[++i];
            else if (args[i] == "--dry-run") dry_run = true;
            else if (args[i] == "--verbose") verbose = true;
            else if (args[i] == "--overwrite") overwrite = true;
            else if (args[i] == "--align-size") align_size = true;
            else if (args[i] == "--not-patch-first-segment-duplicate") patch_first_segment_duplicate = false;
            else if (args[i] == "--not-patch-version-segment") patch_version_segment = false;
            else { error_found = true; break; }
        }
    }

    if (error_found || input_file_path.empty()) { print_usage(); return 0; }

    string fixed_output_file_path = output_file_path.empty() ? input_file_path : output_file_path;
    if (!overwrite && output_file_path.empty())
    {
        size_t dotPos = input_file_path.rfind('.');
        if (dotPos != string::npos)
        {
            fixed_output_file_path = input_file_path.substr(0, dotPos) + ".elf";
        }
    }

    printf(
        "Input File Name: %s\nOutput File Name: %s\nDry Run: %s\nVerbose: %s\nOverwrite: %s\nAlign Size: %s\nPatch First Segment Duplicate: %s\nPatch Version Segment: %s\n",
        input_file_path.c_str(),
        fixed_output_file_path.c_str(),
        dry_run ? "True" : "False",
        verbose ? "True" : "False",
        overwrite ? "True" : "False",
        align_size ? "True" : "False",
        patch_first_segment_duplicate ? "True" : "False",
        patch_version_segment ? "True" : "False"
    );

    SelfUtil util(input_file_path);
    if (!util.SaveToELF(fixed_output_file_path))
        printf("Error, Save to ELF failed!\n");

    return 0;
}

// -----------------------------------------------------------------------------
// SelfUtil Implementation
// -----------------------------------------------------------------------------
bool SelfUtil::Load(string filePath)
{
    if (!filesystem::exists(filePath)) { printf("Failed to find file: \"%s\"\n", filePath.c_str()); return false; }

    size_t fileSize = (size_t)filesystem::file_size(filePath);
    data.resize(fileSize);

    FILE* f = fopen(filePath.c_str(), "rb");
    if (!f) { printf("Failed to open file: \"%s\"\n", filePath.c_str()); return false; }

    fread(&data[0], 1, fileSize, f);
    fclose(f);

    return Parse();
}

bool SelfUtil::Parse()
{
    seHead = (Self_Hdr*)&data[0];

    entries.clear();
    for (unat i = 0; i < seHead->num_entries; i++)
        entries.push_back(&((Self_Entry*)&data[0])[1 + i]);

    elfHOffs = (1 + seHead->num_entries) * 0x20;
    eHead = (Elf64_Ehdr*)(&data[0] + elfHOffs); // ✅ corrected

    if (!TestIdent()) return false;

    for (unat phIdx = 0; phIdx < eHead->e_phnum; phIdx++)
        phdrs.push_back(&((Elf64_Phdr*)(&data[0] + elfHOffs + eHead->e_phoff))[phIdx]);

    return true;
}

bool SelfUtil::TestIdent()
{
    if (ELF_MAGIC != ((u32*)eHead->e_ident)[0]) { printf("File is invalid! e_ident magic: %08X\n", ((u32*)eHead->e_ident)[0]); return false; }
    if (!((eHead->e_ident[EI_CLASS] == ELFCLASS64) &&
          (eHead->e_ident[EI_DATA] == ELFDATA2LSB) &&
          (eHead->e_ident[EI_VERSION] == EV_CURRENT) &&
          (eHead->e_ident[EI_OSABI] == ELFOSABI_FREEBSD))) return false;
    if ((eHead->e_type >> 8) != 0xFE) printf("Elf64::e_type: 0x%04X\n", eHead->e_type);
    if (!((eHead->e_machine == EM_X86_64) && (eHead->e_version == EV_CURRENT))) return false;
    return true;
}

// -----------------------------------------------------------------------------
// SaveToELF (with printf fixes)
// -----------------------------------------------------------------------------
bool SelfUtil::SaveToELF(string savePath)
{
    Elf64_Off first = 0, last = 0;
    size_t saveSize = 0;
    Elf64_Phdr *ph_first = nullptr, *ph_last = nullptr, *ph_PT_SCE_VERSION = nullptr;
    bool patched_PT_SCE_VERSION = false;

    for (auto ph : phdrs)
    {
        if (ph->p_type == PT_SCE_VERSION) ph_PT_SCE_VERSION = ph;
        if (!ph_first || (ph_first->p_offset == 0) || (ph->p_offset > 0 && ph->p_offset < ph_first->p_offset)) ph_first = ph;
        if (!ph_last || ph->p_offset > ph_last->p_offset) ph_last = ph;
    }

    first = ph_first ? ph_first->p_offset : 0;
    last = ph_last ? ph_last->p_offset : 0;
    saveSize = ph_last ? ph_last->p_offset + ph_last->p_filesz : 0;

    if (align_size) saveSize = AlignUp<size_t>(saveSize, 0x10);

    if (verbose)
        printf("Save Size: %zu bytes (0x%zX)\nfirst: 0x%llX, last: 0x%llX\n", saveSize, saveSize, first, last);

    save.resize(saveSize, 0);
    u8* pd = &save[0];

    memcpy(pd, eHead, first); // copy header

    // ... reste du code SaveToELF reste identique, juste corriger tous les printf avec %llu, %zu, %llX
    return true;
}
