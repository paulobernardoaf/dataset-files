#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <stdio.h>
#include <assert.h>
#include <vlc_common.h>
#include <vlc_es_out.h>
#include <vlc_block.h>
#include <vlc_aout.h>
#include <vlc_fourcc.h>
#include <vlc_meta.h>
#include <vlc_list.h>
#include <vlc_decoder.h>
#include <vlc_memstream.h>
#include "input_internal.h"
#include "../clock/input_clock.h"
#include "../clock/clock.h"
#include "decoder.h"
#include "es_out.h"
#include "event.h"
#include "resource.h"
#include "info.h"
#include "item.h"
#include "../stream_output/stream_output.h"
#include <vlc_iso_lang.h>
#include "../text/iso-639_def.h"
typedef struct
{
input_source_t *source;
int i_id;
int i_es;
bool b_selected;
bool b_scrambled;
input_clock_t *p_input_clock;
vlc_clock_main_t *p_main_clock;
vlc_clock_t *p_master_clock;
vlc_meta_t *p_meta;
struct vlc_list node;
} es_out_pgrm_t;
struct vlc_es_id_t
{
int i_id;
enum es_format_category_e i_cat;
input_source_t *source;
bool stable;
char *str_id;
};
struct es_out_id_t
{
vlc_es_id_t id;
es_out_t *out;
es_out_pgrm_t *p_pgrm;
bool b_scrambled;
bool b_forced; 
int i_channel;
vlc_atomic_rc_t rc;
size_t i_pos; 
es_format_t fmt; 
es_format_t fmt_out; 
char *psz_language;
char *psz_language_code;
char *psz_title;
bool b_terminated;
vlc_input_decoder_t *p_dec;
vlc_input_decoder_t *p_dec_record;
vlc_clock_t *p_clock;
bool master;
vlc_tick_t delay;
struct
{
vlc_fourcc_t type;
uint64_t i_bitmap; 
es_out_id_t *pp_es[64]; 
} cc;
es_out_id_t *p_master;
struct vlc_list node;
vlc_mouse_event mouse_event_cb;
void* mouse_event_userdata;
};
typedef struct
{
int i_count; 
es_out_id_t *p_main_es; 
enum es_out_policy_e e_policy;
bool b_autoselect; 
char *str_ids; 
int i_demux_id; 
int i_channel; 
char **ppsz_language;
} es_out_es_props_t;
typedef struct
{
input_thread_t *p_input;
input_source_t *main_source;
vlc_mutex_t lock;
struct vlc_list programs;
es_out_pgrm_t *p_pgrm; 
enum es_format_category_e i_master_source_cat;
int i_id;
struct vlc_list es;
struct vlc_list es_slaves; 
bool b_active;
int i_mode;
es_out_es_props_t video, audio, sub;
int i_group_id;
vlc_tick_t i_audio_delay;
vlc_tick_t i_spu_delay;
vlc_tick_t i_pts_delay;
vlc_tick_t i_tracks_pts_delay;
vlc_tick_t i_pts_jitter;
int i_cr_average;
float rate;
bool b_paused;
vlc_tick_t i_pause_date;
vlc_tick_t i_preroll_end;
bool b_buffering;
vlc_tick_t i_buffering_extra_initial;
vlc_tick_t i_buffering_extra_stream;
vlc_tick_t i_buffering_extra_system;
sout_instance_t *p_sout_record;
int i_prev_stream_level;
es_out_t out;
} es_out_sys_t;
static void EsOutDelLocked( es_out_t *, es_out_id_t * );
static void EsOutDel ( es_out_t *, es_out_id_t * );
static void EsOutTerminate( es_out_t * );
static void EsOutSelect( es_out_t *, es_out_id_t *es, bool b_force );
static void EsOutSelectList( es_out_t *, enum es_format_category_e cat,
vlc_es_id_t *const* es_id_list );
static void EsOutUpdateInfo( es_out_t *, es_out_id_t *es, const vlc_meta_t * );
static int EsOutSetRecord( es_out_t *, bool b_record );
static bool EsIsSelected( es_out_id_t *es );
static void EsOutSelectEs( es_out_t *out, es_out_id_t *es, bool b_force );
static void EsOutDeleteInfoEs( es_out_t *, es_out_id_t *es );
static void EsOutUnselectEs( es_out_t *out, es_out_id_t *es, bool b_update );
static void EsOutDecoderChangeDelay( es_out_t *out, es_out_id_t *p_es );
static void EsOutDecodersChangePause( es_out_t *out, bool b_paused, vlc_tick_t i_date );
static void EsOutProgramChangePause( es_out_t *out, bool b_paused, vlc_tick_t i_date );
static void EsOutProgramsChangeRate( es_out_t *out );
static void EsOutDecodersStopBuffering( es_out_t *out, bool b_forced );
static void EsOutGlobalMeta( es_out_t *p_out, const vlc_meta_t *p_meta );
static void EsOutMeta( es_out_t *p_out, const vlc_meta_t *p_meta, const vlc_meta_t *p_progmeta );
static int EsOutEsUpdateFmt(es_out_t *out, es_out_id_t *es, const es_format_t *fmt);
static int EsOutControlLocked( es_out_t *out, input_source_t *, int i_query, ... );
static int EsOutPrivControlLocked( es_out_t *out, int i_query, ... );
static char *LanguageGetName( const char *psz_code );
static char *LanguageGetCode( const char *psz_lang );
static char **LanguageSplit( const char *psz_langs );
static int LanguageArrayIndex( char **ppsz_langs, const char *psz_lang );
static char *EsOutProgramGetMetaName( es_out_pgrm_t *p_pgrm );
static char *EsInfoCategoryName( es_out_id_t* es );
static inline int EsOutGetClosedCaptionsChannel( const es_format_t *p_fmt )
{
int i_channel;
if( p_fmt->i_codec == VLC_CODEC_CEA608 && p_fmt->subs.cc.i_channel < 4 )
i_channel = p_fmt->subs.cc.i_channel;
else if( p_fmt->i_codec == VLC_CODEC_CEA708 && p_fmt->subs.cc.i_channel < 64 )
i_channel = p_fmt->subs.cc.i_channel;
else
i_channel = -1;
return i_channel;
}
#define foreach_es_then_es_slaves( pos ) for( int fetes_i=0; fetes_i<2; fetes_i++ ) vlc_list_foreach( pos, (!fetes_i ? &p_sys->es : &p_sys->es_slaves), node )
static void
decoder_on_vout_started(vlc_input_decoder_t *decoder, vout_thread_t *vout,
enum vlc_vout_order order, void *userdata)
{
(void) decoder;
es_out_id_t *id = userdata;
es_out_t *out = id->out;
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
if (!p_sys->p_input)
return;
struct vlc_input_event_vout event = {
.action = VLC_INPUT_EVENT_VOUT_STARTED,
.vout = vout,
.order = order,
.id = &id->id,
};
input_SendEventVout(p_sys->p_input, &event);
}
static void
decoder_on_vout_stopped(vlc_input_decoder_t *decoder, vout_thread_t *vout, void *userdata)
{
(void) decoder;
es_out_id_t *id = userdata;
es_out_t *out = id->out;
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
if (!p_sys->p_input)
return;
struct vlc_input_event_vout event = {
.action = VLC_INPUT_EVENT_VOUT_STOPPED,
.vout = vout,
.order = VLC_VOUT_ORDER_NONE,
.id = &id->id,
};
input_SendEventVout(p_sys->p_input, &event);
}
static void
decoder_on_thumbnail_ready(vlc_input_decoder_t *decoder, picture_t *pic, void *userdata)
{
(void) decoder;
es_out_id_t *id = userdata;
es_out_t *out = id->out;
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
if (!p_sys->p_input)
return;
struct vlc_input_event event = {
.type = INPUT_EVENT_THUMBNAIL_READY,
.thumbnail = pic,
};
input_SendEvent(p_sys->p_input, &event);
}
static void
decoder_on_new_video_stats(vlc_input_decoder_t *decoder, unsigned decoded, unsigned lost,
unsigned displayed, void *userdata)
{
(void) decoder;
es_out_id_t *id = userdata;
es_out_t *out = id->out;
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
if (!p_sys->p_input)
return;
struct input_stats *stats = input_priv(p_sys->p_input)->stats;
if (!stats)
return;
atomic_fetch_add_explicit(&stats->decoded_video, decoded,
memory_order_relaxed);
atomic_fetch_add_explicit(&stats->lost_pictures, lost,
memory_order_relaxed);
atomic_fetch_add_explicit(&stats->displayed_pictures, displayed,
memory_order_relaxed);
}
static void
decoder_on_new_audio_stats(vlc_input_decoder_t *decoder, unsigned decoded, unsigned lost,
unsigned played, void *userdata)
{
(void) decoder;
es_out_id_t *id = userdata;
es_out_t *out = id->out;
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
if (!p_sys->p_input)
return;
struct input_stats *stats = input_priv(p_sys->p_input)->stats;
if (!stats)
return;
atomic_fetch_add_explicit(&stats->decoded_audio, decoded,
memory_order_relaxed);
atomic_fetch_add_explicit(&stats->lost_abuffers, lost,
memory_order_relaxed);
atomic_fetch_add_explicit(&stats->played_abuffers, played,
memory_order_relaxed);
}
static int
decoder_get_attachments(vlc_input_decoder_t *decoder,
input_attachment_t ***ppp_attachment,
void *userdata)
{
(void) decoder;
es_out_id_t *id = userdata;
es_out_t *out = id->out;
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
if (!p_sys->p_input)
return -1;
return input_GetAttachments(p_sys->p_input, ppp_attachment);
}
static const struct vlc_input_decoder_callbacks decoder_cbs = {
.on_vout_started = decoder_on_vout_started,
.on_vout_stopped = decoder_on_vout_stopped,
.on_thumbnail_ready = decoder_on_thumbnail_ready,
.on_new_video_stats = decoder_on_new_video_stats,
.on_new_audio_stats = decoder_on_new_audio_stats,
.get_attachments = decoder_get_attachments,
};
static es_out_es_props_t * GetPropsByCat( es_out_sys_t *p_sys, int i_cat )
{
switch( i_cat )
{
case AUDIO_ES:
return &p_sys->audio;
case SPU_ES:
return &p_sys->sub;
case VIDEO_ES:
return &p_sys->video;
}
return NULL;
}
static void EsOutPropsCleanup( es_out_es_props_t *p_props )
{
free( p_props->str_ids );
if( p_props->ppsz_language )
{
for( int i = 0; p_props->ppsz_language[i]; i++ )
free( p_props->ppsz_language[i] );
free( p_props->ppsz_language );
}
}
static void EsOutPropsInit( es_out_es_props_t *p_props,
bool autoselect,
input_thread_t *p_input,
enum es_out_policy_e e_default_policy,
const char *psz_trackidvar,
const char *psz_trackvar,
const char *psz_langvar,
const char *psz_debug )
{
p_props->e_policy = e_default_policy;
p_props->i_count = 0;
p_props->b_autoselect = autoselect;
p_props->str_ids = (psz_trackidvar) ? var_GetNonEmptyString( p_input, psz_trackidvar ) : NULL;
p_props->i_channel = (psz_trackvar) ? var_GetInteger( p_input, psz_trackvar ): -1;
p_props->i_demux_id = -1;
p_props->p_main_es = NULL;
if( !input_priv(p_input)->b_preparsing && psz_langvar )
{
char *psz_string = var_GetString( p_input, psz_langvar );
p_props->ppsz_language = LanguageSplit( psz_string );
if( p_props->ppsz_language )
{
for( int i = 0; p_props->ppsz_language[i]; i++ )
msg_Dbg( p_input, "selected %s language[%d] %s",
psz_debug, i, p_props->ppsz_language[i] );
}
free( psz_string );
}
}
static const struct es_out_callbacks es_out_cbs;
es_out_t *input_EsOutNew( input_thread_t *p_input, input_source_t *main_source, float rate )
{
es_out_sys_t *p_sys = calloc( 1, sizeof( *p_sys ) );
if( !p_sys )
return NULL;
p_sys->out.cbs = &es_out_cbs;
vlc_mutex_init( &p_sys->lock );
p_sys->p_input = p_input;
p_sys->main_source = main_source;
p_sys->b_active = false;
p_sys->i_mode = ES_OUT_MODE_NONE;
vlc_list_init(&p_sys->programs);
vlc_list_init(&p_sys->es);
vlc_list_init(&p_sys->es_slaves);
EsOutPropsInit( &p_sys->video, true, p_input, ES_OUT_ES_POLICY_AUTO,
"video-track-id", "video-track", NULL, NULL );
EsOutPropsInit( &p_sys->audio, true, p_input, ES_OUT_ES_POLICY_EXCLUSIVE,
"audio-track-id", "audio-track", "audio-language", "audio" );
EsOutPropsInit( &p_sys->sub, false, p_input, ES_OUT_ES_POLICY_AUTO,
"sub-track-id", "sub-track", "sub-language", "sub" );
p_sys->i_group_id = var_GetInteger( p_input, "program" );
enum vlc_clock_master_source master_source =
var_InheritInteger( p_input, "clock-master" );
switch( master_source )
{
case VLC_CLOCK_MASTER_AUDIO:
p_sys->i_master_source_cat = AUDIO_ES;
break;
case VLC_CLOCK_MASTER_MONOTONIC:
default:
p_sys->i_master_source_cat = UNKNOWN_ES;
break;
}
p_sys->i_pause_date = -1;
p_sys->rate = rate;
p_sys->b_buffering = true;
p_sys->i_preroll_end = -1;
p_sys->i_prev_stream_level = -1;
return &p_sys->out;
}
static void EsTerminate(es_out_id_t *es)
{
vlc_list_remove(&es->node);
es->b_terminated = true;
}
static char *EsGetTitle( es_out_id_t *es )
{
const es_format_t *fmt = &es->fmt;
char *title;
if( fmt->psz_description && *fmt->psz_description )
{
if( es->psz_language && *es->psz_language )
{
if( asprintf( &title, "%s - [%s]", fmt->psz_description,
es->psz_language ) == -1 )
title = NULL;
}
else
title = strdup( fmt->psz_description );
}
else
{
if( es->psz_language && *es->psz_language )
{
if( asprintf( &title, "%s %zu - [%s]", _("Track"),
es->i_pos, es->psz_language ) == -1 )
title = NULL;
}
else
{
if( asprintf( &title, "%s %zu", _("Track"), es->i_pos ) == -1 )
title = NULL;
}
}
return title;
}
static void EsRelease(es_out_id_t *es)
{
if (vlc_atomic_rc_dec(&es->rc))
{
free(es->psz_title);
free(es->psz_language);
free(es->psz_language_code);
es_format_Clean(&es->fmt);
input_source_Release(es->id.source);
free(es->id.str_id);
free(es);
}
}
static void EsHold(es_out_id_t *es)
{
vlc_atomic_rc_inc(&es->rc);
}
static void EsOutDelete( es_out_t *out )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
assert(vlc_list_is_empty(&p_sys->es));
assert(vlc_list_is_empty(&p_sys->es_slaves));
assert(vlc_list_is_empty(&p_sys->programs));
assert(p_sys->p_pgrm == NULL);
EsOutPropsCleanup( &p_sys->video );
EsOutPropsCleanup( &p_sys->audio );
EsOutPropsCleanup( &p_sys->sub );
free( p_sys );
}
static void ProgramDelete( es_out_pgrm_t *p_pgrm )
{
input_clock_Delete( p_pgrm->p_input_clock );
vlc_clock_main_Delete( p_pgrm->p_main_clock );
if( p_pgrm->p_meta )
vlc_meta_Delete( p_pgrm->p_meta );
input_source_Release( p_pgrm->source );
free( p_pgrm );
}
static void EsOutTerminate( es_out_t *out )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
es_out_id_t *es;
if( p_sys->p_sout_record )
EsOutSetRecord( out, false );
foreach_es_then_es_slaves(es)
{
if (es->p_dec != NULL)
vlc_input_decoder_Delete(es->p_dec);
EsTerminate(es);
EsRelease(es);
}
es_out_pgrm_t *p_pgrm;
vlc_list_foreach(p_pgrm, &p_sys->programs, node)
{
vlc_list_remove(&p_pgrm->node);
input_SendEventProgramDel( p_sys->p_input, p_pgrm->i_id );
ProgramDelete(p_pgrm);
}
p_sys->p_pgrm = NULL;
input_item_SetEpgOffline( input_priv(p_sys->p_input)->p_item );
input_SendEventMetaEpg( p_sys->p_input );
}
static vlc_tick_t EsOutGetWakeup( es_out_t *out )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
input_thread_t *p_input = p_sys->p_input;
if( !p_sys->p_pgrm )
return 0;
if( !input_priv(p_input)->b_can_pace_control ||
input_priv(p_input)->b_out_pace_control ||
p_sys->b_buffering )
return 0;
return input_clock_GetWakeup( p_sys->p_pgrm->p_input_clock );
}
static es_out_id_t es_cat[DATA_ES];
static es_out_id_t *EsOutGetSelectedCat( es_out_t *out,
enum es_format_category_e cat )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
es_out_id_t *es;
foreach_es_then_es_slaves( es )
if( es->fmt.i_cat == cat && EsIsSelected( es ) )
return es;
return NULL;
}
static bool EsOutDecodersIsEmpty( es_out_t *out )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
es_out_id_t *es;
if( p_sys->b_buffering && p_sys->p_pgrm )
{
EsOutDecodersStopBuffering( out, true );
if( p_sys->b_buffering )
return true;
}
foreach_es_then_es_slaves(es)
{
if( es->p_dec && !vlc_input_decoder_IsEmpty( es->p_dec ) )
return false;
if( es->p_dec_record && !vlc_input_decoder_IsEmpty( es->p_dec_record ) )
return false;
}
return true;
}
static void EsOutSetEsDelay(es_out_t *out, es_out_id_t *es, vlc_tick_t delay)
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
assert(es->fmt.i_cat == AUDIO_ES || es->fmt.i_cat == SPU_ES);
es->delay = delay;
EsOutDecoderChangeDelay(out, es);
EsOutPrivControlLocked(out, ES_OUT_PRIV_SET_JITTER, p_sys->i_pts_delay,
p_sys->i_pts_jitter, p_sys->i_cr_average);
}
static void EsOutSetDelay( es_out_t *out, int i_cat, vlc_tick_t i_delay )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
es_out_id_t *es;
if( i_cat == AUDIO_ES )
p_sys->i_audio_delay = i_delay;
else if( i_cat == SPU_ES )
p_sys->i_spu_delay = i_delay;
foreach_es_then_es_slaves(es)
EsOutDecoderChangeDelay(out, es);
EsOutPrivControlLocked(out, ES_OUT_PRIV_SET_JITTER, p_sys->i_pts_delay,
p_sys->i_pts_jitter, p_sys->i_cr_average);
}
static int EsOutSetRecord( es_out_t *out, bool b_record )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
input_thread_t *p_input = p_sys->p_input;
es_out_id_t *p_es;
assert( ( b_record && !p_sys->p_sout_record ) || ( !b_record && p_sys->p_sout_record ) );
if( b_record )
{
char *psz_path = var_CreateGetNonEmptyString( p_input, "input-record-path" );
if( !psz_path )
{
if( var_CountChoices( p_input, "video-es" ) )
psz_path = config_GetUserDir( VLC_VIDEOS_DIR );
else if( var_CountChoices( p_input, "audio-es" ) )
psz_path = config_GetUserDir( VLC_MUSIC_DIR );
else
psz_path = config_GetUserDir( VLC_DOWNLOAD_DIR );
}
char *psz_sout = NULL; 
if( !psz_sout && psz_path )
{
char *psz_file = input_item_CreateFilename( input_GetItem(p_input),
psz_path,
INPUT_RECORD_PREFIX, NULL );
if( psz_file )
{
char* psz_file_esc = config_StringEscape( psz_file );
if ( psz_file_esc )
{
if( asprintf( &psz_sout, "#record{dst-prefix='%s'}", psz_file_esc ) < 0 )
psz_sout = NULL;
free( psz_file_esc );
}
free( psz_file );
}
}
free( psz_path );
if( !psz_sout )
return VLC_EGENERIC;
#if defined(ENABLE_SOUT)
p_sys->p_sout_record = sout_NewInstance( p_input, psz_sout );
#endif
free( psz_sout );
if( !p_sys->p_sout_record )
return VLC_EGENERIC;
vlc_list_foreach( p_es, &p_sys->es, node ) 
{
if( !p_es->p_dec )
continue;
p_es->p_dec_record =
vlc_input_decoder_New( VLC_OBJECT(p_input), &p_es->fmt, NULL,
input_priv(p_input)->p_resource,
p_sys->p_sout_record, false,
&decoder_cbs, p_es );
if( p_es->p_dec_record && p_sys->b_buffering )
vlc_input_decoder_StartWait( p_es->p_dec_record );
}
}
else
{
vlc_list_foreach( p_es, &p_sys->es, node ) 
{
if( !p_es->p_dec_record )
continue;
vlc_input_decoder_Delete( p_es->p_dec_record );
p_es->p_dec_record = NULL;
}
#if defined(ENABLE_SOUT)
sout_DeleteInstance( p_sys->p_sout_record );
#endif
p_sys->p_sout_record = NULL;
}
return VLC_SUCCESS;
}
static void EsOutChangePause( es_out_t *out, bool b_paused, vlc_tick_t i_date )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
if( b_paused )
{
EsOutDecodersChangePause( out, true, i_date );
EsOutProgramChangePause( out, true, i_date );
}
else
{
if( p_sys->i_buffering_extra_initial > 0 )
{
vlc_tick_t i_stream_start;
vlc_tick_t i_system_start;
vlc_tick_t i_stream_duration;
vlc_tick_t i_system_duration;
int i_ret;
i_ret = input_clock_GetState( p_sys->p_pgrm->p_input_clock,
&i_stream_start, &i_system_start,
&i_stream_duration, &i_system_duration );
if( !i_ret )
{
const vlc_tick_t i_used = p_sys->i_buffering_extra_system - p_sys->i_buffering_extra_initial;
i_date -= i_used;
}
p_sys->i_buffering_extra_initial = 0;
p_sys->i_buffering_extra_stream = 0;
p_sys->i_buffering_extra_system = 0;
}
EsOutProgramChangePause( out, false, i_date );
EsOutDecodersChangePause( out, false, i_date );
EsOutProgramsChangeRate( out );
}
p_sys->b_paused = b_paused;
p_sys->i_pause_date = i_date;
}
static void EsOutChangeRate( es_out_t *out, float rate )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
es_out_id_t *es;
p_sys->rate = rate;
EsOutProgramsChangeRate( out );
foreach_es_then_es_slaves(es)
if( es->p_dec != NULL )
vlc_input_decoder_ChangeRate( es->p_dec, rate );
}
static void EsOutChangePosition( es_out_t *out, bool b_flush )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
es_out_id_t *p_es;
input_SendEventCache( p_sys->p_input, 0.0 );
foreach_es_then_es_slaves(p_es)
if( p_es->p_dec != NULL )
{
if( b_flush )
vlc_input_decoder_Flush( p_es->p_dec );
if( !p_sys->b_buffering )
{
vlc_input_decoder_StartWait( p_es->p_dec );
if( p_es->p_dec_record != NULL )
vlc_input_decoder_StartWait( p_es->p_dec_record );
}
}
es_out_pgrm_t *pgrm;
vlc_list_foreach(pgrm, &p_sys->programs, node)
{
input_clock_Reset(pgrm->p_input_clock);
vlc_clock_main_Reset(pgrm->p_main_clock);
}
p_sys->b_buffering = true;
p_sys->i_buffering_extra_initial = 0;
p_sys->i_buffering_extra_stream = 0;
p_sys->i_buffering_extra_system = 0;
p_sys->i_preroll_end = -1;
p_sys->i_prev_stream_level = -1;
}
static void EsOutStopFreeVout( es_out_t *out )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
if( p_sys->b_active )
input_resource_StopFreeVout( input_priv(p_sys->p_input)->p_resource );
}
static void EsOutDecodersStopBuffering( es_out_t *out, bool b_forced )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
es_out_id_t *p_es;
vlc_tick_t i_stream_start;
vlc_tick_t i_system_start;
vlc_tick_t i_stream_duration;
vlc_tick_t i_system_duration;
if (input_clock_GetState( p_sys->p_pgrm->p_input_clock,
&i_stream_start, &i_system_start,
&i_stream_duration, &i_system_duration ))
return;
vlc_tick_t i_preroll_duration = 0;
if( p_sys->i_preroll_end >= 0 )
i_preroll_duration = __MAX( p_sys->i_preroll_end - i_stream_start, 0 );
const vlc_tick_t i_buffering_duration = p_sys->i_pts_delay +
p_sys->i_pts_jitter +
p_sys->i_tracks_pts_delay +
i_preroll_duration +
p_sys->i_buffering_extra_stream - p_sys->i_buffering_extra_initial;
if( i_stream_duration <= i_buffering_duration && !b_forced )
{
double f_level;
if (i_buffering_duration == 0)
f_level = 0;
else
f_level = __MAX( (double)i_stream_duration / i_buffering_duration, 0 );
input_SendEventCache( p_sys->p_input, f_level );
int i_level = (int)(100 * f_level);
if( p_sys->i_prev_stream_level != i_level )
{
msg_Dbg( p_sys->p_input, "Buffering %d%%", i_level );
p_sys->i_prev_stream_level = i_level;
}
return;
}
input_SendEventCache( p_sys->p_input, 1.0 );
msg_Dbg( p_sys->p_input, "Stream buffering done (%d ms in %d ms)",
(int)MS_FROM_VLC_TICK(i_stream_duration), (int)MS_FROM_VLC_TICK(i_system_duration) );
p_sys->b_buffering = false;
p_sys->i_preroll_end = -1;
p_sys->i_prev_stream_level = -1;
if( p_sys->i_buffering_extra_initial > 0 )
{
return;
}
const vlc_tick_t i_decoder_buffering_start = vlc_tick_now();
foreach_es_then_es_slaves(p_es)
{
if( !p_es->p_dec || p_es->fmt.i_cat == SPU_ES )
continue;
vlc_input_decoder_Wait( p_es->p_dec );
if( p_es->p_dec_record )
vlc_input_decoder_Wait( p_es->p_dec_record );
}
msg_Dbg( p_sys->p_input, "Decoder wait done in %d ms",
(int)MS_FROM_VLC_TICK(vlc_tick_now() - i_decoder_buffering_start) );
EsOutStopFreeVout( out );
const vlc_tick_t i_wakeup_delay = VLC_TICK_FROM_MS(10); 
const vlc_tick_t i_current_date = p_sys->b_paused ? p_sys->i_pause_date : vlc_tick_now();
const vlc_tick_t update = i_current_date + i_wakeup_delay - i_buffering_duration;
vlc_clock_main_SetFirstPcr(p_sys->p_pgrm->p_main_clock, update,
i_stream_start);
input_clock_ChangeSystemOrigin( p_sys->p_pgrm->p_input_clock, true, update );
foreach_es_then_es_slaves(p_es)
{
if( !p_es->p_dec )
continue;
vlc_input_decoder_StopWait( p_es->p_dec );
if( p_es->p_dec_record )
vlc_input_decoder_StopWait( p_es->p_dec_record );
}
}
static void EsOutDecodersChangePause( es_out_t *out, bool b_paused, vlc_tick_t i_date )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
es_out_id_t *es;
foreach_es_then_es_slaves(es)
if( es->p_dec )
{
vlc_input_decoder_ChangePause( es->p_dec, b_paused, i_date );
if( es->p_dec_record )
vlc_input_decoder_ChangePause( es->p_dec_record, b_paused,
i_date );
}
}
static bool EsOutIsExtraBufferingAllowed( es_out_t *out )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
es_out_id_t *p_es;
size_t i_size = 0;
foreach_es_then_es_slaves(p_es)
{
if( p_es->p_dec )
i_size += vlc_input_decoder_GetFifoSize( p_es->p_dec );
if( p_es->p_dec_record )
i_size += vlc_input_decoder_GetFifoSize( p_es->p_dec_record );
}
#if defined(OPTIMIZE_MEMORY)
const size_t i_level_high = 512*1024; 
#else
const size_t i_level_high = 10*1024*1024; 
#endif
return i_size < i_level_high;
}
static void EsOutProgramChangePause( es_out_t *out, bool b_paused, vlc_tick_t i_date )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
es_out_pgrm_t *pgrm;
vlc_list_foreach(pgrm, &p_sys->programs, node)
{
input_clock_ChangePause(pgrm->p_input_clock, b_paused, i_date);
vlc_clock_main_ChangePause(pgrm->p_main_clock, i_date, b_paused);
}
}
static void EsOutDecoderChangeDelay( es_out_t *out, es_out_id_t *p_es )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
vlc_tick_t i_delay;
if( p_es->delay != INT64_MAX )
i_delay = p_es->delay; 
else if( p_es->fmt.i_cat == AUDIO_ES )
i_delay = p_sys->i_audio_delay;
else if( p_es->fmt.i_cat == SPU_ES )
i_delay = p_sys->i_spu_delay;
else
return;
if( p_es->p_dec )
vlc_input_decoder_ChangeDelay( p_es->p_dec, i_delay );
if( p_es->p_dec_record )
vlc_input_decoder_ChangeDelay( p_es->p_dec_record, i_delay );
}
static void EsOutProgramsChangeRate( es_out_t *out )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
es_out_pgrm_t *pgrm;
vlc_list_foreach(pgrm, &p_sys->programs, node)
input_clock_ChangeRate(pgrm->p_input_clock, p_sys->rate);
}
static void EsOutFrameNext( es_out_t *out )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
es_out_id_t *p_es_video = NULL, *p_es;
if( p_sys->b_buffering )
{
msg_Warn( p_sys->p_input, "buffering, ignoring 'frame next'" );
return;
}
assert( p_sys->b_paused );
foreach_es_then_es_slaves(p_es)
if( p_es->fmt.i_cat == VIDEO_ES && p_es->p_dec && !p_es_video )
{
p_es_video = p_es;
break;
}
if( !p_es_video )
{
msg_Warn( p_sys->p_input, "No video track selected, ignoring 'frame next'" );
return;
}
vlc_tick_t i_duration;
vlc_input_decoder_FrameNext( p_es_video->p_dec, &i_duration );
msg_Dbg( p_sys->p_input, "EsOutFrameNext consummed %d ms", (int)MS_FROM_VLC_TICK(i_duration) );
if( i_duration <= 0 )
i_duration = VLC_TICK_FROM_MS(40);
if( p_sys->i_buffering_extra_initial <= 0 )
{
vlc_tick_t i_stream_start;
vlc_tick_t i_system_start;
vlc_tick_t i_stream_duration;
vlc_tick_t i_system_duration;
int i_ret;
i_ret = input_clock_GetState( p_sys->p_pgrm->p_input_clock,
&i_stream_start, &i_system_start,
&i_stream_duration, &i_system_duration );
if( i_ret )
return;
p_sys->i_buffering_extra_initial = 1 + i_stream_duration
- p_sys->i_pts_delay
- p_sys->i_pts_jitter
- p_sys->i_tracks_pts_delay; 
p_sys->i_buffering_extra_system =
p_sys->i_buffering_extra_stream = p_sys->i_buffering_extra_initial;
}
const float rate = input_clock_GetRate( p_sys->p_pgrm->p_input_clock );
p_sys->b_buffering = true;
p_sys->i_buffering_extra_system += i_duration;
p_sys->i_buffering_extra_stream = p_sys->i_buffering_extra_initial +
( p_sys->i_buffering_extra_system - p_sys->i_buffering_extra_initial ) * rate;
p_sys->i_preroll_end = -1;
p_sys->i_prev_stream_level = -1;
}
static vlc_tick_t EsOutGetBuffering( es_out_t *out )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
vlc_tick_t i_stream_duration, i_system_start;
if( !p_sys->p_pgrm )
return 0;
else
{
vlc_tick_t i_stream_start, i_system_duration;
if( input_clock_GetState( p_sys->p_pgrm->p_input_clock,
&i_stream_start, &i_system_start,
&i_stream_duration, &i_system_duration ) )
return 0;
}
vlc_tick_t i_delay;
if( p_sys->b_buffering && p_sys->i_buffering_extra_initial <= 0 )
{
i_delay = i_stream_duration;
}
else
{
vlc_tick_t i_system_duration;
if( p_sys->b_paused )
{
i_system_duration = p_sys->i_pause_date - i_system_start;
if( p_sys->i_buffering_extra_initial > 0 )
i_system_duration += p_sys->i_buffering_extra_system - p_sys->i_buffering_extra_initial;
}
else
{
i_system_duration = vlc_tick_now() - i_system_start;
}
const vlc_tick_t i_consumed = i_system_duration * p_sys->rate - i_stream_duration;
i_delay = p_sys->i_pts_delay + p_sys->i_pts_jitter
+ p_sys->i_tracks_pts_delay - i_consumed;
}
if( i_delay < 0 )
return 0;
return i_delay;
}
static void EsOutSendEsEvent(es_out_t *out, es_out_id_t *es, int action,
bool forced)
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
input_thread_t *p_input = p_sys->p_input;
input_SendEventEs(p_input, &(struct vlc_input_event_es) {
.action = action,
.id = &es->id,
.title = es->psz_title ? es->psz_title : "",
.fmt = es->fmt_out.i_cat != UNKNOWN_ES ? &es->fmt_out : &es->fmt,
.forced = forced,
});
}
static inline bool EsOutIsGroupSticky( es_out_t *p_out, input_source_t *source,
int i_group )
{
es_out_sys_t *p_sys = container_of(p_out, es_out_sys_t, out);
return source != input_priv(p_sys->p_input)->master && i_group == 0;
}
static bool EsOutIsProgramVisible( es_out_t *out, input_source_t *source, int i_group )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
return p_sys->i_group_id == 0
|| (p_sys->i_group_id == i_group && p_sys->p_pgrm->source == source);
}
static void EsOutProgramSelect( es_out_t *out, es_out_pgrm_t *p_pgrm )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
input_thread_t *p_input = p_sys->p_input;
es_out_id_t *es;
if( p_sys->p_pgrm == p_pgrm )
return; 
if( p_sys->p_pgrm )
{
es_out_pgrm_t *old = p_sys->p_pgrm;
msg_Dbg( p_input, "unselecting program id=%d", old->i_id );
foreach_es_then_es_slaves(es)
{
if (es->p_pgrm != old)
continue;
if (EsIsSelected(es) && p_sys->i_mode != ES_OUT_MODE_ALL)
EsOutUnselectEs(out, es, true);
if( EsOutIsGroupSticky( out, es->id.source, es->fmt.i_group ) )
es->p_pgrm = NULL; 
else
{
EsOutSendEsEvent( out, es, VLC_INPUT_ES_DELETED, false );
}
}
p_sys->audio.p_main_es = NULL;
p_sys->video.p_main_es = NULL;
p_sys->sub.p_main_es = NULL;
}
msg_Dbg( p_input, "selecting program id=%d", p_pgrm->i_id );
p_pgrm->b_selected = true;
p_sys->p_pgrm = p_pgrm;
input_SendEventProgramSelect( p_input, p_pgrm->i_id );
input_SendEventProgramScrambled( p_input, p_pgrm->i_id, p_pgrm->b_scrambled );
foreach_es_then_es_slaves(es)
{
if (es->p_pgrm == NULL)
{
es->p_pgrm = p_sys->p_pgrm;
}
else if (es->p_pgrm == p_sys->p_pgrm)
{
EsOutSendEsEvent(out, es, VLC_INPUT_ES_ADDED, false);
EsOutUpdateInfo(out, es, NULL);
}
EsOutSelect(out, es, false);
}
input_item_ChangeEPGSource( input_priv(p_input)->p_item, p_pgrm->i_id );
if( p_pgrm->p_meta )
{
input_item_SetESNowPlaying( input_priv(p_input)->p_item,
vlc_meta_Get( p_pgrm->p_meta, vlc_meta_ESNowPlaying ) );
input_item_SetPublisher( input_priv(p_input)->p_item,
vlc_meta_Get( p_pgrm->p_meta, vlc_meta_Publisher ) );
input_item_SetTitle( input_priv(p_input)->p_item,
vlc_meta_Get( p_pgrm->p_meta, vlc_meta_Title ) );
input_SendEventMeta( p_input );
}
}
static es_out_pgrm_t *EsOutProgramAdd( es_out_t *out, input_source_t *source, int i_group )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
input_thread_t *p_input = p_sys->p_input;
if( EsOutIsGroupSticky( out, source, i_group ) )
return NULL;
es_out_pgrm_t *p_pgrm = malloc( sizeof( es_out_pgrm_t ) );
if( !p_pgrm )
return NULL;
p_pgrm->source = input_source_Hold( source );
p_pgrm->i_id = i_group;
p_pgrm->i_es = 0;
p_pgrm->b_selected = false;
p_pgrm->b_scrambled = false;
p_pgrm->p_meta = NULL;
p_pgrm->p_master_clock = NULL;
p_pgrm->p_input_clock = input_clock_New( p_sys->rate );
p_pgrm->p_main_clock = vlc_clock_main_New();
if( !p_pgrm->p_input_clock || !p_pgrm->p_main_clock )
{
if( p_pgrm->p_input_clock )
input_clock_Delete( p_pgrm->p_input_clock );
free( p_pgrm );
return NULL;
}
if( p_sys->b_paused )
input_clock_ChangePause( p_pgrm->p_input_clock, p_sys->b_paused, p_sys->i_pause_date );
const vlc_tick_t pts_delay = p_sys->i_pts_delay + p_sys->i_pts_jitter
+ p_sys->i_tracks_pts_delay;
input_clock_SetJitter( p_pgrm->p_input_clock, pts_delay, p_sys->i_cr_average );
vlc_clock_main_SetInputDejitter( p_pgrm->p_main_clock, pts_delay );
if (input_priv(p_input)->b_low_delay)
vlc_clock_main_SetDejitter(p_pgrm->p_main_clock, 0);
vlc_list_append(&p_pgrm->node, &p_sys->programs);
if( EsOutIsProgramVisible( out, source, i_group ) )
input_SendEventProgramAdd( p_input, i_group, NULL );
if( i_group == p_sys->i_group_id || ( !p_sys->p_pgrm && p_sys->i_group_id == 0 ) )
EsOutProgramSelect( out, p_pgrm );
return p_pgrm;
}
static es_out_pgrm_t *EsOutProgramSearch( es_out_t *p_out, input_source_t *source,
int i_group )
{
es_out_sys_t *p_sys = container_of(p_out, es_out_sys_t, out);
es_out_pgrm_t *pgrm;
vlc_list_foreach(pgrm, &p_sys->programs, node)
if (pgrm->i_id == i_group && pgrm->source == source)
return pgrm;
return NULL;
}
static es_out_pgrm_t *EsOutProgramInsert( es_out_t *p_out, input_source_t *source,
int i_group )
{
es_out_pgrm_t *pgrm = EsOutProgramSearch( p_out, source, i_group );
return pgrm ? pgrm : EsOutProgramAdd( p_out, source, i_group );
}
static int EsOutProgramDel( es_out_t *out, input_source_t *source, int i_group )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
input_thread_t *p_input = p_sys->p_input;
es_out_pgrm_t *p_pgrm = EsOutProgramSearch( out, source, i_group );
if( p_pgrm == NULL )
return VLC_EGENERIC;
if( p_pgrm->i_es )
{
msg_Dbg( p_input, "can't delete program %d which still has %i ES",
i_group, p_pgrm->i_es );
return VLC_EGENERIC;
}
es_out_id_t *es;
foreach_es_then_es_slaves(es)
{
if (es->p_pgrm != p_pgrm)
continue;
assert(EsOutIsGroupSticky( out, es->id.source, es->fmt.i_group));
EsOutUnselectEs(out, es, true);
es->p_pgrm = NULL;
}
vlc_list_remove(&p_pgrm->node);
if( p_sys->p_pgrm == p_pgrm )
p_sys->p_pgrm = NULL;
input_SendEventProgramDel( p_input, i_group );
ProgramDelete( p_pgrm );
return VLC_SUCCESS;
}
static char *EsOutProgramGetMetaName( es_out_pgrm_t *p_pgrm )
{
char *psz = NULL;
if( p_pgrm->p_meta && vlc_meta_Get( p_pgrm->p_meta, vlc_meta_Title ) )
{
if( asprintf( &psz, _("%s [%s %d]"), vlc_meta_Get( p_pgrm->p_meta, vlc_meta_Title ),
_("Program"), p_pgrm->i_id ) == -1 )
return NULL;
}
else
{
if( asprintf( &psz, "%s %d", _("Program"), p_pgrm->i_id ) == -1 )
return NULL;
}
return psz;
}
static char *EsOutProgramGetProgramName( es_out_pgrm_t *p_pgrm )
{
char *psz = NULL;
if( p_pgrm->p_meta && vlc_meta_Get( p_pgrm->p_meta, vlc_meta_Title ) )
{
return strdup( vlc_meta_Get( p_pgrm->p_meta, vlc_meta_Title ) );
}
else
{
if( asprintf( &psz, "%s %d", _("Program"), p_pgrm->i_id ) == -1 )
return NULL;
}
return psz;
}
static char *EsInfoCategoryName( es_out_id_t* es )
{
char *psz_category;
if( asprintf( &psz_category, _("Stream '%s'"), es->id.str_id ) == -1 )
return NULL;
return psz_category;
}
static void EsOutProgramMeta( es_out_t *out, input_source_t *source,
int i_group, const vlc_meta_t *p_meta )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
es_out_pgrm_t *p_pgrm;
input_thread_t *p_input = p_sys->p_input;
input_item_t *p_item = input_priv(p_input)->p_item;
const char *psz_title = NULL;
const char *psz_provider = NULL;
int i;
bool b_has_new_infos = false;
msg_Dbg( p_input, "EsOutProgramMeta: number=%d", i_group );
if( !vlc_meta_Get( p_meta, vlc_meta_Title) &&
!vlc_meta_Get( p_meta, vlc_meta_ESNowPlaying) &&
!vlc_meta_Get( p_meta, vlc_meta_Publisher) )
{
return;
}
if( i_group < 0 )
{
EsOutGlobalMeta( out, p_meta );
return;
}
if( !EsOutIsProgramVisible( out, source, i_group ) )
return;
p_pgrm = EsOutProgramInsert( out, source, i_group );
if( !p_pgrm )
return;
if( p_pgrm->p_meta )
{
const char *psz_current_title = vlc_meta_Get( p_pgrm->p_meta, vlc_meta_Title );
const char *psz_new_title = vlc_meta_Get( p_meta, vlc_meta_Title );
if( (psz_current_title != NULL && psz_new_title != NULL)
? strcmp(psz_new_title, psz_current_title)
: (psz_current_title != psz_new_title) )
{
char *psz_oldinfokey = EsOutProgramGetMetaName( p_pgrm );
if( !input_item_DelInfo( p_item, psz_oldinfokey, NULL ) )
b_has_new_infos = true;
free( psz_oldinfokey );
}
vlc_meta_Delete( p_pgrm->p_meta );
}
p_pgrm->p_meta = vlc_meta_New();
if( p_pgrm->p_meta )
vlc_meta_Merge( p_pgrm->p_meta, p_meta );
if( p_sys->p_pgrm == p_pgrm )
{
EsOutMeta( out, NULL, p_meta );
}
psz_title = vlc_meta_Get( p_meta, vlc_meta_Title);
psz_provider = vlc_meta_Get( p_meta, vlc_meta_Publisher);
if( psz_title && *psz_title )
{
char *psz_text;
if( psz_provider && *psz_provider )
{
if( asprintf( &psz_text, "%s [%s]", psz_title, psz_provider ) < 0 )
psz_text = NULL;
}
else
{
psz_text = strdup( psz_title );
}
if( psz_text )
{
input_SendEventProgramUpdated( p_input, i_group, psz_text );
if( p_sys->p_pgrm == p_pgrm )
input_SendEventProgramSelect( p_input, i_group );
free( psz_text );
}
}
char **ppsz_all_keys = vlc_meta_CopyExtraNames(p_meta );
info_category_t *p_cat = NULL;
if( psz_provider || ( ppsz_all_keys[0] && *ppsz_all_keys[0] ) )
{
char *psz_cat = EsOutProgramGetMetaName( p_pgrm );
if( psz_cat )
p_cat = info_category_New( psz_cat );
free( psz_cat );
}
for( i = 0; ppsz_all_keys[i]; i++ )
{
if( p_cat )
info_category_AddInfo( p_cat, vlc_gettext(ppsz_all_keys[i]), "%s",
vlc_meta_GetExtra( p_meta, ppsz_all_keys[i] ) );
free( ppsz_all_keys[i] );
}
free( ppsz_all_keys );
if( psz_provider )
{
if( p_sys->p_pgrm == p_pgrm )
{
input_item_SetPublisher( input_priv(p_input)->p_item, psz_provider );
input_SendEventMeta( p_input );
}
if( p_cat )
info_category_AddInfo( p_cat, vlc_meta_TypeToLocalizedString(vlc_meta_Publisher),
"%s",psz_provider );
}
if( p_cat )
{
input_item_MergeInfos( p_item, p_cat );
b_has_new_infos = true;
}
if( !input_priv(p_input)->b_preparsing && b_has_new_infos )
input_SendEventMetaInfo( p_input );
}
static void EsOutProgramEpgEvent( es_out_t *out, input_source_t *source,
int i_group, const vlc_epg_event_t *p_event )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
input_thread_t *p_input = p_sys->p_input;
input_item_t *p_item = input_priv(p_input)->p_item;
es_out_pgrm_t *p_pgrm;
if( !EsOutIsProgramVisible( out, source, i_group ) )
return;
p_pgrm = EsOutProgramInsert( out, source, i_group );
if( !p_pgrm )
return;
input_item_SetEpgEvent( p_item, p_event );
}
static void EsOutProgramEpg( es_out_t *out, input_source_t *source,
int i_group, const vlc_epg_t *p_epg )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
input_thread_t *p_input = p_sys->p_input;
input_item_t *p_item = input_priv(p_input)->p_item;
es_out_pgrm_t *p_pgrm;
char *psz_cat;
if( !EsOutIsProgramVisible( out, source, i_group ) )
return;
p_pgrm = EsOutProgramInsert( out, source, i_group );
if( !p_pgrm )
return;
psz_cat = EsOutProgramGetMetaName( p_pgrm );
msg_Dbg( p_input, "EsOutProgramEpg: number=%d name=%s", i_group, psz_cat );
vlc_epg_t epg;
epg = *p_epg;
epg.psz_name = EsOutProgramGetProgramName( p_pgrm );
input_item_SetEpg( p_item, &epg, p_sys->p_pgrm && (p_epg->i_source_id == p_sys->p_pgrm->i_id) );
input_SendEventMetaEpg( p_sys->p_input );
free( epg.psz_name );
if( p_epg->b_present && p_pgrm->p_meta &&
( p_epg->p_current || p_epg->i_event == 0 ) )
{
vlc_meta_SetNowPlaying( p_pgrm->p_meta, NULL );
}
vlc_mutex_lock( &p_item->lock );
for( int i = 0; i < p_item->i_epg; i++ )
{
const vlc_epg_t *p_tmp = p_item->pp_epg[i];
if( p_tmp->b_present && p_tmp->i_source_id == p_pgrm->i_id )
{
const char *psz_name = ( p_tmp->p_current ) ? p_tmp->p_current->psz_name : NULL;
if( !p_pgrm->p_meta )
p_pgrm->p_meta = vlc_meta_New();
if( p_pgrm->p_meta )
vlc_meta_Set( p_pgrm->p_meta, vlc_meta_ESNowPlaying, psz_name );
break;
}
}
vlc_mutex_unlock( &p_item->lock );
if( p_pgrm == p_sys->p_pgrm )
{
const char *psz_nowplaying = p_pgrm->p_meta ?
vlc_meta_Get( p_pgrm->p_meta, vlc_meta_ESNowPlaying ) : NULL;
input_item_SetESNowPlaying( input_priv(p_input)->p_item, psz_nowplaying );
input_SendEventMeta( p_input );
const char *now_playing_tr =
vlc_meta_TypeToLocalizedString(vlc_meta_ESNowPlaying);
int ret;
if( psz_nowplaying )
ret = input_item_AddInfo( p_item, psz_cat, now_playing_tr,
"%s", psz_nowplaying );
else
ret = input_item_DelInfo( p_item, psz_cat, now_playing_tr );
if( ret == VLC_SUCCESS && !input_priv(p_input)->b_preparsing )
input_SendEventMetaInfo( p_input );
}
free( psz_cat );
}
static void EsOutEpgTime( es_out_t *out, int64_t time )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
input_thread_t *p_input = p_sys->p_input;
input_item_t *p_item = input_priv(p_input)->p_item;
input_item_SetEpgTime( p_item, time );
}
static void EsOutProgramUpdateScrambled( es_out_t *p_out, es_out_pgrm_t *p_pgrm )
{
es_out_sys_t *p_sys = container_of(p_out, es_out_sys_t, out);
input_thread_t *p_input = p_sys->p_input;
input_item_t *p_item = input_priv(p_input)->p_item;
es_out_id_t *es;
bool b_scrambled = false;
vlc_list_foreach( es, &p_sys->es, node ) 
if (es->p_pgrm == p_pgrm && es->b_scrambled)
{
b_scrambled = true;
break;
}
if( !p_pgrm->b_scrambled == !b_scrambled )
return;
p_pgrm->b_scrambled = b_scrambled;
char *psz_cat = EsOutProgramGetMetaName( p_pgrm );
int ret;
if( b_scrambled )
ret = input_item_AddInfo( p_item, psz_cat, _("Scrambled"), _("Yes") );
else
ret = input_item_DelInfo( p_item, psz_cat, _("Scrambled") );
free( psz_cat );
if( ret == VLC_SUCCESS && !input_priv(p_input)->b_preparsing )
input_SendEventMetaInfo( p_input );
input_SendEventProgramScrambled( p_input, p_pgrm->i_id, b_scrambled );
}
static void EsOutMeta( es_out_t *p_out, const vlc_meta_t *p_meta, const vlc_meta_t *p_program_meta )
{
es_out_sys_t *p_sys = container_of(p_out, es_out_sys_t, out);
input_thread_t *p_input = p_sys->p_input;
input_item_t *p_item = input_GetItem( p_input );
vlc_mutex_lock( &p_item->lock );
if( p_meta )
vlc_meta_Merge( p_item->p_meta, p_meta );
vlc_mutex_unlock( &p_item->lock );
if( p_meta && (!p_program_meta || vlc_meta_Get( p_program_meta, vlc_meta_Title ) == NULL) &&
vlc_meta_Get( p_meta, vlc_meta_Title ) != NULL )
input_item_SetName( p_item, vlc_meta_Get( p_meta, vlc_meta_Title ) );
const char *psz_arturl = NULL;
char *psz_alloc = NULL;
if( p_program_meta )
psz_arturl = vlc_meta_Get( p_program_meta, vlc_meta_ArtworkURL );
if( psz_arturl == NULL && p_meta )
psz_arturl = vlc_meta_Get( p_meta, vlc_meta_ArtworkURL );
if( psz_arturl == NULL ) 
psz_arturl = psz_alloc = input_item_GetArtURL( p_item );
if( psz_arturl != NULL )
input_item_SetArtURL( p_item, psz_arturl );
if( psz_arturl != NULL && !strncmp( psz_arturl, "attachment://", 13 ) )
{ 
if( input_priv(p_input)->p_sout != NULL )
input_item_SetArtURL( p_item, NULL );
else
input_ExtractAttachmentAndCacheArt( p_input, psz_arturl + 13 );
}
free( psz_alloc );
input_item_SetPreparsed( p_item, true );
input_SendEventMeta( p_input );
}
static void EsOutGlobalMeta( es_out_t *p_out, const vlc_meta_t *p_meta )
{
es_out_sys_t *p_sys = container_of(p_out, es_out_sys_t, out);
EsOutMeta( p_out, p_meta,
(p_sys->p_pgrm && p_sys->p_pgrm->p_meta) ? p_sys->p_pgrm->p_meta : NULL );
}
static void EsOutUpdateEsLanguageTitle(es_out_id_t *es,
const es_format_t *fmt)
{
free( es->psz_title );
free( es->psz_language );
free( es->psz_language_code );
es->psz_language = LanguageGetName( fmt->psz_language );
es->psz_language_code = LanguageGetCode( fmt->psz_language );
es->psz_title = EsGetTitle(es);
}
static void EsOutFillEsFmt(es_out_t *out, es_format_t *fmt)
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
input_thread_t *p_input = p_sys->p_input;
switch( fmt->i_cat )
{
case AUDIO_ES:
{
fmt->i_codec = vlc_fourcc_GetCodecAudio( fmt->i_codec,
fmt->audio.i_bitspersample );
audio_replay_gain_t rg;
memset( &rg, 0, sizeof(rg) );
vlc_mutex_lock( &input_priv(p_input)->p_item->lock );
vlc_audio_replay_gain_MergeFromMeta( &rg, input_priv(p_input)->p_item->p_meta );
vlc_mutex_unlock( &input_priv(p_input)->p_item->lock );
for( int i = 0; i < AUDIO_REPLAY_GAIN_MAX; i++ )
{
if( !fmt->audio_replay_gain.pb_peak[i] )
{
fmt->audio_replay_gain.pb_peak[i] = rg.pb_peak[i];
fmt->audio_replay_gain.pf_peak[i] = rg.pf_peak[i];
}
if( !fmt->audio_replay_gain.pb_gain[i] )
{
fmt->audio_replay_gain.pb_gain[i] = rg.pb_gain[i];
fmt->audio_replay_gain.pf_gain[i] = rg.pf_gain[i];
}
}
break;
}
case VIDEO_ES:
fmt->i_codec = vlc_fourcc_GetCodec( fmt->i_cat, fmt->i_codec );
if( !fmt->video.i_visible_width || !fmt->video.i_visible_height )
{
fmt->video.i_visible_width = fmt->video.i_width;
fmt->video.i_visible_height = fmt->video.i_height;
}
if( fmt->video.i_frame_rate && fmt->video.i_frame_rate_base )
vlc_ureduce( &fmt->video.i_frame_rate,
&fmt->video.i_frame_rate_base,
fmt->video.i_frame_rate,
fmt->video.i_frame_rate_base, 0 );
break;
case SPU_ES:
fmt->i_codec = vlc_fourcc_GetCodec( fmt->i_cat, fmt->i_codec );
break;
default:
break;
}
}
static char *EsOutCreateStrId( es_out_id_t *es, bool stable, const char *id,
es_out_id_t *p_master )
{
struct vlc_memstream ms;
int ret = vlc_memstream_open( &ms );
if( ret != 0 )
return NULL;
if( p_master )
{
vlc_memstream_puts( &ms, p_master->id.str_id );
vlc_memstream_puts( &ms, "/cc/" );
}
else if ( id )
{
assert( strchr( id, ',' ) == NULL);
vlc_memstream_puts( &ms, id );
vlc_memstream_putc( &ms, '/' );
}
switch (es->fmt.i_cat)
{
case VIDEO_ES: vlc_memstream_puts( &ms, "video" ); break;
case AUDIO_ES: vlc_memstream_puts( &ms, "audio" ); break;
case SPU_ES: vlc_memstream_puts( &ms, "spu" ); break;
case DATA_ES: vlc_memstream_puts( &ms, "data" ); break;
default: vlc_memstream_puts( &ms, "unknown" ); break;
}
if( !stable )
vlc_memstream_puts( &ms, "auto/" );
vlc_memstream_printf( &ms, "/%d", es->fmt.i_id );
ret = vlc_memstream_close( &ms );
return ret == 0 ? ms.ptr : NULL;
}
static es_out_id_t *EsOutAddLocked( es_out_t *out, input_source_t *source,
const es_format_t *fmt,
es_out_id_t *p_master )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
input_thread_t *p_input = p_sys->p_input;
assert( source ); 
if( fmt->i_group < 0 )
{
msg_Err( p_input, "invalid group number" );
return NULL;
}
es_out_id_t *es = malloc( sizeof( *es ) );
es_out_pgrm_t *p_pgrm;
if( !es )
return NULL;
es->out = out;
es->id.source = input_source_Hold( source );
if( es_format_Copy( &es->fmt, fmt ) != VLC_SUCCESS )
{
free( es );
return NULL;
}
bool stable;
if( es->fmt.i_id < 0 )
{
es->fmt.i_id = input_source_GetNewAutoId( source );
stable = false;
}
else
stable = true;
if( !es->fmt.i_original_fourcc )
es->fmt.i_original_fourcc = es->fmt.i_codec;
char *str_id =
EsOutCreateStrId( es, stable, input_source_GetStrId(source), p_master );
if( !str_id )
{
es_format_Clean( &es->fmt );
input_source_Release( es->id.source );
free( es );
return NULL;
}
if( !EsOutIsGroupSticky( out, source, fmt->i_group ) )
{
p_pgrm = EsOutProgramInsert( out, source, fmt->i_group );
if( !p_pgrm )
{
es_format_Clean( &es->fmt );
input_source_Release( es->id.source );
free( str_id );
free( es );
return NULL;
}
if( p_pgrm )
p_pgrm->i_es++;
assert( fmt->i_group == 0 || p_pgrm->source == es->id.source );
}
else
p_pgrm = p_sys->p_pgrm; 
es->i_pos = 0;
es_out_id_t *it;
foreach_es_then_es_slaves(it)
if( it->fmt.i_cat == fmt->i_cat && it->fmt.i_group == fmt->i_group )
es->i_pos++;
es->p_pgrm = p_pgrm;
es->id.i_id = es->fmt.i_id;
es->id.i_cat = es->fmt.i_cat;
es->id.str_id = str_id;
es->id.stable = stable;
es_format_Init( &es->fmt_out, UNKNOWN_ES, 0 );
es->b_scrambled = false;
es->b_forced = false;
es->b_terminated = false;
switch( es->fmt.i_cat )
{
case AUDIO_ES:
es->i_channel = p_sys->audio.i_count++;
break;
case VIDEO_ES:
es->i_channel = p_sys->video.i_count++;
break;
case SPU_ES:
es->i_channel = p_sys->sub.i_count++;
break;
default:
es->i_channel = 0;
break;
}
EsOutFillEsFmt( out, &es->fmt );
es->psz_language = LanguageGetName( es->fmt.psz_language ); 
es->psz_language_code = LanguageGetCode( es->fmt.psz_language );
es->psz_title = EsGetTitle(es);
es->p_dec = NULL;
es->p_dec_record = NULL;
es->p_clock = NULL;
es->master = false;
es->cc.type = 0;
es->cc.i_bitmap = 0;
es->p_master = p_master;
es->mouse_event_cb = NULL;
es->mouse_event_userdata = NULL;
es->delay = INT64_MAX;
vlc_list_append(&es->node, es->p_master ? &p_sys->es_slaves : &p_sys->es);
vlc_atomic_rc_init(&es->rc);
if( es->p_pgrm == p_sys->p_pgrm )
EsOutSendEsEvent( out, es, VLC_INPUT_ES_ADDED, false );
EsOutUpdateInfo( out, es, NULL );
EsOutSelect( out, es, false );
return es;
}
static es_out_id_t *EsOutAdd( es_out_t *out, input_source_t *source, const es_format_t *fmt )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
if( !source )
source = p_sys->main_source;
vlc_mutex_lock( &p_sys->lock );
es_out_id_t *es = EsOutAddLocked( out, source, fmt, NULL );
vlc_mutex_unlock( &p_sys->lock );
return es;
}
static bool EsIsSelected( es_out_id_t *es )
{
if( es->p_master )
{
bool b_decode = false;
if( es->p_master->p_dec )
{
int i_channel = EsOutGetClosedCaptionsChannel( &es->fmt );
vlc_input_decoder_GetCcState( es->p_master->p_dec, es->fmt.i_codec,
i_channel, &b_decode );
}
return b_decode;
}
else
{
return es->p_dec != NULL;
}
}
static void ClockUpdate(vlc_tick_t system_ts, vlc_tick_t ts, double rate,
unsigned frame_rate, unsigned frame_rate_base,
void *data)
{
es_out_id_t *es = data;
es_out_sys_t *p_sys = container_of(es->out, es_out_sys_t, out);
input_SendEventOutputClock(p_sys->p_input, &es->id, es->master, system_ts,
ts, rate, frame_rate, frame_rate_base);
}
static void EsOutCreateDecoder( es_out_t *out, es_out_id_t *p_es )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
input_thread_t *p_input = p_sys->p_input;
vlc_input_decoder_t *dec;
static const struct vlc_clock_cbs clock_cbs = {
.on_update = ClockUpdate
};
assert( p_es->p_pgrm );
if( p_es->fmt.i_cat != UNKNOWN_ES
&& p_es->fmt.i_cat == p_sys->i_master_source_cat
&& p_es->p_pgrm->p_master_clock == NULL )
{
p_es->master = true;
p_es->p_pgrm->p_master_clock = p_es->p_clock =
vlc_clock_main_CreateMaster( p_es->p_pgrm->p_main_clock,
&clock_cbs, p_es );
}
else
{
p_es->master = false;
p_es->p_clock = vlc_clock_main_CreateSlave( p_es->p_pgrm->p_main_clock,
p_es->fmt.i_cat,
&clock_cbs, p_es );
}
if( !p_es->p_clock )
{
p_es->master = false;
return;
}
input_thread_private_t *priv = input_priv(p_input);
dec = vlc_input_decoder_New( VLC_OBJECT(p_input), &p_es->fmt, p_es->p_clock,
priv->p_resource, priv->p_sout,
priv->b_thumbnailing, &decoder_cbs, p_es );
if( dec != NULL )
{
vlc_input_decoder_ChangeRate( dec, p_sys->rate );
if( p_sys->b_buffering )
vlc_input_decoder_StartWait( dec );
if( !p_es->p_master && p_sys->p_sout_record )
{
p_es->p_dec_record =
vlc_input_decoder_New( VLC_OBJECT(p_input), &p_es->fmt, NULL,
priv->p_resource, p_sys->p_sout_record,
false, &decoder_cbs, p_es );
if( p_es->p_dec_record && p_sys->b_buffering )
vlc_input_decoder_StartWait( p_es->p_dec_record );
}
if( p_es->mouse_event_cb && p_es->fmt.i_cat == VIDEO_ES )
vlc_input_decoder_SetVoutMouseEvent( dec, p_es->mouse_event_cb,
p_es->mouse_event_userdata );
}
else
{
vlc_clock_Delete( p_es->p_clock );
p_es->p_clock = NULL;
}
p_es->p_dec = dec;
EsOutDecoderChangeDelay( out, p_es );
}
static void EsOutDestroyDecoder( es_out_t *out, es_out_id_t *p_es )
{
VLC_UNUSED(out);
if( !p_es->p_dec )
return;
assert( p_es->p_pgrm );
vlc_input_decoder_Delete( p_es->p_dec );
p_es->p_dec = NULL;
if( p_es->p_pgrm->p_master_clock == p_es->p_clock )
p_es->p_pgrm->p_master_clock = NULL;
vlc_clock_Delete( p_es->p_clock );
p_es->p_clock = NULL;
if( p_es->p_dec_record )
{
vlc_input_decoder_Delete( p_es->p_dec_record );
p_es->p_dec_record = NULL;
}
es_format_Clean( &p_es->fmt_out );
}
static void EsOutSelectEs( es_out_t *out, es_out_id_t *es, bool b_force )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
input_thread_t *p_input = p_sys->p_input;
bool b_thumbnailing = input_priv(p_input)->b_thumbnailing;
if( EsIsSelected( es ) )
{
msg_Warn( p_input, "ES 0x%x is already selected", es->fmt.i_id );
return;
}
if( !es->p_pgrm )
return;
if( es->p_master )
{
int i_channel;
if( !es->p_master->p_dec )
return;
i_channel = EsOutGetClosedCaptionsChannel( &es->fmt );
if( i_channel == -1 ||
vlc_input_decoder_SetCcState( es->p_master->p_dec, es->fmt.i_codec,
i_channel, true ) )
return;
}
else
{
const bool b_sout = input_priv(p_input)->p_sout != NULL;
if( !es->b_forced )
{
if( es->fmt.i_cat == VIDEO_ES || es->fmt.i_cat == SPU_ES )
{
if( !var_GetBool( p_input, b_sout ? "sout-video" : "video" ) )
{
msg_Dbg( p_input, "video is disabled, not selecting ES 0x%x",
es->fmt.i_id );
return;
}
}
else if( es->fmt.i_cat == AUDIO_ES )
{
if( b_thumbnailing
|| !var_GetBool( p_input, b_sout ? "sout-audio" : "audio" ) )
{
msg_Dbg( p_input, "audio is disabled, not selecting ES 0x%x",
es->fmt.i_id );
return;
}
}
if( es->fmt.i_cat == SPU_ES )
{
if( b_thumbnailing
|| !var_GetBool( p_input, b_sout ? "sout-spu" : "spu" ) )
{
msg_Dbg( p_input, "spu is disabled, not selecting ES 0x%x",
es->fmt.i_id );
return;
}
}
}
EsOutCreateDecoder( out, es );
if( es->p_dec == NULL || es->p_pgrm != p_sys->p_pgrm )
return;
}
EsOutSendEsEvent(out, es, VLC_INPUT_ES_SELECTED, b_force);
if( !es->p_master )
{
bool vbi_opaque;
int vbi_page = vlc_input_decoder_GetVbiPage( es->p_dec, &vbi_opaque );
if( vbi_page >= 0 )
{
input_SendEventVbiPage( p_input, vbi_page );
input_SendEventVbiTransparency( p_input, vbi_opaque );
}
}
}
static void EsOutDrainCCChannels( es_out_id_t *parent )
{
uint64_t i_bitmap = parent->cc.i_bitmap;
for( int i = 0; i_bitmap > 0; i++, i_bitmap >>= 1 )
{
if( (i_bitmap & 1) == 0 || !parent->cc.pp_es[i] ||
!parent->cc.pp_es[i]->p_dec )
continue;
vlc_input_decoder_Drain( parent->cc.pp_es[i]->p_dec );
}
}
static void EsDeleteCCChannels( es_out_t *out, es_out_id_t *parent )
{
if( parent->cc.type == 0 )
return;
es_out_id_t *spu_es = EsOutGetSelectedCat( out, SPU_ES );
const int i_spu_id = spu_es ? spu_es->fmt.i_id : -1;
uint64_t i_bitmap = parent->cc.i_bitmap;
for( int i = 0; i_bitmap > 0; i++, i_bitmap >>= 1 )
{
if( (i_bitmap & 1) == 0 || !parent->cc.pp_es[i] )
continue;
if( i_spu_id == parent->cc.pp_es[i]->fmt.i_id )
{
EsOutSendEsEvent(out, parent->cc.pp_es[i], VLC_INPUT_ES_UNSELECTED,
false);
}
EsOutDelLocked( out, parent->cc.pp_es[i] );
}
parent->cc.i_bitmap = 0;
parent->cc.type = 0;
}
static void EsOutUnselectEs( es_out_t *out, es_out_id_t *es, bool b_update )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
input_thread_t *p_input = p_sys->p_input;
if( !EsIsSelected( es ) )
{
msg_Warn( p_input, "ES 0x%x is already unselected", es->fmt.i_id );
return;
}
if( es->p_master )
{
if( es->p_master->p_dec )
{
int i_channel = EsOutGetClosedCaptionsChannel( &es->fmt );
if( i_channel != -1 )
vlc_input_decoder_SetCcState( es->p_master->p_dec, es->fmt.i_codec,
i_channel, false );
}
}
else
{
EsDeleteCCChannels( out, es );
EsOutDestroyDecoder( out, es );
}
if( !b_update )
return;
EsOutSendEsEvent(out, es, VLC_INPUT_ES_UNSELECTED, false);
}
static void EsOutSelect( es_out_t *out, es_out_id_t *es, bool b_force )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
es_out_es_props_t *p_esprops = GetPropsByCat( p_sys, es->fmt.i_cat );
if( !p_sys->b_active ||
( !b_force && es->fmt.i_priority < ES_PRIORITY_SELECTABLE_MIN ) ||
!es->p_pgrm )
{
return;
}
enum es_out_policy_e policy = p_esprops->e_policy;
if( policy == ES_OUT_ES_POLICY_AUTO )
{
if( p_esprops->str_ids && strchr( p_esprops->str_ids, ',' ) != NULL )
policy = ES_OUT_ES_POLICY_SIMULTANEOUS;
else
policy = ES_OUT_ES_POLICY_EXCLUSIVE;
}
bool b_auto_selected = p_esprops->b_autoselect
|| input_source_IsCatAutoselected( es->id.source, es->fmt.i_cat );
bool b_auto_unselect = p_sys->i_mode == ES_OUT_MODE_AUTO &&
policy == ES_OUT_ES_POLICY_EXCLUSIVE &&
p_esprops->p_main_es && p_esprops->p_main_es != es;
if( p_sys->i_mode == ES_OUT_MODE_ALL || b_force )
{
if( !EsIsSelected( es ) )
{
if( b_auto_unselect )
EsOutUnselectEs( out, p_esprops->p_main_es, true );
EsOutSelectEs( out, es, b_force );
}
}
else if( p_sys->i_mode == ES_OUT_MODE_PARTIAL )
{
char *prgms = var_GetNonEmptyString( p_sys->p_input, "programs" );
if( prgms != NULL )
{
char *buf;
for ( const char *prgm = strtok_r( prgms, ",", &buf );
prgm != NULL;
prgm = strtok_r( NULL, ",", &buf ) )
{
if( atoi( prgm ) == es->p_pgrm->i_id || b_force )
{
if( !EsIsSelected( es ) )
EsOutSelectEs( out, es, b_force );
break;
}
}
free( prgms );
}
}
else if( p_sys->i_mode == ES_OUT_MODE_AUTO )
{
const es_out_id_t *wanted_es = NULL;
if( es->p_pgrm != p_sys->p_pgrm || !p_esprops )
return;
if ( p_esprops->str_ids )
{
char *saveptr, *str_ids = strdup( p_esprops->str_ids );
if( str_ids )
{
for( const char *str_id = strtok_r( str_ids, ",", &saveptr );
str_id != NULL && wanted_es != es;
str_id = strtok_r( NULL, ",", &saveptr ) )
{
if( strcmp( str_id, es->id.str_id ) == 0 )
wanted_es = es;
}
}
free( str_ids );
}
else if( p_esprops->i_channel >= 0 )
{
if( p_esprops->i_channel == es->i_channel )
wanted_es = es;
}
else if( p_esprops->ppsz_language )
{
const int i_stop_idx = LanguageArrayIndex( p_esprops->ppsz_language, "none" );
{
int current_es_idx = ( p_esprops->p_main_es == NULL ) ? -1 :
LanguageArrayIndex( p_esprops->ppsz_language,
p_esprops->p_main_es->psz_language_code );
int es_idx = LanguageArrayIndex( p_esprops->ppsz_language,
es->psz_language_code );
if( es_idx >= 0 && (i_stop_idx < 0 || i_stop_idx > es_idx) )
{
if( p_esprops->p_main_es == NULL ||
current_es_idx < 0 || 
es_idx < current_es_idx || 
( es_idx == current_es_idx && 
p_esprops->p_main_es->fmt.i_priority < es->fmt.i_priority ) )
{
wanted_es = es;
}
}
else if( i_stop_idx < 0 ) 
{
if( current_es_idx < 0 ) 
{
if( p_esprops->i_demux_id >= 0 && es->fmt.i_id == p_esprops->i_demux_id )
{
wanted_es = es;
}
else if( p_esprops->p_main_es == NULL ||
es->fmt.i_priority > p_esprops->p_main_es->fmt.i_priority )
{
if( b_auto_selected )
wanted_es = es;
}
}
}
}
}
else if( p_esprops->i_demux_id >= 0 && es->fmt.i_id == p_esprops->i_demux_id )
{
wanted_es = es;
}
else if( p_esprops->p_main_es == NULL ||
es->fmt.i_priority > p_esprops->p_main_es->fmt.i_priority )
{
if( b_auto_selected )
wanted_es = es;
}
if( wanted_es == es && !EsIsSelected( es ) )
{
if( b_auto_unselect )
EsOutUnselectEs( out, p_esprops->p_main_es, true );
EsOutSelectEs( out, es, b_force );
}
}
if( p_esprops && p_sys->i_mode == ES_OUT_MODE_AUTO && EsIsSelected( es ) )
p_esprops->p_main_es = es;
}
static void EsOutSelectListFromProps( es_out_t *out, enum es_format_category_e cat )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
es_out_es_props_t *esprops = GetPropsByCat( p_sys, cat );
if( !esprops || !esprops->str_ids )
return;
char *buffer = malloc( strlen( esprops->str_ids ) + 1);
if( !buffer )
return;
bool unselect_others = false;
es_out_id_t *other;
foreach_es_then_es_slaves( other )
{
if( other->fmt.i_cat != cat )
continue;
bool select = false;
if( !unselect_others )
{
strcpy( buffer, esprops->str_ids );
char *saveptr;
for( const char *str_id = strtok_r( buffer, ",", &saveptr );
str_id != NULL;
str_id = strtok_r( NULL, ",", &saveptr ) )
{
if( strcmp( other->id.str_id, str_id ) == 0 )
{
select = true;
break;
}
}
}
if( !select )
{
if( EsIsSelected( other ) )
EsOutUnselectEs( out, other, other->p_pgrm == p_sys->p_pgrm );
}
else
{
if( !EsIsSelected( other ) )
EsOutSelectEs( out, other, true );
if( esprops->e_policy == ES_OUT_ES_POLICY_EXCLUSIVE )
unselect_others = true;
}
}
free( buffer );
}
static void EsOutSelectList( es_out_t *out, enum es_format_category_e cat,
vlc_es_id_t * const*es_id_list )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
es_out_id_t *other;
es_out_es_props_t *p_esprops = GetPropsByCat( p_sys, cat );
bool unselect_others = false;
foreach_es_then_es_slaves(other)
{
if( other->fmt.i_cat != cat )
continue;
bool select = false;
if( !unselect_others )
{
for( size_t i = 0; ; i++ )
{
vlc_es_id_t *es_id = es_id_list[i];
if( es_id == NULL )
break;
else if( es_id->i_id == other->id.i_id )
{
select = true;
break;
}
}
}
if( !select )
{
if( EsIsSelected( other ) )
EsOutUnselectEs( out, other, other->p_pgrm == p_sys->p_pgrm );
}
else
{
if( !EsIsSelected( other ) )
EsOutSelectEs( out, other, true );
if( p_esprops->e_policy == ES_OUT_ES_POLICY_EXCLUSIVE )
unselect_others = true;
}
}
}
static void EsOutCreateCCChannels( es_out_t *out, vlc_fourcc_t codec, uint64_t i_bitmap,
const char *psz_descfmt, es_out_id_t *parent )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
input_thread_t *p_input = p_sys->p_input;
if( parent->cc.type && parent->cc.type != codec )
return;
uint64_t i_existingbitmap = parent->cc.i_bitmap;
for( int i = 0; i_bitmap > 0; i++, i_bitmap >>= 1, i_existingbitmap >>= 1 )
{
es_format_t fmt;
if( (i_bitmap & 1) == 0 || (i_existingbitmap & 1) )
continue;
msg_Dbg( p_input, "Adding CC track %d for es[%d]", 1+i, parent->fmt.i_id );
es_format_Init( &fmt, SPU_ES, codec );
fmt.subs.cc.i_channel = i;
fmt.i_group = parent->fmt.i_group;
if( asprintf( &fmt.psz_description, psz_descfmt, 1 + i ) == -1 )
fmt.psz_description = NULL;
es_out_id_t **pp_es = &parent->cc.pp_es[i];
*pp_es = EsOutAddLocked( out, parent->p_pgrm->source, &fmt, parent );
es_format_Clean( &fmt );
parent->cc.i_bitmap |= (1ULL << i);
parent->cc.type = codec;
if (p_sys->sub.i_channel == i)
EsOutSelect(out, *pp_es, true);
}
}
static int EsOutSend( es_out_t *out, es_out_id_t *es, block_t *p_block )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
input_thread_t *p_input = p_sys->p_input;
assert( p_block->p_next == NULL );
struct input_stats *stats = input_priv(p_input)->stats;
if( stats != NULL )
{
input_rate_Add( &stats->demux_bitrate, p_block->i_buffer );
if( p_block->i_flags & BLOCK_FLAG_CORRUPTED )
atomic_fetch_add_explicit(&stats->demux_corrupted, 1,
memory_order_relaxed);
if( p_block->i_flags & BLOCK_FLAG_DISCONTINUITY )
atomic_fetch_add_explicit(&stats->demux_discontinuity, 1,
memory_order_relaxed);
}
vlc_mutex_lock( &p_sys->lock );
if( p_sys->i_preroll_end >= 0 )
{
vlc_tick_t i_date = p_block->i_pts;
if( p_block->i_pts == VLC_TICK_INVALID )
i_date = p_block->i_dts;
if( i_date + p_block->i_length < p_sys->i_preroll_end )
p_block->i_flags |= BLOCK_FLAG_PREROLL;
}
if( !es->p_dec )
{
block_Release( p_block );
vlc_mutex_unlock( &p_sys->lock );
return VLC_SUCCESS;
}
#if defined(ENABLE_SOUT)
if( input_priv(p_input)->p_sout )
{
bool pace = sout_instance_ControlsPace(input_priv(p_input)->p_sout);
if( input_priv(p_input)->b_out_pace_control != pace )
msg_Dbg( p_input, "switching to %ssync mode", pace ? "a" : "" );
input_priv(p_input)->b_out_pace_control = pace;
}
#endif
if( es->p_dec_record )
{
block_t *p_dup = block_Duplicate( p_block );
if( p_dup )
vlc_input_decoder_Decode( es->p_dec_record, p_dup,
input_priv(p_input)->b_out_pace_control );
}
vlc_input_decoder_Decode( es->p_dec, p_block,
input_priv(p_input)->b_out_pace_control );
es_format_t fmt_dsc;
vlc_meta_t *p_meta_dsc;
if( vlc_input_decoder_HasFormatChanged( es->p_dec, &fmt_dsc, &p_meta_dsc ) )
{
if (EsOutEsUpdateFmt( out, es, &fmt_dsc) == VLC_SUCCESS)
EsOutSendEsEvent(out, es, VLC_INPUT_ES_UPDATED, false);
EsOutUpdateInfo(out, es, p_meta_dsc);
es_format_Clean( &fmt_dsc );
if( p_meta_dsc )
vlc_meta_Delete( p_meta_dsc );
}
decoder_cc_desc_t desc;
vlc_input_decoder_GetCcDesc( es->p_dec, &desc );
if( var_InheritInteger( p_input, "captions" ) == 708 )
EsOutCreateCCChannels( out, VLC_CODEC_CEA708, desc.i_708_channels,
_("DTVCC Closed captions %u"), es );
EsOutCreateCCChannels( out, VLC_CODEC_CEA608, desc.i_608_channels,
_("Closed captions %u"), es );
vlc_mutex_unlock( &p_sys->lock );
return VLC_SUCCESS;
}
static void
EsOutDrainDecoder( es_out_t *out, es_out_id_t *es )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
assert( es->p_dec );
vlc_input_decoder_Drain( es->p_dec );
EsOutDrainCCChannels( es );
while( !input_Stopped(p_sys->p_input) && !p_sys->b_buffering )
{
if( vlc_input_decoder_IsEmpty( es->p_dec ) &&
( !es->p_dec_record || vlc_input_decoder_IsEmpty( es->p_dec_record ) ))
break;
vlc_tick_sleep(VLC_TICK_FROM_MS(20));
}
}
static void EsOutDelLocked( es_out_t *out, es_out_id_t *es )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
bool b_reselect = false;
es_out_es_props_t *p_esprops = GetPropsByCat( p_sys, es->fmt.i_cat );
if( es->p_dec )
{
EsOutDrainDecoder( out, es );
EsOutUnselectEs( out, es, es->p_pgrm == p_sys->p_pgrm );
}
EsTerminate(es);
if( es->p_pgrm == p_sys->p_pgrm )
EsOutSendEsEvent( out, es, VLC_INPUT_ES_DELETED, false );
EsOutDeleteInfoEs( out, es );
if( EsOutIsGroupSticky( out, es->id.source, es->fmt.i_group ) )
{
assert( es->p_pgrm );
es->p_pgrm->i_es--;
if( es->p_pgrm->i_es == 0 )
msg_Dbg( p_sys->p_input, "Program doesn't contain anymore ES" );
}
if( es->b_scrambled && es->p_pgrm )
EsOutProgramUpdateScrambled( out, es->p_pgrm );
if( p_esprops )
{
if( p_esprops->p_main_es == es )
{
b_reselect = true;
p_esprops->p_main_es = NULL;
}
p_esprops->i_count--;
}
if( b_reselect )
{
es_out_id_t *other;
foreach_es_then_es_slaves(other)
if( es->fmt.i_cat == other->fmt.i_cat )
{
if (EsIsSelected(other))
{
EsOutSendEsEvent(out, es, VLC_INPUT_ES_SELECTED, false);
if( p_esprops->p_main_es == NULL )
p_esprops->p_main_es = other;
}
else
EsOutSelect(out, other, false);
}
}
EsRelease(es);
}
static void EsOutDel( es_out_t *out, es_out_id_t *es )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
vlc_mutex_lock( &p_sys->lock );
EsOutDelLocked( out, es );
vlc_mutex_unlock( &p_sys->lock );
}
static int EsOutVaControlLocked( es_out_t *, input_source_t *, int, va_list );
static int EsOutControlLocked( es_out_t *out, input_source_t *source, int i_query, ... )
{
va_list args;
va_start( args, i_query );
int ret = EsOutVaControlLocked( out, source, i_query, args );
va_end( args );
return ret;
}
static vlc_tick_t EsOutGetTracksDelay(es_out_t *out)
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
vlc_tick_t tracks_delay = 0;
bool has_audio = false;
bool has_spu = false;
es_out_id_t *es;
foreach_es_then_es_slaves(es)
{
if (es->p_dec)
{
if (es->delay != INT64_MAX)
tracks_delay = __MIN(tracks_delay, es->delay);
else if (es->fmt.i_cat == AUDIO_ES)
has_audio = true;
else if (es->fmt.i_cat == SPU_ES)
has_spu = true;
}
}
if (has_audio)
tracks_delay = __MIN(tracks_delay, p_sys->i_audio_delay);
if (has_spu)
tracks_delay = __MIN(tracks_delay, p_sys->i_spu_delay);
return tracks_delay < 0 ? -tracks_delay : 0;
}
static int EsOutVaControlLocked( es_out_t *out, input_source_t *source,
int i_query, va_list args )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
assert( source ); 
switch( i_query )
{
case ES_OUT_SET_ES_STATE:
{
es_out_id_t *es = va_arg( args, es_out_id_t * );
bool b = va_arg( args, int );
if( b && !EsIsSelected( es ) )
{
EsOutSelectEs( out, es, true );
return EsIsSelected( es ) ? VLC_SUCCESS : VLC_EGENERIC;
}
else if( !b && EsIsSelected( es ) )
{
EsOutUnselectEs( out, es, es->p_pgrm == p_sys->p_pgrm );
return VLC_SUCCESS;
}
return VLC_SUCCESS;
}
case ES_OUT_GET_ES_STATE:
{
es_out_id_t *es = va_arg( args, es_out_id_t * );
bool *pb = va_arg( args, bool * );
*pb = EsIsSelected( es );
return VLC_SUCCESS;
}
case ES_OUT_SET_ES_CAT_POLICY:
{
enum es_format_category_e i_cat = va_arg( args, enum es_format_category_e );
enum es_out_policy_e i_pol = va_arg( args, enum es_out_policy_e );
es_out_es_props_t *p_esprops = GetPropsByCat( p_sys, i_cat );
if( p_esprops == NULL )
return VLC_EGENERIC;
p_esprops->e_policy = i_pol;
return VLC_SUCCESS;
}
case ES_OUT_SET_ES:
case ES_OUT_RESTART_ES:
{
#define IGNORE_ES DATA_ES
es_out_id_t *es = va_arg( args, es_out_id_t * ), *other;
enum es_format_category_e i_cat;
if( es == NULL )
i_cat = UNKNOWN_ES;
else if( es == es_cat + AUDIO_ES )
i_cat = AUDIO_ES;
else if( es == es_cat + VIDEO_ES )
i_cat = VIDEO_ES;
else if( es == es_cat + SPU_ES )
i_cat = SPU_ES;
else
{
if (es->b_terminated)
return VLC_EGENERIC;
i_cat = IGNORE_ES;
}
foreach_es_then_es_slaves(other)
{
if( i_cat == IGNORE_ES )
{
if (es == other)
{
if (i_query == ES_OUT_RESTART_ES && es->p_dec != NULL)
{
EsOutDestroyDecoder(out, es);
EsOutCreateDecoder(out, es);
}
else if( i_query == ES_OUT_SET_ES )
{
EsOutSelect(out, es, true);
}
break;
}
}
else if (i_cat == UNKNOWN_ES || other->fmt.i_cat == i_cat)
{
if (EsIsSelected(other))
{
if (i_query == ES_OUT_RESTART_ES)
{
if (other->p_dec != NULL)
{
EsOutDestroyDecoder(out, other);
EsOutCreateDecoder(out, other);
}
}
else
EsOutUnselectEs(out, other, other->p_pgrm == p_sys->p_pgrm);
}
}
}
EsOutStopFreeVout( out );
return VLC_SUCCESS;
}
case ES_OUT_UNSET_ES:
{
es_out_id_t *es = va_arg( args, es_out_id_t * ), *other;
if (es->b_terminated)
return VLC_EGENERIC;
foreach_es_then_es_slaves(other)
{
if (es == other)
{
if (EsIsSelected(other))
{
EsOutUnselectEs(out, other, other->p_pgrm == p_sys->p_pgrm);
EsOutStopFreeVout( out );
return VLC_SUCCESS;
}
break;
}
}
return VLC_EGENERIC;
}
case ES_OUT_SET_ES_DEFAULT:
{
es_out_id_t *es = va_arg( args, es_out_id_t * );
if( es == NULL )
{
p_sys->sub.i_demux_id = -1;
}
else if( es == es_cat + AUDIO_ES )
{
}
else if( es == es_cat + VIDEO_ES )
{
}
else if( es == es_cat + SPU_ES )
{
p_sys->sub.i_demux_id = -1;
}
else
{
if( es->fmt.i_cat == SPU_ES )
p_sys->sub.i_demux_id = es->fmt.i_id;
}
return VLC_SUCCESS;
}
case ES_OUT_SET_PCR:
case ES_OUT_SET_GROUP_PCR:
{
es_out_pgrm_t *p_pgrm = NULL;
int i_group = 0;
vlc_tick_t i_pcr;
if( i_query == ES_OUT_SET_PCR )
{
p_pgrm = p_sys->p_pgrm;
if( !p_pgrm )
p_pgrm = EsOutProgramAdd( out, source, i_group ); 
}
else
{
i_group = va_arg( args, int );
p_pgrm = EsOutProgramInsert( out, source, i_group );
}
if( !p_pgrm )
return VLC_EGENERIC;
i_pcr = va_arg( args, vlc_tick_t );
if( i_pcr == VLC_TICK_INVALID )
{
msg_Err( p_sys->p_input, "Invalid PCR value in ES_OUT_SET_(GROUP_)PCR !" );
return VLC_EGENERIC;
}
const bool b_low_delay = input_priv(p_sys->p_input)->b_low_delay;
bool b_extra_buffering_allowed = !b_low_delay && EsOutIsExtraBufferingAllowed( out );
vlc_tick_t i_late = input_clock_Update(
p_pgrm->p_input_clock, VLC_OBJECT(p_sys->p_input),
input_priv(p_sys->p_input)->b_can_pace_control || p_sys->b_buffering,
b_extra_buffering_allowed,
i_pcr, vlc_tick_now() );
if( !p_sys->p_pgrm )
return VLC_SUCCESS;
if( p_sys->b_buffering )
{
EsOutDecodersStopBuffering( out, false );
}
else if( p_pgrm == p_sys->p_pgrm )
{
if( i_late > 0 && ( !input_priv(p_sys->p_input)->p_sout ||
!input_priv(p_sys->p_input)->b_out_pace_control ) )
{
vlc_tick_t i_clock_total_delay = input_clock_GetJitter( p_pgrm->p_input_clock );
vlc_tick_t i_new_jitter = i_clock_total_delay
- p_sys->i_tracks_pts_delay
- p_sys->i_pts_delay;
const vlc_tick_t i_jitter_max =
VLC_TICK_FROM_MS(var_InheritInteger( p_sys->p_input, "clock-jitter" ));
if( i_new_jitter > i_jitter_max ||
i_clock_total_delay > INPUT_PTS_DELAY_MAX ||
(b_low_delay && i_late > i_jitter_max) )
{
msg_Err( p_sys->p_input,
"ES_OUT_SET_(GROUP_)PCR is called %d ms late (jitter of %d ms ignored)",
(int)MS_FROM_VLC_TICK(i_late),
(int)MS_FROM_VLC_TICK(i_new_jitter) );
i_new_jitter = p_sys->i_pts_jitter;
}
else
{
msg_Err( p_sys->p_input,
"ES_OUT_SET_(GROUP_)PCR is called %d ms late (pts_delay increased to %d ms)",
(int)MS_FROM_VLC_TICK(i_late),
(int)MS_FROM_VLC_TICK(i_clock_total_delay) );
}
EsOutControlLocked( out, source, ES_OUT_RESET_PCR );
EsOutPrivControlLocked( out, ES_OUT_PRIV_SET_JITTER,
p_sys->i_pts_delay, i_new_jitter,
p_sys->i_cr_average );
}
}
return VLC_SUCCESS;
}
case ES_OUT_RESET_PCR:
msg_Dbg( p_sys->p_input, "ES_OUT_RESET_PCR called" );
EsOutChangePosition( out, true );
return VLC_SUCCESS;
case ES_OUT_SET_GROUP:
{
int i = va_arg( args, int );
es_out_pgrm_t *p_pgrm;
vlc_list_foreach(p_pgrm, &p_sys->programs, node)
if( p_pgrm->i_id == i )
{
EsOutProgramSelect( out, p_pgrm );
return VLC_SUCCESS;
}
return VLC_EGENERIC;
}
case ES_OUT_SET_ES_FMT:
{
es_out_id_t *es = va_arg( args, es_out_id_t * );
es_format_t *p_fmt = va_arg( args, es_format_t * );
if( es == NULL || es->fmt.i_cat != p_fmt->i_cat
|| es->fmt.i_id != p_fmt->i_id
|| es->fmt.i_group != p_fmt->i_group )
return VLC_EGENERIC;
es_format_Clean( &es->fmt );
int ret = es_format_Copy( &es->fmt, p_fmt );
if( ret != VLC_SUCCESS )
return ret;
EsOutFillEsFmt( out, &es->fmt );
EsOutUpdateEsLanguageTitle(es, &es->fmt);
const bool b_was_selected = EsIsSelected( es );
if( es->p_dec )
{
EsOutDrainDecoder( out, es );
EsDeleteCCChannels( out, es );
EsOutDestroyDecoder( out, es );
}
if(b_was_selected)
EsOutCreateDecoder( out, es );
EsOutSendEsEvent( out, es, VLC_INPUT_ES_UPDATED, false );
return VLC_SUCCESS;
}
case ES_OUT_SET_ES_SCRAMBLED_STATE:
{
es_out_id_t *es = va_arg( args, es_out_id_t * );
bool b_scrambled = (bool)va_arg( args, int );
if( es->p_pgrm && !es->b_scrambled != !b_scrambled )
{
es->b_scrambled = b_scrambled;
EsOutProgramUpdateScrambled( out, es->p_pgrm );
}
return VLC_SUCCESS;
}
case ES_OUT_SET_NEXT_DISPLAY_TIME:
{
const int64_t i_date = va_arg( args, int64_t );
if( i_date < 0 )
return VLC_EGENERIC;
p_sys->i_preroll_end = i_date;
return VLC_SUCCESS;
}
case ES_OUT_SET_GROUP_META:
{
int i_group = va_arg( args, int );
const vlc_meta_t *p_meta = va_arg( args, const vlc_meta_t * );
EsOutProgramMeta( out, source, i_group, p_meta );
return VLC_SUCCESS;
}
case ES_OUT_SET_GROUP_EPG:
{
int i_group = va_arg( args, int );
const vlc_epg_t *p_epg = va_arg( args, const vlc_epg_t * );
EsOutProgramEpg( out, source, i_group, p_epg );
return VLC_SUCCESS;
}
case ES_OUT_SET_GROUP_EPG_EVENT:
{
int i_group = va_arg( args, int );
const vlc_epg_event_t *p_evt = va_arg( args, const vlc_epg_event_t * );
EsOutProgramEpgEvent( out, source, i_group, p_evt );
return VLC_SUCCESS;
}
case ES_OUT_SET_EPG_TIME:
{
int64_t i64 = va_arg( args, int64_t );
EsOutEpgTime( out, i64 );
return VLC_SUCCESS;
}
case ES_OUT_DEL_GROUP:
{
int i_group = va_arg( args, int );
return EsOutProgramDel( out, source, i_group );
}
case ES_OUT_SET_META:
{
const vlc_meta_t *p_meta = va_arg( args, const vlc_meta_t * );
EsOutGlobalMeta( out, p_meta );
return VLC_SUCCESS;
}
case ES_OUT_GET_EMPTY:
{
bool *pb = va_arg( args, bool* );
*pb = EsOutDecodersIsEmpty( out );
return VLC_SUCCESS;
}
case ES_OUT_GET_PCR_SYSTEM:
{
if( p_sys->b_buffering )
return VLC_EGENERIC;
es_out_pgrm_t *p_pgrm = p_sys->p_pgrm;
if( !p_pgrm )
return VLC_EGENERIC;
vlc_tick_t *pi_system = va_arg( args, vlc_tick_t *);
vlc_tick_t *pi_delay = va_arg( args, vlc_tick_t *);
input_clock_GetSystemOrigin( p_pgrm->p_input_clock, pi_system, pi_delay );
return VLC_SUCCESS;
}
case ES_OUT_MODIFY_PCR_SYSTEM:
{
if( p_sys->b_buffering )
return VLC_EGENERIC;
es_out_pgrm_t *p_pgrm = p_sys->p_pgrm;
if( !p_pgrm )
return VLC_EGENERIC;
const bool b_absolute = va_arg( args, int );
const vlc_tick_t i_system = va_arg( args, vlc_tick_t );
input_clock_ChangeSystemOrigin( p_pgrm->p_input_clock, b_absolute, i_system );
return VLC_SUCCESS;
}
case ES_OUT_POST_SUBNODE:
{
input_thread_t *input = p_sys->p_input;
input_item_node_t *node = va_arg(args, input_item_node_t *);
input_SendEventParsing(input, node);
input_item_node_Delete(node);
return VLC_SUCCESS;
}
case ES_OUT_VOUT_SET_MOUSE_EVENT:
{
es_out_id_t *p_es = va_arg( args, es_out_id_t * );
if( !p_es || p_es->fmt.i_cat != VIDEO_ES )
return VLC_EGENERIC;
p_es->mouse_event_cb = va_arg( args, vlc_mouse_event );
p_es->mouse_event_userdata = va_arg( args, void * );
if( p_es->p_dec )
vlc_input_decoder_SetVoutMouseEvent( p_es->p_dec,
p_es->mouse_event_cb, p_es->mouse_event_userdata );
return VLC_SUCCESS;
}
case ES_OUT_VOUT_ADD_OVERLAY:
{
es_out_id_t *p_es = va_arg( args, es_out_id_t * );
subpicture_t *sub = va_arg( args, subpicture_t * );
size_t *channel = va_arg( args, size_t * );
if( p_es && p_es->fmt.i_cat == VIDEO_ES && p_es->p_dec )
return vlc_input_decoder_AddVoutOverlay( p_es->p_dec, sub, channel );
return VLC_EGENERIC;
}
case ES_OUT_VOUT_DEL_OVERLAY:
{
es_out_id_t *p_es = va_arg( args, es_out_id_t * );
size_t channel = va_arg( args, size_t );
if( p_es && p_es->fmt.i_cat == VIDEO_ES && p_es->p_dec )
return vlc_input_decoder_DelVoutOverlay( p_es->p_dec, channel );
return VLC_EGENERIC;
}
case ES_OUT_SPU_SET_HIGHLIGHT:
{
es_out_id_t *p_es = va_arg( args, es_out_id_t * );
const vlc_spu_highlight_t *spu_hl =
va_arg( args, const vlc_spu_highlight_t * );
if( p_es && p_es->fmt.i_cat == SPU_ES && p_es->p_dec )
return vlc_input_decoder_SetSpuHighlight( p_es->p_dec, spu_hl );
return VLC_EGENERIC;
}
default: vlc_assert_unreachable();
}
}
static int EsOutVaPrivControlLocked( es_out_t *out, int query, va_list args )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
switch (query)
{
case ES_OUT_PRIV_SET_MODE:
{
const int i_mode = va_arg( args, int );
assert( i_mode == ES_OUT_MODE_NONE || i_mode == ES_OUT_MODE_ALL ||
i_mode == ES_OUT_MODE_AUTO || i_mode == ES_OUT_MODE_PARTIAL ||
i_mode == ES_OUT_MODE_END );
if (i_mode != ES_OUT_MODE_NONE && !p_sys->b_active && !vlc_list_is_empty(&p_sys->es))
{
es_out_id_t *p_es;
bool found = false;
foreach_es_then_es_slaves(p_es)
if( p_es->fmt.i_cat == VIDEO_ES && !found )
{
found = true;
break;
}
if (!found)
EsOutStopFreeVout( out );
}
p_sys->b_active = i_mode != ES_OUT_MODE_NONE;
p_sys->i_mode = i_mode;
es_out_id_t *es;
foreach_es_then_es_slaves(es)
{
if (EsIsSelected(es))
EsOutUnselectEs(out, es, es->p_pgrm == p_sys->p_pgrm);
}
foreach_es_then_es_slaves(es)
{
EsOutSelect(out, es, false);
}
if( i_mode == ES_OUT_MODE_END )
EsOutTerminate( out );
return VLC_SUCCESS;
}
case ES_OUT_PRIV_SET_ES:
case ES_OUT_PRIV_UNSET_ES:
case ES_OUT_PRIV_RESTART_ES:
{
vlc_es_id_t *es_id = va_arg( args, vlc_es_id_t * );
es_out_id_t *es = vlc_es_id_get_out( es_id );
int new_query;
switch( query )
{
case ES_OUT_PRIV_SET_ES: new_query = ES_OUT_SET_ES; break;
case ES_OUT_PRIV_UNSET_ES: new_query = ES_OUT_UNSET_ES; break;
case ES_OUT_PRIV_RESTART_ES: new_query = ES_OUT_RESTART_ES; break;
default: vlc_assert_unreachable();
}
return EsOutControlLocked( out, p_sys->main_source, new_query, es );
}
case ES_OUT_PRIV_SET_ES_CAT_IDS:
{
enum es_format_category_e cat = va_arg( args, enum es_format_category_e );
const char *str_ids = va_arg( args, const char * );
es_out_es_props_t *p_esprops = GetPropsByCat( p_sys, cat );
free( p_esprops->str_ids );
p_esprops->str_ids = str_ids ? strdup( str_ids ) : NULL;
if( p_esprops->str_ids )
{
EsOutSelectListFromProps( out, cat );
}
return VLC_SUCCESS;
}
case ES_OUT_PRIV_GET_WAKE_UP:
{
vlc_tick_t *pi_wakeup = va_arg( args, vlc_tick_t* );
*pi_wakeup = EsOutGetWakeup( out );
return VLC_SUCCESS;
}
case ES_OUT_PRIV_SET_ES_LIST:
{
enum es_format_category_e cat = va_arg( args, enum es_format_category_e );
vlc_es_id_t *const*es_id_list = va_arg( args, vlc_es_id_t ** );
EsOutSelectList( out, cat, es_id_list );
return VLC_SUCCESS;
}
case ES_OUT_PRIV_STOP_ALL_ES:
{
es_out_id_t *es;
int count = 0;
foreach_es_then_es_slaves(es)
count++;
vlc_es_id_t **selected_es = vlc_alloc(count + 1, sizeof(vlc_es_id_t *));
if (!selected_es)
return VLC_ENOMEM;
*va_arg(args, vlc_es_id_t ***) = selected_es;
foreach_es_then_es_slaves(es)
{
if (EsIsSelected(es))
{
EsOutDestroyDecoder(out, es);
*selected_es++ = vlc_es_id_Hold(&es->id);
}
*selected_es = NULL;
}
return VLC_SUCCESS;
}
case ES_OUT_PRIV_START_ALL_ES:
{
vlc_es_id_t **selected_es = va_arg( args, vlc_es_id_t ** );
vlc_es_id_t **selected_es_it = selected_es;
for( vlc_es_id_t *id = *selected_es_it; id != NULL;
id = *++selected_es_it )
{
EsOutCreateDecoder( out, vlc_es_id_get_out( id ) );
vlc_es_id_Release( id );
}
free(selected_es);
EsOutStopFreeVout( out );
return VLC_SUCCESS;
}
case ES_OUT_PRIV_GET_BUFFERING:
{
bool *pb = va_arg( args, bool* );
*pb = p_sys->b_buffering;
return VLC_SUCCESS;
}
case ES_OUT_PRIV_SET_ES_DELAY:
{
vlc_es_id_t *es_id = va_arg( args, vlc_es_id_t * );
es_out_id_t *es = vlc_es_id_get_out( es_id );
const vlc_tick_t delay = va_arg(args, vlc_tick_t);
EsOutSetEsDelay(out, es, delay);
return VLC_SUCCESS;
}
case ES_OUT_PRIV_SET_DELAY:
{
const int i_cat = va_arg( args, int );
const vlc_tick_t i_delay = va_arg( args, vlc_tick_t );
EsOutSetDelay( out, i_cat, i_delay );
return VLC_SUCCESS;
}
case ES_OUT_PRIV_SET_RECORD_STATE:
{
bool b = va_arg( args, int );
return EsOutSetRecord( out, b );
}
case ES_OUT_PRIV_SET_PAUSE_STATE:
{
const bool b_source_paused = (bool)va_arg( args, int );
const bool b_paused = (bool)va_arg( args, int );
const vlc_tick_t i_date = va_arg( args, vlc_tick_t );
assert( !b_source_paused == !b_paused );
EsOutChangePause( out, b_paused, i_date );
return VLC_SUCCESS;
}
case ES_OUT_PRIV_SET_RATE:
{
const float src_rate = va_arg( args, double );
const float rate = va_arg( args, double );
assert( src_rate == rate );
EsOutChangeRate( out, rate );
return VLC_SUCCESS;
}
case ES_OUT_PRIV_SET_FRAME_NEXT:
EsOutFrameNext( out );
return VLC_SUCCESS;
case ES_OUT_PRIV_SET_TIMES:
{
double f_position = va_arg( args, double );
vlc_tick_t i_time = va_arg( args, vlc_tick_t );
vlc_tick_t i_normal_time = va_arg( args, vlc_tick_t );
vlc_tick_t i_length = va_arg( args, vlc_tick_t );
if( !p_sys->b_buffering )
{
vlc_tick_t i_delay;
if( !input_priv(p_sys->p_input)->p_sout ||
!input_priv(p_sys->p_input)->b_out_pace_control )
i_delay = EsOutGetBuffering( out );
else
i_delay = 0;
if( i_time != VLC_TICK_INVALID )
{
i_time -= i_delay;
if( i_time < VLC_TICK_0 )
i_time = VLC_TICK_0;
}
if( i_length != VLC_TICK_INVALID )
f_position -= (double)i_delay / i_length;
if( f_position < 0 )
f_position = 0;
assert( i_normal_time >= VLC_TICK_0 );
input_SendEventTimes( p_sys->p_input, f_position, i_time,
i_normal_time, i_length );
}
else
input_SendEventTimes( p_sys->p_input, 0.0, VLC_TICK_INVALID,
i_normal_time, i_length );
return VLC_SUCCESS;
}
case ES_OUT_PRIV_SET_JITTER:
{
vlc_tick_t i_pts_delay = va_arg( args, vlc_tick_t );
vlc_tick_t i_pts_jitter = va_arg( args, vlc_tick_t );
int i_cr_average = va_arg( args, int );
es_out_pgrm_t *pgrm;
const vlc_tick_t i_tracks_pts_delay = EsOutGetTracksDelay(out);
bool b_change_clock =
i_pts_delay != p_sys->i_pts_delay ||
i_pts_jitter != p_sys->i_pts_jitter ||
i_cr_average != p_sys->i_cr_average ||
i_tracks_pts_delay != p_sys->i_tracks_pts_delay;
assert( i_pts_jitter >= 0 );
p_sys->i_pts_delay = i_pts_delay;
p_sys->i_pts_jitter = i_pts_jitter;
p_sys->i_cr_average = i_cr_average;
p_sys->i_tracks_pts_delay = i_tracks_pts_delay;
if (b_change_clock)
{
i_pts_delay += i_pts_jitter + i_tracks_pts_delay;
vlc_list_foreach(pgrm, &p_sys->programs, node)
{
input_clock_SetJitter(pgrm->p_input_clock, i_pts_delay,
i_cr_average);
vlc_clock_main_SetInputDejitter(pgrm->p_main_clock, i_pts_delay);
}
}
return VLC_SUCCESS;
}
case ES_OUT_PRIV_GET_GROUP_FORCED:
{
int *pi_group = va_arg( args, int * );
*pi_group = p_sys->i_group_id;
return VLC_SUCCESS;
}
case ES_OUT_PRIV_SET_EOS:
{
es_out_id_t *id;
foreach_es_then_es_slaves(id)
if (id->p_dec != NULL)
vlc_input_decoder_Drain(id->p_dec);
return VLC_SUCCESS;
}
case ES_OUT_PRIV_SET_VBI_PAGE:
case ES_OUT_PRIV_SET_VBI_TRANSPARENCY:
{
vlc_es_id_t *es_id = va_arg( args, vlc_es_id_t * );
es_out_id_t *es = vlc_es_id_get_out( es_id );
assert(es);
if( !es->p_dec )
return VLC_EGENERIC;
int ret;
if( query == ES_OUT_PRIV_SET_VBI_PAGE )
{
unsigned page = va_arg( args, unsigned );
ret = vlc_input_decoder_SetVbiPage( es->p_dec, page );
if( ret == VLC_SUCCESS )
input_SendEventVbiPage( p_sys->p_input, page );
}
else
{
bool opaque = va_arg( args, int );
ret = vlc_input_decoder_SetVbiOpaque( es->p_dec, opaque );
if( ret == VLC_SUCCESS )
input_SendEventVbiTransparency( p_sys->p_input, opaque );
}
return ret;
}
default: vlc_assert_unreachable();
}
}
static int EsOutControl( es_out_t *out, input_source_t *source,
int i_query, va_list args )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
int i_ret;
if( !source )
source = p_sys->main_source;
vlc_mutex_lock( &p_sys->lock );
i_ret = EsOutVaControlLocked( out, source, i_query, args );
vlc_mutex_unlock( &p_sys->lock );
return i_ret;
}
static int EsOutPrivControlLocked( es_out_t *out, int i_query, ... )
{
va_list args;
va_start( args, i_query );
int ret = EsOutVaPrivControlLocked( out, i_query, args );
va_end( args );
return ret;
}
static int EsOutPrivControl( es_out_t *out, int query, va_list args )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
vlc_mutex_lock( &p_sys->lock );
int ret = EsOutVaPrivControlLocked( out, query, args );
vlc_mutex_unlock( &p_sys->lock );
return ret;
}
static const struct es_out_callbacks es_out_cbs =
{
.add = EsOutAdd,
.send = EsOutSend,
.del = EsOutDel,
.control = EsOutControl,
.destroy = EsOutDelete,
.priv_control = EsOutPrivControl,
};
static char *LanguageGetName( const char *psz_code )
{
const iso639_lang_t *pl;
if( psz_code == NULL || !strcmp( psz_code, "und" ) )
{
return strdup( "" );
}
if( strlen( psz_code ) == 2 )
{
pl = GetLang_1( psz_code );
}
else if( strlen( psz_code ) == 3 )
{
pl = GetLang_2B( psz_code );
if( !strcmp( pl->psz_iso639_1, "??" ) )
{
pl = GetLang_2T( psz_code );
}
}
else
{
char *lang = LanguageGetCode( psz_code );
pl = GetLang_1( lang );
free( lang );
}
if( !strcmp( pl->psz_iso639_1, "??" ) )
{
return strdup( psz_code );
}
else
{
return strdup( vlc_gettext(pl->psz_eng_name) );
}
}
static char *LanguageGetCode( const char *psz_lang )
{
const iso639_lang_t *pl;
if( psz_lang == NULL || *psz_lang == '\0' )
return strdup("??");
for( pl = p_languages; pl->psz_eng_name != NULL; pl++ )
{
if( !strcasecmp( pl->psz_eng_name, psz_lang ) ||
!strcasecmp( pl->psz_iso639_1, psz_lang ) ||
!strcasecmp( pl->psz_iso639_2T, psz_lang ) ||
!strcasecmp( pl->psz_iso639_2B, psz_lang ) )
return strdup( pl->psz_iso639_1 );
}
return strdup("??");
}
static char **LanguageSplit( const char *psz_langs )
{
char *psz_dup;
char *psz_parser;
char **ppsz = NULL;
int i_psz = 0;
if( psz_langs == NULL ) return NULL;
psz_parser = psz_dup = strdup(psz_langs);
while( psz_parser && *psz_parser )
{
char *psz;
char *psz_code;
psz = strchr(psz_parser, ',' );
if( psz ) *psz++ = '\0';
if( !strcmp( psz_parser, "any" ) )
{
TAB_APPEND( i_psz, ppsz, strdup("any") );
}
else if( !strcmp( psz_parser, "none" ) )
{
TAB_APPEND( i_psz, ppsz, strdup("none") );
}
else
{
psz_code = LanguageGetCode( psz_parser );
if( strcmp( psz_code, "??" ) )
{
TAB_APPEND( i_psz, ppsz, psz_code );
}
else
{
free( psz_code );
}
}
psz_parser = psz;
}
if( i_psz )
{
TAB_APPEND( i_psz, ppsz, NULL );
}
free( psz_dup );
return ppsz;
}
static int LanguageArrayIndex( char **ppsz_langs, const char *psz_lang )
{
if( !ppsz_langs || !psz_lang )
return -1;
for( int i = 0; ppsz_langs[i]; i++ )
{
if( !strcasecmp( ppsz_langs[i], psz_lang ) ||
( !strcasecmp( ppsz_langs[i], "any" ) && strcasecmp( psz_lang, "none") ) )
return i;
if( !strcasecmp( ppsz_langs[i], "none" ) )
break;
}
return -1;
}
static int EsOutEsUpdateFmt(es_out_t *out, es_out_id_t *es,
const es_format_t *fmt)
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
input_thread_t *p_input = p_sys->p_input;
assert(es->fmt.i_cat == fmt->i_cat);
es_format_t update = *fmt;
update.i_id = es->fmt.i_id;
update.i_group = es->fmt.i_group;
update.i_priority = es->fmt.i_priority;
update.i_codec = es->fmt.i_codec;
update.i_original_fourcc = es->fmt.i_original_fourcc;
if (update.psz_language == NULL)
update.psz_language = es->fmt.psz_language;
if (update.psz_description == NULL)
update.psz_description = es->fmt.psz_description;
if (update.i_cat == SPU_ES && update.subs.psz_encoding == NULL)
update.subs.psz_encoding = es->fmt.subs.psz_encoding;
if (update.i_extra_languages == 0)
{
assert(update.p_extra_languages == NULL);
update.i_extra_languages = es->fmt.i_extra_languages;
update.p_extra_languages = es->fmt.p_extra_languages;
}
es_format_Clean(&es->fmt_out);
int ret = es_format_Copy(&es->fmt_out, &update);
if (ret == VLC_SUCCESS)
{
EsOutUpdateEsLanguageTitle(es, &es->fmt_out);
input_item_UpdateTracksInfo(input_GetItem(p_input), &es->fmt_out);
}
return ret;
}
static void info_category_AddCodecInfo( info_category_t* p_cat,
const char *psz_info,
vlc_fourcc_t i_fourcc,
const char *psz_description )
{
const char *ps_fcc = (const char*)&i_fourcc;
if( psz_description && *psz_description )
info_category_AddInfo( p_cat, psz_info, "%s (%.4s)",
psz_description, ps_fcc );
else if ( i_fourcc != VLC_FOURCC(0,0,0,0) )
info_category_AddInfo( p_cat, psz_info, "%.4s", ps_fcc );
}
static void EsOutUpdateInfo( es_out_t *out, es_out_id_t *es, const vlc_meta_t *p_meta )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
input_thread_t *p_input = p_sys->p_input;
input_item_t *p_item = input_priv(p_input)->p_item;
const es_format_t *p_fmt_es = &es->fmt;
const es_format_t *fmt = es->fmt_out.i_cat != UNKNOWN_ES ? &es->fmt_out : &es->fmt;
input_item_UpdateTracksInfo( p_item , fmt );
char* psz_cat = EsInfoCategoryName( es );
if( unlikely( !psz_cat ) )
return;
info_category_t* p_cat = info_category_New( psz_cat );
free( psz_cat );
if( unlikely( !p_cat ) )
return;
if( es->psz_title )
info_category_AddInfo( p_cat, _("Title"), "%s", es->psz_title );
const vlc_fourcc_t i_codec_fourcc = p_fmt_es->i_original_fourcc;
const char *psz_codec_description =
vlc_fourcc_GetDescription( p_fmt_es->i_cat, i_codec_fourcc );
info_category_AddCodecInfo( p_cat, _("Codec"),
i_codec_fourcc, psz_codec_description );
if( es->psz_language && *es->psz_language )
info_category_AddInfo( p_cat, _("Language"), "%s",
es->psz_language );
if( fmt->psz_description && *fmt->psz_description )
info_category_AddInfo( p_cat, _("Description"), "%s",
fmt->psz_description );
switch( fmt->i_cat )
{
case AUDIO_ES:
info_category_AddInfo( p_cat, _("Type"), _("Audio") );
if( p_fmt_es->audio.i_physical_channels )
info_category_AddInfo( p_cat, _("Channels"), "%s",
vlc_gettext( aout_FormatPrintChannels( &p_fmt_es->audio ) ) );
if( p_fmt_es->audio.i_rate )
info_category_AddInfo( p_cat, _("Sample rate"), _("%u Hz"),
p_fmt_es->audio.i_rate );
unsigned int i_orgbps = p_fmt_es->audio.i_bitspersample;
if( i_orgbps == 0 )
i_orgbps = aout_BitsPerSample( p_fmt_es->i_codec );
if( i_orgbps != 0 )
info_category_AddInfo( p_cat, _("Bits per sample"), "%u",
i_orgbps );
if( fmt->audio.i_format &&
fmt->audio.i_format != p_fmt_es->i_codec )
{
psz_codec_description = vlc_fourcc_GetDescription( AUDIO_ES,
fmt->audio.i_format );
info_category_AddCodecInfo( p_cat, _("Decoded format"),
fmt->audio.i_format,
psz_codec_description );
}
if( fmt->audio.i_physical_channels &&
fmt->audio.i_physical_channels != p_fmt_es->audio.i_physical_channels )
info_category_AddInfo( p_cat, _("Decoded channels"), "%s",
vlc_gettext( aout_FormatPrintChannels( &fmt->audio ) ) );
if( fmt->audio.i_rate &&
fmt->audio.i_rate != p_fmt_es->audio.i_rate )
info_category_AddInfo( p_cat, _("Decoded sample rate"), _("%u Hz"),
fmt->audio.i_rate );
unsigned i_outbps = fmt->audio.i_bitspersample;
if( i_outbps == 0 )
i_outbps = aout_BitsPerSample( fmt->i_codec );
if( i_outbps != 0 && i_outbps != i_orgbps )
info_category_AddInfo( p_cat, _("Decoded bits per sample"), "%u",
i_outbps );
if( fmt->i_bitrate != 0 )
{
info_category_AddInfo( p_cat, _("Bitrate"), _("%u kb/s"),
fmt->i_bitrate / 1000 );
}
for( int i = 0; i < AUDIO_REPLAY_GAIN_MAX; i++ )
{
const audio_replay_gain_t *p_rg = &fmt->audio_replay_gain;
if( !p_rg->pb_gain[i] )
continue;
const char *psz_name;
if( i == AUDIO_REPLAY_GAIN_TRACK )
psz_name = _("Track replay gain");
else
psz_name = _("Album replay gain");
info_category_AddInfo( p_cat, psz_name, _("%.2f dB"),
p_rg->pf_gain[i] );
}
break;
case VIDEO_ES:
info_category_AddInfo( p_cat, _("Type"), _("Video") );
if( fmt->video.i_visible_width > 0 &&
fmt->video.i_visible_height > 0 )
info_category_AddInfo( p_cat, _("Video resolution"), "%ux%u",
fmt->video.i_visible_width,
fmt->video.i_visible_height);
if( fmt->video.i_width > 0 && fmt->video.i_height > 0 )
info_category_AddInfo( p_cat, _("Buffer dimensions"), "%ux%u",
fmt->video.i_width, fmt->video.i_height );
if( fmt->video.i_frame_rate > 0 &&
fmt->video.i_frame_rate_base > 0 )
{
if( fmt->video.i_frame_rate_base == 1 )
info_category_AddInfo( p_cat, _("Frame rate"), "%u",
fmt->video.i_frame_rate );
else
info_category_AddInfo( p_cat, _("Frame rate"), "%.6f",
(double)fmt->video.i_frame_rate
/ (double)fmt->video.i_frame_rate_base );
}
if( fmt->i_codec != p_fmt_es->i_codec )
{
psz_codec_description = vlc_fourcc_GetDescription( VIDEO_ES,
fmt->i_codec );
info_category_AddCodecInfo( p_cat, _("Decoded format"),
fmt->i_codec,
psz_codec_description );
}
{
static const char orient_names[][13] = {
N_("Top left"), N_("Left top"),
N_("Right bottom"), N_("Top right"),
N_("Bottom left"), N_("Bottom right"),
N_("Left bottom"), N_("Right top"),
};
info_category_AddInfo( p_cat, _("Orientation"), "%s",
vlc_gettext(orient_names[fmt->video.orientation]) );
}
if( fmt->video.primaries != COLOR_PRIMARIES_UNDEF )
{
static const char primaries_names[][32] = {
[COLOR_PRIMARIES_UNDEF] = N_("Undefined"),
[COLOR_PRIMARIES_BT601_525] =
N_("ITU-R BT.601 (525 lines, 60 Hz)"),
[COLOR_PRIMARIES_BT601_625] =
N_("ITU-R BT.601 (625 lines, 50 Hz)"),
[COLOR_PRIMARIES_BT709] = "ITU-R BT.709",
[COLOR_PRIMARIES_BT2020] = "ITU-R BT.2020",
[COLOR_PRIMARIES_DCI_P3] = "DCI/P3 D65",
[COLOR_PRIMARIES_BT470_M] = "ITU-R BT.470 M",
};
static_assert(ARRAY_SIZE(primaries_names) == COLOR_PRIMARIES_MAX+1,
"Color primiaries table mismatch");
info_category_AddInfo( p_cat, _("Color primaries"), "%s",
vlc_gettext(primaries_names[fmt->video.primaries]) );
}
if( fmt->video.transfer != TRANSFER_FUNC_UNDEF )
{
static const char func_names[][20] = {
[TRANSFER_FUNC_UNDEF] = N_("Undefined"),
[TRANSFER_FUNC_LINEAR] = N_("Linear"),
[TRANSFER_FUNC_SRGB] = "sRGB",
[TRANSFER_FUNC_BT470_BG] = "ITU-R BT.470 BG",
[TRANSFER_FUNC_BT470_M] = "ITU-R BT.470 M",
[TRANSFER_FUNC_BT709] = "ITU-R BT.709",
[TRANSFER_FUNC_SMPTE_ST2084] = "SMPTE ST2084 (PQ)",
[TRANSFER_FUNC_SMPTE_240] = "SMPTE 240M",
[TRANSFER_FUNC_HLG] = N_("Hybrid Log-Gamma"),
};
static_assert(ARRAY_SIZE(func_names) == TRANSFER_FUNC_MAX+1,
"Transfer functions table mismatch");
info_category_AddInfo( p_cat, _("Color transfer function"), "%s",
vlc_gettext(func_names[fmt->video.transfer]) );
}
if( fmt->video.space != COLOR_SPACE_UNDEF )
{
static const char space_names[][16] = {
[COLOR_SPACE_UNDEF] = N_("Undefined"),
[COLOR_SPACE_BT601] = "ITU-R BT.601",
[COLOR_SPACE_BT709] = "ITU-R BT.709",
[COLOR_SPACE_BT2020] = "ITU-R BT.2020",
};
static_assert(ARRAY_SIZE(space_names) == COLOR_SPACE_MAX+1,
"Color space table mismatch");
info_category_AddInfo( p_cat, _("Color space"), "%s",
vlc_gettext(space_names[fmt->video.space]) );
}
if( fmt->video.color_range != COLOR_RANGE_UNDEF )
{
static const char range_names[][16] = {
[COLOR_RANGE_UNDEF] = N_("Undefined"),
[COLOR_RANGE_FULL] = N_("Full"),
[COLOR_RANGE_LIMITED] = N_("Limited"),
};
static_assert(ARRAY_SIZE(range_names) == COLOR_RANGE_MAX+1,
"Color range table mismatch");
info_category_AddInfo( p_cat, _("Color Range"), "%s",
vlc_gettext(range_names[fmt->video.color_range]) );
}
if( fmt->video.chroma_location != CHROMA_LOCATION_UNDEF )
{
static const char c_loc_names[][16] = {
[CHROMA_LOCATION_UNDEF] = N_("Undefined"),
[CHROMA_LOCATION_LEFT] = N_("Left"),
[CHROMA_LOCATION_CENTER] = N_("Center"),
[CHROMA_LOCATION_TOP_LEFT] = N_("Top Left"),
[CHROMA_LOCATION_TOP_CENTER] = N_("Top Center"),
[CHROMA_LOCATION_BOTTOM_LEFT] =N_("Bottom Left"),
[CHROMA_LOCATION_BOTTOM_CENTER] = N_("Bottom Center"),
};
static_assert(ARRAY_SIZE(c_loc_names) == CHROMA_LOCATION_MAX+1,
"Chroma location table mismatch");
info_category_AddInfo( p_cat, _("Chroma location"), "%s",
vlc_gettext(c_loc_names[fmt->video.chroma_location]) );
}
if( fmt->video.multiview_mode != MULTIVIEW_2D )
{
static const char c_multiview_names[][18] = {
[MULTIVIEW_2D] = N_("2D"),
[MULTIVIEW_STEREO_SBS] = N_("Side-By-Side"),
[MULTIVIEW_STEREO_TB] = N_("Top-Bottom"),
[MULTIVIEW_STEREO_ROW] = N_("Row Sequential"),
[MULTIVIEW_STEREO_COL] = N_("Column Sequential"),
[MULTIVIEW_STEREO_FRAME] =N_("Frame Sequential"),
[MULTIVIEW_STEREO_CHECKERBOARD] = N_("Checkboard"),
};
static_assert(ARRAY_SIZE(c_multiview_names) == MULTIVIEW_STEREO_MAX+1,
"Multiview format table mismatch");
info_category_AddInfo( p_cat, _("Stereo Mode"), "%s",
vlc_gettext(c_multiview_names[fmt->video.multiview_mode]) );
info_category_AddInfo( p_cat, _("First Stereo Eye"),
vlc_gettext(fmt->video.b_multiview_right_eye_first ?
N_("Right") : N_("Left")) );
}
if( fmt->video.projection_mode != PROJECTION_MODE_RECTANGULAR )
{
const char *psz_loc_name = NULL;
switch (fmt->video.projection_mode)
{
case PROJECTION_MODE_RECTANGULAR:
psz_loc_name = N_("Rectangular");
break;
case PROJECTION_MODE_EQUIRECTANGULAR:
psz_loc_name = N_("Equirectangular");
break;
case PROJECTION_MODE_CUBEMAP_LAYOUT_STANDARD:
psz_loc_name = N_("Cubemap");
break;
default:
vlc_assert_unreachable();
break;
}
info_category_AddInfo( p_cat, _("Projection"), "%s",
vlc_gettext(psz_loc_name) );
info_category_AddInfo( p_cat, vlc_pgettext("ViewPoint", "Yaw"),
"%.2f", fmt->video.pose.yaw );
info_category_AddInfo( p_cat, vlc_pgettext("ViewPoint", "Pitch"),
"%.2f", fmt->video.pose.pitch );
info_category_AddInfo( p_cat, vlc_pgettext("ViewPoint", "Roll"),
"%.2f", fmt->video.pose.roll );
info_category_AddInfo( p_cat,
vlc_pgettext("ViewPoint", "Field of view"),
"%.2f", fmt->video.pose.fov );
}
if ( fmt->video.mastering.max_luminance )
{
info_category_AddInfo( p_cat, _("Max. mastering luminance"), "%.4f cd/m²",
fmt->video.mastering.max_luminance / 10000.f );
}
if ( fmt->video.mastering.min_luminance )
{
info_category_AddInfo( p_cat, _("Min. mastering luminance"), "%.4f cd/m²",
fmt->video.mastering.min_luminance / 10000.f );
}
if ( fmt->video.mastering.primaries[4] &&
fmt->video.mastering.primaries[5] )
{
float x = (float)fmt->video.mastering.primaries[4] / 50000.f;
float y = (float)fmt->video.mastering.primaries[5] / 50000.f;
info_category_AddInfo( p_cat, _("Mastering Primary R"), "x=%.4f y=%.4f", x, y );
}
if ( fmt->video.mastering.primaries[0] &&
fmt->video.mastering.primaries[1] )
{
float x = (float)fmt->video.mastering.primaries[0] / 50000.f;
float y = (float)fmt->video.mastering.primaries[1] / 50000.f;
info_category_AddInfo( p_cat, _("Mastering Primary G"), "x=%.4f y=%.4f", x, y );
}
if ( fmt->video.mastering.primaries[2] &&
fmt->video.mastering.primaries[3] )
{
float x = (float)fmt->video.mastering.primaries[2] / 50000.f;
float y = (float)fmt->video.mastering.primaries[3] / 50000.f;
info_category_AddInfo( p_cat, _("Mastering Primary B"), "x=%.4f y=%.4f", x, y );
}
if ( fmt->video.mastering.white_point[0] &&
fmt->video.mastering.white_point[1] )
{
float x = (float)fmt->video.mastering.white_point[0] / 50000.f;
float y = (float)fmt->video.mastering.white_point[1] / 50000.f;
info_category_AddInfo( p_cat, _("Mastering White point"), "x=%.4f y=%.4f", x, y );
}
if ( fmt->video.lighting.MaxCLL )
{
info_category_AddInfo( p_cat, "MaxCLL", "%" PRIu16 " cd/m²",
fmt->video.lighting.MaxCLL );
}
if ( fmt->video.lighting.MaxFALL )
{
info_category_AddInfo( p_cat, "MaxFALL", "%" PRIu16 " cd/m²",
fmt->video.lighting.MaxFALL );
}
break;
case SPU_ES:
info_category_AddInfo( p_cat, _("Type"), _("Subtitle") );
break;
default:
break;
}
if( p_meta )
{
char **ppsz_all_keys = vlc_meta_CopyExtraNames( p_meta );
for( int i = 0; ppsz_all_keys && ppsz_all_keys[i]; i++ )
{
char *psz_key = ppsz_all_keys[i];
const char *psz_value = vlc_meta_GetExtra( p_meta, psz_key );
if( psz_value )
info_category_AddInfo( p_cat, vlc_gettext(psz_key), "%s",
vlc_gettext(psz_value) );
free( psz_key );
}
free( ppsz_all_keys );
}
input_item_ReplaceInfos( p_item, p_cat );
if( !input_priv(p_input)->b_preparsing )
input_SendEventMetaInfo( p_input );
}
static void EsOutDeleteInfoEs( es_out_t *out, es_out_id_t *es )
{
es_out_sys_t *p_sys = container_of(out, es_out_sys_t, out);
input_thread_t *p_input = p_sys->p_input;
input_item_t *p_item = input_priv(p_input)->p_item;
char* psz_info_category;
if( likely( psz_info_category = EsInfoCategoryName( es ) ) )
{
int ret = input_item_DelInfo( p_item, psz_info_category, NULL );
free( psz_info_category );
if( ret == VLC_SUCCESS && !input_priv(p_input)->b_preparsing )
input_SendEventMetaInfo( p_input );
}
}
es_out_id_t *vlc_es_id_get_out(vlc_es_id_t *id)
{
return container_of(id, es_out_id_t, id);
}
vlc_es_id_t *
vlc_es_id_Hold(vlc_es_id_t *id)
{
EsHold(vlc_es_id_get_out(id));
return id;
}
void
vlc_es_id_Release(vlc_es_id_t *id)
{
EsRelease(vlc_es_id_get_out(id));
}
int
vlc_es_id_GetInputId(vlc_es_id_t *id)
{
return id->i_id;
}
bool
vlc_es_id_IsStrIdStable(vlc_es_id_t *id)
{
return id->stable;
}
const char *
vlc_es_id_GetStrId(vlc_es_id_t *id)
{
return id->str_id;
}
enum es_format_category_e
vlc_es_id_GetCat(vlc_es_id_t *id)
{
return id->i_cat;
}
const input_source_t *vlc_es_id_GetSource(vlc_es_id_t *id)
{
return id->source;
}
