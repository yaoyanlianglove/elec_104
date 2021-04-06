/**
  ******************************************************************************
  * File Name          : debug.h
  * Description        : This file provides the code to redefine the printf 
                         function.
  ******************************************************************************
  * @attention
  
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __debug_H
#define __debug_H

#include <stdio.h>
#include <string.h>

#define DEBUG_LOG  1


void lprintf(const char *fp, const int line);
int log_save(const char *format, ...);

#define DBUG(fmt,...)\
    if(DEBUG_LOG == 0)\
        ;\
    else if(DEBUG_LOG == 1)\
        {lprintf(__FILE__, __LINE__); \
        printf(fmt, ##__VA_ARGS__); }\
    else \
        {log_save(fmt, ##__VA_ARGS__);}

#endif
/************************ZXDQ *****END OF FILE****/
  

