/**
  ******************************************************************************
  * File Name          : ASDU.c
  * Description        : This file provides the code to redefine the ASDU
                         function.
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "asdu.h"
#include "debug.h"

/*****************************************************************************
 Function    : ASDU_Header_Config
 Description : ASDU帧头配置
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
void ASDU_Header_Config(STRU_ASDU_TypeDef *asdu)
{
    if(asdu->headerLength == 9)
    {
        asdu->data[TI_ADDR]    = asdu->ti;
        asdu->data[VSQ_ADDR]   = (asdu->sq << 7) | asdu->infoNum;
        asdu->data[COT1_ADDR]  = asdu->cot | (asdu->t << 7) | (asdu->pn << 6);
        asdu->data[COT2_ADDR]  = 0x00;
        asdu->data[DEV1_ADDR]  = asdu->asduAddr & 0xFF;
        asdu->data[DEV2_ADDR]  = (asdu->asduAddr >> 8) & 0xFF;
        asdu->data[INFO1_ADDR] = asdu->infoAddr & 0xFF;
        asdu->data[INFO2_ADDR] = (asdu->infoAddr >> 8) & 0xFF;
        asdu->data[INFO3_ADDR] = (asdu->infoAddr >> 16) & 0xFF;
        asdu->data[QOI_ADDR]   = asdu->qoi;
    }
    else
    {
        /*其他情况*/
    }
}

/*****************************************************************************
 Function    : ASDU_Frame_Build_Init_OK
 Description : ASDU初始化完成帧配置
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
void ASDU_Frame_Build_Init_OK(STRU_ASDU_TypeDef *asdu, uint8_t qpr)
{
    asdu->ti       = M_EI_NA_1;
    asdu->sq       = 0;
    asdu->infoNum  = 1;
    asdu->t        = 0;
    asdu->pn       = 0;
    asdu->cot      = COT_INIT;
    asdu->infoAddr = 0;
    asdu->qoi      = qpr;
    asdu->length   = 10;
    asdu->priority = SYSTEM_PRIORITY;
    ASDU_Header_Config(asdu);
}
/*****************************************************************************
 Function    : ASDU_Frame_Build_Callall_Signal
 Description : 总召唤用户数据配置
 Input       : length     回传ASDU数据的长度
               ti         ASDU类型
               signal     遥信数据地址（内存地址）
               signalAddr 数据信息体地址（点表地址）
               num        遥信个数
 Output      : None
 Return      : None
 *****************************************************************************/
void ASDU_Frame_Build_Callall_Signal(STRU_ASDU_TypeDef *asdu, uint8_t ti, 
               STRU_SIGNAL_TypeDef **signal, uint32_t signalAddr, uint8_t num)
{
    int i = 0;
    uint8_t headerLen = asdu->headerLength;
    asdu->ti       = ti;
    asdu->sq       = 1;
    asdu->infoNum  = num;
    asdu->t        = 0;
    asdu->pn       = 0;
    asdu->cot      = COT_INTROGEN;
    asdu->infoAddr = signalAddr;
    
    ASDU_Header_Config(asdu);

    /* 当前总召不考虑时标 */
    asdu->length   = headerLen + num*1;
    for(i = 0; i < num; i++)
    {
        asdu->data[headerLen + i*1] = (*signal)->stat | (*signal)->qds;
    }
}
/*****************************************************************************
 Function    : ASDU_Frame_Build_Callall_Detect
 Description : 总召唤用户数据配置
 Input       : length     回传ASDU数据的长度
               ti         ASDU类型
               detect     遥测数据地址（内存地址）
               detectAddr 数据信息体地址（点表地址）
               num        遥信个数
 Output      : None
 Return      : None
 *****************************************************************************/
void ASDU_Frame_Build_Callall_Detect(STRU_ASDU_TypeDef *asdu, uint8_t ti, 
               STRU_DETECT_TypeDef **detect, uint32_t detectAddr, uint8_t num)
{
    int i = 0;
    uint8_t floatTmp[4];
    uint8_t headerLen = asdu->headerLength;

    asdu->ti       = ti;
    asdu->sq       = 1;
    asdu->infoNum  = num;
    asdu->t        = 0;
    asdu->pn       = 0;
    asdu->cot      = COT_INTROGEN;
    asdu->infoAddr = detectAddr;
    
    ASDU_Header_Config(asdu);

    //短浮点
    asdu->length   = headerLen + num*5;
    for(i = 0; i < num; i++)
    {
        memcpy(floatTmp, &((*(detect + i))->value), 4);
        asdu->data[headerLen + i*5]     = floatTmp[0];
        asdu->data[headerLen + i*5 + 1] = floatTmp[1];
        asdu->data[headerLen + i*5 + 2] = floatTmp[2];
        asdu->data[headerLen + i*5 + 3] = floatTmp[3];
        asdu->data[headerLen + i*5 + 4] = ((*detect)->qds);
    }
}
/*****************************************************************************
 Function    : ASDU_Frame_Build_Callall_Control
 Description : 总召唤控制配置
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
void ASDU_Frame_Build_Callall_Control(STRU_ASDU_TypeDef *asdu, uint8_t cot)
{
    asdu->ti       = C_IC_NA_1;
    asdu->sq       = 0;
    asdu->infoNum  = 1;
    asdu->t        = 0;
    asdu->pn       = 0;
    asdu->cot      = cot;
    asdu->infoAddr = 0;
    if(cot == COT_ACTCON)
        asdu->qoi = 20;
    else if(cot == COT_ACTTERM)
        asdu->qoi = 0;
    asdu->length   = 10;
    
    ASDU_Header_Config(asdu);
}
/*****************************************************************************
 Function    : ASDU_Frame_Build_Time
 Description : 対时
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
void ASDU_Frame_Build_Time(STRU_ASDU_TypeDef *asdu, STRU_cp56time2a_TypeDef *cp, uint8_t cot)
{
    asdu->ti       = C_CI_NA_1;
    asdu->sq       = 0;
    asdu->infoNum  = 1;
    asdu->t        = 0;
    asdu->pn       = 0;
    asdu->cot      = cot;
    asdu->infoAddr = 0;
   
    asdu->length   = 16;
    
    ASDU_Header_Config(asdu);
    memcpy(asdu->data + asdu->headerLength, cp, 7);
}
/*****************************************************************************
 Function    : ASDU_Frame_Build_Error
 Description : 错误回复
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
void ASDU_Frame_Build_Error(STRU_ASDU_TypeDef *asdu, STRU_ASDU_TypeDef *req)
{
    memcpy(asdu->data, req->data, req->length);
    if(asdu->headerLength == 9)
        asdu->data[COT2_ADDR] = asdu->cot | 0x40; /* 否定 */
}
/*****************************************************************************
 Function    : ASDU_Frame_Handle
 Description : ASDU帧解析
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
int ASDU_Frame_Handle(STRU_ASDU_TypeDef *asdu)
{
    int res = 0;
    DBUG("asdu->headerLength is %d\n", asdu->headerLength);
    if(asdu->headerLength == 9)
    {
        asdu->ti       = asdu->data[TI_ADDR];
        asdu->cot      = asdu->data[COT1_ADDR] & 0x3F;
        asdu->se       = asdu->data[QOI_ADDR] & 0x80;
        asdu->qoi      = asdu->data[QOI_ADDR];
        asdu->infoAddr = (asdu->data[INFO3_ADDR] << 16) + (asdu->data[INFO2_ADDR] << 8) + asdu->data[INFO1_ADDR];
        asdu->asduAddr = (asdu->data[DEV2_ADDR] << 8) + asdu->data[DEV1_ADDR];
    }
    switch(asdu->ti)
    {
        case C_IC_NA_1:
            res = CMD_CALLALL;
        break;
        case C_CI_NA_1:
            res = CMD_NTP;
        break;
        case C_SC_NA_1:case C_SC_NB_1:
            res = CMD_CONTROL;
        break;
        default:
            res = CMD_ERROR;
    }
    return res;
}
/*****************************************************************************
 Function    : ASDU_Frame_Get_Cp56time2a
 Description : None
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
void ASDU_Frame_Get_Cp56time2a(uint8_t *data, STRU_cp56time2a_TypeDef *cp)
{
    memcpy(cp, data, 7);
}
/************************ZXDQ *****END OF FILE****/


