




























#if defined(PACKAGE)
#error sysdep.h must be included in lieu of config.h
#endif



#include "ansidecl.h"

#if defined(HAVE_STDLIB_H)
#include <stdlib.h>
#endif

#if defined(STRING_WITH_STRINGS)
#include <string.h>
#include <strings.h>
#else
#if defined(HAVE_STRING_H)
#include <string.h>
#else
#if defined(HAVE_STRINGS_H)
#include <strings.h>
#endif
#endif
#endif

#if !HAVE_DECL_STPCPY
extern char *stpcpy (char *__dest, const char *__src);
#endif

#define opcodes_error_handler _bfd_error_handler





#if defined(HAVE_SIGSETJMP)
#define OPCODES_SIGJMP_BUF sigjmp_buf
#define OPCODES_SIGSETJMP(buf) sigsetjmp((buf), 0)
#define OPCODES_SIGLONGJMP(buf,val) siglongjmp((buf), (val))
#else
#define OPCODES_SIGJMP_BUF jmp_buf
#define OPCODES_SIGSETJMP(buf) setjmp(buf)
#define OPCODES_SIGLONGJMP(buf,val) longjmp((buf), (val))
#endif
