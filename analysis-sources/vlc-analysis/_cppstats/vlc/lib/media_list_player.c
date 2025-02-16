#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc/libvlc.h>
#include <vlc/libvlc_renderer_discoverer.h>
#include <vlc/libvlc_picture.h>
#include <vlc/libvlc_media.h>
#include <vlc/libvlc_media_list.h>
#include <vlc/libvlc_media_player.h>
#include <vlc/libvlc_media_list_player.h>
#include <vlc/libvlc_events.h>
#include <assert.h>
#include "libvlc_internal.h"
#include "media_internal.h" 
#include "media_list_path.h"
struct libvlc_media_list_player_t
{
libvlc_event_manager_t event_manager;
int i_refcount;
int seek_offset;
bool dead;
vlc_mutex_t object_lock;
vlc_mutex_t mp_callback_lock;
vlc_cond_t seek_pending;
libvlc_media_list_path_t current_playing_item_path;
libvlc_media_t * p_current_playing_item;
libvlc_media_list_t * p_mlist;
libvlc_media_player_t * p_mi;
libvlc_playback_mode_t e_playback_mode;
vlc_thread_t thread;
};
static
int set_relative_playlist_position_and_play(libvlc_media_list_player_t *p_mlp,
int i_relative_position);
static void stop(libvlc_media_list_player_t * p_mlp);
static inline void lock(libvlc_media_list_player_t * p_mlp)
{
vlc_mutex_lock(&p_mlp->object_lock);
vlc_mutex_lock(&p_mlp->mp_callback_lock);
}
static inline void unlock(libvlc_media_list_player_t * p_mlp)
{
vlc_mutex_unlock(&p_mlp->mp_callback_lock);
vlc_mutex_unlock(&p_mlp->object_lock);
}
static inline void assert_locked(libvlc_media_list_player_t * p_mlp)
{
vlc_mutex_assert(&p_mlp->mp_callback_lock);
(void) p_mlp;
}
static inline libvlc_event_manager_t * mlist_em(libvlc_media_list_player_t * p_mlp)
{
assert_locked(p_mlp);
return libvlc_media_list_event_manager(p_mlp->p_mlist);
}
static inline libvlc_event_manager_t * mplayer_em(libvlc_media_list_player_t * p_mlp)
{
return libvlc_media_player_event_manager(p_mlp->p_mi);
}
static libvlc_media_list_path_t
get_next_path(libvlc_media_list_player_t * p_mlp, bool b_loop)
{
assert_locked(p_mlp);
libvlc_media_list_path_t ret;
libvlc_media_list_t * p_parent_of_playing_item;
libvlc_media_list_t * p_sublist_of_playing_item;
if (!p_mlp->current_playing_item_path)
{
if (!libvlc_media_list_count(p_mlp->p_mlist))
return NULL;
return libvlc_media_list_path_with_root_index(0);
}
p_sublist_of_playing_item = libvlc_media_list_sublist_at_path(
p_mlp->p_mlist,
p_mlp->current_playing_item_path);
if (p_sublist_of_playing_item)
{
int i_count = libvlc_media_list_count(p_sublist_of_playing_item);
libvlc_media_list_release(p_sublist_of_playing_item);
if (i_count > 0)
return libvlc_media_list_path_copy_by_appending(p_mlp->current_playing_item_path, 0);
}
p_parent_of_playing_item = libvlc_media_list_parentlist_at_path(p_mlp->p_mlist,
p_mlp->current_playing_item_path);
int depth = libvlc_media_list_path_depth(p_mlp->current_playing_item_path);
if (depth < 1 || !p_parent_of_playing_item)
return NULL;
ret = libvlc_media_list_path_copy(p_mlp->current_playing_item_path);
ret[depth - 1]++; 
while(ret[depth-1] >= libvlc_media_list_count(p_parent_of_playing_item))
{
depth--;
if (depth <= 0)
{
if(b_loop)
{
ret[0] = 0;
ret[1] = -1;
break;
}
else
{
free(ret);
libvlc_media_list_release(p_parent_of_playing_item);
return NULL;
}
}
ret[depth] = -1;
ret[depth-1]++;
p_parent_of_playing_item = libvlc_media_list_parentlist_at_path(
p_mlp->p_mlist,
ret);
}
libvlc_media_list_release(p_parent_of_playing_item);
return ret;
}
static libvlc_media_list_path_t
find_last_item( libvlc_media_list_t * p_mlist, libvlc_media_list_path_t current_item )
{
libvlc_media_list_t * p_sublist = libvlc_media_list_sublist_at_path(p_mlist, current_item);
libvlc_media_list_path_t last_item_path = current_item;
if(p_sublist)
{
int i_count = libvlc_media_list_count(p_sublist);
if(i_count > 0)
{
last_item_path = libvlc_media_list_path_copy_by_appending(current_item, i_count - 1);
free(current_item);
last_item_path = find_last_item(p_mlist, last_item_path);
}
libvlc_media_list_release(p_sublist);
}
return last_item_path;
}
static libvlc_media_list_path_t
get_previous_path(libvlc_media_list_player_t * p_mlp, bool b_loop)
{
assert_locked(p_mlp);
libvlc_media_list_path_t ret;
libvlc_media_list_t * p_parent_of_playing_item;
if (!p_mlp->current_playing_item_path)
{
if (!libvlc_media_list_count(p_mlp->p_mlist))
return NULL;
return libvlc_media_list_path_with_root_index(0);
}
p_parent_of_playing_item = libvlc_media_list_parentlist_at_path(
p_mlp->p_mlist,
p_mlp->current_playing_item_path);
int depth = libvlc_media_list_path_depth(p_mlp->current_playing_item_path);
if (depth < 1 || !p_parent_of_playing_item)
return NULL;
ret = libvlc_media_list_path_copy(p_mlp->current_playing_item_path);
ret[depth - 1]--; 
ret[depth] = -1;
if(ret[depth - 1] < 0)
{
depth--;
if (depth <= 0)
{
if(b_loop)
{
int i_count = libvlc_media_list_count(p_parent_of_playing_item);
ret[0] = i_count - 1;
ret[1] = -1;
ret = find_last_item(p_mlp->p_mlist, ret);
}
else
{
free(ret);
ret = NULL;
}
}
else
{
ret[depth] = -1;
}
}
else
{
ret = find_last_item(p_mlp->p_mlist, ret);
}
libvlc_media_list_release(p_parent_of_playing_item);
return ret;
}
static void *playlist_thread(void *data)
{
libvlc_media_list_player_t *mlp = data;
vlc_mutex_lock(&mlp->mp_callback_lock);
while (!mlp->dead)
{
if (mlp->seek_offset != 0)
{
set_relative_playlist_position_and_play(mlp, mlp->seek_offset);
mlp->seek_offset = 0;
}
vlc_cond_wait(&mlp->seek_pending, &mlp->mp_callback_lock);
}
vlc_mutex_unlock(&mlp->mp_callback_lock);
return NULL;
}
static void
media_player_reached_end(const libvlc_event_t * p_event, void * p_user_data)
{
VLC_UNUSED(p_event);
libvlc_media_list_player_t * p_mlp = p_user_data;
vlc_mutex_lock(&p_mlp->mp_callback_lock);
p_mlp->seek_offset++;
vlc_cond_signal(&p_mlp->seek_pending);
vlc_mutex_unlock(&p_mlp->mp_callback_lock);
}
static void
mlist_item_deleted(const libvlc_event_t * p_event, void * p_user_data)
{
(void)p_event; (void)p_user_data;
}
static void
install_playlist_observer(libvlc_media_list_player_t * p_mlp)
{
assert_locked(p_mlp);
libvlc_event_attach(mlist_em(p_mlp), libvlc_MediaListItemDeleted, mlist_item_deleted, p_mlp);
}
static void
uninstall_playlist_observer(libvlc_media_list_player_t * p_mlp)
{
assert_locked(p_mlp);
if (!p_mlp->p_mlist) return;
libvlc_event_detach(mlist_em(p_mlp), libvlc_MediaListItemDeleted, mlist_item_deleted, p_mlp);
}
static void
install_media_player_observer(libvlc_media_list_player_t * p_mlp)
{
libvlc_event_attach(mplayer_em(p_mlp), libvlc_MediaPlayerEndReached, media_player_reached_end, p_mlp);
}
static void
uninstall_media_player_observer(libvlc_media_list_player_t * p_mlp)
{
assert_locked(p_mlp);
vlc_mutex_unlock(&p_mlp->mp_callback_lock);
libvlc_event_detach(mplayer_em(p_mlp), libvlc_MediaPlayerEndReached, media_player_reached_end, p_mlp);
vlc_mutex_lock(&p_mlp->mp_callback_lock);
}
static void
set_current_playing_item(libvlc_media_list_player_t * p_mlp, libvlc_media_list_path_t path)
{
assert_locked(p_mlp);
if (p_mlp->current_playing_item_path != path)
{
free(p_mlp->current_playing_item_path);
p_mlp->current_playing_item_path = path;
}
if (!path)
return;
libvlc_media_t * p_md;
p_md = libvlc_media_list_item_at_path(p_mlp->p_mlist, path);
if (!p_md)
return;
uninstall_media_player_observer(p_mlp);
libvlc_media_player_set_media(p_mlp->p_mi, p_md);
install_media_player_observer(p_mlp);
libvlc_media_release(p_md); 
}
libvlc_media_list_player_t *
libvlc_media_list_player_new(libvlc_instance_t * p_instance)
{
libvlc_media_list_player_t * p_mlp;
p_mlp = calloc( 1, sizeof(libvlc_media_list_player_t) );
if (unlikely(p_mlp == NULL))
{
libvlc_printerr("Not enough memory");
return NULL;
}
p_mlp->i_refcount = 1;
p_mlp->seek_offset = 0;
p_mlp->dead = false;
vlc_mutex_init(&p_mlp->object_lock);
vlc_mutex_init(&p_mlp->mp_callback_lock);
vlc_cond_init(&p_mlp->seek_pending);
libvlc_event_manager_init(&p_mlp->event_manager, p_mlp);
p_mlp->p_mi = libvlc_media_player_new(p_instance);
if( p_mlp->p_mi == NULL )
goto error;
install_media_player_observer(p_mlp);
if (vlc_clone(&p_mlp->thread, playlist_thread, p_mlp,
VLC_THREAD_PRIORITY_LOW))
{
libvlc_media_player_release(p_mlp->p_mi);
goto error;
}
return p_mlp;
error:
libvlc_event_manager_destroy(&p_mlp->event_manager);
free(p_mlp);
return NULL;
}
void libvlc_media_list_player_release(libvlc_media_list_player_t * p_mlp)
{
if (!p_mlp)
return;
lock(p_mlp);
p_mlp->i_refcount--;
if (p_mlp->i_refcount > 0)
{
unlock(p_mlp);
return;
}
assert(p_mlp->i_refcount == 0);
unlock(p_mlp);
vlc_mutex_lock(&p_mlp->mp_callback_lock);
p_mlp->dead = true;
vlc_cond_signal(&p_mlp->seek_pending);
vlc_mutex_unlock(&p_mlp->mp_callback_lock);
vlc_join(p_mlp->thread, NULL);
lock(p_mlp);
uninstall_media_player_observer(p_mlp);
libvlc_media_player_release(p_mlp->p_mi);
if (p_mlp->p_mlist)
{
uninstall_playlist_observer(p_mlp);
libvlc_media_list_release(p_mlp->p_mlist);
}
unlock(p_mlp);
libvlc_event_manager_destroy(&p_mlp->event_manager);
free(p_mlp->current_playing_item_path);
free(p_mlp);
}
void libvlc_media_list_player_retain(libvlc_media_list_player_t * p_mlp)
{
if (!p_mlp)
return;
lock(p_mlp);
p_mlp->i_refcount++;
unlock(p_mlp);
}
libvlc_event_manager_t *
libvlc_media_list_player_event_manager(libvlc_media_list_player_t * p_mlp)
{
return &p_mlp->event_manager;
}
void libvlc_media_list_player_set_media_player(libvlc_media_list_player_t * p_mlp, libvlc_media_player_t * p_mi)
{
libvlc_media_player_t *p_oldmi;
assert(p_mi != NULL);
libvlc_media_player_retain(p_mi);
lock(p_mlp);
uninstall_media_player_observer(p_mlp);
p_oldmi = p_mlp->p_mi;
p_mlp->p_mi = p_mi;
install_media_player_observer(p_mlp);
unlock(p_mlp);
libvlc_media_player_release(p_oldmi);
}
libvlc_media_player_t * libvlc_media_list_player_get_media_player(libvlc_media_list_player_t * p_mlp)
{
libvlc_media_player_retain(p_mlp->p_mi);
return p_mlp->p_mi;
}
void libvlc_media_list_player_set_media_list(libvlc_media_list_player_t * p_mlp, libvlc_media_list_t * p_mlist)
{
assert (p_mlist);
lock(p_mlp);
if (p_mlp->p_mlist)
{
uninstall_playlist_observer(p_mlp);
libvlc_media_list_release(p_mlp->p_mlist);
}
libvlc_media_list_retain(p_mlist);
p_mlp->p_mlist = p_mlist;
install_playlist_observer(p_mlp);
unlock(p_mlp);
}
void libvlc_media_list_player_play(libvlc_media_list_player_t * p_mlp)
{
lock(p_mlp);
if (!p_mlp->current_playing_item_path)
{
set_relative_playlist_position_and_play(p_mlp, 1);
unlock(p_mlp);
return; 
}
libvlc_media_player_play(p_mlp->p_mi);
unlock(p_mlp);
}
void libvlc_media_list_player_pause(libvlc_media_list_player_t * p_mlp)
{
lock(p_mlp);
libvlc_media_player_pause(p_mlp->p_mi);
unlock(p_mlp);
}
void libvlc_media_list_player_set_pause(libvlc_media_list_player_t * p_mlp,
int do_pause)
{
lock(p_mlp);
libvlc_media_player_set_pause(p_mlp->p_mi, do_pause);
unlock(p_mlp);
}
bool libvlc_media_list_player_is_playing(libvlc_media_list_player_t * p_mlp)
{
libvlc_state_t state = libvlc_media_player_get_state(p_mlp->p_mi);
return (state == libvlc_Opening) || (state == libvlc_Playing);
}
libvlc_state_t
libvlc_media_list_player_get_state(libvlc_media_list_player_t * p_mlp)
{
return libvlc_media_player_get_state(p_mlp->p_mi);
}
int libvlc_media_list_player_play_item_at_index(libvlc_media_list_player_t * p_mlp, int i_index)
{
lock(p_mlp);
libvlc_media_list_path_t path = libvlc_media_list_path_with_root_index(i_index);
set_current_playing_item(p_mlp, path);
libvlc_media_t *p_md = libvlc_media_player_get_media(p_mlp->p_mi);
libvlc_media_player_play(p_mlp->p_mi);
unlock(p_mlp);
if (!p_md)
return -1;
libvlc_event_t event;
event.type = libvlc_MediaListPlayerNextItemSet;
event.u.media_list_player_next_item_set.item = p_md;
libvlc_event_send(&p_mlp->event_manager, &event);
libvlc_media_release(p_md);
return 0;
}
int libvlc_media_list_player_play_item(libvlc_media_list_player_t * p_mlp, libvlc_media_t * p_md)
{
lock(p_mlp);
libvlc_media_list_path_t path = libvlc_media_list_path_of_item(p_mlp->p_mlist, p_md);
if (!path)
{
libvlc_printerr("Item not found in media list");
unlock(p_mlp);
return -1;
}
set_current_playing_item(p_mlp, path);
libvlc_media_player_play(p_mlp->p_mi);
unlock(p_mlp);
return 0;
}
static void stop(libvlc_media_list_player_t * p_mlp)
{
assert_locked(p_mlp);
uninstall_media_player_observer(p_mlp);
libvlc_media_player_stop_async(p_mlp->p_mi);
install_media_player_observer(p_mlp);
free(p_mlp->current_playing_item_path);
p_mlp->current_playing_item_path = NULL;
libvlc_event_t event;
event.type = libvlc_MediaListPlayerStopped;
libvlc_event_send(&p_mlp->event_manager, &event);
}
void libvlc_media_list_player_stop_async(libvlc_media_list_player_t * p_mlp)
{
lock(p_mlp);
stop(p_mlp);
unlock(p_mlp);
}
static int set_relative_playlist_position_and_play(
libvlc_media_list_player_t * p_mlp,
int i_relative_position)
{
assert_locked(p_mlp);
if (!p_mlp->p_mlist)
{
libvlc_printerr("No media list");
return -1;
}
libvlc_media_list_lock(p_mlp->p_mlist);
libvlc_media_list_path_t path = p_mlp->current_playing_item_path;
if(p_mlp->e_playback_mode != libvlc_playback_mode_repeat)
{
bool b_loop = (p_mlp->e_playback_mode == libvlc_playback_mode_loop);
while (i_relative_position > 0)
{
path = get_next_path(p_mlp, b_loop);
set_current_playing_item(p_mlp, path);
--i_relative_position;
}
while (i_relative_position < 0)
{
path = get_previous_path(p_mlp, b_loop);
set_current_playing_item(p_mlp, path);
++i_relative_position;
}
}
else
{
set_current_playing_item(p_mlp, path);
}
#if defined(DEBUG_MEDIA_LIST_PLAYER)
printf("Playing:");
libvlc_media_list_path_dump(path);
#endif
if (!path)
{
libvlc_media_list_unlock(p_mlp->p_mlist);
libvlc_event_t event;
event.type = libvlc_MediaListPlayerPlayed;
libvlc_event_send(&p_mlp->event_manager, &event);
return -1;
}
libvlc_media_player_play(p_mlp->p_mi);
libvlc_media_list_unlock(p_mlp->p_mlist);
libvlc_event_t event;
event.type = libvlc_MediaListPlayerNextItemSet;
libvlc_media_t * p_md = libvlc_media_list_item_at_path(p_mlp->p_mlist, path);
event.u.media_list_player_next_item_set.item = p_md;
libvlc_event_send(&p_mlp->event_manager, &event);
libvlc_media_release(p_md);
return 0;
}
int libvlc_media_list_player_next(libvlc_media_list_player_t * p_mlp)
{
lock(p_mlp);
int failure = set_relative_playlist_position_and_play(p_mlp, 1);
unlock(p_mlp);
return failure;
}
int libvlc_media_list_player_previous(libvlc_media_list_player_t * p_mlp)
{
lock(p_mlp);
int failure = set_relative_playlist_position_and_play(p_mlp, -1);
unlock(p_mlp);
return failure;
}
void libvlc_media_list_player_set_playback_mode(
libvlc_media_list_player_t * p_mlp,
libvlc_playback_mode_t e_mode )
{
lock(p_mlp);
p_mlp->e_playback_mode = e_mode;
unlock(p_mlp);
}
