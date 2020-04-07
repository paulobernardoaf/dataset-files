#import <Cocoa/Cocoa.h>
@interface VLCSlider : NSSlider
@property (nonatomic, getter=getIndefinite,setter=setIndefinite:) BOOL indefinite;
@property (nonatomic, getter=getKnobHidden,setter=setKnobHidden:) BOOL isKnobHidden;
@property (readwrite) BOOL isScrollable;
- (void)setSliderStyleLight;
- (void)setSliderStyleDark;
@end
