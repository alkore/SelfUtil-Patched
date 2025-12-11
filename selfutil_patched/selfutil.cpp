// selfutil.cpp
#include "pch.h"
#include "selfutil.h"
#include "compat/elf.h"
#include <filesystem>
#include <cstdio>
#include <vector>
#include <string>
#include <cstring>

using namespace std;

// ---- Constantes spécifiques SELF ----
constexpr u32 PS4_SELF_MAGIC = 0x53434546; // 'SELF'
constexpr u32 PS5_SELF_MAGIC = 0x53434550; // 'SCEP' par exemple
// -------------------------------------

void print_usage()
{
    printf("selfutil [--input] [--output] [--dry-run] [--verbose] [--overwrite] [--align-size] [--not-patch-first-segment-duplicate] [--not-patch-version-segment]\n");
}

// Variables globales
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

// ------------------------------------------------------------
// MAIN
// ------------------------------------------------------------
int main(int argc, char* argv[])
{
    vector<string> args;

    if (argc < 2) {
        print_usage();
        return 0;
    }

    for (int i = 1; i < argc; i++)
        args.push_back(argv[i]);

    bool error_found = false;

    if (argc == 2)
        input_file_path = args[0];
    else
        for (size_t i = 0; i < args.size(); i++)
        {
            if (args[i] == "--input") { input_file_path = args[++i]; }
            else if (args[i] == "--output") { output_file_path = args[++i]; }
            else if (args[i] == "--dry-run") dry_run = true;
            else if (args[i] == "--verbose") verbose = true;
            else if (args[i] == "--overwrite") overwrite = true;
            else if (args[i] == "--align-size") align_size = true;
            else if (args[i] == "--not-patch-first-segment-duplicate") patch_first_segment_duplicate = false;
            else if (args[i] == "--not-patch-version-segment") patch_version_segment = false;
            else { error_found = true; break; }
        }

    if (error_found || input_file_path.empty())
    {
        print_usage();
        return 0;
    }

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
        "%s: %s\n%s: %s\n%s: %s\n%s: %s\n%s: %s\n%s: %s\n"
        , "Input File Name", input_file_path.c_str()
        , "Output File Name", fixed_output_file_path.c_str()
        , "Dry Run", dry_run ? "True" : "False"
        , "Verbose", verbose ? "True" : "False"
        , "Overwrite", overwrite ? "True" : "False"
        , "Align Size", align_size ? "True" : "False"
        , "Patch First Segment Duplicate", patch_first_segment_duplicate ? "True" : "False"
        , "Patch Version Segment", patch_version_segment ? "True" : "False"
    );

    SelfUtil util;
    if (!util.Load(input_file_path))
    {
        printf("Error, failed to load input file.\n");
        return 1;
    }

    if (!util.SaveToELF(fixed_output_file_path))
        printf("Error, Save to ELF failed!\n");

    return 0;
}

// ------------------------------------------------------------
// SelfUtil::Load
// ------------------------------------------------------------
bool SelfUtil::Load(const string& filePath)
{
    if (!filesystem::exists(filePath)) {
        printf("Failed to find file: \"%s\"\n", filePath.c_str());
        return false;
    }

    size_t fileSize = (size_t)filesystem::file_size(filePath);
    data.resize(fileSize);

    FILE* f = fopen(filePath.c_str(), "rb");
    if (!f) { printf("Failed to open file: \"%s\"\n", filePath.c_str()); return false; }
    fread(data.data(), 1, fileSize, f);
    fclose(f);

    return Parse();
}

// ------------------------------------------------------------
// SelfUtil::Parse
// ------------------------------------------------------------
bool SelfUtil::Parse()
{
    seHead = reinterpret_cast<Self_Hdr*>(data.data());

    if (seHead->magic != PS4_SELF_MAGIC && seHead->magic != PS5_SELF_MAGIC)
    {
        printf("Invalid Magic! (0x%08X)\n", seHead->magic);
        return false;
    }

    entries.clear();
    for (unat i = 0; i < seHead->num_entries; i++)
        entries.push_back(&reinterpret_cast<Self_Entry*>(data.data())[1 + i]);

    elfHOffs = (1 + seHead->num_entries) * 0x20;
    eHead = reinterpret_cast<Elf64_Ehdr*>(data.data() + elfHOffs);

    if (!TestIdent()) { printf("Elf e_ident invalid!\n"); return false; }

    for (unat phIdx = 0; phIdx < eHead->e_phnum; phIdx++)
        phdrs.push_back(&reinterpret_cast<Elf64_Phdr*>(data.data() + elfHOffs + eHead->e_phoff)[phIdx]);

    return true;
}

// ------------------------------------------------------------
// SelfUtil::TestIdent
// ------------------------------------------------------------
bool SelfUtil::TestIdent()
{
    if (ELF_MAGIC != *reinterpret_cast<u32*>(eHead->e_ident)) {
        printf("File is invalid! e_ident magic: %08X\n", *reinterpret_cast<u32*>(eHead->e_ident));
        return false;
    }

    if (!((eHead->e_ident[EI_CLASS] == ELFCLASS64) &&
          (eHead->e_ident[EI_DATA] == ELFDATA2LSB) &&
          (eHead->e_ident[EI_VERSION] == EV_CURRENT) &&
          (eHead->e_ident[EI_OSABI] == ELFOSABI_FREEBSD)))
        return false;

    if ((eHead->e_type >> 8) != 0xFE)
        printf(" Elf64::e_type: 0x%04X \n", eHead->e_type);

    if (!((eHead->e_machine == EM_X86_64) && (eHead->e_version == EV_CURRENT)))
        return false;

    return true;
}
