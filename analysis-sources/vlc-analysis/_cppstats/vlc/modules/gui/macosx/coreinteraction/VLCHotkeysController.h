#import <Cocoa/Cocoa.h>
#import <vlc_vout.h>
NS_ASSUME_NONNULL_BEGIN
@interface VLCHotkeysController : NSObject
- (BOOL)handleVideoOutputKeyDown:(id)anEvent forVideoOutput:(vout_thread_t *)p_vout;
- (BOOL)performKeyEquivalent:(NSEvent *)anEvent;
@end
NS_ASSUME_NONNULL_END
