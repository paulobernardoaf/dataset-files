





















#import <Cocoa/Cocoa.h>

#import "menus/renderers/VLCRendererItem.h"
#import "menus/renderers/VLCRendererDiscovery.h"

@interface VLCRendererMenuController : NSObject <VLCRendererDiscoveryDelegate>

@property (readwrite, weak) NSMenu *rendererMenu;
@property (readwrite, weak) NSMenuItem *rendererNoneItem;

- (void)startRendererDiscoveries;
- (void)stopRendererDiscoveries;
- (void)selectRenderer:(NSMenuItem *)sender;

@end
