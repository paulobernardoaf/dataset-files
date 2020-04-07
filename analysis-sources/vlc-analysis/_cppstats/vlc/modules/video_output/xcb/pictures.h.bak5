





















#if defined(WORDS_BIGENDIAN)
#define ORDER XCB_IMAGE_ORDER_MSB_FIRST
#else
#define ORDER XCB_IMAGE_ORDER_LSB_FIRST
#endif

#include <stdbool.h>
#include <stdint.h>
#include <xcb/xcb.h>
#include <vlc_picture.h>

const xcb_format_t *vlc_xcb_DepthToPixmapFormat(const xcb_setup_t *,
uint_fast8_t depth);
bool vlc_xcb_VisualToFormat(const xcb_setup_t *, uint_fast8_t depth,
const xcb_visualtype_t *, video_format_t *);

bool XCB_shm_Check (vlc_object_t *obj, xcb_connection_t *conn);
