





















#if !defined(LIBVLC_VOUT_WRAPPER_H)
#define LIBVLC_VOUT_WRAPPER_H 1

#include <vlc_vout_display.h>



picture_pool_t *vout_GetPool(vout_display_t *vd, unsigned count);

bool vout_IsDisplayFiltered(vout_display_t *);
picture_t * vout_ConvertForDisplay(vout_display_t *, picture_t *);
void vout_FilterFlush(vout_display_t *);

void vout_SetDisplayFilled(vout_display_t *, bool is_filled);
void vout_SetDisplayZoom(vout_display_t *, unsigned num, unsigned den);
void vout_SetDisplayAspect(vout_display_t *, unsigned num, unsigned den);
void vout_SetDisplayCrop(vout_display_t *, unsigned num, unsigned den,
unsigned left, unsigned top, int right, int bottom);
void vout_SetDisplayViewpoint(vout_display_t *, const vlc_viewpoint_t *);

#endif 

