






















#import <Cocoa/Cocoa.h>





@class VLCErrorWindowController;

@interface VLCCoreDialogProvider : NSObject


@property (readwrite, strong) IBOutlet NSWindow *authenticationWindow;

@property (readwrite, weak) IBOutlet NSButton *authenticationCancelButton;
@property (readwrite, weak) IBOutlet NSTextField *authenticationDescriptionLabel;
@property (readwrite, weak) IBOutlet NSTextField *authenticationLoginTextField;
@property (readwrite, weak) IBOutlet NSTextField *authenticationLoginLabel;
@property (readwrite, weak) IBOutlet NSButton *authenticationOkButton;
@property (readwrite, weak) IBOutlet NSTextField *authenticationPasswordTextField;
@property (readwrite, weak) IBOutlet NSTextField *authenticationPasswordLabel;
@property (readwrite, weak) IBOutlet NSTextField *authenticationTitleLabel;
@property (readwrite, weak) IBOutlet NSButton *authenticationStorePasswordCheckbox;


@property (readwrite, strong) IBOutlet NSWindow *progressWindow;

@property (readwrite, weak) IBOutlet NSProgressIndicator *progressIndicator;
@property (readwrite, weak) IBOutlet NSButton *progressCancelButton;
@property (readwrite, weak) IBOutlet NSTextField *progressDescriptionLabel;
@property (readwrite, weak) IBOutlet NSTextField *progressTitleLabel;

@property (readonly) VLCErrorWindowController* errorPanel;

- (IBAction)authenticationDialogAction:(id)sender;

- (IBAction)progressDialogAction:(id)sender;

@end
