#import <Foundation/Foundation.h>
#include <vlc_common.h>
NS_ASSUME_NONNULL_BEGIN
@class VLCPlayerController;
@interface VLCRendererItem : NSObject
- (instancetype)initWithRendererItem:(vlc_renderer_item_t*)item;
@property (readonly, assign) vlc_renderer_item_t* rendererItem;
@property (readonly) NSString *name;
@property (readonly) NSString *identifier;
@property (readonly) NSString *type;
@property (readonly) NSString *userReadableType;
@property (readonly) NSString *iconURI;
@property (readonly) int capabilityFlags;
- (void)setRendererForPlayerController:(VLCPlayerController *)playerController;
@end
NS_ASSUME_NONNULL_END
