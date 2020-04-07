#import <Cocoa/Cocoa.h>
@interface VLCTextfieldPanelController : NSWindowController
@property (weak) IBOutlet NSTextField *titleLabel;
@property (weak) IBOutlet NSTextField *subtitleLabel;
@property (weak) IBOutlet NSTextField *textField;
@property (weak) IBOutlet NSButton *cancelButton;
@property (weak) IBOutlet NSButton *okButton;
@property (readwrite) NSString *titleString;
@property (readwrite) NSString *subTitleString;
@property (readwrite) NSString *okButtonString;
@property (readwrite) NSString *cancelButtonString;
typedef void(^TextfieldPanelCompletionBlock)(NSInteger returnCode, NSString *resultingText);
- (void)runModalForWindow:(NSWindow *)window completionHandler:(TextfieldPanelCompletionBlock)handler;
- (IBAction)windowElementAction:(id)sender;
@end
