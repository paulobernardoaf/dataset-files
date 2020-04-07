#include "raylib.h"
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
InitWindow(screenWidth, screenHeight, "raylib [core] example - 3d camera free");
Camera camera = { 0 };
camera.position = (Vector3){ 10.0f, 10.0f, 10.0f };
camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
camera.fovy = 45.0f;
camera.type = CAMERA_PERSPECTIVE;
Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };
Vector2 cubeScreenPosition = { 0.0f, 0.0f };
SetCameraMode(camera, CAMERA_FREE); 
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
UpdateCamera(&camera); 
cubeScreenPosition = GetWorldToScreen((Vector3){cubePosition.x, cubePosition.y + 2.5f, cubePosition.z}, camera);
BeginDrawing();
ClearBackground(RAYWHITE);
BeginMode3D(camera);
DrawCube(cubePosition, 2.0f, 2.0f, 2.0f, RED);
DrawCubeWires(cubePosition, 2.0f, 2.0f, 2.0f, MAROON);
DrawGrid(10, 1.0f);
EndMode3D();
DrawText("Enemy: 100 / 100", cubeScreenPosition.x - MeasureText("Enemy: 100/100", 20)/2, cubeScreenPosition.y, 20, BLACK);
DrawText("Text is always on top of the cube", (screenWidth - MeasureText("Text is always on top of the cube", 20))/2, 25, 20, GRAY);
EndDrawing();
}
CloseWindow(); 
return 0;
}