#include "selfutil.h"
#include <iostream>
#include <fstream>

// ------------------- SelfUtil class implementation -------------------

SelfUtil::SelfUtil(const std::string& filePath) : filePath(filePath) {}

bool SelfUtil::Load(const std::string& filePath)
{
    this->filePath = filePath;
    std::ifstream in(filePath, std::ios::binary | std::ios::ate);
    if (!in.is_open())
        return false;

    size_t size = in.tellg();
    in.seekg(0, std::ios::beg);

    data.resize(size);
    in.read(reinterpret_cast<char*>(data.data()), size);
    in.close();

    // Check magic (PS4/PS5 SELF)
    if (data.size() < sizeof(SelfHeader))
        return false;

    seHead = reinterpret_cast<SelfHeader*>(data.data());
    if (seHead->magic != PS4_SELF_MAGIC && seHead->magic != PS5_SELF_MAGIC)
        return false;

    return true;
}

void SelfUtil::PrintInfo(const std::string& inputFile, const std::string& outputFile,
                         bool dry_run, bool verbose, bool overwrite,
                         bool align_size, bool patch_first_segment_duplicate,
                         bool patch_version_segment)
{
    std::cout << "Input File Name: " << inputFile << "\n"
              << "Output File Name: " << outputFile << "\n"
              << "Dry Run: " << (dry_run ? "True" : "False") << "\n"
              << "Verbose: " << (verbose ? "True" : "False") << "\n"
              << "Overwrite: " << (overwrite ? "True" : "False") << "\n"
              << "Align Size: " << (align_size ? "True" : "False") << "\n"
              << "Patch First Segment Duplicate: " << (patch_first_segment_duplicate ? "True" : "False") << "\n"
              << "Patch Version Segment: " << (patch_version_segment ? "True" : "False") << "\n";
}

// ------------------- Main executable entry -------------------

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: selfutil <SELF file>\n";
        return 1;
    }

    std::string input_file_path = argv[1];

    SelfUtil util(input_file_path);
    if (!util.Load(input_file_path))
    {
        std::cerr << "Failed to load SELF file: " << input_file_path << std::endl;
        return 1;
    }

    util.PrintInfo(input_file_path, "out.elf", false, true, false, true, false, false);

    std::cout << "SELF file loaded successfully!" << std::endl;
    return 0;
}
