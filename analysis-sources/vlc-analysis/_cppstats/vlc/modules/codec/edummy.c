#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_codec.h>
static int OpenEncoder( vlc_object_t * );
static void CloseEncoder( vlc_object_t * );
vlc_module_begin ()
set_shortname( N_("Dummy") )
set_description( N_("Dummy encoder") )
set_capability( "encoder", 0 )
set_callbacks( OpenEncoder, CloseEncoder )
add_shortcut( "dummy" )
vlc_module_end ()
static block_t *EncodeVideo( encoder_t *p_enc, picture_t *p_pict );
static block_t *EncodeAudio( encoder_t *p_enc, block_t *p_buf );
static int OpenEncoder( vlc_object_t *p_this )
{
encoder_t *p_enc = (encoder_t *)p_this;
p_enc->pf_encode_video = EncodeVideo;
p_enc->pf_encode_audio = EncodeAudio;
return VLC_SUCCESS;
}
static block_t *EncodeVideo( encoder_t *p_enc, picture_t *p_pict )
{
VLC_UNUSED(p_enc); VLC_UNUSED(p_pict);
return NULL;
}
static block_t *EncodeAudio( encoder_t *p_enc, block_t *p_buf )
{
VLC_UNUSED(p_enc); VLC_UNUSED(p_buf);
return NULL;
}
static void CloseEncoder( vlc_object_t *p_this )
{
VLC_UNUSED(p_this);
}
