#ifndef _ELEC_104_H_ 
#define _ELEC_104_H_

/*
 **************地址******************
*/
#define APDU_HEAD   01
#define APDU_LENGTH 02
#define ACPI1       03
#define ACPI2       04
#define ACPI3       05
#define ACPI4       06
#define ASDU_TI     07
#define ASDU_VSQ    08
#define ASDU_COT_H  09
#define ASDU_COT_L  10
#define ASDU_ADDR   11
/*
 **************类型标识******************
*/
#define M_SP_NA_1  1
#define M_DP_NA_1  3
#define M_ME_NA_1  9
#define M_ME_NB_1  11
#define M_ME_NC_1  13
#define M_SP_TB_1  30
#define M_DB_TB_1  31
#define M_FT_NA_1  42
#define M_IT_NB_1  206
#define M_IT_TC_1  207
#define C_SC_NA_1  45
#define C_SC_NB_1  46
#define M_EI_NA_1  70
#define C_IC_NA_1  100
#define C_CI_NA_1  103
#define C_CS_NA_1  104
#define C_TS_NA_1  105
#define C_RP_NA_1  200
#define C_SR_NA_1  201
#define C_RR_NA_1  202
#define C_WS_NA_1  203
#define F_FR_NA_1  210
#define F_SR_NA_1  211

/*
 **************传送原因******************
*/
#define COT_PER          1
#define COT_BACK         2
#define COT_SPONT        3
#define COT_INIT         4
#define COT_REQ          5
#define COT_ACT          6
#define COT_ACTCON       7
#define COT_DEACT        8
#define COT_DEACTCON     9
#define COT_ACTTERM      10
#define COT_FILE         13
#define COT_INTROGEN_0   20
#define COT_INTROGEN_1   21
#define COT_INTROGEN_2   22
#define COT_INTROGEN_3   23
#define COT_INTROGEN_4   24
#define COT_INTROGEN_5   25
#define COT_INTROGEN_6   26
#define COT_INTROGEN_7   27
#define COT_INTROGEN_8   28
#define COT_INTROGEN_9   29
#define COT_INTROGEN_10  30
#define COT_INTROGEN_11  31
#define COT_INTROGEN_12  32
#define COT_INTROGEN_13  33
#define COT_INTROGEN_14  34
#define COT_INTROGEN_15  35
#define COT_INTROGEN_16  36
#define COT_NO_TI        44
#define COT_NO_COS       45   
#define COT_NO_DATA_ADDR  46
#define COT_NO_OBJ_ADDR   47
#define COT_R_S_STRAP    48
#define COT_R_TIME_STAMP 49
#define COT_R_RSA        50

#define K 12  
#define W 1
/*
 **************U帧填充******************
*/
#define STARTDT_COMMAND  0x07
#define STARTDT_CONFIRM  0x0B

#define STOPDT_COMMAND   0x13
#define STOPDT_CONFIRM   0x23

#define TESTFR_COMMAND   0x43
#define TESTFR_CONFIRM   0x83

/*
 104状态
*/
typedef enum
{
    STAT_104_OK           = 0x00U,
    STAT_104_NO_FRAME     = 0x01U,
}STAT_104_TypeDef;
/*
 104结构体
*/
typedef struct 
{
    uint8_t  connectStat;     //与主站的连接状态0 未连接,1 已连接
    uint16_t rxFrameCounter;  //接收帧计数
    uint16_t txFrameCounter;  //发送帧基数
    char rx[256];             //rx buffer
    uint8_t  rxCounter;       //接正确的字节计数
    uint8_t  rxFlag;          //接收一帧数据标志，0未接收一帧数据，1已接收一帧数据
    uint8_t  rxLength;        //接收数据的长度
    char tx[256];             //tx buffer,APDU
}STR_104_TypeDef;

#endif