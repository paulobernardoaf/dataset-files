





















#import <Cocoa/Cocoa.h>
#import <vlc_playlist.h>

NS_ASSUME_NONNULL_BEGIN

@interface VLCPlaylistImageCache : NSObject

+ (nullable NSImage *)artworkForPlaylistItemWithURL:(NSURL *)artworkURL;

@end

NS_ASSUME_NONNULL_END
