




















#include "raylib.h"

int main() 
{


const int screenWidth = 800;
const int screenHeight = 450;

InitWindow(screenWidth, screenHeight, "raylib");

Camera camera = { 0 };
camera.position = (Vector3){ 10.0f, 10.0f, 8.0f };
camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
camera.fovy = 60.0f;
camera.type = CAMERA_PERSPECTIVE;

SetCameraMode(camera, CAMERA_ORBITAL);

Vector3 cubePosition = { 0.0f };

SetTargetFPS(60); 



while (!WindowShouldClose()) 
{


UpdateCamera(&camera);




BeginDrawing();

ClearBackground(RAYWHITE);

BeginMode3D(camera);

DrawCube(cubePosition, 2.0f, 2.0f, 2.0f, RED);
DrawCubeWires(cubePosition, 2.0f, 2.0f, 2.0f, MAROON);
DrawGrid(10, 1.0f);

EndMode3D();

DrawText("This is a raylib example", 10, 40, 20, DARKGRAY);

DrawFPS(10, 10);

EndDrawing();

}



CloseWindow(); 


return 0;
}