#import <Foundation/Foundation.h>
#import <vlc_playlist.h>
NS_ASSUME_NONNULL_BEGIN
@class VLCInputItem;
@class VLCMediaLibraryMediaItem;
extern NSString *VLCPlaylistItemPasteboardType;
@interface VLCPlaylistItem : NSObject
@property (readonly) vlc_playlist_item_t *playlistItem;
@property (readonly) uint64_t uniqueID;
@property (readwrite, retain) NSString *title;
@property (readonly, copy, nullable) NSURL *url;
@property (readonly, copy, nullable) NSString *path;
@property (readwrite, assign) vlc_tick_t duration;
@property (readonly, nullable) VLCInputItem *inputItem;
@property (readonly, nullable) VLCMediaLibraryMediaItem *mediaLibraryItem;
@property (readwrite, retain, nullable) NSString *artistName;
@property (readwrite, retain, nullable) NSString *albumName;
@property (readonly, copy) NSImage *artworkImage;
- (instancetype)initWithPlaylistItem:(vlc_playlist_item_t *)p_item;
- (void)updateRepresentation;
@end
NS_ASSUME_NONNULL_END
