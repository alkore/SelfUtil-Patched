#include "selfutil.h"
#include <filesystem>
#include <cstdio>
#include <cstring>

using namespace std;

bool SelfUtil::Load(const string& filePath)
{
    if (!filesystem::exists(filePath)) {
        printf("File not found: \"%s\"\n", filePath.c_str());
        return false;
    }

    size_t fileSize = (size_t)filesystem::file_size(filePath);
    data.resize(fileSize);

    FILE* f = fopen(filePath.c_str(), "rb");
    if (!f) return false;
    fread(&data[0], 1, fileSize, f);
    fclose(f);

    return Parse();
}

bool SelfUtil::Parse()
{
    seHead = (Self_Hdr*)&data[0];

    if (seHead->magic != PS4_SELF_MAGIC && seHead->magic != PS5_SELF_MAGIC)
        return false;

    entries.clear();
    for (unat seIdx = 0; seIdx < seHead->num_entries; seIdx++)
        entries.push_back(&((Self_Entry*)&data[0])[1 + seIdx]);

    elfHOffs = (1 + seHead->num_entries) * sizeof(Self_Entry);
    eHead = (Elf64_Ehdr*)(&data[0] + elfHOffs);

    return TestIdent();
}

bool SelfUtil::TestIdent()
{
    if (((u32*)eHead->e_ident)[0] != ELF_MAGIC)
        return false;

    if (!((eHead->e_ident[EI_CLASS] == ELFCLASS64) &&
          (eHead->e_ident[EI_DATA] == ELFDATA2LSB) &&
          (eHead->e_ident[EI_VERSION] == EV_CURRENT) &&
          (eHead->e_ident[EI_OSABI] == ELFOSABI_FREEBSD)))
        return false;

    if (!((eHead->e_machine == EM_X86_64) && (eHead->e_version == EV_CURRENT)))
        return false;

    return true;
}

bool SelfUtil::SaveToELF(const string& savePath)
{
    save = data;
    if (!savePath.empty()) {
        FILE* f = fopen(savePath.c_str(), "wb");
        if (!f) return false;
        fwrite(&save[0], 1, save.size(), f);
        fclose(f);
    }
    return true;
}
