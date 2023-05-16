#include <Windows.h>
#include <MinHook/include/MinHook.h>

void* o_IsNonPakFilenameAllowed{};

typedef long long(*FindFileInPakFiles_1_t)(void*, const wchar_t*, void**, void*);
FindFileInPakFiles_1_t o_FindFileInPakFiles_1;
FindFileInPakFiles_1_t o_FindFileInPakFiles_2;

long long hk_IsNonPakFilenameAllowed(void* this_ptr, void* InFilename) {
	return 1;
}

long long hk_FindFileInPakFiles_1(void* this_ptr, const wchar_t* Filename, void** OutPakFile, void* OutEntry) {
	auto attr{ GetFileAttributesW(Filename) };
	if (attr != INVALID_FILE_ATTRIBUTES && Filename && wcsstr(Filename, L"../../../")) {
		if (OutPakFile) OutPakFile = nullptr;
		return 0;
	}

	return o_FindFileInPakFiles_1(this_ptr, Filename, OutPakFile, OutEntry);
}

long long hk_FindFileInPakFiles_2(void* this_ptr, const wchar_t* Filename, void** OutPakFile, void* OutEntry) {
	auto attr{ GetFileAttributesW(Filename) };
	if (attr != INVALID_FILE_ATTRIBUTES && Filename && wcsstr(Filename, L"../../../")) {
		if (OutPakFile) OutPakFile = nullptr;
		return 0;
	}

	return o_FindFileInPakFiles_2(this_ptr, Filename, OutPakFile, OutEntry);
}

unsigned long main_thread(void* lpParameter) {
	MH_Initialize();

	unsigned char* module_base{ reinterpret_cast<unsigned char*>(GetModuleHandleA("Chivalry2-Win64-Shipping.exe")) };

	MH_CreateHook(module_base + 0x2FBBD10, hk_IsNonPakFilenameAllowed, &o_IsNonPakFilenameAllowed);
	MH_EnableHook(module_base + 0x2FBBD10);

	MH_CreateHook(module_base + 0x2FB71D0, hk_FindFileInPakFiles_1, reinterpret_cast<void**>(&o_FindFileInPakFiles_1));
	MH_EnableHook(module_base + 0x2FB71D0);

	MH_CreateHook(module_base + 0x2FB72B0, hk_FindFileInPakFiles_2, reinterpret_cast<void**>(&o_FindFileInPakFiles_2));
	MH_EnableHook(module_base + 0x2FB72B0);

	ExitThread(0);
	return 0;
}

int __stdcall DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH: {
		HANDLE thread_handle{ CreateThread(NULL, 0, main_thread, hModule, 0, NULL) };
		if (thread_handle) CloseHandle(thread_handle);
		break;
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return 1;
}