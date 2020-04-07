#include <vlc_vout_display.h>
int vlc_xcb_error_Check(vout_display_t *, xcb_connection_t *conn,
const char *str, xcb_void_cookie_t);
int vlc_xcb_parent_Create(vout_display_t *obj, const vout_window_t *wnd,
xcb_connection_t **connp,
const xcb_screen_t **screenp);
int vlc_xcb_Manage(vout_display_t *vd, xcb_connection_t *conn);
