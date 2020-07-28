
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <psapi.h>
#include <TlHelp32.h>

int* pointer_path;
int num_ptr;


HMODULE* GetProcessBaseAddress(HANDLE process) // from stackoverflow
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
					moduleArray = (HMODULE*)moduleArrayBytes;

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

int main(int argc, char* argv[])
{
	HWND hWnd = NULL;
	FILE* ptr_file;
	char MEE_POINTER_FILE[0x2048];
	#ifdef _WIN64
	printf_s("!!! x64 Version can ONLY be used for the 64 Bit Versions of the game!\n");
	#else
	printf_s("!!! x86 Version can ONLY be used for the 32 Bit Versions of the game!\n");
	#endif

	if (argc == 1)
	{
		strncpy_s(MEE_POINTER_FILE, 0x2048, "mee.ptr", 0x2048);
	}
	else
	{
		strncpy_s(MEE_POINTER_FILE, 0x2048, argv[1], 0x2048);
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
		char* tmp;

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
        
		pointer_path[0] = 0x2594A58;
		pointer_path[1] = 0x60;
		pointer_path[2] = 0x138;
		pointer_path[3] = 0x58;
		pointer_path[4] = 0x480;
		pointer_path[5] = 0x10;
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

	printf_s("\n\nPlease open Minecraft Education Edition\n");
	while (hWnd == NULL)
	{
		hWnd = FindWindow(0, L"Minecraft: Education Edition");
	}
	printf_s("MCEE Window Handle: %x\n", hWnd);
	DWORD proc_id;
	GetWindowThreadProcessId(hWnd, &proc_id);
	printf_s("MCEE Process ID: %x\n", proc_id);
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, proc_id);
	printf_s("MCEE Process Handle: %x\n", hProcess);
	#ifdef _WIN64
	long long int baseAddress = (long long int)GetProcessBaseAddress(proc_id);
	printf_s("MCEE Base Addr: %llx\n", baseAddress);
	#else
	int baseAddress = (int)GetProcessBaseAddress(hProcess);
	printf_s("MCEE Base Addr: %x\n", baseAddress);
	#endif
	
	if (!hProcess)
	{
		MessageBox(NULL, L"Cannot open process!\r\nTry \"Run as administrator\"", L"Error!", MB_OK + MB_ICONERROR);
	}
	else
	{
		// Read first ptr
		printf_s("Waiting for game to Initalize.\n");
         #ifdef _WIN64
		long long int cur_ptr = baseAddress + pointer_path[0];
		long long int ptr = 0;
		#else
		int cur_ptr = baseAddress + pointer_path[0];
		int ptr = 0;
		#endif

		while (ptr == 0)
		{
            #ifdef _WIN64
		    ReadProcessMemory(hProcess, cur_ptr, &ptr, sizeof(long long int), 0);
			#else
			ReadProcessMemory(hProcess, cur_ptr, &ptr, sizeof(int), 0);
			#endif
		}
		
		printf_s("Pointer 1: %x == %x\n", cur_ptr, ptr);

		for (int i = 1; i < num_ptr-1; i++) // Follow path...
		{
			#ifdef _WIN64
			long long int new_ptr = 0;
			#else
			int new_ptr = 0;
			#endif

			cur_ptr = ptr + pointer_path[i];
			#ifdef _WIN64
			ReadProcessMemory(hProcess, cur_ptr, &new_ptr, sizeof(long long int), 0);
			#else
			ReadProcessMemory(hProcess, cur_ptr, &new_ptr, sizeof(int), 0);
			#endif
			if (new_ptr == 0) {
				i -= 1;
				continue;
			}
			else
			{
				ptr = new_ptr;
			}
				

			printf_s("Pointer %i: %x == %x\n", i, cur_ptr, ptr);
		}

		// Wait for 0x1
		printf_s("Waiting for login screen.\n");
		int login_stage = 0;

		while (1)
		{
			ReadProcessMemory(hProcess, (void*)ptr, &login_stage, sizeof(int), 0);
			if (login_stage == 0x1 || login_stage == 0x4)
			{
				printf_s("Trying login stage 6...\n"); // Backwards Comp (1.9 and lower)
				int login_success = 6;
				WriteProcessMemory(hProcess, (void*)ptr, &login_success, sizeof(int), 0);

				Sleep(1 * 500);

				printf_s("Trying login stage 8...\n");
				login_success = 8;
				WriteProcessMemory(hProcess, (void*)ptr, &login_success, sizeof(int), 0);

				break;
			}
		}

		CloseHandle(hProcess);

		printf_s("\nBlessed Be!\n");
		Sleep(5 * 1000);
		return 0;
	}
}

