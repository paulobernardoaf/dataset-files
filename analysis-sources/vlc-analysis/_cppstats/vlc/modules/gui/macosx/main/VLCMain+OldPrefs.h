#import "main/VLCMain.h"
@interface VLCMain (OldPrefs)
- (void)resetAndReinitializeUserDefaults;
- (void)migrateOldPreferences;
- (void)resetPreferences;
@end
