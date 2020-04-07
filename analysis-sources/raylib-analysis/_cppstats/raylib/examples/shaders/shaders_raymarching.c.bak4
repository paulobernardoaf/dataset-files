













#include "raylib.h"

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION 330
#else 
#define GLSL_VERSION 100
#endif

int main(void)
{


int screenWidth = 800;
int screenHeight = 450;

SetConfigFlags(FLAG_WINDOW_RESIZABLE);
InitWindow(screenWidth, screenHeight, "raylib [shaders] example - raymarching shapes");

Camera camera = { 0 };
camera.position = (Vector3){ 2.5f, 2.5f, 3.0f }; 
camera.target = (Vector3){ 0.0f, 0.0f, 0.7f }; 
camera.up = (Vector3){ 0.0f, 1.0f, 0.0f }; 
camera.fovy = 65.0f; 

SetCameraMode(camera, CAMERA_FREE); 



Shader shader = LoadShader(0, FormatText("resources/shaders/glsl%i/raymarching.fs", GLSL_VERSION));


int viewEyeLoc = GetShaderLocation(shader, "viewEye");
int viewCenterLoc = GetShaderLocation(shader, "viewCenter");
int runTimeLoc = GetShaderLocation(shader, "runTime");
int resolutionLoc = GetShaderLocation(shader, "resolution");

float resolution[2] = { (float)screenWidth, (float)screenHeight };
SetShaderValue(shader, resolutionLoc, resolution, UNIFORM_VEC2);

float runTime = 0.0f;

SetTargetFPS(60); 



while (!WindowShouldClose()) 
{


if(IsWindowResized())
{
screenWidth = GetScreenWidth();
screenHeight = GetScreenHeight();
float resolution[2] = { (float)screenWidth, (float)screenHeight };
SetShaderValue(shader, resolutionLoc, resolution, UNIFORM_VEC2);
}



UpdateCamera(&camera); 

float cameraPos[3] = { camera.position.x, camera.position.y, camera.position.z };
float cameraTarget[3] = { camera.target.x, camera.target.y, camera.target.z };

float deltaTime = GetFrameTime();
runTime += deltaTime;


SetShaderValue(shader, viewEyeLoc, cameraPos, UNIFORM_VEC3);
SetShaderValue(shader, viewCenterLoc, cameraTarget, UNIFORM_VEC3);
SetShaderValue(shader, runTimeLoc, &runTime, UNIFORM_FLOAT);




BeginDrawing();

ClearBackground(RAYWHITE);



BeginShaderMode(shader);
DrawRectangle(0, 0, screenWidth, screenHeight, WHITE);
EndShaderMode();

DrawText("(c) Raymarching shader by Iñigo Quilez. MIT License.", screenWidth - 280, screenHeight - 20, 10, BLACK);

EndDrawing();

}



UnloadShader(shader); 

CloseWindow(); 


return 0;
}
