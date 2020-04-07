#import <Cocoa/Cocoa.h>
NS_ASSUME_NONNULL_BEGIN
@interface NSColor (VLCAdditions)
+ (instancetype)VLClibraryHighlightColor;
+ (instancetype)VLClibraryLightTitleColor;
+ (instancetype)VLClibraryDarkTitleColor;
+ (instancetype)VLClibrarySubtitleColor;
+ (instancetype)VLClibraryAnnotationColor;
+ (instancetype)VLClibraryAnnotationBackgroundColor;
+ (instancetype)VLClibrarySeparatorLightColor;
+ (instancetype)VLClibrarySeparatorDarkColor;
+ (instancetype)VLClibraryProgressIndicatorBackgroundColor;
+ (instancetype)VLClibraryProgressIndicatorForegroundColor;
@end
NS_ASSUME_NONNULL_END
