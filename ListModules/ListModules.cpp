// ListModules.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <stdio.h>
#include <Psapi.h>
#include "..\ProcAccess\ProcAccessPublic.h"

bool ListModules(HANDLE hProcess) {
	HMODULE hModule[1024];
	DWORD needed;
	if (!EnumProcessModules(hProcess, hModule, sizeof(hModule), &needed))
		return false;

	WCHAR name[256];
	for (ULONG i = 0; i < needed / sizeof(HMODULE); i++) {
		::GetModuleBaseName(hProcess, hModule[i], name, _countof(name));
		printf("0x%p: %ws\n", hModule[i], name);
	}
	return true;
}

int main(int argc, const char* argv[]) {
	if (argc < 2) {
		printf("Usage: ListModules <pid>\n");
		return 0;
	}
	int pid = atoi(argv[1]);
	auto hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (hProcess) {
		printf("Success in OpenProcess!\n");
		ListModules(hProcess);
		return 0;
	}
	printf("OpenProcess failed!\n");

	HANDLE hDevice = CreateFile(L"\\\\.\\ProcAccess",
		GENERIC_WRITE | GENERIC_READ, 0, nullptr,
		OPEN_EXISTING, 0, nullptr);
	if (hDevice == INVALID_HANDLE_VALUE) {
		printf("Error opening device (%u)\n", GetLastError());
		return 1;
	}

	DWORD ret;
	if (DeviceIoControl(hDevice, IOCTL_OPEN_PROCESS,
		&pid, sizeof(pid), &hProcess, sizeof(hProcess),
		&ret, nullptr)) {
		printf("Success!\n");
		ListModules(hProcess);
	}
	else {
		printf("Error (%u)\n", GetLastError());
	}
	CloseHandle(hDevice);
	return 0;
}
