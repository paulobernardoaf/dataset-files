#include "raylib.h"
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
InitWindow(screenWidth, screenHeight, "raylib [core] example - 3d picking");
Camera camera = { 0 };
camera.position = (Vector3){ 10.0f, 10.0f, 10.0f }; 
camera.target = (Vector3){ 0.0f, 0.0f, 0.0f }; 
camera.up = (Vector3){ 0.0f, 1.0f, 0.0f }; 
camera.fovy = 45.0f; 
camera.type = CAMERA_PERSPECTIVE; 
Vector3 cubePosition = { 0.0f, 1.0f, 0.0f };
Vector3 cubeSize = { 2.0f, 2.0f, 2.0f };
Ray ray = { 0 }; 
bool collision = false;
SetCameraMode(camera, CAMERA_FREE); 
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
UpdateCamera(&camera); 
if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
{
if (!collision)
{
ray = GetMouseRay(GetMousePosition(), camera);
collision = CheckCollisionRayBox(ray,
(BoundingBox){(Vector3){ cubePosition.x - cubeSize.x/2, cubePosition.y - cubeSize.y/2, cubePosition.z - cubeSize.z/2 },
(Vector3){ cubePosition.x + cubeSize.x/2, cubePosition.y + cubeSize.y/2, cubePosition.z + cubeSize.z/2 }});
}
else collision = false;
}
BeginDrawing();
ClearBackground(RAYWHITE);
BeginMode3D(camera);
if (collision)
{
DrawCube(cubePosition, cubeSize.x, cubeSize.y, cubeSize.z, RED);
DrawCubeWires(cubePosition, cubeSize.x, cubeSize.y, cubeSize.z, MAROON);
DrawCubeWires(cubePosition, cubeSize.x + 0.2f, cubeSize.y + 0.2f, cubeSize.z + 0.2f, GREEN);
}
else
{
DrawCube(cubePosition, cubeSize.x, cubeSize.y, cubeSize.z, GRAY);
DrawCubeWires(cubePosition, cubeSize.x, cubeSize.y, cubeSize.z, DARKGRAY);
}
DrawRay(ray, MAROON);
DrawGrid(10, 1.0f);
EndMode3D();
DrawText("Try selecting the box with mouse!", 240, 10, 20, DARKGRAY);
if(collision) DrawText("BOX SELECTED", (screenWidth - MeasureText("BOX SELECTED", 30)) / 2, screenHeight * 0.1f, 30, GREEN);
DrawFPS(10, 10);
EndDrawing();
}
CloseWindow(); 
return 0;
}
