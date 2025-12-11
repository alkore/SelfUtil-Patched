#include "pch.h"
#include "selfutil.h"
#include <cstring>
#include <cstdio>
#include <filesystem>

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

// --- utilitaires ---
bool compare_u8_array(u8* first_array, u8* second_array, int size)
{
    for (int i = 0; i < size; i++)
        if (first_array[i] != second_array[i])
            return false;
    return true;
}

void set_u8_array(u8* arr, int value, int size)
{
    for (int i = 0; i < size; i++)
        arr[i] = value;
}

// --- SelfUtil methods ---
bool SelfUtil::Load(string filePath)
{
    if (!filesystem::exists(filePath)) {
        printf("Failed to find file: \"%s\" \n", filePath.c_str());
        return false;
    }

    size_t fileSize = (size_t)filesystem::file_size(filePath);
    data.resize(fileSize);

    FILE* f = fopen(filePath.c_str(), "rb");
    if (f) {
        fread(&data[0], 1, fileSize, f);
        fclose(f);
    }
    else {
        printf("Failed to open file: \"%s\" \n", filePath.c_str());
        return false;
    }

    return Parse();
}

bool SelfUtil::Parse()
{
    seHead = (Self_Hdr*)&data[0];

    if (seHead->magic == PS4_SELF_MAGIC) {
        if (verbose) printf("Valid PS4 Magic\n");
    }
    else if (seHead->magic == PS5_SELF_MAGIC) {
        if (verbose) printf("Valid PS5 Magic\n");
    }
    else {
        printf("Invalid Magic! (0x%08X)\n", seHead->magic);
        return false;
    }

    entries.clear();
    for (unat seIdx = 0; seIdx < seHead->num_entries; seIdx++)
        entries.push_back(&((Self_Entry*)&data[0])[1 + seIdx]);

    elfHOffs = (1 + seHead->num_entries) * 0x20;
    eHead = (elf64_hdr*)(&data[0] + elfHOffs);

    if (!TestIdent()) {
        printf("Elf e_ident invalid!\n");
        return false;
    }

    for (unat phIdx = 0; phIdx < eHead->e_phnum; phIdx++)
        phdrs.push_back(&((Elf64_Phdr*)(&data[0] + elfHOffs + eHead->e_phoff))[phIdx]);

    return true;
}

bool SelfUtil::TestIdent()
{
    if (ELF_MAGIC != ((u32*)eHead->e_ident)[0]) {
        printf("File is invalid! e_ident magic: %08X\n", ((u32*)eHead->e_ident)[0]);
        return false;
    }

    if (!(
        (eHead->e_ident[EI_CLASS] == ELFCLASS64) &&
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

bool SelfUtil::SaveToELF(string savePath)
{
    // ici tu peux copier le code SaveToELF de ton fichier actuel, 
    // juste remplacer fopen_s par fopen et vérifier memset/memcpy

    return true; // placeholder
}
