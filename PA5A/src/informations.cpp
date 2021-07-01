#include "pa5a.hpp"

EXTERN_C_START

/********************************************************************************************************************************/
BOOL dmi_system_uuid(const PBYTE biosTableData, SHORT version, LPSTR uuid)
{
    BOOL only0xFF = TRUE;
    BOOL only0x00 = TRUE;

    for (DWORD i = 0; i < 16 && (only0x00 || only0xFF); i++)
    {
        if (biosTableData[i] != 0x00)
            only0x00 = FALSE;
        if (biosTableData[i] != 0xFF)
            only0xFF = FALSE;
    }

    if (only0xFF)
    {
        StringCbPrintfA(uuid, UUID_SIZE, "UUID Not Present");
        printf("UUID Not Present");
        return FALSE;
    }

    if (only0x00)
    {
        StringCbPrintfA(uuid, UUID_SIZE, "UUID Not Settable");
        printf("UUID Not Settable");
        return FALSE;
    }

    if (version >= 0x0206)
        StringCbPrintfA(uuid, UUID_SIZE, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
            biosTableData[3], biosTableData[2], biosTableData[1], biosTableData[0], biosTableData[5], biosTableData[4], biosTableData[7], biosTableData[6],
            biosTableData[8], biosTableData[9], biosTableData[10], biosTableData[11], biosTableData[12], biosTableData[13], biosTableData[14], biosTableData[15]);
    else
        StringCbPrintfA(uuid, UUID_SIZE, "-%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
            biosTableData[0], biosTableData[1], biosTableData[2], biosTableData[3], biosTableData[4], biosTableData[5], biosTableData[6], biosTableData[7],
            biosTableData[8], biosTableData[9], biosTableData[10], biosTableData[11], biosTableData[12], biosTableData[13], biosTableData[14], biosTableData[15]);

    return TRUE;
}
/********************************************************************************************************************************/
VOID dmi_string(const dmi_header* header, BYTE index, LPSTR destination, SIZE_T maxLength)
{
    LPSTR data = (LPSTR)(header);

    if (index == 0)
        StringCbPrintfA(destination, maxLength, "Not Specified");

    for (data += header->length; index > 1 && *data; index--)
        data += strlen(data) + 1;

    if (!*data)
        StringCbPrintfA(destination, maxLength, "BAD_INDEX");

    // ASCII filtering
    for (DWORD i = 0; i < strlen(data); i++)
    {
        if (data[i] < 32 || data[i] == 127)
            data[i] = '.';
    }

    StringCbPrintfA(destination, maxLength, "%s", data);
}
/********************************************************************************************************************************/
BOOL GetInformation(LPSTR destination, DWORD maxLength, InformationType informationType)
{
    if (informationType == COMPUTER_NAME)
    {
        if (!GetComputerNameA(destination, &maxLength))
            return FALSE;

        return TRUE;
    }

    SYSTEM_INFO systemInfo = { 0 };
    GetSystemInfo(&systemInfo);
    
    DWORD biosInformationsSize = 0;
    PBYTE biosInformations = NULL;

    if (!(biosInformations = (PBYTE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 65536 * sizeof(BYTE))))
        return FALSE;

    if (!(biosInformationsSize = GetSystemFirmwareTable('RSMB', 0, 0, 0)))
    {
        printf("GetSystemFirmwareTable error\n");
        return FALSE;
    }

    if (!GetSystemFirmwareTable('RSMB', 0, biosInformations, biosInformationsSize))
    {
        printf("GetSystemFirmwareTable error\n");
        return FALSE;
    }

    RawSMBIOSData* Smbios = (RawSMBIOSData*)biosInformations;

    if (Smbios->Length != biosInformationsSize - 8)
    {
        printf("Smbios length error\n");
        return FALSE;
    }

    PBYTE biosTableData = Smbios->SMBIOSTableData;

    for (DWORD i = 0; i < Smbios->Length; i++)
    {
        dmi_header* header = (dmi_header*)biosTableData;

        if (header->type == 1)
        {
            if (informationType == MANUFACTURER)
            {
                dmi_string(header, biosTableData[0x4], destination, maxLength);
                printf("Manufacturer: [%s]\n", destination);
            }
            else if (informationType == PRODUCT_NAME)
            {
                dmi_string(header, biosTableData[0x5], destination, maxLength);
                printf("Product Name: [%s]\n", destination);
            }
            else if (informationType == SERIAL_NUMBER)
            {
                dmi_string(header, biosTableData[0x7], destination, maxLength);
                printf("Serial Number: [%s]\n", destination);
            }
            else if (informationType == FAMILY)
            {
                dmi_string(header, biosTableData[0x1a], destination, maxLength);
                printf("Family: [%s]\n", destination);
            }
            else if (informationType == BIOS_UUID)
            {
                if (!dmi_system_uuid(biosTableData + 0x8, Smbios->SMBIOSMajorVersion * 0x100 + Smbios->SMBIOSMinorVersion, destination))
                {
                    HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, biosInformations);
                    return FALSE;
                }
                printf("UUID: [%s]\n", destination);
            }
        }

        // Go next header
        biosTableData += header->length;

        // Loop through data
        while (*(LPWORD)biosTableData)
            biosTableData++;

        biosTableData += 2;
    }

    HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, biosInformations);
    return TRUE;
}
/********************************************************************************************************************************/
//Fonction qui crée un objet javascript contenant les informations
cJSON* CreateInformationsJsonObject()
{
	//Retrive computer name
	CHAR computerName[INFORMATION_SIZE] = { 0 };
    CHAR manufacturer[INFORMATION_SIZE] = { 0 };
    CHAR productName[INFORMATION_SIZE] = { 0 };
    CHAR serialNumber[INFORMATION_SIZE] = { 0 };
    CHAR family[INFORMATION_SIZE] = { 0 };

    GetInformation(computerName, sizeof(computerName), COMPUTER_NAME);
    GetInformation(manufacturer, sizeof(manufacturer), MANUFACTURER);
    GetInformation(productName, sizeof(productName), PRODUCT_NAME);
    GetInformation(serialNumber, sizeof(serialNumber), SERIAL_NUMBER);
    GetInformation(family, sizeof(family), FAMILY);

	//Create json object to put client informations
	cJSON* object = NULL;

	if (!(object = cJSON_CreateObject()))
		return NULL;

	if (!cJSON_AddStringToObject(object, "computerName", computerName))
		return NULL;
    if (!cJSON_AddStringToObject(object, "manufacturer", manufacturer))
        return NULL;
    if (!cJSON_AddStringToObject(object, "productName", productName))
        return NULL;
    if (!cJSON_AddStringToObject(object, "serialNumber", serialNumber))
        return NULL;
    if (!cJSON_AddStringToObject(object, "family", family))
        return NULL;

	return object;
}
/********************************************************************************************************************************/
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
    GetInformation(uuid, sizeof(uuid), BIOS_UUID);

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
/********************************************************************************************************************************/
EXTERN_C_END
