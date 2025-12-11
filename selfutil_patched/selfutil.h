#pragma once
#include <string>
#include <vector>
#include "compat/elf.h"
#include "self.h"

using namespace std;

class SelfUtil
{
    vector<u8> data, save;

    Self_Hdr* seHead;
    vector<Self_Entry*> entries;

    Elf64_Ehdr* eHead;
    size_t elfHOffs;

    vector<Elf64_Phdr*> phdrs;

public:
    SelfUtil() {}

    // Constructeur unique pour éviter double définition
    explicit SelfUtil(const string& filePath) {
        if(!Load(filePath))
            printf("Error, Load() failed!\n");
    }

    bool Parse();
    bool TestIdent();
    bool Load(const string& filePath);
    bool SaveToELF(const string& savePath);
};
