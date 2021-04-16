/**
  ******************************************************************************
  * File Name          : ASDU.h
  * Description        : This file provides the code to redefine the ASDU
                         function.
  ******************************************************************************
  * @attention
  
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef _ASDU_H_ 
#define _ASDU_H_
#include <stdint.h>


/*
 ***************************************遥控状态**********************************
*/
#define CONTROL_STAT_ORIGIN   0
#define CONTROL_STAT_SELECTED 1
#define CONTROL_STAT_CONFIRM  2
/*
 ***************************************命令*************************************
*/
#define CMD_ERROR           0       /* 不能识别的类型 */
#define CMD_CALLALL         1       /* 总召唤 */
#define CMD_NTP             2       /* 时间相关 */
#define CMD_CONTROL         3       /* 遥控相关 */

/*
 ***************************************帧头地址**********************************
*/
#define TI_ADDR    0
#define VSQ_ADDR   1
#define COT1_ADDR  2
#define COT2_ADDR  3
#define DEV1_ADDR  4
#define DEV2_ADDR  5
#define INFO1_ADDR 6
#define INFO2_ADDR 7
#define INFO3_ADDR 8
#define QOI_ADDR   9
/*
 ****************************************类型标识**********************************
*/
#define M_SP_NA_1  1                /* 单点信息 */
#define M_DP_NA_1  3                /* 双点信息 */
#define M_ME_NA_1  9                /* 归一化 */
#define M_ME_NB_1  11               /* 标度化 */
#define M_ME_NC_1  13               /* 短浮点 */
#define M_SP_TB_1  30
#define M_DB_TB_1  31
#define M_FT_NA_1  42
#define M_IT_NB_1  206
#define M_IT_TC_1  207
#define C_SC_NA_1  45               /* 单点遥控 */
#define C_SC_NB_1  46               /* 双点遥控 */
#define M_EI_NA_1  70               /* 初始化完成 */
#define C_IC_NA_1  100              /* 总召唤 */
#define C_CI_NA_1  103              /* 时钟同步 */
#define C_CS_NA_1  104
#define C_TS_NA_1  105
#define C_RP_NA_1  200
#define C_SR_NA_1  201
#define C_RR_NA_1  202
#define C_WS_NA_1  203
#define F_FR_NA_1  210
#define F_SR_NA_1  211

/*
 ****************************************传送原因**********************************
*/
#define COT_PER          1
#define COT_BACK         2
#define COT_SPONT        3
#define COT_INIT         4        /* 初始化 */
#define COT_REQ          5        /* 请求 */
#define COT_ACT          6        /* 激活 */
#define COT_ACTCON       7        /* 激活确认 */
#define COT_DEACT        8        /* 撤销 */
#define COT_DEACTCON     9        /* 撤销确认 */
#define COT_ACTTERM      10       /* 激活终止 */
#define COT_FILE         13
#define COT_INTROGEN     20
#define COT_GROUP_1      21
#define COT_GROUP_2      22
#define COT_GROUP_3      23
#define COT_GROUP_4      24
#define COT_GROUP_5      25
#define COT_GROUP_6      26
#define COT_GROUP_7      27
#define COT_GROUP_8      28
#define COT_GROUP_9      29
#define COT_GROUP_10     30
#define COT_GROUP_11     31
#define COT_GROUP_12     32
#define COT_GROUP_13     33
#define COT_GROUP_14     34
#define COT_GROUP_15     35
#define COT_GROUP_16     36
#define COT_NO_TI        44       /* 未知的类型 */
#define COT_NO_COS       45       /* 未知的传送原因 */
#define COT_ERR_DEV_ADDR 46       /* 错误的设备地址 */
#define COT_ERR_OBJ_ADDR 47       /* 错误的信息体地址 */
#define COT_R_S_STRAP    48
#define COT_R_TIME_STAMP 49
#define COT_R_RSA        50


/* 总召唤遥信不使用时标 */
#ifndef  USE_CP56Time2a
    #define SIGNAL_NUM_PER_PACKAGE  176
#else
    #define SIGNAL_NUM_PER_PACKAGE  22
#endif

/* 使用短浮点 */
#define USE_FLOAT

#ifndef  USE_FLOAT    
    #define DETECT_NUM_PER_PACKAGE  57
#else
    #define DETECT_NUM_PER_PACKAGE  35
#endif


#define SIGNAL_MAX_NUM         256
#define DETECT_MAX_NUM         1024
#define CONTROL_MAX_NUM        256
#define QOE_MAX_NUM            256
#define PARAMETER_MAX_NUM      256

#define SIGNAL_START_ADDR      0x0001
#define DETECT_START_ADDR      0x4001
#define CONTROL_START_ADDR     0x6001
#define QOE_START_ADDR         0x6401
#define PARAMETER_START_ADDR   0x8001
/*
 QDS
*/
#define QDS_OV 1          /* 溢出 */
#define QDS_BL 16         /* 被封锁 */
#define QDS_SB 32         /* 被取代 */
#define QDS_NT 64         /* 非当前值 */
#define QDS_IV 128        /* 无效 */
/*
 QPR
*/
#define QPR_POWER_ON     0          /* 本地上电 */
#define QPR_HAND_RESET   1          /* 本地手动 */
#define QPR_REMOTE_RESET 2          /* 远方手动 */
/*
 ****************************************优先级************************************
*/
#define SYSTEM_PRIORITY             1
#define CALLALL_INIT_PRIORITY       2
#define CONTROL_CONFIRM_PRIORITY    3
#define TCOS_PRIORITY               4
#define CALLALL_NON_INIT_PRIORITY   5
#define FAULT_EVENT_PRIORITY        6
#define SNTP_CONFIRM_PRIORITY       7
#define TELEMETRY_CHANGE_PRIORITY   8
#define RESET_PRIORITY              9
#define FILE_CALL_PRIORITY          10
#define FILE_TRANSFER_PRIORITY      11
#define ELECTRICITY_CALL_PRIORITY   12
/*
 cp56time2a结构体
*/
typedef struct cp56time2a 
{
    uint16_t msec :16;
    uint8_t  min  :6;
    uint8_t  res1 :1;
    uint8_t  iv   :1;     /* 0有效 1无效 */
    uint8_t  hour :5;
    uint8_t  res2 :2;
    uint8_t  su   :1;     /* 0标准时间  1夏季时间 */
    uint8_t  mday :5;
    uint8_t  wday :3;
    uint8_t  month:4;
    uint8_t  res3 :4;
    uint8_t  year :7;
    uint8_t  res4 :1;
}STRU_cp56time2a_TypeDef;
/*
 ASDU遥信结构体
*/
typedef struct 
{
    uint8_t type;       /* 类型 0 单点 1 双点 */
    uint8_t stat;
    uint8_t qds;
    STRU_cp56time2a_TypeDef cp;
}STRU_SIGNAL_TypeDef; 
/*
 ASDU遥测结构体
*/
typedef struct 
{
    float value;
    uint8_t qds;
}STRU_DETECT_TypeDef;  

/*
 ASDU遥控结构体
*/
typedef struct 
{
    uint8_t timeCounter;/* 遥控选择超时计数器 */
    uint8_t type;       /* 类型 0 单点 1 双点 */
    uint8_t cmd;        /* 0 分 1 合 */
    uint8_t stat;       /* 0 初始状态 1 选择状态 2 确认状态 */
}STRU_CONTRIL_TypeDef;

/*
 ASDU命令状态结构提
*/
typedef struct STRU_CMD_STAT_TypeDef_t
{
    uint8_t callall;    /* 总召命令状态 0 初始状态 COT_ACT 激活状态*/
    uint8_t control;    /* 控制命令状态 */
}STRU_CMD_STAT_TypeDef; 
/*
 ASDU结构体
*/
typedef struct STRU_ASDU_TypeDef_t
{
    uint8_t  *data;           /* 组合后的ASDU数据帧 */
    uint16_t length;          /* ASDU的长度 */
    uint16_t headerLength;    /* ASDU头的长度 */
    uint8_t  priority;        /* ASDU的优先级 */
    uint8_t  ti;              /* ASDU类型 */
    uint8_t  sq;              /* 离散还是顺序 */
    uint8_t  infoNum;         /* 信息体个数 */
    uint8_t  t;               /* 0 未试验 1 试验 */
    uint8_t  pn;              /* 0肯定确认 1否定确认 */
    uint8_t  cot;             /* 传送原因 */
    uint16_t asduAddr;        /* 公共地址 */
    uint32_t infoAddr;        /* 信息体对象地址 */
    uint8_t  qoi;             /* 限定词 */
    uint8_t  se;              /* 1 遥控选择 0 遥控执行 */
}STRU_ASDU_TypeDef;      


int ASDU_Frame_Handle(STRU_ASDU_TypeDef *asdu);

void ASDU_Frame_Build_Init_OK(STRU_ASDU_TypeDef *asdu, uint8_t qpr);

void ASDU_Frame_Build_Callall_Control(STRU_ASDU_TypeDef *asdu, uint8_t cot);

void ASDU_Frame_Build_Callall_Signal(STRU_ASDU_TypeDef *asdu, uint8_t ti, 
               STRU_SIGNAL_TypeDef **signal, uint32_t signalAddr, uint8_t num);
void ASDU_Frame_Build_Callall_Detect(STRU_ASDU_TypeDef *asdu, uint8_t ti, 
               STRU_DETECT_TypeDef **detect, uint32_t detectAddr, uint8_t num);

void ASDU_Frame_Build_Time(STRU_ASDU_TypeDef* asdu, STRU_cp56time2a_TypeDef *cp, uint8_t cot);
void ASDU_Frame_Build_Error(STRU_ASDU_TypeDef* asdu, STRU_ASDU_TypeDef *req);
void ASDU_Frame_Get_Cp56time2a(uint8_t *data, STRU_cp56time2a_TypeDef *cp);

#endif

/************************ZXDQ *****END OF FILE****/


