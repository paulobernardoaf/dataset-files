




















#include "tool_setup.h"

#if defined(__SYMBIAN32__) || defined(NETWARE)

#if defined(NETWARE)
#if defined(__NOVELL_LIBC__)
#include <screen.h>
#else
#include <nwconio.h>
#endif
#endif

#include "tool_panykey.h"

#include "memdebug.h" 

void tool_pressanykey(void)
{
#if defined(__SYMBIAN32__)
getchar();
#elif defined(NETWARE)
pressanykey();
#endif
}

#endif 
