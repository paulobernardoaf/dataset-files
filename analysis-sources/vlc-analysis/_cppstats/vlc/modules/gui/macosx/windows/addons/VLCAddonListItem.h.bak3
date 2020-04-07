





















#import <Cocoa/Cocoa.h>

#import <vlc_common.h>
#import <vlc_addons.h>

@interface VLCAddonListItem : NSObject

- (id)initWithAddon:(addon_entry_t *)p_entry;

- (NSData *)uuid;

- (NSString *)name;
- (NSString *)author;
- (NSString *)version;
- (NSString *)description;

- (BOOL)isInstalled;
- (addon_type_t)type;

@end
