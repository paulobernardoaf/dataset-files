#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#import <Cocoa/Cocoa.h>
#import <vlc_common.h>
#import <vlc_interface.h>
intf_thread_t *getIntf(void);
extern NSString *VLCConfigurationChangedNotification;
@class VLCExtensionsManager;
@class VLCMainMenu;
@class VLCSimplePrefsController;
@class VLCPrefs;
@class VLCCoreDialogProvider;
@class VLCBookmarksWindowController;
@class VLCOpenWindowController;
@class VLCLogWindowController;
@class VLCLibraryWindow;
@class VLCLibraryWindowController;
@class VLCTrackSynchronizationWindowController;
@class VLCAudioEffectsWindowController;
@class VLCVideoEffectsWindowController;
@class VLCConvertAndSaveWindowController;
@class VLCLibraryController;
@class VLCPlaylistController;
@class VLCVideoOutputProvider;
@interface VLCMain : NSObject
+ (VLCMain *)sharedInstance;
+ (void)relaunchApplication;
@property (readonly) BOOL isTerminating;
@property (readonly) VLCAudioEffectsWindowController *audioEffectsPanel;
@property (readonly) VLCBookmarksWindowController *bookmarks;
@property (readonly) VLCConvertAndSaveWindowController *convertAndSaveWindow;
@property (readonly) VLCCoreDialogProvider *coreDialogProvider;
@property (readonly) VLCExtensionsManager *extensionsManager;
@property (readonly) VLCLogWindowController *debugMsgPanel;
@property (readonly) VLCLibraryController *libraryController;
@property (readonly) VLCLibraryWindow *libraryWindow;
@property (readonly) VLCLibraryWindowController *libraryWindowController;
@property (readonly) VLCMainMenu *mainMenu;
@property (readonly) VLCOpenWindowController *open;
@property (readonly) VLCPlaylistController *playlistController;
@property (readonly) VLCPrefs *preferences;
@property (readonly) VLCSimplePrefsController *simplePreferences;
@property (readonly) VLCTrackSynchronizationWindowController *trackSyncPanel;
@property (readonly) VLCVideoEffectsWindowController *videoEffectsPanel;
@property (readonly) VLCVideoOutputProvider *voutProvider;
@end
