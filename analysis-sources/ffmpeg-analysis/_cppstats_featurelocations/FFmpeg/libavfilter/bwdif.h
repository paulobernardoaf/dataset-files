

















#if !defined(AVFILTER_BWDIF_H)
#define AVFILTER_BWDIF_H

#include "libavutil/pixdesc.h"
#include "avfilter.h"
#include "yadif.h"

typedef struct BWDIFContext {
YADIFContext yadif;

void (*filter_intra)(void *dst1, void *cur1, int w, int prefs, int mrefs,
int prefs3, int mrefs3, int parity, int clip_max);
void (*filter_line)(void *dst, void *prev, void *cur, void *next,
int w, int prefs, int mrefs, int prefs2, int mrefs2,
int prefs3, int mrefs3, int prefs4, int mrefs4,
int parity, int clip_max);
void (*filter_edge)(void *dst, void *prev, void *cur, void *next,
int w, int prefs, int mrefs, int prefs2, int mrefs2,
int parity, int clip_max, int spat);
} BWDIFContext;

void ff_bwdif_init_x86(BWDIFContext *bwdif);

#endif 
