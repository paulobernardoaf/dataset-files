#import <Cocoa/Cocoa.h>
#import <mach/mach.h>
#import <mach/mach_error.h>
#import <IOKit/IOKitLib.h>
#import <IOKit/IOCFPlugIn.h>
#import <IOKit/hid/IOHIDLib.h>
#import <IOKit/hid/IOHIDKeys.h>
enum AppleRemoteEventIdentifier
{
kRemoteButtonVolume_Plus =1<<1,
kRemoteButtonVolume_Minus =1<<2,
kRemoteButtonMenu =1<<3,
kRemoteButtonPlay =1<<4,
kRemoteButtonRight =1<<5,
kRemoteButtonLeft =1<<6,
kRemoteButtonRight_Hold =1<<7,
kRemoteButtonLeft_Hold =1<<8,
kRemoteButtonMenu_Hold =1<<9,
kRemoteButtonPlay_Sleep =1<<10,
kRemoteControl_Switched =1<<11,
kRemoteButtonVolume_Plus_Hold =1<<12,
kRemoteButtonVolume_Minus_Hold =1<<13,
k2009RemoteButtonPlay
=1<<14,
k2009RemoteButtonFullscreen
=1<<15
};
typedef enum AppleRemoteEventIdentifier AppleRemoteEventIdentifier;
@interface AppleRemote : NSObject {
IOHIDDeviceInterface** hidDeviceInterface;
IOHIDQueueInterface** queue;
NSArray* _allCookies;
NSDictionary* _cookieToButtonMapping;
CFRunLoopSourceRef eventSource;
BOOL lastEventSimulatedHold;
AppleRemoteEventIdentifier lastPlusMinusEvent;
NSTimeInterval lastPlusMinusEventTime;
int remoteId;
NSTimeInterval lastClickCountEventTime;
AppleRemoteEventIdentifier lastClickCountEvent;
unsigned int eventClickCount;
id delegate;
}
@property (readonly) int remoteId;
@property (readonly) BOOL remoteAvailable;
@property (readwrite) BOOL listeningToRemote;
@property (readwrite) BOOL openInExclusiveMode;
@property (readwrite) BOOL clickCountingEnabled;
@property (readwrite) unsigned int clickCountEnabledButtons;
@property (readwrite) NSTimeInterval maximumClickCountTimeDifference;
@property (readwrite) BOOL processesBacklog;
@property (readwrite) BOOL listeningOnAppActivate;
@property (readwrite) BOOL simulatesPlusMinusHold;
@property (readwrite, assign) id delegate;
- (IBAction) startListening: (id) sender;
- (IBAction) stopListening: (id) sender;
@end
@interface AppleRemote (Singleton)
+ (AppleRemote*) sharedRemote;
@end
@interface NSObject(NSAppleRemoteDelegate)
- (void) appleRemoteButton: (AppleRemoteEventIdentifier)buttonIdentifier pressedDown: (BOOL) pressedDown clickCount: (unsigned int) count;
@end
@interface AppleRemote (PrivateMethods)
@property (readonly) NSDictionary * cookieToButtonMapping;
- (void) setRemoteId: (int) aValue;
- (IOHIDQueueInterface**) queue;
- (IOHIDDeviceInterface**) hidDeviceInterface;
- (void) handleEventWithCookieString: (NSString*) cookieString sumOfValues: (SInt32) sumOfValues;
@end
@interface AppleRemote (IOKitMethods)
- (io_object_t) findAppleRemoteDevice;
- (IOHIDDeviceInterface**) createInterfaceForDevice: (io_object_t) hidDevice;
- (BOOL) initializeCookies;
- (BOOL) openDevice;
@end
@interface AppleRemoteApplicationDelegate : NSObject<NSApplicationDelegate> {
id applicationDelegate;
}
- (id) initWithApplicationDelegate: (id) delegate;
- (id) applicationDelegate;
@end
