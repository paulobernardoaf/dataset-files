#import <Cocoa/Cocoa.h>
@interface VLCMinimalVoutWindow : NSWindow
{
NSRect initialFrame;
}
- (id)initWithContentRect:(NSRect)contentRect;
- (void)enterFullscreen;
- (void)leaveFullscreen;
@end
