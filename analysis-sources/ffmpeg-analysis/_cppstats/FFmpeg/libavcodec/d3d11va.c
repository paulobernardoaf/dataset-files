#include <stddef.h>
#include "config.h"
#if CONFIG_D3D11VA
#include "libavutil/error.h"
#include "libavutil/mem.h"
#include "d3d11va.h"
AVD3D11VAContext *av_d3d11va_alloc_context(void)
{
AVD3D11VAContext* res = av_mallocz(sizeof(AVD3D11VAContext));
if (!res)
return NULL;
res->context_mutex = INVALID_HANDLE_VALUE;
return res;
}
#else
struct AVD3D11VAContext *av_d3d11va_alloc_context(void);
struct AVD3D11VAContext *av_d3d11va_alloc_context(void)
{
return NULL;
}
#endif 
