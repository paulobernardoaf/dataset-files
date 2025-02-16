










#include "raylib.h"

int main(void)
{


const int screenWidth = 800;
const int screenHeight = 450;

InitWindow(screenWidth, screenHeight, "raylib [models] example - cubesmap loading and drawing");


Camera camera = { { 16.0f, 14.0f, 16.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 45.0f, 0 };

Image image = LoadImage("resources/cubicmap.png"); 
Texture2D cubicmap = LoadTextureFromImage(image); 

Mesh mesh = GenMeshCubicmap(image, (Vector3){ 1.0f, 1.0f, 1.0f });
Model model = LoadModelFromMesh(mesh);


Texture2D texture = LoadTexture("resources/cubicmap_atlas.png"); 
model.materials[0].maps[MAP_DIFFUSE].texture = texture; 

Vector3 mapPosition = { -16.0f, 0.0f, -8.0f }; 

UnloadImage(image); 

SetCameraMode(camera, CAMERA_ORBITAL); 

SetTargetFPS(60); 



while (!WindowShouldClose()) 
{


UpdateCamera(&camera); 




BeginDrawing();

ClearBackground(RAYWHITE);

BeginMode3D(camera);

DrawModel(model, mapPosition, 1.0f, WHITE);

EndMode3D();

DrawTextureEx(cubicmap, (Vector2){ screenWidth - cubicmap.width*4 - 20, 20 }, 0.0f, 4.0f, WHITE);
DrawRectangleLines(screenWidth - cubicmap.width*4 - 20, 20, cubicmap.width*4, cubicmap.height*4, GREEN);

DrawText("cubicmap image used to", 658, 90, 10, GRAY);
DrawText("generate map 3d model", 658, 104, 10, GRAY);

DrawFPS(10, 10);

EndDrawing();

}



UnloadTexture(cubicmap); 
UnloadTexture(texture); 
UnloadModel(model); 

CloseWindow(); 


return 0;
}
