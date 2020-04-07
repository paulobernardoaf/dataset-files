





















#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

@class VLCImageView;
@class VLCLibraryTracksDataSource;
@class VLCTrackingView;
@class VLCMediaLibraryAlbum;

@interface VLCLibraryAlbumTableCellView : NSTableCellView

+ (CGFloat)defaultHeight;
+ (CGFloat)heightForAlbum:(VLCMediaLibraryAlbum *)album;

@property (readwrite, assign) IBOutlet VLCTrackingView *trackingView;
@property (readwrite, assign) IBOutlet VLCImageView *representedImageView;
@property (readwrite, assign) IBOutlet NSTextField *albumNameTextField;
@property (readwrite, assign) IBOutlet NSTextField *summaryTextField;
@property (readwrite, assign) IBOutlet NSTextField *yearTextField;
@property (readwrite, assign) IBOutlet NSButton *playInstantlyButton;

@property (readwrite, assign, nonatomic) VLCMediaLibraryAlbum *representedAlbum;

- (IBAction)playInstantly:(id)sender;

@end

NS_ASSUME_NONNULL_END
