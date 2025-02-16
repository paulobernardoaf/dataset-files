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
InitWindow(screenWidth, screenHeight, "raylib [shaders] example - texture waves");
Texture2D texture = LoadTexture("resources/space.png");
Shader shader = LoadShader(0, FormatText("resources/shaders/glsl%i/wave.fs", GLSL_VERSION));
int secondsLoc = GetShaderLocation(shader, "secondes");
int freqXLoc = GetShaderLocation(shader, "freqX");
int freqYLoc = GetShaderLocation(shader, "freqY");
int ampXLoc = GetShaderLocation(shader, "ampX");
int ampYLoc = GetShaderLocation(shader, "ampY");
int speedXLoc = GetShaderLocation(shader, "speedX");
int speedYLoc = GetShaderLocation(shader, "speedY");
float freqX = 25.0f;
float freqY = 25.0f;
float ampX = 5.0f;
float ampY = 5.0f;
float speedX = 8.0f;
float speedY = 8.0f;
float screenSize[2] = { (float)GetScreenWidth(), (float)GetScreenHeight() };
SetShaderValue(shader, GetShaderLocation(shader, "size"), &screenSize, UNIFORM_VEC2);
SetShaderValue(shader, freqXLoc, &freqX, UNIFORM_FLOAT);
SetShaderValue(shader, freqYLoc, &freqY, UNIFORM_FLOAT);
SetShaderValue(shader, ampXLoc, &ampX, UNIFORM_FLOAT);
SetShaderValue(shader, ampYLoc, &ampY, UNIFORM_FLOAT);
SetShaderValue(shader, speedXLoc, &speedX, UNIFORM_FLOAT);
SetShaderValue(shader, speedYLoc, &speedY, UNIFORM_FLOAT);
float seconds = 0.0f;
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
seconds += GetFrameTime();
SetShaderValue(shader, secondsLoc, &seconds, UNIFORM_FLOAT);
BeginDrawing();
ClearBackground(RAYWHITE);
BeginShaderMode(shader);
DrawTexture(texture, 0, 0, WHITE);
DrawTexture(texture, texture.width, 0, WHITE);
EndShaderMode();
EndDrawing();
}
UnloadShader(shader); 
UnloadTexture(texture); 
CloseWindow(); 
return 0;
}
