#import <Cocoa/Cocoa.h>
NS_ASSUME_NONNULL_BEGIN
@class VLCMediaLibraryMediaItem;
@interface VLCLibraryImageCache : NSObject
+ (nullable NSImage *)thumbnailForMediaItem:(VLCMediaLibraryMediaItem *)mediaItem;
@end
NS_ASSUME_NONNULL_END
