#pragma once
#include <cstdint>

typedef uint8_t  u8;
typedef uint32_t u32;
typedef uint64_t u64;
typedef size_t   unat;

#define PS4_SELF_MAGIC 0x53454C46
#define PS5_SELF_MAGIC 0x53454C46

struct Self_Hdr {
    u32 magic;
    u32 num_entries;
};

struct Self_Entry {
    u32 props;
    u64 offs;
    u64 fileSz;
    u64 memSz;
};
