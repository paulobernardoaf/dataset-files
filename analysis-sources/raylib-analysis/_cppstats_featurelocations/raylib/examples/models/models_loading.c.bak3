



















#include "raylib.h"

int main(void)
{


const int screenWidth = 800;
const int screenHeight = 450;

InitWindow(screenWidth, screenHeight, "raylib [models] example - models loading");


Camera camera = { 0 };
camera.position = (Vector3){ 50.0f, 50.0f, 50.0f }; 
camera.target = (Vector3){ 0.0f, 10.0f, 0.0f }; 
camera.up = (Vector3){ 0.0f, 1.0f, 0.0f }; 
camera.fovy = 45.0f; 
camera.type = CAMERA_PERSPECTIVE; 

Model model = LoadModel("resources/models/castle.obj"); 
Texture2D texture = LoadTexture("resources/models/castle_diffuse.png"); 
model.materials[0].maps[MAP_DIFFUSE].texture = texture; 

Vector3 position = { 0.0f, 0.0f, 0.0f }; 

BoundingBox bounds = MeshBoundingBox(model.meshes[0]); 




SetCameraMode(camera, CAMERA_FREE); 

bool selected = false; 

SetTargetFPS(60); 



while (!WindowShouldClose()) 
{


UpdateCamera(&camera);


if (IsFileDropped())
{
int count = 0;
char **droppedFiles = GetDroppedFiles(&count);

if (count == 1) 
{
if (IsFileExtension(droppedFiles[0], ".obj") ||
IsFileExtension(droppedFiles[0], ".gltf") ||
IsFileExtension(droppedFiles[0], ".iqm")) 
{
UnloadModel(model); 
model = LoadModel(droppedFiles[0]); 
model.materials[0].maps[MAP_DIFFUSE].texture = texture; 

bounds = MeshBoundingBox(model.meshes[0]);


}
else if (IsFileExtension(droppedFiles[0], ".png")) 
{

UnloadTexture(texture);
texture = LoadTexture(droppedFiles[0]);
model.materials[0].maps[MAP_DIFFUSE].texture = texture;
}
}

ClearDroppedFiles(); 
}


if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
{

if (CheckCollisionRayBox(GetMouseRay(GetMousePosition(), camera), bounds)) selected = !selected;
else selected = false;
}




BeginDrawing();

ClearBackground(RAYWHITE);

BeginMode3D(camera);

DrawModel(model, position, 1.0f, WHITE); 

DrawGrid(20, 10.0f); 

if (selected) DrawBoundingBox(bounds, GREEN); 

EndMode3D();

DrawText("Drag & drop model to load mesh/texture.", 10, GetScreenHeight() - 20, 10, DARKGRAY);
if (selected) DrawText("MODEL SELECTED", GetScreenWidth() - 110, 10, 10, GREEN);

DrawText("(c) Castle 3D model by Alberto Cano", screenWidth - 200, screenHeight - 20, 10, GRAY);

DrawFPS(10, 10);

EndDrawing();

}



UnloadTexture(texture); 
UnloadModel(model); 

CloseWindow(); 


return 0;
}