



















#if !defined(VLC_PLAYLIST_EXPORT_H)
#define VLC_PLAYLIST_EXPORT_H

#include <vlc_playlist.h>








struct vlc_playlist_view;








VLC_API size_t
vlc_playlist_view_Count(struct vlc_playlist_view *view);












VLC_API vlc_playlist_item_t *
vlc_playlist_view_Get(struct vlc_playlist_view *view, size_t index);




struct vlc_playlist_export
{
struct vlc_object_t obj;
char *base_url;
FILE *file;
struct vlc_playlist_view *playlist_view;
};

#endif
