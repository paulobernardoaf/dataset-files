



















#if !defined(AVCODEC_HTMLSUBTITLES_H)
#define AVCODEC_HTMLSUBTITLES_H

#include "libavutil/bprint.h"

int ff_htmlmarkup_to_ass(void *log_ctx, AVBPrint *dst, const char *in);

#endif 
