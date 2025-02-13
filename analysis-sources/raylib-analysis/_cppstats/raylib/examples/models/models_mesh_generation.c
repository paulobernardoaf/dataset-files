#include "raylib.h"
#define NUM_MODELS 8 
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
InitWindow(screenWidth, screenHeight, "raylib [models] example - mesh generation");
Image checked = GenImageChecked(2, 2, 1, 1, RED, GREEN);
Texture2D texture = LoadTextureFromImage(checked);
UnloadImage(checked);
Model models[NUM_MODELS] = { 0 };
models[0] = LoadModelFromMesh(GenMeshPlane(2, 2, 5, 5));
models[1] = LoadModelFromMesh(GenMeshCube(2.0f, 1.0f, 2.0f));
models[2] = LoadModelFromMesh(GenMeshSphere(2, 32, 32));
models[3] = LoadModelFromMesh(GenMeshHemiSphere(2, 16, 16));
models[4] = LoadModelFromMesh(GenMeshCylinder(1, 2, 16));
models[5] = LoadModelFromMesh(GenMeshTorus(0.25f, 4.0f, 16, 32));
models[6] = LoadModelFromMesh(GenMeshKnot(1.0f, 2.0f, 16, 128));
models[7] = LoadModelFromMesh(GenMeshPoly(5, 2.0f));
for (int i = 0; i < NUM_MODELS; i++) models[i].materials[0].maps[MAP_DIFFUSE].texture = texture;
Camera camera = { { 5.0f, 5.0f, 5.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 45.0f, 0 };
Vector3 position = { 0.0f, 0.0f, 0.0f };
int currentModel = 0;
SetCameraMode(camera, CAMERA_ORBITAL); 
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
UpdateCamera(&camera); 
if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
{
currentModel = (currentModel + 1)%NUM_MODELS; 
}
if (IsKeyPressed(KEY_RIGHT))
{
currentModel++;
if (currentModel >= NUM_MODELS) currentModel = 0;
}
else if (IsKeyPressed(KEY_LEFT))
{
currentModel--;
if (currentModel < 0) currentModel = NUM_MODELS - 1;
}
BeginDrawing();
ClearBackground(RAYWHITE);
BeginMode3D(camera);
DrawModel(models[currentModel], position, 1.0f, WHITE);
DrawGrid(10, 1.0);
EndMode3D();
DrawRectangle(30, 400, 310, 30, Fade(SKYBLUE, 0.5f));
DrawRectangleLines(30, 400, 310, 30, Fade(DARKBLUE, 0.5f));
DrawText("MOUSE LEFT BUTTON to CYCLE PROCEDURAL MODELS", 40, 410, 10, BLUE);
switch(currentModel)
{
case 0: DrawText("PLANE", 680, 10, 20, DARKBLUE); break;
case 1: DrawText("CUBE", 680, 10, 20, DARKBLUE); break;
case 2: DrawText("SPHERE", 680, 10, 20, DARKBLUE); break;
case 3: DrawText("HEMISPHERE", 640, 10, 20, DARKBLUE); break;
case 4: DrawText("CYLINDER", 680, 10, 20, DARKBLUE); break;
case 5: DrawText("TORUS", 680, 10, 20, DARKBLUE); break;
case 6: DrawText("KNOT", 680, 10, 20, DARKBLUE); break;
case 7: DrawText("POLY", 680, 10, 20, DARKBLUE); break;
default: break;
}
EndDrawing();
}
UnloadTexture(texture); 
for (int i = 0; i < NUM_MODELS; i++) UnloadModel(models[i]);
CloseWindow(); 
return 0;
}