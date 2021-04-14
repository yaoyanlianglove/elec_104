/**
  ******************************************************************************
  * File Name          : example.c
  * Description        : This file provides the example code.
  ******************************************************************************
  * @attention
  
  *
  ******************************************************************************
  */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "example.h"
#include "asdu.h"
#include "elec_104.h"
#include "debug.h"
 
STRU_SIGNAL_TypeDef  signal[SIGNAL_MAX_NUM];
STRU_DETECT_TypeDef  detect[DETECT_MAX_NUM];

void Example_Data_Map(STRU_104_TypeDef* str104)
{
    int i = 0;
    str104->config.t0 = 30;
    str104->config.t1 = 15;
    str104->config.t2 = 10;
    str104->config.t3 = 20;
    str104->config.k  = 20;   
    str104->config.bufferSize = 30; 
    str104->config.signalNum  = 32; 
    str104->config.detectNum  = 32; 
    str104->config.controlNum = 16;
    str104->config.qoeNum     = 10;    
    str104->config.paraNum = 10;
    
    for(i = 0; i < str104->config.signalNum; i++)
    {
        signal[i].stat = 1;
        str104->signal[i] = &(signal[i]);
    }
    for(i = 0; i < str104->config.detectNum; i++)
    {
        detect[i].value = 20.1 + i;
        str104->detect[i] = &(detect[i]);
    }  
}
void Elec_104_Set_Time_CallBack(STRU_cp56time2a_TypeDef *cp)
{
    printf("time is %d-%d-%d %d:%d:%d %d\n", cp->year, cp->month, cp->wday, cp->mday, cp->hour, cp->min, cp->msec);
}

void Elec_104_Get_Time_CallBack(STRU_cp56time2a_TypeDef *cp)
{
    cp->year  = 21;
    cp->month = 3;
    cp->wday  = 3;
    cp->mday  = 31;
    cp->hour  = 12;
    cp->min   = 13;
    cp->msec  = 7777;
}
/*****************************************************************************
 Function    : Elec_104_Thread_104_Main
 Description : 104子线程
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
void* Elec_104_Thread_104_Main(void *arg)
{
    pthread_detach(pthread_self());
    STRU_Client_TypeDef *client = (STRU_Client_TypeDef *)arg;
    int socketfd, i, revNum;
    char buff[MAX_SIZE];
    socketfd = client->connfd;
    DBUG("socketfd %d: Pthread_104 start!\n", socketfd);
    int res = 0;
    while(client->str104->breakConnectFlag == 0)
    {
        revNum = recv(socketfd, buff, MAX_SIZE, MSG_DONTWAIT);
        if(revNum > 0)
        {
            for(i = 0; i < revNum; i++)
            {
                res = Elec_104_Frame_Receive(client->str104, buff[i]);
                if(res < 0)
                {
                    client->str104->breakConnectFlag = 1;
                }
            }
        }
        else if(revNum < 0)
        {
            if(errno != EWOULDBLOCK && errno != EAGAIN)
            {
                DBUG("close error with msg is: %s\n", strerror(errno));
                break;
            }     
        }
        else if(revNum == 0)
        {
            DBUG("Master Station close the connect\n"); 
            break;
        } 
    }
    client->str104->breakConnectFlag = 1;
    client->str104->stat104 = STAT_104_CONNECT_BREAK; 
    DBUG("socketfd %d: Pthread_104 end!\n", socketfd);
    close(socketfd);
    memset(client->clientIp, 0, 16);
    client->connfd    = 0;
    client->stat      = 0;
}
/*****************************************************************************
 Function    : Example_Destroy_Thread
 Description : None
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
int Example_Destroy_Thread(STRU_Client_TypeDef *client)
{
    client->str104->breakConnectFlag = 1;
    /* 等待主线程结束 */
    while(client->stat == 1)
        ;
    /* 等待定时器线程结束 */
    while(client->str104->timerCloseFlag == 0)
        ;
    close(client->connfd);
    return 0;
}
/*****************************************************************************
 Function    : Example_Create_Thread
 Description : None
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
int Example_Create_Thread(STRU_Client_TypeDef *client)
{
    int res = 0;
    client->str104->socketfd = client->connfd;
    Elec_104_Init(client->str104);
    res = pthread_create(&(client->pthreadId104Main), NULL, Elec_104_Thread_104_Main, (void *)client);    
    if(res != 0) 
    {
        DBUG("pthreadId104Main create error\n");
        return -1;
    }
    res = pthread_create(&(client->pthreadIdTimerTask), NULL, Elec_104_Thread_Task_Timer, (void *)client->str104);   
    if(res != 0) 
    {
        DBUG("pthreadIdTimerTask create error\n");
        return -1;
    }
    return 0;
}
/*****************************************************************************
 Function    : Tcp_Listen
 Description : None
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
int Tcp_Listen(STRU_Client_TypeDef *client, const char *ip, int port)
{
    int res, connfd, listenfd;
    int opt = SO_REUSEADDR;
    struct sockaddr_in local, client_addr;
    socklen_t sock_client_size;
    sock_client_size = sizeof(client_addr);
    char clientIp[INET_ADDRSTRLEN] = "";   
    res = 0;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd < 0)
    {
        DBUG("socket failed\n");
        return -1;
    }
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    local.sin_family = AF_INET;
    local.sin_port   = htons(port);
    local.sin_addr.s_addr = inet_addr(ip);
    if(bind(listenfd, (struct sockaddr *)&local, sizeof(local)) < 0)
    {
        DBUG("bind failed\n");
        close(listenfd);
        return -1;
    }
    if(listen(listenfd, 5) < 0)
    {
        DBUG("listen failed\n");
        close(listenfd);
        return -1;
    }
    printf("Waiting client...\n"); 
    while(1)
    {
        //获得一个已经建立的连接     
        connfd = accept(listenfd, (struct sockaddr*)&client_addr, &sock_client_size);                                
        if(connfd < 0)  
        {  
            if (errno == EINTR) 
            {   
                DBUG("accept no\n");  
                continue;
            }
            else
                break;
        }   
        // 打印客户端的 ip 和端口  
        inet_ntop(AF_INET, &client_addr.sin_addr, clientIp, INET_ADDRSTRLEN);  
        DBUG("----------------------------------------------\n");  
        DBUG("Client:  ip=%s   port=%d connfd=%d\n", clientIp, ntohs(client_addr.sin_port), connfd);  
        if(connfd >= 0)  
        { 
            if(client->stat == 1)
            {
                /* 如果请求的ip已经连接了，断开链接，重新连接 */
                if(strcmp(clientIp, client->clientIp) == 0)
                {
                    Example_Destroy_Thread(client);
                    client->connfd = connfd;
                    client->stat   = 1;
                    res = Example_Create_Thread(client);
                    if(res != 0)
                    {
                        close(connfd);
                        break;
                    }       
                }
                /* 当前已有连接，并且新连接IP与已有连接IP不同 */
                else
                {
                    close(connfd);
                    DBUG("refuse\n"); 
                    DBUG("----------------------------------------------\n"); 
                    continue;
                }
            }
            /* 如果是新的请求，并且当前连接未被占用 */
            else if(client->stat == 0)
            {
                memcpy(client->clientIp, clientIp, 16);
                client->connfd = connfd;
                client->stat   = 1;
                res = Example_Create_Thread(client);
                if(res != 0)
                {
                    close(connfd);
                    break;
                }     
            }
            
        }  
    }
    close(listenfd);
    return -1;
}
/************************ZXDQ *****END OF FILE****/
  

