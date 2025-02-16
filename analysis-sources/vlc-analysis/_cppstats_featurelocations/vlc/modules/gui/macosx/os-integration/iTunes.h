



















#import <AppKit/AppKit.h>
#import <ScriptingBridge/ScriptingBridge.h>


@class iTunesPrintSettings, iTunesApplication, iTunesItem, iTunesArtwork, iTunesEncoder, iTunesEQPreset, iTunesPlaylist, iTunesAudioCDPlaylist, iTunesLibraryPlaylist, iTunesRadioTunerPlaylist, iTunesSource, iTunesTrack, iTunesAudioCDTrack, iTunesFileTrack, iTunesSharedTrack, iTunesURLTrack, iTunesUserPlaylist, iTunesFolderPlaylist, iTunesVisual, iTunesWindow, iTunesBrowserWindow, iTunesEQWindow, iTunesPlaylistWindow;

enum iTunesEKnd {
iTunesEKndTrackListing = 'kTrk' ,
iTunesEKndAlbumListing = 'kAlb' ,
iTunesEKndCdInsert = 'kCDi' 
};
typedef enum iTunesEKnd iTunesEKnd;

enum iTunesEnum {
iTunesEnumStandard = 'lwst' ,
iTunesEnumDetailed = 'lwdt' 
};
typedef enum iTunesEnum iTunesEnum;

enum iTunesEPlS {
iTunesEPlSStopped = 'kPSS',
iTunesEPlSPlaying = 'kPSP',
iTunesEPlSPaused = 'kPSp',
iTunesEPlSFastForwarding = 'kPSF',
iTunesEPlSRewinding = 'kPSR'
};
typedef enum iTunesEPlS iTunesEPlS;

enum iTunesERpt {
iTunesERptOff = 'kRpO',
iTunesERptOne = 'kRp1',
iTunesERptAll = 'kAll'
};
typedef enum iTunesERpt iTunesERpt;

enum iTunesEVSz {
iTunesEVSzSmall = 'kVSS',
iTunesEVSzMedium = 'kVSM',
iTunesEVSzLarge = 'kVSL'
};
typedef enum iTunesEVSz iTunesEVSz;

enum iTunesESrc {
iTunesESrcLibrary = 'kLib',
iTunesESrcIPod = 'kPod',
iTunesESrcAudioCD = 'kACD',
iTunesESrcMP3CD = 'kMCD',
iTunesESrcRadioTuner = 'kTun',
iTunesESrcSharedLibrary = 'kShd',
iTunesESrcUnknown = 'kUnk'
};
typedef enum iTunesESrc iTunesESrc;

enum iTunesESrA {
iTunesESrAAlbums = 'kSrL' ,
iTunesESrAAll = 'kAll' ,
iTunesESrAArtists = 'kSrR' ,
iTunesESrAComposers = 'kSrC' ,
iTunesESrADisplayed = 'kSrV' ,
iTunesESrASongs = 'kSrS' 
};
typedef enum iTunesESrA iTunesESrA;

enum iTunesESpK {
iTunesESpKNone = 'kNon',
iTunesESpKBooks = 'kSpA',
iTunesESpKFolder = 'kSpF',
iTunesESpKGenius = 'kSpG',
iTunesESpKITunesU = 'kSpU',
iTunesESpKLibrary = 'kSpL',
iTunesESpKMovies = 'kSpI',
iTunesESpKMusic = 'kSpZ',
iTunesESpKPodcasts = 'kSpP',
iTunesESpKPurchasedMusic = 'kSpM',
iTunesESpKTVShows = 'kSpT'
};
typedef enum iTunesESpK iTunesESpK;

enum iTunesEVdK {
iTunesEVdKNone = 'kNon' ,
iTunesEVdKMovie = 'kVdM' ,
iTunesEVdKMusicVideo = 'kVdV' ,
iTunesEVdKTVShow = 'kVdT' 
};
typedef enum iTunesEVdK iTunesEVdK;

enum iTunesERtK {
iTunesERtKUser = 'kRtU' ,
iTunesERtKComputed = 'kRtC' 
};
typedef enum iTunesERtK iTunesERtK;







@interface iTunesPrintSettings : SBObject

@property (readonly) NSInteger copies; 
@property (readonly) BOOL collating; 
@property (readonly) NSInteger startingPage; 
@property (readonly) NSInteger endingPage; 
@property (readonly) NSInteger pagesAcross; 
@property (readonly) NSInteger pagesDown; 
@property (readonly) iTunesEnum errorHandling; 
@property (copy, readonly) NSDate *requestedPrintTime; 
@property (copy, readonly) NSArray *printerFeatures; 
@property (copy, readonly) NSString *faxNumber; 
@property (copy, readonly) NSString *targetPrinter; 

- (void) printPrintDialog:(BOOL)printDialog withProperties:(iTunesPrintSettings *)withProperties kind:(iTunesEKnd)kind theme:(NSString *)theme; 
- (void) close; 
- (void) delete; 
- (SBObject *) duplicateTo:(SBObject *)to; 
- (BOOL) exists; 
- (void) open; 
- (void) playOnce:(BOOL)once; 

@end








@interface iTunesApplication : SBApplication

- (SBElementArray *) browserWindows;
- (SBElementArray *) encoders;
- (SBElementArray *) EQPresets;
- (SBElementArray *) EQWindows;
- (SBElementArray *) playlistWindows;
- (SBElementArray *) sources;
- (SBElementArray *) visuals;
- (SBElementArray *) windows;

@property (copy) iTunesEncoder *currentEncoder; 
@property (copy) iTunesEQPreset *currentEQPreset; 
@property (copy, readonly) iTunesPlaylist *currentPlaylist; 
@property (copy, readonly) NSString *currentStreamTitle; 
@property (copy, readonly) NSString *currentStreamURL; 
@property (copy, readonly) iTunesTrack *currentTrack; 
@property (copy) iTunesVisual *currentVisual; 
@property BOOL EQEnabled; 
@property BOOL fixedIndexing; 
@property BOOL frontmost; 
@property BOOL fullScreen; 
@property (copy, readonly) NSString *name; 
@property BOOL mute; 
@property NSInteger playerPosition; 
@property (readonly) iTunesEPlS playerState; 
@property (copy, readonly) SBObject *selection; 
@property NSInteger soundVolume; 
@property (copy, readonly) NSString *version; 
@property BOOL visualsEnabled; 
@property iTunesEVSz visualSize; 

- (void) printPrintDialog:(BOOL)printDialog withProperties:(iTunesPrintSettings *)withProperties kind:(iTunesEKnd)kind theme:(NSString *)theme; 
- (void) run; 
- (void) quit; 
- (iTunesTrack *) add:(NSArray *)x to:(SBObject *)to; 
- (void) backTrack; 
- (iTunesTrack *) convert:(NSArray *)x; 
- (void) fastForward; 
- (void) nextTrack; 
- (void) pause; 
- (void) playOnce:(BOOL)once; 
- (void) playpause; 
- (void) previousTrack; 
- (void) resume; 
- (void) rewind; 
- (void) stop; 
- (void) update; 
- (void) eject; 
- (void) subscribe:(NSString *)x; 
- (void) updateAllPodcasts; 
- (void) updatePodcast; 
- (void) openLocation:(NSString *)x; 

@end


@interface iTunesItem : SBObject

@property (copy, readonly) SBObject *container; 
- (NSInteger) id; 
@property (readonly) NSInteger index; 
@property (copy) NSString *name; 
@property (copy, readonly) NSString *persistentID; 

- (void) printPrintDialog:(BOOL)printDialog withProperties:(iTunesPrintSettings *)withProperties kind:(iTunesEKnd)kind theme:(NSString *)theme; 
- (void) close; 
- (void) delete; 
- (SBObject *) duplicateTo:(SBObject *)to; 
- (BOOL) exists; 
- (void) open; 
- (void) playOnce:(BOOL)once; 
- (void) reveal; 

@end


@interface iTunesArtwork : iTunesItem

@property (copy) NSImage *data; 
@property (copy) NSString *objectDescription; 
@property (readonly) BOOL downloaded; 
@property (copy, readonly) NSNumber *format; 
@property NSInteger kind; 
@property (copy) NSData *rawData; 


@end


@interface iTunesEncoder : iTunesItem

@property (copy, readonly) NSString *format; 


@end


@interface iTunesEQPreset : iTunesItem

@property double band1; 
@property double band2; 
@property double band3; 
@property double band4; 
@property double band5; 
@property double band6; 
@property double band7; 
@property double band8; 
@property double band9; 
@property double band10; 
@property (readonly) BOOL modifiable; 
@property double preamp; 
@property BOOL updateTracks; 


@end


@interface iTunesPlaylist : iTunesItem

- (SBElementArray *) tracks;

@property (readonly) NSInteger duration; 
@property (copy) NSString *name; 
@property (copy, readonly) iTunesPlaylist *parent; 
@property BOOL shuffle; 
@property (readonly) long long size; 
@property iTunesERpt songRepeat; 
@property (readonly) iTunesESpK specialKind; 
@property (copy, readonly) NSString *time; 
@property (readonly) BOOL visible; 

- (void) moveTo:(SBObject *)to; 
- (iTunesTrack *) searchFor:(NSString *)for_ only:(iTunesESrA)only; 

@end


@interface iTunesAudioCDPlaylist : iTunesPlaylist

- (SBElementArray *) audioCDTracks;

@property (copy) NSString *artist; 
@property BOOL compilation; 
@property (copy) NSString *composer; 
@property NSInteger discCount; 
@property NSInteger discNumber; 
@property (copy) NSString *genre; 
@property NSInteger year; 


@end


@interface iTunesLibraryPlaylist : iTunesPlaylist

- (SBElementArray *) fileTracks;
- (SBElementArray *) URLTracks;
- (SBElementArray *) sharedTracks;


@end


@interface iTunesRadioTunerPlaylist : iTunesPlaylist

- (SBElementArray *) URLTracks;


@end


@interface iTunesSource : iTunesItem

- (SBElementArray *) audioCDPlaylists;
- (SBElementArray *) libraryPlaylists;
- (SBElementArray *) playlists;
- (SBElementArray *) radioTunerPlaylists;
- (SBElementArray *) userPlaylists;

@property (readonly) long long capacity; 
@property (readonly) long long freeSpace; 
@property (readonly) iTunesESrc kind;

- (void) update; 
- (void) eject; 

@end


@interface iTunesTrack : iTunesItem

- (SBElementArray *) artworks;

@property (copy) NSString *album; 
@property (copy) NSString *albumArtist; 
@property NSInteger albumRating; 
@property (readonly) iTunesERtK albumRatingKind; 
@property (copy) NSString *artist; 
@property (readonly) NSInteger bitRate; 
@property double bookmark; 
@property BOOL bookmarkable; 
@property NSInteger bpm; 
@property (copy) NSString *category; 
@property (copy) NSString *comment; 
@property BOOL compilation; 
@property (copy) NSString *composer; 
@property (readonly) NSInteger databaseID; 
@property (copy, readonly) NSDate *dateAdded; 
@property (copy) NSString *objectDescription; 
@property NSInteger discCount; 
@property NSInteger discNumber; 
@property (readonly) double duration; 
@property BOOL enabled; 
@property (copy) NSString *episodeID; 
@property NSInteger episodeNumber; 
@property (copy) NSString *EQ; 
@property double finish; 
@property BOOL gapless; 
@property (copy) NSString *genre; 
@property (copy) NSString *grouping; 
@property (copy, readonly) NSString *kind; 
@property (copy) NSString *longDescription;
@property (copy) NSString *lyrics; 
@property (copy, readonly) NSDate *modificationDate; 
@property NSInteger playedCount; 
@property (copy) NSDate *playedDate; 
@property (readonly) BOOL podcast; 
@property NSInteger rating; 
@property (readonly) iTunesERtK ratingKind; 
@property (copy, readonly) NSDate *releaseDate; 
@property (readonly) NSInteger sampleRate; 
@property NSInteger seasonNumber; 
@property BOOL shufflable; 
@property NSInteger skippedCount; 
@property (copy) NSDate *skippedDate; 
@property (copy) NSString *show; 
@property (copy) NSString *sortAlbum; 
@property (copy) NSString *sortArtist; 
@property (copy) NSString *sortAlbumArtist; 
@property (copy) NSString *sortName; 
@property (copy) NSString *sortComposer; 
@property (copy) NSString *sortShow; 
@property (readonly) NSInteger size; 
@property double start; 
@property (copy, readonly) NSString *time; 
@property NSInteger trackCount; 
@property NSInteger trackNumber; 
@property BOOL unplayed; 
@property iTunesEVdK videoKind; 
@property NSInteger volumeAdjustment; 
@property NSInteger year; 


@end


@interface iTunesAudioCDTrack : iTunesTrack

@property (copy, readonly) NSURL *location; 


@end


@interface iTunesFileTrack : iTunesTrack

@property (copy) NSURL *location; 

- (void) refresh; 

@end


@interface iTunesSharedTrack : iTunesTrack


@end


@interface iTunesURLTrack : iTunesTrack

@property (copy) NSString *address; 

- (void) download; 

@end


@interface iTunesUserPlaylist : iTunesPlaylist

- (SBElementArray *) fileTracks;
- (SBElementArray *) URLTracks;
- (SBElementArray *) sharedTracks;

@property BOOL shared; 
@property (readonly) BOOL smart; 


@end


@interface iTunesFolderPlaylist : iTunesUserPlaylist


@end


@interface iTunesVisual : iTunesItem


@end


@interface iTunesWindow : iTunesItem

@property NSRect bounds; 
@property (readonly) BOOL closeable; 
@property (readonly) BOOL collapseable; 
@property BOOL collapsed; 
@property NSPoint position; 
@property (readonly) BOOL resizable; 
@property BOOL visible; 
@property (readonly) BOOL zoomable; 
@property BOOL zoomed; 


@end


@interface iTunesBrowserWindow : iTunesWindow

@property BOOL minimized; 
@property (copy, readonly) SBObject *selection; 
@property (copy) iTunesPlaylist *view; 


@end


@interface iTunesEQWindow : iTunesWindow

@property BOOL minimized; 


@end


@interface iTunesPlaylistWindow : iTunesWindow

@property (copy, readonly) SBObject *selection; 
@property (copy, readonly) iTunesPlaylist *view; 


@end

