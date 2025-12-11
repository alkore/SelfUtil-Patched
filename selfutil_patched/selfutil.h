#pragma once

#include "elf.h"
#include "self.h"
#include <string>
#include <vector>

using namespace std;

class SelfUtil
{
    vector<u8> data, save;

    Self_Hdr* seHead;
    vector<Self_Entry*> entries;

    elf64_hdr* eHead;
    size_t elfHOffs;

    vector<Elf64_Phdr*> phdrs;

public:
    SelfUtil() {}

    SelfUtil(const string& filePath)
    {
        if (!Load(filePath))
            printf("Error, Load() failed!\n");
    }

    bool Parse();
    bool TestIdent();

    bool Load(const string& filePath);
    bool SaveToELF(const string& savePath);
};
