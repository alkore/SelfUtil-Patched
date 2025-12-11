#include "pch.h"
#include "selfutil.h"
#include <filesystem>
#include <cstring>

using namespace std;

// --- Fonctions utilitaires ---

bool compare_u8_array(u8* first_array, u8* second_array, int size) {
    for (int i = 0; i < size; i++)
        if (first_array[i] != second_array[i])
            return false;
    return true;
}

void set_u8_array(u8* arr, int value, int size) {
    memset(arr, value, size);
}

// --- print usage ---
void print_usage() {
    printf(
        "selfutil [--input <file>] [--output <file>] [--dry-run] [--verbose] "
        "[--overwrite] [--align-size] [--not-patch-first-segment-duplicate] "
        "[--not-patch-version-segment]\n"
    );
}

// --- SelfUtil methods ---

bool SelfUtil::Load(string filePath) {
    if (!filesystem::exists(filePath)) {
        printf("Failed to find file: \"%s\"\n", filePath.c_str());
        return false;
    }

    size_t fileSize = filesystem::file_size(filePath);
    data.resize(fileSize);

    FILE* f = fopen(filePath.c_str(), "rb");
    if (!f) {
        printf("Failed to open file: \"%s\"\n", filePath.c_str());
        return false;
    }
    fread(&data[0], 1, fileSize, f);
    fclose(f);

    return Parse();
}

bool SelfUtil::Parse() {
    seHead = (Self_Hdr*)&data[0];

    if (seHead->magic != PS4_SELF_MAGIC && seHead->magic != PS5_SELF_MAGIC) {
        printf("Invalid Magic! (0x%08X)\n", seHead->magic);
        return false;
    }

    entries.clear();
    for (unat i = 0; i < seHead->num_entries; i++)
        entries.push_back(&((Self_Entry*)&data[0])[1 + i]);

    elfHOffs = (1 + seHead->num_entries) * 0x20;
    eHead = (elf64_hdr*)(&data[0] + elfHOffs);

    if (!TestIdent()) {
        printf("ELF e_ident invalid!\n");
        return false;
    }

    for (unat i = 0; i < eHead->e_phnum; i++)
        phdrs.push_back(&((Elf64_Phdr*)(&data[0] + elfHOffs + eHead->e_phoff))[i]);

    return true;
}

bool SelfUtil::TestIdent() {
    if (*(u32*)eHead->e_ident != ELF_MAGIC) {
        printf("Invalid ELF Magic: 0x%08X\n", *(u32*)eHead->e_ident);
        return false;
    }

    if (eHead->e_ident[EI_CLASS] != ELFCLASS64 ||
        eHead->e_ident[EI_DATA] != ELFDATA2LSB ||
        eHead->e_ident[EI_VERSION] != EV_CURRENT ||
        eHead->e_ident[EI_OSABI] != ELFOSABI_FREEBSD)
        return false;

    if (eHead->e_machine != EM_X86_64 || eHead->e_version != EV_CURRENT)
        return false;

    return true;
}

bool SelfUtil::SaveToELF(string savePath) {
    if (verbose) printf("SaveToELF(\"%s\")\n", savePath.c_str());

    Elf64_Off first = 0, last = 0;
    size_t saveSize = 0;
    Elf64_Phdr *ph_first = nullptr, *ph_last = nullptr, *ph_PT_SCE_VERSION = nullptr;
    bool patched_PT_SCE_VERSION = false;

    for (auto ph : phdrs) {
        if (ph->p_type == PT_SCE_VERSION) ph_PT_SCE_VERSION = ph;
        if (!ph_first || (ph->p_offset > 0 && ph->p_offset < ph_first->p_offset)) ph_first = ph;
        if (!ph_last || ph->p_offset > ph_last->p_offset) ph_last = ph;
    }

    first = ph_first ? ph_first->p_offset : 0;
    if (ph_last) saveSize = align_size ? ((ph_last->p_offset + ph_last->p_filesz + 0xF) & ~0xF) : (ph_last->p_offset + ph_last->p_filesz);

    save.clear();
    save.resize(saveSize, 0);
    u8* pd = save.data();

    // Copy ELF header + program headers
    memcpy(pd, eHead, first);

    // Copy entries
    for (auto ee : entries) {
        bool patch = ((ee->props & 0x800) || (ph_PT_SCE_VERSION && phdrs[(ee->props >> 20) & 0xFFF] == ph_PT_SCE_VERSION && patch_version_segment));
        if (!patch) continue;

        Elf64_Phdr* ph = phdrs[(ee->props >> 20) & 0xFFF];
        void* dstp = pd + ph->p_offset;
        void* srcp = (ph_PT_SCE_VERSION && ph == ph_PT_SCE_VERSION)
                        ? data.data() + data.size() - ph->p_filesz
                        : data.data() + ee->offs;
        size_t datasize = (ph_PT_SCE_VERSION && ph == ph_PT_SCE_VERSION) ? ph->p_filesz : ee->fileSz;
        memcpy(dstp, srcp, datasize);

        if (ph_PT_SCE_VERSION && ph == ph_PT_SCE_VERSION) {
            patched_PT_SCE_VERSION = true;
            if (verbose) printf("Patched PT_SCE_VERSION segment\n");
        }
    }

    // Patch first segment duplicate
    if (patch_first_segment_duplicate && first > 0) {
        int minOffset = -1;
        for (int i = 0; i < entries.size(); i++)
            if (entries[i]->offs - elfHOffs >= 0 && entries[i]->offs - elfHOffs < first)
                minOffset = max(minOffset, int(entries[i]->offs - elfHOffs));

        if (minOffset != -1) set_u8_array(pd + minOffset, 0, first - minOffset);
    }

    if (!dry_run) {
        FILE* f = fopen(savePath.c_str(), "wb");
        if (!f) return false;
        fwrite(pd, 1, saveSize, f);
        fclose(f);
    }

    return true;
}
