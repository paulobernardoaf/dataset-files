










#include "raylib.h"

#define MAX_COLUMNS 20

int main(void)
{


const int screenWidth = 800;
const int screenHeight = 450;

InitWindow(screenWidth, screenHeight, "raylib [core] example - 3d camera first person");


Camera camera = { 0 };
camera.position = (Vector3){ 4.0f, 2.0f, 4.0f };
camera.target = (Vector3){ 0.0f, 1.8f, 0.0f };
camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
camera.fovy = 60.0f;
camera.type = CAMERA_PERSPECTIVE;


float heights[MAX_COLUMNS] = { 0.0f };
Vector3 positions[MAX_COLUMNS] = { 0 };
Color colors[MAX_COLUMNS] = { 0 };

for (int i = 0; i < MAX_COLUMNS; i++)
{
heights[i] = (float)GetRandomValue(1, 12);
positions[i] = (Vector3){ GetRandomValue(-15, 15), heights[i]/2, GetRandomValue(-15, 15) };
colors[i] = (Color){ GetRandomValue(20, 255), GetRandomValue(10, 55), 30, 255 };
}

SetCameraMode(camera, CAMERA_FIRST_PERSON); 

SetTargetFPS(60); 



while (!WindowShouldClose()) 
{


UpdateCamera(&camera); 




BeginDrawing();

ClearBackground(RAYWHITE);

BeginMode3D(camera);

DrawPlane((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector2){ 32.0f, 32.0f }, LIGHTGRAY); 
DrawCube((Vector3){ -16.0f, 2.5f, 0.0f }, 1.0f, 5.0f, 32.0f, BLUE); 
DrawCube((Vector3){ 16.0f, 2.5f, 0.0f }, 1.0f, 5.0f, 32.0f, LIME); 
DrawCube((Vector3){ 0.0f, 2.5f, 16.0f }, 32.0f, 5.0f, 1.0f, GOLD); 


for (int i = 0; i < MAX_COLUMNS; i++)
{
DrawCube(positions[i], 2.0f, heights[i], 2.0f, colors[i]);
DrawCubeWires(positions[i], 2.0f, heights[i], 2.0f, MAROON);
}

EndMode3D();

DrawRectangle( 10, 10, 220, 70, Fade(SKYBLUE, 0.5f));
DrawRectangleLines( 10, 10, 220, 70, BLUE);

DrawText("First person camera default controls:", 20, 20, 10, BLACK);
DrawText("- Move with keys: W, A, S, D", 40, 40, 10, DARKGRAY);
DrawText("- Mouse move to look around", 40, 60, 10, DARKGRAY);

EndDrawing();

}



CloseWindow(); 


return 0;
}