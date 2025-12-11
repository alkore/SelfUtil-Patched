#pragma once

#include <string>
#include <vector>
#include <cstdio>
#include <filesystem>
#include "compat/elf.h"  // <-- inclus avant tout pour les typedef et macros ELF

using namespace std;

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long unat;

constexpr u32 ELF_MAGIC = 0x464C457F; // 0x7F 'E' 'L' 'F'
// PT_SCE_VERSION vient déjà de compat/elf.h

void print_usage();

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

bool compare_u8_array(u8* first_array, u8* second_array, int size);
void set_u8_array(u8* first_array, int value, int size);

class SelfUtil {
public:
    SelfUtil(string filePath) { Load(filePath); }
    bool Load(string filePath);
    bool SaveToELF(string savePath);
    bool Parse();
    bool TestIdent();

private:
    vector<u8> data;
    vector<u8> save;
    Self_Hdr* seHead;
    vector<Self_Entry*> entries;
    elf64_hdr* eHead;
    vector<Elf64_Phdr*> phdrs;
    Elf64_Off elfHOffs;
};
