























#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN





@interface VLCDropDisabledImageView : NSImageView

@end

@protocol VLCDragDropTarget
@required
- (BOOL)handlePasteBoardFromDragSession:(NSPasteboard *)aPasteboard;
@end

@interface VLCDragDropView : NSView

@property (nonatomic, assign) id<VLCDragDropTarget> dropTarget;
@property (nonatomic, assign) BOOL drawBorder;

- (void)enablePlaylistItems;

@end

NS_ASSUME_NONNULL_END
