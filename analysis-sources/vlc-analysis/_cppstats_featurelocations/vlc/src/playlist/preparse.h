



















#if !defined(VLC_PLAYLIST_PREPARSE_H)
#define VLC_PLAYLIST_PREPARSE_H

#include <vlc_common.h>

typedef struct vlc_playlist vlc_playlist_t;
typedef struct input_item_node_t input_item_node_t;

void
vlc_playlist_AutoPreparse(vlc_playlist_t *playlist, input_item_t *input);

int
vlc_playlist_ExpandItem(vlc_playlist_t *playlist, size_t index,
input_item_node_t *node);

int
vlc_playlist_ExpandItemFromNode(vlc_playlist_t *playlist,
input_item_node_t *subitems);

#endif
