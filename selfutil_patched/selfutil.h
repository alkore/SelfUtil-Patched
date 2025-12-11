#pragma once

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>
#include "compat/elf.h"

using namespace std;

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;
typedef uint64_t unat;

// --- SELF constants ---
constexpr u32 PS4_SELF_MAGIC = 0x53454C46; // 'SELF'
constexpr u32 PS5_SELF_MAGIC = 0x53454C46; // 'SELF' (PS5 may reuse same)
constexpr u32 ELF_MAGIC = 0x464C457F;      // 0x7F 'E' 'L' 'F'
constexpr u32 PT_SCE_VERSION = 0x60000000; // PS4/PS5 version segment type

// --- Global options ---
extern string input_file_path;
extern string output_file_path;
extern bool dry_run;
extern bool verbose;
extern bool overwrite;
extern bool align_size;
extern bool patch_first_segment_duplicate;
extern Elf64_Off patch_first_segment_safety_percentage;
extern bool patch_version_segment;
extern int first_min_offset;

// --- SELF structures ---
#pragma pack(push, 1)
struct Self_Hdr {
    u32 magic;
    u32 version;
    u64 num_entries;
    u64 dummy; // placeholder if needed
};

struct Self_Entry {
    u32 props;
    u32 fileSz;
    u64 memSz;
    u64 offs;
};
#pragma pack(pop)

// --- SelfUtil class ---
class SelfUtil {
public:
    SelfUtil() = default;
    SelfUtil(const string& path) { Load(path); }

    bool Load(string filePath);
    bool SaveToELF(string savePath);
    bool Parse();
    bool TestIdent();

private:
    Self_Hdr* seHead = nullptr;
    vector<Self_Entry*> entries;
    vector<Elf64_Phdr*> phdrs;
    elf64_hdr* eHead = nullptr;
    vector<u8> data;
    vector<u8> save;
    Elf64_Off elfHOffs = 0;
};

// --- Utility functions ---
bool compare_u8_array(u8* first_array, u8* second_array, int size);
void set_u8_array(u8* arr, int value, int size);
void print_usage();
