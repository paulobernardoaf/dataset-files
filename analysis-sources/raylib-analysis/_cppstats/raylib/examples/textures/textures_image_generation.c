#include "raylib.h"
#define NUM_TEXTURES 7 
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
InitWindow(screenWidth, screenHeight, "raylib [textures] example - procedural images generation");
Image verticalGradient = GenImageGradientV(screenWidth, screenHeight, RED, BLUE);
Image horizontalGradient = GenImageGradientH(screenWidth, screenHeight, RED, BLUE);
Image radialGradient = GenImageGradientRadial(screenWidth, screenHeight, 0.0f, WHITE, BLACK);
Image checked = GenImageChecked(screenWidth, screenHeight, 32, 32, RED, BLUE);
Image whiteNoise = GenImageWhiteNoise(screenWidth, screenHeight, 0.5f);
Image perlinNoise = GenImagePerlinNoise(screenWidth, screenHeight, 50, 50, 4.0f);
Image cellular = GenImageCellular(screenWidth, screenHeight, 32);
Texture2D textures[NUM_TEXTURES] = { 0 };
textures[0] = LoadTextureFromImage(verticalGradient);
textures[1] = LoadTextureFromImage(horizontalGradient);
textures[2] = LoadTextureFromImage(radialGradient);
textures[3] = LoadTextureFromImage(checked);
textures[4] = LoadTextureFromImage(whiteNoise);
textures[5] = LoadTextureFromImage(perlinNoise);
textures[6] = LoadTextureFromImage(cellular);
UnloadImage(verticalGradient);
UnloadImage(horizontalGradient);
UnloadImage(radialGradient);
UnloadImage(checked);
UnloadImage(whiteNoise);
UnloadImage(perlinNoise);
UnloadImage(cellular);
int currentTexture = 0;
SetTargetFPS(60);
while (!WindowShouldClose())
{
if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsKeyPressed(KEY_RIGHT))
{
currentTexture = (currentTexture + 1)%NUM_TEXTURES; 
}
BeginDrawing();
ClearBackground(RAYWHITE);
DrawTexture(textures[currentTexture], 0, 0, WHITE);
DrawRectangle(30, 400, 325, 30, Fade(SKYBLUE, 0.5f));
DrawRectangleLines(30, 400, 325, 30, Fade(WHITE, 0.5f));
DrawText("MOUSE LEFT BUTTON to CYCLE PROCEDURAL TEXTURES", 40, 410, 10, WHITE);
switch(currentTexture)
{
case 0: DrawText("VERTICAL GRADIENT", 560, 10, 20, RAYWHITE); break;
case 1: DrawText("HORIZONTAL GRADIENT", 540, 10, 20, RAYWHITE); break;
case 2: DrawText("RADIAL GRADIENT", 580, 10, 20, LIGHTGRAY); break;
case 3: DrawText("CHECKED", 680, 10, 20, RAYWHITE); break;
case 4: DrawText("WHITE NOISE", 640, 10, 20, RED); break;
case 5: DrawText("PERLIN NOISE", 630, 10, 20, RAYWHITE); break;
case 6: DrawText("CELLULAR", 670, 10, 20, RAYWHITE); break;
default: break;
}
EndDrawing();
}
for (int i = 0; i < NUM_TEXTURES; i++) UnloadTexture(textures[i]);
CloseWindow(); 
return 0;
}
