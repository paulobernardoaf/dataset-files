#if !defined(HEADER_CURL_TOOL_BINMODE_H)
#define HEADER_CURL_TOOL_BINMODE_H





















#include "tool_setup.h"

#if defined(HAVE_SETMODE)

void set_binmode(FILE *stream);

#else

#define set_binmode(x) Curl_nop_stmt

#endif 

#endif 
