#include <vlc_media_source.h>
vlc_media_source_provider_t *
vlc_media_source_provider_New(vlc_object_t *parent);
#define vlc_media_source_provider_New(obj) vlc_media_source_provider_New(VLC_OBJECT(obj))
void vlc_media_source_provider_Delete(vlc_media_source_provider_t *msp);
