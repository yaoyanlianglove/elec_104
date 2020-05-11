#include "tcp.h"
#include <pthread.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "elec_104.h"

#define MAX_LINE 4096

pthread_mutex_t client_mutex_lock;

void Sent_Data(char *data, int length)
{

}
void* Pthread_104(void *arg)
{
    pthread_detach(pthread_self());
    void *ret;
    STR_Client_TypeDef *client = (STR_Client_TypeDef *) arg;
    int socketfd, i, revNum;
    char buff[MAX_LINE];
    STR_104_TypeDef str_104;
    Elec_104_Init(&str_104);
    pthread_mutex_lock(&client_mutex_lock);
    socketfd = client->connfd;
    pthread_mutex_unlock(&client_mutex_lock);
    printf("socketfd %d: Pthread_104 start!\n", socketfd);
    while(1)
    {
        printf("socketfd %d: Pthread_104 work!\n", socketfd);
        revNum = recv(socketfd, buff, MAX_LINE, MSG_DONTWAIT);
        if(revNum > 0)
        {
            for(i = 0; i < revNum; i++)
                Elec_104_Process(&str_104, buff[i]);
        }
        else if(revNum < 0)
        {
            if(errno != EINTR && errno != EAGAIN)
            {
                printf("close error with msg is: %s\n", strerror(errno));
                break;
            }     
        }
        else if(revNum == 0)
        {
            printf("Master Station close the connect\n"); 
            break;
        } 
        if(str_104.noConfirmFlag == 1)
        {
            if(str_104.t1Counter < T1)
                str_104.t1Counter++;
            else
                break;
        }
        if(str_104.noSendFlag == 1)
        {
            if(str_104.t2Counter < T2)
                str_104.t2Counter++;
            else
            {
                ;//发送S格式报文
                str_104.t2Counter = 0;
            }
        }
        if(str_104.t3Counter < T3)
            str_104.t3Counter++;
        else
        {
            ;//发送测试报文
            str_104.t3Counter = 0;
        }
        sleep(1);
    }
    printf("socketfd %d: Pthread_104 end!\n", socketfd);
    close(socketfd);
    pthread_mutex_lock(&client_mutex_lock);
    memset(client->cli_ip, 0, 16);
    client->connfd = 0;
    client->stat   = 0;
    pthread_mutex_unlock(&client_mutex_lock);
    pthread_exit(0);
}
int Tcp_Listen(const char *ip, int port)
{
    int err, connfd, i;
    pthread_t pthread104;
    STR_Client_TypeDef client;
    void *ret;
    int listenfd;
    int opt = SO_REUSEADDR;
    struct sockaddr_in local;
    struct sockaddr_in client_addr;
    socklen_t sock_client_size;
    sock_client_size = sizeof(client_addr);
    char cli_ip[INET_ADDRSTRLEN] = ""; 
    err = pthread_mutex_init(&client_mutex_lock, NULL);
    if (err != 0) 
    {
        printf("client_mutex_lock init failed\n");
        return -1;
    }    
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd < 0)
    {
        printf("socket failed\n");
        return -1;
    }
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    local.sin_family = AF_INET;
    local.sin_port   = htons(port);
    local.sin_addr.s_addr = inet_addr(ip);
    if(bind(listenfd, (struct sockaddr *)&local, sizeof(local)) < 0)
    {
        printf("bind failed\n");
        close(listenfd);
        return -1;
    }
    if(listen(listenfd, 5) < 0)
    {
        printf("listen failed\n");
        close(listenfd);
        return -1;;
    }
    printf("Waiting client...\n"); 
    while(1)
    {
        //获得一个已经建立的连接     
        connfd = accept(listenfd, (struct sockaddr*)&client_addr, &sock_client_size);                                
        if(connfd < 0)  
        {  
            printf("accept no\n");  
            continue;  
        }   
        // 打印客户端的 ip 和端口  
        inet_ntop(AF_INET, &client_addr.sin_addr, cli_ip, INET_ADDRSTRLEN);  
        printf("----------------------------------------------\n");  
        printf("client ip=%s   port=%d connfd=%d\n", cli_ip, ntohs(client_addr.sin_port), connfd);  
        if(connfd > 0)  
        { 
            pthread_mutex_lock(&client_mutex_lock);
            if(client.stat == 1)
            {
                if(strcmp(cli_ip, client.cli_ip) == 0)
                {
                    printf("delete cli_ip is %s\n", client.cli_ip);
                    close(client.connfd);
                    pthread_cancel(pthread104);
                    pthread_join(pthread104, &ret);
                    memset(client.cli_ip, 0, 16);
                    client.connfd = 0;
                    client.stat   = 0;
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
            }
            //由于同一个进程内的所有线程共享内存和变量，因此在传递参数时需作特殊处理，值传递。  
            err = pthread_create(&pthread104, NULL, Pthread_104, (void *)&client);  //创建线程  
            if (err != 0) 
            {
                printf("pthread_create error\n");
                close(connfd);
                break;
            }
            pthread_mutex_unlock(&client_mutex_lock);
        }  
    }
    pthread_mutex_destroy(&client_mutex_lock);
    close(listenfd);
    return -1;
}
