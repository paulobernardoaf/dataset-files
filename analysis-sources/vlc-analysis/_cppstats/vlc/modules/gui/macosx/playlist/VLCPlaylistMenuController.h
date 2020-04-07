#import <Cocoa/Cocoa.h>
NS_ASSUME_NONNULL_BEGIN
@interface VLCPlaylistMenuController : NSObject
@property (readwrite, weak) NSTableView *playlistTableView;
@property (readonly) NSMenu *playlistMenu;
@end
NS_ASSUME_NONNULL_END
