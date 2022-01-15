#include "pa5a.hpp"

EXTERN_C_START

/********************************************************************************************************************************/
// Set linker to use TLS and define address of static TLS callback
#ifdef _WIN64
#pragma comment (linker, "/INCLUDE:_tls_used")
#pragma comment (linker, "/INCLUDE:tlsCallbacksArray")
#else
#pragma comment (linker, "/INCLUDE:__tls_used")
#pragma comment (linker, "/INCLUDE:_tlsCallbacksArray")
#endif

/*****************************************************************************
Definition of the TLS Callback functions to execute.
/*****************************************************************************/
VOID staticTlsCallback(PVOID hModule, DWORD dwReason, PVOID pContext);
VOID dynamicTlsCallback(PVOID hModule, DWORD dwReason, PVOID pContext);

// Declare one TLS callback to the CRT
#ifdef _WIN64
#pragma const_seg(".CRT$XLB")
EXTERN_C const PIMAGE_TLS_CALLBACK tlsCallbacksArray = (PIMAGE_TLS_CALLBACK)staticTlsCallback;
#pragma const_seg()
#else
#pragma data_seg(".CRT$XLB")
EXTERN_C PIMAGE_TLS_CALLBACK tlsCallbacksArray = (PIMAGE_TLS_CALLBACK)staticTlsCallback;
#pragma data_seg()
#endif

EXTERN_C INT IsBeingDebuggedAsm(VOID);
/********************************************************************************************************************************/
// Static TLS callback
VOID staticTlsCallback(PVOID hModule, DWORD dwReason, PVOID pContext)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		PIMAGE_TLS_CALLBACK* nextCallback = NULL;
		DWORD oldProtection = 0;

		nextCallback = (PIMAGE_TLS_CALLBACK*)&tlsCallbacksArray + 1;

		// Change memory protection in array of callback for dynamic callback
		VirtualProtect(nextCallback, sizeof(nextCallback), PAGE_EXECUTE_READWRITE, &oldProtection);

		// Setting next callback to be dynamic TLS callback
		*nextCallback = (PIMAGE_TLS_CALLBACK)dynamicTlsCallback;

		// Setting NULL terminating array
		nextCallback = (PIMAGE_TLS_CALLBACK*)&tlsCallbacksArray + 2;
		VirtualProtect(nextCallback, sizeof(nextCallback), PAGE_EXECUTE_READWRITE, &oldProtection);
		*nextCallback = (PIMAGE_TLS_CALLBACK)NULL;
	}
}
/********************************************************************************************************************************/
// Dynamic TLS callback
VOID dynamicTlsCallback(PVOID hModule, DWORD dwReason, PVOID pContext)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		#if _DEBUG
			CreateConsole();
		#endif

		printf("%s\n", loader() ? "LoadLibrary -> OK" : "LoadLibrary -> ERROR");
		
		#if NDEBUG
			if (IsBeingDebuggedAsm())
				MyTerminateProcess(GetCurrentProcess(), 0);
		#endif
	}
}
/********************************************************************************************************************************/

EXTERN_C_END