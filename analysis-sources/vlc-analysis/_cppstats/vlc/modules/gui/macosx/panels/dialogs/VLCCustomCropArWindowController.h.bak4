




















#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

@interface VLCCustomCropArWindowController : NSWindowController

@property (readwrite, weak) IBOutlet NSButton *cancelButton;
@property (readwrite, weak) IBOutlet NSButton *applyButton;
@property (readwrite, weak) IBOutlet NSTextField *titleLabel;
@property (readwrite, weak) IBOutlet NSTextField *numeratorTextField;
@property (readwrite, weak) IBOutlet NSTextField *denominatorTextField;
@property (readwrite, retain) NSString *title;

- (IBAction)buttonPressed:(id)sender;





typedef void(^CustomCropArCompletionHandler)(NSInteger returnCode, NSString *geometry);






- (void)runModalForWindow:(NSWindow *)window completionHandler:(CustomCropArCompletionHandler)handler;

@end

NS_ASSUME_NONNULL_END
