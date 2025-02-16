





















#import <Cocoa/Cocoa.h>

@class VLCPlaylistController;
@class VLCDragDropView;

NS_ASSUME_NONNULL_BEGIN

@interface VLCPlaylistDataSource : NSObject <NSTableViewDataSource, NSTableViewDelegate>

@property (readwrite, assign, nonatomic) VLCPlaylistController *playlistController;
@property (readwrite, assign) NSTableView *tableView;
@property (readwrite, assign) VLCDragDropView *dragDropView;
@property (readwrite, assign) NSTextField *counterTextField;

- (void)prepareForUse;
- (void)playlistUpdated;

@end

NS_ASSUME_NONNULL_END
