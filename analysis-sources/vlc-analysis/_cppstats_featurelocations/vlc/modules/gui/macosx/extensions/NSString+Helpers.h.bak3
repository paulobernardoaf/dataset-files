


























#import <Cocoa/Cocoa.h>
#import <vlc_input.h>

#define _NS(s) ((s) ? toNSStr(vlc_gettext(s)) : @"")




#define _ANS(s) [((s) ? toNSStr(vlc_gettext(s)) : @"") substringFromIndex:2]

extern NSString *const kVLCMediaAudioCD;
extern NSString *const kVLCMediaDVD;
extern NSString *const kVLCMediaVCD;
extern NSString *const kVLCMediaSVCD;
extern NSString *const kVLCMediaBD;
extern NSString *const kVLCMediaVideoTSFolder;
extern NSString *const kVLCMediaBDMVFolder;
extern NSString *const kVLCMediaUnknown;

NSString *toNSStr(const char *str);




unsigned int CocoaKeyToVLC(unichar i_key);




bool fixIntfSettings(void);




NSImage *imageFromRes(NSString *name);

@interface NSString (Helpers)











+ (instancetype)stringWithDuration:(vlc_tick_t)duration
currentTime:(vlc_tick_t)time
negative:(BOOL)negative;









+ (instancetype)stringWithTime:(long long int)time;









+ (instancetype)stringWithTimeFromTicks:(vlc_tick_t)time;






+ (NSInteger)timeInSecondsFromStringWithColons:(NSString *)aString;








+ (instancetype)base64StringWithCString:(const char *)cstring;







- (NSString *)base64EncodedString;







- (NSString *)base64DecodedString;









- (NSString *)stringWrappedToWidth:(int)width;

@end









static inline NSString *B64DecNSStr(NSString *s) {
NSString *res = [s base64DecodedString];

return (res == nil) ? @"" : res;
}










static inline NSString *B64EncAndFree(char *cs) {
NSString *res = [NSString base64StringWithCString:cs];
free(cs);

return (res == nil) ? @"" : res;
}

NSString * getVolumeTypeFromMountPath(NSString *mountPath);

NSString * getBSDNodeFromMountPath(NSString *mountPath);






NSString * OSXStringKeyToString(NSString *theString);




NSString * VLCKeyToString(char *theChar);




unsigned int VLCModifiersToCocoa(char *theChar);
