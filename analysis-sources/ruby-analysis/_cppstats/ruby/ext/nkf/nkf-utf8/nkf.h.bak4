





#if !defined(NKF_H)
#define NKF_H



#if !defined(MIME_DECODE_DEFAULT)
#define MIME_DECODE_DEFAULT STRICT_MIME
#endif
#if !defined(X0201_DEFAULT)
#define X0201_DEFAULT TRUE
#endif

#if DEFAULT_NEWLINE == 0x0D0A
#elif DEFAULT_NEWLINE == 0x0D
#else
#define DEFAULT_NEWLINE 0x0A
#endif
#if defined(HELP_OUTPUT_STDERR)
#define HELP_OUTPUT stderr
#else
#define HELP_OUTPUT stdout
#endif




#if defined(nkf_char)
#elif defined(INT_IS_SHORT)
typedef long nkf_char;
#define NKF_INT32_C(n) (n##L)
#else
typedef int nkf_char;
#define NKF_INT32_C(n) (n)
#endif

#if (defined(__TURBOC__) || defined(_MSC_VER) || defined(LSI_C) || (defined(__WATCOMC__) && defined(__386__) && !defined(__LINUX__)) || defined(__MINGW32__) || defined(__EMX__) || defined(__MSDOS__) || defined(__WINDOWS__) || defined(__DOS__) || defined(__OS2__)) && !defined(MSDOS)
#define MSDOS
#if (defined(__Win32__) || defined(_WIN32)) && !defined(__WIN32__)
#define __WIN32__
#endif
#endif

#if defined(PERL_XS)
#undef OVERWRITE
#endif

#if !defined(PERL_XS)
#include <stdio.h>
#endif

#include <stdlib.h>
#include <string.h>

#if defined(MSDOS) || defined(__OS2__)
#include <fcntl.h>
#include <io.h>
#if defined(_MSC_VER) || defined(__WATCOMC__)
#define mktemp _mktemp
#endif
#endif

#if defined(MSDOS)
#if defined(LSI_C)
#define setbinmode(fp) fsetbin(fp)
#elif defined(__DJGPP__)
#include <libc/dosio.h>
void setbinmode(FILE *fp)
{

int fd, m;
fd = fileno(fp);
m = (__file_handle_modes[fd] & (~O_TEXT)) | O_BINARY;
__file_handle_set(fd, m);
}
#else 
#define setbinmode(fp) setmode(fileno(fp), O_BINARY)
#endif
#else 
#define setbinmode(fp) (void)(fp)
#endif

#if defined(_IOFBF)
#define setvbuffer(fp, buf, size) setvbuf(fp, buf, _IOFBF, size)
#else 
#define setvbuffer(fp, buf, size) setbuffer(fp, buf, size)
#endif


#if defined(__TURBOC__) && defined(_Windows) && !defined(__WIN32__) 
#define EASYWIN
#if !defined(__WIN16__)
#define __WIN16__
#endif
#include <windows.h>
#endif

#if defined(OVERWRITE)

#if defined(__EMX__)
#include <sys/types.h>
#endif
#include <sys/stat.h>
#if !defined(MSDOS) || defined(__DJGPP__) 
#include <unistd.h>
#if defined(__WATCOMC__)
#include <sys/utime.h>
#else
#include <utime.h>
#endif
#else 
#if defined(__WIN32__)
#if defined(__BORLANDC__)
#include <utime.h>
#else 
#include <sys/utime.h>
#endif 
#else 
#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__WATCOMC__) || defined(__OS2__) || defined(__EMX__) || defined(__IBMC__) || defined(__IBMCPP__) 
#include <sys/utime.h>
#elif defined(__TURBOC__) 
#include <utime.h>
#elif defined(LSI_C) 
#endif 
#endif
#endif
#endif

#if !defined(DEFAULT_CODE_JIS) && !defined(DEFAULT_CODE_SJIS) && !defined(DEFAULT_CODE_WINDOWS_31J) && !defined(DEFAULT_CODE_EUC) && !defined(DEFAULT_CODE_UTF8) && !defined(DEFAULT_CODE_LOCALE)


#define DEFAULT_CODE_LOCALE
#endif

#if defined(DEFAULT_CODE_LOCALE)

#if defined(__WIN32__) 
#if !defined(HAVE_LOCALE_H)
#define HAVE_LOCALE_H
#endif
#elif defined(__OS2__)
#undef HAVE_LANGINFO_H 
#if !defined(HAVE_LOCALE_H)
#define HAVE_LOCALE_H
#endif
#elif defined(MSDOS)
#if !defined(HAVE_LOCALE_H)
#define HAVE_LOCALE_H
#endif
#elif defined(__BIONIC__) 
#else
#if !defined(HAVE_LANGINFO_H)
#define HAVE_LANGINFO_H
#endif
#if !defined(HAVE_LOCALE_H)
#define HAVE_LOCALE_H
#endif
#endif

#if defined(HAVE_LANGINFO_H)
#include <langinfo.h>
#endif
#if defined(HAVE_LOCALE_H)
#include <locale.h>
#endif

#endif 

#define FALSE 0
#define TRUE 1

#if !defined(ARG_UNUSED)
#if defined(__GNUC__)
#define ARG_UNUSED __attribute__ ((unused))
#else
#define ARG_UNUSED
#endif
#endif

#if defined(WIN32DLL)
#include "nkf32.h"
#endif

#endif 
