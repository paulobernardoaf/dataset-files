#import <Cocoa/Cocoa.h>
@interface VLGetURLScriptCommand : NSScriptCommand
@end
@interface VLControlScriptCommand : NSScriptCommand
@end
@interface NSApplication(ScriptSupport)
@property (readwrite) BOOL scriptFullscreenMode;
@property (readwrite) float audioVolume;
@property (readwrite) long long audioDesync;
@property (readwrite) int currentTime;
@property (readwrite) float playbackRate;
@property (readonly) NSInteger durationOfCurrentItem;
@property (readonly) NSString *pathOfCurrentItem;
@property (readonly) NSString *nameOfCurrentItem;
@property (readonly) BOOL playbackShowsMenu;
@property (readonly) BOOL recordable;
@property (readwrite) BOOL recordingEnabled;
@property (readwrite) BOOL shuffledPlayback;
@property (readwrite) BOOL repeatOne;
@property (readwrite) BOOL repeatAll;
@end
