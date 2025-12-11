#pragma once

#include <cstdint>
#include <vector>
#include <string>

using namespace std;

// -------------------- TYPES --------------------
using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using unat = uintptr_t;
using Elf64_Off = uint64_t;

// -------------------- ELF CONSTANTS --------------------
#define ELF_MAGIC       0x464C457F
#define ELFCLASS64      2
#define ELFDATA2LSB     1
#define EV_CURRENT      1
#define ELFOSABI_FREEBSD 9
#define EM_X86_64       62
#define PT_SCE_VERSION  0x60000000

// -------------------- SELF CONSTANTS --------------------
#define PS4_SELF_MAGIC  0x53454C46
#define PS5_SELF_MAGIC  0x5053454C
#define PS4_PAGE_SIZE   0x1000
#define PS5_PAGE_SIZE   0x1000

// -------------------- ELF STRUCTS --------------------
struct elf64_hdr
{
    u8  e_ident[16];
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

struct Elf64_Phdr
{
    u32 p_type;
    u32 p_flags;
    u64 p_offset;
    u64 p_vaddr;
    u64 p_paddr;
    u64 p_filesz;
    u64 p_memsz;
    u64 p_align;
};

// -------------------- SELF STRUCTS --------------------
struct Self_Hdr
{
    u32 magic;
    u32 num_entries;
    u64 unused[6]; // padding/reserved
};

struct Self_Entry
{
    u32 props;
    u32 reserved;
    u64 offs;
    u64 fileSz;
    u64 memSz;
};

// -------------------- SELFUTIL CLASS --------------------
class SelfUtil
{
    vector<u8> data, save;

    Self_Hdr* seHead;
    vector<Self_Entry*> entries;

    elf64_hdr* eHead;
    unat elfHOffs;

    vector<Elf64_Phdr*> phdrs;

public:
    SelfUtil() {}

    SelfUtil(string filePath)
    {
        if (!Load(filePath))
            printf("Error, Load() failed!\n");
    }

    bool Parse();
    bool TestIdent();

    bool Load(string filePath);
    bool SaveToELF(string savePath);
};
