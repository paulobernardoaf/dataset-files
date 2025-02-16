#if !defined(__MSVC__HEAD)
#define __MSVC__HEAD

#include <direct.h>
#include <process.h>
#include <malloc.h>
#include <io.h>

#pragma warning(disable: 4018) 
#pragma warning(disable: 4244) 
#pragma warning(disable: 4090) 


#define inline __inline
#define __inline__ __inline
#define __attribute__(x)
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define ftruncate _chsize
#define strtoull _strtoui64
#define strtoll _strtoi64

#undef ERROR

#define ftello _ftelli64

typedef int sigset_t;

#define O_ACCMODE (_O_RDONLY | _O_WRONLY | _O_RDWR)

#include "compat/mingw.h"

#endif
