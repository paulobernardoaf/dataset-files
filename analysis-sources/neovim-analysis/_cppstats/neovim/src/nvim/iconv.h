#include "auto/config.h"
#if defined(HAVE_ICONV)
#include <errno.h>
#include <iconv.h>
#if !defined(EILSEQ)
#define EILSEQ 123
#endif
#define ICONV_ERRNO errno
#define ICONV_E2BIG E2BIG
#define ICONV_EINVAL EINVAL
#define ICONV_EILSEQ EILSEQ
#endif
