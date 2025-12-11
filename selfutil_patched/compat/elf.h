#ifndef COMPAT_ELF_H
#define COMPAT_ELF_H

#include <stdint.h>

// ------------------- Types de base -------------------
typedef uint16_t Elf64_Half;
typedef uint32_t Elf64_Word;
typedef int32_t  Elf64_Sword;
typedef uint64_t Elf64_Xword;
typedef int64_t  Elf64_Sxword;
typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Off;

// ------------------- Ident -------------------
#define EI_NIDENT 16

#define EI_MAG0     0
#define EI_MAG1     1
#define EI_MAG2     2
#define EI_MAG3     3
#define EI_CLASS    4
#define EI_DATA     5
#define EI_VERSION  6
#define EI_OSABI    7
#define EI_ABIVER   8
#define EI_PAD      9

#define ELFDATA2LSB 1
#define EV_CURRENT  1
#define ELFOSABI_FREEBSD 9

// ------------------- ELF Header -------------------
typedef struct {
    unsigned char e_ident[EI_NIDENT];
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

// ------------------- Program Header -------------------
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

// ------------------- Segment types -------------------
#define PT_NULL    0
#define PT_LOAD    1
#define PT_DYNAMIC 2
#define PT_INTERP  3
#define PT_NOTE    4
#define PT_SHLIB   5
#define PT_PHDR    6
#define PT_TLS     7
#define PT_LOOS    0x60000000
#define PT_HIOS    0x6fffffff
#define PT_LOPROC  0x70000000
#define PT_HIPROC  0x7fffffff

// Custom segment used by your code
#define PT_SCE_VERSION (PT_LOOS + 0xfffff01)  // .sce_version

// ------------------- Machine types -------------------
#define EM_X86_64 62

// ------------------- ELF flags -------------------
#define PF_R 0x4
#define PF_W 0x2
#define PF_X 0x1

// ------------------- Notes -------------------
typedef struct {
    Elf64_Word n_namesz;
    Elf64_Word n_descsz;
    Elf64_Word n_type;
} Elf64_Nhdr;

// ------------------- Dynamic section -------------------
typedef struct {
    Elf64_Sxword d_tag;
    union {
        Elf64_Xword d_val;
        Elf64_Addr d_ptr;
    } d_un;
} Elf64_Dyn;

extern Elf64_Dyn _DYNAMIC[];

#endif
