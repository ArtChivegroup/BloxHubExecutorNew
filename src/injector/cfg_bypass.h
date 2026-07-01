#pragma once
#include <Windows.h>
#include <cstdint>

#define CFG_PAGE_HASH_KEY  0xF7455279
#define CFG_VALIDATION_XOR 0xA9

#define HashPage(Page) \
    (((uintptr_t)(Page) >> 12) ^ CFG_PAGE_HASH_KEY)

#define ValidationByte(Page) \
    (((uintptr_t)(Page) >> 44) ^ CFG_VALIDATION_XOR)

struct CfgOffsets {
    uintptr_t whitelist;    // address of std::unordered_set in Roblox process
    uintptr_t bitmap_ptr;   // address of CFG bitmap pointer in Roblox process
    uintptr_t insert_set;   // address of insert_set function in Roblox process
    bool valid;
};

struct CfgWhitelistEntry {
    uint8_t padding[0x14];
    uint32_t next[4];       // hash table node pointers (offset 0x14)
    uint32_t hash;          // at +0x18: HashPage(page)
    uint8_t  validation;    // at +0x1C: ValidationByte(page)
    uint8_t  pad2[0x23];
};

// Find CFG-related offsets by scanning RobloxPlayerBeta.dll code
CfgOffsets FindCfgOffsets(HANDLE process, uintptr_t moduleBase, DWORD moduleSize);

// Execute CFG bypass for a specific memory region (from injector process)
bool BypassCfgForRegion(HANDLE process, const CfgOffsets& offsets, uintptr_t regionBase, DWORD regionSize);

// Direct bitmap patch - writes to Roblox's CFG bitmap from external process
bool PatchCfgBitmap(HANDLE process, uintptr_t bitmap_addr, uintptr_t regionBase, DWORD regionSize);
