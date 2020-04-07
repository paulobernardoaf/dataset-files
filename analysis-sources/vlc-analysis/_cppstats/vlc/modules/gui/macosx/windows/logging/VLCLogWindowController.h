#import <Cocoa/Cocoa.h>
@interface VLCLogWindowController : NSWindowController
@property (assign) IBOutlet NSTableView *messageTable;
@property (assign) IBOutlet NSButton *saveButton;
@property (assign) IBOutlet NSButton *clearButton;
@property (assign) IBOutlet NSButton *refreshButton;
@property (assign) IBOutlet NSButton *toggleDetailsButton;
@property (assign) IBOutlet NSSplitView *splitView;
@property (assign) IBOutlet NSView *detailView;
@property (assign) IBOutlet NSArrayController *arrayController;
- (IBAction)saveDebugLog:(id)sender;
- (IBAction)refreshLog:(id)sender;
- (IBAction)clearLog:(id)sender;
@end
