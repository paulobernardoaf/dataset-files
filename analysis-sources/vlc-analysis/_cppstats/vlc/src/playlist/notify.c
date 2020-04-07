#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include "notify.h"
#include "item.h"
#include "playlist.h"
static void
vlc_playlist_NotifyCurrentState(vlc_playlist_t *playlist,
vlc_playlist_listener_id *listener)
{
vlc_playlist_NotifyListener(playlist, listener, on_items_reset,
playlist->items.data, playlist->items.size);
vlc_playlist_NotifyListener(playlist, listener, on_playback_repeat_changed,
playlist->repeat);
vlc_playlist_NotifyListener(playlist, listener, on_playback_order_changed,
playlist->order);
vlc_playlist_NotifyListener(playlist, listener, on_current_index_changed,
playlist->current);
vlc_playlist_NotifyListener(playlist, listener, on_has_prev_changed,
playlist->has_prev);
vlc_playlist_NotifyListener(playlist, listener, on_has_next_changed,
playlist->has_next);
}
vlc_playlist_listener_id *
vlc_playlist_AddListener(vlc_playlist_t *playlist,
const struct vlc_playlist_callbacks *cbs,
void *userdata, bool notify_current_state)
{
vlc_playlist_AssertLocked(playlist);
vlc_playlist_listener_id *listener = malloc(sizeof(*listener));
if (unlikely(!listener))
return NULL;
listener->cbs = cbs;
listener->userdata = userdata;
vlc_list_append(&listener->node, &playlist->listeners);
if (notify_current_state)
vlc_playlist_NotifyCurrentState(playlist, listener);
return listener;
}
void
vlc_playlist_RemoveListener(vlc_playlist_t *playlist,
vlc_playlist_listener_id *listener)
{
vlc_playlist_AssertLocked(playlist); VLC_UNUSED(playlist);
vlc_list_remove(&listener->node);
free(listener);
}
void
vlc_playlist_state_Save(vlc_playlist_t *playlist,
struct vlc_playlist_state *state)
{
state->current = playlist->current;
state->has_prev = playlist->has_prev;
state->has_next = playlist->has_next;
}
void
vlc_playlist_state_NotifyChanges(vlc_playlist_t *playlist,
struct vlc_playlist_state *saved_state)
{
if (saved_state->current != playlist->current)
vlc_playlist_Notify(playlist, on_current_index_changed, playlist->current);
if (saved_state->has_prev != playlist->has_prev)
vlc_playlist_Notify(playlist, on_has_prev_changed, playlist->has_prev);
if (saved_state->has_next != playlist->has_next)
vlc_playlist_Notify(playlist, on_has_next_changed, playlist->has_next);
}
static inline bool
vlc_playlist_HasItemUpdatedListeners(vlc_playlist_t *playlist)
{
vlc_playlist_listener_id *listener;
vlc_playlist_listener_foreach(listener, playlist)
if (listener->cbs->on_items_updated)
return true;
return false;
}
void
vlc_playlist_NotifyMediaUpdated(vlc_playlist_t *playlist, input_item_t *media)
{
vlc_playlist_AssertLocked(playlist);
if (!vlc_playlist_HasItemUpdatedListeners(playlist))
return;
ssize_t index;
if (playlist->current != -1 &&
playlist->items.data[playlist->current]->media == media)
index = playlist->current;
else
{
index = vlc_playlist_IndexOfMedia(playlist, media);
if (index == -1)
return;
}
vlc_playlist_Notify(playlist, on_items_updated, index,
&playlist->items.data[index], 1);
}
