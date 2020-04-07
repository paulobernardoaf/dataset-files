





















#import <Cocoa/Cocoa.h>

@interface VLCPopupPanelController : NSWindowController

@property (weak) IBOutlet NSTextField *titleLabel;
@property (weak) IBOutlet NSTextField *subtitleLabel;
@property (weak) IBOutlet NSPopUpButton *popupButton;
@property (weak) IBOutlet NSButton *cancelButton;
@property (weak) IBOutlet NSButton *okButton;

@property (readwrite) NSString *titleString;
@property (readwrite) NSString *subTitleString;
@property (readwrite) NSString *okButtonString;
@property (readwrite) NSString *cancelButtonString;
@property (readwrite) NSArray *popupButtonContent;






typedef void(^PopupPanelCompletionBlock)(NSInteger returnCode, NSInteger selectedIndex);






- (void)runModalForWindow:(NSWindow *)window completionHandler:(PopupPanelCompletionBlock)handler;

- (IBAction)windowElementAction:(id)sender;

@end
