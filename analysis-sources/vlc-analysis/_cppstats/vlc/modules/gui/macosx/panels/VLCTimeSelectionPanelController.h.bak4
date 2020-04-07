




















#import <Cocoa/Cocoa.h>

@interface VLCTimeSelectionPanelController : NSWindowController

@property (readwrite, weak) IBOutlet NSButton *cancelButton;
@property (readwrite, weak) IBOutlet NSTextField *goToLabel;
@property (readwrite, weak) IBOutlet NSButton *okButton;
@property (readwrite, weak) IBOutlet NSTextField *secsLabel;
@property (readwrite, weak) IBOutlet NSTextField *minsLabel;
@property (readwrite, weak) IBOutlet NSTextField *hoursLabel;

@property (nonatomic) int jumpSecsValue;
@property (nonatomic) int jumpMinsValue;
@property (nonatomic) int jumpHoursValue;
@property (nonatomic) int secsMax;
@property (nonatomic) int minsMax;
@property (nonatomic) int hoursMax;

- (IBAction)buttonPressed:(id)sender;

- (int)getTimeInSecs;
- (void)setMaxTime:(int)secsMax;
- (void)setPosition:(int)secsPos;






typedef void(^TimeSelectionCompletionHandler)(NSInteger returnCode, int64_t returnTime);






- (void)runModalForWindow:(NSWindow *)window completionHandler:(TimeSelectionCompletionHandler)handler;

@end
