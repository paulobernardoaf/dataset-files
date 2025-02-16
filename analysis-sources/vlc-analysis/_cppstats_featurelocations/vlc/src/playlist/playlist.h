



















#if !defined(VLC_PLAYLIST_NEW_INTERNAL_H)
#define VLC_PLAYLIST_NEW_INTERNAL_H

#include <vlc_common.h>
#include <vlc_playlist.h>
#include <vlc_vector.h>
#include "../player/player.h"
#include "randomizer.h"

typedef struct input_item_t input_item_t;

#if defined(TEST_PLAYLIST)

#define vlc_player_New(a,b,c,d) (VLC_UNUSED(a), VLC_UNUSED(b), VLC_UNUSED(c), malloc(1))

#define vlc_player_Delete(p) free(p)
#define vlc_player_Lock(p) VLC_UNUSED(p)
#define vlc_player_Unlock(p) VLC_UNUSED(p)
#define vlc_player_AddListener(a,b,c) (VLC_UNUSED(b), malloc(1))
#define vlc_player_RemoveListener(a,b) free(b)
#define vlc_player_SetCurrentMedia(a,b) (VLC_UNUSED(b), VLC_SUCCESS)
#define vlc_player_InvalidateNextMedia(p) VLC_UNUSED(p)
#define vlc_player_osd_Message(p, fmt...) VLC_UNUSED(p)
#endif 

typedef struct VLC_VECTOR(vlc_playlist_item_t *) playlist_item_vector_t;

struct vlc_playlist
{
vlc_player_t *player;
libvlc_int_t *libvlc;
bool auto_preparse;

struct vlc_player_listener_id *player_listener;
playlist_item_vector_t items;
struct randomizer randomizer;
ssize_t current;
bool has_prev;
bool has_next;
struct vlc_list listeners; 
enum vlc_playlist_playback_repeat repeat;
enum vlc_playlist_playback_order order;
uint64_t idgen;
};


#if !defined(NDEBUG) && !defined(TEST_PLAYLIST)
static inline void
vlc_playlist_AssertLocked(vlc_playlist_t *playlist)
{
vlc_player_assert_locked(playlist->player);
}
#else
#define vlc_playlist_AssertLocked(x) ((void) (0))
#endif

#endif
