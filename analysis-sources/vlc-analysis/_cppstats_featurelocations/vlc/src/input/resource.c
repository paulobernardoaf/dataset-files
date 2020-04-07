
























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <assert.h>

#include <vlc_common.h>
#include <vlc_atomic.h>
#include <vlc_decoder.h>
#include <vlc_vout.h>
#include <vlc_spu.h>
#include <vlc_aout.h>
#include <vlc_sout.h>
#include "../libvlc.h"
#include "../stream_output/stream_output.h"
#include "../audio_output/aout_internal.h"
#include "../video_output/vout_internal.h"
#include "input_interface.h"
#include "event.h"
#include "resource.h"

struct input_resource_t
{
vlc_atomic_rc_t rc;

vlc_object_t *p_parent;



vlc_mutex_t lock;


input_thread_t *p_input;

sout_instance_t *p_sout;
vout_thread_t *p_vout_free;
vout_thread_t *p_vout_dummy;





vlc_mutex_t lock_hold;




vout_thread_t **pp_vout;
int i_vout;

bool b_aout_busy;
audio_output_t *p_aout;
};


static void DestroySout( input_resource_t *p_resource )
{
#if defined(ENABLE_SOUT)
if( p_resource->p_sout )
sout_DeleteInstance( p_resource->p_sout );
#endif
p_resource->p_sout = NULL;
}

static sout_instance_t *RequestSout( input_resource_t *p_resource,
sout_instance_t *p_sout, const char *psz_sout )
{
#if defined(ENABLE_SOUT)
if( !p_sout && !psz_sout )
{
if( p_resource->p_sout )
{
msg_Dbg( p_resource->p_sout, "destroying useless sout" );
DestroySout( p_resource );
}
return NULL;
}

assert( !p_sout || ( !p_resource->p_sout && !psz_sout ) );


if( p_resource->p_sout &&
strcmp( p_resource->p_sout->psz_sout, psz_sout ) )
{
msg_Dbg( p_resource->p_parent, "destroying unusable sout" );
DestroySout( p_resource );
}

if( psz_sout )
{
if( p_resource->p_sout )
{

msg_Dbg( p_resource->p_parent, "reusing sout" );
msg_Dbg( p_resource->p_parent, "you probably want to use gather stream_out" );
}
else
{

p_resource->p_sout = sout_NewInstance( p_resource->p_parent, psz_sout );
}

p_sout = p_resource->p_sout;
p_resource->p_sout = NULL;

return p_sout;
}
else
{
p_resource->p_sout = p_sout;
return NULL;
}
#else
VLC_UNUSED (p_resource); VLC_UNUSED (p_sout); VLC_UNUSED (psz_sout);
return NULL;
#endif
}


static void DestroyVout( input_resource_t *p_resource )
{
assert( p_resource->i_vout == 0 || p_resource->p_vout_free == p_resource->pp_vout[0] );

if( p_resource->p_vout_free )
{
vlc_mutex_lock(&p_resource->lock_hold);
TAB_REMOVE(p_resource->i_vout, p_resource->pp_vout, p_resource->p_vout_free);
vlc_mutex_unlock(&p_resource->lock_hold);

vout_Close( p_resource->p_vout_free );
p_resource->p_vout_free = NULL;
}
}

static void DisplayVoutTitle( input_resource_t *p_resource,
vout_thread_t *p_vout )
{
if( p_resource->p_input == NULL )
return;



input_item_t *p_item = input_GetItem( p_resource->p_input );

char *psz_nowplaying = input_item_GetNowPlayingFb( p_item );
if( psz_nowplaying && *psz_nowplaying )
{
vout_DisplayTitle( p_vout, psz_nowplaying );
}
else
{
char *psz_artist = input_item_GetArtist( p_item );
char *psz_name = input_item_GetTitle( p_item );

if( !psz_name || *psz_name == '\0' )
{
free( psz_name );
psz_name = input_item_GetName( p_item );
}
if( psz_artist && *psz_artist )
{
char *psz_string;
if( asprintf( &psz_string, "%s - %s", psz_name, psz_artist ) != -1 )
{
vout_DisplayTitle( p_vout, psz_string );
free( psz_string );
}
}
else if( psz_name )
{
vout_DisplayTitle( p_vout, psz_name );
}
free( psz_name );
free( psz_artist );
}
free( psz_nowplaying );
}


audio_output_t *input_resource_GetAout( input_resource_t *p_resource )
{
audio_output_t *p_aout;

vlc_mutex_lock( &p_resource->lock_hold );
p_aout = p_resource->p_aout;

if( p_aout == NULL || p_resource->b_aout_busy )
{
msg_Dbg( p_resource->p_parent, "creating audio output" );
vlc_mutex_unlock( &p_resource->lock_hold );

p_aout = aout_New( p_resource->p_parent );
if( p_aout == NULL )
return NULL; 

vlc_mutex_lock( &p_resource->lock_hold );
if( p_resource->p_aout == NULL )
p_resource->p_aout = p_aout;
}
else
msg_Dbg( p_resource->p_parent, "reusing audio output" );

if( p_resource->p_aout == p_aout )
{
assert( !p_resource->b_aout_busy );
p_resource->b_aout_busy = true;
}
vlc_mutex_unlock( &p_resource->lock_hold );
return p_aout;
}

void input_resource_PutAout( input_resource_t *p_resource,
audio_output_t *p_aout )
{
assert( p_aout != NULL );

vlc_mutex_lock( &p_resource->lock_hold );
if( p_aout == p_resource->p_aout )
{
assert( p_resource->b_aout_busy );
p_resource->b_aout_busy = false;
msg_Dbg( p_resource->p_parent, "keeping audio output" );
p_aout = NULL;
}
else
msg_Dbg( p_resource->p_parent, "destroying extra audio output" );
vlc_mutex_unlock( &p_resource->lock_hold );

if( p_aout != NULL )
aout_Destroy( p_aout );
}

audio_output_t *input_resource_HoldAout( input_resource_t *p_resource )
{
audio_output_t *p_aout;

vlc_mutex_lock( &p_resource->lock_hold );
p_aout = p_resource->p_aout;
if( p_aout != NULL )
aout_Hold(p_aout);
vlc_mutex_unlock( &p_resource->lock_hold );

return p_aout;
}


input_resource_t *input_resource_New( vlc_object_t *p_parent )
{
input_resource_t *p_resource = calloc( 1, sizeof(*p_resource) );
if( !p_resource )
return NULL;

p_resource->p_vout_dummy = vout_CreateDummy(p_parent);
if( !p_resource->p_vout_dummy )
{
free( p_resource );
return NULL;
}

vlc_atomic_rc_init( &p_resource->rc );
p_resource->p_parent = p_parent;
vlc_mutex_init( &p_resource->lock );
vlc_mutex_init( &p_resource->lock_hold );
return p_resource;
}

void input_resource_Release( input_resource_t *p_resource )
{
if( !vlc_atomic_rc_dec( &p_resource->rc ) )
return;

DestroySout( p_resource );
DestroyVout( p_resource );
if( p_resource->p_aout != NULL )
aout_Destroy( p_resource->p_aout );

vout_Release( p_resource->p_vout_dummy );
free( p_resource );
}

input_resource_t *input_resource_Hold( input_resource_t *p_resource )
{
vlc_atomic_rc_inc( &p_resource->rc );
return p_resource;
}

void input_resource_SetInput( input_resource_t *p_resource, input_thread_t *p_input )
{
vlc_mutex_lock( &p_resource->lock );

if( p_resource->p_input && !p_input )
assert( p_resource->i_vout == 0 || p_resource->p_vout_free == p_resource->pp_vout[0] );


p_resource->p_input = p_input;

vlc_mutex_unlock( &p_resource->lock );
}

static void input_resource_PutVoutLocked(input_resource_t *p_resource,
vout_thread_t *vout)
{
assert(vout != NULL);
vlc_mutex_lock(&p_resource->lock_hold);
assert( p_resource->i_vout > 0 );

if (p_resource->pp_vout[0] == vout)
{
vlc_mutex_unlock(&p_resource->lock_hold);

assert(p_resource->p_vout_free == NULL || p_resource->p_vout_free == vout);
msg_Dbg(p_resource->p_parent, "saving a free vout");
p_resource->p_vout_free = vout;
}
else
{
msg_Dbg(p_resource->p_parent, "destroying vout (already one saved or active)");
#if !defined(NDEBUG)
{
int index;
TAB_FIND(p_resource->i_vout, p_resource->pp_vout, vout, index);
assert(index >= 0);
}
#endif

TAB_REMOVE(p_resource->i_vout, p_resource->pp_vout, vout);
vlc_mutex_unlock(&p_resource->lock_hold);
vout_Stop(vout);
vout_Close(vout);
}
}

void input_resource_PutVout(input_resource_t *p_resource,
vout_thread_t *vout)
{
vlc_mutex_lock( &p_resource->lock );
input_resource_PutVoutLocked( p_resource, vout );
vlc_mutex_unlock( &p_resource->lock );
}

vout_thread_t *input_resource_GetVoutDecoderDevice(input_resource_t *p_resource,
vout_thread_t *cfg_vout,
enum vlc_vout_order *order,
vlc_decoder_device **pp_dec_dev)
{
vout_thread_t *vout;
vlc_mutex_lock( &p_resource->lock );

if (cfg_vout == NULL) {
cfg_vout = p_resource->p_vout_free;
p_resource->p_vout_free = NULL;

if (cfg_vout == NULL) {



vlc_object_t *parent = p_resource->i_vout == 0 ?
VLC_OBJECT(p_resource->p_vout_dummy) : p_resource->p_parent;
cfg_vout = vout = vout_Create(parent);
if (vout == NULL)
goto out;

vlc_mutex_lock(&p_resource->lock_hold);
*order = p_resource->i_vout == 0 ? VLC_VOUT_ORDER_PRIMARY
: VLC_VOUT_ORDER_SECONDARY;
TAB_APPEND(p_resource->i_vout, p_resource->pp_vout, vout);
vlc_mutex_unlock(&p_resource->lock_hold);
} else
{

*order = VLC_VOUT_ORDER_PRIMARY;
msg_Dbg(p_resource->p_parent, "trying to reuse free vout");
}
}
else
{
vlc_mutex_lock(&p_resource->lock_hold);
assert(p_resource->i_vout > 0);
*order = p_resource->pp_vout[0] == cfg_vout ? VLC_VOUT_ORDER_PRIMARY
: VLC_VOUT_ORDER_SECONDARY;

if (p_resource->p_vout_free == cfg_vout)
p_resource->p_vout_free = NULL;
vlc_mutex_unlock(&p_resource->lock_hold);
}

#if !defined(NDEBUG)
{
int index;
TAB_FIND(p_resource->i_vout, p_resource->pp_vout, cfg_vout, index );
assert(index >= 0);
}
#endif

if (pp_dec_dev)
{
*pp_dec_dev = vout_GetDevice(cfg_vout);
}

vout = cfg_vout;

out:
vlc_mutex_unlock( &p_resource->lock );
return vout;
}

int input_resource_StartVout(input_resource_t *p_resource,
vlc_video_context *vctx,
const vout_configuration_t *cfg)
{
vlc_mutex_lock( &p_resource->lock );
if (vout_Request(cfg, vctx, p_resource->p_input)) {
input_resource_PutVoutLocked(p_resource, cfg->vout);
vlc_mutex_unlock(&p_resource->lock);
return -1;
}

DisplayVoutTitle(p_resource, cfg->vout);


if (p_resource->p_input != NULL)
{
input_control_param_t param = { .viewpoint = cfg->fmt->pose };
input_ControlPush(p_resource->p_input, INPUT_CONTROL_SET_INITIAL_VIEWPOINT,
&param);
}
vlc_mutex_unlock( &p_resource->lock );
return 0;
}

vout_thread_t *input_resource_HoldVout( input_resource_t *p_resource )
{
vlc_mutex_lock( &p_resource->lock_hold );

vout_thread_t *p_vout = p_resource->i_vout > 0 ? p_resource->pp_vout[0] : NULL;
if( p_vout )
vout_Hold(p_vout);

vlc_mutex_unlock( &p_resource->lock_hold );

return p_vout;
}

vout_thread_t *input_resource_HoldDummyVout( input_resource_t *p_resource )
{
return vout_Hold(p_resource->p_vout_dummy);
}

void input_resource_HoldVouts( input_resource_t *p_resource, vout_thread_t ***ppp_vout,
size_t *pi_vout )
{
vout_thread_t **pp_vout;

*pi_vout = 0;
*ppp_vout = NULL;

vlc_mutex_lock( &p_resource->lock_hold );

if( p_resource->i_vout <= 0 )
goto exit;

pp_vout = vlc_alloc( p_resource->i_vout, sizeof(*pp_vout) );
if( !pp_vout )
goto exit;

*ppp_vout = pp_vout;
*pi_vout = p_resource->i_vout;

for( int i = 0; i < p_resource->i_vout; i++ )
{
pp_vout[i] = p_resource->pp_vout[i];
vout_Hold(pp_vout[i]);
}

exit:
vlc_mutex_unlock( &p_resource->lock_hold );
}

void input_resource_StopFreeVout(input_resource_t *p_resource)
{
vlc_mutex_lock(&p_resource->lock);
if (p_resource->p_vout_free != NULL)
{
msg_Dbg(p_resource->p_vout_free, "stop free vout");
vout_Stop(p_resource->p_vout_free);
}
vlc_mutex_unlock(&p_resource->lock);
}


sout_instance_t *input_resource_RequestSout( input_resource_t *p_resource, sout_instance_t *p_sout, const char *psz_sout )
{
vlc_mutex_lock( &p_resource->lock );
sout_instance_t *p_ret = RequestSout( p_resource, p_sout, psz_sout );
vlc_mutex_unlock( &p_resource->lock );

return p_ret;
}
void input_resource_TerminateSout( input_resource_t *p_resource )
{
input_resource_RequestSout( p_resource, NULL, NULL );
}
