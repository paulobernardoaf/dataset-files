





















#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

@class VLCInputNode;
@class VLCMediaSource;

@interface VLCMediaSourceDataSource : NSObject <NSCollectionViewDataSource, NSCollectionViewDelegate, NSTableViewDelegate, NSTableViewDataSource>

@property (readwrite, retain) VLCMediaSource *displayedMediaSource;
@property (readwrite, retain, nonatomic) VLCInputNode *nodeToDisplay;
@property (readwrite, assign) NSCollectionView *collectionView;
@property (readwrite, assign) NSTableView *tableView;
@property (readwrite) NSPathControl *pathControl;
@property (readwrite) BOOL gridViewMode;

- (void)setupViews;

@end

NS_ASSUME_NONNULL_END
