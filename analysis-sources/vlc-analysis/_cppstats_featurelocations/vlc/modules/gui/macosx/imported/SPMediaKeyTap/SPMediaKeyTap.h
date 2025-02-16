






















#import <Cocoa/Cocoa.h>
#import <IOKit/hidsystem/ev_keymap.h>



#define SPSystemDefinedEventMediaKeys 8

@interface SPMediaKeyTap : NSObject

- (id)initWithDelegate:(id)delegate;

+ (BOOL)usesGlobalMediaKeyTap;
- (BOOL)startWatchingMediaKeys;
- (void)stopWatchingMediaKeys;
- (void)handleAndReleaseMediaKeyEvent:(NSEvent *)event;
@end

@interface NSObject (SPMediaKeyTapDelegate)
- (void)mediaKeyTap:(SPMediaKeyTap*)keyTap receivedMediaKeyEvent:(NSEvent*)event;
@end

#if defined(__cplusplus)
extern "C" {
#endif

extern NSString *kIgnoreMediaKeysDefaultsKey;

#if defined(__cplusplus)
}
#endif
