// ListModules.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <stdio.h>
#include "..\ProcAccess\ProcAccessPublic.h"

int main(int argc, const char* argv[]) {
	int pid = atoi(argv[1]);
	auto hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (hProcess) {
		printf("Success in OpenProcess!\n");
		return 0;
	}
	printf("OpenProces failed!\n");

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
	}
	else {
		printf("Error (%u)\n", GetLastError());
	}
	CloseHandle(hDevice);
	return 0;
}
