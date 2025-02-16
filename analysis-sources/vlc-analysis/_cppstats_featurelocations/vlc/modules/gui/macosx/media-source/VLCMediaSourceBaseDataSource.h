





















#import <Cocoa/Cocoa.h>

typedef NS_ENUM(NSInteger, VLCMediaSourceMode) {
VLCMediaSourceModeLAN,
VLCMediaSourceModeInternet,
};

NS_ASSUME_NONNULL_BEGIN

@interface VLCMediaSourceBaseDataSource : NSObject

@property (readwrite) NSCollectionView *collectionView;
@property (readwrite) NSScrollView *collectionViewScrollView;
@property (readwrite) NSTableView *tableView;
@property (readwrite) NSSegmentedControl *gridVsListSegmentedControl;
@property (readwrite) NSButton *homeButton;
@property (readwrite) NSPathControl *pathControl;
@property (readwrite, nonatomic) VLCMediaSourceMode mediaSourceMode;

- (void)setupViews;
- (void)reloadViews;

@end

NS_ASSUME_NONNULL_END
