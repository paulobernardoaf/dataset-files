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
InitWindow(screenWidth, screenHeight, "raylib [shaders] example - texture drawing");
Image imBlank = GenImageColor(1024, 1024, BLANK);
Texture2D texture = LoadTextureFromImage(imBlank); 
UnloadImage(imBlank);
Shader shader = LoadShader(0, FormatText("resources/shaders/glsl%i/cubes_panning.fs", GLSL_VERSION));
float time = 0.0f;
int timeLoc = GetShaderLocation(shader, "uTime");
SetShaderValue(shader, timeLoc, &time, UNIFORM_FLOAT);
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
time = GetTime();
SetShaderValue(shader, timeLoc, &time, UNIFORM_FLOAT);
BeginDrawing();
ClearBackground(RAYWHITE);
BeginShaderMode(shader); 
DrawTexture(texture, 0, 0, WHITE); 
EndShaderMode(); 
DrawText("BACKGROUND is PAINTED and ANIMATED on SHADER!", 10, 10, 20, MAROON);
EndDrawing();
}
UnloadShader(shader);
CloseWindow(); 
return 0;
}
