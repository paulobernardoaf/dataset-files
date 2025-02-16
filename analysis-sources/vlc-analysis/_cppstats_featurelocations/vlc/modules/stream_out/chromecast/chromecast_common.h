























#if !defined(VLC_CHROMECAST_COMMON_H)
#define VLC_CHROMECAST_COMMON_H

#include <vlc_input.h>

#if defined(__cplusplus)
extern "C" {
#endif

#define CC_SHARED_VAR_NAME "cc_sout"

#define CC_PACE_ERR (-2)
#define CC_PACE_ERR_RETRY (-1)
#define CC_PACE_OK (0)
#define CC_PACE_OK_WAIT (1)
#define CC_PACE_OK_ENDED (2)

enum cc_input_event
{
CC_INPUT_EVENT_EOF,
CC_INPUT_EVENT_RETRY,
};

union cc_input_arg
{
bool eof;
};

typedef void (*on_input_event_itf)( void *data, enum cc_input_event, union cc_input_arg );

typedef void (*on_paused_changed_itf)( void *data, bool );

typedef struct
{
void *p_opaque;

void (*pf_set_demux_enabled)(void *, bool enabled, on_paused_changed_itf, void *);

vlc_tick_t (*pf_get_time)(void*);

int (*pf_pace)(void*);

void (*pf_send_input_event)(void*, enum cc_input_event, union cc_input_arg);

void (*pf_set_pause_state)(void*, bool paused);

void (*pf_set_meta)(void*, vlc_meta_t *p_meta);

} chromecast_common;

#if defined(__cplusplus)
}
#endif

#endif 

