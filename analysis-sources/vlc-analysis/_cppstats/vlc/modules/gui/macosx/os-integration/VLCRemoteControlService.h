#import <Cocoa/Cocoa.h>
@interface VLCRemoteControlService : NSObject
- (void)subscribeToRemoteCommands;
- (void)unsubscribeFromRemoteCommands;
@end
