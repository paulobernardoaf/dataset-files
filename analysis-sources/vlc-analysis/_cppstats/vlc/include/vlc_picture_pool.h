#include <vlc_picture.h>
typedef struct picture_pool_t picture_pool_t;
VLC_API picture_pool_t * picture_pool_New(unsigned count,
picture_t *const *tab) VLC_USED;
VLC_API picture_pool_t * picture_pool_NewFromFormat(const video_format_t *fmt,
unsigned count) VLC_USED;
VLC_API void picture_pool_Release( picture_pool_t * );
VLC_API picture_t * picture_pool_Get( picture_pool_t * ) VLC_USED;
VLC_API picture_t *picture_pool_Wait(picture_pool_t *) VLC_USED;
void picture_pool_Cancel( picture_pool_t *, bool canceled );
VLC_API picture_pool_t * picture_pool_Reserve(picture_pool_t *, unsigned count)
VLC_USED;
VLC_API unsigned picture_pool_GetSize(const picture_pool_t *);
