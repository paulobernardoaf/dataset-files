#import <Cocoa/Cocoa.h>
#import <vlc_common.h>
#import <vlc_dialog.h>
#import <vlc_extensions.h>
@class VLCDialogWindow;
@interface VLCExtensionsDialogProvider : NSObject <NSWindowDelegate>
- (void)performEventWithObject: (NSValue *)o_value ofType:(const char*)type;
- (void)triggerClick:(id)sender;
- (void)syncTextField:(NSNotification *)notifcation;
- (void)tableViewSelectionDidChange:(NSNotification *)notifcation;
- (void)popUpSelectionChanged:(id)sender;
- (NSSize)windowWillResize:(NSWindow *)sender toSize:(NSSize)frameSize;
- (BOOL)windowShouldClose:(id)sender;
- (void)updateWidgets:(extension_dialog_t *)dialog;
- (VLCDialogWindow *)createExtensionDialog:(extension_dialog_t *)p_dialog;
- (int)destroyExtensionDialog:(extension_dialog_t *)o_value;
- (VLCDialogWindow *)updateExtensionDialog:(NSValue *)o_value;
- (void)manageDialog:(extension_dialog_t *)p_dialog;
@end
