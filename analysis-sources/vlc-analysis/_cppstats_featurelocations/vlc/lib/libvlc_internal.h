






















#if !defined(_LIBVLC_INTERNAL_H)
#define _LIBVLC_INTERNAL_H 1

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc/libvlc.h>
#include <vlc/libvlc_dialog.h>
#include <vlc/libvlc_picture.h>
#include <vlc/libvlc_media.h>
#include <vlc/libvlc_events.h>

#include <vlc_common.h>








VLC_API libvlc_int_t *libvlc_InternalCreate( void );
VLC_API int libvlc_InternalInit( libvlc_int_t *, int, const char *ppsz_argv[] );
VLC_API void libvlc_InternalCleanup( libvlc_int_t * );
VLC_API void libvlc_InternalDestroy( libvlc_int_t * );

VLC_API int libvlc_InternalAddIntf( libvlc_int_t *, const char * );
VLC_API void libvlc_InternalPlay( libvlc_int_t * );
VLC_API void libvlc_InternalWait( libvlc_int_t * );
VLC_API void libvlc_SetExitHandler( libvlc_int_t *, void (*) (void *), void * );





struct libvlc_instance_t
{
libvlc_int_t *p_libvlc_int;
unsigned ref_count;
vlc_mutex_t instance_lock;
struct libvlc_callback_entry_list_t *p_callback_list;
struct
{
void (*cb) (void *, int, const libvlc_log_t *, const char *, va_list);
void *data;
} log;
struct
{
libvlc_dialog_cbs cbs;
void *data;
} dialog;
};

struct libvlc_event_manager_t
{
void * p_obj;
vlc_array_t listeners;
vlc_mutex_t lock;
};






void libvlc_threads_init (void);
void libvlc_threads_deinit (void);


void libvlc_event_manager_init(libvlc_event_manager_t *, void *);
void libvlc_event_manager_destroy(libvlc_event_manager_t *);

void libvlc_event_send(
libvlc_event_manager_t * p_em,
libvlc_event_t * p_event );

static inline libvlc_time_t from_mtime(vlc_tick_t time)
{
return (time + 500ULL)/ 1000ULL;
}

static inline vlc_tick_t to_mtime(libvlc_time_t time)
{
return VLC_TICK_FROM_MS(time);
}

#endif
