#include "pa5a.hpp"

EXTERN_C_START

/********************************************************************************************************************************/
HMODULE hKernel32 = NULL;
HMODULE hUser32 = NULL;

SetWindowsHookExAProc MySetWindowsHookExA = NULL;
SetWindowsHookExWProc MySetWindowsHookExW = NULL;
VirtualAllocExProc MyVirtualAllocEx = NULL;
WriteProcessMemoryProc MyWriteProcessMemory = NULL;
CreateRemoteThreadProc MyCreateRemoteThread = NULL;
CreateToolhelp32SnapshotProc MyCreateToolhelp32Snapshot = NULL;
Process32FirstWProc MyProcess32FirstW = NULL;
Process32NextWProc MyProcess32NextW = NULL;
GetWindowThreadProcessIdProc MyGetWindowThreadProcessId = NULL;
OpenProcessProc MyOpenProcess = NULL;
TerminateProcessProc MyTerminateProcess = NULL;
CreateProcessAProc MyCreateProcessA = NULL;
CreateProcessWProc MyCreateProcessW = NULL;
GetSystemFirmwareTableProc MyGetSystemFirmwareTable = NULL;

List listWebSockets = { 0 };
struct lws_context* context = NULL;

/********************************************************************************************************************************/
BOOL loader()
{
    if (!(hKernel32 = GetModuleHandle(L"Kernel32.dll")) || !(hUser32 = GetModuleHandle(L"User32.dll")))
        return FALSE;

    if (!(MySetWindowsHookExA = (SetWindowsHookExAProc)GetProcAddress(hUser32, "SetWindowsHookExA")))
        return FALSE;

    if (!(MySetWindowsHookExW = (SetWindowsHookExWProc)GetProcAddress(hUser32, "SetWindowsHookExW")))
        return FALSE;

    if (!(MyVirtualAllocEx = (VirtualAllocExProc)GetProcAddress(hKernel32, "VirtualAllocEx")))
        return FALSE;

    if (!(MyWriteProcessMemory = (WriteProcessMemoryProc)GetProcAddress(hKernel32, "WriteProcessMemory")))
        return FALSE;

    if (!(MyCreateRemoteThread = (CreateRemoteThreadProc)GetProcAddress(hKernel32, "CreateRemoteThread")))
        return FALSE;

    if (!(MyCreateToolhelp32Snapshot = (CreateToolhelp32SnapshotProc)GetProcAddress(hKernel32, "CreateToolhelp32Snapshot")))
        return FALSE;

    if (!(MyProcess32FirstW = (Process32FirstWProc)GetProcAddress(hKernel32, "Process32FirstW")))
        return FALSE;

    if (!(MyProcess32NextW = (Process32NextWProc)GetProcAddress(hKernel32, "Process32NextW")))
        return FALSE;

    if (!(MyGetWindowThreadProcessId = (GetWindowThreadProcessIdProc)GetProcAddress(hUser32, "GetWindowThreadProcessId")))
        return FALSE;

    if (!(MyCreateProcessA = (CreateProcessAProc)GetProcAddress(hKernel32, "CreateProcessA")))
        return FALSE;

    if (!(MyCreateProcessW = (CreateProcessWProc)GetProcAddress(hKernel32, "CreateProcessW")))
        return FALSE;

    if (!(MyOpenProcess = (OpenProcessProc)GetProcAddress(hKernel32, "OpenProcess")))
        return FALSE;

    if (!(MyTerminateProcess = (TerminateProcessProc)GetProcAddress(hKernel32, "TerminateProcess")))
        return FALSE;

    if (!(MyGetSystemFirmwareTable = (GetSystemFirmwareTableProc)GetProcAddress(hKernel32, "GetSystemFirmwareTable")))
        return FALSE;

    return TRUE;
}
/********************************************************************************************************************************/

EXTERN_C_END