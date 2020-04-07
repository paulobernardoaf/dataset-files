
























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif


#if !defined(EAI_BADFLAGS)
#define EAI_BADFLAGS -1
#endif
#if !defined(EAI_NONAME)
#define EAI_NONAME -2
#endif
#if !defined(EAI_AGAIN)
#define EAI_AGAIN -3
#endif
#if !defined(EAI_FAIL)
#define EAI_FAIL -4
#endif
#if !defined(EAI_NODATA)
#define EAI_NODATA -5
#endif
#if !defined(EAI_FAMILY)
#define EAI_FAMILY -6
#endif
#if !defined(EAI_SOCKTYPE)
#define EAI_SOCKTYPE -7
#endif
#if !defined(EAI_SERVICE)
#define EAI_SERVICE -8
#endif
#if !defined(EAI_ADDRFAMILY)
#define EAI_ADDRFAMILY -9
#endif
#if !defined(EAI_MEMORY)
#define EAI_MEMORY -10
#endif
#if !defined(EAI_OVERFLOW)
#define EAI_OVERFLOW -11
#endif
#if !defined(EAI_SYSTEM)
#define EAI_SYSTEM -12
#endif

static const struct
{
int code;
const char msg[41];
} gai_errlist[] =
{
{ 0, "Error 0" },
{ EAI_BADFLAGS, "Invalid flag used" },
{ EAI_NONAME, "Host or service not found" },
{ EAI_AGAIN, "Temporary name service failure" },
{ EAI_FAIL, "Non-recoverable name service failure" },
{ EAI_NODATA, "No data for host name" },
{ EAI_FAMILY, "Unsupported address family" },
{ EAI_SOCKTYPE, "Unsupported socket type" },
{ EAI_SERVICE, "Incompatible service for socket type" },
{ EAI_ADDRFAMILY, "Unavailable address family for host name" },
{ EAI_MEMORY, "Memory allocation failure" },
{ EAI_OVERFLOW, "Buffer overflow" },
{ EAI_SYSTEM, "System error" },
{ 0, "" },
};

static const char gai_unknownerr[] = "Unrecognized error number";




const char *gai_strerror (int errnum)
{
for (unsigned i = 0; *gai_errlist[i].msg; i++)
if (errnum == gai_errlist[i].code)
return gai_errlist[i].msg;

return gai_unknownerr;
}
