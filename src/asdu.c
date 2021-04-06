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

/*****************************************************************************
 Function    : ASDU_Header_Config
 Description : ASDU帧头配置
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
void ASDU_Header_Config(STRU_ASDU_TypeDef *asdu)
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

/*****************************************************************************
 Function    : ASDU_Frame_Build_Init_OK
 Description : ASDU初始化完成帧配置
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
uint8_t* ASDU_Frame_Build_Init_OK(uint16_t *length, uint8_t coi)
{
    STRU_ASDU_TypeDef asdu;
    uint8_t *data;

    asdu.ti       = M_EI_NA_1;
    asdu.sq       = 0;
    asdu.infoNum  = 1;
    asdu.t        = 0;
    asdu.pn       = 0;
    asdu.cot      = COT_INIT;
    asdu.infoAddr = 0;
    asdu.asduAddr = DEVICE_ID;
    asdu.qoi       = coi;
    asdu.length   = 10;
    
    ASDU_Header_Config(&asdu);
    
    *length = asdu.length;
    data = (uint8_t *)malloc(*length);
    memcpy(data, asdu.data, asdu.length);

    return data;
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
uint8_t* ASDU_Frame_Build_Callall_Signal(uint16_t *length, uint8_t ti, 
               STRU_SIGNAL_TypeDef **signal, uint32_t signalAddr, uint8_t num)
{
    STRU_ASDU_TypeDef asdu;
    uint8_t *data;
    int i = 0;

    asdu.ti       = ti;
    asdu.sq       = 1;
    asdu.infoNum  = num;
    asdu.t        = 0;
    asdu.pn       = 0;
    asdu.cot      = COT_INTROGEN;
    asdu.infoAddr = signalAddr;
    asdu.asduAddr = DEVICE_ID;
    
    ASDU_Header_Config(&asdu);

    //当前总召不考虑时标
    asdu.length   = 9 + 3 + num*1;
    for(i = 0; i < num; i++)
    {
        asdu.data[9 + i*1] = (*signal)->stat;
    }

    *length = asdu.length;
    data = (uint8_t *)malloc(*length);
    memcpy(data, asdu.data, asdu.length);

    return data;
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
uint8_t* ASDU_Frame_Build_Callall_Detect(uint16_t *length, uint8_t ti, 
               STRU_DETECT_TypeDef **detect, uint32_t detectAddr, uint8_t num)
{
    STRU_ASDU_TypeDef asdu;
    uint8_t *data;
    uint8_t floatTmp[4];
    int i = 0;

    asdu.ti       = ti;
    asdu.sq       = 1;
    asdu.infoNum  = num;
    asdu.t        = 0;
    asdu.pn       = 0;
    asdu.cot      = COT_INTROGEN;
    asdu.infoAddr = detectAddr;
    asdu.asduAddr = DEVICE_ID;
    
    ASDU_Header_Config(&asdu);

    //短浮点
    asdu.length   = 9 + 3 + num*5;
    for(i = 0; i < num; i++)
    {
        memcpy(floatTmp, &((*(detect + i))->value), 4);
        asdu.data[9 + i*5]     = floatTmp[0];
        asdu.data[9 + i*5 + 1] = floatTmp[1];
        asdu.data[9 + i*5 + 2] = floatTmp[2];
        asdu.data[9 + i*5 + 3] = floatTmp[3];
        asdu.data[9 + i*5 + 4] = ((*detect)->qds);
    }
    *length = asdu.length;
    data = (uint8_t *)malloc(*length);
    memcpy(data, asdu.data, asdu.length);

    return data;
}
/*****************************************************************************
 Function    : ASDU_Frame_Build_Callall_Control
 Description : 总召唤控制配置
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
uint8_t* ASDU_Frame_Build_Callall_Control(uint16_t *length, uint8_t cot)
{
    STRU_ASDU_TypeDef asdu;
    uint8_t *data;

    asdu.ti       = C_IC_NA_1;
    asdu.sq       = 0;
    asdu.infoNum  = 1;
    asdu.t        = 0;
    asdu.pn       = 0;
    asdu.cot      = cot;
    asdu.infoAddr = 0;
    asdu.asduAddr = DEVICE_ID;
    if(cot == COT_ACTCON)
        asdu.qoi = 20;
    else if(cot == COT_ACTTERM)
        asdu.qoi = 0;
    asdu.length   = 10;
    
    ASDU_Header_Config(&asdu);
    
    *length = asdu.length;
    data = (uint8_t *)malloc(*length);
    memcpy(data, asdu.data, asdu.length);
    return data;
}
/*****************************************************************************
 Function    : ASDU_Frame_Build_Time
 Description : 対时
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
uint8_t* ASDU_Frame_Build_Time(uint16_t *length, STRU_cp56time2a_TypeDef *cp, uint8_t cot)
{
    STRU_ASDU_TypeDef asdu;
    uint8_t *data;

    asdu.ti       = C_CI_NA_1;
    asdu.sq       = 0;
    asdu.infoNum  = 1;
    asdu.t        = 0;
    asdu.pn       = 0;
    asdu.cot      = cot;
    asdu.infoAddr = 0;
    asdu.asduAddr = DEVICE_ID;
    
    asdu.length   = 16;
    
    ASDU_Header_Config(&asdu);

    memcpy(asdu.data + 9, cp, 7);
    *length = asdu.length;
    data = (uint8_t *)malloc(*length);
    memcpy(data, asdu.data, asdu.length);
    return data;
}
/*****************************************************************************
 Function    : ASDU_Frame_Handle
 Description : ASDU帧解析
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
int ASDU_Frame_Handle(uint8_t *data, STRU_ASDU_TypeDef *req)
{
    int res = 0;
    req->ti    = data[TI_ADDR];
    req->cot   = data[COT1_ADDR] & 0x3F;
    req->se    = data[QOI_ADDR] & 0x80;
    req->qoi   = data[QOI_ADDR];
    req->infoAddr = (data[INFO3_ADDR] << 16) + (data[INFO2_ADDR] << 8) + data[INFO1_ADDR];
    switch(req->ti)
    {
        case C_IC_NA_1:
            if((req->qoi == 20) && (req->cot == COT_ACT))
                res = CMD_CALLALL;
        break;
        case C_CI_NA_1:
            res = CMD_NTP;
        break;
        case C_SC_NA_1:case C_SC_NB_1:
            res = CMD_CONTROL;
        break;
        default:;
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
/*****************************************************************************
 Function    : ASDU_Frame_Free
 Description : None
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
void ASDU_Frame_Free(uint8_t *data)
{
    if(data != NULL)
    {
        free(data);
        data = NULL;
    }
}
/************************ZXDQ *****END OF FILE****/


