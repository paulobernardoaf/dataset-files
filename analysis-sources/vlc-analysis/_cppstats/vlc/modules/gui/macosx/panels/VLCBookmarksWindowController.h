#import <Cocoa/Cocoa.h>
#import "main/VLCMain.h"
#import <vlc_common.h>
@interface VLCBookmarksWindowController : NSWindowController
@property (readwrite, weak) IBOutlet NSButton *addButton;
@property (readwrite, weak) IBOutlet NSButton *clearButton;
@property (readwrite, weak) IBOutlet NSButton *editButton;
@property (readwrite, weak) IBOutlet NSButton *removeButton;
@property (readwrite, weak) IBOutlet NSTableView *dataTable;
@property (readwrite) IBOutlet NSWindow *editBookmarksWindow;
@property (readwrite, weak) IBOutlet NSButton *editOKButton;
@property (readwrite, weak) IBOutlet NSButton *editCancelButton;
@property (readwrite, weak) IBOutlet NSTextField *editNameLabel;
@property (readwrite, weak) IBOutlet NSTextField *editTimeLabel;
@property (readwrite, weak) IBOutlet NSTextField *editNameTextField;
@property (readwrite, weak) IBOutlet NSTextField *editTimeTextField;
- (IBAction)toggleWindow:(id)sender;
- (IBAction)add:(id)sender;
- (IBAction)clear:(id)sender;
- (IBAction)edit:(id)sender;
- (IBAction)remove:(id)sender;
- (IBAction)goToBookmark:(id)sender;
- (IBAction)edit_cancel:(id)sender;
- (IBAction)edit_ok:(id)sender;
@end
