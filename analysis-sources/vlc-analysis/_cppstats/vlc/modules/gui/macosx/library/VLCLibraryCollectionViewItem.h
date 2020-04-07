#import <Cocoa/Cocoa.h>
NS_ASSUME_NONNULL_BEGIN
extern NSString *VLCLibraryCellIdentifier;
@class VLCImageView;
@class VLCLinearProgressIndicator;
@class VLCMediaLibraryMediaItem;
@interface VLCLibraryCollectionViewItem : NSCollectionViewItem
@property (readwrite, assign) IBOutlet NSTextField *mediaTitleTextField;
@property (readwrite, assign) IBOutlet NSTextField *annotationTextField;
@property (readwrite, assign) IBOutlet NSTextField *unplayedIndicatorTextField;
@property (readwrite, assign) IBOutlet NSTextField *durationTextField;
@property (readwrite, assign) IBOutlet VLCImageView *mediaImageView;
@property (readwrite, assign) IBOutlet NSButton *playInstantlyButton;
@property (readwrite, assign) IBOutlet NSButton *addToPlaylistButton;
@property (readwrite, assign) IBOutlet VLCLinearProgressIndicator *progressIndicator;
@property (readwrite, retain, nonatomic) VLCMediaLibraryMediaItem *representedMediaItem;
- (IBAction)playInstantly:(id)sender;
- (IBAction)addToPlaylist:(id)sender;
@end
NS_ASSUME_NONNULL_END
