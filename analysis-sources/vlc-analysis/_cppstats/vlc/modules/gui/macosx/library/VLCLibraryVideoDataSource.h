#import <Cocoa/Cocoa.h>
NS_ASSUME_NONNULL_BEGIN
@class VLCLibraryModel;
@interface VLCLibraryVideoDataSource : NSObject <NSCollectionViewDataSource, NSCollectionViewDelegate>
@property (readwrite, assign) VLCLibraryModel *libraryModel;
@property (readwrite, assign) NSCollectionView *recentMediaCollectionView;
@property (readwrite, assign) NSCollectionView *libraryMediaCollectionView;
@end
NS_ASSUME_NONNULL_END
