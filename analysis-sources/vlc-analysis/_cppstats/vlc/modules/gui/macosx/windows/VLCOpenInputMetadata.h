#import <Foundation/Foundation.h>
@interface VLCOpenInputMetadata : NSObject
@property (readwrite, copy) NSString *MRLString;
@property (readwrite, copy) NSString *itemName;
@property (readwrite, copy) NSArray *playbackOptions;
@end
