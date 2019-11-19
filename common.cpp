#include "common.h"


void QLog(const char *tag, const char* format, ...)
{
    char buffer[10240];
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);
    if(strcmp(tag,LOG_DEBUG) == 0){
        qDebug("FILE:%s, FUNC:%s, LINE:%d\n[%s]:  %s",__FILE__,__FUNCTION__,__LINE__,tag,buffer);
    }
    qDebug("[%s]:  %s",tag,buffer);
}