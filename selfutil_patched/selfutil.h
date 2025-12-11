#pragma once

#include <cstdio>
#include <vector>
#include <string>
#include <cstdint>
#include "compat/elf.h"

using namespace std;

// -----------------------------------------------------------------------------
// Type aliases
// -----------------------------------------------------------------------------
using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using unat = uint64_t;

// -----------------------------------------------------------------------------
// PS4 / PS5 SELF constants
// -----------------------------------------------------------------------------
constexpr u32 PS4_SELF_MAGIC = 0x53454C46; // "SELF"
constexpr u32 PS5_SELF_MAGIC = 0x53534C46; // "SSLF"
constexpr size_t PS4_PAGE_SIZE = 0x1000;
constexpr size_t PS5_PAGE_SIZE = 0x2000;

// -----------------------------------------------------------------------------
// ELF constants
// -----------------------------------------------------------------------------
constexpr u32 ELF_MAGIC = 0x464C457F; // 0x7F 'E' 'L' 'F'
constexpr u32 PT_SCE_VERSION = 0x60000000; // PS4/PS5 version segment type

// -----------------------------------------------------------------------------
// SELF structures
// -----------------------------------------------------------------------------
#pragma pack(push, 1)
struct Self_Hdr
{
    u32 magic;
    u32 num_entries;
    u32 unk1;
    u32 unk2;
    // add more fields if needed
};

struct Self_Entry
{
    u64 offs;    // offset in SELF
    u64 fileSz;  // size in file
    u64 memSz;   // size in memory
    u32 props;   // flags
    u32 unk;     // unused/reserved
};
#pragma pack(pop)

// -----------------------------------------------------------------------------
// SelfUtil class
// -----------------------------------------------------------------------------
class SelfUtil
{
public:
    SelfUtil() = default;
    explicit SelfUtil(const string& path) { Load(path); }

    bool Load(string filePath);
    bool SaveToELF(string savePath);

private:
    bool Parse();
    bool TestIdent();

    vector<u8> data;
    vector<u8> save;
    Self_Hdr* seHead = nullptr;
    vector<Self_Entry*> entries;
    Elf64_Ehdr* eHead = nullptr;
    vector<Elf64_Phdr*> phdrs;
    Elf64_Off elfHOffs = 0;
};
