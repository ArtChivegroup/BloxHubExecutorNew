#include "injector.hpp"
#include <Windows.h>
#include <TlHelp32.h>
#include <fstream>
#include <vector>

#define valid_handle(x) (x != INVALID_HANDLE_VALUE && x != nullptr)

static bool load_file_into_memory(const std::string file_path, std::vector<uint8_t>& image) {
	std::ifstream file(file_path, std::ios::binary | std::ios::ate);
	if (!file)
		return false;

	const auto size = file.tellg();
	file.seekg(0);

	image.resize(size);
	file.read((char*)image.data(), size);

	return true;
}

injector::InjectionStatus injector::Inject(uint32_t pid, const std::string& dll_path) {
	HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
	if (!valid_handle(process))
		return InjectionStatus::FAILED;

	std::vector<uint8_t> image;
	if (!load_file_into_memory(dll_path, image)) {
		CloseHandle(process);
		return InjectionStatus::FAILED;
	}

	const auto* dos_header = (PIMAGE_DOS_HEADER)image.data();
	if (dos_header->e_magic != IMAGE_DOS_SIGNATURE) {
		CloseHandle(process);
		return InjectionStatus::FAILED;
	}

	auto* nt_headers = (PIMAGE_NT_HEADERS)(image.data() + dos_header->e_lfanew);
	if (nt_headers->Signature != IMAGE_NT_SIGNATURE) {
		CloseHandle(process);
		return InjectionStatus::FAILED;
	}

	const auto* remote_image = (uint8_t*)VirtualAllocEx(process, 0, nt_headers->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!remote_image) {
		CloseHandle(process);
		return InjectionStatus::FAILED;
	}

	std::vector<uint8_t> fixed_image(nt_headers->OptionalHeader.SizeOfImage);

	auto* section_header = IMAGE_FIRST_SECTION(nt_headers);
	for (auto idx = 0; idx < nt_headers->FileHeader.NumberOfSections; idx++) {
		memcpy((void*)(fixed_image.data() + section_header->VirtualAddress),
			image.data() + section_header->PointerToRawData,
			section_header->SizeOfRawData);
		section_header++;
	}

	// Apply base relocations
	auto* realloc_block = (PIMAGE_BASE_RELOCATION)(fixed_image.data() + nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
	while (realloc_block->VirtualAddress) {
		auto count = (realloc_block->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(uint16_t);
		auto* entries = (uint16_t*)(realloc_block + 1);
		for (auto i = 0; i < count; i++) {
			auto type = entries[i] >> 12;
			auto offset = entries[i] & 0xFFF;
			if (type == IMAGE_REL_BASED_DIR64) {
				uint64_t* patch_addr = (uint64_t*)(fixed_image.data() + realloc_block->VirtualAddress + offset);
				*patch_addr += (uint64_t)remote_image - nt_headers->OptionalHeader.ImageBase;
			}
		}
		realloc_block = (PIMAGE_BASE_RELOCATION)((uint8_t*)realloc_block + realloc_block->SizeOfBlock);
	}

	// Resolve imports
	auto* import_desc = (PIMAGE_IMPORT_DESCRIPTOR)(fixed_image.data() + nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
	while (import_desc->Name) {
		const char* dll_name = (const char*)(fixed_image.data() + import_desc->Name);
		HMODULE mod = LoadLibraryA(dll_name);
		if (mod) {
			auto* thunk = (PIMAGE_THUNK_DATA)(fixed_image.data() + import_desc->FirstThunk);
			auto* orig_thunk = import_desc->OriginalFirstThunk ? 
				(PIMAGE_THUNK_DATA)(fixed_image.data() + import_desc->OriginalFirstThunk) : thunk;
			while (orig_thunk->u1.AddressOfData) {
				uint64_t func_addr = 0;
				if (IMAGE_SNAP_BY_ORDINAL(orig_thunk->u1.Ordinal)) {
					func_addr = (uint64_t)GetProcAddress(mod, MAKEINTRESOURCEA(IMAGE_ORDINAL(orig_thunk->u1.Ordinal)));
				} else {
					auto* import_by_name = (PIMAGE_IMPORT_BY_NAME)(fixed_image.data() + orig_thunk->u1.AddressOfData);
					func_addr = (uint64_t)GetProcAddress(mod, import_by_name->Name);
				}
				thunk->u1.Function = func_addr;
				thunk++;
				orig_thunk++;
			}
		}
		import_desc++;
	}

	// Write fixed image to target
	if (!WriteProcessMemory(process, (void*)remote_image, fixed_image.data(), fixed_image.size(), nullptr)) {
		VirtualFreeEx(process, (void*)remote_image, 0, MEM_RELEASE);
		CloseHandle(process);
		return InjectionStatus::FAILED;
	}

	// Find BloxHubInit export
	uintptr_t bloxhub_init_addr = 0;
	if (nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress != 0) {
		auto* export_dir = (PIMAGE_EXPORT_DIRECTORY)(fixed_image.data() + nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
		auto* functions = (PDWORD)(fixed_image.data() + export_dir->AddressOfFunctions);
		auto* names = (PDWORD)(fixed_image.data() + export_dir->AddressOfNames);
		auto* ordinals = (PWORD)(fixed_image.data() + export_dir->AddressOfNameOrdinals);

		for (DWORD i = 0; i < export_dir->NumberOfNames; i++) {
			const char* name = (const char*)(fixed_image.data() + names[i]);
			if (strcmp(name, "BloxHubInit") == 0) {
				bloxhub_init_addr = (uintptr_t)remote_image + functions[ordinals[i]];
				break;
			}
		}
	}

	if (!bloxhub_init_addr) {
		VirtualFreeEx(process, (void*)remote_image, 0, MEM_RELEASE);
		CloseHandle(process);
		return InjectionStatus::FAILED;
	}

	// Create remote thread to execute BloxHubInit
	HANDLE thread = CreateRemoteThread(process, nullptr, 0, (LPTHREAD_START_ROUTINE)bloxhub_init_addr, nullptr, 0, nullptr);
	if (thread) {
		WaitForSingleObject(thread, 5000); // Wait up to 5 seconds
		CloseHandle(thread);
	} else {
		VirtualFreeEx(process, (void*)remote_image, 0, MEM_RELEASE);
		CloseHandle(process);
		return InjectionStatus::FAILED;
	}

	CloseHandle(process);
	return InjectionStatus::SUCCESS;
}
