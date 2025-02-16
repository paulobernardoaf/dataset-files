#include <vlc_vout_window.h>
#include <vlc_vout_display.h>
typedef struct event_thread_t event_thread_t;
typedef struct {
bool is_projected;
unsigned width;
unsigned height;
} event_cfg_t;
typedef struct {
vout_window_t *parent_window;
HWND hparent;
HWND hvideownd;
} event_hwnd_t;
event_thread_t *EventThreadCreate( vlc_object_t *, vout_window_t *);
void EventThreadDestroy( event_thread_t * );
int EventThreadStart( event_thread_t *, event_hwnd_t *, const event_cfg_t * );
void EventThreadStop( event_thread_t * );
