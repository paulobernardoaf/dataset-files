#import <Foundation/Foundation.h>
#import <vlc_services_discovery.h>
@class VLCMediaSource;
NS_ASSUME_NONNULL_BEGIN
@interface VLCMediaSourceProvider : NSObject
+ (NSArray <VLCMediaSource *> *)listOfMediaSourcesForCategory:(enum services_discovery_category_e)category;
@end
NS_ASSUME_NONNULL_END
