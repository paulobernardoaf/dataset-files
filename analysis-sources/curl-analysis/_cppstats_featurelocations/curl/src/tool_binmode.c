




















#include "tool_setup.h"

#if defined(HAVE_SETMODE)

#if defined(HAVE_IO_H)
#include <io.h>
#endif

#if defined(HAVE_FCNTL_H)
#include <fcntl.h>
#endif

#include "tool_binmode.h"

#include "memdebug.h" 

void set_binmode(FILE *stream)
{
#if defined(O_BINARY)
#if defined(__HIGHC__)
_setmode(stream, O_BINARY);
#else
(void)setmode(fileno(stream), O_BINARY);
#endif
#else
(void)stream;
#endif
}

#endif 
