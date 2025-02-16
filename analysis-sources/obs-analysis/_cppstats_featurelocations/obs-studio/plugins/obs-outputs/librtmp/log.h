






















#if !defined(__RTMP_LOG_H__)
#define __RTMP_LOG_H__

#include <stdio.h>
#include <stdarg.h>

#if defined(__cplusplus)
extern "C" {
#endif



#if defined(_DEBUG)
#undef NODEBUG
#endif

typedef enum
{
RTMP_LOGCRIT=0, RTMP_LOGERROR, RTMP_LOGWARNING, RTMP_LOGINFO,
RTMP_LOGDEBUG, RTMP_LOGDEBUG2, RTMP_LOGALL
}
RTMP_LogLevel;

extern RTMP_LogLevel RTMP_debuglevel;

typedef void (RTMP_LogCallback)(int level, const char *fmt, va_list);
void RTMP_LogSetCallback(RTMP_LogCallback *cb);
void RTMP_LogSetOutput(FILE *file);
#if defined(__GNUC__)
void RTMP_LogPrintf(const char *format, ...) __attribute__ ((__format__ (__printf__, 1, 2)));
void RTMP_LogStatus(const char *format, ...) __attribute__ ((__format__ (__printf__, 1, 2)));
void RTMP_Log(int level, const char *format, ...) __attribute__ ((__format__ (__printf__, 2, 3)));
#else
void RTMP_LogPrintf(const char *format, ...);
void RTMP_LogStatus(const char *format, ...);
void RTMP_Log(int level, const char *format, ...);
#endif
void RTMP_LogHex(int level, const uint8_t *data, unsigned long len);
void RTMP_LogHexString(int level, const uint8_t *data, unsigned long len);
void RTMP_LogSetLevel(RTMP_LogLevel lvl);
RTMP_LogLevel RTMP_LogGetLevel(void);

#if defined(__cplusplus)
}
#endif

#endif
