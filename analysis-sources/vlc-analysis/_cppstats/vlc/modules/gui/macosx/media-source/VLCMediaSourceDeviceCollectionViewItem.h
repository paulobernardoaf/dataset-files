#import <Cocoa/Cocoa.h>
NS_ASSUME_NONNULL_BEGIN
@class VLCImageView;
extern NSString *VLCMediaSourceDeviceCellIdentifier;
@interface VLCMediaSourceDeviceCollectionViewItem : NSCollectionViewItem
@property (readwrite, assign) IBOutlet NSTextField *titleTextField;
@property (readwrite, assign) IBOutlet VLCImageView *mediaImageView;
@end
NS_ASSUME_NONNULL_END
