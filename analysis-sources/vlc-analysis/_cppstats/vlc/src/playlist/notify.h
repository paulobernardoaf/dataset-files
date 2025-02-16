#include <vlc_common.h>
#include <vlc_list.h>
typedef struct vlc_playlist vlc_playlist_t;
struct vlc_playlist_listener_id
{
const struct vlc_playlist_callbacks *cbs;
void *userdata;
struct vlc_list node; 
};
struct vlc_playlist_state {
ssize_t current;
bool has_prev;
bool has_next;
};
#define vlc_playlist_listener_foreach(listener, playlist) vlc_list_foreach(listener, &(playlist)->listeners, node)
#define vlc_playlist_NotifyListener(playlist, listener, event, ...) do { if (listener->cbs->event) listener->cbs->event(playlist, ##__VA_ARGS__, listener->userdata); } while (0)
#define vlc_playlist_Notify(playlist, event, ...) do { vlc_playlist_AssertLocked(playlist); vlc_playlist_listener_id *listener; vlc_playlist_listener_foreach(listener, playlist) vlc_playlist_NotifyListener(playlist, listener, event, ##__VA_ARGS__); } while(0)
void
vlc_playlist_state_Save(vlc_playlist_t *playlist,
struct vlc_playlist_state *state);
void
vlc_playlist_state_NotifyChanges(vlc_playlist_t *playlist,
struct vlc_playlist_state *saved_state);
void
vlc_playlist_NotifyMediaUpdated(vlc_playlist_t *playlist, input_item_t *media);
