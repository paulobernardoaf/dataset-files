#import <Cocoa/Cocoa.h>
NS_ASSUME_NONNULL_BEGIN
@interface NSFont (VLCAdditions)
+ (instancetype)VLClibrarySectionHeaderFont;
+ (instancetype)VLClibraryHighlightCellTitleFont;
+ (instancetype)VLClibraryHighlightCellSubtitleFont;
+ (instancetype)VLClibraryHighlightCellHighlightLabelFont;
+ (instancetype)VLClibraryLargeCellTitleFont;
+ (instancetype)VLClibraryLargeCellSubtitleFont;
+ (instancetype)VLClibrarySmallCellTitleFont;
+ (instancetype)VLClibrarySmallCellSubtitleFont;
+ (instancetype)VLClibraryCellAnnotationFont;
+ (instancetype)VLClibraryButtonFont;
+ (instancetype)VLCplaylistLabelFont;
+ (instancetype)VLCplaylistSelectedItemLabelFont;
+ (instancetype)VLCsmallPlaylistLabelFont;
+ (instancetype)VLCsmallPlaylistSelectedItemLabelFont;
@end
NS_ASSUME_NONNULL_END
