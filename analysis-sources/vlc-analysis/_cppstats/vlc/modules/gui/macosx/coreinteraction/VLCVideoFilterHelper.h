#import <Foundation/Foundation.h>
#import <vlc_common.h>
@interface VLCVideoFilterHelper : NSObject
+ (void)setVideoFilter: (const char *)psz_name on:(BOOL)b_on;
+ (void)setVideoFilterProperty: (const char *)psz_property forFilter: (const char *)psz_filter withValue: (vlc_value_t)value;
@end
