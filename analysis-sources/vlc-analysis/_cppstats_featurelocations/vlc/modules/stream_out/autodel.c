

























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_sout.h>
#include <vlc_block.h>




static int Open ( vlc_object_t * );
static void Close ( vlc_object_t * );

#define SOUT_CFG_PREFIX "sout-autodel-"

vlc_module_begin ()
set_shortname( N_("Autodel"))
set_description( N_("Automatically add/delete input streams"))
set_capability( "sout filter", 50 )
add_shortcut( "autodel" )
set_callbacks( Open, Close )
vlc_module_end ()





static void *Add( sout_stream_t *, const es_format_t * );
static void Del( sout_stream_t *, void * );
static int Send( sout_stream_t *, void *, block_t * );

typedef struct sout_stream_id_sys_t sout_stream_id_sys_t;
struct sout_stream_id_sys_t
{
sout_stream_id_sys_t *id;
es_format_t fmt;
vlc_tick_t i_last;
bool b_error;
};

typedef struct
{
sout_stream_id_sys_t **pp_es;
int i_es_num;
} sout_stream_sys_t;




static int Open( vlc_object_t *p_this )
{
sout_stream_t *p_stream = (sout_stream_t*)p_this;
sout_stream_sys_t *p_sys;

p_sys = malloc( sizeof( sout_stream_sys_t ) );

p_sys->pp_es = NULL;
p_sys->i_es_num = 0;

p_stream->pf_add = Add;
p_stream->pf_del = Del;
p_stream->pf_send = Send;

p_stream->p_sys = p_sys;

return VLC_SUCCESS;
}




static void Close( vlc_object_t * p_this )
{
sout_stream_t *p_stream = (sout_stream_t*)p_this;
sout_stream_sys_t *p_sys = (sout_stream_sys_t *)p_stream->p_sys;

free( p_sys );
}

static void *Add( sout_stream_t *p_stream, const es_format_t *p_fmt )
{
sout_stream_sys_t *p_sys = (sout_stream_sys_t *)p_stream->p_sys;
sout_stream_id_sys_t *p_es = malloc( sizeof(sout_stream_id_sys_t) );
if( unlikely(p_es == NULL) )
return NULL;

es_format_Copy( &p_es->fmt, p_fmt );

p_es->id = NULL;
p_es->i_last = VLC_TICK_INVALID;
p_es->b_error = false;
TAB_APPEND( p_sys->i_es_num, p_sys->pp_es, p_es );

return p_es;
}

static void Del( sout_stream_t *p_stream, void *_p_es )
{
sout_stream_sys_t *p_sys = (sout_stream_sys_t *)p_stream->p_sys;
sout_stream_id_sys_t *p_es = (sout_stream_id_sys_t *)_p_es;

if( p_es->id != NULL )
sout_StreamIdDel( p_stream->p_next, p_es->id );

TAB_REMOVE( p_sys->i_es_num, p_sys->pp_es, p_es );
es_format_Clean( &p_es->fmt );
free( p_es );
}

static int Send( sout_stream_t *p_stream, void *_p_es, block_t *p_buffer )
{
sout_stream_sys_t *p_sys = (sout_stream_sys_t *)p_stream->p_sys;
sout_stream_id_sys_t *p_es = (sout_stream_id_sys_t *)_p_es;
vlc_tick_t i_current = vlc_tick_now();
int i;

p_es->i_last = p_buffer->i_dts;
if ( !p_es->id && !p_es->b_error )
{
p_es->id = sout_StreamIdAdd( p_stream->p_next, &p_es->fmt );
if ( p_es->id == NULL )
{
p_es->b_error = true;
msg_Err( p_stream, "couldn't create chain for id %d",
p_es->fmt.i_id );
}
}

if ( !p_es->b_error )
sout_StreamIdSend( p_stream->p_next, p_es->id, p_buffer );
else
block_ChainRelease( p_buffer );

for ( i = 0; i < p_sys->i_es_num; i++ )
{
if ( p_sys->pp_es[i]->id != NULL
&& (p_sys->pp_es[i]->fmt.i_cat == VIDEO_ES
|| p_sys->pp_es[i]->fmt.i_cat == AUDIO_ES)
&& p_sys->pp_es[i]->i_last < i_current )
{
sout_StreamIdDel( p_stream->p_next, p_sys->pp_es[i]->id );
p_sys->pp_es[i]->id = NULL;
}
}

return VLC_SUCCESS;
}
