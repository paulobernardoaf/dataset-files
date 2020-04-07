





















#import <Foundation/Foundation.h>

#import <vlc_media_library.h>

@class VLCLibraryModel;
@class VLCMediaLibraryMediaItem;

NS_ASSUME_NONNULL_BEGIN

@interface VLCLibraryController : NSObject

@property (readonly, nullable) VLCLibraryModel *libraryModel;

- (int)appendItemToPlaylist:(VLCMediaLibraryMediaItem *)mediaItem playImmediately:(BOOL)playImmediately;
- (int)appendItemsToPlaylist:(NSArray <VLCMediaLibraryMediaItem *> *)mediaItemArray playFirstItemImmediately:(BOOL)playFirstItemImmediately;
- (void)showItemInFinder:(VLCMediaLibraryMediaItem *)mediaItem;

- (int)addFolderWithFileURL:(NSURL *)fileURL;
- (int)banFolderWithFileURL:(NSURL *)fileURL;
- (int)unbanFolderWithFileURL:(NSURL *)fileURL;
- (int)removeFolderWithFileURL:(NSURL *)fileURL;

- (int)clearHistory;






- (void)sortByCriteria:(enum vlc_ml_sorting_criteria_t)sortCriteria andDescending:(bool)descending;





@property (readonly) BOOL unsorted;




@property (readonly) enum vlc_ml_sorting_criteria_t lastSortingCriteria;




@property (readonly) bool descendingLibrarySorting;

@end

NS_ASSUME_NONNULL_END
