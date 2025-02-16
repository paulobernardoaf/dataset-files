






















#import <Cocoa/Cocoa.h>

#import "windows/video/VLCWindow.h"

extern NSString *VLCVideoWindowShouldShowFullscreenController;
extern NSString *VLCVideoWindowDidEnterFullscreen;
extern NSString *VLCWindowShouldShowController;
extern const CGFloat VLCVideoWindowCommonMinimalHeight;

@class VLCVoutView;
@class VLCControlsBarCommon;







@interface VLCVideoWindowCommon : VLCWindow <NSWindowDelegate, NSAnimationDelegate>

@property (weak) IBOutlet NSLayoutConstraint *videoViewBottomConstraint;

@property (nonatomic, strong) IBOutlet VLCVoutView* videoView;
@property (nonatomic, weak) IBOutlet VLCControlsBarCommon* controlsBar;
@property (readonly) BOOL inFullscreenTransition;
@property (readonly) BOOL windowShouldExitFullscreenWhenFinished;
@property (readwrite, assign) NSRect previousSavedFrame;
@property (nonatomic, readwrite, assign) NSSize nativeVideoSize;

- (void)setWindowLevel:(NSInteger)i_state;
- (void)resizeWindow;

- (NSRect)getWindowRectForProposedVideoViewSize:(NSSize)size;


- (void)enterFullscreenWithAnimation:(BOOL)b_animation;
- (void)leaveFullscreenWithAnimation:(BOOL)b_animation;


- (void)hideControlsBar;
- (void)showControlsBar;

- (void)windowWillEnterFullScreen:(NSNotification *)notification;
- (void)windowDidEnterFullScreen:(NSNotification *)notification;
- (void)windowWillExitFullScreen:(NSNotification *)notification;

@end
