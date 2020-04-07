












































#if !defined(GESTURES_H)
#define GESTURES_H

#if !defined(PI)
#define PI 3.14159265358979323846
#endif










#if defined(GESTURES_STANDALONE)
#if !defined(__cplusplus)

typedef enum { false, true } bool;
#endif


typedef struct Vector2 {
float x;
float y;
} Vector2;



typedef enum {
GESTURE_NONE = 0,
GESTURE_TAP = 1,
GESTURE_DOUBLETAP = 2,
GESTURE_HOLD = 4,
GESTURE_DRAG = 8,
GESTURE_SWIPE_RIGHT = 16,
GESTURE_SWIPE_LEFT = 32,
GESTURE_SWIPE_UP = 64,
GESTURE_SWIPE_DOWN = 128,
GESTURE_PINCH_IN = 256,
GESTURE_PINCH_OUT = 512
} Gestures;
#endif

typedef enum { TOUCH_UP, TOUCH_DOWN, TOUCH_MOVE } TouchAction;



typedef struct {
int touchAction;
int pointCount;
int pointerId[4];
Vector2 position[4];
} GestureEvent;

#if defined(__cplusplus)
extern "C" { 
#endif









void ProcessGestureEvent(GestureEvent event); 
void UpdateGestures(void); 

#if defined(GESTURES_STANDALONE)
void SetGesturesEnabled(unsigned int gestureFlags); 
bool IsGestureDetected(int gesture); 
int GetGestureDetected(void); 
int GetTouchPointsCount(void); 
float GetGestureHoldDuration(void); 
Vector2 GetGestureDragVector(void); 
float GetGestureDragAngle(void); 
Vector2 GetGesturePinchVector(void); 
float GetGesturePinchAngle(void); 
#endif

#if defined(__cplusplus)
}
#endif

#endif 







#if defined(GESTURES_IMPLEMENTATION)

#if defined(_WIN32)

int __stdcall QueryPerformanceCounter(unsigned long long int *lpPerformanceCount);
int __stdcall QueryPerformanceFrequency(unsigned long long int *lpFrequency);
#elif defined(__linux__)
#if _POSIX_C_SOURCE < 199309L
#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L 
#endif
#include <sys/time.h> 
#include <time.h> 

#include <math.h> 
#endif

#if defined(__APPLE__) 
#include <mach/clock.h> 
#include <mach/mach.h> 
#endif




#define FORCE_TO_SWIPE 0.0005f 
#define MINIMUM_DRAG 0.015f 
#define MINIMUM_PINCH 0.005f 
#define TAP_TIMEOUT 300 
#define PINCH_TIMEOUT 300 
#define DOUBLETAP_RANGE 0.03f 





typedef struct {
int current; 
unsigned int enabledFlags; 
struct {
int firstId; 
int pointCount; 
double eventTime; 
Vector2 upPosition; 
Vector2 downPositionA; 
Vector2 downPositionB; 
Vector2 downDragPosition; 
Vector2 moveDownPositionA; 
Vector2 moveDownPositionB; 
int tapCounter; 
} Touch;
struct {
bool resetRequired; 
double timeDuration; 
} Hold;
struct {
Vector2 vector; 
float angle; 
float distance; 
float intensity; 
} Drag;
struct {
bool start; 
double timeDuration; 
} Swipe;
struct {
Vector2 vector; 
float angle; 
float distance; 
} Pinch;
} GesturesData;




static GesturesData GESTURES = {
.Touch.firstId = -1,
.current = GESTURE_NONE,
.enabledFlags = 0b0000001111111111 
};




#if defined(GESTURES_STANDALONE)

static float Vector2Angle(Vector2 initialPosition, Vector2 finalPosition);
static float Vector2Distance(Vector2 v1, Vector2 v2);
#endif
static double GetCurrentTime(void);






void SetGesturesEnabled(unsigned int gestureFlags)
{
GESTURES.enabledFlags = gestureFlags;
}


bool IsGestureDetected(int gesture)
{
if ((GESTURES.enabledFlags & GESTURES.current) == gesture) return true;
else return false;
}


void ProcessGestureEvent(GestureEvent event)
{

GESTURES.Touch.pointCount = event.pointCount; 

if (GESTURES.Touch.pointCount < 2)
{
if (event.touchAction == TOUCH_DOWN)
{
GESTURES.Touch.tapCounter++; 


if ((GESTURES.current == GESTURE_NONE) && (GESTURES.Touch.tapCounter >= 2) && ((GetCurrentTime() - GESTURES.Touch.eventTime) < TAP_TIMEOUT) && (Vector2Distance(GESTURES.Touch.downPositionA, event.position[0]) < DOUBLETAP_RANGE))
{
GESTURES.current = GESTURE_DOUBLETAP;
GESTURES.Touch.tapCounter = 0;
}
else 
{
GESTURES.Touch.tapCounter = 1;
GESTURES.current = GESTURE_TAP;
}

GESTURES.Touch.downPositionA = event.position[0];
GESTURES.Touch.downDragPosition = event.position[0];

GESTURES.Touch.upPosition = GESTURES.Touch.downPositionA;
GESTURES.Touch.eventTime = GetCurrentTime();

GESTURES.Touch.firstId = event.pointerId[0];

GESTURES.Drag.vector = (Vector2){ 0.0f, 0.0f };
}
else if (event.touchAction == TOUCH_UP)
{
if (GESTURES.current == GESTURE_DRAG) GESTURES.Touch.upPosition = event.position[0];


GESTURES.Drag.distance = Vector2Distance(GESTURES.Touch.downPositionA, GESTURES.Touch.upPosition);
GESTURES.Drag.intensity = GESTURES.Drag.distance/(float)((GetCurrentTime() - GESTURES.Swipe.timeDuration));

GESTURES.Swipe.start = false;


if ((GESTURES.Drag.intensity > FORCE_TO_SWIPE) && (GESTURES.Touch.firstId == event.pointerId[0]))
{

GESTURES.Drag.angle = 360.0f - Vector2Angle(GESTURES.Touch.downPositionA, GESTURES.Touch.upPosition);

if ((GESTURES.Drag.angle < 30) || (GESTURES.Drag.angle > 330)) GESTURES.current = GESTURE_SWIPE_RIGHT; 
else if ((GESTURES.Drag.angle > 30) && (GESTURES.Drag.angle < 120)) GESTURES.current = GESTURE_SWIPE_UP; 
else if ((GESTURES.Drag.angle > 120) && (GESTURES.Drag.angle < 210)) GESTURES.current = GESTURE_SWIPE_LEFT; 
else if ((GESTURES.Drag.angle > 210) && (GESTURES.Drag.angle < 300)) GESTURES.current = GESTURE_SWIPE_DOWN; 
else GESTURES.current = GESTURE_NONE;
}
else
{
GESTURES.Drag.distance = 0.0f;
GESTURES.Drag.intensity = 0.0f;
GESTURES.Drag.angle = 0.0f;

GESTURES.current = GESTURE_NONE;
}

GESTURES.Touch.downDragPosition = (Vector2){ 0.0f, 0.0f };
GESTURES.Touch.pointCount = 0;
}
else if (event.touchAction == TOUCH_MOVE)
{
if (GESTURES.current == GESTURE_DRAG) GESTURES.Touch.eventTime = GetCurrentTime();

if (!GESTURES.Swipe.start)
{
GESTURES.Swipe.timeDuration = GetCurrentTime();
GESTURES.Swipe.start = true;
}

GESTURES.Touch.moveDownPositionA = event.position[0];

if (GESTURES.current == GESTURE_HOLD)
{
if (GESTURES.Hold.resetRequired) GESTURES.Touch.downPositionA = event.position[0];

GESTURES.Hold.resetRequired = false;


if (Vector2Distance(GESTURES.Touch.downPositionA, GESTURES.Touch.moveDownPositionA) >= MINIMUM_DRAG)
{
GESTURES.Touch.eventTime = GetCurrentTime();
GESTURES.current = GESTURE_DRAG;
}
}

GESTURES.Drag.vector.x = GESTURES.Touch.moveDownPositionA.x - GESTURES.Touch.downDragPosition.x;
GESTURES.Drag.vector.y = GESTURES.Touch.moveDownPositionA.y - GESTURES.Touch.downDragPosition.y;
}
}
else 
{
if (event.touchAction == TOUCH_DOWN)
{
GESTURES.Touch.downPositionA = event.position[0];
GESTURES.Touch.downPositionB = event.position[1];



GESTURES.Pinch.vector.x = GESTURES.Touch.downPositionB.x - GESTURES.Touch.downPositionA.x;
GESTURES.Pinch.vector.y = GESTURES.Touch.downPositionB.y - GESTURES.Touch.downPositionA.y;

GESTURES.current = GESTURE_HOLD;
GESTURES.Hold.timeDuration = GetCurrentTime();
}
else if (event.touchAction == TOUCH_MOVE)
{
GESTURES.Pinch.distance = Vector2Distance(GESTURES.Touch.moveDownPositionA, GESTURES.Touch.moveDownPositionB);

GESTURES.Touch.downPositionA = GESTURES.Touch.moveDownPositionA;
GESTURES.Touch.downPositionB = GESTURES.Touch.moveDownPositionB;

GESTURES.Touch.moveDownPositionA = event.position[0];
GESTURES.Touch.moveDownPositionB = event.position[1];

GESTURES.Pinch.vector.x = GESTURES.Touch.moveDownPositionB.x - GESTURES.Touch.moveDownPositionA.x;
GESTURES.Pinch.vector.y = GESTURES.Touch.moveDownPositionB.y - GESTURES.Touch.moveDownPositionA.y;

if ((Vector2Distance(GESTURES.Touch.downPositionA, GESTURES.Touch.moveDownPositionA) >= MINIMUM_PINCH) || (Vector2Distance(GESTURES.Touch.downPositionB, GESTURES.Touch.moveDownPositionB) >= MINIMUM_PINCH))
{
if ((Vector2Distance(GESTURES.Touch.moveDownPositionA, GESTURES.Touch.moveDownPositionB) - GESTURES.Pinch.distance) < 0) GESTURES.current = GESTURE_PINCH_IN;
else GESTURES.current = GESTURE_PINCH_OUT;
}
else
{
GESTURES.current = GESTURE_HOLD;
GESTURES.Hold.timeDuration = GetCurrentTime();
}


GESTURES.Pinch.angle = 360.0f - Vector2Angle(GESTURES.Touch.moveDownPositionA, GESTURES.Touch.moveDownPositionB);
}
else if (event.touchAction == TOUCH_UP)
{
GESTURES.Pinch.distance = 0.0f;
GESTURES.Pinch.angle = 0.0f;
GESTURES.Pinch.vector = (Vector2){ 0.0f, 0.0f };
GESTURES.Touch.pointCount = 0;

GESTURES.current = GESTURE_NONE;
}
}
}


void UpdateGestures(void)
{



if (((GESTURES.current == GESTURE_TAP) || (GESTURES.current == GESTURE_DOUBLETAP)) && (GESTURES.Touch.pointCount < 2))
{
GESTURES.current = GESTURE_HOLD;
GESTURES.Hold.timeDuration = GetCurrentTime();
}

if (((GetCurrentTime() - GESTURES.Touch.eventTime) > TAP_TIMEOUT) && (GESTURES.current == GESTURE_DRAG) && (GESTURES.Touch.pointCount < 2))
{
GESTURES.current = GESTURE_HOLD;
GESTURES.Hold.timeDuration = GetCurrentTime();
GESTURES.Hold.resetRequired = true;
}


if ((GESTURES.current == GESTURE_SWIPE_RIGHT) || (GESTURES.current == GESTURE_SWIPE_UP) || (GESTURES.current == GESTURE_SWIPE_LEFT) || (GESTURES.current == GESTURE_SWIPE_DOWN))
{
GESTURES.current = GESTURE_NONE;
}
}


int GetTouchPointsCount(void)
{


return GESTURES.Touch.pointCount;
}


int GetGestureDetected(void)
{

return (GESTURES.enabledFlags & GESTURES.current);
}


float GetGestureHoldDuration(void)
{


double time = 0.0;

if (GESTURES.current == GESTURE_HOLD) time = GetCurrentTime() - GESTURES.Hold.timeDuration;

return (float)time;
}


Vector2 GetGestureDragVector(void)
{


return GESTURES.Drag.vector;
}



float GetGestureDragAngle(void)
{


return GESTURES.Drag.angle;
}


Vector2 GetGesturePinchVector(void)
{



return GESTURES.Pinch.vector;
}



float GetGesturePinchAngle(void)
{


return GESTURES.Pinch.angle;
}




#if defined(GESTURES_STANDALONE)

static float Vector2Angle(Vector2 v1, Vector2 v2)
{
float angle = atan2f(v2.y - v1.y, v2.x - v1.x)*(180.0f/PI);

if (angle < 0) angle += 360.0f;

return angle;
}


static float Vector2Distance(Vector2 v1, Vector2 v2)
{
float result;

float dx = v2.x - v1.x;
float dy = v2.y - v1.y;

result = (float)sqrt(dx*dx + dy*dy);

return result;
}
#endif


static double GetCurrentTime(void)
{
double time = 0;

#if defined(_WIN32)
unsigned long long int clockFrequency, currentTime;

QueryPerformanceFrequency(&clockFrequency); 
QueryPerformanceCounter(&currentTime);

time = (double)currentTime/clockFrequency*1000.0f; 
#endif

#if defined(__linux__)

struct timespec now;
clock_gettime(CLOCK_MONOTONIC, &now);
unsigned long long int nowTime = (unsigned long long int)now.tv_sec*1000000000LLU + (unsigned long long int)now.tv_nsec; 

time = ((double)nowTime/1000000.0); 
#endif

#if defined(__APPLE__)



clock_serv_t cclock;
mach_timespec_t now;
host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock);


clock_get_time(cclock, &now);
mach_port_deallocate(mach_task_self(), cclock);
unsigned long long int nowTime = (unsigned long long int)now.tv_sec*1000000000LLU + (unsigned long long int)now.tv_nsec; 

time = ((double)nowTime/1000000.0); 
#endif

return time;
}

#endif 
