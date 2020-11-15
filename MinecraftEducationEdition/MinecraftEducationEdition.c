
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <psapi.h>
#include <wchar.h>
#include <string.h>
#include <TlHelp32.h>

int* pointer_path;
int num_ptr;

uintptr_t GetProcessBaseAddress(HANDLE process) // from stackoverflow
{
	DWORD_PTR   baseAddress = 0;
	HANDLE      processHandle = OpenProcess(PROCESS_ALL_ACCESS,TRUE,process);
	HMODULE*    moduleArray;
	LPBYTE      moduleArrayBytes;
	DWORD       bytesRequired;

	if (processHandle)
	{
		if (EnumProcessModules(processHandle, NULL, 0, &bytesRequired))
		{
			if (bytesRequired)
			{
				moduleArrayBytes = (LPBYTE)LocalAlloc(LPTR, bytesRequired);

				if (moduleArrayBytes)
				{
					int moduleCount;

					moduleCount = bytesRequired / sizeof(HMODULE);
					moduleArray = (uintptr_t*)moduleArrayBytes;


					if (EnumProcessModules(processHandle, moduleArray, bytesRequired, &bytesRequired))
					{
						baseAddress = moduleArray[0];
					}

					LocalFree(moduleArrayBytes);
				}
			}
		}

		CloseHandle(processHandle);
	}

	return baseAddress;
}

DWORD GetProcId(WCHAR* name)
{
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry) == TRUE)
	{
		while (Process32Next(snapshot, &entry) == TRUE)
		{
			if (wcscmp(entry.szExeFile, name) == 0)
			{
				return entry.th32ProcessID;
			}
		}
	}

	CloseHandle(snapshot);
	return NULL;
}

int main(int argc, char* argv[])
{
	FILE* ptr_file;
	char MEE_POINTER_FILE[0x2048];
	int LOGIN_STEP_VALUE = -1;

	char* tmp;

	#ifdef _WIN64
	printf_s("!!! x64 Version can ONLY be used for the 64 Bit Versions of the game!\n");
	#else
	printf_s("!!! x86 Version can ONLY be used for the 32 Bit Versions of the game!\n");
	#endif
	
	strncpy_s(MEE_POINTER_FILE, 0x2048, "mee.ptr", 0x2048);
    if(argc > 1)
	{
		for (int i = 0; i < argc; i++)
		{

			if (strcmp(argv[i], "--help") == 0)
			{
				printf_s("--ptr <mee.ptr file>\n");
				printf_s("--lstep <custom login step value>\n");
				return;
			}

			if(strcmp(argv[i],"--ptr") == 0)
				strncpy_s(MEE_POINTER_FILE, 0x2048, argv[i+1], 0x2048);

			if (strcmp(argv[i], "--lstep") == 0)
				LOGIN_STEP_VALUE = strtol(argv[i + 1], &tmp, 10);

		}

		printf_s("MEE.PTR FILE : %s\nLOGIN STEP VALUE: %i\n", MEE_POINTER_FILE, LOGIN_STEP_VALUE);
	}

	// Read text file
	printf_s("Loading %s\n", MEE_POINTER_FILE);
	if ((access(MEE_POINTER_FILE, 0)) != -1)
	{
		fopen_s(&ptr_file, MEE_POINTER_FILE, "r");
		fseek(ptr_file, 0, SEEK_END);
		int sz = ftell(ptr_file)+1;
		fseek(ptr_file, 0, SEEK_SET);

		char* file_contents = (char*)malloc(sz);
		memset(file_contents, 0x00, sz);
		fread(file_contents, sz, 1, ptr_file);

		char* work_buf = (char*)malloc(sz);
		memcpy_s(work_buf, sz, file_contents, sz);

		num_ptr = 0;
		char* next_token1 = NULL;
		char* token = strtok_s(work_buf, " > ", &next_token1);

		// Count number of ptrs
		while (token != NULL) {
			token = strtok_s(NULL, " > ",&next_token1);
			num_ptr += 1;
		}

		pointer_path = (int*)malloc(num_ptr * sizeof(int));

		work_buf = (char*)malloc(sz);
		memcpy_s(work_buf, sz, file_contents, sz);

		char* next_token2 = NULL;

		char* ptrs = strtok_s(work_buf, " > ",&next_token2);
		pointer_path[0] = (int)strtol(ptrs, &tmp, 16);

		// Use ptr
		
		for(int i = 1; i < num_ptr; i++){
			ptrs = strtok_s(NULL, " > ", &next_token2);
			pointer_path[i] = (int)strtol(ptrs, &tmp, 16);
		}

		fclose(ptr_file);

		printf_s("Loaded %s!\n", MEE_POINTER_FILE);
	}
	else
	{
		printf_s("Failed, using default pointer path (MCEE 1.12.60 UWP x64)\n");
		num_ptr = 8;
		pointer_path = (int*)malloc(num_ptr * sizeof(int));
        
		pointer_path[0] = 0x2CFFF40;
		pointer_path[1] = 0x30;
		pointer_path[2] = 0x8;
		pointer_path[3] = 0x20;
		pointer_path[4] = 0x570;
		pointer_path[5] = 0x18;
		pointer_path[6] = 0xA8;
		pointer_path[7] = 0x0;
	}

	printf_s("\nPointer Path: ");
	for (int i = 0; i < num_ptr; i++)
	{
		printf_s("%x", pointer_path[i]);
		if (i != num_ptr - 1)
		{
			printf_s(" > ");
		}
	}
	printf_s("\n");

	// Hack the universe.
	DWORD proc_id = NULL;

	printf_s("\nPlease open Minecraft Education Edition\n");
	while (proc_id == NULL)
	{
		proc_id = GetProcId(L"Minecraft.Windows.exe");
		if (proc_id == NULL)
			proc_id = GetProcId(L"Minecraft.Win10.DX11.exe");
	}

	printf_s("MCEE Process ID: %x\n", proc_id);
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, proc_id);
	printf_s("MCEE Process Handle: %x\n", hProcess);
	
	if (!hProcess)
	{
		MessageBox(NULL, L"Cannot open process!\r\nTry \"Run as administrator\"", L"Error!", MB_OK + MB_ICONERROR);
	}
	else
	{

		uintptr_t baseAddress = NULL;
	    while(baseAddress == NULL)
			baseAddress = (uintptr_t)GetProcessBaseAddress(proc_id);
		
		printf_s("MCEE Base Addr: %llx\n", baseAddress);


		printf_s("Waiting for game to initalize....\n");

	read_ptr_path:

		baseAddress = (uintptr_t)GetProcessBaseAddress(proc_id); // recalculate base address idk why but this seems to be required.
		

		// Read first ptr
		uintptr_t first_ptr = pointer_path[0];
		uintptr_t cur_ptr = baseAddress + first_ptr;
		uintptr_t ptr = 0;
		uintptr_t new_ptr = 0;



		ReadProcessMemory(hProcess, cur_ptr, &ptr, sizeof(uintptr_t), 0);
		if (ptr == 0)
			goto read_ptr_path;
		
		
		for (int i = 1; i < num_ptr-1; i++) // Follow path...
		{


			cur_ptr = ptr + pointer_path[i];
			ReadProcessMemory(hProcess, cur_ptr, &new_ptr, sizeof(uintptr_t), 0);
			if (new_ptr == 0) {
				i -= 1;
				goto read_ptr_path;
			}
			else
			{
				ptr = new_ptr;
				
			}
				

		}

		// Wait for 0x1
		int login_step_value = 0;
		ReadProcessMemory(hProcess, (void*)ptr, &login_step_value, sizeof(int), 0);

		if (login_step_value != 0x0)
		{
			if (LOGIN_STEP_VALUE != -1)
			{
				printf_s("Trying login stage %i", LOGIN_STEP_VALUE);
				WriteProcessMemory(hProcess, (void*)ptr, &LOGIN_STEP_VALUE, sizeof(int), 0);
				goto finish;
			}

			printf_s("Trying login stage 5...\n"); // Backwards Comp (0.xx)
			int login_step_value = 5;
			WriteProcessMemory(hProcess, (void*)ptr, &login_step_value, sizeof(int), 0);

			Sleep(1 * 200);

			printf_s("Trying login stage 6...\n"); // Backwards Comp (1.9 and lower)
			login_step_value = 6;
			WriteProcessMemory(hProcess, (void*)ptr, &login_step_value, sizeof(int), 0);

			Sleep(1 * 200);

			printf_s("Trying login stage 8...\n");
			login_step_value = 8;
			WriteProcessMemory(hProcess, (void*)ptr, &login_step_value, sizeof(int), 0);

		}
		else
		{
			goto read_ptr_path;
		}

		finish:
		

		CloseHandle(hProcess);

		printf_s("\nBlessed Be!\n");
		return 0;
	}
}

