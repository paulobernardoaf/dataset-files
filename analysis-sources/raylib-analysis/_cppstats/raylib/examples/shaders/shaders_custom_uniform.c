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
InitWindow(screenWidth, screenHeight, "raylib [shaders] example - custom uniform variable");
Camera camera = { 0 };
camera.position = (Vector3){ 8.0f, 8.0f, 8.0f };
camera.target = (Vector3){ 0.0f, 1.5f, 0.0f };
camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
camera.fovy = 45.0f;
camera.type = CAMERA_PERSPECTIVE;
Model model = LoadModel("resources/models/barracks.obj"); 
Texture2D texture = LoadTexture("resources/models/barracks_diffuse.png"); 
model.materials[0].maps[MAP_DIFFUSE].texture = texture; 
Vector3 position = { 0.0f, 0.0f, 0.0f }; 
Shader shader = LoadShader(0, FormatText("resources/shaders/glsl%i/swirl.fs", GLSL_VERSION));
int swirlCenterLoc = GetShaderLocation(shader, "center");
float swirlCenter[2] = { (float)screenWidth/2, (float)screenHeight/2 };
RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);
SetCameraMode(camera, CAMERA_ORBITAL); 
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
Vector2 mousePosition = GetMousePosition();
swirlCenter[0] = mousePosition.x;
swirlCenter[1] = screenHeight - mousePosition.y;
SetShaderValue(shader, swirlCenterLoc, swirlCenter, UNIFORM_VEC2);
UpdateCamera(&camera); 
BeginDrawing();
ClearBackground(RAYWHITE);
BeginTextureMode(target); 
ClearBackground(RAYWHITE); 
BeginMode3D(camera); 
DrawModel(model, position, 0.5f, WHITE); 
DrawGrid(10, 1.0f); 
EndMode3D(); 
DrawText("TEXT DRAWN IN RENDER TEXTURE", 200, 10, 30, RED);
EndTextureMode(); 
BeginShaderMode(shader);
DrawTextureRec(target.texture, (Rectangle){ 0, 0, target.texture.width, -target.texture.height }, (Vector2){ 0, 0 }, WHITE);
EndShaderMode();
DrawText("(c) Barracks 3D model by Alberto Cano", screenWidth - 220, screenHeight - 20, 10, GRAY);
DrawFPS(10, 10);
EndDrawing();
}
UnloadShader(shader); 
UnloadTexture(texture); 
UnloadModel(model); 
UnloadRenderTexture(target); 
CloseWindow(); 
return 0;
}