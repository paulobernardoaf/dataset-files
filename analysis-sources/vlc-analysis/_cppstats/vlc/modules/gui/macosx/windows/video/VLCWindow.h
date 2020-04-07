#import <Cocoa/Cocoa.h>
@class VLCVoutView;
@interface VLCWindow : NSWindow <NSAnimationDelegate>
@property (readwrite) BOOL canBecomeKeyWindow;
@property (readwrite) BOOL canBecomeMainWindow;
@property (nonatomic, readwrite) BOOL hasActiveVideo;
@property (nonatomic, readwrite) BOOL fullscreen;
- (void)closeAndAnimate:(BOOL)animate;
- (void)orderFront:(id)sender animate:(BOOL)animate;
- (void)orderOut:(id)sender animate:(BOOL)animate;
- (VLCVoutView *)videoView;
@end
