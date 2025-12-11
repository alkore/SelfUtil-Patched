#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <cstdio>
#include "compat/elf.h"
#include "self.h"

using namespace std;

// Typedefs
typedef uint8_t  u8;
typedef uint64_t u64;
typedef uint64_t unat;

class SelfUtil
{
    vector<u8> data, save;

    Self_Hdr* seHead;
    vector<Self_Entry*> entries;

    Elf64_Ehdr* eHead;
    unat elfHOffs;

    vector<Elf64_Phdr*> phdrs;

public:
    SelfUtil() {}

    SelfUtil(string filePath)
    {
        if(!Load(filePath))
            printf("Error, Load() failed!\n");
    }

    bool Parse();
    bool TestIdent();
    bool Load(string filePath);
    bool SaveToELF(string savePath);
};
