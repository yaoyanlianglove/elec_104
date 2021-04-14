#include <stdio.h>
#include <stdlib.h>
#include "example.h"

int main()
{
    STRU_Client_TypeDef client;
    client.str104 = (STRU_104_TypeDef *)malloc(sizeof(struct STRU_104_TypeDef_t));
    Example_Data_Map(client.str104);
    Tcp_Listen(&client, "192.168.1.144", 2404);
    if(client.str104 != NULL)
    {
        free(client.str104);
        client.str104 = NULL;
    }
    return 0;    
}

