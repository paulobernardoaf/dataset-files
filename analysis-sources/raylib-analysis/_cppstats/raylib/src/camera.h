#if defined(CAMERA_STANDALONE)
typedef struct Vector2 {
float x;
float y;
} Vector2;
typedef struct Vector3 {
float x;
float y;
float z;
} Vector3;
typedef struct Camera3D {
Vector3 position; 
Vector3 target; 
Vector3 up; 
float fovy; 
int type; 
} Camera3D;
typedef Camera3D Camera; 
typedef enum {
CAMERA_CUSTOM = 0,
CAMERA_FREE,
CAMERA_ORBITAL,
CAMERA_FIRST_PERSON,
CAMERA_THIRD_PERSON
} CameraMode;
typedef enum {
CAMERA_PERSPECTIVE = 0,
CAMERA_ORTHOGRAPHIC
} CameraType;
#endif
#if defined(__cplusplus)
extern "C" { 
#endif
#if defined(CAMERA_STANDALONE)
void SetCameraMode(Camera camera, int mode); 
void UpdateCamera(Camera *camera); 
void SetCameraPanControl(int panKey); 
void SetCameraAltControl(int altKey); 
void SetCameraSmoothZoomControl(int szoomKey); 
void SetCameraMoveControls(int frontKey, int backKey,
int rightKey, int leftKey,
int upKey, int downKey); 
#endif
#if defined(__cplusplus)
}
#endif
#if defined(CAMERA_IMPLEMENTATION)
#include <math.h> 
#if !defined(PI)
#define PI 3.14159265358979323846
#endif
#if !defined(DEG2RAD)
#define DEG2RAD (PI/180.0f)
#endif
#if !defined(RAD2DEG)
#define RAD2DEG (180.0f/PI)
#endif
#define CAMERA_MOUSE_MOVE_SENSITIVITY 0.003f
#define CAMERA_MOUSE_SCROLL_SENSITIVITY 1.5f
#define CAMERA_FREE_MOUSE_SENSITIVITY 0.01f
#define CAMERA_FREE_DISTANCE_MIN_CLAMP 0.3f
#define CAMERA_FREE_DISTANCE_MAX_CLAMP 120.0f
#define CAMERA_FREE_MIN_CLAMP 85.0f
#define CAMERA_FREE_MAX_CLAMP -85.0f
#define CAMERA_FREE_SMOOTH_ZOOM_SENSITIVITY 0.05f
#define CAMERA_FREE_PANNING_DIVIDER 5.1f
#define CAMERA_ORBITAL_SPEED 0.01f 
#define CAMERA_FIRST_PERSON_FOCUS_DISTANCE 25.0f
#define CAMERA_FIRST_PERSON_MIN_CLAMP 89.0f
#define CAMERA_FIRST_PERSON_MAX_CLAMP -89.0f
#define CAMERA_FIRST_PERSON_STEP_TRIGONOMETRIC_DIVIDER 5.0f
#define CAMERA_FIRST_PERSON_STEP_DIVIDER 30.0f
#define CAMERA_FIRST_PERSON_WAVING_DIVIDER 200.0f
#define CAMERA_THIRD_PERSON_DISTANCE_CLAMP 1.2f
#define CAMERA_THIRD_PERSON_MIN_CLAMP 5.0f
#define CAMERA_THIRD_PERSON_MAX_CLAMP -85.0f
#define CAMERA_THIRD_PERSON_OFFSET (Vector3){ 0.4f, 0.0f, 0.0f }
#define PLAYER_MOVEMENT_SENSITIVITY 20.0f
typedef enum {
MOVE_FRONT = 0,
MOVE_BACK,
MOVE_RIGHT,
MOVE_LEFT,
MOVE_UP,
MOVE_DOWN
} CameraMove;
typedef struct {
int mode; 
float targetDistance; 
float playerEyesPosition; 
Vector2 angle; 
int moveControl[6];
int smoothZoomControl; 
int altControl; 
int panControl; 
} CameraData;
static CameraData CAMERA = { 
.mode = 0,
.targetDistance = 0,
.playerEyesPosition = 1.85f,
.angle = { 0 },
.moveControl = { 'W', 'S', 'D', 'A', 'E', 'Q' },
.smoothZoomControl = 341,
.altControl = 342,
.panControl = 2
};
#if defined(CAMERA_STANDALONE)
static void EnableCursor() {} 
static void DisableCursor() {} 
static int IsKeyDown(int key) { return 0; }
static int IsMouseButtonDown(int button) { return 0;}
static int GetMouseWheelMove() { return 0; }
static Vector2 GetMousePosition() { return (Vector2){ 0.0f, 0.0f }; }
#endif
void SetCameraMode(Camera camera, int mode)
{
Vector3 v1 = camera.position;
Vector3 v2 = camera.target;
float dx = v2.x - v1.x;
float dy = v2.y - v1.y;
float dz = v2.z - v1.z;
CAMERA.targetDistance = sqrtf(dx*dx + dy*dy + dz*dz);
CAMERA.angle.x = atan2f(dx, dz); 
CAMERA.angle.y = atan2f(dy, sqrtf(dx*dx + dz*dz)); 
CAMERA.playerEyesPosition = camera.position.y;
if ((mode == CAMERA_FIRST_PERSON) || (mode == CAMERA_THIRD_PERSON)) DisableCursor();
else EnableCursor();
CAMERA.mode = mode;
}
void UpdateCamera(Camera *camera)
{
static int swingCounter = 0; 
static Vector2 previousMousePosition = { 0.0f, 0.0f };
Vector2 mousePositionDelta = { 0.0f, 0.0f };
Vector2 mousePosition = GetMousePosition();
int mouseWheelMove = GetMouseWheelMove();
bool panKey = IsMouseButtonDown(CAMERA.panControl);
bool altKey = IsKeyDown(CAMERA.altControl);
bool szoomKey = IsKeyDown(CAMERA.smoothZoomControl);
bool direction[6] = { IsKeyDown(CAMERA.moveControl[MOVE_FRONT]),
IsKeyDown(CAMERA.moveControl[MOVE_BACK]),
IsKeyDown(CAMERA.moveControl[MOVE_RIGHT]),
IsKeyDown(CAMERA.moveControl[MOVE_LEFT]),
IsKeyDown(CAMERA.moveControl[MOVE_UP]),
IsKeyDown(CAMERA.moveControl[MOVE_DOWN]) };
if (CAMERA.mode != CAMERA_CUSTOM)
{
mousePositionDelta.x = mousePosition.x - previousMousePosition.x;
mousePositionDelta.y = mousePosition.y - previousMousePosition.y;
previousMousePosition = mousePosition;
}
switch (CAMERA.mode)
{
case CAMERA_FREE: 
{
if ((CAMERA.targetDistance < CAMERA_FREE_DISTANCE_MAX_CLAMP) && (mouseWheelMove < 0))
{
CAMERA.targetDistance -= (mouseWheelMove*CAMERA_MOUSE_SCROLL_SENSITIVITY);
if (CAMERA.targetDistance > CAMERA_FREE_DISTANCE_MAX_CLAMP) CAMERA.targetDistance = CAMERA_FREE_DISTANCE_MAX_CLAMP;
}
else if ((camera->position.y > camera->target.y) && (CAMERA.targetDistance == CAMERA_FREE_DISTANCE_MAX_CLAMP) && (mouseWheelMove < 0))
{
camera->target.x += mouseWheelMove*(camera->target.x - camera->position.x)*CAMERA_MOUSE_SCROLL_SENSITIVITY/CAMERA.targetDistance;
camera->target.y += mouseWheelMove*(camera->target.y - camera->position.y)*CAMERA_MOUSE_SCROLL_SENSITIVITY/CAMERA.targetDistance;
camera->target.z += mouseWheelMove*(camera->target.z - camera->position.z)*CAMERA_MOUSE_SCROLL_SENSITIVITY/CAMERA.targetDistance;
}
else if ((camera->position.y > camera->target.y) && (camera->target.y >= 0))
{
camera->target.x += mouseWheelMove*(camera->target.x - camera->position.x)*CAMERA_MOUSE_SCROLL_SENSITIVITY/CAMERA.targetDistance;
camera->target.y += mouseWheelMove*(camera->target.y - camera->position.y)*CAMERA_MOUSE_SCROLL_SENSITIVITY/CAMERA.targetDistance;
camera->target.z += mouseWheelMove*(camera->target.z - camera->position.z)*CAMERA_MOUSE_SCROLL_SENSITIVITY/CAMERA.targetDistance;
}
else if ((camera->position.y > camera->target.y) && (camera->target.y < 0) && (mouseWheelMove > 0))
{
CAMERA.targetDistance -= (mouseWheelMove*CAMERA_MOUSE_SCROLL_SENSITIVITY);
if (CAMERA.targetDistance < CAMERA_FREE_DISTANCE_MIN_CLAMP) CAMERA.targetDistance = CAMERA_FREE_DISTANCE_MIN_CLAMP;
}
else if ((camera->position.y < camera->target.y) && (CAMERA.targetDistance == CAMERA_FREE_DISTANCE_MAX_CLAMP) && (mouseWheelMove < 0))
{
camera->target.x += mouseWheelMove*(camera->target.x - camera->position.x)*CAMERA_MOUSE_SCROLL_SENSITIVITY/CAMERA.targetDistance;
camera->target.y += mouseWheelMove*(camera->target.y - camera->position.y)*CAMERA_MOUSE_SCROLL_SENSITIVITY/CAMERA.targetDistance;
camera->target.z += mouseWheelMove*(camera->target.z - camera->position.z)*CAMERA_MOUSE_SCROLL_SENSITIVITY/CAMERA.targetDistance;
}
else if ((camera->position.y < camera->target.y) && (camera->target.y <= 0))
{
camera->target.x += mouseWheelMove*(camera->target.x - camera->position.x)*CAMERA_MOUSE_SCROLL_SENSITIVITY/CAMERA.targetDistance;
camera->target.y += mouseWheelMove*(camera->target.y - camera->position.y)*CAMERA_MOUSE_SCROLL_SENSITIVITY/CAMERA.targetDistance;
camera->target.z += mouseWheelMove*(camera->target.z - camera->position.z)*CAMERA_MOUSE_SCROLL_SENSITIVITY/CAMERA.targetDistance;
}
else if ((camera->position.y < camera->target.y) && (camera->target.y > 0) && (mouseWheelMove > 0))
{
CAMERA.targetDistance -= (mouseWheelMove*CAMERA_MOUSE_SCROLL_SENSITIVITY);
if (CAMERA.targetDistance < CAMERA_FREE_DISTANCE_MIN_CLAMP) CAMERA.targetDistance = CAMERA_FREE_DISTANCE_MIN_CLAMP;
}
if (panKey)
{
if (altKey) 
{
if (szoomKey)
{
CAMERA.targetDistance += (mousePositionDelta.y*CAMERA_FREE_SMOOTH_ZOOM_SENSITIVITY);
}
else
{
CAMERA.angle.x += mousePositionDelta.x*-CAMERA_FREE_MOUSE_SENSITIVITY;
CAMERA.angle.y += mousePositionDelta.y*-CAMERA_FREE_MOUSE_SENSITIVITY;
if (CAMERA.angle.y > CAMERA_FREE_MIN_CLAMP*DEG2RAD) CAMERA.angle.y = CAMERA_FREE_MIN_CLAMP*DEG2RAD;
else if (CAMERA.angle.y < CAMERA_FREE_MAX_CLAMP*DEG2RAD) CAMERA.angle.y = CAMERA_FREE_MAX_CLAMP*DEG2RAD;
}
}
else
{
camera->target.x += ((mousePositionDelta.x*CAMERA_FREE_MOUSE_SENSITIVITY)*cosf(CAMERA.angle.x) + (mousePositionDelta.y*CAMERA_FREE_MOUSE_SENSITIVITY)*sinf(CAMERA.angle.x)*sinf(CAMERA.angle.y))*(CAMERA.targetDistance/CAMERA_FREE_PANNING_DIVIDER);
camera->target.y += ((mousePositionDelta.y*CAMERA_FREE_MOUSE_SENSITIVITY)*cosf(CAMERA.angle.y))*(CAMERA.targetDistance/CAMERA_FREE_PANNING_DIVIDER);
camera->target.z += ((mousePositionDelta.x*-CAMERA_FREE_MOUSE_SENSITIVITY)*sinf(CAMERA.angle.x) + (mousePositionDelta.y*CAMERA_FREE_MOUSE_SENSITIVITY)*cosf(CAMERA.angle.x)*sinf(CAMERA.angle.y))*(CAMERA.targetDistance/CAMERA_FREE_PANNING_DIVIDER);
}
}
camera->position.x = -sinf(CAMERA.angle.x)*CAMERA.targetDistance*cosf(CAMERA.angle.y) + camera->target.x;
camera->position.y = -sinf(CAMERA.angle.y)*CAMERA.targetDistance + camera->target.y;
camera->position.z = -cosf(CAMERA.angle.x)*CAMERA.targetDistance*cosf(CAMERA.angle.y) + camera->target.z;
} break;
case CAMERA_ORBITAL: 
{
CAMERA.angle.x += CAMERA_ORBITAL_SPEED; 
CAMERA.targetDistance -= (mouseWheelMove*CAMERA_MOUSE_SCROLL_SENSITIVITY); 
if (CAMERA.targetDistance < CAMERA_THIRD_PERSON_DISTANCE_CLAMP) CAMERA.targetDistance = CAMERA_THIRD_PERSON_DISTANCE_CLAMP;
camera->position.x = sinf(CAMERA.angle.x)*CAMERA.targetDistance*cosf(CAMERA.angle.y) + camera->target.x;
camera->position.y = ((CAMERA.angle.y <= 0.0f)? 1 : -1)*sinf(CAMERA.angle.y)*CAMERA.targetDistance*sinf(CAMERA.angle.y) + camera->target.y;
camera->position.z = cosf(CAMERA.angle.x)*CAMERA.targetDistance*cosf(CAMERA.angle.y) + camera->target.z;
} break;
case CAMERA_FIRST_PERSON: 
{
camera->position.x += (sinf(CAMERA.angle.x)*direction[MOVE_BACK] -
sinf(CAMERA.angle.x)*direction[MOVE_FRONT] -
cosf(CAMERA.angle.x)*direction[MOVE_LEFT] +
cosf(CAMERA.angle.x)*direction[MOVE_RIGHT])/PLAYER_MOVEMENT_SENSITIVITY;
camera->position.y += (sinf(CAMERA.angle.y)*direction[MOVE_FRONT] -
sinf(CAMERA.angle.y)*direction[MOVE_BACK] +
1.0f*direction[MOVE_UP] - 1.0f*direction[MOVE_DOWN])/PLAYER_MOVEMENT_SENSITIVITY;
camera->position.z += (cosf(CAMERA.angle.x)*direction[MOVE_BACK] -
cosf(CAMERA.angle.x)*direction[MOVE_FRONT] +
sinf(CAMERA.angle.x)*direction[MOVE_LEFT] -
sinf(CAMERA.angle.x)*direction[MOVE_RIGHT])/PLAYER_MOVEMENT_SENSITIVITY;
CAMERA.angle.x += (mousePositionDelta.x*-CAMERA_MOUSE_MOVE_SENSITIVITY);
CAMERA.angle.y += (mousePositionDelta.y*-CAMERA_MOUSE_MOVE_SENSITIVITY);
if (CAMERA.angle.y > CAMERA_FIRST_PERSON_MIN_CLAMP*DEG2RAD) CAMERA.angle.y = CAMERA_FIRST_PERSON_MIN_CLAMP*DEG2RAD;
else if (CAMERA.angle.y < CAMERA_FIRST_PERSON_MAX_CLAMP*DEG2RAD) CAMERA.angle.y = CAMERA_FIRST_PERSON_MAX_CLAMP*DEG2RAD;
Matrix translation = MatrixTranslate(0, 0, (CAMERA.targetDistance/CAMERA_FREE_PANNING_DIVIDER));
Matrix rotation = MatrixRotateXYZ((Vector3){ PI*2 - CAMERA.angle.y, PI*2 - CAMERA.angle.x, 0 });
Matrix transform = MatrixMultiply(translation, rotation);
camera->target.x = camera->position.x - transform.m12;
camera->target.y = camera->position.y - transform.m13;
camera->target.z = camera->position.z - transform.m14;
for (int i = 0; i < 6; i++) if (direction[i]) { swingCounter++; break; }
camera->position.y = CAMERA.playerEyesPosition - sinf(swingCounter/CAMERA_FIRST_PERSON_STEP_TRIGONOMETRIC_DIVIDER)/CAMERA_FIRST_PERSON_STEP_DIVIDER;
camera->up.x = sinf(swingCounter/(CAMERA_FIRST_PERSON_STEP_TRIGONOMETRIC_DIVIDER*2))/CAMERA_FIRST_PERSON_WAVING_DIVIDER;
camera->up.z = -sinf(swingCounter/(CAMERA_FIRST_PERSON_STEP_TRIGONOMETRIC_DIVIDER*2))/CAMERA_FIRST_PERSON_WAVING_DIVIDER;
} break;
case CAMERA_THIRD_PERSON: 
{
camera->position.x += (sinf(CAMERA.angle.x)*direction[MOVE_BACK] -
sinf(CAMERA.angle.x)*direction[MOVE_FRONT] -
cosf(CAMERA.angle.x)*direction[MOVE_LEFT] +
cosf(CAMERA.angle.x)*direction[MOVE_RIGHT])/PLAYER_MOVEMENT_SENSITIVITY;
camera->position.y += (sinf(CAMERA.angle.y)*direction[MOVE_FRONT] -
sinf(CAMERA.angle.y)*direction[MOVE_BACK] +
1.0f*direction[MOVE_UP] - 1.0f*direction[MOVE_DOWN])/PLAYER_MOVEMENT_SENSITIVITY;
camera->position.z += (cosf(CAMERA.angle.x)*direction[MOVE_BACK] -
cosf(CAMERA.angle.x)*direction[MOVE_FRONT] +
sinf(CAMERA.angle.x)*direction[MOVE_LEFT] -
sinf(CAMERA.angle.x)*direction[MOVE_RIGHT])/PLAYER_MOVEMENT_SENSITIVITY;
CAMERA.angle.x += (mousePositionDelta.x*-CAMERA_MOUSE_MOVE_SENSITIVITY);
CAMERA.angle.y += (mousePositionDelta.y*-CAMERA_MOUSE_MOVE_SENSITIVITY);
if (CAMERA.angle.y > CAMERA_THIRD_PERSON_MIN_CLAMP*DEG2RAD) CAMERA.angle.y = CAMERA_THIRD_PERSON_MIN_CLAMP*DEG2RAD;
else if (CAMERA.angle.y < CAMERA_THIRD_PERSON_MAX_CLAMP*DEG2RAD) CAMERA.angle.y = CAMERA_THIRD_PERSON_MAX_CLAMP*DEG2RAD;
CAMERA.targetDistance -= (mouseWheelMove*CAMERA_MOUSE_SCROLL_SENSITIVITY);
if (CAMERA.targetDistance < CAMERA_THIRD_PERSON_DISTANCE_CLAMP) CAMERA.targetDistance = CAMERA_THIRD_PERSON_DISTANCE_CLAMP;
camera->position.x = sinf(CAMERA.angle.x)*CAMERA.targetDistance*cosf(CAMERA.angle.y) + camera->target.x;
if (CAMERA.angle.y <= 0.0f) camera->position.y = sinf(CAMERA.angle.y)*CAMERA.targetDistance*sinf(CAMERA.angle.y) + camera->target.y;
else camera->position.y = -sinf(CAMERA.angle.y)*CAMERA.targetDistance*sinf(CAMERA.angle.y) + camera->target.y;
camera->position.z = cosf(CAMERA.angle.x)*CAMERA.targetDistance*cosf(CAMERA.angle.y) + camera->target.z;
} break;
case CAMERA_CUSTOM: break;
default: break;
}
}
void SetCameraPanControl(int panKey) { CAMERA.panControl = panKey; }
void SetCameraAltControl(int altKey) { CAMERA.altControl = altKey; }
void SetCameraSmoothZoomControl(int szoomKey) { CAMERA.smoothZoomControl = szoomKey; }
void SetCameraMoveControls(int frontKey, int backKey, int rightKey, int leftKey, int upKey, int downKey)
{
CAMERA.moveControl[MOVE_FRONT] = frontKey;
CAMERA.moveControl[MOVE_BACK] = backKey;
CAMERA.moveControl[MOVE_RIGHT] = rightKey;
CAMERA.moveControl[MOVE_LEFT] = leftKey;
CAMERA.moveControl[MOVE_UP] = upKey;
CAMERA.moveControl[MOVE_DOWN] = downKey;
}
#endif 
