/**
  ******************************************************************************
  * File Name          : example.c
  * Description        : This file provides the example code.
  ******************************************************************************
  * @attention
  
  *
  ******************************************************************************
  */
#include <stdio.h>
#include "example.h"
#include "asdu.h"
#include "tcp.h"
#include "elec_104.h"

extern STRU_104_TypeDef g_str104;
STRU_SIGNAL_TypeDef  signal[SIGNAL_MAX_NUM];
STRU_DETECT_TypeDef  detect[DETECT_MAX_NUM];

void Example_Data_Map(void)
{
    int i = 0;
    g_str104.config.t0 = 30;
    g_str104.config.t1 = 15;
    g_str104.config.t2 = 10;
    g_str104.config.t3 = 20;
    g_str104.config.k  = 20;   
    g_str104.config.bufferSize = 30; 
    g_str104.config.signalNum  = 32; 
    g_str104.config.detectNum  = 32; 
    g_str104.config.controlNum = 16;
    g_str104.config.qoeNum     = 10;    
    g_str104.config.paraNum = 10;
    
    for(i = 0; i < g_str104.config.signalNum; i++)
    {
        signal[i].stat = 1;
        g_str104.signal[i] = &(signal[i]);
    }
    for(i = 0; i < g_str104.config.detectNum; i++)
    {
        detect[i].value = 20.1 + i;
        g_str104.detect[i] = &(detect[i]);
    }  
}
void Example_Start(void)
{
    g_str104.stat104 = STAT_104_POWER_ON;
    Tcp_Listen("192.168.1.144", 2404);
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
/************************ZXDQ *****END OF FILE****/
  

