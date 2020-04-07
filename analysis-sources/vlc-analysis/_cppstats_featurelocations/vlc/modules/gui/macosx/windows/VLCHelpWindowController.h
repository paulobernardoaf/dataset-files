






















#import <WebKit/WebKit.h>

@interface VLCHelpWindowController : NSWindowController
{
IBOutlet WebView *helpWebView;
IBOutlet id backButton;
IBOutlet id forwardButton;
IBOutlet id homeButton;
}

- (IBAction)helpGoHome:(id)sender;
- (void)showHelp;

@end
