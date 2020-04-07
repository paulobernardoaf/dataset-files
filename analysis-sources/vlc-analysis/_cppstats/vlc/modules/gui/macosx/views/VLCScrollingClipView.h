#import <Cocoa/Cocoa.h>
@interface VLCScrollingClipView : NSClipView
- (void)startScrolling;
- (void)stopScrolling;
- (void)resetScrolling;
@property IBOutlet NSScrollView *parentScrollView;
@end
