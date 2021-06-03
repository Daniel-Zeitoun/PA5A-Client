#include "pa5a.hpp"

EXTERN_C_START

DWORD WINAPI ThreadProc(HANDLE wsData)
{

    while (TRUE)
    {
        CHAR buffer[BUFSIZE] = { 0 }; 
     
        if (ReadFromPipe((struct WebSocketData*)wsData, buffer, sizeof(buffer)) == FALSE)
        {
            printf("Failed to read from stdou in the created thread");
        }
        
        send_data(((struct WebSocketData*)wsData)->socket, buffer);
    }

    ExitThread(0);
}

//MODIFIER CETTE FONCTION POUR UNE GESTION D'ERREUR
BOOL CreatePipes(struct WebSocketData* wsData) {

    SECURITY_ATTRIBUTES saAttr;

    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    // Create a pipe for the child process's STDOUT. 
    if (!CreatePipe(&wsData->hChildStd_Output_Rd, &wsData->hChildStd_Output_Wr, &saAttr, 0)) {
        return FALSE;
    }

    // Use to inherit the object handle
    if (!SetHandleInformation(wsData->hChildStd_Output_Rd, HANDLE_FLAG_INHERIT, 0)) {
        return FALSE;
    }

   // Create a pipe for the child process's STDIN. 
    if (!CreatePipe(&wsData->hChildStd_Input_Rd, &wsData->hChildStd_Input_Wr, &saAttr, 0)) {
        return FALSE;
    }
      
    // Use to inherit the object handle
    if (!SetHandleInformation(wsData->hChildStd_Input_Wr, HANDLE_FLAG_INHERIT, 0)) {
        return FALSE;
    }
        

    return TRUE;
}


int CreateChildProcess(LPCWSTR processName, struct WebSocketData* wsData)
// Create a child process that uses the previously created pipes for STDIN and STDOUT.
{
    
    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;
    BOOL bSuccess = FALSE;

    // Set up members of the PROCESS_INFORMATION structure. 

    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    // Set up members of the STARTUPINFO structure. 
    // This structure specifies the STDIN and STDOUT handles for redirection.

    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = wsData->hChildStd_Input_Rd;
    siStartInfo.hStdOutput = wsData->hChildStd_Output_Wr;
    siStartInfo.hStdInput = wsData->hChildStd_Input_Rd;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    // Create the child process. 

    bSuccess = CreateProcess(processName,
        (LPWSTR)processName,     // command line 
        NULL,          // process security attributes 
        NULL,          // primary thread security attributes 
        TRUE,          // handles are inherited 
        0,             // creation flags 
        NULL,          // use parent's environment 
        NULL,          // use parent's current directory 
        &siStartInfo,  // STARTUPINFO pointer 
        &piProcInfo);  // receives PROCESS_INFORMATION 

    if (bSuccess) {
        CloseHandle(piProcInfo.hProcess);
        CloseHandle(piProcInfo.hThread);
    }
    
    wsData->hCmdProcess = piProcInfo.hProcess;
    return bSuccess;
   
}
    

BOOL WriteToPipe(char* command, struct WebSocketData* wsData) {

    DWORD dwWritten;
    BOOL bSuccess = FALSE;
    SetLastError(0);
    if (WriteFile(wsData->hChildStd_Input_Wr, command, strlen(command), &dwWritten, NULL) == 0) {
        printf("Failed");
    }

    bSuccess = GetLastError();

    return TRUE;
}

BOOL ReadFromPipe(struct WebSocketData* wsData, LPSTR buffer, SIZE_T length)
{

    DWORD dwRead;
  
    BOOL bSuccess = FALSE;

    SetLastError(0);

    if (!ReadFile(wsData->hChildStd_Output_Rd, buffer, length, &dwRead, NULL))
    {
        printf("ReadFile fails - GetLastError(%d)\n", GetLastError());
        return -1;
    }

    bSuccess = GetLastError();
    //PrintError("ReadFromPipe", bSuccess);
    return TRUE;
}

void PrintError(char* text, int err) {
    DWORD retSize;
    LPSTR pTemp = NULL;

    if (!err) return;

    retSize = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_ARGUMENT_ARRAY,
        NULL,
        err,
        LANG_NEUTRAL,
        (LPSTR)&pTemp,
        0,
        NULL);


    if (pTemp) printf("%s: %s\n", text, pTemp);
    LocalFree((HLOCAL)pTemp);
    return;

}

EXTERN_C_END