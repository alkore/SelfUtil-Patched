#pragma once
#include "compat/elf.h"
#include <string>
#include <vector>

using namespace std;

// Magic SELF
constexpr uint32_t PS4_SELF_MAGIC = 0x53454C46; // 'SELF'
constexpr uint32_t PS5_SELF_MAGIC = 0x53454C46; // identique

class SelfUtil
{
public:
    explicit SelfUtil(const string& filePath) : filePath(filePath) { }

    // Chargement d’un fichier SELF
    bool Load(const string& filePath);

    // Sauvegarde en ELF
    bool SaveToELF(const string& outFile);

    // Patch segments
    void PatchSegments(bool patchFirstSegmentDuplicate, bool patchVersionSegment);

    // Affichage info
    void PrintInfo(const string& input_file_path,
                   const string& output_file_path,
                   bool dry_run,
                   bool verbose,
                   bool overwrite,
                   bool align_size,
                   bool patch_first_segment_duplicate,
                   bool patch_version_segment);

private:
    string filePath;
    vector<uint8_t> data;
};
