

















#include <stdint.h>

struct vout_window_t;
struct wl_registry;
struct wl_surface;
struct wl_list;

int seat_create(struct vout_window_t *wnd, struct wl_registry *,
uint32_t name, uint32_t version, struct wl_list *list);
int seat_destroy_one(struct wl_list *list, uint32_t name);
void seat_destroy_all(struct wl_list *list);

int seat_next_timeout(const struct wl_list *list);
void seat_timeout(struct wl_list *list);

struct wl_surface *window_get_cursor(vout_window_t *wnd, int32_t *hotspot_x,
int32_t *hotspot_y);
