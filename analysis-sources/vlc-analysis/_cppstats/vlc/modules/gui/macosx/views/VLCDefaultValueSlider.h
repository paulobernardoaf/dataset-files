#import <Cocoa/Cocoa.h>
@interface VLCDefaultValueSlider : NSSlider
@property (readwrite) BOOL isScrollable;
- (void)setDefaultValue:(double)value;
- (double)defaultValue;
@end
