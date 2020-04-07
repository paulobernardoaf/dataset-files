#if defined(_MERGE_PROXYSTUB)
#define REGISTER_PROXY_DLL 
#define _WIN32_WINNT 0x0500 
#define USE_STUBLESS_PROXY 
#pragma comment(lib, "rpcns4.lib")
#pragma comment(lib, "rpcrt4.lib")
#define ENTRY_PREFIX Prx
#include "dlldata.c"
#include "seg_service_p.c"
#endif 
