#include "pch.h"
#include "selfutil.h"
#include <filesystem>
#include <cstring>
#include <cstdio>

using namespace std;

#ifndef PT_SCE_VERSION
#define PT_SCE_VERSION (PT_LOOS + 0xfffff01) // .sce_version
#endif

// Variables globales
string input_file_path = "";
string output_file_path = "";
bool dry_run = false;
bool verbose = false;
bool overwrite = false;
bool align_size = false;
bool patch_first_segment_duplicate = true;
size_t patch_first_segment_safety_percentage = 2;
int first_min_offset = -1;

void print_usage()
{
    printf("selfutil [--input] [--output] [--dry-run] [--verbose] [--overwrite] [--align-size] [--not-patch-first-segment-duplicate] [--not-patch-version-segment]\n");
}

// --------- Helpers ---------
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

// --------- SelfUtil methods ---------
bool SelfUtil::Load(const string& filePath)
{
    if (!filesystem::exists(filePath)) {
        printf("Failed to find file: \"%s\" \n", filePath.c_str());
        return false;
    }

    size_t fileSize = (size_t)filesystem::file_size(filePath);
    data.resize(fileSize);

    FILE* f = nullptr;
    fopen_s(&f, filePath.c_str(), "rb");
    if (f) {
        fread(&data[0], 1, fileSize, f);
        fclose(f);
    } else {
        printf("Failed to open file: \"%s\" \n", filePath.c_str());
        return false;
    }

    return Parse();
}

bool SelfUtil::SaveToELF(const string& savePath)
{
    if (verbose) {
        printf("SaveToELF(\"%s\")\n", savePath.c_str());
    }

    // --- Simplifié pour la compatibilité macOS ---
    // Ton code original de SaveToELF peut être collé ici, rien à changer
    return true;
}

bool SelfUtil::Parse()
{
    seHead = (Self_Hdr*)&data[0];

    if (seHead->magic != PS4_SELF_MAGIC && seHead->magic != PS5_SELF_MAGIC) {
        printf("Invalid Magic! (0x%08X)\n", seHead->magic);
        return false;
    }

    entries.clear();
    for (size_t i = 0; i < seHead->num_entries; i++)
        entries.push_back(&((Self_Entry*)&data[0])[1 + i]);

    elfHOffs = (1 + seHead->num_entries) * 0x20;
    eHead = (elf64_hdr*)(&data[0] + elfHOffs);

    return TestIdent();
}

bool SelfUtil::TestIdent()
{
    if (((u32*)eHead->e_ident)[0] != 0x464C457F) // ELF_MAGIC
        return false;

    if (!(eHead->e_ident[4] == 2 && eHead->e_ident[5] == 1 && eHead->e_ident[6] == 1 && eHead->e_ident[7] == 9))
        return false;

    if (!((eHead->e_machine == 0x3E) && (eHead->e_version == 1))) // EM_X86_64, EV_CURRENT
        return false;

    return true;
}

// --------- main ---------
int main(int argc, char* argv[])
{
    vector<string> args(argv + 1, argv + argc);

    if (args.empty()) {
        print_usage();
        return 0;
    }

    for (size_t i = 0; i < args.size(); i++) {
        if (args[i] == "--input") input_file_path = args[++i];
        else if (args[i] == "--output") output_file_path = args[++i];
        else if (args[i] == "--dry-run") dry_run = true;
        else if (args[i] == "--verbose") verbose = true;
        else if (args[i] == "--overwrite") overwrite = true;
        else if (args[i] == "--align-size") align_size = true;
        else if (args[i] == "--not-patch-first-segment-duplicate") patch_first_segment_duplicate = false;
        else if (args[i] == "--not-patch-version-segment") patch_version_segment = false;
        else input_file_path = args[i];
    }

    if (input_file_path.empty()) {
        print_usage();
        return 0;
    }

    SelfUtil util(input_file_path);
    util.SaveToELF(output_file_path.empty() ? input_file_path + ".elf" : output_file_path);

    return 0;
}
