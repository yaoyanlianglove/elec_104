/**
  ******************************************************************************
  * File Name          : tcp.c
  * Description        : This file provides code about tcp.
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */
#include <pthread.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "elec_104.h"
#include "debug.h"
#include "tcp.h"

#define MAX_SIZE 4096

pthread_mutex_t sharedMutexLock;   //发送线程与接收线程共享数据线程锁
STRU_104_TypeDef g_str104;
/*****************************************************************************
 Function    : Elec_104_Task_Timer
 Description : 104线程的子线程
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
void* Elec_104_Task_Timer(void *arg)
{
    int res = 1;
    int timeCount = 0;
    STRU_Queue_TypeDef *p;
    pthread_detach(pthread_self());
    STRU_104_TypeDef *str104 = (STRU_104_TypeDef *) arg;
    DBUG("Elec_104_Task_Timer start\n");
    while(g_str104.breakConnectFlag == 0)
    {
        usleep(1000);
        if(timeCount < 1000)
            timeCount++;
        else
        {
            timeCount = 0;
            if(str104->t3Counter < str104->config.t3)
                str104->t3Counter++;
            else
            {
                str104->t3Counter = 0;
                str104->lastStat104 = str104->stat104;
                str104->stat104 = STAT_104_SEND_TEST;
            }
            if(str104->uT1Flag == 1)
            {
                if(str104->t1Counter < str104->config.t1)
                    str104->t1Counter++;
                else
                {
                    str104->t1Counter = 0;
                    str104->breakConnectFlag = 1;
                    break;
                }
            }
            if(str104->numSDWTC > 0)
            {
                p = str104->pSendHead;
                while((p != NULL))
                {
                    if(p->txFlag == 1)
                    {
                        if(p->t1Counter < str104->config.t1)
                            p->t1Counter++;
                        else
                        {
                            str104->breakConnectFlag = 1;
                            break;
                        }
                    }
                    p = p->next;
                }
            }
        }
        if(str104->stat104 == STAT_104_SEND_START_ACK)
            str104->stat104 = STAT_104_DEVICE_INIT_OK;
    }
    printf("Elec_104_Task_Timer end\n"); 
    pthread_exit(0);
}
/*****************************************************************************
 Function    : Pthread_104
 Description : 104子线程
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
void* Pthread_104(void *arg)
{
    pthread_detach(pthread_self());
    pthread_mutex_init(&sharedMutexLock, NULL);
    STRU_Client_TypeDef *client = (STRU_Client_TypeDef *) arg;
    int err;
    pthread_t pthread104Task;
    err = pthread_create(&pthread104Task, NULL, Elec_104_Task_Timer, (void *)&g_str104);  //创建线程  
    if(err != 0) 
    {
        printf("pthread_create error\n");
        goto Pthread_104_END;
    }
    int socketfd, i, revNum;
    char buff[MAX_SIZE];
    Elec_104_Init(&g_str104);
    socketfd = client->connfd;
    g_str104.socketfd = socketfd;
    DBUG("socketfd %d: Pthread_104 start!\n", socketfd);
    int res = 0;
    while(client->closeFlag == 0 && g_str104.breakConnectFlag == 0)
    {
        revNum = recv(socketfd, buff, MAX_SIZE, MSG_DONTWAIT);
        if(revNum > 0)
        {
            for(i = 0; i < revNum; i++)
            {
                res = Elec_104_Frame_Receive(&g_str104, buff[i]);
                if(res < 0)
                {
                    client->closeFlag = 1;
                }
            }
        }
        else if(revNum < 0)
        {
            if(errno != EWOULDBLOCK && errno != EAGAIN)
            {
                printf("close error with msg is: %s\n", strerror(errno));
                break;
            }     
        }
        else if(revNum == 0)
        {
            DBUG("Master Station close the connect\n"); 
            break;
        } 
    }
    g_str104.stat104 = STAT_104_CONNECT_BREAK;
    //关闭子线程
    pthread_cancel(pthread104Task);
    pthread_join(pthread104Task, NULL);
Pthread_104_END:    
    DBUG("socketfd %d: Pthread_104 end!\n", socketfd);
    close(socketfd);
    memset(client->cli_ip, 0, 16);
    client->connfd    = 0;
    client->stat      = 0;
    client->closeFlag = 0;
    pthread_mutex_destroy(&sharedMutexLock);
    pthread_exit(0);
}
/*****************************************************************************
 Function    : Tcp_Listen
 Description : None
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
int Tcp_Listen(const char *ip, int port)
{
    int err, connfd, i;
    pthread_t pthread104;
    STRU_Client_TypeDef client;
    client.stat = 0;
    int listenfd;
    int opt = SO_REUSEADDR;
    struct sockaddr_in local;
    struct sockaddr_in client_addr;
    socklen_t sock_client_size;
    sock_client_size = sizeof(client_addr);
    char cli_ip[INET_ADDRSTRLEN] = "";    
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
            {
                return -1;
            }  
        }   
        // 打印客户端的 ip 和端口  
        inet_ntop(AF_INET, &client_addr.sin_addr, cli_ip, INET_ADDRSTRLEN);  
        DBUG("----------------------------------------------\n");  
        DBUG("client ip=%s   port=%d connfd=%d\n", cli_ip, ntohs(client_addr.sin_port), connfd);  
        if(connfd > 0)  
        { 
            if(client.stat == 1)
            {
                if(strcmp(cli_ip, client.cli_ip) == 0)
                {
                    client.closeFlag = 1;//104子线还有104send子线
                    while(client.stat == 1)
                        ;
                }
                else
                {
                    close(connfd);
                    continue;
                }
            }
            if(client.stat == 0)
            {
                memcpy(client.cli_ip, cli_ip, 16);
                client.connfd = connfd;
                client.stat   = 1;
                client.closeFlag = 0;
            }
            err = pthread_create(&pthread104, NULL, Pthread_104, (void *)&client);  //创建线程  
            if (err != 0) 
            {
                DBUG("pthread_create error\n");
                close(connfd);
                break;
            }
        }  
    }
    close(listenfd);
    return -1;
}
/************************ZXDQ *****END OF FILE****/
