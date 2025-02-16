#import <Cocoa/Cocoa.h>
@interface VLCAddonsWindowController : NSWindowController
{
IBOutlet NSPopUpButton *_typeSwitcher;
IBOutlet NSButton *_localAddonsOnlyCheckbox;
IBOutlet NSButton *_downloadCatalogButton;
IBOutlet NSTableView *_addonsTable;
IBOutlet NSProgressIndicator *_spinner;
IBOutlet NSTextField *_name;
IBOutlet NSTextField *_author;
IBOutlet NSTextField *_version;
IBOutlet NSTextView *_description;
IBOutlet NSButton *_installButton;
}
- (IBAction)switchType:(id)sender;
- (IBAction)toggleLocalCheckbox:(id)sender;
- (IBAction)downloadCatalog:(id)sender;
- (IBAction)installSelection:(id)sender;
- (IBAction)uninstallSelection:(id)sender;
@end
