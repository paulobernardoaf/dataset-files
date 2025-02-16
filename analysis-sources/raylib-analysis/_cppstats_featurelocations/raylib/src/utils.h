

























#if !defined(UTILS_H)
#define UTILS_H

#if defined(PLATFORM_ANDROID)
#include <stdio.h> 
#include <android/asset_manager.h> 
#endif

#if defined(SUPPORT_TRACELOG)
#define TRACELOG(level, ...) TraceLog(level, __VA_ARGS__)

#if defined(SUPPORT_TRACELOG_DEBUG)
#define TRACELOGD(...) TraceLog(LOG_DEBUG, __VA_ARGS__)
#else
#define TRACELOGD(...) (void)0
#endif
#else
#define TRACELOG(level, ...) (void)0
#define TRACELOGD(...) (void)0
#endif




#if defined(PLATFORM_ANDROID)
#define fopen(name, mode) android_fopen(name, mode)
#endif




#if defined(__cplusplus)
extern "C" { 
#endif









#if defined(PLATFORM_ANDROID)
void InitAssetManager(AAssetManager *manager, const char *dataPath); 
FILE *android_fopen(const char *fileName, const char *mode); 
#endif

#if defined(PLATFORM_UWP)

typedef enum {
UWP_MSG_NONE = 0,


UWP_MSG_SHOW_MOUSE,
UWP_MSG_HIDE_MOUSE,
UWP_MSG_LOCK_MOUSE,
UWP_MSG_UNLOCK_MOUSE,
UWP_MSG_SET_MOUSE_LOCATION, 


UWP_MSG_REGISTER_KEY, 
UWP_MSG_REGISTER_CLICK, 
UWP_MSG_SCROLL_WHEEL_UPDATE, 
UWP_MSG_UPDATE_MOUSE_LOCATION, 
UWP_MSG_SET_GAMEPAD_ACTIVE, 
UWP_MSG_SET_GAMEPAD_BUTTON, 
UWP_MSG_SET_GAMEPAD_AXIS, 
UWP_MSG_SET_DISPLAY_DIMS, 
UWP_MSG_HANDLE_RESIZE, 
UWP_MSG_SET_GAME_TIME, 
} UWPMessageType;

typedef struct UWPMessage {
UWPMessageType type; 

Vector2 paramVector0; 
int paramInt0; 
int paramInt1; 
char paramChar0; 
float paramFloat0; 
double paramDouble0; 
bool paramBool0; 


} UWPMessage;


RLAPI UWPMessage* CreateUWPMessage(void);


RLAPI void DeleteUWPMessage(UWPMessage* msg);


RLAPI bool UWPHasMessages(void);
RLAPI UWPMessage* UWPGetMessage(void);
RLAPI void UWPSendMessage(UWPMessage* msg);


#if !defined(__cplusplus)
void SendMessageToUWP(UWPMessage* msg);
bool HasMessageFromUWP(void);
UWPMessage* GetMessageFromUWP(void);
#endif

#endif 

#if defined(__cplusplus)
}
#endif

#endif 
