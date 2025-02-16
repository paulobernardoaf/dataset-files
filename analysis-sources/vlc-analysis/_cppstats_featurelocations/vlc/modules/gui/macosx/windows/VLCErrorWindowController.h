






















#import <Cocoa/Cocoa.h>

@interface VLCErrorWindowController : NSWindowController

@property (readwrite, weak) IBOutlet NSButton *cleanupButton;
@property (readwrite, weak) IBOutlet NSTableView *errorTable;

- (IBAction)cleanupTable:(id)sender;

- (void)addError:(NSString *)title withMsg:(NSString *)message;

@end
