#import <Foundation/Foundation.h>
@interface VLCKeyboardBacklightControl : NSObject
- (void)lightsUp;
- (void)lightsDown;
- (void)switchLightsAsync:(BOOL)on;
- (void)switchLightsInstantly:(BOOL)on;
@end
