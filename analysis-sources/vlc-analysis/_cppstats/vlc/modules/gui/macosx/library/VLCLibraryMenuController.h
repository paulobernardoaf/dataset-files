#import <Cocoa/Cocoa.h>
NS_ASSUME_NONNULL_BEGIN
@class VLCMediaLibraryMediaItem;
@interface VLCLibraryMenuController : NSObject
@property (readwrite, weak) VLCMediaLibraryMediaItem *representedMediaItem;
- (void)popupMenuWithEvent:(NSEvent *)theEvent forView:(NSView *)theView;
@end
NS_ASSUME_NONNULL_END
