





















#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

@class VLCLibraryModel;
@class VLCLibraryGroupDataSource;
@class VLCMediaLibraryAlbum;

@interface VLCLibraryAudioDataSource : NSObject <NSTableViewDataSource, NSTableViewDelegate>

@property (readwrite, assign) VLCLibraryModel *libraryModel;
@property (readwrite, assign) VLCLibraryGroupDataSource *groupDataSource;
@property (readwrite, assign) NSSegmentedControl *segmentedControl;
@property (readwrite, assign) NSTableView *collectionSelectionTableView;
@property (readwrite, assign) NSTableView *groupSelectionTableView;
@property (readwrite, assign) NSCollectionView *collectionView;

- (void)setupAppearance;
- (void)reloadAppearance;

@end

@interface VLCLibraryGroupDataSource : NSObject <NSTableViewDataSource, NSTableViewDelegate>

@property (readwrite, retain, nullable) NSArray <VLCMediaLibraryAlbum *> *representedListOfAlbums;

@end

NS_ASSUME_NONNULL_END
