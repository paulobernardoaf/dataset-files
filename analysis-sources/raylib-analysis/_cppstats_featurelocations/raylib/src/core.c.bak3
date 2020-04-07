


















































































































#include "raylib.h" 


#if !defined(EXTERNAL_CONFIG_FLAGS)
#include "config.h" 
#else
#define RAYLIB_VERSION "3.0"
#endif

#include "utils.h" 

#if (defined(__linux__) || defined(PLATFORM_WEB)) && _POSIX_C_SOURCE < 199309L
#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L 
#endif

#define RAYMATH_IMPLEMENTATION 
#include "raymath.h" 

#define RLGL_IMPLEMENTATION
#include "rlgl.h" 

#if defined(SUPPORT_GESTURES_SYSTEM)
#define GESTURES_IMPLEMENTATION
#include "gestures.h" 
#endif

#if defined(SUPPORT_CAMERA_SYSTEM)
#define CAMERA_IMPLEMENTATION
#include "camera.h" 
#endif

#if defined(SUPPORT_GIF_RECORDING)
#define RGIF_MALLOC RL_MALLOC
#define RGIF_FREE RL_FREE

#define RGIF_IMPLEMENTATION
#include "external/rgif.h" 
#endif

#if defined(__APPLE__)
#define SUPPORT_HIGH_DPI 
#endif

#include <stdlib.h> 
#include <stdio.h> 
#include <string.h> 
#include <time.h> 
#include <math.h> 

#include <sys/stat.h> 

#if (defined(PLATFORM_DESKTOP) || defined(PLATFORM_UWP)) && defined(_WIN32) && (defined(_MSC_VER) || defined(__TINYC__))
#define DIRENT_MALLOC RL_MALLOC
#define DIRENT_FREE RL_FREE

#include "external/dirent.h" 
#else
#include <dirent.h> 
#endif

#if defined(_WIN32)
#include <direct.h> 
#define GETCWD _getcwd 
#define CHDIR _chdir
#include <io.h> 
#else
#include <unistd.h> 
#define GETCWD getcwd
#define CHDIR chdir
#endif

#if defined(PLATFORM_DESKTOP)
#define GLFW_INCLUDE_NONE 

#include <GLFW/glfw3.h> 



#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h> 

#if !defined(SUPPORT_BUSY_WAIT_LOOP)

unsigned int __stdcall timeBeginPeriod(unsigned int uPeriod);
unsigned int __stdcall timeEndPeriod(unsigned int uPeriod);
#endif

#elif defined(__linux__)
#include <sys/time.h> 




#include <GLFW/glfw3native.h> 
#elif defined(__APPLE__)
#include <unistd.h> 


#include <GLFW/glfw3native.h> 
#endif
#endif

#if defined(__linux__)
#define MAX_FILEPATH_LENGTH 4096 
#else
#define MAX_FILEPATH_LENGTH 512 
#endif

#if defined(PLATFORM_ANDROID)

#include <android/window.h> 
#include <android_native_app_glue.h> 

#include <EGL/egl.h> 
#include <GLES2/gl2.h> 
#endif

#if defined(PLATFORM_RPI)
#include <fcntl.h> 
#include <unistd.h> 
#include <termios.h> 
#include <pthread.h> 
#include <dirent.h> 

#include <sys/ioctl.h> 
#include <linux/kd.h> 
#include <linux/input.h> 
#include <linux/joystick.h> 

#include "bcm_host.h" 

#include "EGL/egl.h" 
#include "EGL/eglext.h" 
#include "GLES2/gl2.h" 
#endif

#if defined(PLATFORM_UWP)
#include "EGL/egl.h" 
#include "EGL/eglext.h" 
#include "GLES2/gl2.h" 
#endif

#if defined(PLATFORM_WEB)
#define GLFW_INCLUDE_ES2 
#include <GLFW/glfw3.h> 
#include <sys/time.h> 

#include <emscripten/emscripten.h> 
#include <emscripten/html5.h> 
#endif

#if defined(SUPPORT_COMPRESSION_API)

unsigned char *stbi_zlib_compress(unsigned char *data, int data_len, int *out_len, int quality);
char *stbi_zlib_decode_malloc(char const *buffer, int len, int *outlen);
#endif




#if defined(PLATFORM_RPI)
#define USE_LAST_TOUCH_DEVICE 


#define DEFAULT_KEYBOARD_DEV STDIN_FILENO 
#define DEFAULT_GAMEPAD_DEV "/dev/input/js" 
#define DEFAULT_EVDEV_PATH "/dev/input/" 






#define MOUSE_SENSITIVITY 0.8f
#endif

#define MAX_GAMEPADS 4 
#define MAX_GAMEPAD_AXIS 8 
#define MAX_GAMEPAD_BUTTONS 32 

#define MAX_CHARS_QUEUE 16 

#if defined(SUPPORT_DATA_STORAGE)
#define STORAGE_DATA_FILE "storage.data"
#endif




#if defined(PLATFORM_RPI)
typedef struct {
pthread_t threadId; 
int fd; 
int eventNum; 
Rectangle absRange; 
int touchSlot; 
bool isMouse; 
bool isTouch; 
bool isMultitouch; 
bool isKeyboard; 
bool isGamepad; 
} InputEventWorker;

typedef struct {
int contents[8]; 
char head; 
char tail; 
} KeyEventFifo;
#endif

typedef struct { int x; int y; } Point;
typedef struct { unsigned int width; unsigned int height; } Size;

#if defined(PLATFORM_UWP)
extern EGLNativeWindowType handle; 
#endif


typedef struct CoreData {
struct {
#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
GLFWwindow *handle; 
#endif
#if defined(PLATFORM_RPI)

EGL_DISPMANX_WINDOW_T handle; 
#endif
#if defined(PLATFORM_ANDROID) || defined(PLATFORM_RPI) || defined(PLATFORM_UWP)
EGLDisplay device; 
EGLSurface surface; 
EGLContext context; 
EGLConfig config; 
#endif
unsigned int flags; 
const char *title; 
bool ready; 
bool minimized; 
bool resized; 
bool fullscreen; 
bool alwaysRun; 
bool shouldClose; 

Point position; 
Size display; 
Size screen; 
Size currentFbo; 
Size render; 
Point renderOffset; 
Matrix screenScale; 

char **dropFilesPath; 
int dropFilesCount; 

} Window;
#if defined(PLATFORM_ANDROID)
struct {
bool appEnabled; 
struct android_app *app; 
struct android_poll_source *source; 
const char *internalDataPath; 
bool contextRebindRequired; 
} Android;
#endif
struct {
#if defined(PLATFORM_RPI)
InputEventWorker eventWorker[10]; 
#endif
struct {
int exitKey; 
char currentKeyState[512]; 
char previousKeyState[512]; 

int keyPressedQueue[MAX_CHARS_QUEUE]; 
int keyPressedQueueCount; 
#if defined(PLATFORM_RPI)
int defaultMode; 
struct termios defaultSettings; 
KeyEventFifo lastKeyPressed; 
#endif
} Keyboard;
struct {
Vector2 position; 
Vector2 offset; 
Vector2 scale; 

bool cursorHidden; 
bool cursorOnScreen; 
#if defined(PLATFORM_WEB)
bool cursorLockRequired; 
#endif
char currentButtonState[3]; 
char previousButtonState[3]; 
int currentWheelMove; 
int previousWheelMove; 
#if defined(PLATFORM_RPI)
char currentButtonStateEvdev[3]; 
#endif
} Mouse;
struct {
Vector2 position[MAX_TOUCH_POINTS]; 
char currentTouchState[MAX_TOUCH_POINTS]; 
char previousTouchState[MAX_TOUCH_POINTS]; 
} Touch;
struct {
int lastButtonPressed; 
int axisCount; 
#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_RPI) || defined(PLATFORM_WEB) || defined(PLATFORM_UWP)
bool ready[MAX_GAMEPADS]; 
float axisState[MAX_GAMEPADS][MAX_GAMEPAD_AXIS]; 
char currentState[MAX_GAMEPADS][MAX_GAMEPAD_BUTTONS]; 
char previousState[MAX_GAMEPADS][MAX_GAMEPAD_BUTTONS]; 
#endif
#if defined(PLATFORM_RPI)
pthread_t threadId; 
int streamId[MAX_GAMEPADS]; 
char name[64]; 
#endif
} Gamepad;
} Input;
struct {
double current; 
double previous; 
double update; 
double draw; 
double frame; 
double target; 
#if defined(PLATFORM_ANDROID) || defined(PLATFORM_RPI) || defined(PLATFORM_UWP)
unsigned long long base; 
#endif
} Time;
} CoreData;




static CoreData CORE = { 0 }; 

static char **dirFilesPath = NULL; 
static int dirFilesCount = 0; 

#if defined(SUPPORT_SCREEN_CAPTURE)
static int screenshotCounter = 0; 
#endif

#if defined(SUPPORT_GIF_RECORDING)
static int gifFramesCounter = 0; 
static bool gifRecording = false; 
#endif





#if defined(SUPPORT_DEFAULT_FONT)
extern void LoadFontDefault(void); 
extern void UnloadFontDefault(void); 
#endif




static bool InitGraphicsDevice(int width, int height); 
static void SetupFramebuffer(int width, int height); 
static void SetupViewport(int width, int height); 
static void SwapBuffers(void); 

static void InitTimer(void); 
static void Wait(float ms); 

static int GetGamepadButton(int button); 
static int GetGamepadAxis(int axis); 
static void PollInputEvents(void); 

#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
static void ErrorCallback(int error, const char *description); 
static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods); 
static void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods); 
static void MouseCursorPosCallback(GLFWwindow *window, double x, double y); 
static void CharCallback(GLFWwindow *window, unsigned int key); 
static void ScrollCallback(GLFWwindow *window, double xoffset, double yoffset); 
static void CursorEnterCallback(GLFWwindow *window, int enter); 
static void WindowSizeCallback(GLFWwindow *window, int width, int height); 
static void WindowIconifyCallback(GLFWwindow *window, int iconified); 
static void WindowDropCallback(GLFWwindow *window, int count, const char **paths); 
#endif

#if defined(PLATFORM_ANDROID)
static void AndroidCommandCallback(struct android_app *app, int32_t cmd); 
static int32_t AndroidInputCallback(struct android_app *app, AInputEvent *event); 
#endif

#if defined(PLATFORM_WEB)
static EM_BOOL EmscriptenFullscreenChangeCallback(int eventType, const EmscriptenFullscreenChangeEvent *event, void *userData);
static EM_BOOL EmscriptenKeyboardCallback(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData);
static EM_BOOL EmscriptenMouseCallback(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData);
static EM_BOOL EmscriptenTouchCallback(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData);
static EM_BOOL EmscriptenGamepadCallback(int eventType, const EmscriptenGamepadEvent *gamepadEvent, void *userData);
#endif

#if defined(PLATFORM_RPI)
#if defined(SUPPORT_SSH_KEYBOARD_RPI)
static void InitKeyboard(void); 
static void ProcessKeyboard(void); 
static void RestoreKeyboard(void); 
#else
static void InitTerminal(void); 
static void RestoreTerminal(void); 
#endif

static void InitEvdevInput(void); 
static void EventThreadSpawn(char *device); 
static void *EventThread(void *arg); 

static void InitGamepad(void); 
static void *GamepadThread(void *arg); 
#endif 

#if defined(_WIN32)

void __stdcall Sleep(unsigned long msTimeout); 
#endif





#if defined(PLATFORM_ANDROID)


extern int main(int argc, char *argv[]);

void android_main(struct android_app *app)
{
char arg0[] = "raylib"; 
CORE.Android.app = app;


(void)main(1, (char *[]) { arg0, NULL });
}


struct android_app *GetAndroidApp(void)
{
return CORE.Android.app;
}
#endif
#if defined(PLATFORM_RPI) && !defined(SUPPORT_SSH_KEYBOARD_RPI)

static void InitTerminal(void)
{
TRACELOG(LOG_INFO, "RPI: Reconfiguring terminal...");


struct termios keyboardNewSettings;
tcgetattr(STDIN_FILENO, &CORE.Input.Keyboard.defaultSettings); 
keyboardNewSettings = CORE.Input.Keyboard.defaultSettings;



keyboardNewSettings.c_lflag &= ~(ICANON | ECHO | ISIG);
keyboardNewSettings.c_cc[VMIN] = 1;
keyboardNewSettings.c_cc[VTIME] = 0;


tcsetattr(STDIN_FILENO, TCSANOW, &keyboardNewSettings);


if (ioctl(STDIN_FILENO, KDGKBMODE, &CORE.Input.Keyboard.defaultMode) < 0)
{

TRACELOG(LOG_WARNING, "RPI: Failed to change keyboard mode (not a local terminal)");
}
else ioctl(STDIN_FILENO, KDSKBMODE, K_XLATE);


atexit(RestoreTerminal);
}

static void RestoreTerminal(void)
{
TRACELOG(LOG_INFO, "RPI: Restoring terminal...");


tcsetattr(STDIN_FILENO, TCSANOW, &CORE.Input.Keyboard.defaultSettings);


ioctl(STDIN_FILENO, KDSKBMODE, CORE.Input.Keyboard.defaultMode);
}
#endif


void InitWindow(int width, int height, const char *title)
{
TRACELOG(LOG_INFO, "Initializing raylib %s", RAYLIB_VERSION);

CORE.Window.title = title;


CORE.Input.Keyboard.exitKey = KEY_ESCAPE;
CORE.Input.Mouse.scale = (Vector2){ 1.0f, 1.0f };
CORE.Input.Gamepad.lastButtonPressed = -1;

#if defined(PLATFORM_ANDROID)
CORE.Window.screen.width = width;
CORE.Window.screen.height = height;
CORE.Window.currentFbo.width = width;
CORE.Window.currentFbo.height = height;


CORE.Android.internalDataPath = CORE.Android.app->activity->internalDataPath;


ANativeActivity_setWindowFlags(CORE.Android.app->activity, AWINDOW_FLAG_FULLSCREEN, 0); 

int orientation = AConfiguration_getOrientation(CORE.Android.app->config);

if (orientation == ACONFIGURATION_ORIENTATION_PORT) TRACELOG(LOG_INFO, "ANDROID: Window orientation set as portrait");
else if (orientation == ACONFIGURATION_ORIENTATION_LAND) TRACELOG(LOG_INFO, "ANDROID: Window orientation set as landscape");


if (width <= height)
{
AConfiguration_setOrientation(CORE.Android.app->config, ACONFIGURATION_ORIENTATION_PORT);
TRACELOG(LOG_WARNING, "ANDROID: Window orientation changed to portrait");
}
else
{
AConfiguration_setOrientation(CORE.Android.app->config, ACONFIGURATION_ORIENTATION_LAND);
TRACELOG(LOG_WARNING, "ANDROID: Window orientation changed to landscape");
}






CORE.Android.app->onAppCmd = AndroidCommandCallback;
CORE.Android.app->onInputEvent = AndroidInputCallback;

InitAssetManager(CORE.Android.app->activity->assetManager, CORE.Android.app->activity->internalDataPath);

TRACELOG(LOG_INFO, "ANDROID: App initialized successfully");


int pollResult = 0;
int pollEvents = 0;


while (!CORE.Window.ready)
{

while ((pollResult = ALooper_pollAll(0, NULL, &pollEvents, (void**)&CORE.Android.source)) >= 0)
{

if (CORE.Android.source != NULL) CORE.Android.source->process(CORE.Android.app, CORE.Android.source);



}
}
#else


CORE.Window.ready = InitGraphicsDevice(width, height);
if (!CORE.Window.ready) return;


InitTimer();

#if defined(SUPPORT_DEFAULT_FONT)


LoadFontDefault();
Rectangle rec = GetFontDefault().recs[95];

SetShapesTexture(GetFontDefault().texture, (Rectangle){ rec.x + 1, rec.y + 1, rec.width - 2, rec.height - 2 });
#endif
#if defined(PLATFORM_DESKTOP) && defined(SUPPORT_HIGH_DPI)

SetTextureFilter(GetFontDefault().texture, FILTER_BILINEAR);
#endif

#if defined(PLATFORM_RPI)

InitEvdevInput(); 
InitGamepad(); 
#if defined(SUPPORT_SSH_KEYBOARD_RPI)
InitKeyboard(); 
#else
InitTerminal(); 
#endif
#endif

#if defined(PLATFORM_WEB)

emscripten_set_fullscreenchange_callback("#canvas", NULL, 1, EmscriptenFullscreenChangeCallback);


emscripten_set_keypress_callback("#canvas", NULL, 1, EmscriptenKeyboardCallback);


emscripten_set_click_callback("#canvas", NULL, 1, EmscriptenMouseCallback);


emscripten_set_touchstart_callback("#canvas", NULL, 1, EmscriptenTouchCallback);
emscripten_set_touchend_callback("#canvas", NULL, 1, EmscriptenTouchCallback);
emscripten_set_touchmove_callback("#canvas", NULL, 1, EmscriptenTouchCallback);
emscripten_set_touchcancel_callback("#canvas", NULL, 1, EmscriptenTouchCallback);


emscripten_set_gamepadconnected_callback(NULL, 1, EmscriptenGamepadCallback);
emscripten_set_gamepaddisconnected_callback(NULL, 1, EmscriptenGamepadCallback);
#endif

CORE.Input.Mouse.position.x = (float)CORE.Window.screen.width/2.0f;
CORE.Input.Mouse.position.y = (float)CORE.Window.screen.height/2.0f;
#endif 
}


void CloseWindow(void)
{
#if defined(SUPPORT_GIF_RECORDING)
if (gifRecording)
{
GifEnd();
gifRecording = false;
}
#endif

#if defined(SUPPORT_DEFAULT_FONT)
UnloadFontDefault();
#endif

rlglClose(); 

#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
glfwDestroyWindow(CORE.Window.handle);
glfwTerminate();
#endif

#if !defined(SUPPORT_BUSY_WAIT_LOOP) && defined(_WIN32)
timeEndPeriod(1); 
#endif

#if defined(PLATFORM_ANDROID) || defined(PLATFORM_RPI) || defined(PLATFORM_UWP)

if (CORE.Window.device != EGL_NO_DISPLAY)
{
eglMakeCurrent(CORE.Window.device, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

if (CORE.Window.surface != EGL_NO_SURFACE)
{
eglDestroySurface(CORE.Window.device, CORE.Window.surface);
CORE.Window.surface = EGL_NO_SURFACE;
}

if (CORE.Window.context != EGL_NO_CONTEXT)
{
eglDestroyContext(CORE.Window.device, CORE.Window.context);
CORE.Window.context = EGL_NO_CONTEXT;
}

eglTerminate(CORE.Window.device);
CORE.Window.device = EGL_NO_DISPLAY;
}
#endif

#if defined(PLATFORM_RPI)




CORE.Window.shouldClose = true; 

for (int i = 0; i < sizeof(CORE.Input.eventWorker)/sizeof(InputEventWorker); ++i)
{
if (CORE.Input.eventWorker[i].threadId)
{
pthread_join(CORE.Input.eventWorker[i].threadId, NULL);
}
}

if (CORE.Input.Gamepad.threadId) pthread_join(CORE.Input.Gamepad.threadId, NULL);
#endif

TRACELOG(LOG_INFO, "Window closed successfully");
}


bool IsWindowReady(void)
{
return CORE.Window.ready;
}


bool WindowShouldClose(void)
{
#if defined(PLATFORM_WEB)





emscripten_sleep(16);
return false;
#endif

#if defined(PLATFORM_DESKTOP)
if (CORE.Window.ready)
{

while (!CORE.Window.alwaysRun && CORE.Window.minimized) glfwWaitEvents();

CORE.Window.shouldClose = glfwWindowShouldClose(CORE.Window.handle);

return CORE.Window.shouldClose;
}
else return true;
#endif

#if defined(PLATFORM_ANDROID) || defined(PLATFORM_RPI) || defined(PLATFORM_UWP)
if (CORE.Window.ready) return CORE.Window.shouldClose;
else return true;
#endif
}


bool IsWindowMinimized(void)
{
#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB) || defined(PLATFORM_UWP)
return CORE.Window.minimized;
#else
return false;
#endif
}


bool IsWindowResized(void)
{
#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB) || defined(PLATFORM_UWP)
return CORE.Window.resized;
#else
return false;
#endif
}


bool IsWindowHidden(void)
{
#if defined(PLATFORM_DESKTOP)
return (glfwGetWindowAttrib(CORE.Window.handle, GLFW_VISIBLE) == GL_FALSE);
#endif
return false;
}


bool IsWindowFullscreen(void)
{
return CORE.Window.fullscreen;
}


void ToggleFullscreen(void)
{
CORE.Window.fullscreen = !CORE.Window.fullscreen; 

#if defined(PLATFORM_DESKTOP)

if (CORE.Window.fullscreen)
{

glfwGetWindowPos(CORE.Window.handle, &CORE.Window.position.x, &CORE.Window.position.y);

GLFWmonitor *monitor = glfwGetPrimaryMonitor();
if (!monitor)
{
TRACELOG(LOG_WARNING, "GLFW: Failed to get monitor");
glfwSetWindowMonitor(CORE.Window.handle, glfwGetPrimaryMonitor(), 0, 0, CORE.Window.screen.width, CORE.Window.screen.height, GLFW_DONT_CARE);
return;
}

const GLFWvidmode *mode = glfwGetVideoMode(monitor);
glfwSetWindowMonitor(CORE.Window.handle, glfwGetPrimaryMonitor(), 0, 0, CORE.Window.screen.width, CORE.Window.screen.height, mode->refreshRate);



if (CORE.Window.flags & FLAG_VSYNC_HINT) glfwSwapInterval(1);
}
else glfwSetWindowMonitor(CORE.Window.handle, NULL, CORE.Window.position.x, CORE.Window.position.y, CORE.Window.screen.width, CORE.Window.screen.height, GLFW_DONT_CARE);
#endif
#if defined(PLATFORM_WEB)
if (CORE.Window.fullscreen) EM_ASM(Module.requestFullscreen(false, false););
else EM_ASM(document.exitFullscreen(););
#endif
#if defined(PLATFORM_ANDROID) || defined(PLATFORM_RPI)
TRACELOG(LOG_WARNING, "SYSTEM: Failed to toggle to windowed mode");
#endif
}



void SetWindowIcon(Image image)
{
#if defined(PLATFORM_DESKTOP)
if (image.format == UNCOMPRESSED_R8G8B8A8)
{
GLFWimage icon[1] = { 0 };

icon[0].width = image.width;
icon[0].height = image.height;
icon[0].pixels = (unsigned char *)image.data;



glfwSetWindowIcon(CORE.Window.handle, 1, icon);
}
else TRACELOG(LOG_WARNING, "GLFW: Window icon image must be in R8G8B8A8 pixel format");
#endif
}


void SetWindowTitle(const char *title)
{
CORE.Window.title = title;
#if defined(PLATFORM_DESKTOP)
glfwSetWindowTitle(CORE.Window.handle, title);
#endif
}


void SetWindowPosition(int x, int y)
{
#if defined(PLATFORM_DESKTOP)
glfwSetWindowPos(CORE.Window.handle, x, y);
#endif
}


void SetWindowMonitor(int monitor)
{
#if defined(PLATFORM_DESKTOP)
int monitorCount = 0;
GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

if ((monitor >= 0) && (monitor < monitorCount))
{
TRACELOG(LOG_INFO, "GLFW: Selected fullscreen monitor: [%i] %s", monitor, glfwGetMonitorName(monitors[monitor]));

const GLFWvidmode *mode = glfwGetVideoMode(monitors[monitor]);
glfwSetWindowMonitor(CORE.Window.handle, monitors[monitor], 0, 0, mode->width, mode->height, mode->refreshRate);
}
else TRACELOG(LOG_WARNING, "GLFW: Failed to find selected monitor");
#endif
}


void SetWindowMinSize(int width, int height)
{
#if defined(PLATFORM_DESKTOP)
const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
glfwSetWindowSizeLimits(CORE.Window.handle, width, height, mode->width, mode->height);
#endif
}



void SetWindowSize(int width, int height)
{
#if defined(PLATFORM_DESKTOP)
glfwSetWindowSize(CORE.Window.handle, width, height);
#endif
#if defined(PLATFORM_WEB)
emscripten_set_canvas_size(width, height); 





#endif
}


void UnhideWindow(void)
{
#if defined(PLATFORM_DESKTOP)
glfwShowWindow(CORE.Window.handle);
#endif
}


void HideWindow(void)
{
#if defined(PLATFORM_DESKTOP)
glfwHideWindow(CORE.Window.handle);
#endif
}


int GetScreenWidth(void)
{
return CORE.Window.screen.width;
}


int GetScreenHeight(void)
{
return CORE.Window.screen.height;
}


void *GetWindowHandle(void)
{
#if defined(PLATFORM_DESKTOP) && defined(_WIN32)

return glfwGetWin32Window(CORE.Window.handle);
#elif defined(__linux__)




return NULL; 
#elif defined(__APPLE__)

return NULL; 
#else
return NULL;
#endif
}


int GetMonitorCount(void)
{
#if defined(PLATFORM_DESKTOP)
int monitorCount;
glfwGetMonitors(&monitorCount);
return monitorCount;
#else
return 1;
#endif
}


int GetMonitorWidth(int monitor)
{
#if defined(PLATFORM_DESKTOP)
int monitorCount;
GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

if ((monitor >= 0) && (monitor < monitorCount))
{
const GLFWvidmode *mode = glfwGetVideoMode(monitors[monitor]);
return mode->width;
}
else TRACELOG(LOG_WARNING, "GLFW: Failed to find selected monitor");
#endif
return 0;
}


int GetMonitorHeight(int monitor)
{
#if defined(PLATFORM_DESKTOP)
int monitorCount;
GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

if ((monitor >= 0) && (monitor < monitorCount))
{
const GLFWvidmode *mode = glfwGetVideoMode(monitors[monitor]);
return mode->height;
}
else TRACELOG(LOG_WARNING, "GLFW: Failed to find selected monitor");
#endif
return 0;
}


int GetMonitorPhysicalWidth(int monitor)
{
#if defined(PLATFORM_DESKTOP)
int monitorCount;
GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

if ((monitor >= 0) && (monitor < monitorCount))
{
int physicalWidth;
glfwGetMonitorPhysicalSize(monitors[monitor], &physicalWidth, NULL);
return physicalWidth;
}
else TRACELOG(LOG_WARNING, "GLFW: Failed to find selected monitor");
#endif
return 0;
}


int GetMonitorPhysicalHeight(int monitor)
{
#if defined(PLATFORM_DESKTOP)
int monitorCount;
GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

if ((monitor >= 0) && (monitor < monitorCount))
{
int physicalHeight;
glfwGetMonitorPhysicalSize(monitors[monitor], NULL, &physicalHeight);
return physicalHeight;
}
else TRACELOG(LOG_WARNING, "GLFW: Failed to find selected monitor");
#endif
return 0;
}


Vector2 GetWindowPosition(void)
{
int x = 0;
int y = 0;
#if defined(PLATFORM_DESKTOP)
glfwGetWindowPos(CORE.Window.handle, &x, &y);
#endif
return (Vector2){ (float)x, (float)y };
}


const char *GetMonitorName(int monitor)
{
#if defined(PLATFORM_DESKTOP)
int monitorCount;
GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

if ((monitor >= 0) && (monitor < monitorCount))
{
return glfwGetMonitorName(monitors[monitor]);
}
else TRACELOG(LOG_WARNING, "GLFW: Failed to find selected monitor");
#endif
return "";
}



const char *GetClipboardText(void)
{
#if defined(PLATFORM_DESKTOP)
return glfwGetClipboardString(CORE.Window.handle);
#else
return NULL;
#endif
}


void SetClipboardText(const char *text)
{
#if defined(PLATFORM_DESKTOP)
glfwSetClipboardString(CORE.Window.handle, text);
#endif
}


void ShowCursor(void)
{
#if defined(PLATFORM_DESKTOP)
glfwSetInputMode(CORE.Window.handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
#endif
#if defined(PLATFORM_UWP)
UWPMessage *msg = CreateUWPMessage();
msg->type = UWP_MSG_SHOW_MOUSE;
SendMessageToUWP(msg);
#endif
CORE.Input.Mouse.cursorHidden = false;
}


void HideCursor(void)
{
#if defined(PLATFORM_DESKTOP)
glfwSetInputMode(CORE.Window.handle, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
#endif
#if defined(PLATFORM_UWP)
UWPMessage *msg = CreateUWPMessage();
msg->type = UWP_MSG_HIDE_MOUSE;
SendMessageToUWP(msg);
#endif
CORE.Input.Mouse.cursorHidden = true;
}


bool IsCursorHidden(void)
{
return CORE.Input.Mouse.cursorHidden;
}


void EnableCursor(void)
{
#if defined(PLATFORM_DESKTOP)
glfwSetInputMode(CORE.Window.handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
#endif
#if defined(PLATFORM_WEB)
CORE.Input.Mouse.cursorLockRequired = true;
#endif
#if defined(PLATFORM_UWP)
UWPMessage *msg = CreateUWPMessage();
msg->type = UWP_MSG_LOCK_MOUSE;
SendMessageToUWP(msg);
#endif
CORE.Input.Mouse.cursorHidden = false;
}


void DisableCursor(void)
{
#if defined(PLATFORM_DESKTOP)
glfwSetInputMode(CORE.Window.handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
#endif
#if defined(PLATFORM_WEB)
CORE.Input.Mouse.cursorLockRequired = true;
#endif
#if defined(PLATFORM_UWP)
UWPMessage *msg = CreateUWPMessage();
msg->type = UWP_MSG_UNLOCK_MOUSE;
SendMessageToUWP(msg);
#endif
CORE.Input.Mouse.cursorHidden = true;
}


void ClearBackground(Color color)
{
rlClearColor(color.r, color.g, color.b, color.a); 
rlClearScreenBuffers(); 
}


void BeginDrawing(void)
{
CORE.Time.current = GetTime(); 
CORE.Time.update = CORE.Time.current - CORE.Time.previous;
CORE.Time.previous = CORE.Time.current;

rlLoadIdentity(); 
rlMultMatrixf(MatrixToFloat(CORE.Window.screenScale)); 



}


void EndDrawing(void)
{
#if defined(PLATFORM_RPI) && defined(SUPPORT_MOUSE_CURSOR_RPI)


DrawRectangle(CORE.Input.Mouse.position.x, CORE.Input.Mouse.position.y, 3, 3, MAROON);
#endif

rlglDraw(); 

#if defined(SUPPORT_GIF_RECORDING)
#define GIF_RECORD_FRAMERATE 10

if (gifRecording)
{
gifFramesCounter++;


if ((gifFramesCounter%GIF_RECORD_FRAMERATE) == 0)
{


unsigned char *screenData = rlReadScreenPixels(CORE.Window.screen.width, CORE.Window.screen.height);
GifWriteFrame(screenData, CORE.Window.screen.width, CORE.Window.screen.height, 10, 8, false);

RL_FREE(screenData); 
}

if (((gifFramesCounter/15)%2) == 1)
{
DrawCircle(30, CORE.Window.screen.height - 20, 10, RED);
DrawText("RECORDING", 50, CORE.Window.screen.height - 25, 10, MAROON);
}

rlglDraw(); 
}
#endif

SwapBuffers(); 
PollInputEvents(); 


CORE.Time.current = GetTime();
CORE.Time.draw = CORE.Time.current - CORE.Time.previous;
CORE.Time.previous = CORE.Time.current;

CORE.Time.frame = CORE.Time.update + CORE.Time.draw;


if (CORE.Time.frame < CORE.Time.target)
{
Wait((float)(CORE.Time.target - CORE.Time.frame)*1000.0f);

CORE.Time.current = GetTime();
double waitTime = CORE.Time.current - CORE.Time.previous;
CORE.Time.previous = CORE.Time.current;

CORE.Time.frame += waitTime; 




}
}


void BeginMode2D(Camera2D camera)
{
rlglDraw(); 

rlLoadIdentity(); 


rlMultMatrixf(MatrixToFloat(GetCameraMatrix2D(camera)));


rlMultMatrixf(MatrixToFloat(CORE.Window.screenScale));
}


void EndMode2D(void)
{
rlglDraw(); 

rlLoadIdentity(); 
rlMultMatrixf(MatrixToFloat(CORE.Window.screenScale)); 
}


void BeginMode3D(Camera3D camera)
{
rlglDraw(); 

rlMatrixMode(RL_PROJECTION); 
rlPushMatrix(); 
rlLoadIdentity(); 

float aspect = (float)CORE.Window.currentFbo.width/(float)CORE.Window.currentFbo.height;

if (camera.type == CAMERA_PERSPECTIVE)
{

double top = 0.01*tan(camera.fovy*0.5*DEG2RAD);
double right = top*aspect;

rlFrustum(-right, right, -top, top, DEFAULT_NEAR_CULL_DISTANCE, DEFAULT_FAR_CULL_DISTANCE);
}
else if (camera.type == CAMERA_ORTHOGRAPHIC)
{

double top = camera.fovy/2.0;
double right = top*aspect;

rlOrtho(-right, right, -top,top, DEFAULT_NEAR_CULL_DISTANCE, DEFAULT_FAR_CULL_DISTANCE);
}



rlMatrixMode(RL_MODELVIEW); 
rlLoadIdentity(); 


Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);
rlMultMatrixf(MatrixToFloat(matView)); 

rlEnableDepthTest(); 
}


void EndMode3D(void)
{
rlglDraw(); 

rlMatrixMode(RL_PROJECTION); 
rlPopMatrix(); 

rlMatrixMode(RL_MODELVIEW); 
rlLoadIdentity(); 

rlMultMatrixf(MatrixToFloat(CORE.Window.screenScale)); 

rlDisableDepthTest(); 
}


void BeginTextureMode(RenderTexture2D target)
{
rlglDraw(); 

rlEnableRenderTexture(target.id); 


rlViewport(0, 0, target.texture.width, target.texture.height);

rlMatrixMode(RL_PROJECTION); 
rlLoadIdentity(); 



rlOrtho(0, target.texture.width, target.texture.height, 0, 0.0f, 1.0f);

rlMatrixMode(RL_MODELVIEW); 
rlLoadIdentity(); 





CORE.Window.currentFbo.width = target.texture.width;
CORE.Window.currentFbo.height = target.texture.height;
}


void EndTextureMode(void)
{
rlglDraw(); 

rlDisableRenderTexture(); 


SetupViewport(CORE.Window.render.width, CORE.Window.render.height);


CORE.Window.currentFbo.width = GetScreenWidth();
CORE.Window.currentFbo.height = GetScreenHeight();
}



void BeginScissorMode(int x, int y, int width, int height)
{
rlglDraw(); 

rlEnableScissorTest();
rlScissor(x, GetScreenHeight() - (y + height), width, height);
}


void EndScissorMode(void)
{
rlglDraw(); 
rlDisableScissorTest();
}


Ray GetMouseRay(Vector2 mouse, Camera camera)
{
Ray ray;



float x = (2.0f*mouse.x)/(float)GetScreenWidth() - 1.0f;
float y = 1.0f - (2.0f*mouse.y)/(float)GetScreenHeight();
float z = 1.0f;


Vector3 deviceCoords = { x, y, z };


Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);

Matrix matProj = MatrixIdentity();

if (camera.type == CAMERA_PERSPECTIVE)
{

matProj = MatrixPerspective(camera.fovy*DEG2RAD, ((double)GetScreenWidth()/(double)GetScreenHeight()), DEFAULT_NEAR_CULL_DISTANCE, DEFAULT_FAR_CULL_DISTANCE);
}
else if (camera.type == CAMERA_ORTHOGRAPHIC)
{
float aspect = (float)CORE.Window.screen.width/(float)CORE.Window.screen.height;
double top = camera.fovy/2.0;
double right = top*aspect;


matProj = MatrixOrtho(-right, right, -top, top, 0.01, 1000.0);
}


Vector3 nearPoint = rlUnproject((Vector3){ deviceCoords.x, deviceCoords.y, 0.0f }, matProj, matView);
Vector3 farPoint = rlUnproject((Vector3){ deviceCoords.x, deviceCoords.y, 1.0f }, matProj, matView);




Vector3 cameraPlanePointerPos = rlUnproject((Vector3){ deviceCoords.x, deviceCoords.y, -1.0f }, matProj, matView);


Vector3 direction = Vector3Normalize(Vector3Subtract(farPoint, nearPoint));

if (camera.type == CAMERA_PERSPECTIVE) ray.position = camera.position;
else if (camera.type == CAMERA_ORTHOGRAPHIC) ray.position = cameraPlanePointerPos;


ray.direction = direction;

return ray;
}


Matrix GetCameraMatrix(Camera camera)
{
return MatrixLookAt(camera.position, camera.target, camera.up);
}


Matrix GetCameraMatrix2D(Camera2D camera)
{
Matrix matTransform = { 0 };














Matrix matOrigin = MatrixTranslate(-camera.target.x, -camera.target.y, 0.0f);
Matrix matRotation = MatrixRotate((Vector3){ 0.0f, 0.0f, 1.0f }, camera.rotation*DEG2RAD);
Matrix matScale = MatrixScale(camera.zoom, camera.zoom, 1.0f);
Matrix matTranslation = MatrixTranslate(camera.offset.x, camera.offset.y, 0.0f);

matTransform = MatrixMultiply(MatrixMultiply(matOrigin, MatrixMultiply(matScale, matRotation)), matTranslation);

return matTransform;
}


Vector2 GetWorldToScreen(Vector3 position, Camera camera)
{
Vector2 screenPosition = GetWorldToScreenEx(position, camera, GetScreenWidth(), GetScreenHeight());

return screenPosition;
}


Vector2 GetWorldToScreenEx(Vector3 position, Camera camera, int width, int height)
{

Matrix matProj = MatrixIdentity();

if (camera.type == CAMERA_PERSPECTIVE)
{

matProj = MatrixPerspective(camera.fovy * DEG2RAD, ((double)width/(double)height), DEFAULT_NEAR_CULL_DISTANCE, DEFAULT_FAR_CULL_DISTANCE);
}
else if (camera.type == CAMERA_ORTHOGRAPHIC)
{
float aspect = (float)CORE.Window.screen.width/(float)CORE.Window.screen.height;
double top = camera.fovy/2.0;
double right = top*aspect;


matProj = MatrixOrtho(-right, right, -top, top, DEFAULT_NEAR_CULL_DISTANCE, DEFAULT_FAR_CULL_DISTANCE);
}


Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);


Quaternion worldPos = { position.x, position.y, position.z, 1.0f };


worldPos = QuaternionTransform(worldPos, matView);


worldPos = QuaternionTransform(worldPos, matProj);


Vector3 ndcPos = { worldPos.x/worldPos.w, -worldPos.y/worldPos.w, worldPos.z/worldPos.w };


Vector2 screenPosition = { (ndcPos.x + 1.0f)/2.0f*(float)width, (ndcPos.y + 1.0f)/2.0f*(float)height };

return screenPosition;
}


Vector2 GetWorldToScreen2D(Vector2 position, Camera2D camera)
{
Matrix matCamera = GetCameraMatrix2D(camera);
Vector3 transform = Vector3Transform((Vector3){ position.x, position.y, 0 }, matCamera);

return (Vector2){ transform.x, transform.y };
}


Vector2 GetScreenToWorld2D(Vector2 position, Camera2D camera)
{
Matrix invMatCamera = MatrixInvert(GetCameraMatrix2D(camera));
Vector3 transform = Vector3Transform((Vector3){ position.x, position.y, 0 }, invMatCamera);

return (Vector2){ transform.x, transform.y };
}


void SetTargetFPS(int fps)
{
if (fps < 1) CORE.Time.target = 0.0;
else CORE.Time.target = 1.0/(double)fps;

TRACELOG(LOG_INFO, "TIMER: Target time per frame: %02.03f milliseconds", (float)CORE.Time.target*1000);
}



int GetFPS(void)
{
#define FPS_CAPTURE_FRAMES_COUNT 30 
#define FPS_AVERAGE_TIME_SECONDS 0.5f 
#define FPS_STEP (FPS_AVERAGE_TIME_SECONDS/FPS_CAPTURE_FRAMES_COUNT)

static int index = 0;
static float history[FPS_CAPTURE_FRAMES_COUNT] = { 0 };
static float average = 0, last = 0;
float fpsFrame = GetFrameTime();

if (fpsFrame == 0) return 0;

if ((GetTime() - last) > FPS_STEP)
{
last = GetTime();
index = (index + 1)%FPS_CAPTURE_FRAMES_COUNT;
average -= history[index];
history[index] = fpsFrame/FPS_CAPTURE_FRAMES_COUNT;
average += history[index];
}

return (int)roundf(1.0f/average);
}


float GetFrameTime(void)
{
return (float)CORE.Time.frame;
}




double GetTime(void)
{
#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
return glfwGetTime(); 
#endif

#if defined(PLATFORM_ANDROID) || defined(PLATFORM_RPI)
struct timespec ts;
clock_gettime(CLOCK_MONOTONIC, &ts);
unsigned long long int time = (unsigned long long int)ts.tv_sec*1000000000LLU + (unsigned long long int)ts.tv_nsec;

return (double)(time - CORE.Time.base)*1e-9; 
#endif

#if defined(PLATFORM_UWP)

return CORE.Time.current;
#endif
}


int ColorToInt(Color color)
{
return (((int)color.r << 24) | ((int)color.g << 16) | ((int)color.b << 8) | (int)color.a);
}


Vector4 ColorNormalize(Color color)
{
Vector4 result;

result.x = (float)color.r/255.0f;
result.y = (float)color.g/255.0f;
result.z = (float)color.b/255.0f;
result.w = (float)color.a/255.0f;

return result;
}


Color ColorFromNormalized(Vector4 normalized)
{
Color result;

result.r = normalized.x*255.0f;
result.g = normalized.y*255.0f;
result.b = normalized.z*255.0f;
result.a = normalized.w*255.0f;

return result;
}



Vector3 ColorToHSV(Color color)
{
Vector3 hsv = { 0 };
Vector3 rgb = { (float)color.r/255.0f, (float)color.g/255.0f, (float)color.b/255.0f };
float min, max, delta;

min = rgb.x < rgb.y? rgb.x : rgb.y;
min = min < rgb.z? min : rgb.z;

max = rgb.x > rgb.y? rgb.x : rgb.y;
max = max > rgb.z? max : rgb.z;

hsv.z = max; 
delta = max - min;

if (delta < 0.00001f)
{
hsv.y = 0.0f;
hsv.x = 0.0f; 
return hsv;
}

if (max > 0.0f)
{

hsv.y = (delta/max); 
}
else
{

hsv.y = 0.0f;
hsv.x = NAN; 
return hsv;
}


if (rgb.x >= max) hsv.x = (rgb.y - rgb.z)/delta; 
else
{
if (rgb.y >= max) hsv.x = 2.0f + (rgb.z - rgb.x)/delta; 
else hsv.x = 4.0f + (rgb.x - rgb.y)/delta; 
}

hsv.x *= 60.0f; 

if (hsv.x < 0.0f) hsv.x += 360.0f;

return hsv;
}




Color ColorFromHSV(Vector3 hsv)
{
Color color = { 0, 0, 0, 255 };
float h = hsv.x, s = hsv.y, v = hsv.z;


float k = fmod((5.0f + h/60.0f), 6);
float t = 4.0f - k;
k = (t < k)? t : k;
k = (k < 1)? k : 1;
k = (k > 0)? k : 0;
color.r = (v - v*s*k)*255;


k = fmod((3.0f + h/60.0f), 6);
t = 4.0f - k;
k = (t < k)? t : k;
k = (k < 1)? k : 1;
k = (k > 0)? k : 0;
color.g = (v - v*s*k)*255;


k = fmod((1.0f + h/60.0f), 6);
t = 4.0f - k;
k = (t < k)? t : k;
k = (k < 1)? k : 1;
k = (k > 0)? k : 0;
color.b = (v - v*s*k)*255;

return color;
}


Color GetColor(int hexValue)
{
Color color;

color.r = (unsigned char)(hexValue >> 24) & 0xFF;
color.g = (unsigned char)(hexValue >> 16) & 0xFF;
color.b = (unsigned char)(hexValue >> 8) & 0xFF;
color.a = (unsigned char)hexValue & 0xFF;

return color;
}


int GetRandomValue(int min, int max)
{
if (min > max)
{
int tmp = max;
max = min;
min = tmp;
}

return (rand()%(abs(max - min) + 1) + min);
}


Color Fade(Color color, float alpha)
{
if (alpha < 0.0f) alpha = 0.0f;
else if (alpha > 1.0f) alpha = 1.0f;

return (Color){color.r, color.g, color.b, (unsigned char)(255.0f*alpha)};
}


void SetConfigFlags(unsigned int flags)
{
CORE.Window.flags = flags;

if (CORE.Window.flags & FLAG_FULLSCREEN_MODE) CORE.Window.fullscreen = true;
if (CORE.Window.flags & FLAG_WINDOW_ALWAYS_RUN) CORE.Window.alwaysRun = true;
}






void TakeScreenshot(const char *fileName)
{
unsigned char *imgData = rlReadScreenPixels(CORE.Window.render.width, CORE.Window.render.height);
Image image = { imgData, CORE.Window.render.width, CORE.Window.render.height, 1, UNCOMPRESSED_R8G8B8A8 };

char path[512] = { 0 };
#if defined(PLATFORM_ANDROID)
strcpy(path, CORE.Android.internalDataPath);
strcat(path, "/");
strcat(path, fileName);
#else
strcpy(path, fileName);
#endif

ExportImage(image, path);
RL_FREE(imgData);

#if defined(PLATFORM_WEB)


emscripten_run_script(TextFormat("saveFileFromMEMFSToDisk('%s','%s')", GetFileName(path), GetFileName(path)));
#endif


TRACELOG(LOG_INFO, "SYSTEM: [%s] Screenshot taken successfully", path);
}


bool FileExists(const char *fileName)
{
bool result = false;

#if defined(_WIN32)
if (_access(fileName, 0) != -1) result = true;
#else
if (access(fileName, F_OK) != -1) result = true;
#endif

return result;
}



bool IsFileExtension(const char *fileName, const char *ext)
{
bool result = false;
const char *fileExt = GetExtension(fileName);

if (fileExt != NULL)
{
int extCount = 0;
const char **checkExts = TextSplit(ext, ';', &extCount);

char fileExtLower[16] = { 0 };
strcpy(fileExtLower, TextToLower(fileExt));

for (int i = 0; i < extCount; i++)
{
if (TextIsEqual(fileExtLower, TextToLower(checkExts[i] + 1)))
{
result = true;
break;
}
}
}

return result;
}


bool DirectoryExists(const char *dirPath)
{
bool result = false;
DIR *dir = opendir(dirPath);

if (dir != NULL)
{
result = true;
closedir(dir);
}

return result;
}


const char *GetExtension(const char *fileName)
{
const char *dot = strrchr(fileName, '.');

if (!dot || dot == fileName) return NULL;

return (dot + 1);
}


static const char *strprbrk(const char *s, const char *charset)
{
const char *latestMatch = NULL;
for (; s = strpbrk(s, charset), s != NULL; latestMatch = s++) { }
return latestMatch;
}


const char *GetFileName(const char *filePath)
{
const char *fileName = NULL;
if (filePath != NULL) fileName = strprbrk(filePath, "\\/");

if (!fileName || (fileName == filePath)) return filePath;

return fileName + 1;
}


const char *GetFileNameWithoutExt(const char *filePath)
{
#define MAX_FILENAMEWITHOUTEXT_LENGTH 128

static char fileName[MAX_FILENAMEWITHOUTEXT_LENGTH];
memset(fileName, 0, MAX_FILENAMEWITHOUTEXT_LENGTH);

if (filePath != NULL) strcpy(fileName, GetFileName(filePath)); 

int len = strlen(fileName);

for (int i = 0; (i < len) && (i < MAX_FILENAMEWITHOUTEXT_LENGTH); i++)
{
if (fileName[i] == '.')
{

fileName[i] = '\0';
break;
}
}

return fileName;
}


const char *GetDirectoryPath(const char *filePath)
{









const char *lastSlash = NULL;
static char dirPath[MAX_FILEPATH_LENGTH];
memset(dirPath, 0, MAX_FILEPATH_LENGTH);



if (filePath[1] != ':')
{


dirPath[0] = '.';
dirPath[1] = '/';
}

lastSlash = strprbrk(filePath, "\\/");
if (lastSlash)
{

strncpy(dirPath + ((filePath[1] != ':')? 2 : 0), filePath, strlen(filePath) - (strlen(lastSlash) - 1));
dirPath[strlen(filePath) - strlen(lastSlash) + ((filePath[1] != ':')? 2 : 0)] = '\0'; 
}

return dirPath;
}


const char *GetPrevDirectoryPath(const char *dirPath)
{
static char prevDirPath[MAX_FILEPATH_LENGTH];
memset(prevDirPath, 0, MAX_FILEPATH_LENGTH);
int pathLen = strlen(dirPath);

if (pathLen <= 3) strcpy(prevDirPath, dirPath);

for (int i = (pathLen - 1); (i > 0) && (pathLen > 3); i--)
{
if ((dirPath[i] == '\\') || (dirPath[i] == '/'))
{
if (i == 2) i++; 
strncpy(prevDirPath, dirPath, i);
break;
}
}

return prevDirPath;
}


const char *GetWorkingDirectory(void)
{
static char currentDir[MAX_FILEPATH_LENGTH];
memset(currentDir, 0, MAX_FILEPATH_LENGTH);

GETCWD(currentDir, MAX_FILEPATH_LENGTH - 1);

return currentDir;
}



char **GetDirectoryFiles(const char *dirPath, int *fileCount)
{
#define MAX_DIRECTORY_FILES 512

ClearDirectoryFiles();


dirFilesPath = (char **)RL_MALLOC(sizeof(char *)*MAX_DIRECTORY_FILES);
for (int i = 0; i < MAX_DIRECTORY_FILES; i++) dirFilesPath[i] = (char *)RL_MALLOC(sizeof(char)*MAX_FILEPATH_LENGTH);

int counter = 0;
struct dirent *entity;
DIR *dir = opendir(dirPath);

if (dir != NULL) 
{




while ((entity = readdir(dir)) != NULL)
{
strcpy(dirFilesPath[counter], entity->d_name);
counter++;
}

closedir(dir);
}
else TRACELOG(LOG_WARNING, "FILEIO: Failed to open requested directory"); 

dirFilesCount = counter;
*fileCount = dirFilesCount;

return dirFilesPath;
}


void ClearDirectoryFiles(void)
{
if (dirFilesCount > 0)
{
for (int i = 0; i < MAX_DIRECTORY_FILES; i++) RL_FREE(dirFilesPath[i]);

RL_FREE(dirFilesPath);
}

dirFilesCount = 0;
}


bool ChangeDirectory(const char *dir)
{
return (CHDIR(dir) == 0);
}


bool IsFileDropped(void)
{
if (CORE.Window.dropFilesCount > 0) return true;
else return false;
}


char **GetDroppedFiles(int *count)
{
*count = CORE.Window.dropFilesCount;
return CORE.Window.dropFilesPath;
}


void ClearDroppedFiles(void)
{
if (CORE.Window.dropFilesCount > 0)
{
for (int i = 0; i < CORE.Window.dropFilesCount; i++) RL_FREE(CORE.Window.dropFilesPath[i]);

RL_FREE(CORE.Window.dropFilesPath);

CORE.Window.dropFilesCount = 0;
}
}


long GetFileModTime(const char *fileName)
{
struct stat result = { 0 };

if (stat(fileName, &result) == 0)
{
time_t mod = result.st_mtime;

return (long)mod;
}

return 0;
}


unsigned char *CompressData(unsigned char *data, int dataLength, int *compDataLength)
{
#define COMPRESSION_QUALITY_DEFLATE 8

unsigned char *compData = NULL;

#if defined(SUPPORT_COMPRESSION_API)
compData = stbi_zlib_compress(data, dataLength, compDataLength, COMPRESSION_QUALITY_DEFLATE);
#endif

return compData;
}


unsigned char *DecompressData(unsigned char *compData, int compDataLength, int *dataLength)
{
char *data = NULL;

#if defined(SUPPORT_COMPRESSION_API)
data = stbi_zlib_decode_malloc((char *)compData, compDataLength, dataLength);
#endif

return (unsigned char *)data;
}



void SaveStorageValue(unsigned int position, int value)
{
#if defined(SUPPORT_DATA_STORAGE)
char path[512] = { 0 };
#if defined(PLATFORM_ANDROID)
strcpy(path, CORE.Android.internalDataPath);
strcat(path, "/");
strcat(path, STORAGE_DATA_FILE);
#else
strcpy(path, STORAGE_DATA_FILE);
#endif

unsigned int dataSize = 0;
unsigned int newDataSize = 0;
unsigned char *fileData = LoadFileData(path, &dataSize);
unsigned char *newFileData = NULL;

if (fileData != NULL)
{
if (dataSize <= (position*sizeof(int)))
{

newDataSize = (position + 1)*sizeof(int);
newFileData = (unsigned char *)RL_REALLOC(fileData, newDataSize);

if (newFileData != NULL)
{

int *dataPtr = (int *)newFileData;
dataPtr[position] = value; 
}
else
{

TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to realloc data (%u), position in bytes (%u) bigger than actual file size", path, dataSize, position*sizeof(int)); 


newFileData = fileData;
newDataSize = dataSize;
}
}
else
{

newFileData = fileData;
newDataSize = dataSize;


int *dataPtr = (int *)newFileData;
dataPtr[position] = value;
}

SaveFileData(path, newFileData, newDataSize);
RL_FREE(newFileData);
}
else
{
TRACELOG(LOG_INFO, "FILEIO: [%s] File not found, creating it", path);

dataSize = (position + 1)*sizeof(int);
fileData = (unsigned char *)RL_MALLOC(dataSize);
int *dataPtr = (int *)fileData;
dataPtr[position] = value;

SaveFileData(path, fileData, dataSize);
RL_FREE(fileData);
}
#endif
}



int LoadStorageValue(unsigned int position)
{
int value = 0;
#if defined(SUPPORT_DATA_STORAGE)
char path[512] = { 0 };
#if defined(PLATFORM_ANDROID)
strcpy(path, CORE.Android.internalDataPath);
strcat(path, "/");
strcat(path, STORAGE_DATA_FILE);
#else
strcpy(path, STORAGE_DATA_FILE);
#endif

unsigned int dataSize = 0;
unsigned char *fileData = LoadFileData(path, &dataSize);

if (fileData != NULL)
{
if (dataSize < (position*4)) TRACELOG(LOG_WARNING, "SYSTEM: Failed to find storage position");
else
{
int *dataPtr = (int *)fileData;
value = dataPtr[position];
}

RL_FREE(fileData);
}
#endif
return value;
}






void OpenURL(const char *url)
{


if (strchr(url, '\'') != NULL)
{
TRACELOG(LOG_WARNING, "SYSTEM: Provided URL is not valid");
}
else
{
#if defined(PLATFORM_DESKTOP)
char *cmd = (char *)RL_CALLOC(strlen(url) + 10, sizeof(char));
#if defined(_WIN32)
sprintf(cmd, "explorer %s", url);
#elif defined(__linux__)
sprintf(cmd, "xdg-open '%s'", url); 
#elif defined(__APPLE__)
sprintf(cmd, "open '%s'", url);
#endif
system(cmd);
RL_FREE(cmd);
#endif
#if defined(PLATFORM_WEB)
emscripten_run_script(TextFormat("window.open('%s', '_blank')", url));
#endif
}
}





bool IsKeyPressed(int key)
{
bool pressed = false;

if ((CORE.Input.Keyboard.previousKeyState[key] == 0) && (CORE.Input.Keyboard.currentKeyState[key] == 1)) pressed = true;
else pressed = false;

return pressed;
}


bool IsKeyDown(int key)
{
if (CORE.Input.Keyboard.currentKeyState[key] == 1) return true;
else return false;
}


bool IsKeyReleased(int key)
{
bool released = false;

if ((CORE.Input.Keyboard.previousKeyState[key] == 1) && (CORE.Input.Keyboard.currentKeyState[key] == 0)) released = true;
else released = false;

return released;
}


bool IsKeyUp(int key)
{
if (CORE.Input.Keyboard.currentKeyState[key] == 0) return true;
else return false;
}


int GetKeyPressed(void)
{
int value = 0;

if (CORE.Input.Keyboard.keyPressedQueueCount > 0)
{

value = CORE.Input.Keyboard.keyPressedQueue[0];


for (int i = 0; i < (CORE.Input.Keyboard.keyPressedQueueCount - 1); i++) CORE.Input.Keyboard.keyPressedQueue[i] = CORE.Input.Keyboard.keyPressedQueue[i + 1];


CORE.Input.Keyboard.keyPressedQueue[CORE.Input.Keyboard.keyPressedQueueCount] = 0;
CORE.Input.Keyboard.keyPressedQueueCount--;
}

return value;
}



void SetExitKey(int key)
{
#if !defined(PLATFORM_ANDROID)
CORE.Input.Keyboard.exitKey = key;
#endif
}




bool IsGamepadAvailable(int gamepad)
{
bool result = false;

#if !defined(PLATFORM_ANDROID)
if ((gamepad < MAX_GAMEPADS) && CORE.Input.Gamepad.ready[gamepad]) result = true;
#endif

return result;
}


bool IsGamepadName(int gamepad, const char *name)
{
bool result = false;

#if !defined(PLATFORM_ANDROID)
const char *currentName = NULL;

if (CORE.Input.Gamepad.ready[gamepad]) currentName = GetGamepadName(gamepad);
if ((name != NULL) && (currentName != NULL)) result = (strcmp(name, currentName) == 0);
#endif

return result;
}


const char *GetGamepadName(int gamepad)
{
#if defined(PLATFORM_DESKTOP)
if (CORE.Input.Gamepad.ready[gamepad]) return glfwGetJoystickName(gamepad);
else return NULL;
#elif defined(PLATFORM_RPI)
if (CORE.Input.Gamepad.ready[gamepad]) ioctl(CORE.Input.Gamepad.streamId[gamepad], JSIOCGNAME(64), &CORE.Input.Gamepad.name);

return CORE.Input.Gamepad.name;
#else
return NULL;
#endif
}


int GetGamepadAxisCount(int gamepad)
{
#if defined(PLATFORM_RPI)
int axisCount = 0;
if (CORE.Input.Gamepad.ready[gamepad]) ioctl(CORE.Input.Gamepad.streamId[gamepad], JSIOCGAXES, &axisCount);
CORE.Input.Gamepad.axisCount = axisCount;
#endif
return CORE.Input.Gamepad.axisCount;
}


float GetGamepadAxisMovement(int gamepad, int axis)
{
float value = 0;

#if !defined(PLATFORM_ANDROID)
if ((gamepad < MAX_GAMEPADS) && CORE.Input.Gamepad.ready[gamepad] && (axis < MAX_GAMEPAD_AXIS)) value = CORE.Input.Gamepad.axisState[gamepad][axis];
#endif

return value;
}


bool IsGamepadButtonPressed(int gamepad, int button)
{
bool pressed = false;

#if !defined(PLATFORM_ANDROID)
if ((gamepad < MAX_GAMEPADS) && CORE.Input.Gamepad.ready[gamepad] && (button < MAX_GAMEPAD_BUTTONS) &&
(CORE.Input.Gamepad.currentState[gamepad][button] != CORE.Input.Gamepad.previousState[gamepad][button]) &&
(CORE.Input.Gamepad.currentState[gamepad][button] == 1)) pressed = true;
#endif

return pressed;
}


bool IsGamepadButtonDown(int gamepad, int button)
{
bool result = false;

#if !defined(PLATFORM_ANDROID)
if ((gamepad < MAX_GAMEPADS) && CORE.Input.Gamepad.ready[gamepad] && (button < MAX_GAMEPAD_BUTTONS) &&
(CORE.Input.Gamepad.currentState[gamepad][button] == 1)) result = true;
#endif

return result;
}


bool IsGamepadButtonReleased(int gamepad, int button)
{
bool released = false;

#if !defined(PLATFORM_ANDROID)
if ((gamepad < MAX_GAMEPADS) && CORE.Input.Gamepad.ready[gamepad] && (button < MAX_GAMEPAD_BUTTONS) &&
(CORE.Input.Gamepad.currentState[gamepad][button] != CORE.Input.Gamepad.previousState[gamepad][button]) &&
(CORE.Input.Gamepad.currentState[gamepad][button] == 0)) released = true;
#endif

return released;
}


bool IsGamepadButtonUp(int gamepad, int button)
{
bool result = false;

#if !defined(PLATFORM_ANDROID)
if ((gamepad < MAX_GAMEPADS) && CORE.Input.Gamepad.ready[gamepad] && (button < MAX_GAMEPAD_BUTTONS) &&
(CORE.Input.Gamepad.currentState[gamepad][button] == 0)) result = true;
#endif

return result;
}


int GetGamepadButtonPressed(void)
{
return CORE.Input.Gamepad.lastButtonPressed;
}


bool IsMouseButtonPressed(int button)
{
bool pressed = false;

if ((CORE.Input.Mouse.currentButtonState[button] == 1) && (CORE.Input.Mouse.previousButtonState[button] == 0)) pressed = true;


if ((CORE.Input.Touch.currentTouchState[button] == 1) && (CORE.Input.Touch.previousTouchState[button] == 0)) pressed = true;

return pressed;
}


bool IsMouseButtonDown(int button)
{
bool down = false;

if (CORE.Input.Mouse.currentButtonState[button] == 1) down = true;


if (CORE.Input.Touch.currentTouchState[button] == 1) down = true;

return down;
}


bool IsMouseButtonReleased(int button)
{
bool released = false;

if ((CORE.Input.Mouse.currentButtonState[button] == 0) && (CORE.Input.Mouse.previousButtonState[button] == 1)) released = true;


if ((CORE.Input.Touch.currentTouchState[button] == 0) && (CORE.Input.Touch.previousTouchState[button] == 1)) released = true;

return released;
}


bool IsMouseButtonUp(int button)
{
return !IsMouseButtonDown(button);
}


int GetMouseX(void)
{
#if defined(PLATFORM_ANDROID)
return (int)CORE.Input.Touch.position[0].x;
#else
return (int)((CORE.Input.Mouse.position.x + CORE.Input.Mouse.offset.x)*CORE.Input.Mouse.scale.x);
#endif
}


int GetMouseY(void)
{
#if defined(PLATFORM_ANDROID)
return (int)CORE.Input.Touch.position[0].y;
#else
return (int)((CORE.Input.Mouse.position.y + CORE.Input.Mouse.offset.y)*CORE.Input.Mouse.scale.y);
#endif
}


Vector2 GetMousePosition(void)
{
Vector2 position = { 0 };

#if defined(PLATFORM_ANDROID) || defined(PLATFORM_WEB)
position = GetTouchPosition(0);
#else
position.x = (CORE.Input.Mouse.position.x + CORE.Input.Mouse.offset.x)*CORE.Input.Mouse.scale.x;
position.y = (CORE.Input.Mouse.position.y + CORE.Input.Mouse.offset.y)*CORE.Input.Mouse.scale.y;
#endif

return position;
}


void SetMousePosition(int x, int y)
{
CORE.Input.Mouse.position = (Vector2){ (float)x, (float)y };
#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)

glfwSetCursorPos(CORE.Window.handle, CORE.Input.Mouse.position.x, CORE.Input.Mouse.position.y);
#endif
#if defined(PLATFORM_UWP)
UWPMessage *msg = CreateUWPMessage();
msg->type = UWP_MSG_SET_MOUSE_LOCATION;
msg->paramVector0.x = CORE.Input.Mouse.position.x;
msg->paramVector0.y = CORE.Input.Mouse.position.y;
SendMessageToUWP(msg);
#endif
}



void SetMouseOffset(int offsetX, int offsetY)
{
CORE.Input.Mouse.offset = (Vector2){ (float)offsetX, (float)offsetY };
}



void SetMouseScale(float scaleX, float scaleY)
{
CORE.Input.Mouse.scale = (Vector2){ scaleX, scaleY };
}


int GetMouseWheelMove(void)
{
#if defined(PLATFORM_ANDROID)
return 0;
#elif defined(PLATFORM_WEB)
return CORE.Input.Mouse.previousWheelMove/100;
#else
return CORE.Input.Mouse.previousWheelMove;
#endif
}


int GetTouchX(void)
{
#if defined(PLATFORM_ANDROID) || defined(PLATFORM_WEB)
return (int)CORE.Input.Touch.position[0].x;
#else 
return GetMouseX();
#endif
}


int GetTouchY(void)
{
#if defined(PLATFORM_ANDROID) || defined(PLATFORM_WEB)
return (int)CORE.Input.Touch.position[0].y;
#else 
return GetMouseY();
#endif
}



Vector2 GetTouchPosition(int index)
{
Vector2 position = { -1.0f, -1.0f };

#if defined(PLATFORM_ANDROID) || defined(PLATFORM_WEB) || defined(PLATFORM_RPI)
if (index < MAX_TOUCH_POINTS) position = CORE.Input.Touch.position[index];
else TRACELOG(LOG_WARNING, "INPUT: Required touch point out of range (Max touch points: %i)", MAX_TOUCH_POINTS);

#if defined(PLATFORM_ANDROID)
if ((CORE.Window.screen.width > CORE.Window.display.width) || (CORE.Window.screen.height > CORE.Window.display.height))
{
position.x = position.x*((float)CORE.Window.screen.width/(float)(CORE.Window.display.width - CORE.Window.renderOffset.x)) - CORE.Window.renderOffset.x/2;
position.y = position.y*((float)CORE.Window.screen.height/(float)(CORE.Window.display.height - CORE.Window.renderOffset.y)) - CORE.Window.renderOffset.y/2;
}
else
{
position.x = position.x*((float)CORE.Window.render.width/(float)CORE.Window.display.width) - CORE.Window.renderOffset.x/2;
position.y = position.y*((float)CORE.Window.render.height/(float)CORE.Window.display.height) - CORE.Window.renderOffset.y/2;
}
#endif

#elif defined(PLATFORM_DESKTOP)



if (index == 0) position = GetMousePosition();
#endif

return position;
}









static bool InitGraphicsDevice(int width, int height)
{
CORE.Window.screen.width = width; 
CORE.Window.screen.height = height; 

CORE.Window.screenScale = MatrixIdentity(); 




#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
glfwSetErrorCallback(ErrorCallback);

#if defined(__APPLE__)
glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE);
#endif

if (!glfwInit())
{
TRACELOG(LOG_WARNING, "GLFW: Failed to initialize GLFW");
return false;
}


#if defined(PLATFORM_DESKTOP)

GLFWmonitor *monitor = glfwGetPrimaryMonitor();
if (!monitor)
{
TRACELOG(LOG_WARNING, "GLFW: Failed to get primary monitor");
return false;
}
const GLFWvidmode *mode = glfwGetVideoMode(monitor);

CORE.Window.display.width = mode->width;
CORE.Window.display.height = mode->height;


if (CORE.Window.screen.width <= 0) CORE.Window.screen.width = CORE.Window.display.width;
if (CORE.Window.screen.height <= 0) CORE.Window.screen.height = CORE.Window.display.height;
#endif 

#if defined(PLATFORM_WEB)
CORE.Window.display.width = CORE.Window.screen.width;
CORE.Window.display.height = CORE.Window.screen.height;
#endif 

glfwDefaultWindowHints(); 








#if defined(PLATFORM_DESKTOP) && defined(SUPPORT_HIGH_DPI)



glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE); 
#endif


if (CORE.Window.flags & FLAG_WINDOW_HIDDEN) glfwWindowHint(GLFW_VISIBLE, GL_FALSE); 
else glfwWindowHint(GLFW_VISIBLE, GL_TRUE); 

if (CORE.Window.flags & FLAG_WINDOW_RESIZABLE) glfwWindowHint(GLFW_RESIZABLE, GL_TRUE); 
else glfwWindowHint(GLFW_RESIZABLE, GL_FALSE); 

if (CORE.Window.flags & FLAG_WINDOW_UNDECORATED) glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); 
else glfwWindowHint(GLFW_DECORATED, GLFW_TRUE); 

#if defined(GLFW_TRANSPARENT_FRAMEBUFFER)
if (CORE.Window.flags & FLAG_WINDOW_TRANSPARENT) glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE); 
else glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE); 
#endif

if (CORE.Window.flags & FLAG_MSAA_4X_HINT) glfwWindowHint(GLFW_SAMPLES, 4); 






if (rlGetVersion() == OPENGL_21)
{
glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2); 
glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1); 
}
else if (rlGetVersion() == OPENGL_33)
{
glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); 
glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); 
glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 

#if defined(__APPLE__)
glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE); 
#else
glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_FALSE); 
#endif

}
else if (rlGetVersion() == OPENGL_ES_20) 
{
glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#if defined(PLATFORM_DESKTOP)
glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
#else
glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
#endif
}

if (CORE.Window.fullscreen)
{

CORE.Window.position.x = CORE.Window.display.width/2 - CORE.Window.screen.width/2;
CORE.Window.position.y = CORE.Window.display.height/2 - CORE.Window.screen.height/2;

if (CORE.Window.position.x < 0) CORE.Window.position.x = 0;
if (CORE.Window.position.y < 0) CORE.Window.position.y = 0;


int count = 0;
const GLFWvidmode *modes = glfwGetVideoModes(glfwGetPrimaryMonitor(), &count);


for (int i = 0; i < count; i++)
{
if (modes[i].width >= CORE.Window.screen.width)
{
if (modes[i].height >= CORE.Window.screen.height)
{
CORE.Window.display.width = modes[i].width;
CORE.Window.display.height = modes[i].height;
break;
}
}
}

#if defined(PLATFORM_DESKTOP)

if ((CORE.Window.screen.height == CORE.Window.display.height) && (CORE.Window.screen.width == CORE.Window.display.width))
{
glfwWindowHint(GLFW_AUTO_ICONIFY, 0);
}
#endif

TRACELOG(LOG_WARNING, "SYSTEM: Closest fullscreen videomode: %i x %i", CORE.Window.display.width, CORE.Window.display.height);











SetupFramebuffer(CORE.Window.display.width, CORE.Window.display.height);

CORE.Window.handle = glfwCreateWindow(CORE.Window.display.width, CORE.Window.display.height, CORE.Window.title, glfwGetPrimaryMonitor(), NULL);



}
else
{

CORE.Window.handle = glfwCreateWindow(CORE.Window.screen.width, CORE.Window.screen.height, CORE.Window.title, NULL, NULL);

if (CORE.Window.handle)
{
#if defined(PLATFORM_DESKTOP)

int windowPosX = CORE.Window.display.width/2 - CORE.Window.screen.width/2;
int windowPosY = CORE.Window.display.height/2 - CORE.Window.screen.height/2;

if (windowPosX < 0) windowPosX = 0;
if (windowPosY < 0) windowPosY = 0;

glfwSetWindowPos(CORE.Window.handle, windowPosX, windowPosY);
#endif
CORE.Window.render.width = CORE.Window.screen.width;
CORE.Window.render.height = CORE.Window.screen.height;
}
}

if (!CORE.Window.handle)
{
glfwTerminate();
TRACELOG(LOG_WARNING, "GLFW: Failed to initialize Window");
return false;
}
else
{
TRACELOG(LOG_INFO, "DISPLAY: Device initialized successfully");
#if defined(PLATFORM_DESKTOP)
TRACELOG(LOG_INFO, " > Display size: %i x %i", CORE.Window.display.width, CORE.Window.display.height);
#endif
TRACELOG(LOG_INFO, " > Render size: %i x %i", CORE.Window.render.width, CORE.Window.render.height);
TRACELOG(LOG_INFO, " > Screen size: %i x %i", CORE.Window.screen.width, CORE.Window.screen.height);
TRACELOG(LOG_INFO, " > Viewport offsets: %i, %i", CORE.Window.renderOffset.x, CORE.Window.renderOffset.y);
}

glfwSetWindowSizeCallback(CORE.Window.handle, WindowSizeCallback); 
glfwSetCursorEnterCallback(CORE.Window.handle, CursorEnterCallback);
glfwSetKeyCallback(CORE.Window.handle, KeyCallback);
glfwSetMouseButtonCallback(CORE.Window.handle, MouseButtonCallback);
glfwSetCursorPosCallback(CORE.Window.handle, MouseCursorPosCallback); 
glfwSetCharCallback(CORE.Window.handle, CharCallback);
glfwSetScrollCallback(CORE.Window.handle, ScrollCallback);
glfwSetWindowIconifyCallback(CORE.Window.handle, WindowIconifyCallback);
glfwSetDropCallback(CORE.Window.handle, WindowDropCallback);

glfwMakeContextCurrent(CORE.Window.handle);

#if !defined(PLATFORM_WEB)
glfwSwapInterval(0); 
#endif

#if defined(PLATFORM_DESKTOP)


rlLoadExtensions(glfwGetProcAddress);
#endif



if (CORE.Window.flags & FLAG_VSYNC_HINT)
{

glfwSwapInterval(1);
TRACELOG(LOG_INFO, "DISPLAY: Trying to enable VSYNC");
}
#endif 

#if defined(PLATFORM_ANDROID) || defined(PLATFORM_RPI) || defined(PLATFORM_UWP)
CORE.Window.fullscreen = true;


if (CORE.Window.screen.width <= 0) CORE.Window.screen.width = CORE.Window.display.width;
if (CORE.Window.screen.height <= 0) CORE.Window.screen.height = CORE.Window.display.height;

#if defined(PLATFORM_RPI)
bcm_host_init();

DISPMANX_ELEMENT_HANDLE_T dispmanElement;
DISPMANX_DISPLAY_HANDLE_T dispmanDisplay;
DISPMANX_UPDATE_HANDLE_T dispmanUpdate;

VC_RECT_T dstRect;
VC_RECT_T srcRect;
#endif

EGLint samples = 0;
EGLint sampleBuffer = 0;
if (CORE.Window.flags & FLAG_MSAA_4X_HINT)
{
samples = 4;
sampleBuffer = 1;
TRACELOG(LOG_INFO, "DISPLAY: Trying to enable MSAA x4");
}

const EGLint framebufferAttribs[] =
{
EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, 

EGL_RED_SIZE, 8, 
EGL_GREEN_SIZE, 8, 
EGL_BLUE_SIZE, 8, 


EGL_DEPTH_SIZE, 16, 

EGL_SAMPLE_BUFFERS, sampleBuffer, 
EGL_SAMPLES, samples, 
EGL_NONE
};

const EGLint contextAttribs[] =
{
EGL_CONTEXT_CLIENT_VERSION, 2,
EGL_NONE
};

#if defined(PLATFORM_UWP)
const EGLint surfaceAttributes[] =
{


EGL_ANGLE_SURFACE_RENDER_TO_BACK_BUFFER, EGL_TRUE,
EGL_NONE
};

const EGLint defaultDisplayAttributes[] =
{


EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,



EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER, EGL_TRUE,




EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE, EGL_TRUE,
EGL_NONE,
};

const EGLint fl9_3DisplayAttributes[] =
{


EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE, 9,
EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE, 3,
EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER, EGL_TRUE,
EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE, EGL_TRUE,
EGL_NONE,
};

const EGLint warpDisplayAttributes[] =
{


EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_DEVICE_TYPE_WARP_ANGLE,
EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER, EGL_TRUE,
EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE, EGL_TRUE,
EGL_NONE,
};


PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT = (PFNEGLGETPLATFORMDISPLAYEXTPROC)(eglGetProcAddress("eglGetPlatformDisplayEXT"));
if (!eglGetPlatformDisplayEXT)
{
TRACELOG(LOG_WARNING, "DISPLAY: Failed to get function eglGetPlatformDisplayEXT");
return false;
}












CORE.Window.device = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, EGL_DEFAULT_DISPLAY, defaultDisplayAttributes);
if (CORE.Window.device == EGL_NO_DISPLAY)
{
TRACELOG(LOG_WARNING, "DISPLAY: Failed to initialize EGL device");
return false;
}

if (eglInitialize(CORE.Window.device, NULL, NULL) == EGL_FALSE)
{

CORE.Window.device = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, EGL_DEFAULT_DISPLAY, fl9_3DisplayAttributes);
if (CORE.Window.device == EGL_NO_DISPLAY)
{
TRACELOG(LOG_WARNING, "DISPLAY: Failed to initialize EGL device");
return false;
}

if (eglInitialize(CORE.Window.device, NULL, NULL) == EGL_FALSE)
{

CORE.Window.device = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, EGL_DEFAULT_DISPLAY, warpDisplayAttributes);
if (CORE.Window.device == EGL_NO_DISPLAY)
{
TRACELOG(LOG_WARNING, "DISPLAY: Failed to initialize EGL device");
return false;
}

if (eglInitialize(CORE.Window.device, NULL, NULL) == EGL_FALSE)
{

TRACELOG(LOG_WARNING, "DISPLAY: Failed to initialize EGL device");
return false;
}
}
}

EGLint numConfigs = 0;
if ((eglChooseConfig(CORE.Window.device, framebufferAttribs, &CORE.Window.config, 1, &numConfigs) == EGL_FALSE) || (numConfigs == 0))
{
TRACELOG(LOG_WARNING, "DISPLAY: Failed to choose first EGL configuration");
return false;
}
































CORE.Window.surface = eglCreateWindowSurface(CORE.Window.device, CORE.Window.config, handle, surfaceAttributes);
if (CORE.Window.surface == EGL_NO_SURFACE)
{
TRACELOG(LOG_WARNING, "DISPLAY: Failed to create EGL fullscreen surface");
return false;
}

CORE.Window.context = eglCreateContext(CORE.Window.device, CORE.Window.config, EGL_NO_CONTEXT, contextAttribs);
if (CORE.Window.context == EGL_NO_CONTEXT)
{
TRACELOG(LOG_WARNING, "DISPLAY: Failed to create EGL context");
return false;
}


eglQuerySurface(CORE.Window.device, CORE.Window.surface, EGL_WIDTH, &CORE.Window.screen.width);
eglQuerySurface(CORE.Window.device, CORE.Window.surface, EGL_HEIGHT, &CORE.Window.screen.height);

#else 
EGLint numConfigs;


CORE.Window.device = eglGetDisplay(EGL_DEFAULT_DISPLAY);
if (CORE.Window.device == EGL_NO_DISPLAY)
{
TRACELOG(LOG_WARNING, "DISPLAY: Failed to initialize EGL device");
return false;
}


if (eglInitialize(CORE.Window.device, NULL, NULL) == EGL_FALSE)
{

TRACELOG(LOG_WARNING, "DISPLAY: Failed to initialize EGL device");
return false;
}


eglChooseConfig(CORE.Window.device, framebufferAttribs, &CORE.Window.config, 1, &numConfigs);


eglBindAPI(EGL_OPENGL_ES_API);


CORE.Window.context = eglCreateContext(CORE.Window.device, CORE.Window.config, EGL_NO_CONTEXT, contextAttribs);
if (CORE.Window.context == EGL_NO_CONTEXT)
{
TRACELOG(LOG_WARNING, "DISPLAY: Failed to create EGL context");
return false;
}
#endif



#if defined(PLATFORM_ANDROID)
EGLint displayFormat;



eglGetConfigAttrib(CORE.Window.device, CORE.Window.config, EGL_NATIVE_VISUAL_ID, &displayFormat);



SetupFramebuffer(CORE.Window.display.width, CORE.Window.display.height);

ANativeWindow_setBuffersGeometry(CORE.Android.app->window, CORE.Window.render.width, CORE.Window.render.height, displayFormat);


CORE.Window.surface = eglCreateWindowSurface(CORE.Window.device, CORE.Window.config, CORE.Android.app->window, NULL);
#endif 

#if defined(PLATFORM_RPI)
graphics_get_display_size(0, &CORE.Window.display.width, &CORE.Window.display.height);



SetupFramebuffer(CORE.Window.display.width, CORE.Window.display.height);

dstRect.x = 0;
dstRect.y = 0;
dstRect.width = CORE.Window.display.width;
dstRect.height = CORE.Window.display.height;

srcRect.x = 0;
srcRect.y = 0;
srcRect.width = CORE.Window.render.width << 16;
srcRect.height = CORE.Window.render.height << 16;




VC_DISPMANX_ALPHA_T alpha;
alpha.flags = DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS;

alpha.opacity = 255; 
alpha.mask = 0;

dispmanDisplay = vc_dispmanx_display_open(0); 
dispmanUpdate = vc_dispmanx_update_start(0);

dispmanElement = vc_dispmanx_element_add(dispmanUpdate, dispmanDisplay, 0, &dstRect, 0,
&srcRect, DISPMANX_PROTECTION_NONE, &alpha, 0, DISPMANX_NO_ROTATE);

CORE.Window.handle.element = dispmanElement;
CORE.Window.handle.width = CORE.Window.render.width;
CORE.Window.handle.height = CORE.Window.render.height;
vc_dispmanx_update_submit_sync(dispmanUpdate);

CORE.Window.surface = eglCreateWindowSurface(CORE.Window.device, CORE.Window.config, &CORE.Window.handle, NULL);

#endif 




if (eglMakeCurrent(CORE.Window.device, CORE.Window.surface, CORE.Window.surface, CORE.Window.context) == EGL_FALSE)
{
TRACELOG(LOG_WARNING, "DISPLAY: Failed to attach EGL rendering context to EGL surface");
return false;
}
else
{




TRACELOG(LOG_INFO, "DISPLAY: Device initialized successfully");
TRACELOG(LOG_INFO, " > Display size: %i x %i", CORE.Window.display.width, CORE.Window.display.height);
TRACELOG(LOG_INFO, " > Render size: %i x %i", CORE.Window.render.width, CORE.Window.render.height);
TRACELOG(LOG_INFO, " > Screen size: %i x %i", CORE.Window.screen.width, CORE.Window.screen.height);
TRACELOG(LOG_INFO, " > Viewport offsets: %i, %i", CORE.Window.renderOffset.x, CORE.Window.renderOffset.y);
}
#endif 



rlglInit(CORE.Window.screen.width, CORE.Window.screen.height);

int fbWidth = CORE.Window.render.width;
int fbHeight = CORE.Window.render.height;

#if defined(PLATFORM_DESKTOP) && defined(SUPPORT_HIGH_DPI)
glfwGetFramebufferSize(CORE.Window.handle, &fbWidth, &fbHeight);


CORE.Window.screenScale = MatrixScale((float)fbWidth/CORE.Window.screen.width, (float)fbHeight/CORE.Window.screen.height, 1.0f);
#if !defined(__APPLE__)
SetMouseScale((float)CORE.Window.screen.width/fbWidth, (float)CORE.Window.screen.height/fbHeight);
#endif
#endif 


SetupViewport(fbWidth, fbHeight);

CORE.Window.currentFbo.width = CORE.Window.screen.width;
CORE.Window.currentFbo.height = CORE.Window.screen.height;

ClearBackground(RAYWHITE); 

#if defined(PLATFORM_ANDROID)
CORE.Window.ready = true;
#endif
return true;
}


static void SetupViewport(int width, int height)
{
CORE.Window.render.width = width;
CORE.Window.render.height = height;




rlViewport(CORE.Window.renderOffset.x/2, CORE.Window.renderOffset.y/2, CORE.Window.render.width - CORE.Window.renderOffset.x, CORE.Window.render.height - CORE.Window.renderOffset.y);

rlMatrixMode(RL_PROJECTION); 
rlLoadIdentity(); 



rlOrtho(0, CORE.Window.render.width, CORE.Window.render.height, 0, 0.0f, 1.0f);

rlMatrixMode(RL_MODELVIEW); 
rlLoadIdentity(); 
}



static void SetupFramebuffer(int width, int height)
{

if ((CORE.Window.screen.width > CORE.Window.display.width) || (CORE.Window.screen.height > CORE.Window.display.height))
{
TRACELOG(LOG_WARNING, "DISPLAY: Downscaling required: Screen size (%ix%i) is bigger than display size (%ix%i)", CORE.Window.screen.width, CORE.Window.screen.height, CORE.Window.display.width, CORE.Window.display.height);


float widthRatio = (float)CORE.Window.display.width/(float)CORE.Window.screen.width;
float heightRatio = (float)CORE.Window.display.height/(float)CORE.Window.screen.height;

if (widthRatio <= heightRatio)
{
CORE.Window.render.width = CORE.Window.display.width;
CORE.Window.render.height = (int)round((float)CORE.Window.screen.height*widthRatio);
CORE.Window.renderOffset.x = 0;
CORE.Window.renderOffset.y = (CORE.Window.display.height - CORE.Window.render.height);
}
else
{
CORE.Window.render.width = (int)round((float)CORE.Window.screen.width*heightRatio);
CORE.Window.render.height = CORE.Window.display.height;
CORE.Window.renderOffset.x = (CORE.Window.display.width - CORE.Window.render.width);
CORE.Window.renderOffset.y = 0;
}


float scaleRatio = (float)CORE.Window.render.width/(float)CORE.Window.screen.width;
CORE.Window.screenScale = MatrixScale(scaleRatio, scaleRatio, 1.0f);



CORE.Window.render.width = CORE.Window.display.width;
CORE.Window.render.height = CORE.Window.display.height;

TRACELOG(LOG_WARNING, "DISPLAY: Downscale matrix generated, content will be rendered at (%ix%i)", CORE.Window.render.width, CORE.Window.render.height);
}
else if ((CORE.Window.screen.width < CORE.Window.display.width) || (CORE.Window.screen.height < CORE.Window.display.height))
{

TRACELOG(LOG_INFO, "DISPLAY: Upscaling required: Screen size (%ix%i) smaller than display size (%ix%i)", CORE.Window.screen.width, CORE.Window.screen.height, CORE.Window.display.width, CORE.Window.display.height);


float displayRatio = (float)CORE.Window.display.width/(float)CORE.Window.display.height;
float screenRatio = (float)CORE.Window.screen.width/(float)CORE.Window.screen.height;

if (displayRatio <= screenRatio)
{
CORE.Window.render.width = CORE.Window.screen.width;
CORE.Window.render.height = (int)round((float)CORE.Window.screen.width/displayRatio);
CORE.Window.renderOffset.x = 0;
CORE.Window.renderOffset.y = (CORE.Window.render.height - CORE.Window.screen.height);
}
else
{
CORE.Window.render.width = (int)round((float)CORE.Window.screen.height*displayRatio);
CORE.Window.render.height = CORE.Window.screen.height;
CORE.Window.renderOffset.x = (CORE.Window.render.width - CORE.Window.screen.width);
CORE.Window.renderOffset.y = 0;
}
}
else
{
CORE.Window.render.width = CORE.Window.screen.width;
CORE.Window.render.height = CORE.Window.screen.height;
CORE.Window.renderOffset.x = 0;
CORE.Window.renderOffset.y = 0;
}
}


static void InitTimer(void)
{
srand((unsigned int)time(NULL)); 

#if !defined(SUPPORT_BUSY_WAIT_LOOP) && defined(_WIN32)
timeBeginPeriod(1); 
#endif

#if defined(PLATFORM_ANDROID) || defined(PLATFORM_RPI)
struct timespec now;

if (clock_gettime(CLOCK_MONOTONIC, &now) == 0) 
{
CORE.Time.base = (unsigned long long int)now.tv_sec*1000000000LLU + (unsigned long long int)now.tv_nsec;
}
else TRACELOG(LOG_WARNING, "TIMER: Hi-resolution timer not available");
#endif

CORE.Time.previous = GetTime(); 
}






static void Wait(float ms)
{
#if defined(SUPPORT_BUSY_WAIT_LOOP) && !defined(PLATFORM_UWP)
double prevTime = GetTime();
double nextTime = 0.0;


while ((nextTime - prevTime) < ms/1000.0f) nextTime = GetTime();
#else
#if defined(SUPPORT_HALFBUSY_WAIT_LOOP)
#define MAX_HALFBUSY_WAIT_TIME 4
double destTime = GetTime() + ms/1000;
if (ms > MAX_HALFBUSY_WAIT_TIME) ms -= MAX_HALFBUSY_WAIT_TIME;
#endif

#if defined(_WIN32)
Sleep((unsigned int)ms);
#elif defined(__linux__) || defined(PLATFORM_WEB)
struct timespec req = { 0 };
time_t sec = (int)(ms/1000.0f);
ms -= (sec*1000);
req.tv_sec = sec;
req.tv_nsec = ms*1000000L;


while (nanosleep(&req, &req) == -1) continue;
#elif defined(__APPLE__)
usleep(ms*1000.0f);
#endif

#if defined(SUPPORT_HALFBUSY_WAIT_LOOP)
while (GetTime() < destTime) { }
#endif
#endif
}


static int GetGamepadButton(int button)
{
int btn = GAMEPAD_BUTTON_UNKNOWN;
#if defined(PLATFORM_DESKTOP)
switch (button)
{
case GLFW_GAMEPAD_BUTTON_Y: btn = GAMEPAD_BUTTON_RIGHT_FACE_UP; break;
case GLFW_GAMEPAD_BUTTON_B: btn = GAMEPAD_BUTTON_RIGHT_FACE_RIGHT; break;
case GLFW_GAMEPAD_BUTTON_A: btn = GAMEPAD_BUTTON_RIGHT_FACE_DOWN; break;
case GLFW_GAMEPAD_BUTTON_X: btn = GAMEPAD_BUTTON_RIGHT_FACE_LEFT; break;

case GLFW_GAMEPAD_BUTTON_LEFT_BUMPER: btn = GAMEPAD_BUTTON_LEFT_TRIGGER_1; break;
case GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER: btn = GAMEPAD_BUTTON_RIGHT_TRIGGER_1; break;

case GLFW_GAMEPAD_BUTTON_BACK: btn = GAMEPAD_BUTTON_MIDDLE_LEFT; break;
case GLFW_GAMEPAD_BUTTON_GUIDE: btn = GAMEPAD_BUTTON_MIDDLE; break;
case GLFW_GAMEPAD_BUTTON_START: btn = GAMEPAD_BUTTON_MIDDLE_RIGHT; break;

case GLFW_GAMEPAD_BUTTON_DPAD_UP: btn = GAMEPAD_BUTTON_LEFT_FACE_UP; break;
case GLFW_GAMEPAD_BUTTON_DPAD_RIGHT: btn = GAMEPAD_BUTTON_LEFT_FACE_RIGHT; break;
case GLFW_GAMEPAD_BUTTON_DPAD_DOWN: btn = GAMEPAD_BUTTON_LEFT_FACE_DOWN; break;
case GLFW_GAMEPAD_BUTTON_DPAD_LEFT: btn = GAMEPAD_BUTTON_LEFT_FACE_LEFT; break;

case GLFW_GAMEPAD_BUTTON_LEFT_THUMB: btn = GAMEPAD_BUTTON_LEFT_THUMB; break;
case GLFW_GAMEPAD_BUTTON_RIGHT_THUMB: btn = GAMEPAD_BUTTON_RIGHT_THUMB; break;
}
#endif

#if defined(PLATFORM_UWP)
btn = button; 
#endif

#if defined(PLATFORM_WEB)

switch (button)
{
case 0: btn = GAMEPAD_BUTTON_RIGHT_FACE_DOWN; break;
case 1: btn = GAMEPAD_BUTTON_RIGHT_FACE_RIGHT; break;
case 2: btn = GAMEPAD_BUTTON_RIGHT_FACE_LEFT; break;
case 3: btn = GAMEPAD_BUTTON_RIGHT_FACE_UP; break;
case 4: btn = GAMEPAD_BUTTON_LEFT_TRIGGER_1; break;
case 5: btn = GAMEPAD_BUTTON_RIGHT_TRIGGER_1; break;
case 6: btn = GAMEPAD_BUTTON_LEFT_TRIGGER_2; break;
case 7: btn = GAMEPAD_BUTTON_RIGHT_TRIGGER_2; break;
case 8: btn = GAMEPAD_BUTTON_MIDDLE_LEFT; break;
case 9: btn = GAMEPAD_BUTTON_MIDDLE_RIGHT; break;
case 10: btn = GAMEPAD_BUTTON_LEFT_THUMB; break;
case 11: btn = GAMEPAD_BUTTON_RIGHT_THUMB; break;
case 12: btn = GAMEPAD_BUTTON_LEFT_FACE_UP; break;
case 13: btn = GAMEPAD_BUTTON_LEFT_FACE_DOWN; break;
case 14: btn = GAMEPAD_BUTTON_LEFT_FACE_LEFT; break;
case 15: btn = GAMEPAD_BUTTON_LEFT_FACE_RIGHT; break;
}
#endif

return btn;
}


static int GetGamepadAxis(int axis)
{
int axs = GAMEPAD_AXIS_UNKNOWN;
#if defined(PLATFORM_DESKTOP)
switch (axis)
{
case GLFW_GAMEPAD_AXIS_LEFT_X: axs = GAMEPAD_AXIS_LEFT_X; break;
case GLFW_GAMEPAD_AXIS_LEFT_Y: axs = GAMEPAD_AXIS_LEFT_Y; break;
case GLFW_GAMEPAD_AXIS_RIGHT_X: axs = GAMEPAD_AXIS_RIGHT_X; break;
case GLFW_GAMEPAD_AXIS_RIGHT_Y: axs = GAMEPAD_AXIS_RIGHT_Y; break;
case GLFW_GAMEPAD_AXIS_LEFT_TRIGGER: axs = GAMEPAD_AXIS_LEFT_TRIGGER; break;
case GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER: axs = GAMEPAD_AXIS_RIGHT_TRIGGER; break;
}
#endif

#if defined(PLATFORM_UWP)
axs = axis; 
#endif

#if defined(PLATFORM_WEB)

switch (axis)
{
case 0: axs = GAMEPAD_AXIS_LEFT_X;
case 1: axs = GAMEPAD_AXIS_LEFT_Y;
case 2: axs = GAMEPAD_AXIS_RIGHT_X;
case 3: axs = GAMEPAD_AXIS_RIGHT_X;
}
#endif

return axs;
}


static void PollInputEvents(void)
{
#if defined(SUPPORT_GESTURES_SYSTEM)


UpdateGestures();
#endif


CORE.Input.Keyboard.keyPressedQueueCount = 0;

#if !defined(PLATFORM_RPI)

CORE.Input.Gamepad.lastButtonPressed = -1;
CORE.Input.Gamepad.axisCount = 0;
#endif

#if defined(PLATFORM_RPI)

for (int i = 0; i < 512; i++) CORE.Input.Keyboard.previousKeyState[i] = CORE.Input.Keyboard.currentKeyState[i];


if (CORE.Input.Keyboard.lastKeyPressed.head != CORE.Input.Keyboard.lastKeyPressed.tail)
{
CORE.Input.Keyboard.keyPressedQueue[CORE.Input.Keyboard.keyPressedQueueCount] = CORE.Input.Keyboard.lastKeyPressed.contents[CORE.Input.Keyboard.lastKeyPressed.tail]; 
CORE.Input.Keyboard.keyPressedQueueCount++;

CORE.Input.Keyboard.lastKeyPressed.tail = (CORE.Input.Keyboard.lastKeyPressed.tail + 1) & 0x07; 
}


CORE.Input.Mouse.previousWheelMove = CORE.Input.Mouse.currentWheelMove;
CORE.Input.Mouse.currentWheelMove = 0;
for (int i = 0; i < 3; i++)
{
CORE.Input.Mouse.previousButtonState[i] = CORE.Input.Mouse.currentButtonState[i];
CORE.Input.Mouse.currentButtonState[i] = CORE.Input.Mouse.currentButtonStateEvdev[i];
}
#endif

#if defined(PLATFORM_UWP)

for (int i = 0; i < 512; i++) CORE.Input.Keyboard.previousKeyState[i] = CORE.Input.Keyboard.currentKeyState[i];

for (int i = 0; i < MAX_GAMEPADS; i++)
{
if (CORE.Input.Gamepad.ready[i])
{
for (int k = 0; k < MAX_GAMEPAD_BUTTONS; k++) CORE.Input.Gamepad.previousState[i][k] = CORE.Input.Gamepad.currentState[i][k];
}
}


CORE.Input.Mouse.previousWheelMove = CORE.Input.Mouse.currentWheelMove;
CORE.Input.Mouse.currentWheelMove = 0;

for (int i = 0; i < 3; i++) CORE.Input.Mouse.previousButtonState[i] = CORE.Input.Mouse.currentButtonState[i];


while (HasMessageFromUWP())
{
UWPMessage *msg = GetMessageFromUWP();

switch (msg->type)
{
case UWP_MSG_REGISTER_KEY:
{

int actualKey = -1;

switch (msg->paramInt0)
{
case 0x08: actualKey = KEY_BACKSPACE; break;
case 0x20: actualKey = KEY_SPACE; break;
case 0x1B: actualKey = KEY_ESCAPE; break;
case 0x0D: actualKey = KEY_ENTER; break;
case 0x2E: actualKey = KEY_DELETE; break;
case 0x27: actualKey = KEY_RIGHT; break;
case 0x25: actualKey = KEY_LEFT; break;
case 0x28: actualKey = KEY_DOWN; break;
case 0x26: actualKey = KEY_UP; break;
case 0x70: actualKey = KEY_F1; break;
case 0x71: actualKey = KEY_F2; break;
case 0x72: actualKey = KEY_F3; break;
case 0x73: actualKey = KEY_F4; break;
case 0x74: actualKey = KEY_F5; break;
case 0x75: actualKey = KEY_F6; break;
case 0x76: actualKey = KEY_F7; break;
case 0x77: actualKey = KEY_F8; break;
case 0x78: actualKey = KEY_F9; break;
case 0x79: actualKey = KEY_F10; break;
case 0x7A: actualKey = KEY_F11; break;
case 0x7B: actualKey = KEY_F12; break;
case 0xA0: actualKey = KEY_LEFT_SHIFT; break;
case 0xA2: actualKey = KEY_LEFT_CONTROL; break;
case 0xA4: actualKey = KEY_LEFT_ALT; break;
case 0xA1: actualKey = KEY_RIGHT_SHIFT; break;
case 0xA3: actualKey = KEY_RIGHT_CONTROL; break;
case 0xA5: actualKey = KEY_RIGHT_ALT; break;
case 0x30: actualKey = KEY_ZERO; break;
case 0x31: actualKey = KEY_ONE; break;
case 0x32: actualKey = KEY_TWO; break;
case 0x33: actualKey = KEY_THREE; break;
case 0x34: actualKey = KEY_FOUR; break;
case 0x35: actualKey = KEY_FIVE; break;
case 0x36: actualKey = KEY_SIX; break;
case 0x37: actualKey = KEY_SEVEN; break;
case 0x38: actualKey = KEY_EIGHT; break;
case 0x39: actualKey = KEY_NINE; break;
case 0x41: actualKey = KEY_A; break;
case 0x42: actualKey = KEY_B; break;
case 0x43: actualKey = KEY_C; break;
case 0x44: actualKey = KEY_D; break;
case 0x45: actualKey = KEY_E; break;
case 0x46: actualKey = KEY_F; break;
case 0x47: actualKey = KEY_G; break;
case 0x48: actualKey = KEY_H; break;
case 0x49: actualKey = KEY_I; break;
case 0x4A: actualKey = KEY_J; break;
case 0x4B: actualKey = KEY_K; break;
case 0x4C: actualKey = KEY_L; break;
case 0x4D: actualKey = KEY_M; break;
case 0x4E: actualKey = KEY_N; break;
case 0x4F: actualKey = KEY_O; break;
case 0x50: actualKey = KEY_P; break;
case 0x51: actualKey = KEY_Q; break;
case 0x52: actualKey = KEY_R; break;
case 0x53: actualKey = KEY_S; break;
case 0x54: actualKey = KEY_T; break;
case 0x55: actualKey = KEY_U; break;
case 0x56: actualKey = KEY_V; break;
case 0x57: actualKey = KEY_W; break;
case 0x58: actualKey = KEY_X; break;
case 0x59: actualKey = KEY_Y; break;
case 0x5A: actualKey = KEY_Z; break;
default: break;
}

if (actualKey > -1) CORE.Input.Keyboard.currentKeyState[actualKey] = msg->paramChar0;

} break;
case UWP_MSG_REGISTER_CLICK: CORE.Input.Mouse.currentButtonState[msg->paramInt0] = msg->paramChar0; break;
case UWP_MSG_SCROLL_WHEEL_UPDATE: CORE.Input.Mouse.currentWheelMove += msg->paramInt0; break;
case UWP_MSG_UPDATE_MOUSE_LOCATION: CORE.Input.Mouse.position = msg->paramVector0; break;
case UWP_MSG_SET_GAMEPAD_ACTIVE: if (msg->paramInt0 < MAX_GAMEPADS) CORE.Input.Gamepad.ready[msg->paramInt0] = msg->paramBool0; break;
case UWP_MSG_SET_GAMEPAD_BUTTON:
{
if ((msg->paramInt0 < MAX_GAMEPADS) && (msg->paramInt1 < MAX_GAMEPAD_BUTTONS)) CORE.Input.Gamepad.currentState[msg->paramInt0][msg->paramInt1] = msg->paramChar0;
} break;
case UWP_MSG_SET_GAMEPAD_AXIS:
{
if ((msg->paramInt0 < MAX_GAMEPADS) && (msg->paramInt1 < MAX_GAMEPAD_AXIS)) CORE.Input.Gamepad.axisState[msg->paramInt0][msg->paramInt1] = msg->paramFloat0;


CORE.Input.Gamepad.currentState[msg->paramInt0][GAMEPAD_BUTTON_LEFT_TRIGGER_2] = (char)(CORE.Input.Gamepad.axisState[msg->paramInt0][GAMEPAD_AXIS_LEFT_TRIGGER] > 0.1);
CORE.Input.Gamepad.currentState[msg->paramInt0][GAMEPAD_BUTTON_RIGHT_TRIGGER_2] = (char)(CORE.Input.Gamepad.axisState[msg->paramInt0][GAMEPAD_AXIS_RIGHT_TRIGGER] > 0.1);
} break;
case UWP_MSG_SET_DISPLAY_DIMS:
{
CORE.Window.display.width = msg->paramVector0.x;
CORE.Window.display.height = msg->paramVector0.y;
} break;
case UWP_MSG_HANDLE_RESIZE:
{
eglQuerySurface(CORE.Window.device, CORE.Window.surface, EGL_WIDTH, &CORE.Window.screen.width);
eglQuerySurface(CORE.Window.device, CORE.Window.surface, EGL_HEIGHT, &CORE.Window.screen.height);


rlViewport(0, 0, CORE.Window.screen.width, CORE.Window.screen.height); 
rlMatrixMode(RL_PROJECTION); 
rlLoadIdentity(); 
rlOrtho(0, CORE.Window.screen.width, CORE.Window.screen.height, 0, 0.0f, 1.0f); 
rlMatrixMode(RL_MODELVIEW); 
rlLoadIdentity(); 
rlClearScreenBuffers(); 




CORE.Window.currentFbo.width = CORE.Window.screen.width;
CORE.Window.currentFbo.height = CORE.Window.screen.height;



CORE.Window.resized = true;

} break;
case UWP_MSG_SET_GAME_TIME: CORE.Time.current = msg->paramDouble0; break;
default: break;
}

DeleteUWPMessage(msg); 
}
#endif 

#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)



for (int i = 0; i < 512; i++) CORE.Input.Keyboard.previousKeyState[i] = CORE.Input.Keyboard.currentKeyState[i];


for (int i = 0; i < 3; i++) CORE.Input.Mouse.previousButtonState[i] = CORE.Input.Mouse.currentButtonState[i];


CORE.Input.Mouse.previousWheelMove = CORE.Input.Mouse.currentWheelMove;
CORE.Input.Mouse.currentWheelMove = 0;
#endif


for (int i = 0; i < MAX_TOUCH_POINTS; i++) CORE.Input.Touch.previousTouchState[i] = CORE.Input.Touch.currentTouchState[i];

#if defined(PLATFORM_DESKTOP)


for (int i = 0; i < MAX_GAMEPADS; i++)
{
if (glfwJoystickPresent(i)) CORE.Input.Gamepad.ready[i] = true;
else CORE.Input.Gamepad.ready[i] = false;
}


for (int i = 0; i < MAX_GAMEPADS; i++)
{
if (CORE.Input.Gamepad.ready[i]) 
{

for (int k = 0; k < MAX_GAMEPAD_BUTTONS; k++) CORE.Input.Gamepad.previousState[i][k] = CORE.Input.Gamepad.currentState[i][k];




GLFWgamepadstate state;
glfwGetGamepadState(i, &state); 
const unsigned char *buttons = state.buttons;

for (int k = 0; (buttons != NULL) && (k < GLFW_GAMEPAD_BUTTON_DPAD_LEFT + 1) && (k < MAX_GAMEPAD_BUTTONS); k++)
{
const GamepadButton button = GetGamepadButton(k);

if (buttons[k] == GLFW_PRESS)
{
CORE.Input.Gamepad.currentState[i][button] = 1;
CORE.Input.Gamepad.lastButtonPressed = button;
}
else CORE.Input.Gamepad.currentState[i][button] = 0;
}


const float *axes = state.axes;

for (int k = 0; (axes != NULL) && (k < GLFW_GAMEPAD_AXIS_LAST + 1) && (k < MAX_GAMEPAD_AXIS); k++)
{
const int axis = GetGamepadAxis(k);
CORE.Input.Gamepad.axisState[i][axis] = axes[k];
}


CORE.Input.Gamepad.currentState[i][GAMEPAD_BUTTON_LEFT_TRIGGER_2] = (char)(CORE.Input.Gamepad.axisState[i][GAMEPAD_AXIS_LEFT_TRIGGER] > 0.1);
CORE.Input.Gamepad.currentState[i][GAMEPAD_BUTTON_RIGHT_TRIGGER_2] = (char)(CORE.Input.Gamepad.axisState[i][GAMEPAD_AXIS_RIGHT_TRIGGER] > 0.1);

CORE.Input.Gamepad.axisCount = GLFW_GAMEPAD_AXIS_LAST;
}
}

CORE.Window.resized = false;

#if defined(SUPPORT_EVENTS_WAITING)
glfwWaitEvents();
#else
glfwPollEvents(); 
#endif
#endif 



#if defined(PLATFORM_WEB)

int numGamepads = 0;
if (emscripten_sample_gamepad_data() == EMSCRIPTEN_RESULT_SUCCESS) numGamepads = emscripten_get_num_gamepads();

for (int i = 0; (i < numGamepads) && (i < MAX_GAMEPADS); i++)
{

for (int k = 0; k < MAX_GAMEPAD_BUTTONS; k++) CORE.Input.Gamepad.previousState[i][k] = CORE.Input.Gamepad.currentState[i][k];

EmscriptenGamepadEvent gamepadState;

int result = emscripten_get_gamepad_status(i, &gamepadState);

if (result == EMSCRIPTEN_RESULT_SUCCESS)
{

for (int j = 0; (j < gamepadState.numButtons) && (j < MAX_GAMEPAD_BUTTONS); j++)
{
const GamepadButton button = GetGamepadButton(j);
if (gamepadState.digitalButton[j] == 1)
{
CORE.Input.Gamepad.currentState[i][button] = 1;
CORE.Input.Gamepad.lastButtonPressed = button;
}
else CORE.Input.Gamepad.currentState[i][button] = 0;


}


for (int j = 0; (j < gamepadState.numAxes) && (j < MAX_GAMEPAD_AXIS); j++)
{
const int axis = GetGamepadAxis(j);
CORE.Input.Gamepad.axisState[i][axis] = gamepadState.axis[j];
}

CORE.Input.Gamepad.axisCount = gamepadState.numAxes;
}
}
#endif

#if defined(PLATFORM_ANDROID)


for (int i = 0; i < 260; i++) CORE.Input.Keyboard.previousKeyState[i] = CORE.Input.Keyboard.currentKeyState[i];


int pollResult = 0;
int pollEvents = 0;



while ((pollResult = ALooper_pollAll(CORE.Android.appEnabled? 0 : -1, NULL, &pollEvents, (void**)&CORE.Android.source)) >= 0)
{

if (CORE.Android.source != NULL) CORE.Android.source->process(CORE.Android.app, CORE.Android.source);


if (CORE.Android.app->destroyRequested != 0)
{


}
}
#endif

#if defined(PLATFORM_RPI) && defined(SUPPORT_SSH_KEYBOARD_RPI)


ProcessKeyboard();



#endif
}


static void SwapBuffers(void)
{
#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
glfwSwapBuffers(CORE.Window.handle);
#endif

#if defined(PLATFORM_ANDROID) || defined(PLATFORM_RPI) || defined(PLATFORM_UWP)
eglSwapBuffers(CORE.Window.device, CORE.Window.surface);
#endif
}

#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)

static void ErrorCallback(int error, const char *description)
{
TRACELOG(LOG_WARNING, "GLFW: Error: %i Description: %s", error, description);
}


static void ScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
CORE.Input.Mouse.currentWheelMove = (int)yoffset;
}


static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
if (key == CORE.Input.Keyboard.exitKey && action == GLFW_PRESS)
{
glfwSetWindowShouldClose(CORE.Window.handle, GLFW_TRUE);


}
else if (key == GLFW_KEY_F12 && action == GLFW_PRESS)
{
#if defined(SUPPORT_GIF_RECORDING)
if (mods == GLFW_MOD_CONTROL)
{
if (gifRecording)
{
GifEnd();
gifRecording = false;

#if defined(PLATFORM_WEB)


emscripten_run_script(TextFormat("saveFileFromMEMFSToDisk('%s','%s')", TextFormat("screenrec%03i.gif", screenshotCounter - 1), TextFormat("screenrec%03i.gif", screenshotCounter - 1)));
#endif

TRACELOG(LOG_INFO, "SYSTEM: Finish animated GIF recording");
}
else
{
gifRecording = true;
gifFramesCounter = 0;

char path[512] = { 0 };
#if defined(PLATFORM_ANDROID)
strcpy(path, CORE.Android.internalDataPath);
strcat(path, TextFormat("./screenrec%03i.gif", screenshotCounter));
#else
strcpy(path, TextFormat("./screenrec%03i.gif", screenshotCounter));
#endif



GifBegin(path, CORE.Window.screen.width, CORE.Window.screen.height, (int)(GetFrameTime()*10.0f), 8, false);
screenshotCounter++;

TRACELOG(LOG_INFO, "SYSTEM: Start animated GIF recording: %s", TextFormat("screenrec%03i.gif", screenshotCounter));
}
}
else
#endif 
#if defined(SUPPORT_SCREEN_CAPTURE)
{
TakeScreenshot(TextFormat("screenshot%03i.png", screenshotCounter));
screenshotCounter++;
}
#endif 
}
else
{


if (action == GLFW_RELEASE) CORE.Input.Keyboard.currentKeyState[key] = 0;
else CORE.Input.Keyboard.currentKeyState[key] = 1;
}
}


static void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{


CORE.Input.Mouse.currentButtonState[button] = action;

#if defined(SUPPORT_GESTURES_SYSTEM) && defined(SUPPORT_MOUSE_GESTURES)

GestureEvent gestureEvent = { 0 };


if ((CORE.Input.Mouse.currentButtonState[button] == 1) && (CORE.Input.Mouse.previousButtonState[button] == 0)) gestureEvent.touchAction = TOUCH_DOWN;
else if ((CORE.Input.Mouse.currentButtonState[button] == 0) && (CORE.Input.Mouse.previousButtonState[button] == 1)) gestureEvent.touchAction = TOUCH_UP;




gestureEvent.pointerId[0] = 0;


gestureEvent.pointCount = 1;


gestureEvent.position[0] = GetMousePosition();


gestureEvent.position[0].x /= (float)GetScreenWidth();
gestureEvent.position[0].y /= (float)GetScreenHeight();


ProcessGestureEvent(gestureEvent);
#endif
}


static void MouseCursorPosCallback(GLFWwindow *window, double x, double y)
{
CORE.Input.Mouse.position.x = (float)x;
CORE.Input.Mouse.position.y = (float)y;
CORE.Input.Touch.position[0] = CORE.Input.Mouse.position;

#if defined(SUPPORT_GESTURES_SYSTEM) && defined(SUPPORT_MOUSE_GESTURES)

GestureEvent gestureEvent = { 0 };

gestureEvent.touchAction = TOUCH_MOVE;


gestureEvent.pointerId[0] = 0;


gestureEvent.pointCount = 1;


gestureEvent.position[0] = CORE.Input.Touch.position[0];


gestureEvent.position[0].x /= (float)GetScreenWidth();
gestureEvent.position[0].y /= (float)GetScreenHeight();


ProcessGestureEvent(gestureEvent);
#endif
}


static void CharCallback(GLFWwindow *window, unsigned int key)
{






if (CORE.Input.Keyboard.keyPressedQueueCount < MAX_CHARS_QUEUE)
{

CORE.Input.Keyboard.keyPressedQueue[CORE.Input.Keyboard.keyPressedQueueCount] = key;
CORE.Input.Keyboard.keyPressedQueueCount++;
}
}


static void CursorEnterCallback(GLFWwindow *window, int enter)
{
if (enter == true) CORE.Input.Mouse.cursorOnScreen = true;
else CORE.Input.Mouse.cursorOnScreen = false;
}



static void WindowSizeCallback(GLFWwindow *window, int width, int height)
{
SetupViewport(width, height); 


CORE.Window.screen.width = width;
CORE.Window.screen.height = height;
CORE.Window.currentFbo.width = width;
CORE.Window.currentFbo.height = height;



CORE.Window.resized = true;
}


static void WindowIconifyCallback(GLFWwindow *window, int iconified)
{
if (iconified) CORE.Window.minimized = true; 
else CORE.Window.minimized = false; 
}




static void WindowDropCallback(GLFWwindow *window, int count, const char **paths)
{
ClearDroppedFiles();

CORE.Window.dropFilesPath = (char **)RL_MALLOC(sizeof(char *)*count);

for (int i = 0; i < count; i++)
{
CORE.Window.dropFilesPath[i] = (char *)RL_MALLOC(sizeof(char)*MAX_FILEPATH_LENGTH);
strcpy(CORE.Window.dropFilesPath[i], paths[i]);
}

CORE.Window.dropFilesCount = count;
}
#endif

#if defined(PLATFORM_ANDROID)

static void AndroidCommandCallback(struct android_app *app, int32_t cmd)
{
switch (cmd)
{
case APP_CMD_START:
{

} break;
case APP_CMD_RESUME: break;
case APP_CMD_INIT_WINDOW:
{
if (app->window != NULL)
{
if (CORE.Android.contextRebindRequired)
{

EGLint displayFormat;
eglGetConfigAttrib(CORE.Window.device, CORE.Window.config, EGL_NATIVE_VISUAL_ID, &displayFormat);
ANativeWindow_setBuffersGeometry(app->window, CORE.Window.render.width, CORE.Window.render.height, displayFormat);


CORE.Window.surface = eglCreateWindowSurface(CORE.Window.device, CORE.Window.config, app->window, NULL);
eglMakeCurrent(CORE.Window.device, CORE.Window.surface, CORE.Window.surface, CORE.Window.context);

CORE.Android.contextRebindRequired = false;
}
else
{
CORE.Window.display.width = ANativeWindow_getWidth(CORE.Android.app->window);
CORE.Window.display.height = ANativeWindow_getHeight(CORE.Android.app->window);


InitGraphicsDevice(CORE.Window.screen.width, CORE.Window.screen.height);


InitTimer();

#if defined(SUPPORT_DEFAULT_FONT)


LoadFontDefault();
Rectangle rec = GetFontDefault().recs[95];

SetShapesTexture(GetFontDefault().texture, (Rectangle){ rec.x + 1, rec.y + 1, rec.width - 2, rec.height - 2 });
#endif















}
}
} break;
case APP_CMD_GAINED_FOCUS:
{
CORE.Android.appEnabled = true;

} break;
case APP_CMD_PAUSE: break;
case APP_CMD_LOST_FOCUS:
{
CORE.Android.appEnabled = false;

} break;
case APP_CMD_TERM_WINDOW:
{



eglMakeCurrent(CORE.Window.device, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
eglDestroySurface(CORE.Window.device, CORE.Window.surface);

CORE.Android.contextRebindRequired = true;
} break;
case APP_CMD_SAVE_STATE: break;
case APP_CMD_STOP: break;
case APP_CMD_DESTROY:
{


} break;
case APP_CMD_CONFIG_CHANGED:
{




} break;
default: break;
}
}


static int32_t AndroidInputCallback(struct android_app *app, AInputEvent *event)
{




int type = AInputEvent_getType(event);
int source = AInputEvent_getSource(event);

if (type == AINPUT_EVENT_TYPE_MOTION)
{
if ((source & AINPUT_SOURCE_JOYSTICK) == AINPUT_SOURCE_JOYSTICK || (source & AINPUT_SOURCE_GAMEPAD) == AINPUT_SOURCE_GAMEPAD)
{

CORE.Input.Touch.position[0].x = AMotionEvent_getX(event, 0);
CORE.Input.Touch.position[0].y = AMotionEvent_getY(event, 0);


CORE.Input.Touch.position[1].x = AMotionEvent_getX(event, 1);
CORE.Input.Touch.position[1].y = AMotionEvent_getY(event, 1);

int32_t keycode = AKeyEvent_getKeyCode(event);
if (AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_DOWN)
{
CORE.Input.Keyboard.currentKeyState[keycode] = 1; 

CORE.Input.Keyboard.keyPressedQueue[CORE.Input.Keyboard.keyPressedQueueCount] = keycode;
CORE.Input.Keyboard.keyPressedQueueCount++;
}
else CORE.Input.Keyboard.currentKeyState[keycode] = 0; 


return 1;
}
}
else if (type == AINPUT_EVENT_TYPE_KEY)
{
int32_t keycode = AKeyEvent_getKeyCode(event);




if (AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_DOWN)
{
CORE.Input.Keyboard.currentKeyState[keycode] = 1; 

CORE.Input.Keyboard.keyPressedQueue[CORE.Input.Keyboard.keyPressedQueueCount] = keycode;
CORE.Input.Keyboard.keyPressedQueueCount++;
}
else CORE.Input.Keyboard.currentKeyState[keycode] = 0; 

if (keycode == AKEYCODE_POWER)
{





return 0;
}
else if ((keycode == AKEYCODE_BACK) || (keycode == AKEYCODE_MENU))
{

return 1;
}
else if ((keycode == AKEYCODE_VOLUME_UP) || (keycode == AKEYCODE_VOLUME_DOWN))
{

return 0;
}

return 0;
}

CORE.Input.Touch.position[0].x = AMotionEvent_getX(event, 0);
CORE.Input.Touch.position[0].y = AMotionEvent_getY(event, 0);

int32_t action = AMotionEvent_getAction(event);
unsigned int flags = action & AMOTION_EVENT_ACTION_MASK;

if (flags == AMOTION_EVENT_ACTION_DOWN || flags == AMOTION_EVENT_ACTION_MOVE)
{
CORE.Input.Touch.currentTouchState[MOUSE_LEFT_BUTTON] = 1;
}
else if (flags == AMOTION_EVENT_ACTION_UP)
{
CORE.Input.Touch.currentTouchState[MOUSE_LEFT_BUTTON] = 0;
}

#if defined(SUPPORT_GESTURES_SYSTEM)

GestureEvent gestureEvent;


if (flags == AMOTION_EVENT_ACTION_DOWN) gestureEvent.touchAction = TOUCH_DOWN;
else if (flags == AMOTION_EVENT_ACTION_UP) gestureEvent.touchAction = TOUCH_UP;
else if (flags == AMOTION_EVENT_ACTION_MOVE) gestureEvent.touchAction = TOUCH_MOVE;




gestureEvent.pointCount = AMotionEvent_getPointerCount(event);


if ((gestureEvent.pointCount > 0) && (gestureEvent.pointCount < 4))
{


gestureEvent.pointerId[0] = AMotionEvent_getPointerId(event, 0);
gestureEvent.pointerId[1] = AMotionEvent_getPointerId(event, 1);


gestureEvent.position[0] = (Vector2){ AMotionEvent_getX(event, 0), AMotionEvent_getY(event, 0) };
gestureEvent.position[1] = (Vector2){ AMotionEvent_getX(event, 1), AMotionEvent_getY(event, 1) };


gestureEvent.position[0].x /= (float)GetScreenWidth();
gestureEvent.position[0].y /= (float)GetScreenHeight();

gestureEvent.position[1].x /= (float)GetScreenWidth();
gestureEvent.position[1].y /= (float)GetScreenHeight();


ProcessGestureEvent(gestureEvent);
}
#endif

return 0;
}
#endif

#if defined(PLATFORM_WEB)

static EM_BOOL EmscriptenFullscreenChangeCallback(int eventType, const EmscriptenFullscreenChangeEvent *event, void *userData)
{







if (event->isFullscreen)
{
CORE.Window.fullscreen = true;
TRACELOG(LOG_INFO, "WEB: Canvas scaled to fullscreen. ElementSize: (%ix%i), ScreenSize(%ix%i)", event->elementWidth, event->elementHeight, event->screenWidth, event->screenHeight);
}
else
{
CORE.Window.fullscreen = false;
TRACELOG(LOG_INFO, "WEB: Canvas scaled to windowed. ElementSize: (%ix%i), ScreenSize(%ix%i)", event->elementWidth, event->elementHeight, event->screenWidth, event->screenHeight);
}



return 0;
}


static EM_BOOL EmscriptenKeyboardCallback(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData)
{
if ((eventType == EMSCRIPTEN_EVENT_KEYPRESS) && (strcmp(keyEvent->code, "Escape") == 0))
{
emscripten_exit_pointerlock();
}

return 0;
}


static EM_BOOL EmscriptenMouseCallback(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData)
{

if ((eventType == EMSCRIPTEN_EVENT_CLICK) && CORE.Input.Mouse.cursorLockRequired)
{
EmscriptenPointerlockChangeEvent plce;
emscripten_get_pointerlock_status(&plce);

if (!plce.isActive) emscripten_request_pointerlock(0, 1);
else
{
emscripten_exit_pointerlock();
emscripten_get_pointerlock_status(&plce);

}

CORE.Input.Mouse.cursorLockRequired = false;
}

return 0;
}


static EM_BOOL EmscriptenTouchCallback(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData)
{
for (int i = 0; i < touchEvent->numTouches; i++)
{
if (eventType == EMSCRIPTEN_EVENT_TOUCHSTART) CORE.Input.Touch.currentTouchState[i] = 1;
else if (eventType == EMSCRIPTEN_EVENT_TOUCHEND) CORE.Input.Touch.currentTouchState[i] = 0;
}

#if defined(SUPPORT_GESTURES_SYSTEM)
GestureEvent gestureEvent = { 0 };


if (eventType == EMSCRIPTEN_EVENT_TOUCHSTART) gestureEvent.touchAction = TOUCH_DOWN;
else if (eventType == EMSCRIPTEN_EVENT_TOUCHEND) gestureEvent.touchAction = TOUCH_UP;
else if (eventType == EMSCRIPTEN_EVENT_TOUCHMOVE) gestureEvent.touchAction = TOUCH_MOVE;


gestureEvent.pointCount = touchEvent->numTouches;


gestureEvent.pointerId[0] = touchEvent->touches[0].identifier;
gestureEvent.pointerId[1] = touchEvent->touches[1].identifier;



gestureEvent.position[0] = (Vector2){ touchEvent->touches[0].targetX, touchEvent->touches[0].targetY };
gestureEvent.position[1] = (Vector2){ touchEvent->touches[1].targetX, touchEvent->touches[1].targetY };

double canvasWidth, canvasHeight;



emscripten_get_element_css_size("#canvas", &canvasWidth, &canvasHeight);


gestureEvent.position[0].x *= ((float)GetScreenWidth()/(float)canvasWidth);
gestureEvent.position[0].y *= ((float)GetScreenHeight()/(float)canvasHeight);
gestureEvent.position[1].x *= ((float)GetScreenWidth()/(float)canvasWidth);
gestureEvent.position[1].y *= ((float)GetScreenHeight()/(float)canvasHeight);

CORE.Input.Touch.position[0] = gestureEvent.position[0];
CORE.Input.Touch.position[1] = gestureEvent.position[1];


ProcessGestureEvent(gestureEvent);
#else

if (eventType == EMSCRIPTEN_EVENT_TOUCHSTART)
{

CORE.Input.Touch.position[0] = (Vector2){ touchEvent->touches[0].targetX, touchEvent->touches[0].targetY };

double canvasWidth, canvasHeight;

emscripten_get_element_css_size("#canvas", &canvasWidth, &canvasHeight);


CORE.Input.Touch.position[0].x *= ((float)GetScreenWidth()/(float)canvasWidth);
CORE.Input.Touch.position[0].y *= ((float)GetScreenHeight()/(float)canvasHeight);
}
#endif

return 1;
}


static EM_BOOL EmscriptenGamepadCallback(int eventType, const EmscriptenGamepadEvent *gamepadEvent, void *userData)
{









if ((gamepadEvent->connected) && (gamepadEvent->index < MAX_GAMEPADS)) CORE.Input.Gamepad.ready[gamepadEvent->index] = true;
else CORE.Input.Gamepad.ready[gamepadEvent->index] = false;



return 0;
}
#endif

#if defined(PLATFORM_RPI)

#if defined(SUPPORT_SSH_KEYBOARD_RPI)

static void InitKeyboard(void)
{



int flags = fcntl(STDIN_FILENO, F_GETFL, 0); 
fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK); 


struct termios keyboardNewSettings;
tcgetattr(STDIN_FILENO, &CORE.Input.Keyboard.defaultSettings); 
keyboardNewSettings = CORE.Input.Keyboard.defaultSettings;



keyboardNewSettings.c_lflag &= ~(ICANON | ECHO | ISIG);

keyboardNewSettings.c_cc[VMIN] = 1;
keyboardNewSettings.c_cc[VTIME] = 0;


tcsetattr(STDIN_FILENO, TCSANOW, &keyboardNewSettings);




if (ioctl(STDIN_FILENO, KDGKBMODE, &CORE.Input.Keyboard.defaultMode) < 0)
{

TRACELOG(LOG_WARNING, "RPI: Failed to change keyboard mode (SSH keyboard?)");
}
else
{





ioctl(STDIN_FILENO, KDSKBMODE, K_XLATE);
}


atexit(RestoreKeyboard);
}



static void ProcessKeyboard(void)
{
#define MAX_KEYBUFFER_SIZE 32 


int bufferByteCount = 0; 
char keysBuffer[MAX_KEYBUFFER_SIZE]; 


bufferByteCount = read(STDIN_FILENO, keysBuffer, MAX_KEYBUFFER_SIZE); 


for (int i = 0; i < 512; i++) CORE.Input.Keyboard.currentKeyState[i] = 0;





for (int i = 0; i < bufferByteCount; i++)
{


if (keysBuffer[i] == 0x1b)
{

if (bufferByteCount == 1) CORE.Input.Keyboard.currentKeyState[CORE.Input.Keyboard.exitKey] = 1;
else
{
if (keysBuffer[i + 1] == 0x5b) 
{
if ((keysBuffer[i + 2] == 0x5b) || (keysBuffer[i + 2] == 0x31) || (keysBuffer[i + 2] == 0x32))
{

switch (keysBuffer[i + 3])
{
case 0x41: CORE.Input.Keyboard.currentKeyState[290] = 1; break; 
case 0x42: CORE.Input.Keyboard.currentKeyState[291] = 1; break; 
case 0x43: CORE.Input.Keyboard.currentKeyState[292] = 1; break; 
case 0x44: CORE.Input.Keyboard.currentKeyState[293] = 1; break; 
case 0x45: CORE.Input.Keyboard.currentKeyState[294] = 1; break; 
case 0x37: CORE.Input.Keyboard.currentKeyState[295] = 1; break; 
case 0x38: CORE.Input.Keyboard.currentKeyState[296] = 1; break; 
case 0x39: CORE.Input.Keyboard.currentKeyState[297] = 1; break; 
case 0x30: CORE.Input.Keyboard.currentKeyState[298] = 1; break; 
case 0x31: CORE.Input.Keyboard.currentKeyState[299] = 1; break; 
case 0x33: CORE.Input.Keyboard.currentKeyState[300] = 1; break; 
case 0x34: CORE.Input.Keyboard.currentKeyState[301] = 1; break; 
default: break;
}

if (keysBuffer[i + 2] == 0x5b) i += 4;
else if ((keysBuffer[i + 2] == 0x31) || (keysBuffer[i + 2] == 0x32)) i += 5;
}
else
{
switch (keysBuffer[i + 2])
{
case 0x41: CORE.Input.Keyboard.currentKeyState[265] = 1; break; 
case 0x42: CORE.Input.Keyboard.currentKeyState[264] = 1; break; 
case 0x43: CORE.Input.Keyboard.currentKeyState[262] = 1; break; 
case 0x44: CORE.Input.Keyboard.currentKeyState[263] = 1; break; 
default: break;
}

i += 3; 
}


}
}
}
else if (keysBuffer[i] == 0x0a) 
{
CORE.Input.Keyboard.currentKeyState[257] = 1;

CORE.Input.Keyboard.keyPressedQueue[CORE.Input.Keyboard.keyPressedQueueCount] = 257; 
CORE.Input.Keyboard.keyPressedQueueCount++;
}
else if (keysBuffer[i] == 0x7f) 
{
CORE.Input.Keyboard.currentKeyState[259] = 1;

CORE.Input.Keyboard.keyPressedQueue[CORE.Input.Keyboard.keyPressedQueueCount] = 257; 
CORE.Input.Keyboard.keyPressedQueueCount++;
}
else
{

if ((keysBuffer[i] >= 97) && (keysBuffer[i] <= 122))
{
CORE.Input.Keyboard.currentKeyState[(int)keysBuffer[i] - 32] = 1;
}
else CORE.Input.Keyboard.currentKeyState[(int)keysBuffer[i]] = 1;

CORE.Input.Keyboard.keyPressedQueue[CORE.Input.Keyboard.keyPressedQueueCount] = keysBuffer[i]; 
CORE.Input.Keyboard.keyPressedQueueCount++;
}
}


if (CORE.Input.Keyboard.currentKeyState[CORE.Input.Keyboard.exitKey] == 1) CORE.Window.shouldClose = true;

#if defined(SUPPORT_SCREEN_CAPTURE)

if (CORE.Input.Keyboard.currentKeyState[301] == 1)
{
TakeScreenshot(FormatText("screenshot%03i.png", screenshotCounter));
screenshotCounter++;
}
#endif
}


static void RestoreKeyboard(void)
{

tcsetattr(STDIN_FILENO, TCSANOW, &CORE.Input.Keyboard.defaultSettings);


ioctl(STDIN_FILENO, KDSKBMODE, CORE.Input.Keyboard.defaultMode);
}
#endif 


static void InitEvdevInput(void)
{
char path[MAX_FILEPATH_LENGTH];
DIR *directory;
struct dirent *entity;


for (int i = 0; i < MAX_TOUCH_POINTS; ++i)
{
CORE.Input.Touch.position[i].x = -1;
CORE.Input.Touch.position[i].y = -1;
}


CORE.Input.Keyboard.lastKeyPressed.head = 0;
CORE.Input.Keyboard.lastKeyPressed.tail = 0;


for (int i = 0; i < 512; i++) CORE.Input.Keyboard.currentKeyState[i] = 0;


directory = opendir(DEFAULT_EVDEV_PATH);

if (directory)
{
while ((entity = readdir(directory)) != NULL)
{
if (strncmp("event", entity->d_name, strlen("event")) == 0) 
{
sprintf(path, "%s%s", DEFAULT_EVDEV_PATH, entity->d_name);
EventThreadSpawn(path); 
}
}

closedir(directory);
}
else TRACELOG(LOG_WARNING, "RPI: Failed to open linux event directory: %s", DEFAULT_EVDEV_PATH);
}


static void EventThreadSpawn(char *device)
{
#define BITS_PER_LONG (sizeof(long)*8)
#define NBITS(x) ((((x) - 1)/BITS_PER_LONG) + 1)
#define OFF(x) ((x)%BITS_PER_LONG)
#define BIT(x) (1UL<<OFF(x))
#define LONG(x) ((x)/BITS_PER_LONG)
#define TEST_BIT(array, bit) ((array[LONG(bit)] >> OFF(bit)) & 1)

struct input_absinfo absinfo;
unsigned long evBits[NBITS(EV_MAX)];
unsigned long absBits[NBITS(ABS_MAX)];
unsigned long relBits[NBITS(REL_MAX)];
unsigned long keyBits[NBITS(KEY_MAX)];
bool hasAbs = false;
bool hasRel = false;
bool hasAbsMulti = false;
int freeWorkerId = -1;
int fd = -1;

InputEventWorker *worker;




for (int i = 0; i < sizeof(CORE.Input.eventWorker)/sizeof(InputEventWorker); ++i)
{
if (CORE.Input.eventWorker[i].threadId == 0)
{
freeWorkerId = i;
break;
}
}


if (freeWorkerId >= 0)
{
worker = &(CORE.Input.eventWorker[freeWorkerId]); 
memset(worker, 0, sizeof(InputEventWorker)); 
}
else
{
TRACELOG(LOG_WARNING, "RPI: Failed to create input device thread for %s, out of worker slots", device);
return;
}


fd = open(device, O_RDONLY | O_NONBLOCK);
if (fd < 0)
{
TRACELOG(LOG_WARNING, "RPI: Failed to open input device (error: %d)", device, worker->fd);
return;
}
worker->fd = fd;


int devNum = 0;
char *ptrDevName = strrchr(device, 't');
worker->eventNum = -1;

if (ptrDevName != NULL)
{
if (sscanf(ptrDevName, "t%d", &devNum) == 1)
worker->eventNum = devNum;
}







ioctl(fd, EVIOCGBIT(0, sizeof(evBits)), evBits); 


if (TEST_BIT(evBits, EV_ABS))
{
ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(absBits)), absBits);


if (TEST_BIT(absBits, ABS_X) && TEST_BIT(absBits, ABS_Y))
{
hasAbs = true;


ioctl(fd, EVIOCGABS(ABS_X), &absinfo);
worker->absRange.x = absinfo.minimum;
worker->absRange.width = absinfo.maximum - absinfo.minimum;
ioctl(fd, EVIOCGABS(ABS_Y), &absinfo);
worker->absRange.y = absinfo.minimum;
worker->absRange.height = absinfo.maximum - absinfo.minimum;
}


if (TEST_BIT(absBits, ABS_MT_POSITION_X) && TEST_BIT(absBits, ABS_MT_POSITION_Y))
{
hasAbsMulti = true;


ioctl(fd, EVIOCGABS(ABS_X), &absinfo);
worker->absRange.x = absinfo.minimum;
worker->absRange.width = absinfo.maximum - absinfo.minimum;
ioctl(fd, EVIOCGABS(ABS_Y), &absinfo);
worker->absRange.y = absinfo.minimum;
worker->absRange.height = absinfo.maximum - absinfo.minimum;
}
}


if (TEST_BIT(evBits, EV_REL))
{
ioctl(fd, EVIOCGBIT(EV_REL, sizeof(relBits)), relBits);

if (TEST_BIT(relBits, REL_X) && TEST_BIT(relBits, REL_Y)) hasRel = true;
}


if (TEST_BIT(evBits, EV_KEY))
{
ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keyBits)), keyBits);

if (hasAbs || hasAbsMulti)
{
if (TEST_BIT(keyBits, BTN_TOUCH)) worker->isTouch = true; 
if (TEST_BIT(keyBits, BTN_TOOL_FINGER)) worker->isTouch = true; 
if (TEST_BIT(keyBits, BTN_TOOL_PEN)) worker->isTouch = true; 
if (TEST_BIT(keyBits, BTN_STYLUS)) worker->isTouch = true; 
if (worker->isTouch || hasAbsMulti) worker->isMultitouch = true; 
}

if (hasRel)
{
if (TEST_BIT(keyBits, BTN_LEFT)) worker->isMouse = true; 
if (TEST_BIT(keyBits, BTN_RIGHT)) worker->isMouse = true; 
}

if (TEST_BIT(keyBits, BTN_A)) worker->isGamepad = true; 
if (TEST_BIT(keyBits, BTN_TRIGGER)) worker->isGamepad = true; 
if (TEST_BIT(keyBits, BTN_START)) worker->isGamepad = true; 
if (TEST_BIT(keyBits, BTN_TL)) worker->isGamepad = true; 
if (TEST_BIT(keyBits, BTN_TL)) worker->isGamepad = true; 

if (TEST_BIT(keyBits, KEY_SPACE)) worker->isKeyboard = true; 
}




if (worker->isTouch || worker->isMouse || worker->isKeyboard)
{

TRACELOG(LOG_INFO, "RPI: Opening input device: %s (%s%s%s%s%s)", device,
worker->isMouse? "mouse " : "",
worker->isMultitouch? "multitouch " : "",
worker->isTouch? "touchscreen " : "",
worker->isGamepad? "gamepad " : "",
worker->isKeyboard? "keyboard " : "");


int error = pthread_create(&worker->threadId, NULL, &EventThread, (void *)worker);
if (error != 0)
{
TRACELOG(LOG_WARNING, "RPI: Failed to create input device thread: %s (error: %d)", device, error);
worker->threadId = 0;
close(fd);
}

#if defined(USE_LAST_TOUCH_DEVICE)

int maxTouchNumber = -1;

for (int i = 0; i < sizeof(CORE.Input.eventWorker)/sizeof(InputEventWorker); ++i)
{
if (CORE.Input.eventWorker[i].isTouch && (CORE.Input.eventWorker[i].eventNum > maxTouchNumber)) maxTouchNumber = CORE.Input.eventWorker[i].eventNum;
}


for (int i = 0; i < sizeof(CORE.Input.eventWorker)/sizeof(InputEventWorker); ++i)
{
if (CORE.Input.eventWorker[i].isTouch && (CORE.Input.eventWorker[i].eventNum < maxTouchNumber))
{
if (CORE.Input.eventWorker[i].threadId != 0)
{
TRACELOG(LOG_WARNING, "RPI: Found duplicate touchscreen, killing touchscreen on event: %d", i);
pthread_cancel(CORE.Input.eventWorker[i].threadId);
close(CORE.Input.eventWorker[i].fd);
}
}
}
#endif
}
else close(fd); 

}


static void *EventThread(void *arg)
{



static const int keymap_US[] =
{ 0,256,49,50,51,52,53,54,55,56,57,48,45,61,259,258,81,87,69,82,84,
89,85,73,79,80,91,93,257,341,65,83,68,70,71,72,74,75,76,59,39,96,
340,92,90,88,67,86,66,78,77,44,46,47,344,332,342,32,280,290,291,
292,293,294,295,296,297,298,299,282,281,327,328,329,333,324,325,
326,334,321,322,323,320,330,0,85,86,300,301,89,90,91,92,93,94,95,
335,345,331,283,346,101,268,265,266,263,262,269,264,267,260,261,
112,113,114,115,116,117,118,119,120,121,122,123,124,125,347,127,
128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
192,193,194,0,0,0,0,0,200,201,202,203,204,205,206,207,208,209,210,
211,212,213,214,215,216,217,218,219,220,221,222,223,224,225,226,
227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,
243,244,245,246,247,248,0,0,0,0,0,0,0, };

struct input_event event;
InputEventWorker *worker = (InputEventWorker *)arg;

int touchAction = -1;
bool gestureUpdate = false;
int keycode;

while (!CORE.Window.shouldClose)
{

if (read(worker->fd, &event, sizeof(event)) == (int)sizeof(event))
{

if (event.type == EV_REL)
{
if (event.code == REL_X)
{
CORE.Input.Mouse.position.x += event.value;
CORE.Input.Touch.position[0].x = CORE.Input.Mouse.position.x;

#if defined(SUPPORT_GESTURES_SYSTEM)
touchAction = TOUCH_MOVE;
gestureUpdate = true;
#endif
}

if (event.code == REL_Y)
{
CORE.Input.Mouse.position.y += event.value;
CORE.Input.Touch.position[0].y = CORE.Input.Mouse.position.y;

#if defined(SUPPORT_GESTURES_SYSTEM)
touchAction = TOUCH_MOVE;
gestureUpdate = true;
#endif
}

if (event.code == REL_WHEEL) CORE.Input.Mouse.currentWheelMove += event.value;
}


if (event.type == EV_ABS)
{

if (event.code == ABS_X)
{
CORE.Input.Mouse.position.x = (event.value - worker->absRange.x)*CORE.Window.screen.width/worker->absRange.width; 

#if defined(SUPPORT_GESTURES_SYSTEM)
touchAction = TOUCH_MOVE;
gestureUpdate = true;
#endif
}

if (event.code == ABS_Y)
{
CORE.Input.Mouse.position.y = (event.value - worker->absRange.y)*CORE.Window.screen.height/worker->absRange.height; 

#if defined(SUPPORT_GESTURES_SYSTEM)
touchAction = TOUCH_MOVE;
gestureUpdate = true;
#endif
}


if (event.code == ABS_MT_SLOT) worker->touchSlot = event.value; 

if (event.code == ABS_MT_POSITION_X)
{
if (worker->touchSlot < MAX_TOUCH_POINTS) CORE.Input.Touch.position[worker->touchSlot].x = (event.value - worker->absRange.x)*CORE.Window.screen.width/worker->absRange.width; 
}

if (event.code == ABS_MT_POSITION_Y)
{
if (worker->touchSlot < MAX_TOUCH_POINTS) CORE.Input.Touch.position[worker->touchSlot].y = (event.value - worker->absRange.y)*CORE.Window.screen.height/worker->absRange.height; 
}

if (event.code == ABS_MT_TRACKING_ID)
{
if ((event.value < 0) && (worker->touchSlot < MAX_TOUCH_POINTS))
{

CORE.Input.Touch.position[worker->touchSlot].x = -1;
CORE.Input.Touch.position[worker->touchSlot].y = -1;
}
}
}


if (event.type == EV_KEY)
{

if ((event.code == BTN_TOUCH) || (event.code == BTN_LEFT))
{
CORE.Input.Mouse.currentButtonStateEvdev[MOUSE_LEFT_BUTTON] = event.value;

#if defined(SUPPORT_GESTURES_SYSTEM)
if (event.value > 0) touchAction = TOUCH_DOWN;
else touchAction = TOUCH_UP;
gestureUpdate = true;
#endif
}

if (event.code == BTN_RIGHT) CORE.Input.Mouse.currentButtonStateEvdev[MOUSE_RIGHT_BUTTON] = event.value;
if (event.code == BTN_MIDDLE) CORE.Input.Mouse.currentButtonStateEvdev[MOUSE_MIDDLE_BUTTON] = event.value;


if ((event.code >= 1) && (event.code <= 255)) 
{
keycode = keymap_US[event.code & 0xFF]; 


if ((keycode > 0) && (keycode < sizeof(CORE.Input.Keyboard.currentKeyState)))
{












CORE.Input.Keyboard.currentKeyState[keycode] = event.value;
if (event.value == 1)
{
CORE.Input.Keyboard.keyPressedQueue[CORE.Input.Keyboard.keyPressedQueueCount] = keycode; 
CORE.Input.Keyboard.keyPressedQueueCount++;
}

#if defined(SUPPORT_SCREEN_CAPTURE)

if (CORE.Input.Keyboard.currentKeyState[301] == 1)
{
TakeScreenshot(FormatText("screenshot%03i.png", screenshotCounter));
screenshotCounter++;
}
#endif

if (CORE.Input.Keyboard.currentKeyState[CORE.Input.Keyboard.exitKey] == 1) CORE.Window.shouldClose = true;

TRACELOGD("RPI: KEY_%s ScanCode: %4i KeyCode: %4i", event.value == 0 ? "UP":"DOWN", event.code, keycode);
}
}
}


if (CORE.Input.Mouse.position.x < 0) CORE.Input.Mouse.position.x = 0;
if (CORE.Input.Mouse.position.x > CORE.Window.screen.width/CORE.Input.Mouse.scale.x) CORE.Input.Mouse.position.x = CORE.Window.screen.width/CORE.Input.Mouse.scale.x;

if (CORE.Input.Mouse.position.y < 0) CORE.Input.Mouse.position.y = 0;
if (CORE.Input.Mouse.position.y > CORE.Window.screen.height/CORE.Input.Mouse.scale.y) CORE.Input.Mouse.position.y = CORE.Window.screen.height/CORE.Input.Mouse.scale.y;


if (gestureUpdate)
{
#if defined(SUPPORT_GESTURES_SYSTEM)
GestureEvent gestureEvent = { 0 };

gestureEvent.pointCount = 0;
gestureEvent.touchAction = touchAction;

if (CORE.Input.Touch.position[0].x >= 0) gestureEvent.pointCount++;
if (CORE.Input.Touch.position[1].x >= 0) gestureEvent.pointCount++;
if (CORE.Input.Touch.position[2].x >= 0) gestureEvent.pointCount++;
if (CORE.Input.Touch.position[3].x >= 0) gestureEvent.pointCount++;

gestureEvent.pointerId[0] = 0;
gestureEvent.pointerId[1] = 1;
gestureEvent.pointerId[2] = 2;
gestureEvent.pointerId[3] = 3;

gestureEvent.position[0] = CORE.Input.Touch.position[0];
gestureEvent.position[1] = CORE.Input.Touch.position[1];
gestureEvent.position[2] = CORE.Input.Touch.position[2];
gestureEvent.position[3] = CORE.Input.Touch.position[3];

ProcessGestureEvent(gestureEvent);
#endif
}
}
else
{
usleep(5000); 
}
}

close(worker->fd);

return NULL;
}


static void InitGamepad(void)
{
char gamepadDev[128] = "";

for (int i = 0; i < MAX_GAMEPADS; i++)
{
sprintf(gamepadDev, "%s%i", DEFAULT_GAMEPAD_DEV, i);

if ((CORE.Input.Gamepad.streamId[i] = open(gamepadDev, O_RDONLY|O_NONBLOCK)) < 0)
{

if (i == 0) TRACELOG(LOG_WARNING, "RPI: Failed to open Gamepad device, no gamepad available");
}
else
{
CORE.Input.Gamepad.ready[i] = true;


if (i == 0)
{
int error = pthread_create(&CORE.Input.Gamepad.threadId, NULL, &GamepadThread, NULL);

if (error != 0) TRACELOG(LOG_WARNING, "RPI: Failed to create gamepad input event thread");
else TRACELOG(LOG_INFO, "RPI: Gamepad device initialized successfully");
}
}
}
}


static void *GamepadThread(void *arg)
{
#define JS_EVENT_BUTTON 0x01 
#define JS_EVENT_AXIS 0x02 
#define JS_EVENT_INIT 0x80 

struct js_event {
unsigned int time; 
short value; 
unsigned char type; 
unsigned char number; 
};


struct js_event gamepadEvent;

while (!CORE.Window.shouldClose)
{
for (int i = 0; i < MAX_GAMEPADS; i++)
{
if (read(CORE.Input.Gamepad.streamId[i], &gamepadEvent, sizeof(struct js_event)) == (int)sizeof(struct js_event))
{
gamepadEvent.type &= ~JS_EVENT_INIT; 


if (gamepadEvent.type == JS_EVENT_BUTTON)
{
TRACELOGD("RPI: Gamepad button: %i, value: %i", gamepadEvent.number, gamepadEvent.value);

if (gamepadEvent.number < MAX_GAMEPAD_BUTTONS)
{

CORE.Input.Gamepad.currentState[i][gamepadEvent.number] = (int)gamepadEvent.value;

if ((int)gamepadEvent.value == 1) CORE.Input.Gamepad.lastButtonPressed = gamepadEvent.number;
else CORE.Input.Gamepad.lastButtonPressed = -1;
}
}
else if (gamepadEvent.type == JS_EVENT_AXIS)
{
TRACELOGD("RPI: Gamepad axis: %i, value: %i", gamepadEvent.number, gamepadEvent.value);

if (gamepadEvent.number < MAX_GAMEPAD_AXIS)
{

CORE.Input.Gamepad.axisState[i][gamepadEvent.number] = (float)gamepadEvent.value/32768;
}
}
}
else
{
usleep(1000); 
}
}
}

return NULL;
}
#endif 
