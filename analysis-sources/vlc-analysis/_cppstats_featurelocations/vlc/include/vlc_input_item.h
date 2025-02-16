






















#if !defined(VLC_INPUT_ITEM_H)
#define VLC_INPUT_ITEM_H 1






#include <vlc_meta.h>
#include <vlc_epg.h>
#include <vlc_events.h>
#include <vlc_list.h>

#include <string.h>

typedef struct input_item_opaque input_item_opaque_t;
typedef struct input_item_slave input_item_slave_t;
typedef struct input_preparser_callbacks_t input_preparser_callbacks_t;

struct info_t
{
char *psz_name; 
char *psz_value; 
struct vlc_list node;
};

#define info_foreach(info, cat) vlc_list_foreach(info, cat, node)

struct info_category_t
{
char *psz_name; 
struct vlc_list infos; 
};

enum input_item_type_e
{
ITEM_TYPE_UNKNOWN,
ITEM_TYPE_FILE,
ITEM_TYPE_DIRECTORY,
ITEM_TYPE_DISC,
ITEM_TYPE_CARD,
ITEM_TYPE_STREAM,
ITEM_TYPE_PLAYLIST,
ITEM_TYPE_NODE,


ITEM_TYPE_NUMBER
};




struct input_item_t
{
char *psz_name; 
char *psz_uri; 

int i_options; 
char **ppsz_options; 
uint8_t *optflagv; 
unsigned optflagc;
input_item_opaque_t *opaques; 

vlc_tick_t i_duration; 


int i_categories; 
info_category_t **pp_categories; 

int i_es; 
es_format_t **es; 

input_stats_t *p_stats; 

vlc_meta_t *p_meta;

int i_epg; 
vlc_epg_t **pp_epg; 
int64_t i_epg_time; 
const vlc_epg_t *p_epg_table; 

int i_slaves; 
input_item_slave_t **pp_slaves; 


vlc_event_manager_t event_manager;

vlc_mutex_t lock; 

enum input_item_type_e i_type; 
bool b_net; 

bool b_error_when_reading;

int i_preparse_depth; 


bool b_preparse_interact; 

};

#define INPUT_ITEM_URI_NOP "vlc://nop" 



#define INPUT_DURATION_UNSET VLC_TICK_INVALID
#define INPUT_DURATION_INDEFINITE (-1) 

enum input_item_net_type
{
ITEM_NET_UNKNOWN,
ITEM_NET,
ITEM_LOCAL
};

enum slave_type
{
SLAVE_TYPE_SPU,
SLAVE_TYPE_AUDIO,
};

enum slave_priority
{
SLAVE_PRIORITY_MATCH_NONE = 1,
SLAVE_PRIORITY_MATCH_RIGHT,
SLAVE_PRIORITY_MATCH_LEFT,
SLAVE_PRIORITY_MATCH_ALL,
SLAVE_PRIORITY_USER
};


#define MASTER_EXTENSIONS "asf", "avi", "divx", "f4v", "flv", "m1v", "m2v", "m4v", "mkv", "mov", "mp2", "mp2v", "mp4", "mp4v", "mpe", "mpeg", "mpeg1", "mpeg2", "mpeg4", "mpg", "mpv2", "mxf", "ogv", "ogx", "ps", "vro","webm", "wmv", "wtv"











#define SLAVE_SPU_EXTENSIONS "aqt", "ass", "cdg", "dks", "idx", "jss", "mpl2", "mpsub", "pjs", "psb", "rt", "sami", "sbv", "scc", "smi", "srt", "ssa", "stl", "sub", "tt", "ttml", "usf", "vtt", "webvtt"









#define SLAVE_AUDIO_EXTENSIONS "aac", "ac3", "dts", "dtshd", "eac3", "flac", "m4a", "mp3", "pcm" 




struct input_item_slave
{
enum slave_type i_type; 
enum slave_priority i_priority; 
bool b_forced; 
char psz_uri[]; 
};

struct input_item_node_t
{
input_item_t * p_item;
int i_children;
input_item_node_t **pp_children;
};

VLC_API void input_item_CopyOptions( input_item_t *p_child, input_item_t *p_parent );
VLC_API void input_item_SetName( input_item_t *p_item, const char *psz_name );






VLC_API input_item_node_t * input_item_node_Create( input_item_t *p_input ) VLC_USED;




VLC_API input_item_node_t * input_item_node_AppendItem( input_item_node_t *p_node, input_item_t *p_item );




VLC_API void input_item_node_AppendNode( input_item_node_t *p_parent, input_item_node_t *p_child );




VLC_API void input_item_node_RemoveNode( input_item_node_t *parent,
input_item_node_t *child );




VLC_API void input_item_node_Delete( input_item_node_t *p_node );




enum input_item_option_e
{


VLC_INPUT_OPTION_TRUSTED = 0x2,



VLC_INPUT_OPTION_UNIQUE = 0x100,
};




VLC_API int input_item_AddOption(input_item_t *, const char *, unsigned i_flags );



VLC_API int input_item_AddOptions(input_item_t *, int i_options,
const char *const *ppsz_options,
unsigned i_flags );
VLC_API int input_item_AddOpaque(input_item_t *, const char *, void *);

void input_item_ApplyOptions(vlc_object_t *, input_item_t *);

VLC_API bool input_item_slave_GetType(const char *, enum slave_type *);

VLC_API input_item_slave_t *input_item_slave_New(const char *, enum slave_type,
enum slave_priority);
#define input_item_slave_Delete(p_slave) free(p_slave)





VLC_API int input_item_AddSlave(input_item_t *, input_item_slave_t *);


VLC_API bool input_item_HasErrorWhenReading( input_item_t * );
VLC_API void input_item_SetMeta( input_item_t *, vlc_meta_type_t meta_type, const char *psz_val );
VLC_API bool input_item_MetaMatch( input_item_t *p_i, vlc_meta_type_t meta_type, const char *psz );
VLC_API char * input_item_GetMeta( input_item_t *p_i, vlc_meta_type_t meta_type ) VLC_USED;
VLC_API const char *input_item_GetMetaLocked(input_item_t *, vlc_meta_type_t meta_type);
VLC_API char * input_item_GetName( input_item_t * p_i ) VLC_USED;
VLC_API char * input_item_GetTitleFbName( input_item_t * p_i ) VLC_USED;
VLC_API char * input_item_GetURI( input_item_t * p_i ) VLC_USED;
VLC_API char * input_item_GetNowPlayingFb( input_item_t *p_item ) VLC_USED;
VLC_API void input_item_SetURI( input_item_t * p_i, const char *psz_uri );
VLC_API vlc_tick_t input_item_GetDuration( input_item_t * p_i );
VLC_API void input_item_SetDuration( input_item_t * p_i, vlc_tick_t i_duration );
VLC_API bool input_item_IsPreparsed( input_item_t *p_i );
VLC_API bool input_item_IsArtFetched( input_item_t *p_i );

#define INPUT_META( name ) static inline void input_item_Set ##name (input_item_t *p_input, const char *val) { input_item_SetMeta (p_input, vlc_meta_ ##name, val); } static inline char *input_item_Get ##name (input_item_t *p_input) { return input_item_GetMeta (p_input, vlc_meta_ ##name); }











INPUT_META(Title)
INPUT_META(Artist)
INPUT_META(AlbumArtist)
INPUT_META(Genre)
INPUT_META(Copyright)
INPUT_META(Album)
INPUT_META(TrackNumber)
INPUT_META(Description)
INPUT_META(Rating)
INPUT_META(Date)
INPUT_META(Setting)
INPUT_META(URL)
INPUT_META(Language)
INPUT_META(NowPlaying)
INPUT_META(ESNowPlaying)
INPUT_META(Publisher)
INPUT_META(EncodedBy)
INPUT_META(ArtworkURL)
INPUT_META(TrackID)
INPUT_META(TrackTotal)
INPUT_META(Director)
INPUT_META(Season)
INPUT_META(Episode)
INPUT_META(ShowName)
INPUT_META(Actors)
INPUT_META(DiscNumber)
INPUT_META(DiscTotal)

#define input_item_SetTrackNum input_item_SetTrackNumber
#define input_item_GetTrackNum input_item_GetTrackNumber
#define input_item_SetArtURL input_item_SetArtworkURL
#define input_item_GetArtURL input_item_GetArtworkURL

VLC_API char * input_item_GetInfo( input_item_t *p_i, const char *psz_cat,const char *psz_name ) VLC_USED;
VLC_API int input_item_AddInfo( input_item_t *p_i, const char *psz_cat, const char *psz_name, const char *psz_format, ... ) VLC_FORMAT( 4, 5 );
VLC_API int input_item_DelInfo( input_item_t *p_i, const char *psz_cat, const char *psz_name );
VLC_API void input_item_ReplaceInfos( input_item_t *, info_category_t * );
VLC_API void input_item_MergeInfos( input_item_t *, info_category_t * );






VLC_API input_item_t * input_item_NewExt( const char *psz_uri,
const char *psz_name,
vlc_tick_t i_duration, enum input_item_type_e i_type,
enum input_item_net_type i_net ) VLC_USED;

#define input_item_New( psz_uri, psz_name ) input_item_NewExt( psz_uri, psz_name, INPUT_DURATION_UNSET, ITEM_TYPE_UNKNOWN, ITEM_NET_UNKNOWN )


#define input_item_NewCard( psz_uri, psz_name ) input_item_NewExt( psz_uri, psz_name, INPUT_DURATION_INDEFINITE, ITEM_TYPE_CARD, ITEM_LOCAL )


#define input_item_NewDisc( psz_uri, psz_name, i_duration ) input_item_NewExt( psz_uri, psz_name, i_duration, ITEM_TYPE_DISC, ITEM_LOCAL )


#define input_item_NewStream( psz_uri, psz_name, i_duration ) input_item_NewExt( psz_uri, psz_name, i_duration, ITEM_TYPE_STREAM, ITEM_NET )


#define input_item_NewDirectory( psz_uri, psz_name, i_net ) input_item_NewExt( psz_uri, psz_name, INPUT_DURATION_UNSET, ITEM_TYPE_DIRECTORY, i_net )


#define input_item_NewFile( psz_uri, psz_name, i_duration, i_net ) input_item_NewExt( psz_uri, psz_name, i_duration, ITEM_TYPE_FILE, i_net )





VLC_API input_item_t * input_item_Copy(input_item_t * ) VLC_USED;


VLC_API input_item_t *input_item_Hold(input_item_t *);


VLC_API void input_item_Release(input_item_t *);





#define INPUT_RECORD_PREFIX "vlc-record-%Y-%m-%d-%Hh%Mm%Ss-$ N-$ p"




VLC_API char * input_item_CreateFilename( input_item_t *,
const char *psz_path, const char *psz_prefix,
const char *psz_extension ) VLC_USED;




typedef struct input_item_parser_id_t input_item_parser_id_t;




typedef struct input_item_parser_cbs_t
{









void (*on_ended)(input_item_t *item, int status, void *userdata);










void (*on_subtree_added)(input_item_t *item, input_item_node_t *subtree, void *userdata);
} input_item_parser_cbs_t;
















VLC_API input_item_parser_id_t *
input_item_Parse(input_item_t *item, vlc_object_t *parent,
const input_item_parser_cbs_t *cbs, void *userdata) VLC_USED;










VLC_API void
input_item_parser_id_Interrupt(input_item_parser_id_t *parser);






VLC_API void
input_item_parser_id_Release(input_item_parser_id_t *parser);

typedef enum input_item_meta_request_option_t
{
META_REQUEST_OPTION_NONE = 0x00,
META_REQUEST_OPTION_SCOPE_LOCAL = 0x01,
META_REQUEST_OPTION_SCOPE_NETWORK = 0x02,
META_REQUEST_OPTION_SCOPE_ANY = 0x03,
META_REQUEST_OPTION_FETCH_LOCAL = 0x04,
META_REQUEST_OPTION_FETCH_NETWORK = 0x08,
META_REQUEST_OPTION_FETCH_ANY = 0x0C,
META_REQUEST_OPTION_DO_INTERACT = 0x10,
} input_item_meta_request_option_t;


enum input_item_preparse_status
{
ITEM_PREPARSE_SKIPPED,
ITEM_PREPARSE_FAILED,
ITEM_PREPARSE_TIMEOUT,
ITEM_PREPARSE_DONE
};

typedef struct input_preparser_callbacks_t {
void (*on_preparse_ended)(input_item_t *, enum input_item_preparse_status status, void *userdata);
void (*on_subtree_added)(input_item_t *, input_item_node_t *subtree, void *userdata);
} input_preparser_callbacks_t;

typedef struct input_fetcher_callbacks_t {
void (*on_art_fetch_ended)(input_item_t *, bool fetched, void *userdata);
} input_fetcher_callbacks_t;

VLC_API int libvlc_MetadataRequest( libvlc_int_t *, input_item_t *,
input_item_meta_request_option_t,
const input_preparser_callbacks_t *cbs,
void *cbs_userdata,
int, void * );
VLC_API int libvlc_ArtRequest(libvlc_int_t *, input_item_t *,
input_item_meta_request_option_t,
const input_fetcher_callbacks_t *cbs,
void *cbs_userdata );
VLC_API void libvlc_MetadataCancel( libvlc_int_t *, void * );




struct input_stats_t
{

int64_t i_read_packets;
int64_t i_read_bytes;
float f_input_bitrate;


int64_t i_demux_read_packets;
int64_t i_demux_read_bytes;
float f_demux_bitrate;
int64_t i_demux_corrupted;
int64_t i_demux_discontinuity;


int64_t i_decoded_audio;
int64_t i_decoded_video;


int64_t i_displayed_pictures;
int64_t i_lost_pictures;


int64_t i_played_abuffers;
int64_t i_lost_abuffers;
};







struct vlc_readdir_helper
{
input_item_node_t *p_node;
void **pp_slaves;
size_t i_slaves;
void **pp_dirs;
size_t i_dirs;
int i_sub_autodetect_fuzzy;
bool b_show_hiddenfiles;
bool b_flatten;
char *psz_ignored_exts;
};







VLC_API void vlc_readdir_helper_init(struct vlc_readdir_helper *p_rdh,
vlc_object_t *p_obj, input_item_node_t *p_node);
#define vlc_readdir_helper_init(p_rdh, p_obj, p_node) vlc_readdir_helper_init(p_rdh, VLC_OBJECT(p_obj), p_node)







VLC_API void vlc_readdir_helper_finish(struct vlc_readdir_helper *p_rdh, bool b_success);
















VLC_API int vlc_readdir_helper_additem(struct vlc_readdir_helper *p_rdh,
const char *psz_uri, const char *psz_flatpath,
const char *psz_filename,
int i_type, int i_net);

#endif
