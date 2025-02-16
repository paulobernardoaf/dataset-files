











#include "os_dos.h" 
#if !defined(__CYGWIN__)

#if !defined(PROTO)
#include <direct.h> 
#endif
#endif

#define BINARY_FILE_IO
#define USE_EXE_NAME 
#define USE_TERM_CONSOLE
#if !defined(HAVE_STRING_H)
#define HAVE_STRING_H
#endif
#if !defined(HAVE_MATH_H)
#define HAVE_MATH_H
#endif
#define HAVE_STRCSPN
#if !defined(__GNUC__)
#define HAVE_STRICMP
#define HAVE_STRNICMP
#endif
#if !defined(HAVE_STRFTIME)
#define HAVE_STRFTIME 
#endif
#define HAVE_MEMSET
#if !defined(HAVE_LOCALE_H)
#define HAVE_LOCALE_H 1
#endif
#if !defined(HAVE_FCNTL_H)
#define HAVE_FCNTL_H
#endif
#define HAVE_QSORT
#define HAVE_ST_MODE 

#define FEAT_SHORTCUT 

#if (!defined(_MSC_VER) || _MSC_VER > 1020)




#define HAVE_ACL
#endif

#define USE_FNAME_CASE 
#if !defined(FEAT_CLIPBOARD)
#define FEAT_CLIPBOARD 
#endif
#if defined(__DATE__) && defined(__TIME__)
#define HAVE_DATE_TIME
#endif
#if !defined(FEAT_GUI_MSWIN)
#define BREAKCHECK_SKIP 1 
#endif

#define HAVE_TOTAL_MEM

#define HAVE_PUTENV 

#if defined(FEAT_GUI_MSWIN) && !defined(VIMDLL)
#define NO_CONSOLE 
#endif



#define BROKEN_TOUPPER

#define FNAME_ILLEGAL "\"*?><|" 

#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>

#if !defined(STRICT)
#define STRICT
#endif
#if !defined(COBJMACROS)
#define COBJMACROS 
#endif
#if !defined(PROTO)
#include <windows.h>
#if !defined(SM_CXPADDEDBORDER)
#define SM_CXPADDEDBORDER 92
#endif
#endif




#define CMDBUFFSIZE 1024 



#define MAXPATHL 1024

#if !defined(BASENAMELEN)
#define BASENAMELEN (_MAX_PATH - 5) 
#endif

#define TEMPNAMELEN _MAX_PATH 

#if !defined(DFLT_MAXMEM)
#define DFLT_MAXMEM (2*1024) 
#endif

#if !defined(DFLT_MAXMEMTOT)
#define DFLT_MAXMEMTOT (5*1024) 
#endif




#if !defined(FILE_ATTRIBUTE_REPARSE_POINT)
#define FILE_ATTRIBUTE_REPARSE_POINT 0x00000400
#endif
#if !defined(IO_REPARSE_TAG_MOUNT_POINT)
#define IO_REPARSE_TAG_MOUNT_POINT 0xA0000003
#endif
#if !defined(IO_REPARSE_TAG_SYMLINK)
#define IO_REPARSE_TAG_SYMLINK 0xA000000C
#endif

#if defined(_MSC_VER)


#define HAVE_TRY_EXCEPT 1
#include <malloc.h> 
#if defined(_MSC_VER) && (_MSC_VER >= 1300)
#define RESETSTKOFLW _resetstkoflw
#else
#define RESETSTKOFLW myresetstkoflw
#define MYRESETSTKOFLW
#endif
#endif






#if defined(_DEBUG)

#if defined(_MSC_VER) && (_MSC_VER >= 1000)

#include <crtdbg.h>
#define ASSERT(f) _ASSERT(f)
#else
#include <assert.h>
#define ASSERT(f) assert(f)
#endif

#define TRACE Trace
#define TRACE0(sz) Trace(_T("%s"), _T(sz))
#define TRACE1(sz, p1) Trace(_T(sz), p1)
#define TRACE2(sz, p1, p2) Trace(_T(sz), p1, p2)
#define TRACE3(sz, p1, p2, p3) Trace(_T(sz), p1, p2, p3)
#define TRACE4(sz, p1, p2, p3, p4) Trace(_T(sz), p1, p2, p3, p4)


void __cdecl
Trace(char *pszFormat, ...);

#else 


#define ASSERT(f) ((void)0)
#define TRACE 1 ? (void)0 : printf
#define TRACE0(sz)
#define TRACE1(sz, p1)
#define TRACE2(sz, p1, p2)
#define TRACE3(sz, p1, p2, p3)
#define TRACE4(sz, p1, p2, p3, p4)

#endif 


#define ASSERT_POINTER(p, type) ASSERT(((p) != NULL) && IsValidAddress((p), sizeof(type), FALSE))


#define ASSERT_NULL_OR_POINTER(p, type) ASSERT(((p) == NULL) || IsValidAddress((p), sizeof(type), FALSE))


#if !defined(HAVE_SETENV)
#define HAVE_SETENV
#endif
#define mch_getenv(x) (char_u *)getenv((char *)(x))
#define vim_mkdir(x, y) mch_mkdir(x)


#define pDispatchMessage DispatchMessageW
#define pGetMessage GetMessageW
#define pIsDialogMessage IsDialogMessageW
#define pPeekMessage PeekMessageW
