#include <vlc_common.h>
#if defined(__cplusplus)
extern "C" {
#endif
typedef struct input_item_t input_item_t;
typedef struct vlc_player_t vlc_player_t;
typedef struct vlc_playlist vlc_playlist_t;
typedef struct vlc_playlist_item vlc_playlist_item_t;
typedef struct vlc_playlist_listener_id vlc_playlist_listener_id;
enum vlc_playlist_playback_repeat
{
VLC_PLAYLIST_PLAYBACK_REPEAT_NONE,
VLC_PLAYLIST_PLAYBACK_REPEAT_CURRENT,
VLC_PLAYLIST_PLAYBACK_REPEAT_ALL,
};
enum vlc_playlist_playback_order
{
VLC_PLAYLIST_PLAYBACK_ORDER_NORMAL,
VLC_PLAYLIST_PLAYBACK_ORDER_RANDOM,
};
enum vlc_playlist_sort_key
{
VLC_PLAYLIST_SORT_KEY_TITLE,
VLC_PLAYLIST_SORT_KEY_DURATION,
VLC_PLAYLIST_SORT_KEY_ARTIST,
VLC_PLAYLIST_SORT_KEY_ALBUM,
VLC_PLAYLIST_SORT_KEY_ALBUM_ARTIST,
VLC_PLAYLIST_SORT_KEY_GENRE,
VLC_PLAYLIST_SORT_KEY_DATE,
VLC_PLAYLIST_SORT_KEY_TRACK_NUMBER,
VLC_PLAYLIST_SORT_KEY_DISC_NUMBER,
VLC_PLAYLIST_SORT_KEY_URL,
VLC_PLAYLIST_SORT_KEY_RATING,
};
enum vlc_playlist_sort_order
{
VLC_PLAYLIST_SORT_ORDER_ASCENDING,
VLC_PLAYLIST_SORT_ORDER_DESCENDING,
};
struct vlc_playlist_sort_criterion
{
enum vlc_playlist_sort_key key;
enum vlc_playlist_sort_order order;
};
struct vlc_playlist_callbacks
{
void
(*on_items_reset)(vlc_playlist_t *, vlc_playlist_item_t *const items[],
size_t count, void *userdata);
void
(*on_items_added)(vlc_playlist_t *playlist, size_t index,
vlc_playlist_item_t *const items[], size_t count,
void *userdata);
void
(*on_items_moved)(vlc_playlist_t *playlist, size_t index, size_t count,
size_t target, void *userdata);
void
(*on_items_removed)(vlc_playlist_t *playlist, size_t index, size_t count,
void *userdata);
void
(*on_items_updated)(vlc_playlist_t *playlist, size_t index,
vlc_playlist_item_t *const items[], size_t count,
void *userdata);
void
(*on_playback_repeat_changed)(vlc_playlist_t *playlist,
enum vlc_playlist_playback_repeat repeat,
void *userdata);
void
(*on_playback_order_changed)(vlc_playlist_t *playlist,
enum vlc_playlist_playback_order order,
void *userdata);
void
(*on_current_index_changed)(vlc_playlist_t *playlist, ssize_t index,
void *userdata);
void
(*on_has_prev_changed)(vlc_playlist_t *playlist, bool has_prev,
void *userdata);
void
(*on_has_next_changed)(vlc_playlist_t *playlist,
bool has_next, void *userdata);
};
VLC_API void
vlc_playlist_item_Hold(vlc_playlist_item_t *);
VLC_API void
vlc_playlist_item_Release(vlc_playlist_item_t *);
VLC_API input_item_t *
vlc_playlist_item_GetMedia(vlc_playlist_item_t *);
VLC_API uint64_t
vlc_playlist_item_GetId(vlc_playlist_item_t *);
VLC_API VLC_USED vlc_playlist_t *
vlc_playlist_New(vlc_object_t *parent);
VLC_API void
vlc_playlist_Delete(vlc_playlist_t *);
VLC_API void
vlc_playlist_Lock(vlc_playlist_t *);
VLC_API void
vlc_playlist_Unlock(vlc_playlist_t *);
VLC_API VLC_USED vlc_playlist_listener_id *
vlc_playlist_AddListener(vlc_playlist_t *playlist,
const struct vlc_playlist_callbacks *cbs,
void *userdata, bool notify_current_state);
VLC_API void
vlc_playlist_RemoveListener(vlc_playlist_t *, vlc_playlist_listener_id *);
VLC_API size_t
vlc_playlist_Count(vlc_playlist_t *playlist);
VLC_API vlc_playlist_item_t *
vlc_playlist_Get(vlc_playlist_t *playlist, size_t index);
VLC_API void
vlc_playlist_Clear(vlc_playlist_t *playlist);
VLC_API int
vlc_playlist_Insert(vlc_playlist_t *playlist, size_t index,
input_item_t *const media[], size_t count);
static inline int
vlc_playlist_InsertOne(vlc_playlist_t *playlist, size_t index,
input_item_t *media)
{
return vlc_playlist_Insert(playlist, index, &media, 1);
}
static inline int
vlc_playlist_Append(vlc_playlist_t *playlist, input_item_t *const media[],
size_t count)
{
size_t size = vlc_playlist_Count(playlist);
return vlc_playlist_Insert(playlist, size, media, count);
}
static inline int
vlc_playlist_AppendOne(vlc_playlist_t *playlist, input_item_t *media)
{
return vlc_playlist_Append(playlist, &media, 1);
}
VLC_API void
vlc_playlist_Move(vlc_playlist_t *playlist, size_t index, size_t count,
size_t target);
static inline void
vlc_playlist_MoveOne(vlc_playlist_t *playlist, size_t index, size_t target)
{
vlc_playlist_Move(playlist, index, 1, target);
}
VLC_API void
vlc_playlist_Remove(vlc_playlist_t *playlist, size_t index, size_t count);
static inline void
vlc_playlist_RemoveOne(vlc_playlist_t *playlist, size_t index)
{
vlc_playlist_Remove(playlist, index, 1);
}
VLC_API int
vlc_playlist_RequestInsert(vlc_playlist_t *playlist, size_t index,
input_item_t *const media[], size_t count);
VLC_API int
vlc_playlist_RequestMove(vlc_playlist_t *playlist,
vlc_playlist_item_t *const items[], size_t count,
size_t target, ssize_t index_hint);
VLC_API int
vlc_playlist_RequestRemove(vlc_playlist_t *playlist,
vlc_playlist_item_t *const items[], size_t count,
ssize_t index_hint);
VLC_API void
vlc_playlist_Shuffle(vlc_playlist_t *playlist);
VLC_API int
vlc_playlist_Sort(vlc_playlist_t *playlist,
const struct vlc_playlist_sort_criterion criteria[],
size_t count);
VLC_API ssize_t
vlc_playlist_IndexOf(vlc_playlist_t *playlist, const vlc_playlist_item_t *item);
VLC_API ssize_t
vlc_playlist_IndexOfMedia(vlc_playlist_t *playlist, const input_item_t *media);
VLC_API ssize_t
vlc_playlist_IndexOfId(vlc_playlist_t *playlist, uint64_t id);
VLC_API enum vlc_playlist_playback_repeat
vlc_playlist_GetPlaybackRepeat(vlc_playlist_t *playlist);
VLC_API enum vlc_playlist_playback_order
vlc_playlist_GetPlaybackOrder(vlc_playlist_t *);
VLC_API void
vlc_playlist_SetPlaybackRepeat(vlc_playlist_t *playlist,
enum vlc_playlist_playback_repeat repeat);
VLC_API void
vlc_playlist_SetPlaybackOrder(vlc_playlist_t *playlist,
enum vlc_playlist_playback_order order);
VLC_API ssize_t
vlc_playlist_GetCurrentIndex(vlc_playlist_t *playlist);
VLC_API bool
vlc_playlist_HasPrev(vlc_playlist_t *playlist);
VLC_API bool
vlc_playlist_HasNext(vlc_playlist_t *playlist);
VLC_API int
vlc_playlist_Prev(vlc_playlist_t *playlist);
VLC_API int
vlc_playlist_Next(vlc_playlist_t *playlist);
VLC_API int
vlc_playlist_GoTo(vlc_playlist_t *playlist, ssize_t index);
VLC_API int
vlc_playlist_RequestGoTo(vlc_playlist_t *playlist, vlc_playlist_item_t *item,
ssize_t index_hint);
VLC_API vlc_player_t *
vlc_playlist_GetPlayer(vlc_playlist_t *playlist);
VLC_API int
vlc_playlist_Start(vlc_playlist_t *playlist);
VLC_API void
vlc_playlist_Stop(vlc_playlist_t *playlist);
VLC_API void
vlc_playlist_Pause(vlc_playlist_t *playlist);
VLC_API void
vlc_playlist_Resume(vlc_playlist_t *playlist);
static inline int
vlc_playlist_PlayAt(vlc_playlist_t *playlist, size_t index)
{
int ret = vlc_playlist_GoTo(playlist, index);
if (ret != VLC_SUCCESS)
return ret;
return vlc_playlist_Start(playlist);
}
VLC_API void
vlc_playlist_Preparse(vlc_playlist_t *playlist, input_item_t *media);
VLC_API int
vlc_playlist_Export(vlc_playlist_t *playlist, const char *filename,
const char *type);
#if defined(__cplusplus)
}
#endif
