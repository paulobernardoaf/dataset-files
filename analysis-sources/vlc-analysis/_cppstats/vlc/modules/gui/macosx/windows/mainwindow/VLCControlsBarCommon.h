#import <Cocoa/Cocoa.h>
@class VLCDragDropView;
@class VLCSlider;
@class VLCTimeField;
@class VLCBottomBarView;
@interface VLCControlsBarCommon : NSObject
@property (readwrite, strong) IBOutlet VLCDragDropView *dropView;
@property (readwrite, strong) IBOutlet NSButton *playButton;
@property (readwrite, strong) IBOutlet NSButton *backwardButton;
@property (readwrite, strong) IBOutlet NSButton *forwardButton;
@property (readwrite, strong) IBOutlet VLCSlider *timeSlider;
@property (readwrite, strong) IBOutlet VLCTimeField *timeField;
@property (readwrite, strong) IBOutlet NSButton *fullscreenButton;
@property (readwrite, strong) IBOutlet NSLayoutConstraint *fullscreenButtonWidthConstraint;
@property (readwrite, strong) IBOutlet VLCBottomBarView *bottomBarView;
@property (readonly) BOOL nativeFullscreenMode;
- (CGFloat)height;
- (IBAction)play:(id)sender;
- (IBAction)bwd:(id)sender;
- (IBAction)fwd:(id)sender;
- (IBAction)timeSliderAction:(id)sender;
- (IBAction)fullscreen:(id)sender;
@end
