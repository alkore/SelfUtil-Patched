#pragma once

#include <string>
#include <vector>
#include <cstdio>
#include <cstdint>
#include <filesystem>
#include "compat/elf.h"  // IMPORTANT : utiliser uniquement compat/elf.h

using std::string;
using std::vector;
using std::uint8_t;
using u8 = uint8_t;
using u32 = uint32_t;
using u64 = uint64_t;
using std::size_t;

struct Self_Hdr {
    u32 magic;
    u32 num_entries;
    // autres champs nécessaires...
};

struct Self_Entry {
    u32 props;
    u64 offs;
    u64 fileSz;
    u64 memSz;
};

class SelfUtil
{
public:
    SelfUtil(const string& filePath) : filePath(filePath) { }
    
    bool Load(const string& filePath);
    bool SaveToELF(const string& savePath);
    bool Parse();
    bool TestIdent();

    vector<u8> data;
    vector<u8> save;
    vector<Self_Entry*> entries;
    vector<Elf64_Phdr*> phdrs;

    Self_Hdr* seHead = nullptr;
    Elf64_Ehdr* eHead = nullptr;
    size_t elfHOffs = 0;

private:
    string filePath;
};

// Fonctions utilitaires
bool compare_u8_array(u8* first_array, u8* second_array, int size);
void set_u8_array(u8* first_array, int value, int size);

// Macros déjà définies dans compat/elf.h, plus besoin de constexpr
// #define ELF_MAGIC 0x464C457F  // pas à redéfinir
// #define PT_SCE_VERSION (PT_LOOS + 0xfffff01)

