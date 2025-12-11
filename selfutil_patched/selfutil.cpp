#include "pch.h"
#include "selfutil.h"
#include <filesystem>
#include <cstring>
#include <cstdio>

using namespace std;

void print_usage()
{
    printf("selfutil [--input] [--output] [--dry-run] [--verbose] [--overwrite] [--align-size] [--not-patch-first-segment-duplicate] [--not-patch-version-segment]\n");
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

int main(int argc, char* argv[])
{
    vector<string> args;

    if (argc < 2) {
        print_usage();
        return 0;
    }

    bool error_found = false;

    for (int i = 1; i < argc; i++)
        args.push_back(argv[i]);

    if (argc == 2)
        input_file_path = args[0];
    else
    {
        for (size_t i = 0; i < args.size(); i++)
        {
            if (args[i] == "--input" && i + 1 < args.size()) {
                input_file_path = args[++i];
            }
            else if (args[i] == "--output" && i + 1 < args.size()) {
                output_file_path = args[++i];
            }
            else if (args[i] == "--dry-run")
                dry_run = true;
            else if (args[i] == "--verbose")
                verbose = true;
            else if (args[i] == "--overwrite")
                overwrite = true;
            else if (args[i] == "--align-size")
                align_size = true;
            else if (args[i] == "--not-patch-first-segment-duplicate")
                patch_first_segment_duplicate = false;
            else if (args[i] == "--not-patch-version-segment")
                patch_version_segment = false;
            else {
                error_found = true;
                break;
            }
        }
    }

    if (error_found || input_file_path.empty())
    {
        print_usage();
        return 0;
    }

    string fixed_output_file_path = output_file_path.empty() ? input_file_path : output_file_path;

    if (!overwrite && output_file_path.empty())
    {
        size_t dot = input_file_path.rfind('.');
        if (dot != string::npos) {
            fixed_output_file_path = input_file_path.substr(0, dot) + ".elf";
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

    if (!util.Load(input_file_path) || !util.SaveToELF(fixed_output_file_path))
        printf("Error, Save to ELF failed!\n");

    return 0;
}

// ---------------------------------
// Utilitaires
bool compare_u8_array(u8* first_array, u8* second_array, int size)
{
    for (int i = 0; i < size; i++)
        if (first_array[i] != second_array[i])
            return false;
    return true;
}

void set_u8_array(u8* array, int value, int size)
{
    memset(array, value, size);
}

// ---------------------------------
// SelfUtil methods
bool SelfUtil::Load(string filePath)
{
    if (!filesystem::exists(filePath)) {
        printf("Failed to find file: \"%s\"\n", filePath.c_str());
        return false;
    }

    size_t fileSize = filesystem::file_size(filePath);
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

bool SelfUtil::Parse()
{
    seHead = (Self_Hdr*)&data[0];

    if (seHead->magic != PS4_SELF_MAGIC && seHead->magic != PS5_SELF_MAGIC)
    {
        printf("Invalid Magic! (0x%08X)\n", seHead->magic);
        return false;
    }

    entries.clear();
    for (u32 i = 0; i < seHead->num_entries; i++)
    {
        entries.push_back(&((Self_Entry*)&data[0])[1 + i]);
        if (verbose) {
            Self_Entry* e = entries.back();
            printf("Segment[%02d] P:%08X @0x%016llX +%llX (mem:%llX)\n", i, e->props, e->offs, e->fileSz, e->memSz);
        }
    }

    elfHOffs = (1 + seHead->num_entries) * 0x20;
    eHead = (Elf64_Ehdr*)(&data[0] + elfHOffs);

    return TestIdent();
}

bool SelfUtil::TestIdent()
{
    if (*(u32*)eHead->e_ident != ELF_MAGIC)
        return false;

    if (!(eHead->e_ident[EI_CLASS] == ELFCLASS64 &&
          eHead->e_ident[EI_DATA] == ELFDATA2LSB &&
          eHead->e_ident[EI_VERSION] == EV_CURRENT &&
          eHead->e_ident[EI_OSABI] == ELFOSABI_FREEBSD))
        return false;

    if (!(eHead->e_machine == EM_X86_64 && eHead->e_version == EV_CURRENT))
        return false;

    return true;
}
