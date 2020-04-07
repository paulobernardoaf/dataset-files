#import <Cocoa/Cocoa.h>
NS_ASSUME_NONNULL_BEGIN
@class VLCImageView;
@class VLCTrackingView;
@class VLCMediaLibraryMediaItem;
@class VLCInputItem;
@interface VLCLibraryTableCellView : NSTableCellView
@property (readwrite, assign) IBOutlet VLCTrackingView *trackingView;
@property (readwrite, assign) IBOutlet NSTextField *singlePrimaryTitleTextField;
@property (readwrite, assign) IBOutlet NSTextField *secondaryTitleTextField;
@property (readwrite, assign) IBOutlet NSTextField *primaryTitleTextField;
@property (readwrite, assign) IBOutlet VLCImageView *representedImageView;
@property (readwrite, assign) IBOutlet NSButton *playInstantlyButton;
@property (readwrite, strong, nonatomic) VLCMediaLibraryMediaItem *representedMediaItem;
@property (readwrite, strong, nonatomic) VLCInputItem *representedInputItem;
@end
NS_ASSUME_NONNULL_END
