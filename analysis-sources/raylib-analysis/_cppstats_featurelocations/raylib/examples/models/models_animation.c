


















#include <stdlib.h>
#include "raylib.h"

int main(void)
{


const int screenWidth = 800;
const int screenHeight = 450;

InitWindow(screenWidth, screenHeight, "raylib [models] example - model animation");


Camera camera = { 0 };
camera.position = (Vector3){ 10.0f, 10.0f, 10.0f }; 
camera.target = (Vector3){ 0.0f, 0.0f, 0.0f }; 
camera.up = (Vector3){ 0.0f, 1.0f, 0.0f }; 
camera.fovy = 45.0f; 
camera.type = CAMERA_PERSPECTIVE; 

Model model = LoadModel("resources/guy/guy.iqm"); 
Texture2D texture = LoadTexture("resources/guy/guytex.png"); 
SetMaterialTexture(&model.materials[0], MAP_DIFFUSE, texture); 

Vector3 position = { 0.0f, 0.0f, 0.0f }; 


int animsCount = 0;
ModelAnimation *anims = LoadModelAnimations("resources/guy/guyanim.iqm", &animsCount);
int animFrameCounter = 0;

SetCameraMode(camera, CAMERA_FREE); 

SetTargetFPS(60); 



while (!WindowShouldClose()) 
{


UpdateCamera(&camera);


if (IsKeyDown(KEY_SPACE))
{
animFrameCounter++;
UpdateModelAnimation(model, anims[0], animFrameCounter);
if (animFrameCounter >= anims[0].frameCount) animFrameCounter = 0;
}




BeginDrawing();

ClearBackground(RAYWHITE);

BeginMode3D(camera);

DrawModelEx(model, position, (Vector3){ 1.0f, 0.0f, 0.0f }, -90.0f, (Vector3){ 1.0f, 1.0f, 1.0f }, WHITE);

for (int i = 0; i < model.boneCount; i++)
{
DrawCube(anims[0].framePoses[animFrameCounter][i].translation, 0.2f, 0.2f, 0.2f, RED);
}

DrawGrid(10, 1.0f); 

EndMode3D();

DrawText("PRESS SPACE to PLAY MODEL ANIMATION", 10, 10, 20, MAROON);
DrawText("(c) Guy IQM 3D model by @culacant", screenWidth - 200, screenHeight - 20, 10, GRAY);

EndDrawing();

}



UnloadTexture(texture); 


for (int i = 0; i < animsCount; i++) UnloadModelAnimation(anims[i]);
RL_FREE(anims);

UnloadModel(model); 

CloseWindow(); 


return 0;
}
