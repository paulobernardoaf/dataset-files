#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>
@interface VLCSliderCell : NSSliderCell
@property NSColor *gradientColor;
@property NSColor *gradientColor2;
@property NSColor *trackStrokeColor;
@property NSColor *filledTrackColor;
@property NSColor *knobFillColor;
@property NSColor *activeKnobFillColor;
@property NSColor *shadowColor;
@property NSColor *knobStrokeColor;
@property NSColor *highlightBackground;
@property NSGradient *trackGradient;
@property NSGradient *highlightGradient;
@property NSShadow *knobShadow;
@property NSInteger animationWidth;
@property (nonatomic, setter=setIndefinite:) BOOL indefinite;
@property (nonatomic, setter=setKnobHidden:) BOOL isKnobHidden;
- (void)setSliderStyleLight;
- (void)setSliderStyleDark;
@end
