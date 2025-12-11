#pragma once
#include <cstdint>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

// ---- ELF Constants ----
#ifndef ELFCLASS64
#define ELFCLASS64 2
#endif

#ifndef ELFDATA2LSB
#define ELFDATA2LSB 1
#endif

#ifndef EV_CURRENT
#define EV_CURRENT 1
#endif

#ifndef ELFOSABI_FREEBSD
#define ELFOSABI_FREEBSD 9
#endif

#ifndef PT_LOOS
#define PT_LOOS 0x60000000
#endif

#ifndef PT_SCE_VERSION
#define PT_SCE_VERSION (PT_LOOS + 0xfffff01)
#endif

#ifndef ELF_MAGIC
#define ELF_MAGIC 0x464C457F
#endif

// ---- ELF types ----
#ifndef ELF64_EHDR_DEFINED
#define ELF64_EHDR_DEFINED
typedef struct elf64_hdr {
    u8  e_ident[16];
    u16 e_type;
    u16 e_machine;
    u32 e_version;
    u64 e_entry;
    u64 e_phoff;
    u64 e_shoff;
    u32 e_flags;
    u16 e_ehsize;
    u16 e_phentsize;
    u16 e_phnum;
    u16 e_shentsize;
    u16 e_shnum;
    u16 e_shstrndx;
} Elf64_Ehdr;
#endif

#ifndef ELF64_PHDR_DEFINED
#define ELF64_PHDR_DEFINED
typedef struct elf64_phdr {
    u32 p_type;
    u32 p_flags;
    u64 p_offset;
    u64 p_vaddr;
    u64 p_paddr;
    u64 p_filesz;
    u64 p_memsz;
    u64 p_align;
} Elf64_Phdr;
#endif
