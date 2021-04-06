/**
  ******************************************************************************
  * File Name          : elec_104.h
  * Description        : This file provides the code to redefine the elec_104 
                         function.
  ******************************************************************************
  * @attention
  
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef _ELEC_104_H_ 
#define _ELEC_104_H_
#include <stdint.h>
#include "asdu.h"
/*
 **************地址******************
*/
#define ADDR_APDU_HEAD   0
#define ADDR_APDU_LENGTH 1
#define ADDR_APCI1       2
#define ADDR_APCI2       3
#define ADDR_APCI3       4
#define ADDR_APCI4       5

#define K_MAX 12  
#define W_MAX 8

#define T0 30  //秒
#define T1 15  //秒
#define T2 10  //秒
#define T3 20  //秒
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
 **************优先级******************
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
 104状态
*/
typedef enum
{
    STAT_104_POWER_ON          = 0x00U,     //设备上电  
    STAT_104_RECIEVE_START     = 0x01U,     //接收到启动连接命令
    STAT_104_SEND_START_ACK    = 0x02U,     //发送启动连接确认 
    STAT_104_DEVICE_INIT_OK    = 0x03U,     //设备初始化完成
    STAT_104_SEND_INIT_OK      = 0x04U,     //发送初始化完成命令
    STAT_104_SEND_INIT_OK_ACK  = 0x05U,     //初始化完成命令被确认
    STAT_104_CONNECT_OK        = 0x06U,     //连接成功
    STAT_104_RECIEVE_STOP      = 0x07U,     //收到停止连接命令
    STAT_104_SEND_STOP_ACK     = 0x08U,     //发送停止连接命令确认
    STAT_104_CONNECT_BREAK     = 0x09U,     //连接断开     
    STAT_104_RECIEVE_TEST_CMD  = 0x0AU,     //收到测试命令
    STAT_104_RECIEVE_TEST_CON  = 0x0BU,     //收到测试确认命令
    STAT_104_SEND_TEST         = 0x0CU,     //发送测试命令
}ENUM_STAT_104_TypeDef;

/*
 队列结构体
 插入队列时，优先级小的在前面;
 发送数据时，取缓存里未发送数据的第一个发送;
 确认发送完成，删除队列里面发送序号小于等于当前对端接受序号的。
*/
typedef struct STRU_Queue_TypeDef_t
{
    uint8_t  txFlag;                        //发送状态，1 已发送，0 未发送
    uint8_t  t1Counter;                     //I帧超时计数
    uint32_t txNum;                         //发送序号 
    uint8_t  priority;                      //优先级
    uint8_t  *data;                         //数据
    uint8_t  length;                        //数据长度
    struct STRU_Queue_TypeDef_t *next;
}STRU_Queue_TypeDef;


/*
 104config
*/
typedef struct 
{
    uint8_t  t0;
    uint8_t  t1;
    uint8_t  t2;
    uint8_t  t3;
    uint16_t k;
    uint16_t bufferSize;                    //bufferSize缓存大小，必须大于k值
    uint16_t signalNum;                     //遥信个数
    uint16_t detectNum;                     //遥测个数
    uint16_t controlNum;                    //遥控个数
    uint16_t qoeNum;                        //电能量个数
    uint16_t paraNum;                       //参数个数
}STRU_104_Config_TypeDef;
/*
 104结构体
*/
typedef struct 
{
    ENUM_STAT_104_TypeDef  stat104;         //104的状态
    ENUM_STAT_104_TypeDef  lastStat104;     //上一个104的状态
    int      socketfd;                
    uint8_t  breakConnectFlag;              //断开连接标志
    uint16_t rxNum;                         //自己的接收序号
    uint16_t otherRxNum;                    //对端的接收序号 
    uint16_t txNum;                         //自己的发送序号
    uint16_t otherTxNum;                    //对端的发送序号
    uint8_t  rx[256];                       //rx buffer
    uint8_t  rxCounter;                     //接正确的字节计数
    uint16_t rxLength;                      //接收数据的长度
    uint8_t  tx[256];                       //发送数据
    uint16_t txLength;                      //发送数据的长度
        
    uint16_t numAll;                        //需要传输的所有缓存数据，包括已发送待确认和待发送数据
    uint16_t numSDWTC;                      //已发送，待确认的数据个数
    STRU_Queue_TypeDef *pSendHead;          //发送数据的缓存头指针
        
    STRU_104_Config_TypeDef config;         //配置文件
    uint8_t  t1Counter;                     //发送u帧测试帧后t1计时
    uint8_t  uT1Flag;                       //发送u帧测试帧标志
    uint8_t  t0Counter;
    uint8_t  t2Counter;
    uint8_t  t3Counter;
    STRU_SIGNAL_TypeDef  *signal[SIGNAL_MAX_NUM];       //状态量信息
    STRU_DETECT_TypeDef  *detect[DETECT_MAX_NUM];       //模拟量信息
    STRU_CONTRIL_TypeDef  *control[CONTROL_MAX_NUM];    //控制量信息
    float    *qoe[QOE_MAX_NUM];             //电能量信息
    int16_t  *parameter[PARAMETER_MAX_NUM]; //参数量信息
}STRU_104_TypeDef;


/********缓存区链表操作*************************************************
* 所有待发送的数据插入链表中，确认被接收的数据删除，
**********************************************************************/
STRU_Queue_TypeDef* Elec_104_Queue_Init(void);
void Elec_104_Queue_Print(STRU_Queue_TypeDef *list);
STRU_Queue_TypeDef* Elec_104_Queue_Insert(STRU_104_TypeDef *str104, 
                                          uint8_t priority);
int  Elec_104_Queue_Delete(STRU_104_TypeDef *str104);

void Elec_104_Init(STRU_104_TypeDef *str104);
int  Elec_104_Send_Test_Frame(STRU_104_TypeDef *str104);
int  Elec_104_Sent_Init_OK(STRU_104_TypeDef *str104);
void Elec_104_Print_Frame(STRU_104_TypeDef *str104, uint8_t type); //type 0 打印rx，1 打印tx
int  Elec_104_Frame_Receive(STRU_104_TypeDef *str104, uint8_t data);


int  Elec_104_Stat_Handle(STRU_104_TypeDef *str104);



__attribute__((weak)) void Elec_104_Set_Time_CallBack(STRU_cp56time2a_TypeDef *cp);
__attribute__((weak)) void Elec_104_Get_Time_CallBack(STRU_cp56time2a_TypeDef *cp);

#endif


