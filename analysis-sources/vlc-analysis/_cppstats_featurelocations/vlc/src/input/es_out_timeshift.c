
























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#if defined (_WIN32)
#include <direct.h>
#endif
#include <sys/stat.h>
#include <unistd.h>

#include <vlc_common.h>
#include <vlc_fs.h>
#include <vlc_mouse.h>
#if defined(_WIN32)
#include <vlc_charset.h>
#endif
#include <vlc_es_out.h>
#include <vlc_block.h>
#include "input_internal.h"
#include "es_out.h"






#if defined(HAVE_ATTRIBUTE_PACKED)
#define attribute_packed __attribute__((__packed__))
#else
#define attribute_packed
#endif

enum
{
C_ADD,
C_SEND,
C_DEL,
C_CONTROL,
};

typedef struct attribute_packed
{
input_source_t *in;
es_out_id_t *p_es;
es_format_t *p_fmt;
} ts_cmd_add_t;

typedef struct attribute_packed
{
es_out_id_t *p_es;
} ts_cmd_del_t;

typedef struct attribute_packed
{
es_out_id_t *p_es;
block_t *p_block;
int i_offset; 
} ts_cmd_send_t;

typedef struct attribute_packed
{
input_source_t *in;
int i_query;

union
{
bool b_bool;
int i_int;
int64_t i_i64;
es_out_id_t *p_es;
struct
{
int i_int;
int64_t i_i64;
} int_i64;
struct
{
int i_int;
vlc_meta_t *p_meta;
} int_meta;
struct
{
int i_int;
vlc_epg_t *p_epg;
} int_epg;
struct
{
int i_int;
vlc_epg_event_t *p_evt;
} int_epg_evt;
struct
{
es_out_id_t *p_es;
bool b_bool;
} es_bool;
struct
{
es_out_id_t *p_es;
es_format_t *p_fmt;
} es_fmt;
struct
{
int i_cat;
int i_policy;
} es_policy;
} u;
} ts_cmd_control_t;

typedef struct attribute_packed
{
int i_query;

union
{
int i_int;
struct
{
vlc_tick_t i_pts_delay;
vlc_tick_t i_pts_jitter;
int i_cr_average;
} jitter;
struct
{
double f_position;
vlc_tick_t i_time;
vlc_tick_t i_normal_time;
vlc_tick_t i_length;
} times;
} u;
} ts_cmd_privcontrol_t;

typedef struct attribute_packed
{
int8_t i_type;
vlc_tick_t i_date;
union
{
ts_cmd_add_t add;
ts_cmd_del_t del;
ts_cmd_send_t send;
ts_cmd_control_t control;
ts_cmd_privcontrol_t privcontrol;
} u;
} ts_cmd_t;

typedef struct ts_storage_t ts_storage_t;
struct ts_storage_t
{
ts_storage_t *p_next;


#if defined(_WIN32)
char *psz_file; 
#endif
size_t i_file_max; 
int64_t i_file_size;
FILE *p_filew; 
FILE *p_filer; 


int i_cmd_r;
int i_cmd_w;
int i_cmd_max;
ts_cmd_t *p_cmd;
};

typedef struct
{
vlc_thread_t thread;
input_thread_t *p_input;
es_out_t *p_out;
int64_t i_tmp_size_max;
const char *psz_tmp_path;


vlc_mutex_t lock;
vlc_cond_t wait;


bool b_paused;
vlc_tick_t i_pause_date;


float rate;
float rate_source;
vlc_tick_t i_rate_date;
vlc_tick_t i_rate_delay;


vlc_tick_t i_buffering_delay;


ts_storage_t *p_storage_r;
ts_storage_t *p_storage_w;

vlc_tick_t i_cmd_delay;

} ts_thread_t;

struct es_out_id_t
{
es_out_id_t *p_es;
};

typedef struct
{
input_thread_t *p_input;
es_out_t *p_out;


int64_t i_tmp_size_max; 
char *psz_tmp_path; 


vlc_mutex_t lock;


bool b_delayed;
ts_thread_t *p_ts;


bool b_input_paused;
bool b_input_paused_source;
float input_rate;
float input_rate_source;


int i_es;
es_out_id_t **pp_es;

es_out_t out;
} es_out_sys_t;

static void Del ( es_out_t *, es_out_id_t * );

static int TsStart( es_out_t * );
static void TsAutoStop( es_out_t * );

static void TsStop( ts_thread_t * );
static void TsPushCmd( ts_thread_t *, ts_cmd_t * );
static int TsPopCmdLocked( ts_thread_t *, ts_cmd_t *, bool b_flush );
static bool TsHasCmd( ts_thread_t * );
static bool TsIsUnused( ts_thread_t * );
static int TsChangePause( ts_thread_t *, bool b_source_paused, bool b_paused, vlc_tick_t i_date );
static int TsChangeRate( ts_thread_t *, float src_rate, float rate );

static void *TsRun( void * );

static ts_storage_t *TsStorageNew( const char *psz_path, int64_t i_tmp_size_max );
static void TsStorageDelete( ts_storage_t * );
static void TsStoragePack( ts_storage_t *p_storage );
static bool TsStorageIsFull( ts_storage_t *, const ts_cmd_t *p_cmd );
static bool TsStorageIsEmpty( ts_storage_t * );
static void TsStoragePushCmd( ts_storage_t *, const ts_cmd_t *p_cmd, bool b_flush );
static void TsStoragePopCmd( ts_storage_t *p_storage, ts_cmd_t *p_cmd, bool b_flush );

static void CmdClean( ts_cmd_t * );
static void cmd_cleanup_routine( void *p ) { CmdClean( p ); }

static int CmdInitAdd ( ts_cmd_t *, input_source_t *, es_out_id_t *, const es_format_t *, bool b_copy );
static void CmdInitSend ( ts_cmd_t *, es_out_id_t *, block_t * );
static int CmdInitDel ( ts_cmd_t *, es_out_id_t * );
static int CmdInitControl( ts_cmd_t *, input_source_t *, int i_query, va_list, bool b_copy );
static int CmdInitPrivControl( ts_cmd_t *, int i_query, va_list, bool b_copy );


static void CmdCleanAdd ( ts_cmd_t * );
static void CmdCleanSend ( ts_cmd_t * );
static void CmdCleanControl( ts_cmd_t *p_cmd );


static void CmdExecuteAdd ( es_out_t *, ts_cmd_t * );
static int CmdExecuteSend ( es_out_t *, ts_cmd_t * );
static void CmdExecuteDel ( es_out_t *, ts_cmd_t * );
static int CmdExecuteControl( es_out_t *, ts_cmd_t * );
static int CmdExecutePrivControl( es_out_t *, ts_cmd_t * );


static int GetTmpFile( char **ppsz_file, const char *psz_path );

static const struct es_out_callbacks es_out_timeshift_cbs;




es_out_t *input_EsOutTimeshiftNew( input_thread_t *p_input, es_out_t *p_next_out, float rate )
{
es_out_sys_t *p_sys = malloc( sizeof(*p_sys) );
if( !p_sys )
return NULL;

p_sys->out.cbs = &es_out_timeshift_cbs;


p_sys->b_input_paused = false;
p_sys->b_input_paused_source = false;
p_sys->p_input = p_input;
p_sys->input_rate = rate;
p_sys->input_rate_source = rate;

p_sys->p_out = p_next_out;
vlc_mutex_init_recursive( &p_sys->lock );

p_sys->b_delayed = false;
p_sys->p_ts = NULL;

TAB_INIT( p_sys->i_es, p_sys->pp_es );


const int i_tmp_size_max = var_CreateGetInteger( p_input, "input-timeshift-granularity" );
if( i_tmp_size_max < 0 )
p_sys->i_tmp_size_max = 50*1024*1024;
else
p_sys->i_tmp_size_max = __MAX( i_tmp_size_max, 1*1024*1024 );
msg_Dbg( p_input, "using timeshift granularity of %d MiB",
(int)p_sys->i_tmp_size_max/(1024*1024) );

p_sys->psz_tmp_path = var_InheritString( p_input, "input-timeshift-path" );
#if defined (_WIN32) && !VLC_WINSTORE_APP
if( p_sys->psz_tmp_path == NULL )
{
const DWORD count = GetTempPath( 0, NULL );
if( count > 0 )
{
WCHAR *path = vlc_alloc( count + 1, sizeof(WCHAR) );
if( path != NULL )
{
DWORD ret = GetTempPath( count + 1, path );
if( ret != 0 && ret <= count )
p_sys->psz_tmp_path = FromWide( path );
free( path );
}
}
}
if( p_sys->psz_tmp_path == NULL )
{
wchar_t *wpath = _wgetcwd( NULL, 0 );
if( wpath != NULL )
{
p_sys->psz_tmp_path = FromWide( wpath );
free( wpath );
}
}
if( p_sys->psz_tmp_path == NULL )
p_sys->psz_tmp_path = strdup( "C:" );

if( p_sys->psz_tmp_path != NULL )
{
size_t len = strlen( p_sys->psz_tmp_path );

while( len > 0 && p_sys->psz_tmp_path[len - 1] == DIR_SEP_CHAR )
len--;

p_sys->psz_tmp_path[len] = '\0';
}
#endif
if( p_sys->psz_tmp_path != NULL )
msg_Dbg( p_input, "using timeshift path: %s", p_sys->psz_tmp_path );
else
msg_Dbg( p_input, "using default timeshift path" );

#if 0
#define S(t) msg_Err( p_input, "SIZEOF("#t")=%d", sizeof(t) )
S(ts_cmd_t);
S(ts_cmd_control_t);
S(ts_cmd_send_t);
S(ts_cmd_del_t);
S(ts_cmd_add_t);
#undef S
#endif

return &p_sys->out;
}




static void Destroy( es_out_t *p_out )
{
es_out_sys_t *p_sys = container_of(p_out, es_out_sys_t, out);

if( p_sys->b_delayed )
{
TsStop( p_sys->p_ts );
p_sys->b_delayed = false;
}

while( p_sys->i_es > 0 )
Del( p_out, p_sys->pp_es[0] );
TAB_CLEAN( p_sys->i_es, p_sys->pp_es );

free( p_sys->psz_tmp_path );
free( p_sys );
}

static es_out_id_t *Add( es_out_t *p_out, input_source_t *in, const es_format_t *p_fmt )
{
es_out_sys_t *p_sys = container_of(p_out, es_out_sys_t, out);
ts_cmd_t cmd;

es_out_id_t *p_es = malloc( sizeof( *p_es ) );
if( !p_es )
return NULL;

vlc_mutex_lock( &p_sys->lock );

TsAutoStop( p_out );

if( CmdInitAdd( &cmd, in, p_es, p_fmt, p_sys->b_delayed ) )
{
vlc_mutex_unlock( &p_sys->lock );
free( p_es );
return NULL;
}

TAB_APPEND( p_sys->i_es, p_sys->pp_es, p_es );

if( p_sys->b_delayed )
TsPushCmd( p_sys->p_ts, &cmd );
else
CmdExecuteAdd( p_sys->p_out, &cmd );

vlc_mutex_unlock( &p_sys->lock );

return p_es;
}
static int Send( es_out_t *p_out, es_out_id_t *p_es, block_t *p_block )
{
es_out_sys_t *p_sys = container_of(p_out, es_out_sys_t, out);
ts_cmd_t cmd;
int i_ret = VLC_SUCCESS;

vlc_mutex_lock( &p_sys->lock );

TsAutoStop( p_out );

CmdInitSend( &cmd, p_es, p_block );
if( p_sys->b_delayed )
TsPushCmd( p_sys->p_ts, &cmd );
else
i_ret = CmdExecuteSend( p_sys->p_out, &cmd) ;

vlc_mutex_unlock( &p_sys->lock );

return i_ret;
}
static void Del( es_out_t *p_out, es_out_id_t *p_es )
{
es_out_sys_t *p_sys = container_of(p_out, es_out_sys_t, out);
ts_cmd_t cmd;

vlc_mutex_lock( &p_sys->lock );

TsAutoStop( p_out );

CmdInitDel( &cmd, p_es );
if( p_sys->b_delayed )
TsPushCmd( p_sys->p_ts, &cmd );
else
CmdExecuteDel( p_sys->p_out, &cmd );

TAB_REMOVE( p_sys->i_es, p_sys->pp_es, p_es );

vlc_mutex_unlock( &p_sys->lock );
}

static inline int es_out_in_vaControl( es_out_t *p_out, input_source_t *in,
int i_query, va_list args)
{
return p_out->cbs->control( p_out, in, i_query, args );
}

static inline int es_out_in_Control( es_out_t *p_out, input_source_t *in,
int i_query, ... )
{
va_list args;
int i_result;

va_start( args, i_query );
i_result = es_out_in_vaControl( p_out, in, i_query, args );
va_end( args );
return i_result;
}

static int ControlLockedGetEmpty( es_out_t *p_out, input_source_t *in,
bool *pb_empty )
{
es_out_sys_t *p_sys = container_of(p_out, es_out_sys_t, out);

if( p_sys->b_delayed && TsHasCmd( p_sys->p_ts ) )
*pb_empty = false;
else
{
int ret = es_out_in_Control( p_sys->p_out, in, ES_OUT_GET_EMPTY, pb_empty );
assert( ret == VLC_SUCCESS );
}

return VLC_SUCCESS;
}
static int ControlLockedGetWakeup( es_out_t *p_out, vlc_tick_t *pi_wakeup )
{
es_out_sys_t *p_sys = container_of(p_out, es_out_sys_t, out);

if( p_sys->b_delayed )
{
assert( !input_priv(p_sys->p_input)->b_can_pace_control );
*pi_wakeup = 0;
}
else
{
*pi_wakeup = es_out_GetWakeup( p_sys->p_out );
}

return VLC_SUCCESS;
}
static int ControlLockedGetBuffering( es_out_t *p_out, bool *pb_buffering )
{
es_out_sys_t *p_sys = container_of(p_out, es_out_sys_t, out);

if( p_sys->b_delayed )
*pb_buffering = true;
else
*pb_buffering = es_out_GetBuffering( p_sys->p_out );

return VLC_SUCCESS;
}
static int ControlLockedSetPauseState( es_out_t *p_out, bool b_source_paused, bool b_paused, vlc_tick_t i_date )
{
es_out_sys_t *p_sys = container_of(p_out, es_out_sys_t, out);
int i_ret;

if( !p_sys->b_delayed && !b_source_paused == !b_paused )
{
i_ret = es_out_SetPauseState( p_sys->p_out, b_source_paused, b_paused, i_date );
}
else
{
i_ret = VLC_EGENERIC;
if( !input_priv(p_sys->p_input)->b_can_pace_control )
{
if( !p_sys->b_delayed )
TsStart( p_out );
if( p_sys->b_delayed )
i_ret = TsChangePause( p_sys->p_ts, b_source_paused, b_paused, i_date );
}
else
{



msg_Err( p_sys->p_input, "EsOutTimeshift does not work with streams that have pace control" );
}
}

if( !i_ret )
{
p_sys->b_input_paused_source = b_source_paused;
p_sys->b_input_paused = b_paused;
}
return i_ret;
}
static int ControlLockedSetRate( es_out_t *p_out, float src_rate, float rate )
{
es_out_sys_t *p_sys = container_of(p_out, es_out_sys_t, out);
int i_ret;

if( !p_sys->b_delayed && src_rate == rate )
{
i_ret = es_out_SetRate( p_sys->p_out, src_rate, rate );
}
else
{
i_ret = VLC_EGENERIC;
if( !input_priv(p_sys->p_input)->b_can_pace_control )
{
if( !p_sys->b_delayed )
TsStart( p_out );
if( p_sys->b_delayed )
i_ret = TsChangeRate( p_sys->p_ts, src_rate, rate );
}
else
{



msg_Err( p_sys->p_input, "EsOutTimeshift does not work with streams that have pace control" );
}

}

if( !i_ret )
{
p_sys->input_rate_source = src_rate;
p_sys->input_rate = rate;
}
return i_ret;
}
static int ControlLockedSetFrameNext( es_out_t *p_out )
{
es_out_sys_t *p_sys = container_of(p_out, es_out_sys_t, out);

return es_out_SetFrameNext( p_sys->p_out );
}

static int ControlLocked( es_out_t *p_out, input_source_t *in, int i_query,
va_list args )
{
es_out_sys_t *p_sys = container_of(p_out, es_out_sys_t, out);

switch( i_query )
{

case ES_OUT_SET_GROUP:
case ES_OUT_SET_PCR:
case ES_OUT_SET_GROUP_PCR:
case ES_OUT_RESET_PCR:
case ES_OUT_SET_NEXT_DISPLAY_TIME:
case ES_OUT_SET_GROUP_META:
case ES_OUT_SET_GROUP_EPG:
case ES_OUT_SET_GROUP_EPG_EVENT:
case ES_OUT_SET_EPG_TIME:
case ES_OUT_SET_ES_SCRAMBLED_STATE:
case ES_OUT_DEL_GROUP:
case ES_OUT_SET_META:
case ES_OUT_SET_ES:
case ES_OUT_UNSET_ES:
case ES_OUT_RESTART_ES:
case ES_OUT_SET_ES_DEFAULT:
case ES_OUT_SET_ES_STATE:
case ES_OUT_SET_ES_CAT_POLICY:
case ES_OUT_SET_ES_FMT:
{
ts_cmd_t cmd;
if( CmdInitControl( &cmd, in, i_query, args, p_sys->b_delayed ) )
return VLC_EGENERIC;
if( p_sys->b_delayed )
{
TsPushCmd( p_sys->p_ts, &cmd );
return VLC_SUCCESS;
}
return CmdExecuteControl( p_sys->p_out, &cmd );
}


case ES_OUT_GET_ES_STATE:
{
es_out_id_t *p_es = va_arg( args, es_out_id_t * );
bool *pb_enabled = va_arg( args, bool* );

if( p_sys->b_delayed )
{
*pb_enabled = true;
return VLC_SUCCESS;
}
return es_out_in_Control( p_sys->p_out, in, ES_OUT_GET_ES_STATE,
p_es->p_es, pb_enabled );
}
case ES_OUT_VOUT_SET_MOUSE_EVENT:
{
es_out_id_t *p_es = va_arg( args, es_out_id_t * );
vlc_mouse_event cb = va_arg( args, vlc_mouse_event );
void *user_data = va_arg( args, void * );
return es_out_in_Control( p_sys->p_out, in, ES_OUT_VOUT_SET_MOUSE_EVENT,
p_es->p_es, cb, user_data );
}
case ES_OUT_VOUT_ADD_OVERLAY:
{
es_out_id_t *p_es = va_arg( args, es_out_id_t * );
subpicture_t *sub = va_arg( args, subpicture_t * );
size_t *channel = va_arg( args, size_t * );
return es_out_in_Control( p_sys->p_out, in, ES_OUT_VOUT_ADD_OVERLAY,
p_es->p_es, sub, channel );
}
case ES_OUT_VOUT_DEL_OVERLAY:
{
es_out_id_t *p_es = va_arg( args, es_out_id_t * );
size_t channel = va_arg( args, size_t );
return es_out_in_Control( p_sys->p_out, in, ES_OUT_VOUT_DEL_OVERLAY,
p_es->p_es, channel );
}
case ES_OUT_SPU_SET_HIGHLIGHT:
{
es_out_id_t *p_es = va_arg( args, es_out_id_t * );
const vlc_spu_highlight_t *p_hl = va_arg( args, const vlc_spu_highlight_t * );
return es_out_in_Control( p_sys->p_out, in, ES_OUT_SPU_SET_HIGHLIGHT,
p_es->p_es, p_hl );
}

case ES_OUT_GET_EMPTY:
{
bool *pb_empty = va_arg( args, bool* );
return ControlLockedGetEmpty( p_out, in, pb_empty );
}

case ES_OUT_GET_PCR_SYSTEM:
if( p_sys->b_delayed )
return VLC_EGENERIC;

case ES_OUT_POST_SUBNODE:
return es_out_in_vaControl( p_sys->p_out, in, i_query, args );

case ES_OUT_MODIFY_PCR_SYSTEM:
{
const bool b_absolute = va_arg( args, int );
const vlc_tick_t i_system = va_arg( args, vlc_tick_t );

if( b_absolute && p_sys->b_delayed )
return VLC_EGENERIC;

return es_out_in_Control( p_sys->p_out, in, i_query, b_absolute,
i_system );
}

default:
vlc_assert_unreachable();
return VLC_EGENERIC;
}
}

static int Control( es_out_t *p_out, input_source_t *in, int i_query, va_list args )
{
es_out_sys_t *p_sys = container_of(p_out, es_out_sys_t, out);
int i_ret;

vlc_mutex_lock( &p_sys->lock );

TsAutoStop( p_out );

i_ret = ControlLocked( p_out, in, i_query, args );

vlc_mutex_unlock( &p_sys->lock );

return i_ret;
}

static int PrivControlLocked( es_out_t *p_out, int i_query, va_list args )
{
es_out_sys_t *p_sys = container_of(p_out, es_out_sys_t, out);

switch( i_query )
{

case ES_OUT_PRIV_SET_MODE:
case ES_OUT_PRIV_SET_TIMES:
case ES_OUT_PRIV_SET_JITTER:
case ES_OUT_PRIV_SET_EOS:
{
ts_cmd_t cmd;
if( CmdInitPrivControl( &cmd, i_query, args, p_sys->b_delayed ) )
return VLC_EGENERIC;
if( p_sys->b_delayed )
{
TsPushCmd( p_sys->p_ts, &cmd );
return VLC_SUCCESS;
}
return CmdExecutePrivControl( p_sys->p_out, &cmd );
}
case ES_OUT_PRIV_GET_WAKE_UP: 
{
vlc_tick_t *pi_wakeup = va_arg( args, vlc_tick_t* );
return ControlLockedGetWakeup( p_out, pi_wakeup );
}
case ES_OUT_PRIV_GET_BUFFERING:
{
bool *pb_buffering = va_arg( args, bool* );
return ControlLockedGetBuffering( p_out, pb_buffering );
}
case ES_OUT_PRIV_SET_PAUSE_STATE:
{
const bool b_source_paused = (bool)va_arg( args, int );
const bool b_paused = (bool)va_arg( args, int );
const vlc_tick_t i_date = va_arg( args, vlc_tick_t );

return ControlLockedSetPauseState( p_out, b_source_paused, b_paused, i_date );
}
case ES_OUT_PRIV_SET_RATE:
{
const float src_rate = va_arg( args, double );
const float rate = va_arg( args, double );

return ControlLockedSetRate( p_out, src_rate, rate );
}
case ES_OUT_PRIV_SET_FRAME_NEXT:
{
return ControlLockedSetFrameNext( p_out );
}
case ES_OUT_PRIV_GET_GROUP_FORCED:
return es_out_vaPrivControl( p_sys->p_out, i_query, args );

case ES_OUT_PRIV_SET_ES:
case ES_OUT_PRIV_UNSET_ES:
case ES_OUT_PRIV_RESTART_ES:
case ES_OUT_PRIV_SET_ES_CAT_IDS:
case ES_OUT_PRIV_SET_ES_LIST:
case ES_OUT_PRIV_STOP_ALL_ES:
case ES_OUT_PRIV_START_ALL_ES:
case ES_OUT_PRIV_SET_ES_DELAY:
case ES_OUT_PRIV_SET_DELAY:
case ES_OUT_PRIV_SET_RECORD_STATE:
case ES_OUT_PRIV_SET_VBI_PAGE:
case ES_OUT_PRIV_SET_VBI_TRANSPARENCY:
default: vlc_assert_unreachable();
}
}

static int PrivControl( es_out_t *p_out, int i_query, va_list args )
{
es_out_sys_t *p_sys = container_of(p_out, es_out_sys_t, out);
int i_ret;

vlc_mutex_lock( &p_sys->lock );

TsAutoStop( p_out );

i_ret = PrivControlLocked( p_out, i_query, args );

vlc_mutex_unlock( &p_sys->lock );

return i_ret;
}

static const struct es_out_callbacks es_out_timeshift_cbs =
{
.add = Add,
.send = Send,
.del = Del,
.control = Control,
.destroy = Destroy,
.priv_control = PrivControl,
};




static void TsDestroy( ts_thread_t *p_ts )
{
free( p_ts );
}
static int TsStart( es_out_t *p_out )
{
es_out_sys_t *p_sys = container_of(p_out, es_out_sys_t, out);
ts_thread_t *p_ts;

assert( !p_sys->b_delayed );

p_sys->p_ts = p_ts = calloc(1, sizeof(*p_ts));
if( !p_ts )
return VLC_EGENERIC;

p_ts->i_tmp_size_max = p_sys->i_tmp_size_max;
p_ts->psz_tmp_path = p_sys->psz_tmp_path;
p_ts->p_input = p_sys->p_input;
p_ts->p_out = p_sys->p_out;
vlc_mutex_init( &p_ts->lock );
vlc_cond_init( &p_ts->wait );
p_ts->b_paused = p_sys->b_input_paused && !p_sys->b_input_paused_source;
p_ts->i_pause_date = p_ts->b_paused ? vlc_tick_now() : -1;
p_ts->rate_source = p_sys->input_rate_source;
p_ts->rate = p_sys->input_rate;
p_ts->i_rate_date = -1;
p_ts->i_rate_delay = 0;
p_ts->i_buffering_delay = 0;
p_ts->i_cmd_delay = 0;
p_ts->p_storage_r = NULL;
p_ts->p_storage_w = NULL;

p_sys->b_delayed = true;
if( vlc_clone( &p_ts->thread, TsRun, p_ts, VLC_THREAD_PRIORITY_INPUT ) )
{
msg_Err( p_sys->p_input, "cannot create timeshift thread" );

TsDestroy( p_ts );

p_sys->b_delayed = false;
return VLC_EGENERIC;
}

return VLC_SUCCESS;
}
static void TsAutoStop( es_out_t *p_out )
{
es_out_sys_t *p_sys = container_of(p_out, es_out_sys_t, out);

if( !p_sys->b_delayed || !TsIsUnused( p_sys->p_ts ) )
return;

msg_Warn( p_sys->p_input, "es out timeshift: auto stop" );
TsStop( p_sys->p_ts );

p_sys->b_delayed = false;
}
static void TsStop( ts_thread_t *p_ts )
{
vlc_cancel( p_ts->thread );
vlc_join( p_ts->thread, NULL );

vlc_mutex_lock( &p_ts->lock );
for( ;; )
{
ts_cmd_t cmd;

if( TsPopCmdLocked( p_ts, &cmd, true ) )
break;

CmdClean( &cmd );
}
assert( !p_ts->p_storage_r || !p_ts->p_storage_r->p_next );
if( p_ts->p_storage_r )
TsStorageDelete( p_ts->p_storage_r );
vlc_mutex_unlock( &p_ts->lock );

TsDestroy( p_ts );
}
static void TsPushCmd( ts_thread_t *p_ts, ts_cmd_t *p_cmd )
{
vlc_mutex_lock( &p_ts->lock );

if( !p_ts->p_storage_w || TsStorageIsFull( p_ts->p_storage_w, p_cmd ) )
{
ts_storage_t *p_storage = TsStorageNew( p_ts->psz_tmp_path, p_ts->i_tmp_size_max );

if( !p_storage )
{
CmdClean( p_cmd );
vlc_mutex_unlock( &p_ts->lock );

return;
}

if( !p_ts->p_storage_w )
{
p_ts->p_storage_r = p_ts->p_storage_w = p_storage;
}
else
{
TsStoragePack( p_ts->p_storage_w );
p_ts->p_storage_w->p_next = p_storage;
p_ts->p_storage_w = p_storage;
}
}


TsStoragePushCmd( p_ts->p_storage_w, p_cmd, p_ts->p_storage_r == p_ts->p_storage_w );

vlc_cond_signal( &p_ts->wait );

vlc_mutex_unlock( &p_ts->lock );
}
static int TsPopCmdLocked( ts_thread_t *p_ts, ts_cmd_t *p_cmd, bool b_flush )
{
vlc_mutex_assert( &p_ts->lock );

if( TsStorageIsEmpty( p_ts->p_storage_r ) )
return VLC_EGENERIC;

TsStoragePopCmd( p_ts->p_storage_r, p_cmd, b_flush );

while( TsStorageIsEmpty( p_ts->p_storage_r ) )
{
ts_storage_t *p_next = p_ts->p_storage_r->p_next;
if( !p_next )
break;

TsStorageDelete( p_ts->p_storage_r );
p_ts->p_storage_r = p_next;
}

return VLC_SUCCESS;
}
static bool TsHasCmd( ts_thread_t *p_ts )
{
bool b_cmd;

vlc_mutex_lock( &p_ts->lock );
b_cmd = !TsStorageIsEmpty( p_ts->p_storage_r );
vlc_mutex_unlock( &p_ts->lock );

return b_cmd;
}
static bool TsIsUnused( ts_thread_t *p_ts )
{
bool b_unused;

vlc_mutex_lock( &p_ts->lock );
b_unused = !p_ts->b_paused &&
p_ts->rate == p_ts->rate_source &&
TsStorageIsEmpty( p_ts->p_storage_r );
vlc_mutex_unlock( &p_ts->lock );

return b_unused;
}
static int TsChangePause( ts_thread_t *p_ts, bool b_source_paused, bool b_paused, vlc_tick_t i_date )
{
vlc_mutex_lock( &p_ts->lock );

int i_ret;
if( b_paused )
{
assert( !b_source_paused );
i_ret = es_out_SetPauseState( p_ts->p_out, true, true, i_date );
}
else
{
i_ret = es_out_SetPauseState( p_ts->p_out, false, false, i_date );
}

if( !i_ret )
{
if( !b_paused )
{
assert( p_ts->i_pause_date > 0 );

p_ts->i_cmd_delay += i_date - p_ts->i_pause_date;
}

p_ts->b_paused = b_paused;
p_ts->i_pause_date = i_date;

vlc_cond_signal( &p_ts->wait );
}
vlc_mutex_unlock( &p_ts->lock );
return i_ret;
}
static int TsChangeRate( ts_thread_t *p_ts, float src_rate, float rate )
{
int i_ret;

vlc_mutex_lock( &p_ts->lock );
p_ts->i_cmd_delay += p_ts->i_rate_delay;

p_ts->i_rate_date = -1;
p_ts->i_rate_delay = 0;
p_ts->rate = rate;
p_ts->rate_source = src_rate;

i_ret = es_out_SetRate( p_ts->p_out, rate, rate );
vlc_mutex_unlock( &p_ts->lock );

return i_ret;
}

static void *TsRun( void *p_data )
{
ts_thread_t *p_ts = p_data;
vlc_tick_t i_buffering_date = -1;

for( ;; )
{
ts_cmd_t cmd;
vlc_tick_t i_deadline;
bool b_buffering;


vlc_mutex_lock( &p_ts->lock );
mutex_cleanup_push( &p_ts->lock );

for( ;; )
{
const int canc = vlc_savecancel();
b_buffering = es_out_GetBuffering( p_ts->p_out );

if( ( !p_ts->b_paused || b_buffering ) && !TsPopCmdLocked( p_ts, &cmd, false ) )
{
vlc_restorecancel( canc );
break;
}
vlc_restorecancel( canc );

vlc_cond_wait( &p_ts->wait, &p_ts->lock );
}

if( b_buffering && i_buffering_date < 0 )
{
i_buffering_date = cmd.i_date;
}
else if( i_buffering_date > 0 )
{
p_ts->i_buffering_delay += i_buffering_date - cmd.i_date; 
if( b_buffering )
i_buffering_date = cmd.i_date;
else
i_buffering_date = -1;
}

if( p_ts->i_rate_date < 0 )
p_ts->i_rate_date = cmd.i_date;

p_ts->i_rate_delay = 0;
if( p_ts->rate_source != p_ts->rate )
{
const vlc_tick_t i_duration = cmd.i_date - p_ts->i_rate_date;
p_ts->i_rate_delay = i_duration * p_ts->rate_source / p_ts->rate - i_duration;
}
if( p_ts->i_cmd_delay + p_ts->i_rate_delay + p_ts->i_buffering_delay < 0 && p_ts->rate != p_ts->rate_source )
{
const int canc = vlc_savecancel();


msg_Warn( p_ts->p_input, "es out timeshift: auto reset rate to %f", p_ts->rate_source );

p_ts->i_cmd_delay = 0;
p_ts->i_buffering_delay = 0;

p_ts->i_rate_delay = 0;
p_ts->i_rate_date = -1;
p_ts->rate = p_ts->rate_source;

if( !es_out_SetRate( p_ts->p_out, p_ts->rate_source, p_ts->rate ) )
{
vlc_value_t val = { .f_float = p_ts->rate };



input_ControlPushHelper( p_ts->p_input, INPUT_CONTROL_SET_RATE, &val );
}

vlc_restorecancel( canc );
}
i_deadline = cmd.i_date + p_ts->i_cmd_delay + p_ts->i_rate_delay + p_ts->i_buffering_delay;

vlc_cleanup_pop();
vlc_mutex_unlock( &p_ts->lock );



vlc_cleanup_push( cmd_cleanup_routine, &cmd );

vlc_tick_wait( i_deadline );

vlc_cleanup_pop();


const int canc = vlc_savecancel();
switch( cmd.i_type )
{
case C_ADD:
CmdExecuteAdd( p_ts->p_out, &cmd );
CmdCleanAdd( &cmd );
break;
case C_SEND:
CmdExecuteSend( p_ts->p_out, &cmd );
CmdCleanSend( &cmd );
break;
case C_CONTROL:
CmdExecuteControl( p_ts->p_out, &cmd );
CmdCleanControl( &cmd );
break;
case C_DEL:
CmdExecuteDel( p_ts->p_out, &cmd );
break;
default:
vlc_assert_unreachable();
break;
}
vlc_restorecancel( canc );
}

return NULL;
}




static ts_storage_t *TsStorageNew( const char *psz_tmp_path, int64_t i_tmp_size_max )
{
ts_storage_t *p_storage = malloc( sizeof (*p_storage) );
if( unlikely(p_storage == NULL) )
return NULL;

char *psz_file;
int fd = GetTmpFile( &psz_file, psz_tmp_path );
if( fd == -1 )
{
free( p_storage );
return NULL;
}

p_storage->p_filew = fdopen( fd, "w+b" );
if( p_storage->p_filew == NULL )
{
vlc_close( fd );
vlc_unlink( psz_file );
goto error;
}

p_storage->p_filer = vlc_fopen( psz_file, "rb" );
if( p_storage->p_filer == NULL )
{
fclose( p_storage->p_filew );
vlc_unlink( psz_file );
goto error;
}

#if !defined(_WIN32)
vlc_unlink( psz_file );
free( psz_file );
#else
p_storage->psz_file = psz_file;
#endif
p_storage->p_next = NULL;


p_storage->i_file_max = i_tmp_size_max;
p_storage->i_file_size = 0;


p_storage->i_cmd_w = 0;
p_storage->i_cmd_r = 0;
p_storage->i_cmd_max = 30000;
p_storage->p_cmd = vlc_alloc( p_storage->i_cmd_max, sizeof(*p_storage->p_cmd) );


if( !p_storage->p_cmd )
{
TsStorageDelete( p_storage );
return NULL;
}
return p_storage;
error:
free( psz_file );
free( p_storage );
return NULL;
}

static void TsStorageDelete( ts_storage_t *p_storage )
{
while( p_storage->i_cmd_r < p_storage->i_cmd_w )
{
ts_cmd_t cmd;

TsStoragePopCmd( p_storage, &cmd, true );

CmdClean( &cmd );
}
free( p_storage->p_cmd );

fclose( p_storage->p_filer );
fclose( p_storage->p_filew );
#if defined(_WIN32)
vlc_unlink( p_storage->psz_file );
free( p_storage->psz_file );
#endif
free( p_storage );
}

static void TsStoragePack( ts_storage_t *p_storage )
{

if( p_storage->i_cmd_w >= p_storage->i_cmd_max )
return;

p_storage->i_cmd_max = __MAX( p_storage->i_cmd_w, 1 );

ts_cmd_t *p_new = realloc( p_storage->p_cmd, p_storage->i_cmd_max * sizeof(*p_storage->p_cmd) );
if( p_new )
p_storage->p_cmd = p_new;
}
static bool TsStorageIsFull( ts_storage_t *p_storage, const ts_cmd_t *p_cmd )
{
if( p_cmd && p_cmd->i_type == C_SEND && p_storage->i_cmd_w > 0 )
{
size_t i_size = sizeof(*p_cmd->u.send.p_block) + p_cmd->u.send.p_block->i_buffer;

if( p_storage->i_file_size + i_size >= p_storage->i_file_max )
return true;
}
return p_storage->i_cmd_w >= p_storage->i_cmd_max;
}
static bool TsStorageIsEmpty( ts_storage_t *p_storage )
{
return !p_storage || p_storage->i_cmd_r >= p_storage->i_cmd_w;
}
static void TsStoragePushCmd( ts_storage_t *p_storage, const ts_cmd_t *p_cmd, bool b_flush )
{
ts_cmd_t cmd = *p_cmd;

assert( !TsStorageIsFull( p_storage, p_cmd ) );

if( cmd.i_type == C_SEND )
{
block_t *p_block = cmd.u.send.p_block;

cmd.u.send.p_block = NULL;
cmd.u.send.i_offset = ftell( p_storage->p_filew );

if( fwrite( p_block, sizeof(*p_block), 1, p_storage->p_filew ) != 1 )
{
block_Release( p_block );
return;
}
p_storage->i_file_size += sizeof(*p_block);
if( p_block->i_buffer > 0 )
{
if( fwrite( p_block->p_buffer, p_block->i_buffer, 1, p_storage->p_filew ) != 1 )
{
block_Release( p_block );
return;
}
}
p_storage->i_file_size += p_block->i_buffer;
block_Release( p_block );

if( b_flush )
fflush( p_storage->p_filew );
}
p_storage->p_cmd[p_storage->i_cmd_w++] = cmd;
}
static void TsStoragePopCmd( ts_storage_t *p_storage, ts_cmd_t *p_cmd, bool b_flush )
{
assert( !TsStorageIsEmpty( p_storage ) );

*p_cmd = p_storage->p_cmd[p_storage->i_cmd_r++];
if( p_cmd->i_type == C_SEND )
{
block_t block;

if( !b_flush &&
!fseek( p_storage->p_filer, p_cmd->u.send.i_offset, SEEK_SET ) &&
fread( &block, sizeof(block), 1, p_storage->p_filer ) == 1 )
{
block_t *p_block = block_Alloc( block.i_buffer );
if( p_block )
{
p_block->i_dts = block.i_dts;
p_block->i_pts = block.i_pts;
p_block->i_flags = block.i_flags;
p_block->i_length = block.i_length;
p_block->i_nb_samples = block.i_nb_samples;
p_block->i_buffer = fread( p_block->p_buffer, 1, block.i_buffer, p_storage->p_filer );
}
p_cmd->u.send.p_block = p_block;
}
else
{

p_cmd->u.send.p_block = block_Alloc( 1 );
}
}
}




static void CmdClean( ts_cmd_t *p_cmd )
{
switch( p_cmd->i_type )
{
case C_ADD:
CmdCleanAdd( p_cmd );
break;
case C_SEND:
CmdCleanSend( p_cmd );
break;
case C_CONTROL:
CmdCleanControl( p_cmd );
break;
case C_DEL:
break;
default:
vlc_assert_unreachable();
break;
}
}

static int CmdInitAdd( ts_cmd_t *p_cmd, input_source_t *in, es_out_id_t *p_es,
const es_format_t *p_fmt, bool b_copy )
{
p_cmd->i_type = C_ADD;
p_cmd->i_date = vlc_tick_now();
p_cmd->u.add.p_es = p_es;
if( b_copy )
{
p_cmd->u.add.p_fmt = malloc( sizeof(*p_fmt) );
if( !p_cmd->u.add.p_fmt )
return VLC_EGENERIC;
es_format_Copy( p_cmd->u.add.p_fmt, p_fmt );
p_cmd->u.add.in = in ? input_source_Hold( in ) : NULL;
}
else
{
p_cmd->u.add.p_fmt = (es_format_t*)p_fmt;
p_cmd->u.add.in = in;
}
return VLC_SUCCESS;
}
static void CmdExecuteAdd( es_out_t *p_out, ts_cmd_t *p_cmd )
{
p_cmd->u.add.p_es->p_es = p_out->cbs->add( p_out, p_cmd->u.add.in,
p_cmd->u.add.p_fmt );
}
static void CmdCleanAdd( ts_cmd_t *p_cmd )
{
es_format_Clean( p_cmd->u.add.p_fmt );
if( p_cmd->u.add.in )
input_source_Release( p_cmd->u.add.in );
free( p_cmd->u.add.p_fmt );
}

static void CmdInitSend( ts_cmd_t *p_cmd, es_out_id_t *p_es, block_t *p_block )
{
p_cmd->i_type = C_SEND;
p_cmd->i_date = vlc_tick_now();
p_cmd->u.send.p_es = p_es;
p_cmd->u.send.p_block = p_block;
}
static int CmdExecuteSend( es_out_t *p_out, ts_cmd_t *p_cmd )
{
block_t *p_block = p_cmd->u.send.p_block;

p_cmd->u.send.p_block = NULL;

if( p_block )
{
if( p_cmd->u.send.p_es->p_es )
return es_out_Send( p_out, p_cmd->u.send.p_es->p_es, p_block );
block_Release( p_block );
}
return VLC_EGENERIC;
}
static void CmdCleanSend( ts_cmd_t *p_cmd )
{
if( p_cmd->u.send.p_block )
block_Release( p_cmd->u.send.p_block );
}

static int CmdInitDel( ts_cmd_t *p_cmd, es_out_id_t *p_es )
{
p_cmd->i_type = C_DEL;
p_cmd->i_date = vlc_tick_now();
p_cmd->u.del.p_es = p_es;
return VLC_SUCCESS;
}
static void CmdExecuteDel( es_out_t *p_out, ts_cmd_t *p_cmd )
{
if( p_cmd->u.del.p_es->p_es )
es_out_Del( p_out, p_cmd->u.del.p_es->p_es );
free( p_cmd->u.del.p_es );
}

static int CmdInitControl( ts_cmd_t *p_cmd, input_source_t *in,
int i_query, va_list args, bool b_copy )
{
p_cmd->i_type = C_CONTROL;
p_cmd->i_date = vlc_tick_now();
p_cmd->u.control.i_query = i_query;

if( b_copy )
p_cmd->u.control.in = in ? input_source_Hold( in ) : NULL;
else
p_cmd->u.control.in = in;

switch( i_query )
{

case ES_OUT_SET_GROUP: 
case ES_OUT_DEL_GROUP: 
p_cmd->u.control.u.i_int = va_arg( args, int );
break;

case ES_OUT_SET_PCR: 
case ES_OUT_SET_NEXT_DISPLAY_TIME: 
p_cmd->u.control.u.i_i64 = va_arg( args, int64_t );
break;

case ES_OUT_SET_GROUP_PCR: 
p_cmd->u.control.u.int_i64.i_int = va_arg( args, int );
p_cmd->u.control.u.int_i64.i_i64 = va_arg( args, vlc_tick_t );
break;

case ES_OUT_SET_ES_SCRAMBLED_STATE:
p_cmd->u.control.u.es_bool.p_es = va_arg( args, es_out_id_t * );
p_cmd->u.control.u.es_bool.b_bool = (bool)va_arg( args, int );
break;

case ES_OUT_RESET_PCR: 
break;

case ES_OUT_SET_META: 
case ES_OUT_SET_GROUP_META: 
{
if( i_query == ES_OUT_SET_GROUP_META )
p_cmd->u.control.u.int_meta.i_int = va_arg( args, int );
const vlc_meta_t *p_meta = va_arg( args, const vlc_meta_t * );

if( b_copy )
{
p_cmd->u.control.u.int_meta.p_meta = vlc_meta_New();
if( !p_cmd->u.control.u.int_meta.p_meta )
return VLC_EGENERIC;
vlc_meta_Merge( p_cmd->u.control.u.int_meta.p_meta, p_meta );
}
else
{

p_cmd->u.control.u.int_meta.p_meta = (vlc_meta_t*)p_meta;
}
break;
}

case ES_OUT_SET_GROUP_EPG: 
{
p_cmd->u.control.u.int_epg.i_int = va_arg( args, int );
const vlc_epg_t *p_epg = va_arg( args, const vlc_epg_t * );

if( b_copy )
{
p_cmd->u.control.u.int_epg.p_epg = vlc_epg_Duplicate( p_epg );
if( !p_cmd->u.control.u.int_epg.p_epg )
return VLC_EGENERIC;
}
else
{

p_cmd->u.control.u.int_epg.p_epg = (vlc_epg_t*)p_epg;
}
break;
}
case ES_OUT_SET_GROUP_EPG_EVENT: 
{
p_cmd->u.control.u.int_epg_evt.i_int = va_arg( args, int );
const vlc_epg_event_t *p_evt = va_arg( args, const vlc_epg_event_t * );

if( b_copy )
{
p_cmd->u.control.u.int_epg_evt.p_evt = vlc_epg_event_Duplicate( p_evt );
if( !p_cmd->u.control.u.int_epg_evt.p_evt )
return VLC_EGENERIC;
}
else
{

p_cmd->u.control.u.int_epg_evt.p_evt = (vlc_epg_event_t*)p_evt;
}
break;
}
case ES_OUT_SET_EPG_TIME: 
p_cmd->u.control.u.i_i64 = va_arg( args, int64_t );
break;


case ES_OUT_SET_ES: 
case ES_OUT_UNSET_ES: 
case ES_OUT_RESTART_ES: 
case ES_OUT_SET_ES_DEFAULT: 
p_cmd->u.control.u.p_es = va_arg( args, es_out_id_t * );
break;

case ES_OUT_SET_ES_CAT_POLICY:
p_cmd->u.control.u.es_policy.i_cat = va_arg( args, int );
p_cmd->u.control.u.es_policy.i_policy = va_arg( args, int );
break;

case ES_OUT_SET_ES_STATE:
p_cmd->u.control.u.es_bool.p_es = va_arg( args, es_out_id_t * );
p_cmd->u.control.u.es_bool.b_bool = (bool)va_arg( args, int );
break;

case ES_OUT_SET_ES_FMT: 
{
p_cmd->u.control.u.es_fmt.p_es = va_arg( args, es_out_id_t * );
es_format_t *p_fmt = va_arg( args, es_format_t * );

if( b_copy )
{
p_cmd->u.control.u.es_fmt.p_fmt = malloc( sizeof(*p_fmt) );
if( !p_cmd->u.control.u.es_fmt.p_fmt )
return VLC_EGENERIC;
es_format_Copy( p_cmd->u.control.u.es_fmt.p_fmt, p_fmt );
}
else
{
p_cmd->u.control.u.es_fmt.p_fmt = p_fmt;
}
break;
}
default:
vlc_assert_unreachable();
return VLC_EGENERIC;
}

return VLC_SUCCESS;
}
static int CmdExecuteControl( es_out_t *p_out, ts_cmd_t *p_cmd )
{
const int i_query = p_cmd->u.control.i_query;
input_source_t *in = p_cmd->u.control.in;

switch( i_query )
{

case ES_OUT_SET_GROUP: 
case ES_OUT_DEL_GROUP: 
return es_out_in_Control( p_out, in, i_query, p_cmd->u.control.u.i_int );

case ES_OUT_SET_PCR: 
case ES_OUT_SET_NEXT_DISPLAY_TIME: 
return es_out_in_Control( p_out, in, i_query, p_cmd->u.control.u.i_i64 );

case ES_OUT_SET_GROUP_PCR: 
return es_out_in_Control( p_out, in, i_query, p_cmd->u.control.u.int_i64.i_int,
p_cmd->u.control.u.int_i64.i_i64 );

case ES_OUT_RESET_PCR: 
return es_out_in_Control( p_out, in, i_query );

case ES_OUT_SET_GROUP_META: 
return es_out_in_Control( p_out, in, i_query, p_cmd->u.control.u.int_meta.i_int,
p_cmd->u.control.u.int_meta.p_meta );

case ES_OUT_SET_GROUP_EPG: 
return es_out_in_Control( p_out, in, i_query, p_cmd->u.control.u.int_epg.i_int,
p_cmd->u.control.u.int_epg.p_epg );

case ES_OUT_SET_GROUP_EPG_EVENT: 
return es_out_in_Control( p_out, in, i_query, p_cmd->u.control.u.int_epg_evt.i_int,
p_cmd->u.control.u.int_epg_evt.p_evt );

case ES_OUT_SET_EPG_TIME: 
return es_out_in_Control( p_out, in, i_query, p_cmd->u.control.u.i_i64 );

case ES_OUT_SET_ES_SCRAMBLED_STATE: 
return es_out_in_Control( p_out, in, i_query, p_cmd->u.control.u.es_bool.p_es->p_es,
p_cmd->u.control.u.es_bool.b_bool );

case ES_OUT_SET_META: 
return es_out_in_Control( p_out, in, i_query, p_cmd->u.control.u.int_meta.p_meta );


case ES_OUT_SET_ES: 
case ES_OUT_UNSET_ES: 
case ES_OUT_RESTART_ES: 
case ES_OUT_SET_ES_DEFAULT: 
return es_out_in_Control( p_out, in, i_query, !p_cmd->u.control.u.p_es ? NULL :
p_cmd->u.control.u.p_es->p_es );

case ES_OUT_SET_ES_STATE:
return es_out_in_Control( p_out, in, i_query, p_cmd->u.control.u.es_bool.p_es->p_es,
p_cmd->u.control.u.es_bool.b_bool );

case ES_OUT_SET_ES_CAT_POLICY:
return es_out_in_Control( p_out, in, i_query, p_cmd->u.control.u.es_policy.i_cat,
p_cmd->u.control.u.es_policy.i_policy );

case ES_OUT_SET_ES_FMT: 
return es_out_in_Control( p_out, in, i_query, p_cmd->u.control.u.es_fmt.p_es->p_es,
p_cmd->u.control.u.es_fmt.p_fmt );

default:
vlc_assert_unreachable();
return VLC_EGENERIC;
}
}
static void CmdCleanControl( ts_cmd_t *p_cmd )
{
if( p_cmd->u.control.in )
input_source_Release( p_cmd->u.control.in );

switch( p_cmd->u.control.i_query )
{
case ES_OUT_SET_GROUP_META:
case ES_OUT_SET_META:
if( p_cmd->u.control.u.int_meta.p_meta )
vlc_meta_Delete( p_cmd->u.control.u.int_meta.p_meta );
break;
case ES_OUT_SET_GROUP_EPG:
if( p_cmd->u.control.u.int_epg.p_epg )
vlc_epg_Delete( p_cmd->u.control.u.int_epg.p_epg );
break;
case ES_OUT_SET_GROUP_EPG_EVENT:
if( p_cmd->u.control.u.int_epg_evt.p_evt )
vlc_epg_event_Delete( p_cmd->u.control.u.int_epg_evt.p_evt );
break;
case ES_OUT_SET_ES_FMT:
if( p_cmd->u.control.u.es_fmt.p_fmt )
{
es_format_Clean( p_cmd->u.control.u.es_fmt.p_fmt );
free( p_cmd->u.control.u.es_fmt.p_fmt );
}
break;
}
}

static int CmdInitPrivControl( ts_cmd_t *p_cmd, int i_query, va_list args, bool b_copy )
{
VLC_UNUSED( b_copy );
p_cmd->i_type = C_CONTROL;
p_cmd->i_date = vlc_tick_now();
p_cmd->u.privcontrol.i_query = i_query;

switch( i_query )
{

case ES_OUT_PRIV_SET_MODE: 
p_cmd->u.privcontrol.u.i_int = va_arg( args, int );
break;
case ES_OUT_PRIV_SET_JITTER:
{
vlc_tick_t i_pts_delay = va_arg( args, vlc_tick_t );
vlc_tick_t i_pts_jitter = va_arg( args, vlc_tick_t );
int i_cr_average = va_arg( args, int );

p_cmd->u.privcontrol.u.jitter.i_pts_delay = i_pts_delay;
p_cmd->u.privcontrol.u.jitter.i_pts_jitter = i_pts_jitter;
p_cmd->u.privcontrol.u.jitter.i_cr_average = i_cr_average;
break;
}
case ES_OUT_PRIV_SET_TIMES:
{
double f_position = va_arg( args, double );
vlc_tick_t i_time = va_arg( args, vlc_tick_t );
vlc_tick_t i_normal_time = va_arg( args, vlc_tick_t );
vlc_tick_t i_length = va_arg( args, vlc_tick_t );

p_cmd->u.privcontrol.u.times.f_position = f_position;
p_cmd->u.privcontrol.u.times.i_time = i_time;
p_cmd->u.privcontrol.u.times.i_normal_time = i_normal_time;
p_cmd->u.privcontrol.u.times.i_length = i_length;
break;
}
case ES_OUT_PRIV_SET_EOS: 
break;
default: vlc_assert_unreachable();
}

return VLC_SUCCESS;
}

static int CmdExecutePrivControl( es_out_t *p_out, ts_cmd_t *p_cmd )
{
const int i_query = p_cmd->u.privcontrol.i_query;

switch( i_query )
{

case ES_OUT_PRIV_SET_MODE: 
return es_out_PrivControl( p_out, i_query, p_cmd->u.privcontrol.u.i_int );
case ES_OUT_PRIV_SET_JITTER:
return es_out_PrivControl( p_out, i_query, p_cmd->u.privcontrol.u.jitter.i_pts_delay,
p_cmd->u.privcontrol.u.jitter.i_pts_jitter,
p_cmd->u.privcontrol.u.jitter.i_cr_average );
case ES_OUT_PRIV_SET_TIMES:
return es_out_PrivControl( p_out, i_query,
p_cmd->u.privcontrol.u.times.f_position,
p_cmd->u.privcontrol.u.times.i_time,
p_cmd->u.privcontrol.u.times.i_normal_time,
p_cmd->u.privcontrol.u.times.i_length );
case ES_OUT_PRIV_SET_EOS: 
return es_out_PrivControl( p_out, i_query );
default: vlc_assert_unreachable();
}
}

static int GetTmpFile( char **filename, const char *dirname )
{
if( dirname != NULL
&& asprintf( filename, "%s"DIR_SEP PACKAGE_NAME"-timeshift.XXXXXX",
dirname ) >= 0 )
{
vlc_mkdir( dirname, 0700 );

int fd = vlc_mkstemp( *filename );
if( fd != -1 )
return fd;

free( *filename );
}

*filename = strdup( DIR_SEP"tmp"DIR_SEP PACKAGE_NAME"-timeshift.XXXXXX" );
if( unlikely(*filename == NULL) )
return -1;

int fd = vlc_mkstemp( *filename );
if( fd != -1 )
return fd;

free( *filename );
return -1;
}
