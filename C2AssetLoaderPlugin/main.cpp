#include <Windows.h>
#include <MinHook/include/MinHook.h>

void* o_IsNonPakFilenameAllowed{};

typedef long long(*FindFileInPakFiles_1_t)(void*, const wchar_t*, void**, void*);
FindFileInPakFiles_1_t o_FindFileInPakFiles_1;
FindFileInPakFiles_1_t o_FindFileInPakFiles_2;

long long hk_IsNonPakFilenameAllowed(void* this_ptr, void* InFilename) { // 48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 56 48 83 EC 30 48 8B F1 45 33 C0 48 8D 4C 24 ? 4C 8B F2 
	return 1;
}

long long hk_FindFileInPakFiles_1(void* this_ptr, const wchar_t* Filename, void** OutPakFile, void* OutEntry) { // 48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 54 41 55 41 56 41 57 48 83 EC 30 33 FF 
	auto attr{ GetFileAttributesW(Filename) };
	if (attr != INVALID_FILE_ATTRIBUTES && Filename && wcsstr(Filename, L"../../../")) {
		if (OutPakFile) OutPakFile = nullptr;
		return 0;
	}

	return o_FindFileInPakFiles_1(this_ptr, Filename, OutPakFile, OutEntry);
}

long long hk_FindFileInPakFiles_2(void* this_ptr, const wchar_t* Filename, void** OutPakFile, void* OutEntry) { // Next function under FindFileInPakFiles_1
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

	MH_CreateHook(module_base + 0x2FC3CE0, hk_IsNonPakFilenameAllowed, &o_IsNonPakFilenameAllowed);
	MH_EnableHook(module_base + 0x2FC3CE0);

	MH_CreateHook(module_base + 0x2FBF1A0, hk_FindFileInPakFiles_1, reinterpret_cast<void**>(&o_FindFileInPakFiles_1));
	MH_EnableHook(module_base + 0x2FBF1A0);

	MH_CreateHook(module_base + 0x2FBF280, hk_FindFileInPakFiles_2, reinterpret_cast<void**>(&o_FindFileInPakFiles_2));
	MH_EnableHook(module_base + 0x2FBF280);

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