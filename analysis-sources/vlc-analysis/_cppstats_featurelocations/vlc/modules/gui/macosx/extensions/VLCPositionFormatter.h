





















#import <Foundation/Foundation.h>

@interface VLCPositionFormatter : NSFormatter

- (NSString*)stringForObjectValue:(id)obj;

- (BOOL)getObjectValue:(id*)obj
forString:(NSString*)string
errorDescription:(NSString**)error;

- (BOOL)isPartialStringValid:(NSString*)partialString
newEditingString:(NSString**)newString
errorDescription:(NSString**)error;

@end
