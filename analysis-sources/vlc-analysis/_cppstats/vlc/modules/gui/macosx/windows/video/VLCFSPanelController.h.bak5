
























#import <Cocoa/Cocoa.h>

@class VLCTimeField;
@class VLCSlider;
@class VLCDefaultValueSlider;

extern NSString *VLCFSPanelShouldBecomeActive;
extern NSString *VLCFSPanelShouldBecomeInactive;

@interface VLCFSPanelController : NSWindowController

@property (readwrite, weak) NSTimer *hideTimer;

@property IBOutlet NSView *controlsView;
@property IBOutlet NSButton *playPauseButton;
@property IBOutlet NSButton *forwardButton;
@property IBOutlet NSButton *backwardButton;
@property IBOutlet NSButton *nextButton;
@property IBOutlet NSButton *previousButton;
@property IBOutlet NSButton *fullscreenButton;
@property IBOutlet NSTextField *mediaTitle;
@property IBOutlet VLCTimeField *elapsedTime;
@property IBOutlet VLCTimeField *remainingOrTotalTime;
@property IBOutlet VLCSlider *timeSlider;
@property IBOutlet VLCDefaultValueSlider *volumeSlider;

@property (assign) IBOutlet NSLayoutConstraint *heightMaxConstraint;

- (IBAction)togglePlayPause:(id)sender;
- (IBAction)jumpForward:(id)sender;
- (IBAction)jumpBackward:(id)sender;
- (IBAction)gotoPrevious:(id)sender;
- (IBAction)gotoNext:(id)sender;
- (IBAction)toggleFullscreen:(id)sender;
- (IBAction)timeSliderUpdate:(id)sender;
- (IBAction)volumeSliderUpdate:(id)sender;

- (void)shouldBecomeInactive:(NSNotification *)aNotification;
- (void)shouldBecomeActive:(NSNotification *)aNotification;
- (void)fadeIn;
- (void)fadeOut;


- (NSRect)contrainFrameToAssociatedVoutWindow:(NSRect)frame;

@end
