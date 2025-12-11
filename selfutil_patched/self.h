#pragma once
#include <cstdint>
#include <cstddef>

typedef size_t unat;

#define PS4_SELF_MAGIC 0x53454C46
#define PS5_SELF_MAGIC 0x50534546
#define PS4_PAGE_SIZE 0x1000
#define PS5_PAGE_SIZE 0x1000

struct Self_Hdr {
    u32 magic;
    u32 num_entries;
};

struct Self_Entry {
    u64 offs;
    u64 fileSz;
    u64 memSz;
    u32 props;
};
