





















#import <Foundation/Foundation.h>
#import <vlc_playlist.h>

NS_ASSUME_NONNULL_BEGIN

@class VLCPlaylistModel;
@class VLCPlaylistDataSource;
@class VLCPlayerController;
@class VLCPlaylistExportModuleDescription;
@class VLCOpenInputMetadata;
@class VLCInputItem;

extern NSString *VLCPlaybackOrderChanged;
extern NSString *VLCPlaybackRepeatChanged;
extern NSString *VLCPlaybackHasPreviousChanged;
extern NSString *VLCPlaybackHasNextChanged;
extern NSString *VLCPlaylistCurrentItemChanged;
extern NSString *VLCPlaylistItemsAdded;
extern NSString *VLCPlaylistItemsRemoved;

@interface VLCPlaylistController : NSObject

- (instancetype)initWithPlaylist:(vlc_playlist_t *)playlist;





@property (readonly) vlc_playlist_t *p_playlist;





@property (readonly) VLCPlaylistModel *playlistModel;




@property (readwrite, assign) VLCPlaylistDataSource *playlistDataSource;




@property (readonly) VLCPlayerController *playerController;






@property (readonly) size_t currentPlaylistIndex;







@property (readonly, nullable) VLCInputItem *currentlyPlayingInputItem;





@property (readonly) BOOL hasPreviousPlaylistItem;





@property (readonly) BOOL hasNextPlaylistItem;





@property (readwrite, nonatomic) enum vlc_playlist_playback_repeat playbackRepeat;





@property (readwrite, nonatomic) enum vlc_playlist_playback_order playbackOrder;





- (void)addPlaylistItems:(NSArray <VLCOpenInputMetadata *> *)array;







- (void)addPlaylistItems:(NSArray <VLCOpenInputMetadata *> *)itemArray
atPosition:(size_t)insertionIndex
startPlayback:(BOOL)startPlayback;








- (int)addInputItem:(input_item_t *)p_inputItem
atPosition:(size_t)insertionIndex
startPlayback:(BOOL)startPlayback;







- (int)moveItemWithID:(int64_t)uniqueID toPosition:(size_t)target;





- (void)removeItemsAtIndexes:(NSIndexSet *)indexes;




- (void)clearPlaylist;







- (int)sortByKey:(enum vlc_playlist_sort_key)sortKey andOrder:(enum vlc_playlist_sort_order)sortOrder;





@property (readonly) BOOL unsorted;




@property (readonly) enum vlc_playlist_sort_key lastSortKey;




@property (readonly) enum vlc_playlist_sort_order lastSortOrder;





- (int)startPlaylist;





- (int)playPreviousItem;






- (int)playItemAtIndex:(size_t)index;





- (int)playNextItem;




- (void)stopPlayback;




- (void)pausePlayback;




- (void)resumePlayback;




@property (readonly) NSArray <VLCPlaylistExportModuleDescription *> *availablePlaylistExportModules;







- (int)exportPlaylistToPath:(NSString *)path exportModule:(VLCPlaylistExportModuleDescription *)exportModule;

@end

@interface VLCPlaylistExportModuleDescription : NSObject

@property (readwrite, retain) NSString *humanReadableName;
@property (readwrite, retain) NSString *fileExtension;
@property (readwrite, retain) NSString *moduleName;

@end

NS_ASSUME_NONNULL_END
