

















#include <stdint.h>

struct vout_window_t;
struct wl_registry;
struct output_list;

struct output_list *output_list_create(struct vout_window_t *wnd);
int output_create(struct output_list *, struct wl_registry *,
uint32_t name, uint32_t version);
int output_destroy_by_name(struct output_list *, uint32_t name);
void output_list_destroy(struct output_list *);
