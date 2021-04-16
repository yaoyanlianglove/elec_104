/**
  ******************************************************************************
  * File Name          : main.h
  * Description        : This file provides the example code.
  ******************************************************************************
  * @attention
  
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __example_H
#define __example_H

#include <stdint.h>
#include <pthread.h>
#include "elec_104.h"

#define INET_ADDRSTRLEN 16
#define MAX_SIZE 4096

typedef struct STRU_Client_TypeDef_t
{
    pthread_t pthreadIdTimerTask;
    pthread_t pthreadId104Main;
    pthread_t pthreadId104Recv;
    STRU_104_TypeDef *str104;
    char clientIp[INET_ADDRSTRLEN];
    int  connfd;
    uint8_t stat;
}STRU_Client_TypeDef;

int Elec_104_Tcp_Listen(STRU_Client_TypeDef *client, const char *ip, int port);
void Elec_104_Data_Map(STRU_104_TypeDef* str104);
#endif
/************************ZXDQ *****END OF FILE****/
  

