#pragma once
#include <cstddef> // size_t
#include <cstdint>
#include <vector>

typedef size_t unat;

// SELF constants
#define PS4_SELF_MAGIC 0x53454C46
#define PS5_SELF_MAGIC 0x50534546
#define PS4_PAGE_SIZE 0x1000
#define PS5_PAGE_SIZE 0x1000

struct Self_Hdr {
    u32 magic;
    u32 num_entries;
    // autres champs si nécessaire
};

struct Self_Entry {
    u64 offs;
    u64 fileSz;
    u64 memSz;
    u32 props;
};
