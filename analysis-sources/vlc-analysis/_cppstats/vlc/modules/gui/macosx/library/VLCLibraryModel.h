#import <Cocoa/Cocoa.h>
#import <vlc_media_library.h>
NS_ASSUME_NONNULL_BEGIN
@class VLCMediaLibraryMediaItem;
@class VLCMediaLibraryArtist;
@class VLCMediaLibraryAlbum;
@class VLCMediaLibraryGenre;
@class VLCMediaLibraryEntryPoint;
extern NSString *VLCLibraryModelAudioMediaListUpdated;
extern NSString *VLCLibraryModelArtistListUpdated;
extern NSString *VLCLibraryModelAlbumListUpdated;
extern NSString *VLCLibraryModelVideoMediaListUpdated;
extern NSString *VLCLibraryModelRecentMediaListUpdated;
extern NSString *VLCLibraryModelMediaItemUpdated;
@interface VLCLibraryModel : NSObject
+ (NSArray *)availableAudioCollections;
- (instancetype)initWithLibrary:(vlc_medialibrary_t *)library;
@property (readonly) size_t numberOfAudioMedia;
@property (readonly) NSArray <VLCMediaLibraryMediaItem *> *listOfAudioMedia;
@property (readonly) size_t numberOfArtists;
@property (readonly) NSArray <VLCMediaLibraryArtist *> *listOfArtists;
@property (readonly) size_t numberOfAlbums;
@property (readonly) NSArray <VLCMediaLibraryAlbum *> *listOfAlbums;
@property (readonly) size_t numberOfGenres;
@property (readonly) NSArray <VLCMediaLibraryGenre *> *listOfGenres;
@property (readonly) size_t numberOfVideoMedia;
@property (readonly) NSArray <VLCMediaLibraryMediaItem *> *listOfVideoMedia;
@property (readonly) size_t numberOfRecentMedia;
@property (readonly) NSArray <VLCMediaLibraryMediaItem *> *listOfRecentMedia;
@property (readonly) NSArray <VLCMediaLibraryEntryPoint *> *listOfMonitoredFolders;
- (nullable NSArray <VLCMediaLibraryAlbum *>*)listAlbumsOfParentType:(enum vlc_ml_parent_type)parentType forID:(int64_t)ID;
- (void)sortByCriteria:(enum vlc_ml_sorting_criteria_t)sortCriteria andDescending:(bool)descending;
@end
NS_ASSUME_NONNULL_END
