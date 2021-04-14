/**
  ******************************************************************************
  * File Name          : elec_104.c
  * Description        : This file provides the code to redefine the elec_104 
                         function.
  ******************************************************************************
  * @attention
  
  *
  ******************************************************************************
  */
#include <pthread.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include "elec_104.h"
#include "debug.h"

pthread_mutex_t sharedMutexLock;
/*****************************************************************************
 Function    : Elec_104_Queue_Init
 Description : None
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
STRU_Queue_TypeDef* Elec_104_Queue_Init(void)
{
    STRU_Queue_TypeDef *p;
    p = (STRU_Queue_TypeDef*)malloc(sizeof(*p));
    if(p == NULL) 
        return NULL;
    else
    {
        p->next = NULL; //设置p指向的区域（头结点）的next域的值
        return p;
    }
}
/*****************************************************************************
 Function    : Elec_104_Queue_Print
 Description : None
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
void Elec_104_Queue_Print(STRU_Queue_TypeDef *queue)
{
    int i = 0;
    STRU_Queue_TypeDef *p = queue;
    DBUG("**************************************************\n");
    if(p == NULL)
    {
        DBUG("Queue is NULL\n ");
        return ;
    }
    do
    {
        if(p->data != NULL)
        {
            DBUG("Queue:\n");
            for(i = 0; i < p->length; i++)
                DBUG("%02x ", p->data[i]);
            DBUG("\n");
            DBUG("p->txFlag:    %d\n", p->txFlag);
            DBUG("p->txNum:     %d\n", p->txNum);
            DBUG("p->length:    %d\n", p->length);
            DBUG("p->priority:  %d\n", p->priority);
            DBUG("\n");
        }
        else
            DBUG("Queue is NULL\n ");   
        p = p->next;
    }while(p);
    DBUG("**************************************************\n");
}
/*****************************************************************************
 Function    : Elec_104_Queue_Insert
 Description : 优先级小的插前面
 Input       : None
 Output      : None
 Return      : Queue的尾部指针
 *****************************************************************************/
STRU_Queue_TypeDef* Elec_104_Queue_Insert(STRU_104_TypeDef *str104, 
                                            uint8_t priority)
{
    STRU_Queue_TypeDef *newQueue, *p, *p1;
    p = str104->pSendHead;
    uint8_t prio = priority;
    /* 缓存达到最大值不再增加 */
    if(str104->numAll >= str104->config.bufferSize)
        return str104->pSendHead;
    newQueue = Elec_104_Queue_Init();
    if(newQueue == NULL) 
        return NULL;
    newQueue->data = (uint8_t*)malloc(256);
    if(newQueue->data == NULL) 
        return NULL;
    memcpy(newQueue->data, str104->tx + 6, str104->txLength - 6);
    newQueue->length    = str104->txLength - 6;
    newQueue->priority  = prio;
    newQueue->t1Counter = 0;
    newQueue->txNum     = 0xFFFFFFFF;
    newQueue->txFlag    = 0;

    
    str104->numAll   = str104->numAll + 1;
    if(p == NULL)
    {
        return newQueue;
    }
    if(prio < p->priority)
    {
        newQueue->next = p;
        str104->pSendHead = newQueue;
    }
    else
    {
        if(p->next == NULL)
        {
            p->next = newQueue;
            newQueue->next = NULL;
        }
        else
        {
            while(p->next != NULL)
            {
                p1 = p;
                p  = p->next;
                if(prio < p->priority)
                {
                    p1->next = newQueue;
                    newQueue->next = p;
                    break;
                }
                /* 找到尾部还未找到小于的优先级，插在尾部 */
                if(p->next == NULL) 
                {
                    p->next = newQueue;
                    newQueue->next = NULL;
                }
            }
        } 
    }
    return str104->pSendHead;
}
/*****************************************************************************
 Function    : Elec_104_Queue_Delete
 Description : 删除发送序号小于对方接收序号的
 Input       : None
 Output      : None
 Return      : -1,故障 0 正常
 *****************************************************************************/
int Elec_104_Queue_Delete(STRU_104_TypeDef *str104)
{
    STRU_Queue_TypeDef *p, *p1;
    p = str104->pSendHead;
    if(p == NULL)
        return -1;
    while(p != NULL)
    {
        if(p->txNum <= str104->otherRxNum)
        {
            if(p == str104->pSendHead)
            {
                str104->pSendHead = p->next;
                if(p->data != NULL)
                {
                    free(p->data);
                    p->data = NULL;
                }
                free(p);
                p = NULL;
                p = str104->pSendHead;
            }
            else
            {
                p1->next = p->next;
                if(p->data != NULL)
                {
                    free(p->data);
                    p->data = NULL;
                }
                free(p);
                p = NULL;
                p = p1->next;
            }
            str104->numAll--;
            str104->numSDWTC--;
        }
        else
        {
            p1 = p;
            p = p->next;
        } 
    }
    return 0;
}

/*****************************************************************************
 Function    : Elec_104_I_Frame_Confirm
 Description : None
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
int Elec_104_I_Frame_Confirm(STRU_104_TypeDef *str104)
{
    uint16_t num = 0;
    uint16_t i = 0;
    int res = 1;
    Elec_104_Queue_Print(str104->pSendHead);
    if((str104->stat104 == STAT_104_SEND_INIT_OK) && (str104->otherRxNum == 1))
        str104->stat104 = STAT_104_SEND_INIT_OK_ACK;
    if(str104->numSDWTC > 0)
    {
        res = Elec_104_Queue_Delete(str104);
    }
    return res;
}
/*****************************************************************************
 Function    : Elec_104_Init
 Description : None
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
void Elec_104_Init(STRU_104_TypeDef *str104)
{
    STRU_Queue_TypeDef *p;
    if(str104->stat104 != STAT_104_POWER_ON)
    {
        /* 设备重新连接，将缓存中的发送状态清零 */
        p = str104->pSendHead;
        while(p != NULL)
        {
            p->txFlag = 0;
            p = p->next;
        }
    }
    else
    {
        str104->pSendHead = NULL;
        str104->numAll  = 0;
    }
    str104->breakConnectFlag = 0;
    str104->uT1Flag = 0;
    str104->timerCloseFlag = 0;
    str104->rxNum = 0;
    str104->txNum = 0;
    /* 待确认数据清零，重新连接后，缓存数据不变 */
    str104->numSDWTC = 0;    
    str104->otherRxNum = 0;
    str104->otherTxNum = 0;
    str104->rxCounter = 0;
    str104->t0Counter = 0;
    str104->t1Counter = 0;
    str104->t2Counter = 0;
    str104->t3Counter = 0;
}

/*****************************************************************************
 Function    : Elec_104_Build_U_Frame
 Description : None
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
void Elec_104_Build_U_Frame(STRU_104_TypeDef *str104, uint8_t type)
{
    str104->tx[ADDR_APDU_HEAD] = 0x68;
    str104->tx[ADDR_APDU_LENGTH] = 0x04;
    str104->tx[ADDR_APCI1] = type;
    str104->tx[ADDR_APCI2] = 0;
    str104->tx[ADDR_APCI3] = 0;
    str104->tx[ADDR_APCI4] = 0;
    str104->txLength = 6;
}
/*****************************************************************************
 Function    : Elec_104_Build_I_Frame
 Description : None
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
void Elec_104_Build_I_Frame(STRU_104_TypeDef *str104)
{
    str104->tx[ADDR_APDU_HEAD] = 0x68;
    str104->tx[ADDR_APCI1] = (str104->txNum) & 0xFE;
    str104->tx[ADDR_APCI2] = ((str104->txNum) >> 8) & 0xFF;
    str104->tx[ADDR_APCI3] = (str104->rxNum) & 0xFE;
    str104->tx[ADDR_APCI4] = ((str104->rxNum) >> 8) & 0xFF;  
    str104->txLength = str104->tx[ADDR_APDU_LENGTH] + 2;
}
/*****************************************************************************
 Function    : Elec_104_Set_S
 Description : None
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
void Elec_104_Build_S_Frame(STRU_104_TypeDef *str104, uint8_t type)
{
    str104->tx[ADDR_APDU_HEAD] = 0x68;
    str104->tx[ADDR_APDU_LENGTH] = 0x04;
    str104->tx[ADDR_APCI1] = 0x01;
    str104->tx[ADDR_APCI2] = 0x00;
    str104->tx[ADDR_APCI3] = (str104->rxNum) & 0xFE;
    str104->tx[ADDR_APCI4] = ((str104->rxNum) >> 8) & 0xFF;
    str104->txLength = str104->tx[ADDR_APDU_LENGTH] + 2;
}
/*****************************************************************************
 Function    : Elec_104_Send_Data
 Description : None
 Input       : None
 Output      : None
 Return      : -1 error,  0 ok
 *****************************************************************************/
int Elec_104_Send_Data(STRU_104_TypeDef *str104)
{
    int res = 0;
    while(1)
    {
        res = send(str104->socketfd, str104->tx, str104->txLength, MSG_DONTWAIT);
        if(res < 0)
        {
            if(errno != EWOULDBLOCK && errno != EAGAIN)
            {
                DBUG("close error with msg is: %s\n", strerror(errno));
                break;
            } 
        }
        else if(res == 0)
        {
            return -1;
        }
        else
        {
            res = 0;
            break;
        }
    }
    if((str104->tx[ADDR_APCI1] & 0x01) == 0)
    {
        str104->txNum++;//发送后再执行
        str104->numSDWTC = str104->numSDWTC + 1;
    }
    Elec_104_Print_Frame(str104, 1);
    memset(str104->tx, 0, 256);
    str104->txLength = 0;    
    return res;
}
/*****************************************************************************
 Function    : Elec_104_Frame_Handle_U
 Description : None
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
void Elec_104_Frame_Handle_U(STRU_104_TypeDef *str104)
{
    switch(str104->rx[ADDR_APCI1])
    {
        case STARTDT_COMMAND:
            str104->stat104 = STAT_104_RECIEVE_START;
        break;
        case STOPDT_COMMAND:
            str104->stat104 = STAT_104_RECIEVE_STOP;
        break;
        case TESTFR_COMMAND:
            str104->lastStat104 = str104->stat104;
            str104->stat104 = STAT_104_RECIEVE_TEST_CMD;
        break;
        case TESTFR_CONFIRM:
            str104->lastStat104 = str104->stat104;
            str104->stat104 = STAT_104_RECIEVE_TEST_CON;
        break;
        default:;
    }
}
/*****************************************************************************
 Function    : Elec_104_ASDU_Insert
 Description : None
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/  
int Elec_104_ASDU_Insert(STRU_104_TypeDef *str104, uint8_t *data, uint8_t length, uint8_t priority)
{
    int res = 0;
    memcpy(str104->tx + 6, data, length);
    ASDU_Frame_Free(data);
    str104->txLength = length + 6;
    str104->pSendHead = Elec_104_Queue_Insert(str104, priority);
    if(str104->pSendHead == NULL)
    {
        res = -1;
    }
    return res;
}
/*****************************************************************************
 Function    : Elec_104_Handle_Callall
 Description : None
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
int Elec_104_Handle_Callall(STRU_104_TypeDef *str104)
{
    uint8_t *data;
    uint16_t length;
    uint8_t priority;
    int res = 0;
    int i = 0;
    int num = 0;
    int r = 0;
    uint16_t sNum = str104->config.signalNum;
    uint16_t dNum = str104->config.detectNum;
    if(str104->otherRxNum == 1)
        priority = CALLALL_INIT_PRIORITY;
    else
        priority = CALLALL_NON_INIT_PRIORITY;
    data = ASDU_Frame_Build_Callall_Control(&length, COT_ACTCON);
    res  = Elec_104_ASDU_Insert(str104, data, length, priority);
    if(res != 0)
        return res;

    /* 遥信 */
    num = sNum/SIGNAL_NUM_PER_PACKAGE;
    r   = sNum%SIGNAL_NUM_PER_PACKAGE;
    for(i = 0; i < num; i++)
    {
        data = ASDU_Frame_Build_Callall_Signal(&length, M_SP_NA_1, 
            str104->signal + SIGNAL_NUM_PER_PACKAGE * i, 
            SIGNAL_START_ADDR + SIGNAL_NUM_PER_PACKAGE * i,
            SIGNAL_NUM_PER_PACKAGE);
        res  = Elec_104_ASDU_Insert(str104, data, length, priority);
        if(res != 0)
            return res;
    }
    data = ASDU_Frame_Build_Callall_Signal(&length, M_SP_NA_1, 
            str104->signal + SIGNAL_NUM_PER_PACKAGE * num, 
            SIGNAL_START_ADDR + SIGNAL_NUM_PER_PACKAGE * num,
            r);
    res  = Elec_104_ASDU_Insert(str104, data, length, priority);
    if(res != 0)
        return res;
        
    /* 遥测 */
    num = dNum/DETECT_NUM_PER_PACKAGE;
    r   = dNum%DETECT_NUM_PER_PACKAGE;
    for(i = 0; i < num; i++)
    {
        data = ASDU_Frame_Build_Callall_Detect(&length, M_ME_NC_1, 
            str104->detect + DETECT_NUM_PER_PACKAGE * i, 
            DETECT_START_ADDR + DETECT_NUM_PER_PACKAGE * i,
            DETECT_NUM_PER_PACKAGE);
        res  = Elec_104_ASDU_Insert(str104, data, length, priority);
        if(res != 0)
            return res;
    }
    data = ASDU_Frame_Build_Callall_Detect(&length, M_ME_NC_1, 
            str104->detect + DETECT_NUM_PER_PACKAGE * num, 
            DETECT_START_ADDR + DETECT_NUM_PER_PACKAGE * num,
            r);
    res  = Elec_104_ASDU_Insert(str104, data, length, priority);
    if(res != 0)
        return res;

    data = ASDU_Frame_Build_Callall_Control(&length, COT_ACTTERM);
    res  = Elec_104_ASDU_Insert(str104, data, length, priority);

    if(res != 0)
        return res;
}
/*****************************************************************************
 Function    : Elec_104_Set_Time
 Description : None
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
void Elec_104_Set_Time(STRU_104_TypeDef *str104)
{
    STRU_cp56time2a_TypeDef cp;
    ASDU_Frame_Get_Cp56time2a(str104->rx + 15, &cp);
    Elec_104_Set_Time_CallBack(&cp);
}
/*****************************************************************************
 Function    : Elec_104_Build_ASDU_Time
 Description : None
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
int Elec_104_Build_ASDU_Time(STRU_104_TypeDef *str104, uint8_t cot)
{
    uint8_t *data;
    uint16_t length;
    uint8_t priority;
    int res = 0;
    STRU_cp56time2a_TypeDef cp;
    Elec_104_Get_Time_CallBack(&cp);
    priority = SNTP_CONFIRM_PRIORITY;
    data = ASDU_Frame_Build_Time(&length, &cp, cot);
    res  = Elec_104_ASDU_Insert(str104, data, length, priority);
    if(res != 0)
        return res;
    return res;
}
/*****************************************************************************
 Function    : Elec_104_Handle_NTP
 Description : None
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
int Elec_104_Handle_NTP(STRU_104_TypeDef *str104, STRU_ASDU_TypeDef *req)
{
    int res = 0;
    if(req->cot == COT_ACT)
    {
        Elec_104_Set_Time(str104);
        res = Elec_104_Build_ASDU_Time(str104, COT_ACTCON);
    }
    else if(req->cot == COT_REQ)
        res = Elec_104_Build_ASDU_Time(str104, COT_REQ);
        
    return res;   
}
/*****************************************************************************
 Function    : Elec_104_Handle_Control
 Description : None
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
int Elec_104_Handle_Control(STRU_104_TypeDef *str104, STRU_ASDU_TypeDef *req)
{
    uint32_t addrOffset = 0;
    int res = 0; 
    // if(req->ti == C_SC_NA_1)  //单点遥控
    // {
    //     if((req->cot == COT_ACT) && (req->se == 1)) //遥控选择
    //     {
    //         addrOffset = req->infoAddr - 
    //     }
    // }
    // else if(req->ti == C_SC_NB_1) //双点遥控
    // {

    // }
    return res;
}
/*****************************************************************************
 Function    : Elec_104_Frame_Handle_I
 Description : None
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
int Elec_104_Frame_Handle_I(STRU_104_TypeDef *str104)
{
    int cmdType = 0;
    int res = 0;
    STRU_ASDU_TypeDef req;
    str104->rxNum = str104->rxNum + 1;
    str104->otherRxNum = (str104->rx[ADDR_APCI3] >> 1) + (str104->rx[ADDR_APCI4] << 8);
    str104->otherTxNum = (str104->rx[ADDR_APCI1] >> 1) + (str104->rx[ADDR_APCI2] << 8);
    res = Elec_104_I_Frame_Confirm(str104); 
    if(res < 0)
        return res;
    cmdType = ASDU_Frame_Handle(str104->rx + 6, &req);
    DBUG("cmdType is : %d\n", cmdType)
    switch(cmdType)
    {
        case CMD_CALLALL:
            res = Elec_104_Handle_Callall(str104);
            if(res < 0)
                return res;
        break;
        case CMD_NTP:
            res = Elec_104_Handle_NTP(str104, &req);
            if(res < 0)
                return res;
        break;
        case CMD_CONTROL:
            res = Elec_104_Handle_Control(str104, &req);
            if(res < 0)
                return res;
        break;
        default:;
    }
    return res;
}
/*****************************************************************************
 Function    : Elec_104_Frame_Handle_S
 Description : None
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
int Elec_104_Frame_Handle_S(STRU_104_TypeDef *str104)
{
    int res = 0;
    str104->otherRxNum = (str104->rx[ADDR_APCI3] >> 1) + (str104->rx[ADDR_APCI4] << 8);
    res = Elec_104_I_Frame_Confirm(str104);
    return res;
}
/*****************************************************************************
 Function    : Elec_104_Frame_Handle
 Description : None
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
int Elec_104_Frame_Handle(STRU_104_TypeDef *str104)
{
    int res = 0;
    if((str104->rx[ADDR_APCI3] & 0x01) == 0)
    {
        if((str104->rx[ADDR_APCI1] & 0x01) == 0)
        {
            if(str104->stat104 >= STAT_104_SEND_INIT_OK)
                res = Elec_104_Frame_Handle_I(str104);
            else
                res = 1;
        }
        else if((str104->rx[ADDR_APCI1] & 0x02) == 0)
        {
            if(str104->stat104 >= STAT_104_SEND_INIT_OK)
                res = Elec_104_Frame_Handle_S(str104);
            else
                res = 1;
        }
        else
            Elec_104_Frame_Handle_U(str104);
    }
    res = Elec_104_Stat_Handle(str104);
    return res;       
}
/*****************************************************************************
 Function    : Elec_104_Print_Frame
 Description : None
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
void Elec_104_Print_Frame(STRU_104_TypeDef *str104, uint8_t type)
{
    int i = 0;
    DBUG("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    if(type == 0)
    {
        DBUG("Receive Frame: \n");
        DBUG("    ");
        for(i = 0; i < str104->rxLength; i++)
            DBUG("%02x ", str104->rx[i]);
        DBUG("\n");

    }
    else if(type == 1)
    {
        DBUG("Send Frame: \n");
        DBUG("    ");
        for(i = 0; i < str104->txLength; i++)
            DBUG("%02x ", str104->tx[i]);
        DBUG("\n");

    }
    DBUG("    rxLength:   %02x \n", str104->rxLength);
    DBUG("    rxNum:      %02x \n", str104->rxNum);
    DBUG("    txNum:      %02x \n", str104->txNum);
    DBUG("    otherRxNum: %02x \n", str104->otherRxNum);
    DBUG("    otherTxNum: %02x \n", str104->otherTxNum);
    DBUG("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    DBUG("\n");
}
/*****************************************************************************
 Function    : Elec_104_Frame_Receive
 Description : None
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
int Elec_104_Frame_Receive(STRU_104_TypeDef *str104, uint8_t data)
{
    int res = 0;
    switch(str104->rxCounter)
    {
        case 0:
            if(data == 0x68)
            {
                str104->rx[str104->rxCounter] = data;
                str104->rxCounter++;
            }
        break;
        case 1:
            str104->rx[str104->rxCounter] = data;
            str104->rxLength = data + 2;
            str104->rxCounter++;
        break;
        default:
            if(str104->rxCounter < str104->rxLength)
            {
                str104->rx[str104->rxCounter] = data;
                str104->rxCounter++;
            }
            else
            {
                str104->rxCounter = 0;
                Elec_104_Print_Frame(str104, 0);
                str104->t3Counter = 0; //收到任意帧，t3重新计数
                res = Elec_104_Frame_Handle(str104);
                memset(str104->rx, 0, 256);
                str104->rxLength = 0;
            }
        break;
    }
    return res;
}
/*****************************************************************************
 Function    : Elec_104_Send_Init_OK
 Description : 发送设备初始化完成
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
int Elec_104_Send_Init_OK(STRU_104_TypeDef *str104)
{
    uint16_t length = 0;
    uint8_t *data;
    int res = 0;
    data = ASDU_Frame_Build_Init_OK(&length, COI_POWER_ON);
    res = Elec_104_ASDU_Insert(str104, data, length, SYSTEM_PRIORITY);
    if(res != 0)
        return res;
    res = Elec_104_Send_Data(str104);
    if(res == 0)
    {
        str104->pSendHead->txFlag = 1;
        str104->pSendHead->txNum  = str104->txNum;
    }
    return res;
}
/*****************************************************************************
 Function    : Elec_104_Stat_Handle
 Description : 104状态处理
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
int Elec_104_Stat_Handle(STRU_104_TypeDef *str104)
{
    int res = 0;
    int num = 0;
    STRU_Queue_TypeDef *p;
    DBUG("Elec_104_Stat: %d\n", str104->stat104);
    switch(str104->stat104)
    {
        case STAT_104_POWER_ON:
            ;
        break;
        case STAT_104_RECIEVE_START:
            Elec_104_Build_U_Frame(str104, STARTDT_CONFIRM);
            res = Elec_104_Send_Data(str104);
            str104->stat104 = STAT_104_SEND_START_ACK;
        break;
        case STAT_104_DEVICE_INIT_OK:
            res = Elec_104_Send_Init_OK(str104);
            if(res == 0)
                str104->stat104 = STAT_104_SEND_INIT_OK;
        break;
        case STAT_104_SEND_INIT_OK_ACK:
            str104->stat104 = STAT_104_CONNECT_OK;
        break;
        case STAT_104_CONNECT_OK:
            //超过k值的数据未确认，停止发送
            if(str104->numSDWTC >= str104->config.k)
                break;
            //待发送数据等于所有缓存数据减去已发送数据
            num = str104->numAll - str104->numSDWTC;
            if(num > 0)
            {
                p = str104->pSendHead;
                while(p != NULL)
                {
                    if(p->txFlag == 0)
                    {
                        break;
                    }
                    else
                    {
                        p = p->next;
                    }
                }
                memcpy(str104->tx + 6, p->data, p->length);
                str104->tx[ADDR_APDU_LENGTH] = p->length + 4;
                Elec_104_Build_I_Frame(str104);
                res = Elec_104_Send_Data(str104);
                if(res == 0) //发送成功，缓存置标志发送成功
                {
                    p->txNum = str104->txNum;
                    p->txFlag = 1;
                }
            }
        break;
        case STAT_104_RECIEVE_STOP:
            if(str104->numSDWTC == 0)
            {
                Elec_104_Build_U_Frame(str104, STOPDT_CONFIRM);
                res = Elec_104_Send_Data(str104);
                str104->stat104 = STAT_104_SEND_STOP_ACK;
            }
        break;
        case STAT_104_SEND_STOP_ACK:
            str104->stat104 = STAT_104_CONNECT_BREAK;
            str104->breakConnectFlag = 1;
        break;
        case STAT_104_RECIEVE_TEST_CMD:
            Elec_104_Build_U_Frame(str104, STARTDT_CONFIRM);
            res = Elec_104_Send_Data(str104);
            str104->stat104 = str104->lastStat104;
        break;
        case STAT_104_RECIEVE_TEST_CON:
            str104->stat104 = str104->lastStat104;
            str104->uT1Flag = 0;
            str104->t1Counter = 0;
        break;
        case STAT_104_SEND_TEST:
            Elec_104_Build_U_Frame(str104, TESTFR_COMMAND);
            res = Elec_104_Send_Data(str104);
            str104->uT1Flag = 1;
            str104->stat104 = str104->lastStat104;
        break;
        default:
        ;
    }
    return res;
}

/*****************************************************************************
 Function    : Elec_104_Thread_Task_Timer
 Description : 定时任务子线程
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
void* Elec_104_Thread_Task_Timer(void *arg)
{
    pthread_detach(pthread_self());
    int res = 1;
    int timeCount = 0;
    STRU_Queue_TypeDef *p;
    STRU_104_TypeDef *str104 = (STRU_104_TypeDef *) arg;
    DBUG("Elec_104_Thread_Task_Timer start\n");
    while(str104->breakConnectFlag == 0)
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
    str104->timerCloseFlag = 1;
    DBUG("Elec_104_Thread_Task_Timer end\n"); 
}
/************************ZXDQ *****END OF FILE****/

