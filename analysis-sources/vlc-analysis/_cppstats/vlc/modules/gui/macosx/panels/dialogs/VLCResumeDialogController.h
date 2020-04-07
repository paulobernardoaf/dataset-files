#import <Cocoa/Cocoa.h>
@class VLCInputItem;
@class VLCWrappableTextField;
enum ResumeResult {
RESUME_RESTART,
RESUME_NOW,
RESUME_FAIL
};
typedef void(^CompletionBlock)(enum ResumeResult);
@interface VLCResumeDialogController : NSWindowController
@property (readwrite, strong) IBOutlet NSTextField *titleLabel;
@property (readwrite, strong) IBOutlet VLCWrappableTextField *descriptionLabel;
@property (readwrite, strong) IBOutlet NSButton *restartButton;
@property (readwrite, strong) IBOutlet NSButton *resumeButton;
@property (readwrite, strong) IBOutlet NSButton *alwaysResumeCheckbox;
- (IBAction)buttonClicked:(id)sender;
- (IBAction)resumeSettingChanged:(id)sender;
- (void)showWindowWithItem:(VLCInputItem *)inputItem withLastPosition:(NSInteger)pos completionBlock:(CompletionBlock)block;
- (void)cancel;
@end
