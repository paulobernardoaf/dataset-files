#include <vlc_common.h>
#include <vlc_vector.h>
typedef struct vlc_playlist_item vlc_playlist_item_t;
struct randomizer {
struct VLC_VECTOR(vlc_playlist_item_t *) items;
unsigned short xsubi[3]; 
bool loop;
size_t head;
size_t next;
size_t history;
};
void
randomizer_Init(struct randomizer *randomizer);
void
randomizer_Destroy(struct randomizer *randomizer);
void
randomizer_SetLoop(struct randomizer *randomizer, bool loop);
bool
randomizer_Count(struct randomizer *randomizer);
void
randomizer_Reshuffle(struct randomizer *randomizer);
bool
randomizer_HasPrev(struct randomizer *randomizer);
bool
randomizer_HasNext(struct randomizer *randomizer);
vlc_playlist_item_t *
randomizer_PeekPrev(struct randomizer *randomizer);
vlc_playlist_item_t *
randomizer_PeekNext(struct randomizer *randomizer);
vlc_playlist_item_t *
randomizer_Prev(struct randomizer *randomizer);
vlc_playlist_item_t *
randomizer_Next(struct randomizer *randomizer);
void
randomizer_Select(struct randomizer *randomizer,
const vlc_playlist_item_t *item);
bool
randomizer_Add(struct randomizer *randomizer, vlc_playlist_item_t *items[],
size_t count);
void
randomizer_Remove(struct randomizer *randomizer,
vlc_playlist_item_t *const items[], size_t count);
void
randomizer_Clear(struct randomizer *randomizer);
