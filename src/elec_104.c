#include "elec_104.h"
STR_104_TypeDef g_str_104;

void Elec_104_Init(STR_104_TypeDef *str_104)
{
    str_104->tx[0] = 0x68;
    str_104->connectStat = 0;
    str_104->rxFrameCounter = 0;
    str_104->txFrameCounter = 0;
    str_104->rxCounter = 0;
}
void Elec_104_Set_U(STR_104_TypeDef *str_104, uint8_t type)
{
    str_104->tx[APDU_LENGTH] = 0x04;
    str_104->tx[ACPI1] = type;
    str_104->tx[ACPI2] = 0;
    str_104->tx[ACPI3] = 0;
    str_104->tx[ACPI4] = 0;
}
void Elec_104_Set_I(STR_104_TypeDef *str_104, uint8_t type)
{
    str_104->tx[ACPI1] = (str_104->txCounter) & 0xFE;
    str_104->tx[ACPI2] = ((str_104->txCounter) >> 8) & 0xFF;
    str_104->tx[ACPI3] = (str_104->rxCounter) & 0xFE;
    str_104->tx[ACPI4] = ((str_104->rxCounter) >> 8) & 0xFF;

    str_104->tx[ASDU_TI]     = type;
    switch(type)
    {
        case M_EI_NA_1: //初始化链路结束
            str_104->tx[APDU_LENGTH] = 14;
            str_104->tx[ASDU_VSQ]    = 1;
            str_104->tx[ASDU_COT_H]  = COT_INIT;  
            str_104->tx[ASDU_COT_L]  = 0;                           
        break;
    }
    
}
void Elec_104_Set_S(STR_104_TypeDef *str_104, uint8_t type)
{
    str_104->tx[APDU_LENGTH] = 0x04;
    str_104->tx[ACPI1] = 0x01;
    str_104->tx[ACPI2] = 0x00;
    str_104->tx[ACPI3] = (str_104->rxCounter) & 0xFE;
    str_104->tx[ACPI4] = ((str_104->rxCounter) >> 8) & 0xFF;
}
void Elec_104_Send_Data(str_104)
{
    Tcp_Send_Data(str_104->tx, (str_104->tx[APDU_LENGTH]) + 2);
    if(str_104->tx[ACPI1] & 0x03 != 0x03)
        str_104->txFrameCounter++;//发送后再执行
}
STAT_104_TypeDef Elec_104_Handle_U(str_104)
{
    switch(str_104->rx[ACPI1])
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
STAT_104_TypeDef Elec_104_Handle(STR_104_TypeDef *str_104)
{
    if(str_104->rx[ACPI3] & 0x01 == 0)
    {
        if(str_104->rx[ACPI1] & 0x01 == 0)
        {
            if(str_104->connectStat == 1)
                return Elec_104_Handle_I(str_104);
            else
                return STAT_104_NO_CONNECT;
        }
        else if(str_104->rx[ACPI1] & 0x02 == 0)
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
    if(str_104->rxFlag == 1)
    {
        return STAT_104_OK;
    }
    switch(str_104->rxCounter)
    {
        case 0:
            if(data == 0x68 && str_104->rxCounter < 255)
                str_104->rxCounter++;
        break;
        case 1:
            str_104->rxLength = data;
        break;
        default:
            if(str_104->rxCounter < str_104->rxLength - 1)
                str_104->rxCounter++;
            else
            {
                str_104->rxFlag = 1;
                return Elec_104_Handle(str_104);
            }
        break;
    }
    str_104->rxFlag = 0;
    return STAT_104_OK;
}

