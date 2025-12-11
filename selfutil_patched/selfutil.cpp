#include "selfutil.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <cstdio>

using namespace std;

// Chargement d’un fichier SELF
bool SelfUtil::Load(const string& filePath)
{
    ifstream file(filePath, ios::binary | ios::ate);
    if (!file.is_open())
    {
        cerr << "Failed to open file: " << filePath << endl;
        return false;
    }

    streamsize size = file.tellg();
    file.seekg(0, ios::beg);

    data.resize(size);
    if (!file.read((char*)data.data(), size))
    {
        cerr << "Failed to read file: " << filePath << endl;
        return false;
    }

    if (size < sizeof(uint32_t))
    {
        cerr << "File too small to be a SELF" << endl;
        return false;
    }

    uint32_t* magic = reinterpret_cast<uint32_t*>(data.data());
    if (*magic != PS4_SELF_MAGIC && *magic != PS5_SELF_MAGIC)
    {
        cerr << "Invalid SELF magic: 0x" << hex << *magic << endl;
        return false;
    }

    return true;
}

// Sauvegarde en ELF
bool SelfUtil::SaveToELF(const string& outFile)
{
    ofstream file(outFile, ios::binary);
    if (!file.is_open())
    {
        cerr << "Failed to open output file: " << outFile << endl;
        return false;
    }

    if (!data.empty())
        file.write((const char*)data.data(), data.size());

    return true;
}

// Patch segments
void SelfUtil::PatchSegments(bool patchFirstSegmentDuplicate, bool patchVersionSegment)
{
    if (patchFirstSegmentDuplicate)
    {
        // TODO: implémenter duplication du premier segment si nécessaire
    }

    if (patchVersionSegment)
    {
        for (size_t i = 0; i + sizeof(Elf64_Phdr) <= data.size(); i++)
        {
            Elf64_Phdr* ph = reinterpret_cast<Elf64_Phdr*>(&data[i]);
            if (ph->p_type == PT_SCE_VERSION)
            {
                ph->p_flags = 0x6; // exemple lecture + exécution
                break;
            }
        }
    }
}

// Affichage info
void SelfUtil::PrintInfo(const string& input_file_path,
                         const string& output_file_path,
                         bool dry_run,
                         bool verbose,
                         bool overwrite,
                         bool align_size,
                         bool patch_first_segment_duplicate,
                         bool patch_version_segment)
{
    printf(
        "Input File Name: %s\nOutput File Name: %s\nDry Run: %s\nVerbose: %s\nOverwrite: %s\nAlign Size: %s\nPatch First Segment Duplicate: %s\nPatch Version Segment: %s\n",
        input_file_path.c_str(),
        output_file_path.c_str(),
        dry_run ? "True" : "False",
        verbose ? "True" : "False",
        overwrite ? "True" : "False",
        align_size ? "True" : "False",
        patch_first_segment_duplicate ? "True" : "False",
        patch_version_segment ? "True" : "False");
}
