



















#if !defined(VLC_PLAYLIST_PLAYER_H)
#define VLC_PLAYLIST_PLAYER_H

#include <vlc_common.h>

typedef struct vlc_playlist vlc_playlist_t;

bool
vlc_playlist_PlayerInit(vlc_playlist_t *playlist, vlc_object_t *parent);

void
vlc_playlist_PlayerDestroy(vlc_playlist_t *playlist);

#endif
