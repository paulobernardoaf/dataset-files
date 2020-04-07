#include "raylib.h"
#include <stdlib.h> 
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
InitWindow(screenWidth, screenHeight, "raylib [models] example - first person maze");
Camera camera = { { 0.2f, 0.4f, 0.2f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 45.0f, 0 };
Image imMap = LoadImage("resources/cubicmap.png"); 
Texture2D cubicmap = LoadTextureFromImage(imMap); 
Mesh mesh = GenMeshCubicmap(imMap, (Vector3){ 1.0f, 1.0f, 1.0f });
Model model = LoadModelFromMesh(mesh);
Texture2D texture = LoadTexture("resources/cubicmap_atlas.png"); 
model.materials[0].maps[MAP_DIFFUSE].texture = texture; 
Color *mapPixels = GetImageData(imMap);
UnloadImage(imMap); 
Vector3 mapPosition = { -16.0f, 0.0f, -8.0f }; 
Vector3 playerPosition = camera.position; 
SetCameraMode(camera, CAMERA_FIRST_PERSON); 
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
Vector3 oldCamPos = camera.position; 
UpdateCamera(&camera); 
Vector2 playerPos = { camera.position.x, camera.position.z };
float playerRadius = 0.1f; 
int playerCellX = (int)(playerPos.x - mapPosition.x + 0.5f);
int playerCellY = (int)(playerPos.y - mapPosition.z + 0.5f);
if (playerCellX < 0) playerCellX = 0;
else if (playerCellX >= cubicmap.width) playerCellX = cubicmap.width - 1;
if (playerCellY < 0) playerCellY = 0;
else if (playerCellY >= cubicmap.height) playerCellY = cubicmap.height - 1;
for (int y = 0; y < cubicmap.height; y++)
{
for (int x = 0; x < cubicmap.width; x++)
{
if ((mapPixels[y*cubicmap.width + x].r == 255) && 
(CheckCollisionCircleRec(playerPos, playerRadius,
(Rectangle){ mapPosition.x - 0.5f + x*1.0f, mapPosition.z - 0.5f + y*1.0f, 1.0f, 1.0f })))
{
camera.position = oldCamPos;
}
}
}
BeginDrawing();
ClearBackground(RAYWHITE);
BeginMode3D(camera);
DrawModel(model, mapPosition, 1.0f, WHITE); 
EndMode3D();
DrawTextureEx(cubicmap, (Vector2){ GetScreenWidth() - cubicmap.width*4 - 20, 20 }, 0.0f, 4.0f, WHITE);
DrawRectangleLines(GetScreenWidth() - cubicmap.width*4 - 20, 20, cubicmap.width*4, cubicmap.height*4, GREEN);
DrawRectangle(GetScreenWidth() - cubicmap.width*4 - 20 + playerCellX*4, 20 + playerCellY*4, 4, 4, RED);
DrawFPS(10, 10);
EndDrawing();
}
free(mapPixels); 
UnloadTexture(cubicmap); 
UnloadTexture(texture); 
UnloadModel(model); 
CloseWindow(); 
return 0;
}
