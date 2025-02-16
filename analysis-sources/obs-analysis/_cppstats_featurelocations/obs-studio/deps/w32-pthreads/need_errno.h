














#if _MSC_VER > 1000
#pragma once
#endif

#if !defined(_INC_ERRNO)
#define _INC_ERRNO

#if !defined(_WIN32)
#error ERROR: Only Win32 targets supported!
#endif

#include <winsock.h>

#if defined(__cplusplus)
extern "C" {
#endif





#if !defined(_CRTIMP)
#if defined(_DLL)
#define _CRTIMP __declspec(dllimport)
#else 
#define _CRTIMP
#endif 
#endif 




#if ( !defined(_MSC_VER) && !defined(__cdecl) )
#define __cdecl
#endif



#if !defined(_CRTAPI1)
#if _MSC_VER >= 800 && _M_IX86 >= 300
#define _CRTAPI1 __cdecl
#else
#define _CRTAPI1
#endif
#endif

#if !defined(PTW32_STATIC_LIB)
#if defined(PTW32_BUILD)
#define PTW32_DLLPORT __declspec (dllexport)
#else
#define PTW32_DLLPORT __declspec (dllimport)
#endif
#else
#define PTW32_DLLPORT
#endif



#if (defined(_MT) || defined(_MD) || defined(_DLL)) && !defined(_MAC)
PTW32_DLLPORT int * __cdecl _errno(void);
#define errno (*_errno())
#else 
_CRTIMP extern int errno;
#endif 



#define EPERM 1
#define ENOENT 2
#define ESRCH 3
#define EINTR 4
#define EIO 5
#define ENXIO 6
#define E2BIG 7
#define ENOEXEC 8
#define EBADF 9
#define ECHILD 10
#define EAGAIN 11
#define ENOMEM 12
#define EACCES 13
#define EFAULT 14
#define EBUSY 16
#define EEXIST 17
#define EXDEV 18
#define ENODEV 19
#define ENOTDIR 20
#define EISDIR 21
#define EINVAL 22
#define ENFILE 23
#define EMFILE 24
#define ENOTTY 25
#define EFBIG 27
#define ENOSPC 28
#define ESPIPE 29
#define EROFS 30
#define EMLINK 31
#define EPIPE 32
#define EDOM 33
#define ERANGE 34
#define EDEADLK 36


#if !defined(ENAMETOOLONG)
#define ENAMETOOLONG 38
#endif

#define ENOLCK 39
#define ENOSYS 40


#if !defined(ENOTEMPTY)
#define ENOTEMPTY 41
#endif

#define EILSEQ 42


#define EOWNERDEAD 43
#define ENOTRECOVERABLE 44




#define EDEADLOCK EDEADLK

#if defined(__cplusplus)
}
#endif

#endif 
