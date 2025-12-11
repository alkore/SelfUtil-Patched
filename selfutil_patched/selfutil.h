#pragma once

#include <vector>
#include <string>
#include <cstdio>
#include <filesystem>
#include "elf.h"
#include "self.h"

using namespace std;

// --- Compat macOS pour ELF ---
#ifndef EI_CLASS
#define EI_CLASS      4
#define EI_DATA       5
#define EI_VERSION    6
#define EI_OSABI      7
#endif

#ifndef ELFCLASS64
#define ELFCLASS64    2
#endif

#ifndef ELFDATA2LSB
#define ELFDATA2LSB   1
#endif

#ifndef EV_CURRENT
#define EV_CURRENT    1
#endif

#ifndef ELFOSABI_FREEBSD
#define ELFOSABI_FREEBSD 9
#endif

#ifndef ELF_MAGIC
#define ELF_MAGIC 0x464C457F
#endif

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
        if(!Load(filePath))
            printf("Error, Load() failed!\n");
    }

    bool Parse();
    bool TestIdent();
    bool Load(string filePath);
    bool SaveToELF(string savePath);
};
