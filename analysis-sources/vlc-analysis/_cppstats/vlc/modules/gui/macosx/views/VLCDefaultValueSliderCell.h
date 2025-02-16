#import <Cocoa/Cocoa.h>
@interface VLCDefaultValueSliderCell : NSSliderCell
@property (readwrite) BOOL drawTickMarkForDefault;
@property (readwrite) BOOL snapsToDefault;
@property (getter=defaultValue, setter=setDefaultValue:) double defaultValue;
@property (getter=defaultTickMarkColor, setter=setDefaultTickMarkColor:) NSColor *defaultTickMarkColor;
- (void)drawDefaultTickMarkWithFrame:(NSRect)rect;
@end
