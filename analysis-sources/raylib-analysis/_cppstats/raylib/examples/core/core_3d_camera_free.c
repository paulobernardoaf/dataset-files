#include "raylib.h"
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
InitWindow(screenWidth, screenHeight, "raylib [core] example - 3d camera free");
Camera3D camera = { 0 };
camera.position = (Vector3){ 10.0f, 10.0f, 10.0f }; 
camera.target = (Vector3){ 0.0f, 0.0f, 0.0f }; 
camera.up = (Vector3){ 0.0f, 1.0f, 0.0f }; 
camera.fovy = 45.0f; 
camera.type = CAMERA_PERSPECTIVE; 
Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };
SetCameraMode(camera, CAMERA_FREE); 
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
UpdateCamera(&camera); 
if (IsKeyDown('Z')) camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
BeginDrawing();
ClearBackground(RAYWHITE);
BeginMode3D(camera);
DrawCube(cubePosition, 2.0f, 2.0f, 2.0f, RED);
DrawCubeWires(cubePosition, 2.0f, 2.0f, 2.0f, MAROON);
DrawGrid(10, 1.0f);
EndMode3D();
DrawRectangle( 10, 10, 320, 133, Fade(SKYBLUE, 0.5f));
DrawRectangleLines( 10, 10, 320, 133, BLUE);
DrawText("Free camera default controls:", 20, 20, 10, BLACK);
DrawText("- Mouse Wheel to Zoom in-out", 40, 40, 10, DARKGRAY);
DrawText("- Mouse Wheel Pressed to Pan", 40, 60, 10, DARKGRAY);
DrawText("- Alt + Mouse Wheel Pressed to Rotate", 40, 80, 10, DARKGRAY);
DrawText("- Alt + Ctrl + Mouse Wheel Pressed for Smooth Zoom", 40, 100, 10, DARKGRAY);
DrawText("- Z to zoom to (0, 0, 0)", 40, 120, 10, DARKGRAY);
EndDrawing();
}
CloseWindow(); 
return 0;
}