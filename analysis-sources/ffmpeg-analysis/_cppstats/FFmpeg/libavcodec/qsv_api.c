#include "config.h"
#include <stddef.h>
#include "libavutil/mem.h"
#if CONFIG_QSV
#include "qsv.h"
AVQSVContext *av_qsv_alloc_context(void)
{
return av_mallocz(sizeof(AVQSVContext));
}
#else
struct AVQSVContext *av_qsv_alloc_context(void);
struct AVQSVContext *av_qsv_alloc_context(void)
{
return NULL;
}
#endif
