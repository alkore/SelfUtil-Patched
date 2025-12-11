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

    Elf64_Ehdr* eHead;
    unat elfHOffs;
    vector<Elf64_Phdr*> phdrs;

public:
    SelfUtil() {}
    SelfUtil(const string& filePath) { Load(filePath); }

    bool Parse();
    bool TestIdent();

    bool Load(const string& filePath);
    bool SaveToELF(const string& savePath);
};
