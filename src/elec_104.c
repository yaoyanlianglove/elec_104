#include "elec_104.h"
#include "tcp.h"

void Elec_104_Init(STR_104_TypeDef *str_104)
{
    str_104->tx[ADDR_APDU_HEAD] = 0x68;
    str_104->connectStat = 0;
    str_104->rxFrameCounter = 0;
    str_104->txFrameCounter = 0;
    str_104->rxCounter = 0;
    str_104->t1Counter = 0;
    str_104->t2Counter = 0;
    str_104->t3Counter = 0;
    str_104->noConfirmFlag = 0;
    str_104->noSendFlag = 0;
    str_104->kCounter = 0;
    str_104->wCounter = 0;
}
void Elec_104_Set_U(STR_104_TypeDef *str_104, uint8_t type)
{
    str_104->tx[ADDR_APDU_LENGTH] = 0x04;
    str_104->tx[ADDR_ACPI1] = type;
    str_104->tx[ADDR_ACPI2] = 0;
    str_104->tx[ADDR_ACPI3] = 0;
    str_104->tx[ADDR_ACPI4] = 0;
}
void Elec_104_Set_I(STR_104_TypeDef *str_104, uint8_t type)
{
    uint8_t i, obj_num;
    str_104->tx[ADDR_ACPI1] = (str_104->txFrameCounter) & 0xFE;
    str_104->tx[ADDR_ACPI2] = ((str_104->txFrameCounter) >> 8) & 0xFF;
    str_104->tx[ADDR_ACPI3] = (str_104->rxFrameCounter) & 0xFE;
    str_104->tx[ADDR_ACPI4] = ((str_104->rxFrameCounter) >> 8) & 0xFF;

    str_104->tx[ADDR_ASDU_TI]     = type;
    switch(type)
    {
        case M_EI_NA_1: //初始化链路结束
            str_104->tx[ADDR_APDU_LENGTH] = 14;
            str_104->tx[ADDR_ASDU_VSQ]    = 1;
            str_104->tx[ADDR_ASDU_COT_L]  = COT_INIT;  
            str_104->tx[ADDR_ASDU_COT_H]  = 0; 
            str_104->tx[ADDR_ASDU_ADDR_L] = 1; 
            str_104->tx[ADDR_ASDU_ADDR_H] = 0; 
            obj_num = ADDR_ASDU_VSQ & 0x7F;
            for(i = 1; i <= 3; i++)
                str_104->tx[ADDR_ASDU_ADDR_H + i] = 0;               
        break;
    }   
}
void Elec_104_Set_S(STR_104_TypeDef *str_104, uint8_t type)
{
    str_104->tx[ADDR_APDU_LENGTH] = 0x04;
    str_104->tx[ADDR_ACPI1] = 0x01;
    str_104->tx[ADDR_ACPI2] = 0x00;
    str_104->tx[ADDR_ACPI3] = (str_104->rxFrameCounter) & 0xFE;
    str_104->tx[ADDR_ACPI4] = ((str_104->rxFrameCounter) >> 8) & 0xFF;
}
void Elec_104_Send_Data(STR_104_TypeDef *str_104)
{
 //   Tcp_Sent_Data(str_104->tx, (str_104->tx[ADDR_APDU_LENGTH]) + 2);
    if(str_104->tx[ADDR_ACPI1] & 0x03 != 0x03)
        str_104->txFrameCounter++;//发送后再执行
}
STAT_104_TypeDef Elec_104_Handle_U(STR_104_TypeDef *str_104)
{
    switch(str_104->rx[ADDR_ACPI1])
    {
        case STARTDT_COMMAND:
            Elec_104_Set_U(str_104, STARTDT_CONFIRM);
            Elec_104_Send_Data(str_104);
            Elec_104_Set_I(str_104, M_EI_NA_1);
            Elec_104_Send_Data(str_104);
        break;
    }
    return STAT_104_OK;
}
STAT_104_TypeDef Elec_104_Handle_I(STR_104_TypeDef *str_104)
{
    return STAT_104_OK;
}
STAT_104_TypeDef Elec_104_Handle_S(STR_104_TypeDef *str_104)
{
    return STAT_104_OK;
}
STAT_104_TypeDef Elec_104_Handle(STR_104_TypeDef *str_104)
{
    if(str_104->rx[ADDR_ACPI3] & 0x01 == 0)
    {
        if(str_104->rx[ADDR_ACPI1] & 0x01 == 0)
        {
            if(str_104->connectStat == 1)
                return Elec_104_Handle_I(str_104);
            else
                return STAT_104_NO_CONNECT;
        }
        else if(str_104->rx[ADDR_ACPI1] & 0x02 == 0)
        {
            if(str_104->connectStat == 1)
                return Elec_104_Handle_S(str_104);
            else
                return STAT_104_NO_CONNECT;
        }
        else
            return Elec_104_Handle_U(str_104);
    }
    else
        return STAT_104_NO_FRAME;
}
STAT_104_TypeDef Elec_104_Process(STR_104_TypeDef *str_104, uint8_t data)
{
    switch(str_104->rxCounter)
    {
        case 0:
            if(data == 0x68)
            {
                str_104->rx[str_104->rxCounter] = data;
                str_104->rxCounter++;
            }
        break;
        case 1:
            str_104->rx[str_104->rxCounter] = data;
            str_104->rxLength = data;
            str_104->rxCounter++;
        break;
        default:
            if(str_104->rxCounter < str_104->rxLength - 1)
            {
                str_104->rx[str_104->rxCounter] = data;
                str_104->rxCounter++;
            }
            else
            {
                str_104->rxCounter = 0;
                return Elec_104_Handle(str_104);
            }
        break;
    }
    return STAT_104_OK;
}

