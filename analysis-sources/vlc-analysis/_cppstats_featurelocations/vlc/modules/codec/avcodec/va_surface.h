

























#if !defined(AVCODEC_VA_SURFACE_INTERNAL_H)
#define AVCODEC_VA_SURFACE_INTERNAL_H

#include <libavcodec/avcodec.h>
#include "va.h"


typedef struct vlc_va_surface_t vlc_va_surface_t;
typedef struct va_pool_t va_pool_t;

#define MAX_SURFACE_COUNT (64)

struct va_pool_cfg {
int (*pf_create_device)(vlc_va_t *);



void (*pf_destroy_device)(void *opaque);




int (*pf_create_decoder_surfaces)(vlc_va_t *, int codec_id,
const video_format_t *fmt,
size_t surface_count);



void (*pf_setup_avcodec_ctx)(void *opaque, AVCodecContext *avctx);

void *opaque;
};







va_pool_t * va_pool_Create(vlc_va_t *, const struct va_pool_cfg *);









void va_pool_Close(va_pool_t *);







int va_pool_SetupDecoder(vlc_va_t *, va_pool_t *, AVCodecContext *, const video_format_t *, size_t count);




vlc_va_surface_t *va_pool_Get(va_pool_t *);




size_t va_surface_GetIndex(const vlc_va_surface_t *surface);




void va_surface_AddRef(vlc_va_surface_t *surface);









void va_surface_Release(vlc_va_surface_t *surface);

#endif 
