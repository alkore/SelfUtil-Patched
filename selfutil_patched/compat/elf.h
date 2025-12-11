#pragma once

#include <cstdint>

// Typedefs ELF64
typedef uint64_t Elf64_Addr;
typedef uint16_t Elf64_Half;
typedef uint64_t Elf64_Off;
typedef int32_t  Elf64_Sword;
typedef uint32_t Elf64_Word;
typedef uint64_t Elf64_Xword;
typedef int64_t  Elf64_Sxword;
typedef uint8_t  u8;
typedef uint32_t u32;
typedef uint64_t unat;

// ELF Identification Indexes
#ifndef EI_MAG0
#define EI_MAG0     0
#define EI_MAG1     1
#define EI_MAG2     2
#define EI_MAG3     3
#define EI_CLASS    4
#define EI_DATA     5
#define EI_VERSION  6
#define EI_OSABI    7
#define EI_ABIVERSION 8
#define EI_PAD      9
#define EI_NIDENT   16
#endif

// ELF Classes
#ifndef ELFCLASS64
#define ELFCLASS32 1
#define ELFCLASS64 2
#endif

// Data encoding
#ifndef ELFDATA2LSB
#define ELFDATA2LSB 1
#endif

// ELF Versions
#ifndef EV_CURRENT
#define EV_CURRENT 1
#endif

// OS ABI
#ifndef ELFOSABI_NONE
#define ELFOSABI_NONE 0
#endif
#ifndef ELFOSABI_FREEBSD
#define ELFOSABI_FREEBSD 9
#endif

// ELF Types
#ifndef ET_NONE
#define ET_NONE   0
#define ET_REL    1
#define ET_EXEC   2
#define ET_DYN    3
#define ET_CORE   4
#endif

// Machine
#ifndef EM_X86_64
#define EM_X86_64 62
#endif

// Program header types
#ifndef PT_NULL
#define PT_NULL    0
#define PT_LOAD    1
#define PT_DYNAMIC 2
#define PT_INTERP  3
#define PT_NOTE    4
#define PT_SHLIB   5
#define PT_PHDR    6
#define PT_TLS     7
#define PT_LOOS    0x60000000
#endif

#ifndef PT_SCE_VERSION
#define PT_SCE_VERSION (PT_LOOS + 0xfffff01) // .sce_version
#endif

// ELF Header
typedef struct elf64_hdr {
    unsigned char e_ident[16]; // ELF identification
    Elf64_Half e_type;         // Object file type
    Elf64_Half e_machine;      // Machine type
    Elf64_Word e_version;      // Object file version
    Elf64_Addr e_entry;        // Entry point address
    Elf64_Off  e_phoff;        // Program header offset
    Elf64_Off  e_shoff;        // Section header offset
    Elf64_Word e_flags;        // Processor-specific flags
    Elf64_Half e_ehsize;       // ELF header size
    Elf64_Half e_phentsize;    // Size of program header entry
    Elf64_Half e_phnum;        // Number of program header entries
    Elf64_Half e_shentsize;    // Size of section header entry
    Elf64_Half e_shnum;        // Number of section header entries
    Elf64_Half e_shstrndx;     // Section name string table index
} Elf64_Ehdr;

// Program header
typedef struct elf64_phdr {
    Elf64_Word  p_type;   // Type of segment
    Elf64_Word  p_flags;  // Segment attributes
    Elf64_Off   p_offset; // Offset in file
    Elf64_Addr  p_vaddr;  // Virtual address in memory
    Elf64_Addr  p_paddr;  // Physical address (unused)
    Elf64_Xword p_filesz; // Size of segment in file
    Elf64_Xword p_memsz;  // Size of segment in memory
    Elf64_Xword p_align;  // Alignment of segment
} Elf64_Phdr;

