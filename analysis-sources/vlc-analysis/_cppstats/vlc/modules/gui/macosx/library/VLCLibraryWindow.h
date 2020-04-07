#import "windows/video/VLCVideoWindowCommon.h"
NS_ASSUME_NONNULL_BEGIN
@class VLCDragDropView;
@class VLCRoundedCornerTextField;
@interface VLCLibraryWindowController : NSWindowController
- (instancetype)initWithLibraryWindow;
@end
@interface VLCLibraryWindow : VLCVideoWindowCommon
@property (readwrite, weak) IBOutlet NSSegmentedControl *segmentedTitleControl;
@property (readwrite, weak) IBOutlet NSSegmentedControl *gridVsListSegmentedControl;
@property (readwrite, weak) IBOutlet NSSplitView *mainSplitView;
@property (readwrite, weak) IBOutlet NSStackView *videoLibraryStackView;
@property (readwrite, strong) IBOutlet NSView *playlistView;
@property (readwrite, weak) IBOutlet NSCollectionView *videoLibraryCollectionView;
@property (readwrite, weak) IBOutlet NSCollectionView *recentVideoLibraryCollectionView;
@property (readwrite, weak) IBOutlet NSCollectionView *mediaSourceCollectionView;
@property (readwrite, weak) IBOutlet NSView *audioLibraryView;
@property (readwrite, weak) IBOutlet NSSplitView *audioLibrarySplitView;
@property (readwrite, weak) IBOutlet NSTableView *audioCollectionSelectionTableView;
@property (readwrite, weak) IBOutlet NSTableView *audioGroupSelectionTableView;
@property (readwrite, weak) IBOutlet NSScrollView *audioCollectionViewScrollView;
@property (readwrite, weak) IBOutlet NSCollectionView *audioLibraryCollectionView;
@property (readwrite, weak) IBOutlet NSSegmentedControl *audioSegmentedControl;
@property (readwrite, weak) IBOutlet NSView *mediaSourceView;
@property (readwrite, weak) IBOutlet NSButton *mediaSourceHomeButton;
@property (readwrite, weak) IBOutlet NSPathControl *mediaSourcePathControl;
@property (readwrite, weak) IBOutlet NSTableView *mediaSourceTableView;
@property (readwrite, weak) IBOutlet NSScrollView *mediaSourceCollectionViewScrollView;
@property (readwrite, weak) IBOutlet NSView *libraryTargetView;
@property (readwrite, weak) IBOutlet NSTableView *playlistTableView;
@property (readwrite, weak) IBOutlet NSTextField *upNextLabel;
@property (readwrite, weak) IBOutlet VLCDragDropView *playlistDragDropView;
@property (readwrite, weak) IBOutlet NSButton *openMediaButton;
@property (readwrite, weak) IBOutlet NSBox *upNextSeparator;
@property (readwrite, weak) IBOutlet NSButton *clearPlaylistButton;
@property (readwrite, weak) IBOutlet NSBox *clearPlaylistSeparator;
@property (readwrite, weak) IBOutlet NSButton *repeatPlaylistButton;
@property (readwrite, weak) IBOutlet NSButton *shufflePlaylistButton;
@property (readwrite, weak) IBOutlet VLCRoundedCornerTextField *playlistCounterTextField;
@property (readwrite, weak) IBOutlet NSButton *librarySortButton;
@property (readonly) BOOL nativeFullscreenMode;
@property (readwrite) BOOL nonembedded;
- (void)videoPlaybackWillBeStarted;
- (void)enableVideoPlaybackAppearance;
- (void)disableVideoPlaybackAppearance;
- (IBAction)playlistDoubleClickAction:(id)sender;
- (IBAction)shuffleAction:(id)sender;
- (IBAction)repeatAction:(id)sender;
- (IBAction)clearPlaylist:(id)sender;
- (IBAction)sortPlaylist:(id)sender;
- (IBAction)sortLibrary:(id)sender;
- (IBAction)openMedia:(id)sender;
- (IBAction)showAndHidePlaylist:(id)sender;
@end
NS_ASSUME_NONNULL_END
