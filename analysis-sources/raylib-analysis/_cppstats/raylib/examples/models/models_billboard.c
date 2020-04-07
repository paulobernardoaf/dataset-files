#include "raylib.h"
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
InitWindow(screenWidth, screenHeight, "raylib [models] example - drawing billboards");
Camera camera = { 0 };
camera.position = (Vector3){ 5.0f, 4.0f, 5.0f };
camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };
camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
camera.fovy = 45.0f;
camera.type = CAMERA_PERSPECTIVE;
Texture2D bill = LoadTexture("resources/billboard.png"); 
Vector3 billPosition = { 0.0f, 2.0f, 0.0f }; 
SetCameraMode(camera, CAMERA_ORBITAL); 
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
UpdateCamera(&camera); 
BeginDrawing();
ClearBackground(RAYWHITE);
BeginMode3D(camera);
DrawGrid(10, 1.0f); 
DrawBillboard(camera, bill, billPosition, 2.0f, WHITE);
EndMode3D();
DrawFPS(10, 10);
EndDrawing();
}
UnloadTexture(bill); 
CloseWindow(); 
return 0;
}