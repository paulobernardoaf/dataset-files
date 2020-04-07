#include <vlc_viewpoint.h>
enum {
VOUT_CONTROL_CHANGE_FILTERS, 
VOUT_CONTROL_CHANGE_INTERLACE, 
VOUT_CONTROL_MOUSE_STATE, 
VOUT_CONTROL_VIEWPOINT, 
};
typedef struct {
int type;
union {
bool boolean;
char *string;
struct {
int channel;
float value;
} spu_rate;
struct {
int channel;
vlc_tick_t value;
} spu_delay;
vlc_mouse_t mouse;
vlc_viewpoint_t viewpoint;
};
} vout_control_cmd_t;
void vout_control_cmd_Init(vout_control_cmd_t *, int type);
void vout_control_cmd_Clean(vout_control_cmd_t *);
typedef struct {
vlc_mutex_t lock;
vlc_cond_t wait_request;
vlc_cond_t wait_available;
bool is_dead;
bool can_sleep;
bool is_waiting;
bool is_held;
DECL_ARRAY(vout_control_cmd_t) cmd;
} vout_control_t;
void vout_control_Init(vout_control_t *);
void vout_control_Clean(vout_control_t *);
void vout_control_WaitEmpty(vout_control_t *);
void vout_control_Push(vout_control_t *, vout_control_cmd_t *);
void vout_control_PushVoid(vout_control_t *, int type);
void vout_control_PushBool(vout_control_t *, int type, bool boolean);
void vout_control_PushString(vout_control_t *, int type, const char *string);
void vout_control_Wake(vout_control_t *);
void vout_control_Hold(vout_control_t *);
void vout_control_Release(vout_control_t *);
int vout_control_Pop(vout_control_t *, vout_control_cmd_t *, vlc_tick_t deadline);
void vout_control_Dead(vout_control_t *);
