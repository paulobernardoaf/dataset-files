#import <Cocoa/Cocoa.h>
NS_ASSUME_NONNULL_BEGIN
@class VLCMediaLibraryMediaItem;
@interface VLCLibraryInformationPanel : NSWindowController
@property (readwrite, weak) IBOutlet NSTextField *multiLineTextLabel;
@property (readwrite, weak) IBOutlet NSImageView *imageView;
- (void)setRepresentedMediaItem:(VLCMediaLibraryMediaItem *)representedMediaItem;
@end
NS_ASSUME_NONNULL_END
