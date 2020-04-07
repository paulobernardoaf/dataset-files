#include <assert.h>
#include <vlc_common.h>
#if defined(__cplusplus)
extern "C" {
#endif
typedef enum vlc_ml_media_type_t
{
VLC_ML_MEDIA_TYPE_UNKNOWN,
VLC_ML_MEDIA_TYPE_VIDEO,
VLC_ML_MEDIA_TYPE_AUDIO,
} vlc_ml_media_type_t;
typedef enum vlc_ml_media_subtype_t
{
VLC_ML_MEDIA_SUBTYPE_UNKNOWN,
VLC_ML_MEDIA_SUBTYPE_SHOW_EPISODE,
VLC_ML_MEDIA_SUBTYPE_MOVIE,
VLC_ML_MEDIA_SUBTYPE_ALBUMTRACK,
} vlc_ml_media_subtype_t;
typedef enum vlc_ml_file_type_t
{
VLC_ML_FILE_TYPE_UNKNOWN,
VLC_ML_FILE_TYPE_MAIN,
VLC_ML_FILE_TYPE_PART,
VLC_ML_FILE_TYPE_SOUNDTRACK,
VLC_ML_FILE_TYPE_SUBTITLE,
VLC_ML_FILE_TYPE_PLAYLIST,
} vlc_ml_file_type_t;
typedef enum vlc_ml_track_type_t
{
VLC_ML_TRACK_TYPE_UNKNOWN,
VLC_ML_TRACK_TYPE_VIDEO,
VLC_ML_TRACK_TYPE_AUDIO,
} vlc_ml_track_type_t;
typedef enum vlc_ml_thumbnail_size_t
{
VLC_ML_THUMBNAIL_SMALL,
VLC_ML_THUMBNAIL_BANNER,
VLC_ML_THUMBNAIL_SIZE_COUNT
} vlc_ml_thumbnail_size_t;
typedef enum vlc_ml_history_type_t
{
VLC_ML_HISTORY_TYPE_MEDIA,
VLC_ML_HISTORY_TYPE_NETWORK,
} vlc_ml_history_type_t;
typedef struct vlc_ml_thumbnail_t
{
char* psz_mrl;
bool b_generated;
} vlc_ml_thumbnail_t;
typedef struct vlc_ml_movie_t
{
char* psz_summary;
char* psz_imdb_id;
} vlc_ml_movie_t;
typedef struct vlc_ml_show_episode_t
{
char* psz_summary;
char* psz_tvdb_id;
uint32_t i_episode_nb;
uint32_t i_season_number;
} vlc_ml_show_episode_t;
typedef struct vlc_ml_show_t
{
int64_t i_id;
char* psz_name;
char* psz_summary;
char* psz_artwork_mrl;
char* psz_tvdb_id;
unsigned int i_release_year;
uint32_t i_nb_episodes;
uint32_t i_nb_seasons;
} vlc_ml_show_t;
typedef struct vlc_ml_album_track_t
{
int64_t i_artist_id;
int64_t i_album_id;
int64_t i_genre_id;
int i_track_nb;
int i_disc_nb;
} vlc_ml_album_track_t;
typedef struct vlc_ml_label_t
{
int64_t i_id;
char* psz_name;
} vlc_ml_label_t;
typedef struct vlc_ml_label_list_t
{
size_t i_nb_items;
vlc_ml_label_t p_items[];
} vlc_ml_label_list_t;
typedef struct vlc_ml_file_t
{
char* psz_mrl;
vlc_ml_file_type_t i_type;
bool b_external;
bool b_removable;
bool b_present;
} vlc_ml_file_t;
typedef struct vlc_ml_file_list_t
{
size_t i_nb_items;
vlc_ml_file_t p_items[];
} vlc_ml_file_list_t;
typedef struct vlc_ml_media_track_t
{
char* psz_codec;
char* psz_language;
char* psz_description;
vlc_ml_track_type_t i_type;
uint32_t i_bitrate;
union
{
struct
{
uint32_t i_nbChannels;
uint32_t i_sampleRate;
} a;
struct
{
uint32_t i_height;
uint32_t i_width;
uint32_t i_sarNum;
uint32_t i_sarDen;
uint32_t i_fpsNum;
uint32_t i_fpsDen;
} v;
};
} vlc_ml_media_track_t;
typedef struct vlc_ml_media_track_list_t
{
size_t i_nb_items;
vlc_ml_media_track_t p_items[];
} vlc_ml_media_track_list_t;
typedef struct vlc_ml_media_t
{
int64_t i_id;
vlc_ml_media_type_t i_type;
vlc_ml_media_subtype_t i_subtype;
vlc_ml_file_list_t* p_files;
vlc_ml_media_track_list_t* p_tracks;
int32_t i_year;
int64_t i_duration;
uint32_t i_playcount;
time_t i_last_played_date;
char* psz_title;
vlc_ml_thumbnail_t thumbnails[VLC_ML_THUMBNAIL_SIZE_COUNT];
bool b_is_favorite;
union
{
vlc_ml_show_episode_t show_episode;
vlc_ml_movie_t movie;
vlc_ml_album_track_t album_track;
};
} vlc_ml_media_t;
typedef struct vlc_ml_playlist_t
{
int64_t i_id;
char* psz_name;
uint32_t i_creation_date;
char* psz_artwork_mrl;
} vlc_ml_playlist_t;
typedef struct vlc_ml_artist_t
{
int64_t i_id;
char* psz_name;
char* psz_shortbio;
vlc_ml_thumbnail_t thumbnails[VLC_ML_THUMBNAIL_SIZE_COUNT];
char* psz_mb_id;
unsigned int i_nb_album;
unsigned int i_nb_tracks;
} vlc_ml_artist_t;
typedef struct vlc_ml_artist_list_t
{
size_t i_nb_items;
vlc_ml_artist_t p_items[];
} vlc_ml_artist_list_t;
typedef struct vlc_ml_album_t {
int64_t i_id;
char* psz_title;
char* psz_summary;
vlc_ml_thumbnail_t thumbnails[VLC_ML_THUMBNAIL_SIZE_COUNT];
char* psz_artist;
int64_t i_artist_id;
size_t i_nb_tracks;
unsigned int i_duration;
unsigned int i_year;
} vlc_ml_album_t;
typedef struct vlc_ml_genre_t
{
int64_t i_id;
char* psz_name;
size_t i_nb_tracks;
} vlc_ml_genre_t;
typedef struct vlc_ml_media_list_t
{
size_t i_nb_items;
vlc_ml_media_t p_items[];
} vlc_ml_media_list_t;
typedef struct vlc_ml_album_list_t
{
size_t i_nb_items;
vlc_ml_album_t p_items[];
} vlc_ml_album_list_t;
typedef struct vlc_ml_show_list_t
{
size_t i_nb_items;
vlc_ml_show_t p_items[];
} vlc_ml_show_list_t;
typedef struct vlc_ml_genre_list_t
{
size_t i_nb_items;
vlc_ml_genre_t p_items[];
} vlc_ml_genre_list_t;
typedef struct vlc_ml_playlist_list_t
{
size_t i_nb_items;
vlc_ml_playlist_t p_items[];
} vlc_ml_playlist_list_t;
typedef struct vlc_ml_entry_point_t
{
char* psz_mrl; 
bool b_present; 
bool b_banned; 
} vlc_ml_entry_point_t;
typedef struct vlc_ml_entry_point_list_t
{
size_t i_nb_items;
vlc_ml_entry_point_t p_items[];
} vlc_ml_entry_point_list_t;
typedef struct vlc_medialibrary_t vlc_medialibrary_t;
typedef struct vlc_medialibrary_module_t vlc_medialibrary_module_t;
typedef struct vlc_ml_event_callback_t vlc_ml_event_callback_t;
typedef enum vlc_ml_sorting_criteria_t
{
VLC_ML_SORTING_DEFAULT,
VLC_ML_SORTING_ALPHA,
VLC_ML_SORTING_DURATION,
VLC_ML_SORTING_INSERTIONDATE,
VLC_ML_SORTING_LASTMODIFICATIONDATE,
VLC_ML_SORTING_RELEASEDATE,
VLC_ML_SORTING_FILESIZE,
VLC_ML_SORTING_ARTIST,
VLC_ML_SORTING_PLAYCOUNT,
VLC_ML_SORTING_ALBUM,
VLC_ML_SORTING_FILENAME,
VLC_ML_SORTING_TRACKNUMBER,
} vlc_ml_sorting_criteria_t;
typedef struct vlc_ml_query_params_t vlc_ml_query_params_t;
struct vlc_ml_query_params_t
{
const char* psz_pattern;
uint32_t i_nbResults;
uint32_t i_offset;
vlc_ml_sorting_criteria_t i_sort;
bool b_desc;
};
enum vlc_ml_get_queries
{
VLC_ML_GET_MEDIA, 
VLC_ML_GET_MEDIA_BY_MRL, 
VLC_ML_GET_INPUT_ITEM, 
VLC_ML_GET_INPUT_ITEM_BY_MRL,
VLC_ML_GET_ALBUM, 
VLC_ML_GET_ARTIST, 
VLC_ML_GET_GENRE, 
VLC_ML_GET_SHOW, 
VLC_ML_GET_PLAYLIST, 
};
enum vlc_ml_list_queries
{
VLC_ML_LIST_VIDEOS, 
VLC_ML_COUNT_VIDEOS, 
VLC_ML_LIST_AUDIOS, 
VLC_ML_COUNT_AUDIOS, 
VLC_ML_LIST_ALBUMS, 
VLC_ML_COUNT_ALBUMS, 
VLC_ML_LIST_GENRES, 
VLC_ML_COUNT_GENRES, 
VLC_ML_LIST_ARTISTS, 
VLC_ML_COUNT_ARTISTS, 
VLC_ML_LIST_SHOWS, 
VLC_ML_COUNT_SHOWS, 
VLC_ML_LIST_PLAYLISTS, 
VLC_ML_COUNT_PLAYLISTS, 
VLC_ML_LIST_HISTORY, 
VLC_ML_LIST_STREAM_HISTORY, 
VLC_ML_LIST_ALBUM_TRACKS, 
VLC_ML_COUNT_ALBUM_TRACKS, 
VLC_ML_LIST_ALBUM_ARTISTS, 
VLC_ML_COUNT_ALBUM_ARTISTS, 
VLC_ML_LIST_ARTIST_ALBUMS, 
VLC_ML_COUNT_ARTIST_ALBUMS, 
VLC_ML_LIST_ARTIST_TRACKS, 
VLC_ML_COUNT_ARTIST_TRACKS, 
VLC_ML_LIST_GENRE_ARTISTS, 
VLC_ML_COUNT_GENRE_ARTISTS, 
VLC_ML_LIST_GENRE_TRACKS, 
VLC_ML_COUNT_GENRE_TRACKS, 
VLC_ML_LIST_GENRE_ALBUMS, 
VLC_ML_COUNT_GENRE_ALBUMS, 
VLC_ML_LIST_SHOW_EPISODES, 
VLC_ML_COUNT_SHOW_EPISODES, 
VLC_ML_LIST_MEDIA_LABELS, 
VLC_ML_COUNT_MEDIA_LABELS, 
VLC_ML_LIST_PLAYLIST_MEDIA, 
VLC_ML_COUNT_PLAYLIST_MEDIA, 
VLC_ML_LIST_MEDIA_OF, 
VLC_ML_COUNT_MEDIA_OF, 
VLC_ML_LIST_ARTISTS_OF, 
VLC_ML_COUNT_ARTISTS_OF, 
VLC_ML_LIST_ALBUMS_OF, 
VLC_ML_COUNT_ALBUMS_OF, 
};
enum vlc_ml_parent_type
{
VLC_ML_PARENT_UNKNOWN,
VLC_ML_PARENT_ALBUM,
VLC_ML_PARENT_ARTIST,
VLC_ML_PARENT_SHOW,
VLC_ML_PARENT_GENRE,
VLC_ML_PARENT_PLAYLIST,
};
enum vlc_ml_control
{
VLC_ML_ADD_FOLDER, 
VLC_ML_REMOVE_FOLDER, 
VLC_ML_BAN_FOLDER, 
VLC_ML_UNBAN_FOLDER, 
VLC_ML_LIST_FOLDERS, 
VLC_ML_IS_INDEXED, 
VLC_ML_RELOAD_FOLDER,
VLC_ML_PAUSE_BACKGROUND, 
VLC_ML_RESUME_BACKGROUND, 
VLC_ML_CLEAR_HISTORY, 
VLC_ML_NEW_EXTERNAL_MEDIA, 
VLC_ML_NEW_STREAM, 
VLC_ML_MEDIA_INCREASE_PLAY_COUNT, 
VLC_ML_MEDIA_GET_MEDIA_PLAYBACK_STATE, 
VLC_ML_MEDIA_SET_MEDIA_PLAYBACK_STATE, 
VLC_ML_MEDIA_GET_ALL_MEDIA_PLAYBACK_STATES, 
VLC_ML_MEDIA_SET_ALL_MEDIA_PLAYBACK_STATES, 
VLC_ML_MEDIA_SET_THUMBNAIL, 
VLC_ML_MEDIA_GENERATE_THUMBNAIL, 
VLC_ML_MEDIA_ADD_EXTERNAL_MRL, 
VLC_ML_MEDIA_SET_TYPE, 
};
enum vlc_ml_playback_state
{
VLC_ML_PLAYBACK_STATE_RATING,
VLC_ML_PLAYBACK_STATE_PROGRESS,
VLC_ML_PLAYBACK_STATE_SPEED,
VLC_ML_PLAYBACK_STATE_TITLE,
VLC_ML_PLAYBACK_STATE_CHAPTER,
VLC_ML_PLAYBACK_STATE_PROGRAM,
VLC_ML_PLAYBACK_STATE_SEEN,
VLC_ML_PLAYBACK_STATE_VIDEO_TRACK,
VLC_ML_PLAYBACK_STATE_ASPECT_RATIO,
VLC_ML_PLAYBACK_STATE_ZOOM,
VLC_ML_PLAYBACK_STATE_CROP,
VLC_ML_PLAYBACK_STATE_DEINTERLACE,
VLC_ML_PLAYBACK_STATE_VIDEO_FILTER,
VLC_ML_PLAYBACK_STATE_AUDIO_TRACK,
VLC_ML_PLAYBACK_STATE_GAIN,
VLC_ML_PLAYBACK_STATE_AUDIO_DELAY,
VLC_ML_PLAYBACK_STATE_SUBTITLE_TRACK,
VLC_ML_PLAYBACK_STATE_SUBTITLE_DELAY,
VLC_ML_PLAYBACK_STATE_APP_SPECIFIC,
};
typedef struct vlc_ml_playback_states_all
{
float progress;
float rate;
float zoom;
int current_title;
char* current_video_track;
char* current_audio_track;
char *current_subtitle_track;
char* aspect_ratio;
char* crop;
char* deinterlace;
char* video_filter;
} vlc_ml_playback_states_all;
enum vlc_ml_event_type
{
VLC_ML_EVENT_MEDIA_ADDED,
VLC_ML_EVENT_MEDIA_UPDATED,
VLC_ML_EVENT_MEDIA_DELETED,
VLC_ML_EVENT_ARTIST_ADDED,
VLC_ML_EVENT_ARTIST_UPDATED,
VLC_ML_EVENT_ARTIST_DELETED,
VLC_ML_EVENT_ALBUM_ADDED,
VLC_ML_EVENT_ALBUM_UPDATED,
VLC_ML_EVENT_ALBUM_DELETED,
VLC_ML_EVENT_PLAYLIST_ADDED,
VLC_ML_EVENT_PLAYLIST_UPDATED,
VLC_ML_EVENT_PLAYLIST_DELETED,
VLC_ML_EVENT_GENRE_ADDED,
VLC_ML_EVENT_GENRE_UPDATED,
VLC_ML_EVENT_GENRE_DELETED,
VLC_ML_EVENT_DISCOVERY_STARTED,
VLC_ML_EVENT_DISCOVERY_PROGRESS,
VLC_ML_EVENT_DISCOVERY_COMPLETED,
VLC_ML_EVENT_RELOAD_STARTED,
VLC_ML_EVENT_RELOAD_COMPLETED,
VLC_ML_EVENT_ENTRY_POINT_ADDED,
VLC_ML_EVENT_ENTRY_POINT_REMOVED,
VLC_ML_EVENT_ENTRY_POINT_BANNED,
VLC_ML_EVENT_ENTRY_POINT_UNBANNED,
VLC_ML_EVENT_BACKGROUND_IDLE_CHANGED,
VLC_ML_EVENT_PARSING_PROGRESS_UPDATED,
VLC_ML_EVENT_MEDIA_THUMBNAIL_GENERATED,
VLC_ML_EVENT_HISTORY_CHANGED,
VLC_ML_EVENT_RESCAN_STARTED,
};
typedef struct vlc_ml_event_t
{
int i_type;
union
{
struct
{
const char* psz_entry_point;
} discovery_started;
struct
{
const char* psz_entry_point;
} discovery_progress;
struct
{
const char* psz_entry_point;
bool b_success;
} discovery_completed;
struct
{
const char* psz_entry_point;
} reload_started;
struct
{
const char* psz_entry_point;
bool b_success;
} reload_completed;
struct
{
const char* psz_entry_point;
bool b_success;
} entry_point_added;
struct
{
const char* psz_entry_point;
bool b_success;
} entry_point_removed;
struct
{
const char* psz_entry_point;
bool b_success;
} entry_point_banned;
struct
{
const char* psz_entry_point;
bool b_success;
} entry_point_unbanned;
struct
{
uint8_t i_percent;
} parsing_progress;
union
{
const vlc_ml_media_t* p_media;
const vlc_ml_artist_t* p_artist;
const vlc_ml_album_t* p_album;
const vlc_ml_playlist_t* p_playlist;
const vlc_ml_genre_t* p_genre;
} creation;
struct
{
int64_t i_entity_id;
} modification;
struct
{
int64_t i_entity_id;
} deletion;
struct
{
bool b_idle;
} background_idle_changed;
struct
{
const vlc_ml_media_t* p_media;
vlc_ml_thumbnail_size_t i_size;
bool b_success;
} media_thumbnail_generated;
struct
{
vlc_ml_history_type_t history_type;
} history_changed;
};
} vlc_ml_event_t;
typedef void (*vlc_ml_callback_t)( void* p_data, const vlc_ml_event_t* p_event );
typedef struct vlc_medialibrary_callbacks_t
{
void (*pf_send_event)( vlc_medialibrary_module_t* p_ml, const vlc_ml_event_t* p_event );
} vlc_medialibrary_callbacks_t;
struct vlc_medialibrary_module_t
{
struct vlc_object_t obj;
module_t *p_module;
void* p_sys;
int (*pf_control)( struct vlc_medialibrary_module_t* p_ml, int i_query, va_list args );
int (*pf_list)( struct vlc_medialibrary_module_t* p_ml, int i_query,
const vlc_ml_query_params_t* p_params, va_list args );
void* (*pf_get)( struct vlc_medialibrary_module_t* p_ml, int i_query, va_list args );
const vlc_medialibrary_callbacks_t* cbs;
};
vlc_medialibrary_t* libvlc_MlCreate( libvlc_int_t* p_libvlc );
void libvlc_MlRelease( vlc_medialibrary_t* p_ml );
VLC_API vlc_medialibrary_t* vlc_ml_instance_get( vlc_object_t* p_obj ) VLC_USED;
#define vlc_ml_instance_get(x) vlc_ml_instance_get( VLC_OBJECT(x) )
VLC_API void* vlc_ml_get( vlc_medialibrary_t* p_ml, int i_query, ... ) VLC_USED;
VLC_API int vlc_ml_control( vlc_medialibrary_t* p_ml, int i_query, ... ) VLC_USED;
VLC_API int vlc_ml_list( vlc_medialibrary_t* p_ml, int i_query,
const vlc_ml_query_params_t* p_params, ... );
VLC_API vlc_ml_event_callback_t*
vlc_ml_event_register_callback( vlc_medialibrary_t* p_ml, vlc_ml_callback_t cb, void* p_data );
VLC_API void vlc_ml_event_unregister_callback( vlc_medialibrary_t* p_ml,
vlc_ml_event_callback_t* p_callback );
VLC_API void vlc_ml_event_unregister_from_callback( vlc_medialibrary_t* p_ml,
vlc_ml_event_callback_t* p_callback );
VLC_API void vlc_ml_show_release( vlc_ml_show_t* p_show );
VLC_API void vlc_ml_artist_release( vlc_ml_artist_t* p_artist );
VLC_API void vlc_ml_genre_release( vlc_ml_genre_t* p_genre );
VLC_API void vlc_ml_media_release( vlc_ml_media_t* p_media );
VLC_API void vlc_ml_album_release( vlc_ml_album_t* p_album );
VLC_API void vlc_ml_playlist_release( vlc_ml_playlist_t* p_playlist );
VLC_API void vlc_ml_label_list_release( vlc_ml_label_list_t* p_list );
VLC_API void vlc_ml_file_list_release( vlc_ml_file_list_t* p_list );
VLC_API void vlc_ml_artist_list_release( vlc_ml_artist_list_t* p_list );
VLC_API void vlc_ml_media_list_release( vlc_ml_media_list_t* p_list );
VLC_API void vlc_ml_album_list_release( vlc_ml_album_list_t* p_list );
VLC_API void vlc_ml_show_list_release( vlc_ml_show_list_t* p_list );
VLC_API void vlc_ml_genre_list_release( vlc_ml_genre_list_t* p_list );
VLC_API void vlc_ml_playlist_list_release( vlc_ml_playlist_list_t* p_list );
VLC_API void vlc_ml_entry_point_list_release( vlc_ml_entry_point_list_t* p_list );
VLC_API void vlc_ml_playback_states_all_release( vlc_ml_playback_states_all* prefs );
static inline vlc_ml_query_params_t vlc_ml_query_params_create()
{
return (vlc_ml_query_params_t) {
.psz_pattern = NULL,
.i_nbResults = 0,
.i_offset = 0,
.i_sort = VLC_ML_SORTING_DEFAULT,
.b_desc = false
};
}
static inline int vlc_ml_add_folder( vlc_medialibrary_t* p_ml, const char* psz_folder )
{
return vlc_ml_control( p_ml, VLC_ML_ADD_FOLDER, psz_folder );
}
static inline int vlc_ml_remove_folder( vlc_medialibrary_t* p_ml, const char* psz_folder )
{
return vlc_ml_control( p_ml, VLC_ML_REMOVE_FOLDER, psz_folder );
}
static inline int vlc_ml_ban_folder( vlc_medialibrary_t* p_ml, const char* psz_folder )
{
return vlc_ml_control( p_ml, VLC_ML_BAN_FOLDER, psz_folder );
}
static inline int vlc_ml_unban_folder( vlc_medialibrary_t* p_ml, const char* psz_folder )
{
return vlc_ml_control( p_ml, VLC_ML_UNBAN_FOLDER, psz_folder );
}
static inline int vlc_ml_list_folder( vlc_medialibrary_t* p_ml,
vlc_ml_entry_point_list_t** pp_entrypoints )
{
return vlc_ml_control( p_ml, VLC_ML_LIST_FOLDERS, pp_entrypoints );
}
static inline int vlc_ml_is_indexed( vlc_medialibrary_t* p_ml,
const char* psz_mrl, bool* p_res )
{
return vlc_ml_control( p_ml, VLC_ML_IS_INDEXED, psz_mrl, p_res );
}
static inline int vlc_ml_reload_folder( vlc_medialibrary_t* p_ml, const char* psz_mrl )
{
return vlc_ml_control( p_ml, VLC_ML_RELOAD_FOLDER, psz_mrl );
}
static inline int vlc_ml_pause_background( vlc_medialibrary_t* p_ml )
{
return vlc_ml_control( p_ml, VLC_ML_PAUSE_BACKGROUND );
}
static inline int vlc_ml_resume_background( vlc_medialibrary_t* p_ml )
{
return vlc_ml_control( p_ml, VLC_ML_RESUME_BACKGROUND );
}
static inline int vlc_ml_clear_history( vlc_medialibrary_t* p_ml )
{
return vlc_ml_control( p_ml, VLC_ML_CLEAR_HISTORY );
}
static inline vlc_ml_media_t* vlc_ml_new_external_media( vlc_medialibrary_t* p_ml, const char* psz_mrl )
{
vlc_ml_media_t* res;
if ( vlc_ml_control( p_ml, VLC_ML_NEW_EXTERNAL_MEDIA, psz_mrl, &res ) != VLC_SUCCESS )
return NULL;
return res;
}
static inline vlc_ml_media_t* vlc_ml_new_stream( vlc_medialibrary_t* p_ml, const char* psz_mrl )
{
vlc_ml_media_t* res;
if ( vlc_ml_control( p_ml, VLC_ML_NEW_STREAM, psz_mrl, &res ) != VLC_SUCCESS )
return NULL;
return res;
}
static inline int vlc_ml_media_increase_playcount( vlc_medialibrary_t* p_ml, int64_t i_media_id )
{
return vlc_ml_control( p_ml, VLC_ML_MEDIA_INCREASE_PLAY_COUNT, i_media_id );
}
static inline int vlc_ml_media_get_playback_state( vlc_medialibrary_t* p_ml, int64_t i_media_id, int i_state, char** ppsz_result )
{
return vlc_ml_control( p_ml, VLC_ML_MEDIA_GET_MEDIA_PLAYBACK_STATE, i_media_id, i_state, ppsz_result );
}
static inline int vlc_ml_media_set_playback_state( vlc_medialibrary_t* p_ml, int64_t i_media_id, int i_state, const char* psz_value )
{
return vlc_ml_control( p_ml, VLC_ML_MEDIA_SET_MEDIA_PLAYBACK_STATE, i_media_id, i_state, psz_value );
}
static inline int vlc_ml_media_get_all_playback_pref( vlc_medialibrary_t* p_ml,
int64_t i_media_id,
vlc_ml_playback_states_all* prefs )
{
return vlc_ml_control( p_ml,VLC_ML_MEDIA_GET_ALL_MEDIA_PLAYBACK_STATES, i_media_id, prefs );
}
static inline int vlc_ml_media_set_all_playback_states( vlc_medialibrary_t* p_ml,
int64_t i_media_id,
const vlc_ml_playback_states_all* prefs )
{
return vlc_ml_control( p_ml, VLC_ML_MEDIA_SET_ALL_MEDIA_PLAYBACK_STATES, i_media_id, prefs );
}
static inline int vlc_ml_media_set_thumbnail( vlc_medialibrary_t* p_ml, int64_t i_media_id,
const char* psz_mrl, vlc_ml_thumbnail_size_t sizeType )
{
return vlc_ml_control( p_ml, VLC_ML_MEDIA_SET_THUMBNAIL, i_media_id, psz_mrl, sizeType );
}
static inline int vlc_ml_media_generate_thumbnail( vlc_medialibrary_t* p_ml, int64_t i_media_id,
vlc_ml_thumbnail_size_t size_type,
uint32_t i_desired_width,
uint32_t i_desired_height,
float position )
{
return vlc_ml_control( p_ml, VLC_ML_MEDIA_GENERATE_THUMBNAIL, i_media_id,
size_type, i_desired_width, i_desired_height, position );
}
static inline int vlc_ml_media_add_external_mrl( vlc_medialibrary_t* p_ml, int64_t i_media_id,
const char* psz_mrl, int i_type )
{
return vlc_ml_control( p_ml, VLC_ML_MEDIA_ADD_EXTERNAL_MRL, i_media_id, psz_mrl, i_type );
}
static inline int vlc_ml_media_set_type( vlc_medialibrary_t* p_ml, int64_t i_media_id,
vlc_ml_media_type_t i_type )
{
return vlc_ml_control( p_ml, VLC_ML_MEDIA_SET_TYPE, i_media_id, (int)i_type );
}
static inline vlc_ml_media_t* vlc_ml_get_media( vlc_medialibrary_t* p_ml, int64_t i_media_id )
{
return (vlc_ml_media_t*)vlc_ml_get( p_ml, VLC_ML_GET_MEDIA, i_media_id );
}
static inline vlc_ml_media_t* vlc_ml_get_media_by_mrl( vlc_medialibrary_t* p_ml,
const char* psz_mrl )
{
return (vlc_ml_media_t*)vlc_ml_get( p_ml, VLC_ML_GET_MEDIA_BY_MRL, psz_mrl );
}
static inline input_item_t* vlc_ml_get_input_item( vlc_medialibrary_t* p_ml, int64_t i_media_id )
{
return (input_item_t*)vlc_ml_get( p_ml, VLC_ML_GET_INPUT_ITEM, i_media_id );
}
static inline input_item_t* vlc_ml_get_input_item_by_mrl( vlc_medialibrary_t* p_ml,
const char* psz_mrl )
{
return (input_item_t*)vlc_ml_get( p_ml, VLC_ML_GET_INPUT_ITEM_BY_MRL, psz_mrl );
}
static inline vlc_ml_album_t* vlc_ml_get_album( vlc_medialibrary_t* p_ml, int64_t i_album_id )
{
return (vlc_ml_album_t*)vlc_ml_get( p_ml, VLC_ML_GET_ALBUM, i_album_id );
}
static inline vlc_ml_artist_t* vlc_ml_get_artist( vlc_medialibrary_t* p_ml, int64_t i_artist_id )
{
return (vlc_ml_artist_t*)vlc_ml_get( p_ml, VLC_ML_GET_ARTIST, i_artist_id );
}
static inline vlc_ml_genre_t* vlc_ml_get_genre( vlc_medialibrary_t* p_ml, int64_t i_genre_id )
{
return (vlc_ml_genre_t*)vlc_ml_get( p_ml, VLC_ML_GET_GENRE, i_genre_id );
}
static inline vlc_ml_show_t* vlc_ml_get_show( vlc_medialibrary_t* p_ml, int64_t i_show_id )
{
return (vlc_ml_show_t*)vlc_ml_get( p_ml, VLC_ML_GET_SHOW, i_show_id );
}
static inline vlc_ml_playlist_t* vlc_ml_get_playlist( vlc_medialibrary_t* p_ml, int64_t i_playlist_id )
{
return (vlc_ml_playlist_t*)vlc_ml_get( p_ml, VLC_ML_GET_PLAYLIST, i_playlist_id );
}
static inline vlc_ml_media_list_t* vlc_ml_list_media_of( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params, int i_parent_type, int64_t i_parent_id )
{
vlc_assert( p_ml != NULL );
vlc_ml_media_list_t* res;
if ( vlc_ml_list( p_ml, VLC_ML_LIST_MEDIA_OF, params, i_parent_type, i_parent_id, &res ) != VLC_SUCCESS )
return NULL;
return res;
}
static inline size_t vlc_ml_count_media_of( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params, int i_parent_type, int64_t i_parent_id )
{
vlc_assert( p_ml != NULL );
size_t res;
if ( vlc_ml_list( p_ml, VLC_ML_COUNT_MEDIA_OF, params, i_parent_type, i_parent_id, &res ) != VLC_SUCCESS )
return 0;
return res;
}
static inline vlc_ml_artist_list_t* vlc_ml_list_artist_of( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params, int i_parent_type, int64_t i_parent_id )
{
vlc_assert( p_ml != NULL );
vlc_ml_artist_list_t* res;
if ( vlc_ml_list( p_ml, VLC_ML_LIST_ARTISTS_OF, params, i_parent_type, i_parent_id, &res ) != VLC_SUCCESS )
return NULL;
return res;
}
static inline size_t vlc_ml_count_artists_of( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params, int i_parent_type, int64_t i_parent_id )
{
vlc_assert( p_ml != NULL );
size_t res;
if ( vlc_ml_list( p_ml, VLC_ML_COUNT_ARTISTS_OF, params, i_parent_type, i_parent_id, &res ) != VLC_SUCCESS )
return 0;
return res;
}
static inline vlc_ml_album_list_t* vlc_ml_list_albums_of( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params, int i_parent_type, int64_t i_parent_id )
{
vlc_assert( p_ml != NULL );
vlc_ml_album_list_t* res;
if ( vlc_ml_list( p_ml, VLC_ML_LIST_ALBUMS_OF, params, i_parent_type, i_parent_id, &res ) != VLC_SUCCESS )
return NULL;
return res;
}
static inline size_t vlc_ml_count_albums_of( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params, int i_parent_type, int64_t i_parent_id )
{
vlc_assert( p_ml != NULL );
size_t res;
if ( vlc_ml_list( p_ml, VLC_ML_COUNT_ALBUMS_OF, params, i_parent_type, i_parent_id, &res ) != VLC_SUCCESS )
return 0;
return res;
}
static inline vlc_ml_media_list_t* vlc_ml_list_album_tracks( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params, int64_t i_album_id )
{
vlc_assert( p_ml != NULL );
vlc_ml_media_list_t* res;
if ( vlc_ml_list( p_ml, VLC_ML_LIST_ALBUM_TRACKS, params, i_album_id, &res ) != VLC_SUCCESS )
return NULL;
return res;
}
static inline size_t vlc_ml_count_album_tracks( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params, int64_t i_album_id )
{
vlc_assert( p_ml != NULL );
size_t count;
if ( vlc_ml_list( p_ml, VLC_ML_COUNT_ALBUM_TRACKS, params, i_album_id, &count ) != VLC_SUCCESS )
return 0;
return count;
}
static inline vlc_ml_media_list_t* vlc_ml_list_album_artists( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params, int64_t i_album_id )
{
vlc_assert( p_ml != NULL );
vlc_ml_media_list_t* res;
if ( vlc_ml_list( p_ml, VLC_ML_LIST_ALBUM_ARTISTS, params, i_album_id, &res ) != VLC_SUCCESS )
return NULL;
return res;
}
static inline size_t vlc_ml_count_album_artists( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params, int64_t i_album_id )
{
vlc_assert( p_ml != NULL );
size_t count;
if ( vlc_ml_list( p_ml, VLC_ML_COUNT_ALBUM_ARTISTS, params, i_album_id, &count ) != VLC_SUCCESS )
return 0;
return count;
}
static inline vlc_ml_album_list_t* vlc_ml_list_artist_albums( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params, int64_t i_artist_id )
{
vlc_assert( p_ml != NULL );
vlc_ml_album_list_t* res;
if ( vlc_ml_list( p_ml, VLC_ML_LIST_ARTIST_ALBUMS, params, i_artist_id, &res ) != VLC_SUCCESS )
return NULL;
return res;
}
static inline size_t vlc_ml_count_artist_albums( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params, int64_t i_artist_id )
{
vlc_assert( p_ml != NULL );
size_t count;
if ( vlc_ml_list( p_ml, VLC_ML_COUNT_ARTIST_ALBUMS, params, i_artist_id, &count ) != VLC_SUCCESS )
return 0;
return count;
}
static inline vlc_ml_media_list_t* vlc_ml_list_artist_tracks( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params, int64_t i_artist_id )
{
vlc_assert( p_ml != NULL );
vlc_ml_media_list_t* res;
if ( vlc_ml_list( p_ml, VLC_ML_LIST_ARTIST_TRACKS, params, i_artist_id, &res ) != VLC_SUCCESS )
return NULL;
return res;
}
static inline size_t vlc_ml_count_artist_tracks( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params, int64_t i_artist_id )
{
vlc_assert( p_ml != NULL );
size_t count;
if ( vlc_ml_list( p_ml, VLC_ML_COUNT_ARTIST_TRACKS, params, i_artist_id, &count ) != VLC_SUCCESS )
return 0;
return count;
}
static inline vlc_ml_media_list_t* vlc_ml_list_video_media( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params )
{
vlc_assert( p_ml != NULL );
vlc_ml_media_list_t* res;
if ( vlc_ml_list( p_ml, VLC_ML_LIST_VIDEOS, params, &res ) != VLC_SUCCESS )
return NULL;
return res;
}
static inline size_t vlc_ml_count_video_media( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params )
{
vlc_assert( p_ml != NULL );
size_t count;
if ( vlc_ml_list( p_ml, VLC_ML_COUNT_VIDEOS, params, &count ) != VLC_SUCCESS )
return 0;
return count;
}
static inline vlc_ml_media_list_t* vlc_ml_list_audio_media( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params )
{
vlc_assert( p_ml != NULL );
vlc_ml_media_list_t* res;
if ( vlc_ml_list( p_ml, VLC_ML_LIST_AUDIOS, params, &res ) != VLC_SUCCESS )
return NULL;
return res;
}
static inline size_t vlc_ml_count_audio_media( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params )
{
vlc_assert( p_ml != NULL );
size_t count;
if ( vlc_ml_list( p_ml, VLC_ML_COUNT_AUDIOS, params, &count ) != VLC_SUCCESS )
return 0;
return count;
}
static inline vlc_ml_album_list_t* vlc_ml_list_albums( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params )
{
vlc_assert( p_ml != NULL );
vlc_ml_album_list_t* res;
if ( vlc_ml_list( p_ml, VLC_ML_LIST_ALBUMS, params, &res ) != VLC_SUCCESS )
return NULL;
return res;
}
static inline size_t vlc_ml_count_albums( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params )
{
vlc_assert( p_ml != NULL );
size_t count;
if ( vlc_ml_list( p_ml, VLC_ML_COUNT_ALBUMS, params, &count ) != VLC_SUCCESS )
return 0;
return count;
}
static inline vlc_ml_genre_list_t* vlc_ml_list_genres( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params )
{
vlc_assert( p_ml != NULL );
vlc_ml_genre_list_t* res;
if ( vlc_ml_list( p_ml, VLC_ML_LIST_GENRES, params, &res ) != VLC_SUCCESS )
return NULL;
return res;
}
static inline size_t vlc_ml_count_genres( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params )
{
vlc_assert( p_ml != NULL );
size_t count;
if ( vlc_ml_list( p_ml, VLC_ML_COUNT_GENRES, params, &count ) != VLC_SUCCESS )
return 0;
return count;
}
static inline vlc_ml_artist_list_t* vlc_ml_list_artists( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params, bool b_include_all )
{
vlc_assert( p_ml != NULL );
vlc_ml_artist_list_t* res;
if ( vlc_ml_list( p_ml, VLC_ML_LIST_ARTISTS, params, (int)b_include_all, &res ) != VLC_SUCCESS )
return NULL;
return res;
}
static inline size_t vlc_ml_count_artists( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params, bool includeAll )
{
vlc_assert( p_ml != NULL );
size_t count;
if ( vlc_ml_list( p_ml, VLC_ML_COUNT_ARTISTS, params, includeAll, &count ) != VLC_SUCCESS )
return 0;
return count;
}
static inline vlc_ml_show_list_t* vlc_ml_list_shows( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params )
{
vlc_assert( p_ml != NULL );
vlc_ml_show_list_t* res;
if ( vlc_ml_list( p_ml, VLC_ML_LIST_SHOWS, params, &res ) != VLC_SUCCESS )
return NULL;
return res;
}
static inline size_t vlc_ml_count_shows( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params )
{
vlc_assert( p_ml != NULL );
size_t count;
if ( vlc_ml_list( p_ml, VLC_ML_COUNT_SHOWS, params, &count ) != VLC_SUCCESS )
return 0;
return count;
}
static inline vlc_ml_media_list_t* vlc_ml_list_genre_artists( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params, int64_t i_genre_id )
{
vlc_assert( p_ml != NULL );
vlc_ml_media_list_t* res;
if ( vlc_ml_list( p_ml, VLC_ML_LIST_GENRE_ARTISTS, params, i_genre_id, &res ) != VLC_SUCCESS )
return NULL;
return res;
}
static inline size_t vlc_ml_count_genre_artists( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params, int64_t i_genre_id )
{
vlc_assert( p_ml != NULL );
size_t count;
if ( vlc_ml_list( p_ml, VLC_ML_COUNT_GENRE_ARTISTS, params, i_genre_id, &count ) != VLC_SUCCESS )
return 0;
return count;
}
static inline vlc_ml_media_list_t* vlc_ml_list_genre_tracks( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params, int64_t i_genre_id )
{
vlc_assert( p_ml != NULL );
vlc_ml_media_list_t* res;
if ( vlc_ml_list( p_ml, VLC_ML_LIST_GENRE_TRACKS, params, i_genre_id, &res ) != VLC_SUCCESS )
return NULL;
return res;
}
static inline size_t vlc_ml_count_genre_tracks( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params, int64_t i_genre_id )
{
vlc_assert( p_ml != NULL );
size_t count;
if ( vlc_ml_list( p_ml, VLC_ML_COUNT_GENRE_TRACKS, params, i_genre_id, &count ) != VLC_SUCCESS )
return 0;
return count;
}
static inline vlc_ml_album_list_t* vlc_ml_list_genre_albums( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params, int64_t i_genre_id )
{
vlc_assert( p_ml != NULL );
vlc_ml_album_list_t* res;
if ( vlc_ml_list( p_ml, VLC_ML_LIST_GENRE_ALBUMS, params, i_genre_id, &res ) != VLC_SUCCESS )
return NULL;
return res;
}
static inline size_t vlc_ml_count_genre_albums( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params, int64_t i_genre_id )
{
vlc_assert( p_ml != NULL );
size_t count;
if ( vlc_ml_list( p_ml, VLC_ML_COUNT_GENRE_ALBUMS, params, i_genre_id, &count ) != VLC_SUCCESS )
return 0;
return count;
}
static inline vlc_ml_media_list_t* vlc_ml_list_show_episodes( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params, int64_t i_show_id )
{
vlc_assert( p_ml != NULL );
vlc_ml_media_list_t* res;
if ( vlc_ml_list( p_ml, VLC_ML_LIST_SHOW_EPISODES, params, i_show_id, &res ) != VLC_SUCCESS )
return NULL;
return res;
}
static inline size_t vlc_ml_count_show_episodes( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params, int64_t i_show_id )
{
vlc_assert( p_ml != NULL );
size_t count;
if ( vlc_ml_list( p_ml, VLC_ML_COUNT_GENRE_ALBUMS, params, i_show_id, &count ) != VLC_SUCCESS )
return 0;
return count;
}
static inline vlc_ml_label_list_t* vlc_ml_list_media_labels( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params, int64_t i_media_id )
{
vlc_assert( p_ml != NULL );
vlc_ml_label_list_t* res;
if ( vlc_ml_list( p_ml, VLC_ML_LIST_MEDIA_LABELS, params, i_media_id, &res ) != VLC_SUCCESS )
return NULL;
return res;
}
static inline size_t vlc_ml_count_media_labels( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params, int64_t i_media_id )
{
vlc_assert( p_ml != NULL );
size_t count;
if ( vlc_ml_list( p_ml, VLC_ML_LIST_MEDIA_LABELS, params, i_media_id, &count ) != VLC_SUCCESS )
return 0;
return count;
}
static inline vlc_ml_media_list_t* vlc_ml_list_history( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params )
{
vlc_assert( p_ml != NULL );
vlc_ml_media_list_t* res;
if ( vlc_ml_list( p_ml, VLC_ML_LIST_HISTORY, params, &res ) != VLC_SUCCESS )
return NULL;
return res;
}
static inline vlc_ml_media_list_t* vlc_ml_list_stream_history( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params )
{
vlc_assert( p_ml != NULL );
vlc_ml_media_list_t* res;
if ( vlc_ml_list( p_ml, VLC_ML_LIST_STREAM_HISTORY, params, &res ) != VLC_SUCCESS )
return NULL;
return res;
}
static inline vlc_ml_playlist_list_t* vlc_ml_list_playlists( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params )
{
vlc_assert( p_ml != NULL );
vlc_ml_playlist_list_t* res;
if ( vlc_ml_list( p_ml, VLC_ML_LIST_PLAYLISTS, params, &res ) != VLC_SUCCESS )
return NULL;
return res;
}
static inline size_t vlc_ml_count_playlists( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params )
{
vlc_assert( p_ml != NULL );
size_t count;
if ( vlc_ml_list( p_ml, VLC_ML_COUNT_PLAYLISTS, params, &count ) != VLC_SUCCESS )
return 0;
return count;
}
static inline vlc_ml_media_list_t* vlc_ml_list_playlist_media( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params, int64_t i_playlist_id )
{
vlc_assert( p_ml != NULL );
vlc_ml_media_list_t* res;
if ( vlc_ml_list( p_ml, VLC_ML_LIST_PLAYLIST_MEDIA, params, i_playlist_id, &res ) != VLC_SUCCESS )
return NULL;
return res;
}
static inline size_t vlc_ml_count_playlist_media( vlc_medialibrary_t* p_ml, const vlc_ml_query_params_t* params, int64_t i_playlist_id )
{
vlc_assert( p_ml != NULL );
size_t count;
if ( vlc_ml_list( p_ml, VLC_ML_COUNT_PLAYLIST_MEDIA, params, i_playlist_id, &count ) != VLC_SUCCESS )
return 0;
return count;
}
#if defined(__cplusplus)
}
#endif 
#if !defined(__cplusplus)
#define vlc_ml_release( OBJ ) _Generic( ( OBJ ), vlc_ml_show_t*: vlc_ml_show_release, vlc_ml_artist_t*: vlc_ml_artist_release, vlc_ml_album_t*: vlc_ml_album_release, vlc_ml_genre_t*: vlc_ml_genre_release, vlc_ml_media_t*: vlc_ml_media_release, vlc_ml_playlist_t*: vlc_ml_playlist_release, vlc_ml_label_list_t*: vlc_ml_label_list_release, vlc_ml_file_list_t*: vlc_ml_file_list_release, vlc_ml_artist_list_t*: vlc_ml_artist_list_release, vlc_ml_media_list_t*: vlc_ml_media_list_release, vlc_ml_album_list_t*: vlc_ml_album_list_release, vlc_ml_show_list_t*: vlc_ml_show_list_release, vlc_ml_genre_list_t*: vlc_ml_genre_list_release, vlc_ml_playlist_list_t*: vlc_ml_playlist_list_release, vlc_ml_entry_point_list_t*: vlc_ml_entry_point_list_release, vlc_ml_playback_states_all*: vlc_ml_playback_states_all_release )( OBJ )
#else
static inline void vlc_ml_release( vlc_ml_show_t* show ) { vlc_ml_show_release( show ); }
static inline void vlc_ml_release( vlc_ml_artist_t* artist ) { vlc_ml_artist_release( artist ); }
static inline void vlc_ml_release( vlc_ml_album_t* album ) { vlc_ml_album_release( album ); }
static inline void vlc_ml_release( vlc_ml_genre_t* genre ) { vlc_ml_genre_release( genre ); }
static inline void vlc_ml_release( vlc_ml_media_t* media ) { vlc_ml_media_release( media ); }
static inline void vlc_ml_release( vlc_ml_playlist_t* playlist ) { vlc_ml_playlist_release( playlist ); }
static inline void vlc_ml_release( vlc_ml_label_list_t* list ) { vlc_ml_label_list_release( list ); }
static inline void vlc_ml_release( vlc_ml_file_list_t* list ) { vlc_ml_file_list_release( list ); }
static inline void vlc_ml_release( vlc_ml_artist_list_t* list ) { vlc_ml_artist_list_release( list ); }
static inline void vlc_ml_release( vlc_ml_media_list_t* list ) { vlc_ml_media_list_release( list ); }
static inline void vlc_ml_release( vlc_ml_album_list_t* list ) { vlc_ml_album_list_release( list ); }
static inline void vlc_ml_release( vlc_ml_show_list_t* list ) { vlc_ml_show_list_release( list ); }
static inline void vlc_ml_release( vlc_ml_genre_list_t* list ) { vlc_ml_genre_list_release( list ); }
static inline void vlc_ml_release( vlc_ml_playlist_list_t* list ) { vlc_ml_playlist_list_release( list ); }
static inline void vlc_ml_release( vlc_ml_entry_point_list_t* list ) { vlc_ml_entry_point_list_release( list ); }
static inline void vlc_ml_release( vlc_ml_playback_states_all* prefs ) { vlc_ml_playback_states_all_release( prefs ); }
#endif
