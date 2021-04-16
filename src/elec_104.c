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
        p->next = NULL; /* 设置p指向的区域（头结点）的next域的值 */
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
    printf("**************************************************\n");
    if(p == NULL)
    {
        printf("Queue is NULL\n ");
        return ;
    }
    do
    {
        if(p->data != NULL)
        {
            printf("Queue:\n");
            for(i = 0; i < p->length; i++)
                printf("%02x ", p->data[i]);
            printf("\n");
            printf("p->txFlag:    %d\n", p->txFlag);
            printf("p->txNum:     %d\n", p->txNum);
            printf("p->length:    %d\n", p->length);
            printf("p->priority:  %d\n", p->priority);
            printf("\n");
        }
        else
            printf("Queue is NULL\n ");   
        p = p->next;
    }while(p);
    printf("**************************************************\n");
}
/*****************************************************************************
 Function    : Elec_104_Queue_Insert
 Description : 优先级小的插前面
 Input       : None
 Output      : None
 Return      : Queue的尾部指针
 *****************************************************************************/
int Elec_104_Queue_Insert(STRU_104_TypeDef *str104)
{
    STRU_Queue_TypeDef *newQueue, *p, *p1;
    p = str104->pSendHead;
    uint8_t prio = str104->rep->priority;
    /* 缓存达到最大值不再增加 */
    if(str104->numAll >= str104->config->bufferSize)
        return 0;
    newQueue = Elec_104_Queue_Init();
    if(newQueue == NULL)
        return -1;  
    newQueue->data = (uint8_t*)malloc(256);
    if(newQueue->data == NULL) 
        return -1;
    memcpy(newQueue->data, str104->rep->data, str104->rep->length);
    newQueue->length    = str104->rep->length;
    newQueue->priority  = prio;
    newQueue->t1Counter = 0;
    newQueue->txNum     = 0xFFFFFFFF;
    newQueue->txFlag    = 0;

    str104->numAll   = str104->numAll + 1;
    if(p == NULL)
    {
        str104->pSendHead = newQueue;
        return 0;
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
    return 0;
}
/*****************************************************************************
 Function    : Elec_104_Queue_Delete
 Description : 删除发送序号小于对方接收序号的
 Input       : None
 Output      : None
 Return      : 0
 *****************************************************************************/
int Elec_104_Queue_Delete(STRU_104_TypeDef *str104)
{
    STRU_Queue_TypeDef *p, *p1;
    p = str104->pSendHead;
    if(p == NULL)
        return 0;
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
    int res = 0;
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
        str104->initQPR   = QPR_POWER_ON; /* 断电复位 */
        str104->pSendHead = NULL;
        str104->numAll    = 0;
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
    str104->rxOverFlag = 0;
    str104->t0Counter = 0;
    str104->t1Counter = 0;
    str104->t2Counter = 0;
    str104->t3Counter = 0;

    str104->cmdStat->callall = 0;
    str104->cmdStat->control = 0;
    str104->rep->asduAddr = str104->config->deviceAddr;
    str104->rep->headerLength = str104->config->asduHeaderLength;
    str104->req->asduAddr = str104->config->deviceAddr;
    str104->req->headerLength = str104->config->asduHeaderLength;
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
    str104->tx[ADDR_APCI1] = ((str104->txNum) << 1) & 0xFF;
    str104->tx[ADDR_APCI2] = ((str104->txNum) >> 7) & 0xFF;
    str104->tx[ADDR_APCI3] = ((str104->rxNum) << 1) & 0xFF;
    str104->tx[ADDR_APCI4] = ((str104->rxNum) >> 7) & 0xFF; 
    str104->tx[ADDR_APDU_LENGTH] = str104->txLength - 2; 
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
    str104->tx[ADDR_APCI3] = ((str104->rxNum) << 1) & 0xFF;
    str104->tx[ADDR_APCI4] = ((str104->rxNum) >> 7) & 0xFF;
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
    uint16_t timeoutCount = 0;
    uint16_t length = 0;
    length = str104->txLength;
    while(1)
    {
        res = send(str104->socketfd, str104->tx, length, MSG_DONTWAIT);
        if(res < 0)
        {
            if(errno != EWOULDBLOCK && errno != EAGAIN)
            {
                DBUG("close error with msg is: %s\n", strerror(errno));
                break;
            } 
        }
        else if(res == 0)
            return -1;
        else 
        {
            if(res < length)
            {
                length = length -res;
                continue;
            }
            else
            {
                res = 0;
                break;
            }
        }
        if(timeoutCount < 5000)
            timeoutCount++;
        else
            return -1;  /* 超时退出 */
    }
    if((str104->tx[ADDR_APCI1] & 0x01) == 0)
    {
        str104->txNum++;/* 发送后再执行 */
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
 Function    : Elec_104_Handle_Callall
 Description : None
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
int Elec_104_Handle_Callall(STRU_104_TypeDef *str104)
{
    int res = 0;
    int i = 0;
    int num = 0;
    int r = 0;
    uint16_t sNum = str104->config->signalNum;
    uint16_t dNum = str104->config->detectNum;
    /* 没有总召唤命令的情况下才能接收总召唤命令*/
    if(str104->cmdStat->callall == 0)
        str104->cmdStat->callall = COT_ACT;
    else
    {
        res = COT_NO_COS;
        return res;
    }
    if(str104->req->asduAddr != str104->config->deviceAddr)
    {
        res = COT_ERR_DEV_ADDR;
        return res;
    }
    if(str104->req->infoAddr != 0)
    {
        res = COT_ERR_OBJ_ADDR;
        return res;
    }
    if(str104->otherRxNum == 1)
        str104->rep->priority = CALLALL_INIT_PRIORITY;
    else
        str104->rep->priority = CALLALL_NON_INIT_PRIORITY;

    ASDU_Frame_Build_Callall_Control(str104->rep, COT_ACTCON);
    res  = Elec_104_Queue_Insert(str104);
    if(res != 0)
        return res;

    /* 遥信 */
    num = sNum/SIGNAL_NUM_PER_PACKAGE;
    r   = sNum%SIGNAL_NUM_PER_PACKAGE;
    for(i = 0; i < num; i++)
    {
        ASDU_Frame_Build_Callall_Signal(str104->rep, M_SP_NA_1, 
            str104->signal + SIGNAL_NUM_PER_PACKAGE * i, 
            SIGNAL_START_ADDR + SIGNAL_NUM_PER_PACKAGE * i,
            SIGNAL_NUM_PER_PACKAGE);
        res  = Elec_104_Queue_Insert(str104);
        if(res != 0)
            return res;
    }
    ASDU_Frame_Build_Callall_Signal(str104->rep, M_SP_NA_1, 
            str104->signal + SIGNAL_NUM_PER_PACKAGE * num, 
            SIGNAL_START_ADDR + SIGNAL_NUM_PER_PACKAGE * num,
            r);
    res  = Elec_104_Queue_Insert(str104);
    if(res != 0)
        return res;
        
    /* 遥测 */
    num = dNum/DETECT_NUM_PER_PACKAGE;
    r   = dNum%DETECT_NUM_PER_PACKAGE;
    for(i = 0; i < num; i++)
    {
        ASDU_Frame_Build_Callall_Detect(str104->rep, M_ME_NC_1, 
            str104->detect + DETECT_NUM_PER_PACKAGE * i, 
            DETECT_START_ADDR + DETECT_NUM_PER_PACKAGE * i,
            DETECT_NUM_PER_PACKAGE);
        res  = Elec_104_Queue_Insert(str104);
        if(res != 0)
            return res;
    }
    ASDU_Frame_Build_Callall_Detect(str104->rep, M_ME_NC_1, 
            str104->detect + DETECT_NUM_PER_PACKAGE * num, 
            DETECT_START_ADDR + DETECT_NUM_PER_PACKAGE * num,
            r);
    res  = Elec_104_Queue_Insert(str104);
    if(res != 0)
        return res;

    ASDU_Frame_Build_Callall_Control(str104->rep, COT_ACTTERM);
    res  = Elec_104_Queue_Insert(str104);

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
    str104->rep->priority = SNTP_CONFIRM_PRIORITY;
    ASDU_Frame_Build_Time(str104->rep, &cp, cot);
    res  = Elec_104_Queue_Insert(str104);
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
int Elec_104_Handle_NTP(STRU_104_TypeDef *str104)
{
    int res = 0;
    if(str104->req->cot == COT_ACT)
    {
        Elec_104_Set_Time(str104);
        res = Elec_104_Build_ASDU_Time(str104, COT_ACTCON);
    }
    else if(str104->req->cot == COT_REQ)
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
int Elec_104_Handle_Control(STRU_104_TypeDef *str104)
{
    uint32_t addrOffset = 0;
    int res = 0;
    int repCot = 0; 
    addrOffset = str104->req->infoAddr - CONTROL_START_ADDR;

    if(str104->req->ti == C_SC_NA_1)  /* 单点遥控 */
    {
        if((str104->req->cot == COT_ACT) && (str104->req->se == 1)) /* 遥控选择 */
        {
            if(str104->control[addrOffset]->stat == CONTROL_STAT_ORIGIN)
            {
                str104->control[addrOffset]->stat = CONTROL_STAT_SELECTED;
                str104->control[addrOffset]->timeCounter = str104->config->controlTimeout;
            }
            else
            {
                str104->control[addrOffset]->stat = CONTROL_STAT_ORIGIN;
                str104->control[addrOffset]->timeCounter = 0;
               // res = ERR;
            } 
        }
        else if((str104->req->cot == COT_ACT) && (str104->req->se == 0)) /* 遥控确认 */
        {
            if(str104->control[addrOffset]->stat == CONTROL_STAT_SELECTED)
            {
                str104->control[addrOffset]->stat = CONTROL_STAT_CONFIRM;
                /*执行遥控*/
            }
            else
            {
                //res = ERR;
                str104->control[addrOffset]->stat = CONTROL_STAT_ORIGIN;
            }
            str104->control[addrOffset]->timeCounter = 0;   
        }
        else if((str104->req->cot == COT_DEACT) && (str104->req->se == 0)) /* 遥控取消 */
        {
            if(str104->control[addrOffset]->stat == CONTROL_STAT_SELECTED)
                str104->control[addrOffset]->stat = CONTROL_STAT_ORIGIN;
            else
            {
               // res = ERR;
                str104->control[addrOffset]->stat = CONTROL_STAT_ORIGIN;
            }
            str104->control[addrOffset]->timeCounter = 0;
        }
    }
    else if(str104->req->ti == C_SC_NB_1) /* 双点遥控 */
    {

    }
    return res;
}
/*****************************************************************************
 Function    : Elec_104_Build_I_Frame_Error
 Description : None
 Input       : None
 Output      : None
 Return      : res < 0 退出程序， 0 正常 
 *****************************************************************************/
int Elec_104_Build_I_Frame_Error(STRU_104_TypeDef *str104)
{
    int res = 0;
    ASDU_Frame_Build_Error(str104->rep, str104->req);
    res  = Elec_104_Queue_Insert(str104);
    return res;
}
/*****************************************************************************
 Function    : Elec_104_Frame_Handle_I
 Description : None
 Input       : None
 Output      : None
 Return      : res < 0 退出程序， 0 正常，>0 错误COT
 *****************************************************************************/
int Elec_104_Frame_Handle_I(STRU_104_TypeDef *str104)
{
    int cmdType = 0;
    int res = 0;
    str104->rxNum = str104->rxNum + 1;
    str104->otherRxNum = (str104->rx[ADDR_APCI3] >> 1) + (str104->rx[ADDR_APCI4] << 8);
    str104->otherTxNum = (str104->rx[ADDR_APCI1] >> 1) + (str104->rx[ADDR_APCI2] << 8);
    res = Elec_104_I_Frame_Confirm(str104); 
    if(res < 0)
        return res;
    str104->req->length = str104->rxLength - 6;
    memcpy(str104->req->data, str104->rx + 6, str104->rxLength);
    cmdType = ASDU_Frame_Handle(str104->req);
    DBUG("cmdType is : %d\n", cmdType);
    switch(cmdType)
    {
        case CMD_ERROR:
            res = COT_NO_TI;
            str104->rep->priority = SYSTEM_PRIORITY;
        break;
        case CMD_CALLALL:
            res = Elec_104_Handle_Callall(str104);
        break;
        case CMD_NTP:
            res = Elec_104_Handle_NTP(str104);
        break;
        case CMD_CONTROL:
            res = Elec_104_Handle_Control(str104);
        break;
        default:;
    }
    if(res > 0)
        res = Elec_104_Build_I_Frame_Error(str104);
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
        }
        else if((str104->rx[ADDR_APCI1] & 0x02) == 0)
        {
            if(str104->stat104 >= STAT_104_SEND_INIT_OK)
                res = Elec_104_Frame_Handle_S(str104);
        }
        else
            Elec_104_Frame_Handle_U(str104);
    }
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
    printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    if(type == 0)
    {
        printf("Receive Frame: \n");
        printf("    ");
        for(i = 0; i < str104->rxLength; i++)
            printf("%02x ", str104->rx[i]);
        printf("\n");
        printf("    rxLength:   %02x \n", str104->rxLength);
    }
    else if(type == 1)
    {
        printf("Send Frame: \n");
        printf("    ");
        for(i = 0; i < str104->txLength; i++)
            printf("%02x ", str104->tx[i]);
        printf("\n");
        printf("    txLength:   %02x \n", str104->txLength);
    }
    printf("    numAll:     %02x \n", str104->numAll);
    printf("    numSDWTC:   %02x \n", str104->numSDWTC);
    printf("    rxNum:      %02x \n", str104->rxNum);
    printf("    txNum:      %02x \n", str104->txNum);
    printf("    otherRxNum: %02x \n", str104->otherRxNum);
    printf("    otherTxNum: %02x \n", str104->otherTxNum);
    printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    printf("\n");
}
/*****************************************************************************
 Function    : Elec_104_Frame_Receive
 Description : None
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
void Elec_104_Frame_Receive(STRU_104_TypeDef *str104, uint8_t data)
{
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
            str104->rx[str104->rxCounter] = data;
            if(str104->rxCounter < str104->rxLength - 1)
            {
                str104->rxCounter++;
            }
            else
            {
                str104->rxCounter = 0;
                Elec_104_Print_Frame(str104, 0);
                str104->t3Counter = 0; /* 收到任意帧，t3重新计数 */
                str104->rxOverFlag = 1;
            }
        break;
    }
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
    int res = 0;
    ASDU_Frame_Build_Init_OK(str104->rep, str104->initQPR);
    res = Elec_104_Queue_Insert(str104);
    if(res != 0)
        return res;
    str104->txLength = str104->rep->length + 6;
    memcpy(str104->tx + 6, str104->rep->data, str104->rep->length);
    Elec_104_Build_I_Frame(str104);
    res = Elec_104_Send_Data(str104);
    DBUG("Elec_104_Send_Init_OK res is %d\n", res);
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
    //DBUG("Elec_104_Stat: %d\n", str104->stat104);
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
            /* 超过k值的数据未确认，停止发送 */
            if(str104->numSDWTC >= str104->config->k)
                break;
            /* 待发送数据等于所有缓存数据减去已发送数据 */
            num = str104->numAll - str104->numSDWTC;
            if(num > 0)
            {
                p = str104->pSendHead;
                while(p != NULL)
                {
                    if(p->txFlag == 0)
                        break;
                    else
                        p = p->next;
                }
                memcpy(str104->tx + 6, p->data, p->length);
                str104->txLength = p->length + 6;
                Elec_104_Build_I_Frame(str104);
                res = Elec_104_Send_Data(str104);
                if(res == 0) /* 发送成功，缓存置标志发送成功 */
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
            Elec_104_Build_U_Frame(str104, TESTFR_CONFIRM);
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
            if(str104->t3Counter < str104->config->t3)
                str104->t3Counter++;
            else
            {
                str104->t3Counter = 0;
                str104->lastStat104 = str104->stat104;
                str104->stat104 = STAT_104_SEND_TEST;
            }
            if(str104->uT1Flag == 1)
            {
                if(str104->t1Counter < str104->config->t1)
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
                        if(p->t1Counter < str104->config->t1)
                            p->t1Counter++;
                        else
                        {
                            p->t1Counter = 0;
                            str104->breakConnectFlag = 1;
                            break;
                        }
                    }
                    p = p->next;
                }
            }
        }
        if(str104->stat104 == STAT_104_SEND_START_ACK)
        {
            str104->stat104 = STAT_104_DEVICE_INIT_OK;
        }      
    }
    str104->timerCloseFlag = 1;
    DBUG("Elec_104_Thread_Task_Timer end\n"); 
}
/*****************************************************************************
 Function    : Elec_104_Malloc
 Description : 内存分配
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
STRU_104_TypeDef* Elec_104_Malloc(void)
{
    STRU_104_TypeDef *str104;
    str104 = (STRU_104_TypeDef *)malloc(sizeof(struct STRU_104_TypeDef_t));
    if(str104 == NULL)
        return NULL;

    str104->rx = (uint8_t *)malloc(256);
    if(str104->rx == NULL)
        goto ELEC_104_MALLOC_FAILED;

    str104->tx = (uint8_t *)malloc(256);
    if(str104->tx == NULL)
        goto ELEC_104_MALLOC_FAILED;
    
    str104->config = (STRU_104_Config_TypeDef *)malloc(sizeof(struct STRU_104_Config_TypeDef_t));
    if(str104->config == NULL)
        goto ELEC_104_MALLOC_FAILED;

    str104->rep = (STRU_ASDU_TypeDef *)malloc(sizeof(struct STRU_ASDU_TypeDef_t));
    if(str104->rep == NULL)
        goto ELEC_104_MALLOC_FAILED;
    
    str104->req = (STRU_ASDU_TypeDef *)malloc(sizeof(struct STRU_ASDU_TypeDef_t));
    if(str104->req == NULL)
        goto ELEC_104_MALLOC_FAILED;
    
    str104->cmdStat = (STRU_CMD_STAT_TypeDef *)malloc(sizeof(struct STRU_CMD_STAT_TypeDef_t));
    if(str104->cmdStat == NULL)
        goto ELEC_104_MALLOC_FAILED;
    
    str104->req->data = (uint8_t *)malloc(256);
    if(str104->req->data == NULL)
        goto ELEC_104_MALLOC_FAILED;
    
    str104->rep->data = (uint8_t *)malloc(256);
    if(str104->rep->data == NULL)
        goto ELEC_104_MALLOC_FAILED;
    
    return str104;
ELEC_104_MALLOC_FAILED:
    Elec_104_Free(str104);
    return str104;
}
/*****************************************************************************
 Function    : Elec_104_Free
 Description : 内存释放
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
void Elec_104_Free(STRU_104_TypeDef *str104)
{
    if(str104->rx != NULL)
    {
        free(str104->rx);
        str104->rx = NULL;
    }

    if(str104->tx != NULL)
    {
        free(str104->tx);
        str104->tx = NULL;
    }
    if(str104->config != NULL)
    {
        free(str104->config);
        str104->config = NULL;
    }
    if(str104->rep->data != NULL)
    {
        free(str104->rep->data);
        str104->rep->data = NULL;
    }
    if(str104->rep != NULL)
    {
        free(str104->rep);
        str104->rep = NULL;
    }
    if(str104->req->data != NULL)
    {
        free(str104->req->data);
        str104->req->data = NULL;
    }
    if(str104->req != NULL)
    {
        free(str104->req);
        str104->req = NULL;
    }
    if(str104->cmdStat != NULL)
    {
        free(str104->cmdStat);
        str104->cmdStat = NULL;
    }
    if(str104 != NULL)
    {
        free(str104);
        str104 = NULL;
    }
}
/************************ZXDQ *****END OF FILE****/

