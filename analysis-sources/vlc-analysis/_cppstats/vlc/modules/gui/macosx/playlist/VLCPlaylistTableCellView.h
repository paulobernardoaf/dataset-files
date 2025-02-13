#import <Cocoa/Cocoa.h>
@class VLCImageView;
@class VLCPlaylistItem;
NS_ASSUME_NONNULL_BEGIN
@interface VLCPlaylistTableCellView : NSTableCellView
@property (readwrite, assign, nonatomic) VLCPlaylistItem *representedPlaylistItem;
@property (readwrite, nonatomic) BOOL representsCurrentPlaylistItem;
@property (readwrite, assign) IBOutlet NSTextField *artistTextField;
@property (readwrite, assign) IBOutlet NSTextField *secondaryMediaTitleTextField;
@property (readwrite, assign) IBOutlet NSTextField *mediaTitleTextField;
@property (readwrite, assign) IBOutlet NSTextField *durationTextField;
@property (readwrite, assign) IBOutlet VLCImageView *mediaImageView;
@property (readwrite, assign) IBOutlet NSImageView *audioMediaTypeIndicator;
@property (readwrite, assign) IBOutlet VLCImageView *audioArtworkImageView;
@end
NS_ASSUME_NONNULL_END
