#ifndef _TCP_H_ 
#define _TCP_H_
#include <sys/socket.h>
#include <stdint.h>
#define INET_ADDRSTRLEN 16


typedef struct 
{
    char cli_ip[INET_ADDRSTRLEN];
    int  connfd;
    uint8_t stat;
}STR_Client_TypeDef;

int Tcp_Listen(const char *ip, int port);


#endif


