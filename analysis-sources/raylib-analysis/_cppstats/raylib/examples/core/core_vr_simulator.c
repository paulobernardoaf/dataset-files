#include "raylib.h"
#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION 330
#else 
#define GLSL_VERSION 100
#endif
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
SetConfigFlags(FLAG_MSAA_4X_HINT);
InitWindow(screenWidth, screenHeight, "raylib [core] example - vr simulator");
InitVrSimulator();
VrDeviceInfo hmd = { 0 }; 
hmd.hResolution = 2160; 
hmd.vResolution = 1200; 
hmd.hScreenSize = 0.133793f; 
hmd.vScreenSize = 0.0669f; 
hmd.vScreenCenter = 0.04678f; 
hmd.eyeToScreenDistance = 0.041f; 
hmd.lensSeparationDistance = 0.07f; 
hmd.interpupillaryDistance = 0.07f; 
hmd.lensDistortionValues[0] = 1.0f; 
hmd.lensDistortionValues[1] = 0.22f; 
hmd.lensDistortionValues[2] = 0.24f; 
hmd.lensDistortionValues[3] = 0.0f; 
hmd.chromaAbCorrection[0] = 0.996f; 
hmd.chromaAbCorrection[1] = -0.004f; 
hmd.chromaAbCorrection[2] = 1.014f; 
hmd.chromaAbCorrection[3] = 0.0f; 
Shader distortion = LoadShader(0, FormatText("resources/distortion%i.fs", GLSL_VERSION));
SetVrConfiguration(hmd, distortion); 
Camera camera = { 0 };
camera.position = (Vector3){ 5.0f, 2.0f, 5.0f }; 
camera.target = (Vector3){ 0.0f, 2.0f, 0.0f }; 
camera.up = (Vector3){ 0.0f, 1.0f, 0.0f }; 
camera.fovy = 60.0f; 
camera.type = CAMERA_PERSPECTIVE; 
Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };
SetCameraMode(camera, CAMERA_FIRST_PERSON); 
SetTargetFPS(90); 
while (!WindowShouldClose()) 
{
UpdateCamera(&camera); 
if (IsKeyPressed(KEY_SPACE)) ToggleVrMode(); 
BeginDrawing();
ClearBackground(RAYWHITE);
BeginVrDrawing();
BeginMode3D(camera);
DrawCube(cubePosition, 2.0f, 2.0f, 2.0f, RED);
DrawCubeWires(cubePosition, 2.0f, 2.0f, 2.0f, MAROON);
DrawGrid(40, 1.0f);
EndMode3D();
EndVrDrawing();
DrawFPS(10, 10);
EndDrawing();
}
UnloadShader(distortion); 
CloseVrSimulator(); 
CloseWindow(); 
return 0;
}