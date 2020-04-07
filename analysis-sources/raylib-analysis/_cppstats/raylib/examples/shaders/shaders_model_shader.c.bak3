

















#include "raylib.h"

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION 330
#else 
#define GLSL_VERSION 100
#endif

int main(void)
{


const int screenWidth = 800;
const int screenHeight = 450;

SetConfigFlags(FLAG_MSAA_4X_HINT); 

InitWindow(screenWidth, screenHeight, "raylib [shaders] example - model shader");


Camera camera = { 0 };
camera.position = (Vector3){ 4.0f, 4.0f, 4.0f };
camera.target = (Vector3){ 0.0f, 1.0f, -1.0f };
camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
camera.fovy = 45.0f;
camera.type = CAMERA_PERSPECTIVE;

Model model = LoadModel("resources/models/watermill.obj"); 
Texture2D texture = LoadTexture("resources/models/watermill_diffuse.png"); 



Shader shader = LoadShader(0, FormatText("resources/shaders/glsl%i/grayscale.fs", GLSL_VERSION));

model.materials[0].shader = shader; 
model.materials[0].maps[MAP_DIFFUSE].texture = texture; 

Vector3 position = { 0.0f, 0.0f, 0.0f }; 

SetCameraMode(camera, CAMERA_FREE); 

SetTargetFPS(60); 



while (!WindowShouldClose()) 
{


UpdateCamera(&camera); 




BeginDrawing();

ClearBackground(RAYWHITE);

BeginMode3D(camera);

DrawModel(model, position, 0.2f, WHITE); 

DrawGrid(10, 1.0f); 

EndMode3D();

DrawText("(c) Watermill 3D model by Alberto Cano", screenWidth - 210, screenHeight - 20, 10, GRAY);

DrawFPS(10, 10);

EndDrawing();

}



UnloadShader(shader); 
UnloadTexture(texture); 
UnloadModel(model); 

CloseWindow(); 


return 0;
}