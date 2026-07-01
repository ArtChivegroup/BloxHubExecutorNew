#pragma once
#include <Windows.h>
#include <stdint.h>

typedef struct _EXPORT_ENTRY
{
    LPCSTR      pszName;
    ULONG_PTR   uFuncAddress;
    WORD        wOrdinal;
    LPCSTR      pszForward;
} EXPORT_ENTRY, *PEXPORT_ENTRY;

#define EDATA_SECTION_NAME      ".edata"
#define INVALID_ORDINAL         (WORD)(0xFFFFF)
#define ALIGN_UP(x, align)      (((x) + (align) - 1) & ~((align) - 1))

BOOL ConvertPayloadToProxy(
    LPCWSTR pwszPayloadPath,
    LPCWSTR pwszSystemDllPath,
    LPCWSTR pwszForwardDllName,
    OUT PBYTE* ppDllBuffer,
    OUT DWORD* pdwDllFileSize
);
