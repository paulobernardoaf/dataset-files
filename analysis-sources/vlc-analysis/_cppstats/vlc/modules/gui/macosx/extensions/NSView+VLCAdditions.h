#import <Cocoa/Cocoa.h>
NS_ASSUME_NONNULL_BEGIN
@interface NSView (VLCAdditions)
@property (readonly) BOOL shouldShowDarkAppearance;
- (void)enableSubviews:(BOOL)enabled;
@end
NS_ASSUME_NONNULL_END
