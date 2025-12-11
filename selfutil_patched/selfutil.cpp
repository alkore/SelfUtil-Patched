#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>

using namespace std;

// ------------------- ELF definitions -------------------
using u32 = uint32_t;
using u64 = uint64_t;

constexpr u32 ELF_MAGIC = 0x464C457F; // 0x7F 'E' 'L' 'F'
constexpr u32 PT_SCE_VERSION = 0x60000000; // PS4/PS5 version segment type

struct Elf64_Ehdr {
    unsigned char e_ident[16];
    u16 e_type;
    u16 e_machine;
    u32 e_version;
    u64 e_entry;
    u64 e_phoff;
    u64 e_shoff;
    u32 e_flags;
    u16 e_ehsize;
    u16 e_phentsize;
    u16 e_phnum;
    u16 e_shentsize;
    u16 e_shnum;
    u16 e_shstrndx;
};

struct Elf64_Phdr {
    u32 p_type;
    u32 p_flags;
    u64 p_offset;
    u64 p_vaddr;
    u64 p_paddr;
    u64 p_filesz;
    u64 p_memsz;
    u64 p_align;
};

// ------------------- SELF definitions -------------------

constexpr u32 PS4_SELF_MAGIC = 0x53454C46; // 'SELF'
constexpr u32 PS5_SELF_MAGIC = 0x53454C46; // same for simplicity

struct SelfHeader {
    u32 magic;
    u32 version;
    u64 fileSize;
    // add more fields as needed
};

// ------------------- SelfUtil class -------------------

class SelfUtil {
public:
    string filePath;
    vector<uint8_t> data;
    SelfHeader* seHead = nullptr;

    SelfUtil(const string& filePath) : filePath(filePath) { }

    bool Load(const string& filePath)
    {
        this->filePath = filePath;
        ifstream in(filePath, ios::binary | ios::ate);
        if (!in.is_open()) return false;

        size_t size = in.tellg();
        in.seekg(0, ios::beg);

        data.resize(size);
        in.read(reinterpret_cast<char*>(data.data()), size);
        in.close();

        if (data.size() < sizeof(SelfHeader)) return false;

        seHead = reinterpret_cast<SelfHeader*>(data.data());
        if (seHead->magic != PS4_SELF_MAGIC && seHead->magic != PS5_SELF_MAGIC)
            return false;

        return true;
    }

    void PrintInfo(const string& inputFile, const string& outputFile,
                   bool dry_run, bool verbose, bool overwrite,
                   bool align_size, bool patch_first_segment_duplicate,
                   bool patch_version_segment)
    {
        cout << "Input File Name: " << inputFile << "\n"
             << "Output File Name: " << outputFile << "\n"
             << "Dry Run: " << (dry_run ? "True" : "False") << "\n"
             << "Verbose: " << (verbose ? "True" : "False") << "\n"
             << "Overwrite: " << (overwrite ? "True" : "False") << "\n"
             << "Align Size: " << (align_size ? "True" : "False") << "\n"
             << "Patch First Segment Duplicate: " << (patch_first_segment_duplicate ? "True" : "False") << "\n"
             << "Patch Version Segment: " << (patch_version_segment ? "True" : "False") << "\n";
    }
};

// ------------------- Main executable -------------------

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        cerr << "Usage: selfutil <SELF file>\n";
        return 1;
    }

    string input_file_path = argv[1];
    SelfUtil util(input_file_path);

    if (!util.Load(input_file_path))
    {
        cerr << "Failed to load SELF file: " << input_file_path << endl;
        return 1;
    }

    util.PrintInfo(input_file_path, "out.elf", false, true, false, true, false, false);
    cout << "SELF file loaded successfully!" << endl;

    return 0;
}
