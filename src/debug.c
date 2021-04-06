/**
  ******************************************************************************
  * File Name          : debug.c
  * Description        : This file provides the code to redefine the printf 
                         function.
  ******************************************************************************
  * @attention
  
  *
  ******************************************************************************
  */
#include "debug.h"
#include <time.h>
#include <stdarg.h>
#include <unistd.h>
#define LOG_MAX_SIZE      1024*1024
#define VQC_LOG_PATH      "/var/log/curl2tf.log"       
/*****************************************************************************
 Function    : lprintf
 Description : None
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
void lprintf(const char *fp, const int line)
{
    char sDebug[1024];
    char *ptr=NULL;

    memset(sDebug,'\0',sizeof(sDebug));
    time_t time_log = time(NULL);  
    struct tm* tm_log = localtime(&time_log);  
    sprintf(sDebug, "%04d-%02d-%02d %02d:%02d:%02d ", tm_log->tm_year + 1900, 
        tm_log->tm_mon + 1, tm_log->tm_mday, tm_log->tm_hour, tm_log->tm_min, tm_log->tm_sec);

    ptr=strrchr(fp,'/');

    if(ptr)
    {
        sprintf(sDebug + strlen(sDebug),"%s[%d]:", ptr+1, line);
    }
    else
    {
        sprintf(sDebug + strlen(sDebug),"%s[%d]:", fp, line);
    }
    printf("%s", sDebug);
}
/*****************************************************************************
 Function    : get_file_size
 Description : None
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
long get_file_size(char* filename)
{
    long length = 0;
    FILE *fp = NULL;

    fp = fopen(filename, "rb");
    if (fp != NULL)
    {
        fseek(fp, 0, SEEK_END);
        length = ftell(fp);
    }

    if (fp != NULL)
    {
        fclose(fp);
        fp = NULL;
    }
    return length;
}
/*****************************************************************************
 Function    : log_save
 Description : None
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
int log_save(const char *format, ...)
{  
    long length = get_file_size(VQC_LOG_PATH);
    if (length > LOG_MAX_SIZE)
    {
        unlink(VQC_LOG_PATH); 
    }
    FILE* pFile = fopen(VQC_LOG_PATH, "a+");
    if(!pFile)
    {
        return -1;
    }
    va_list arg;  
    int done;  
    va_start(arg, format);  
    time_t time_log = time(NULL);  
    struct tm* tm_log = localtime(&time_log);  
    fprintf(pFile, "%04d-%02d-%02d %02d:%02d:%02d ", tm_log->tm_year + 1900, 
        tm_log->tm_mon + 1, tm_log->tm_mday, 
        tm_log->tm_hour, tm_log->tm_min, tm_log->tm_sec);  
    done = vfprintf (pFile, format, arg);  
    va_end(arg);  

    fflush(pFile);  
    fclose(pFile);
    return done;  
}
/************************ZXDQ *****END OF FILE****/
