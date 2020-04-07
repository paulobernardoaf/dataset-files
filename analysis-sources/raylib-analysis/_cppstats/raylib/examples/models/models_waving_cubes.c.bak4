












#include "raylib.h"

#include <math.h>

int main()
{


const int screenWidth = 800;
const int screenHeight = 450;

InitWindow(screenWidth, screenHeight, "raylib [models] example - waving cubes");


Camera3D camera = { 0 };
camera.position = (Vector3){ 30.0f, 20.0f, 30.0f };
camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
camera.fovy = 70.0f;
camera.type = CAMERA_PERSPECTIVE;


const int numBlocks = 15;

SetTargetFPS(60);



while (!WindowShouldClose()) 
{


double time = GetTime();


float scale = (2.0f + (float)sin(time))*0.7f;


double cameraTime = time*0.3;
camera.position.x = (float)cos(cameraTime)*40.0f;
camera.position.z = (float)sin(cameraTime)*40.0f;




BeginDrawing();

ClearBackground(RAYWHITE);

BeginMode3D(camera);

DrawGrid(10, 5.0f);

for (int x = 0; x < numBlocks; x++) 
{
for (int y = 0; y < numBlocks; y++) 
{
for (int z = 0; z < numBlocks; z++) 
{

float blockScale = (x + y + z)/30.0f;


float scatter = sinf(blockScale*20.0f + (float)(time*4.0f));


Vector3 cubePos = {
(float)(x - numBlocks/2)*(scale*3.0f) + scatter,
(float)(y - numBlocks/2)*(scale*2.0f) + scatter,
(float)(z - numBlocks/2)*(scale*3.0f) + scatter
};


Color cubeColor = ColorFromHSV((Vector3){ (float)(((x + y + z)*18)%360), 0.75f, 0.9f });


float cubeSize = (2.4f - scale)*blockScale;


DrawCube(cubePos, cubeSize, cubeSize, cubeSize, cubeColor);
}
}
}

EndMode3D();

DrawFPS(10, 10);

EndDrawing();

}



CloseWindow(); 


return 0;
}
