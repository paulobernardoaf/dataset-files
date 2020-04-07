




#if defined(_LARGEFILE64_SOURCE)
#if !defined(_LARGEFILE_SOURCE)
#define _LARGEFILE_SOURCE 1
#endif
#if defined(_FILE_OFFSET_BITS)
#undef _FILE_OFFSET_BITS
#endif
#endif

#if defined(HAVE_HIDDEN)
#define ZLIB_INTERNAL __attribute__((visibility ("hidden")))
#else
#define ZLIB_INTERNAL
#endif

#include <stdio.h>
#include "zlib.h"
#if defined(STDC)
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#endif
#include <fcntl.h>

#if defined(__WINDOWS__)
#include <stddef.h>
#endif

#if defined(__TURBOC__) || defined(_MSC_VER) || defined(__WINDOWS__)
#include <io.h>
#endif

#if defined(WINAPI_FAMILY)
#define open _open
#define read _read
#define write _write
#define close _close
#endif

#if defined(NO_DEFLATE)
#define NO_GZCOMPRESS
#endif

#if defined(STDC99) || (defined(__TURBOC__) && __TURBOC__ >= 0x550)
#if !defined(HAVE_VSNPRINTF)
#define HAVE_VSNPRINTF
#endif
#endif

#if defined(__CYGWIN__)
#if !defined(HAVE_VSNPRINTF)
#define HAVE_VSNPRINTF
#endif
#endif

#if defined(MSDOS) && defined(__BORLANDC__) && (BORLANDC > 0x410)
#if !defined(HAVE_VSNPRINTF)
#define HAVE_VSNPRINTF
#endif
#endif

#if !defined(HAVE_VSNPRINTF)
#if defined(MSDOS)


#define NO_vsnprintf
#endif
#if defined(__TURBOC__)
#define NO_vsnprintf
#endif
#if defined(WIN32)

#if !defined(vsnprintf) && !defined(NO_vsnprintf)
#if !defined(_MSC_VER) || ( defined(_MSC_VER) && _MSC_VER < 1500 )
#define vsnprintf _vsnprintf
#endif
#endif
#endif
#if defined(__SASC)
#define NO_vsnprintf
#endif
#if defined(VMS)
#define NO_vsnprintf
#endif
#if defined(__OS400__)
#define NO_vsnprintf
#endif
#if defined(__MVS__)
#define NO_vsnprintf
#endif
#endif





#if defined(_MSC_VER)
#define snprintf _snprintf
#endif

#if !defined(local)
#define local static
#endif



#if !defined(STDC)
extern voidp malloc OF((uInt size));
extern void free OF((voidpf ptr));
#endif


#if defined UNDER_CE
#include <windows.h>
#define zstrerror() gz_strwinerror((DWORD)GetLastError())
#else
#if !defined(NO_STRERROR)
#include <errno.h>
#define zstrerror() strerror(errno)
#else
#define zstrerror() "stdio error (consult errno)"
#endif
#endif


#if !defined(_LARGEFILE64_SOURCE) || _LFS64_LARGEFILE-0 == 0
ZEXTERN gzFile ZEXPORT gzopen64 OF((const char *, const char *));
ZEXTERN z_off64_t ZEXPORT gzseek64 OF((gzFile, z_off64_t, int));
ZEXTERN z_off64_t ZEXPORT gztell64 OF((gzFile));
ZEXTERN z_off64_t ZEXPORT gzoffset64 OF((gzFile));
#endif


#if MAX_MEM_LEVEL >= 8
#define DEF_MEM_LEVEL 8
#else
#define DEF_MEM_LEVEL MAX_MEM_LEVEL
#endif



#define GZBUFSIZE 8192


#define GZ_NONE 0
#define GZ_READ 7247
#define GZ_WRITE 31153
#define GZ_APPEND 1 


#define LOOK 0 
#define COPY 1 
#define GZIP 2 


typedef struct {

struct gzFile_s x; 




int mode; 
int fd; 
char *path; 
unsigned size; 
unsigned want; 
unsigned char *in; 
unsigned char *out; 
int direct; 

int how; 
z_off64_t start; 
int eof; 
int past; 

int level; 
int strategy; 

z_off64_t skip; 
int seek; 

int err; 
char *msg; 

z_stream strm; 
} gz_state;
typedef gz_state FAR *gz_statep;


void ZLIB_INTERNAL gz_error OF((gz_statep, int, const char *));
#if defined UNDER_CE
char ZLIB_INTERNAL *gz_strwinerror OF((DWORD error));
#endif




#if defined(INT_MAX)
#define GT_OFF(x) (sizeof(int) == sizeof(z_off64_t) && (x) > INT_MAX)
#else
unsigned ZLIB_INTERNAL gz_intmax OF((void));
#define GT_OFF(x) (sizeof(int) == sizeof(z_off64_t) && (x) > gz_intmax())
#endif
