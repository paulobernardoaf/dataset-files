



















#if !defined(MEDIALIBRARY_H)
#define MEDIALIBRARY_H

#include <medialibrary/IMediaLibrary.h>
#include <medialibrary/parser/IParserService.h>
#include <medialibrary/parser/IItem.h>
#include <medialibrary/parser/Parser.h>
#include <medialibrary/IMedia.h>
#include <medialibrary/IThumbnailer.h>

#include <vlc_common.h>
#include <vlc_threads.h>
#include <vlc_input_item.h>
#include <vlc_input.h>
#include <vlc_media_library.h>
#include <vlc_cxx_helpers.hpp>

#include <cstdarg>

struct vlc_event_t;
struct vlc_object_t;
struct vlc_thumbnailer_t;
struct vlc_thumbnailer_request_t;

class Logger;

class MetadataExtractor : public medialibrary::parser::IParserService
{
private:
struct ParseContext
{
ParseContext( MetadataExtractor* mde, medialibrary::parser::IItem& item )
: needsProbing( false )
, success( false )
, mde( mde )
, item( item )
, inputItem( nullptr, &input_item_Release )
, inputParser( nullptr, &input_item_parser_id_Release )
{
}

bool needsProbing;
bool success;
MetadataExtractor* mde;
medialibrary::parser::IItem& item;
std::unique_ptr<input_item_t, decltype(&input_item_Release)> inputItem;


std::unique_ptr<input_item_parser_id_t, decltype(&input_item_parser_id_Release)> inputParser;
};

public:
MetadataExtractor( vlc_object_t* parent );
virtual ~MetadataExtractor() = default;



private:
virtual medialibrary::parser::Status run( medialibrary::parser::IItem& item ) override;
virtual const char*name() const override;
virtual medialibrary::parser::Step targetedStep() const override;
virtual bool initialize( medialibrary::IMediaLibrary* ml ) override;
virtual void onFlushing() override;
virtual void onRestarted() override;
virtual void stop() override;

void onParserEnded( ParseContext& ctx, int status );
void addSubtree( ParseContext& ctx, input_item_node_t *root );
void populateItem( medialibrary::parser::IItem& item, input_item_t* inputItem );

static void onParserEnded( input_item_t *, int status, void *user_data );
static void onParserSubtreeAdded( input_item_t *, input_item_node_t *subtree,
void *user_data );

private:
vlc::threads::condition_variable m_cond;
vlc::threads::mutex m_mutex;
ParseContext* m_currentCtx;
vlc_object_t* m_obj;
};

class Thumbnailer : public medialibrary::IThumbnailer
{
struct ThumbnailerCtx
{
~ThumbnailerCtx()
{
if ( thumbnail != nullptr )
picture_Release( thumbnail );
}
Thumbnailer* thumbnailer;
bool done;
picture_t* thumbnail;
vlc_thumbnailer_request_t* request;
};
public:
Thumbnailer(vlc_medialibrary_module_t* ml);
virtual bool generate( const medialibrary::IMedia&, const std::string& mrl,
uint32_t desiredWidth, uint32_t desiredHeight,
float position, const std::string& dest ) override;
virtual void stop() override;

private:
static void onThumbnailComplete( void* data, picture_t* thumbnail );

private:
vlc_medialibrary_module_t* m_ml;
vlc::threads::mutex m_mutex;
vlc::threads::condition_variable m_cond;
ThumbnailerCtx* m_currentContext;
std::unique_ptr<vlc_thumbnailer_t, void(*)(vlc_thumbnailer_t*)> m_thumbnailer;
};

class MediaLibrary : public medialibrary::IMediaLibraryCb
{
public:
MediaLibrary( vlc_medialibrary_module_t* ml );
bool Init();
bool Start();
int Control( int query, va_list args );
int List( int query, const vlc_ml_query_params_t* params, va_list args );
void* Get( int query, va_list args );

private:
int controlMedia( int query, va_list args );
int getMeta( const medialibrary::IMedia& media, int meta, char** result );
int getMeta( const medialibrary::IMedia& media, vlc_ml_playback_states_all* result );
int setMeta( medialibrary::IMedia& media, int meta, const char* value );
int setMeta( medialibrary::IMedia& media, const vlc_ml_playback_states_all* values );
int filterListChildrenQuery( int query, int parentType );
int listAlbums( int listQuery, const medialibrary::QueryParameters* paramsPtr,
const char* pattern, uint32_t nbItems, uint32_t offset, va_list args );
int listArtists( int listQuery, const medialibrary::QueryParameters* paramsPtr,
const char* pattern, uint32_t nbItems, uint32_t offset, va_list args );
int listGenre( int listQuery, const medialibrary::QueryParameters* paramsPtr,
const char* pattern, uint32_t nbItems, uint32_t offset, va_list args );
int listPlaylist( int listQuery, const medialibrary::QueryParameters* paramsPtr,
const char* pattern, uint32_t nbItems, uint32_t offset, va_list args );

static medialibrary::IMedia::MetadataType metadataType( int meta );
static medialibrary::SortingCriteria sortingCriteria( int sort );

private:
vlc_medialibrary_module_t* m_vlc_ml;
std::unique_ptr<Logger> m_logger;
std::unique_ptr<medialibrary::IMediaLibrary> m_ml;


public:
virtual void onMediaAdded(std::vector<medialibrary::MediaPtr> media) override;
virtual void onMediaModified(std::vector<int64_t> media) override;
virtual void onMediaDeleted(std::vector<int64_t> mediaIds) override;
virtual void onArtistsAdded(std::vector<medialibrary::ArtistPtr> artists) override;
virtual void onArtistsModified(std::vector<int64_t> artists) override;
virtual void onArtistsDeleted(std::vector<int64_t> artistsIds) override;
virtual void onAlbumsAdded(std::vector<medialibrary::AlbumPtr> albums) override;
virtual void onAlbumsModified(std::vector<int64_t> albums) override;
virtual void onAlbumsDeleted(std::vector<int64_t> albumsIds) override;
virtual void onPlaylistsAdded(std::vector<medialibrary::PlaylistPtr> playlists) override;
virtual void onPlaylistsModified(std::vector<int64_t> playlists) override;
virtual void onPlaylistsDeleted(std::vector<int64_t> playlistIds) override;
virtual void onGenresAdded(std::vector<medialibrary::GenrePtr> genres) override;
virtual void onGenresModified(std::vector<int64_t> genres) override;
virtual void onGenresDeleted(std::vector<int64_t> genreIds) override;
virtual void onMediaGroupAdded( std::vector<medialibrary::MediaGroupPtr> mediaGroups ) override;
virtual void onMediaGroupModified( std::vector<int64_t> mediaGroupsIds ) override;
virtual void onMediaGroupDeleted( std::vector<int64_t> mediaGroupsIds ) override;
virtual void onDiscoveryStarted(const std::string& entryPoint) override;
virtual void onDiscoveryProgress(const std::string& entryPoint) override;
virtual void onDiscoveryCompleted(const std::string& entryPoint, bool success) override;
virtual void onReloadStarted(const std::string& entryPoint) override;
virtual void onReloadCompleted(const std::string& entryPoint, bool success) override;
virtual void onEntryPointAdded(const std::string& entryPoint, bool success) override;
virtual void onEntryPointRemoved(const std::string& entryPoint, bool success) override;
virtual void onEntryPointBanned(const std::string& entryPoint, bool success) override;
virtual void onEntryPointUnbanned(const std::string& entryPoint, bool success) override;
virtual void onParsingStatsUpdated(uint32_t percent) override;
virtual void onBackgroundTasksIdleChanged(bool isIdle) override;
virtual void onMediaThumbnailReady(medialibrary::MediaPtr media,
medialibrary::ThumbnailSizeType sizeType,
bool success) override;
virtual void onHistoryChanged( medialibrary::HistoryType historyType ) override;
virtual void onRescanStarted() override;
};

bool Convert( const medialibrary::IMedia* input, vlc_ml_media_t& output );
bool Convert( const medialibrary::IFile* input, vlc_ml_file_t& output );
bool Convert( const medialibrary::IMovie* input, vlc_ml_movie_t& output );
bool Convert( const medialibrary::IShowEpisode* input, vlc_ml_show_episode_t& output );
bool Convert( const medialibrary::IAlbumTrack* input, vlc_ml_album_track_t& output );
bool Convert( const medialibrary::IAlbum* input, vlc_ml_album_t& output );
bool Convert( const medialibrary::IArtist* input, vlc_ml_artist_t& output );
bool Convert( const medialibrary::IGenre* input, vlc_ml_genre_t& output );
bool Convert( const medialibrary::IShow* input, vlc_ml_show_t& output );
bool Convert( const medialibrary::ILabel* input, vlc_ml_label_t& output );
bool Convert( const medialibrary::IPlaylist* input, vlc_ml_playlist_t& output );
bool Convert( const medialibrary::IFolder* input, vlc_ml_entry_point_t& output );
input_item_t* MediaToInputItem( const medialibrary::IMedia* media );

template <typename To, typename ItemType, typename From>
To* ml_convert_list( const std::vector<std::shared_ptr<From>>& input )
{

static_assert( std::is_pointer<To>::value == false,
"Destination type must not be a pointer" );


static_assert( std::is_array<decltype(To::p_items)>::value == false ||
( std::is_same<typename std::remove_extent<decltype(To::p_items)>::type,
ItemType>::value ), "Invalid/mismatching list/item types" );


auto list = vlc::wrap_cptr(
static_cast<To*>( calloc( 1, sizeof( To ) + input.size() * sizeof( ItemType ) ) ),
static_cast<void(*)(To*)>( &vlc_ml_release ) );
if ( unlikely( list == nullptr ) )
return nullptr;

list->i_nb_items = 0;

for ( auto i = 0u; i < input.size(); ++i )
{
if ( Convert( input[i].get(), list->p_items[i] ) == false )
return nullptr;
list->i_nb_items++;
}
return list.release();
}

template <typename T, typename Input>
T* CreateAndConvert( const Input* input )
{
if ( input == nullptr )
return nullptr;
auto res = vlc::wrap_cptr(
static_cast<T*>( calloc( 1, sizeof( T ) ) ),
static_cast<void(*)(T*)>( &vlc_ml_release ) );
if ( unlikely( res == nullptr ) )
return nullptr;
if ( Convert( input, *res ) == false )
return nullptr;





return res.release();
}



#endif 
