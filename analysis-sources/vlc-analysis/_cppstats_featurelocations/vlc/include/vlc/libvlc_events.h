






















#if !defined(LIBVLC_EVENTS_H)
#define LIBVLC_EVENTS_H 1






#if defined(__cplusplus)
extern "C" {
#endif

typedef struct libvlc_renderer_item_t libvlc_renderer_item_t;









enum libvlc_event_e {







libvlc_MediaMetaChanged=0,




libvlc_MediaSubItemAdded,




libvlc_MediaDurationChanged,






libvlc_MediaParsedChanged,



libvlc_MediaFreed,





libvlc_MediaStateChanged,



libvlc_MediaSubItemTreeAdded,




libvlc_MediaThumbnailGenerated,

libvlc_MediaPlayerMediaChanged=0x100,
libvlc_MediaPlayerNothingSpecial,
libvlc_MediaPlayerOpening,
libvlc_MediaPlayerBuffering,
libvlc_MediaPlayerPlaying,
libvlc_MediaPlayerPaused,
libvlc_MediaPlayerStopped,
libvlc_MediaPlayerForward,
libvlc_MediaPlayerBackward,
libvlc_MediaPlayerEndReached,
libvlc_MediaPlayerEncounteredError,
libvlc_MediaPlayerTimeChanged,
libvlc_MediaPlayerPositionChanged,
libvlc_MediaPlayerSeekableChanged,
libvlc_MediaPlayerPausableChanged,
libvlc_MediaPlayerTitleChanged,
libvlc_MediaPlayerSnapshotTaken,
libvlc_MediaPlayerLengthChanged,
libvlc_MediaPlayerVout,
libvlc_MediaPlayerScrambledChanged,
libvlc_MediaPlayerESAdded,
libvlc_MediaPlayerESDeleted,
libvlc_MediaPlayerESSelected,
libvlc_MediaPlayerCorked,
libvlc_MediaPlayerUncorked,
libvlc_MediaPlayerMuted,
libvlc_MediaPlayerUnmuted,
libvlc_MediaPlayerAudioVolume,
libvlc_MediaPlayerAudioDevice,
libvlc_MediaPlayerChapterChanged,





libvlc_MediaListItemAdded=0x200,




libvlc_MediaListWillAddItem,




libvlc_MediaListItemDeleted,




libvlc_MediaListWillDeleteItem,






libvlc_MediaListEndReached,





libvlc_MediaListViewItemAdded=0x300,




libvlc_MediaListViewWillAddItem,




libvlc_MediaListViewItemDeleted,




libvlc_MediaListViewWillDeleteItem,





libvlc_MediaListPlayerPlayed=0x400,






libvlc_MediaListPlayerNextItemSet,





libvlc_MediaListPlayerStopped,






libvlc_RendererDiscovererItemAdded=0x502,






libvlc_RendererDiscovererItemDeleted,
};




typedef struct libvlc_event_t
{
int type; 
void *p_obj; 
union
{

struct
{
libvlc_meta_t meta_type;
} media_meta_changed;
struct
{
libvlc_media_t * new_child;
} media_subitem_added;
struct
{
int64_t new_duration;
} media_duration_changed;
struct
{
int new_status; 
} media_parsed_changed;
struct
{
libvlc_media_t * md;
} media_freed;
struct
{
int new_state; 
} media_state_changed;
struct
{
libvlc_picture_t* p_thumbnail;
} media_thumbnail_generated;
struct
{
libvlc_media_t * item;
} media_subitemtree_added;


struct
{
float new_cache;
} media_player_buffering;
struct
{
int new_chapter;
} media_player_chapter_changed;
struct
{
float new_position;
} media_player_position_changed;
struct
{
libvlc_time_t new_time;
} media_player_time_changed;
struct
{
int new_title;
} media_player_title_changed;
struct
{
int new_seekable;
} media_player_seekable_changed;
struct
{
int new_pausable;
} media_player_pausable_changed;
struct
{
int new_scrambled;
} media_player_scrambled_changed;
struct
{
int new_count;
} media_player_vout;


struct
{
libvlc_media_t * item;
int index;
} media_list_item_added;
struct
{
libvlc_media_t * item;
int index;
} media_list_will_add_item;
struct
{
libvlc_media_t * item;
int index;
} media_list_item_deleted;
struct
{
libvlc_media_t * item;
int index;
} media_list_will_delete_item;


struct
{
libvlc_media_t * item;
} media_list_player_next_item_set;


struct
{
char* psz_filename ;
} media_player_snapshot_taken ;


struct
{
libvlc_time_t new_length;
} media_player_length_changed;


struct
{
libvlc_media_t * new_media;
} media_player_media_changed;

struct
{
libvlc_track_type_t i_type;
int i_id;
} media_player_es_changed;

struct
{
float volume;
} media_player_audio_volume;

struct
{
const char *device;
} media_player_audio_device;

struct
{
libvlc_renderer_item_t *item;
} renderer_discoverer_item_added;
struct
{
libvlc_renderer_item_t *item;
} renderer_discoverer_item_deleted;
} u; 
} libvlc_event_t;




#if defined(__cplusplus)
}
#endif

#endif 
