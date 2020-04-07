






















#import <Cocoa/Cocoa.h>

@interface VLCTrackSynchronizationWindowController : NSWindowController


@property (readwrite, weak) IBOutlet NSButton *resetButton;


@property (readwrite, weak) IBOutlet NSTextField *avLabel;
@property (readwrite, weak) IBOutlet NSTextField *av_advanceLabel;
@property (readwrite, weak) IBOutlet NSTextField *av_advanceTextField;
@property (readwrite, weak) IBOutlet NSStepper *avStepper;


@property (readwrite, weak) IBOutlet NSTextField *svLabel;
@property (readwrite, weak) IBOutlet NSTextField *sv_advanceLabel;
@property (readwrite, weak) IBOutlet NSTextField *sv_advanceTextField;
@property (readwrite, weak) IBOutlet NSStepper *sv_advanceStepper;
@property (readwrite, weak) IBOutlet NSTextField *sv_speedLabel;
@property (readwrite, weak) IBOutlet NSTextField *sv_speedTextField;
@property (readwrite, weak) IBOutlet NSStepper *sv_speedStepper;
@property (readwrite, weak) IBOutlet NSTextField *sv_durLabel;
@property (readwrite, weak) IBOutlet NSTextField *sv_durTextField;
@property (readwrite, weak) IBOutlet NSStepper *sv_durStepper;

- (IBAction)toggleWindow:(id)sender;
- (IBAction)resetValues:(id)sender;

- (IBAction)avValueChanged:(id)sender;

- (IBAction)svAdvanceValueChanged:(id)sender;
- (IBAction)svSpeedValueChanged:(id)sender;
- (IBAction)svDurationValueChanged:(id)sender;
@end
