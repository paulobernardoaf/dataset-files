#import <Cocoa/Cocoa.h>
extern NSString *VLCTimeFieldDisplayTimeAsRemaining;
@interface VLCTimeField : NSTextField
@property (readonly) BOOL timeRemaining;
- (void)setRemainingIdentifier:(NSString *)o_string;
@end
