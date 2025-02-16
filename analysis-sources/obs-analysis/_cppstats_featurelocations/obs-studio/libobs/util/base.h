















#pragma once

#include <stdarg.h>

#include "c99defs.h"





#if defined(__cplusplus)
extern "C" {
#endif

#define STRINGIFY(x) #x
#define STRINGIFY_(x) STRINGIFY(x)
#define S__LINE__ STRINGIFY_(__LINE__)

#define INT_CUR_LINE __LINE__
#define FILE_LINE __FILE__ " (" S__LINE__ "): "

enum {







LOG_ERROR = 100,








LOG_WARNING = 200,




LOG_INFO = 300,




LOG_DEBUG = 400
};

typedef void (*log_handler_t)(int lvl, const char *msg, va_list args, void *p);

EXPORT void base_get_log_handler(log_handler_t *handler, void **param);
EXPORT void base_set_log_handler(log_handler_t handler, void *param);

EXPORT void base_set_crash_handler(void (*handler)(const char *, va_list,
void *),
void *param);

EXPORT void blogva(int log_level, const char *format, va_list args);

#if !defined(_MSC_VER) && !defined(SWIG)
#define PRINTFATTR(f, a) __attribute__((__format__(__printf__, f, a)))
#else
#define PRINTFATTR(f, a)
#endif

PRINTFATTR(2, 3)
EXPORT void blog(int log_level, const char *format, ...);
PRINTFATTR(1, 2)
EXPORT void bcrash(const char *format, ...);

#undef PRINTFATTR

#if defined(__cplusplus)
}
#endif
