/**
  ******************************************************************************
  * File Name          : tcp.h
  * Description        : This file provides the code to redefine the tcp 
                         function.
  ******************************************************************************
  * @attention
  
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
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
    uint8_t closeFlag;                  //关闭socket标志
}STRU_Client_TypeDef;

int Tcp_Listen(const char *ip, int port);


#endif
/************************ZXDQ *****END OF FILE****/
  


