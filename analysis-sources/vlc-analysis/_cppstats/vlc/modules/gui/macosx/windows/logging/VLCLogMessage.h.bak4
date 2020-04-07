





















#import <Foundation/Foundation.h>
#import <vlc_common.h>

@interface VLCLogMessage : NSObject

@property (readonly, getter=typeName) NSString *typeName;
@property (readonly) int type;
@property (readonly) NSString *message;
@property (readonly) NSString *component;
@property (readonly) NSString *function;
@property (readonly) NSString *location;
@property (readonly, getter=fullMessage) NSString *fullMessage;

+ (instancetype)logMessage:(char *)msg type:(int)type info:(const vlc_log_t *)info;
- (instancetype)initWithMessage:(char *)message type:(int)type info:(const vlc_log_t *)info;

@end
