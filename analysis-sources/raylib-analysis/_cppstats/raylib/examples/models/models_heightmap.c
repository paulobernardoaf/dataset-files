#include "raylib.h"
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
InitWindow(screenWidth, screenHeight, "raylib [models] example - heightmap loading and drawing");
Camera camera = { { 18.0f, 18.0f, 18.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 45.0f, 0 };
Image image = LoadImage("resources/heightmap.png"); 
Texture2D texture = LoadTextureFromImage(image); 
Mesh mesh = GenMeshHeightmap(image, (Vector3){ 16, 8, 16 }); 
Model model = LoadModelFromMesh(mesh); 
model.materials[0].maps[MAP_DIFFUSE].texture = texture; 
Vector3 mapPosition = { -8.0f, 0.0f, -8.0f }; 
UnloadImage(image); 
SetCameraMode(camera, CAMERA_ORBITAL); 
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
UpdateCamera(&camera); 
BeginDrawing();
ClearBackground(RAYWHITE);
BeginMode3D(camera);
DrawModel(model, mapPosition, 1.0f, RED);
DrawGrid(20, 1.0f);
EndMode3D();
DrawTexture(texture, screenWidth - texture.width - 20, 20, WHITE);
DrawRectangleLines(screenWidth - texture.width - 20, 20, texture.width, texture.height, GREEN);
DrawFPS(10, 10);
EndDrawing();
}
UnloadTexture(texture); 
UnloadModel(model); 
CloseWindow(); 
return 0;
}