














#include "raylib.h"

#define FOVY_PERSPECTIVE 45.0f
#define WIDTH_ORTHOGRAPHIC 10.0f

int main(void)
{


const int screenWidth = 800;
const int screenHeight = 450;

InitWindow(screenWidth, screenHeight, "raylib [models] example - geometric shapes");


Camera camera = { { 0.0f, 10.0f, 10.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, FOVY_PERSPECTIVE, CAMERA_PERSPECTIVE };

SetTargetFPS(60); 



while (!WindowShouldClose()) 
{


if (IsKeyPressed(KEY_SPACE))
{
if (camera.type == CAMERA_PERSPECTIVE)
{
camera.fovy = WIDTH_ORTHOGRAPHIC;
camera.type = CAMERA_ORTHOGRAPHIC;
}
else
{
camera.fovy = FOVY_PERSPECTIVE;
camera.type = CAMERA_PERSPECTIVE;
}
}




BeginDrawing();

ClearBackground(RAYWHITE);

BeginMode3D(camera);

DrawCube((Vector3){-4.0f, 0.0f, 2.0f}, 2.0f, 5.0f, 2.0f, RED);
DrawCubeWires((Vector3){-4.0f, 0.0f, 2.0f}, 2.0f, 5.0f, 2.0f, GOLD);
DrawCubeWires((Vector3){-4.0f, 0.0f, -2.0f}, 3.0f, 6.0f, 2.0f, MAROON);

DrawSphere((Vector3){-1.0f, 0.0f, -2.0f}, 1.0f, GREEN);
DrawSphereWires((Vector3){1.0f, 0.0f, 2.0f}, 2.0f, 16, 16, LIME);

DrawCylinder((Vector3){4.0f, 0.0f, -2.0f}, 1.0f, 2.0f, 3.0f, 4, SKYBLUE);
DrawCylinderWires((Vector3){4.0f, 0.0f, -2.0f}, 1.0f, 2.0f, 3.0f, 4, DARKBLUE);
DrawCylinderWires((Vector3){4.5f, -1.0f, 2.0f}, 1.0f, 1.0f, 2.0f, 6, BROWN);

DrawCylinder((Vector3){1.0f, 0.0f, -4.0f}, 0.0f, 1.5f, 3.0f, 8, GOLD);
DrawCylinderWires((Vector3){1.0f, 0.0f, -4.0f}, 0.0f, 1.5f, 3.0f, 8, PINK);

DrawGrid(10, 1.0f); 

EndMode3D();

DrawText("Press Spacebar to switch camera type", 10, GetScreenHeight() - 30, 20, DARKGRAY);

if (camera.type == CAMERA_ORTHOGRAPHIC) DrawText("ORTHOGRAPHIC", 10, 40, 20, BLACK);
else if (camera.type == CAMERA_PERSPECTIVE) DrawText("PERSPECTIVE", 10, 40, 20, BLACK);

DrawFPS(10, 10);

EndDrawing();

}



CloseWindow(); 


return 0;
}
