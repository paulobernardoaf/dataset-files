

























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <stddef.h>
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_aout.h>
#include <vlc_aout_volume.h>




static int Create( vlc_object_t * );




vlc_module_begin ()
set_category( CAT_AUDIO )
set_subcategory( SUBCAT_AUDIO_MISC )
set_description( N_("Single precision audio volume") )
set_capability( "audio volume", 10 )
set_callback( Create )
vlc_module_end ()




static void FilterFL32( audio_volume_t *p_volume, block_t *p_buffer,
float f_multiplier )
{
if( f_multiplier == 1.f )
return; 

float *p = (float *)p_buffer->p_buffer;
for( size_t i = p_buffer->i_buffer / sizeof(*p); i > 0; i-- )
*(p++) *= f_multiplier;

(void) p_volume;
}

static void FilterFL64( audio_volume_t *p_volume, block_t *p_buffer,
float f_multiplier )
{
double *p = (double *)p_buffer->p_buffer;
double mult = f_multiplier;
if( mult == 1. )
return; 

for( size_t i = p_buffer->i_buffer / sizeof(*p); i > 0; i-- )
*(p++) *= mult;

(void) p_volume;
}




static int Create( vlc_object_t *p_this )
{
audio_volume_t *p_volume = (audio_volume_t *)p_this;

switch (p_volume->format)
{
case VLC_CODEC_FL32:
p_volume->amplify = FilterFL32;
break;
case VLC_CODEC_FL64:
p_volume->amplify = FilterFL64;
break;
default:
return -1;
}
return 0;
}
