#pragma once

#include <cstdint>

// -----------------------------------------------------------------------------
// Basic ELF types
// -----------------------------------------------------------------------------
typedef uint16_t Elf64_Half;
typedef uint32_t Elf64_Word;
typedef int32_t  Elf64_Sword;
typedef uint64_t Elf64_Xword;
typedef int64_t  Elf64_Sxword;
typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Off;
typedef uint16_t Elf64_Section;
typedef uint16_t Elf64_Versym;

// -----------------------------------------------------------------------------
// ELF Header
// -----------------------------------------------------------------------------
typedef struct {
    unsigned char e_ident[16];
    Elf64_Half    e_type;
    Elf64_Half    e_machine;
    Elf64_Word    e_version;
    Elf64_Addr    e_entry;
    Elf64_Off     e_phoff;
    Elf64_Off     e_shoff;
    Elf64_Word    e_flags;
    Elf64_Half    e_ehsize;
    Elf64_Half    e_phentsize;
    Elf64_Half    e_phnum;
    Elf64_Half    e_shentsize;
    Elf64_Half    e_shnum;
    Elf64_Half    e_shstrndx;
} Elf64_Ehdr;

// -----------------------------------------------------------------------------
// Program Header
// -----------------------------------------------------------------------------
typedef struct {
    Elf64_Word p_type;
    Elf64_Word p_flags;
    Elf64_Off  p_offset;
    Elf64_Addr p_vaddr;
    Elf64_Addr p_paddr;
    Elf64_Xword p_filesz;
    Elf64_Xword p_memsz;
    Elf64_Xword p_align;
} Elf64_Phdr;

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------
#ifndef PT_SCE_VERSION
#define PT_SCE_VERSION (PT_LOOS + 0xfffff01)  // .sce_version
#endif

#ifndef ELFCLASS64
#define ELFCLASS64 2
#endif

#define ELF_MAGIC 0x464C457F  // 0x7F 'E' 'L' 'F'
#define EV_CURRENT 1
#define EI_CLASS 4
#define EI_DATA 5
#define EI_VERSION 6
#define EI_OSABI 7
#define ELFDATA2LSB 1
#define ELFOSABI_FREEBSD 9
#define EM_X86_64 62
