#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc/vlc.h>
#include <vlc/libvlc_media.h>
#include <vlc_input.h>
#include <vlc_player.h>
#include <vlc_viewpoint.h>
#include "../modules/audio_filter/equalizer_presets.h"
struct libvlc_media_player_t
{
struct vlc_object_t obj;
int i_refcount;
vlc_player_t *player;
vlc_player_listener_id *listener;
vlc_player_aout_listener_id *aout_listener;
struct libvlc_instance_t * p_libvlc_instance; 
libvlc_media_t * p_md; 
libvlc_event_manager_t event_manager;
};
libvlc_track_description_t * libvlc_get_track_description(
libvlc_media_player_t *p_mi,
enum es_format_category_e cat );
struct libvlc_equalizer_t
{
float f_preamp;
float f_amp[EQZ_BANDS_MAX];
};
