#pragma once

#include "compat/elf.h"
#include "self.h"
#include <vector>
#include <string>
#include <cstdio>

using namespace std;

class SelfUtil
{
    vector<uint8_t> data, save;

    Self_Hdr* seHead;
    vector<Self_Entry*> entries;

    Elf64_Ehdr* eHead;   // anciennement elf64_hdr*
    uint64_t elfHOffs;

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
