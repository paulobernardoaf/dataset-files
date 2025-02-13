










#include "raylib.h"

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION 330
#else 
#define GLSL_VERSION 100
#endif

#include <stdlib.h>

int main(void)
{


const int screenWidth = 800;
const int screenHeight = 450;

InitWindow(screenWidth, screenHeight, "raylib [text] example - SDF fonts");



const char msg[50] = "Signed Distance Fields";


Font fontDefault = { 0 };
fontDefault.baseSize = 16;
fontDefault.charsCount = 95;

fontDefault.chars = LoadFontData("resources/AnonymousPro-Bold.ttf", 16, 0, 95, FONT_DEFAULT);

Image atlas = GenImageFontAtlas(fontDefault.chars, &fontDefault.recs, 95, 16, 4, 0);
fontDefault.texture = LoadTextureFromImage(atlas);
UnloadImage(atlas);


Font fontSDF = { 0 };
fontSDF.baseSize = 16;
fontSDF.charsCount = 95;

fontSDF.chars = LoadFontData("resources/AnonymousPro-Bold.ttf", 16, 0, 0, FONT_SDF);

atlas = GenImageFontAtlas(fontSDF.chars, &fontSDF.recs, 95, 16, 0, 1);
fontSDF.texture = LoadTextureFromImage(atlas);
UnloadImage(atlas);


Shader shader = LoadShader(0, FormatText("resources/shaders/glsl%i/sdf.fs", GLSL_VERSION));
SetTextureFilter(fontSDF.texture, FILTER_BILINEAR); 

Vector2 fontPosition = { 40, screenHeight/2 - 50 };
Vector2 textSize = { 0.0f, 0.0f };
float fontSize = 16.0f;
int currentFont = 0; 

SetTargetFPS(60); 



while (!WindowShouldClose()) 
{


fontSize += GetMouseWheelMove()*8.0f;

if (fontSize < 6) fontSize = 6;

if (IsKeyDown(KEY_SPACE)) currentFont = 1;
else currentFont = 0;

if (currentFont == 0) textSize = MeasureTextEx(fontDefault, msg, fontSize, 0);
else textSize = MeasureTextEx(fontSDF, msg, fontSize, 0);

fontPosition.x = GetScreenWidth()/2 - textSize.x/2;
fontPosition.y = GetScreenHeight()/2 - textSize.y/2 + 80;




BeginDrawing();

ClearBackground(RAYWHITE);

if (currentFont == 1)
{

BeginShaderMode(shader); 
DrawTextEx(fontSDF, msg, fontPosition, fontSize, 0, BLACK);
EndShaderMode(); 

DrawTexture(fontSDF.texture, 10, 10, BLACK);
}
else
{
DrawTextEx(fontDefault, msg, fontPosition, fontSize, 0, BLACK);
DrawTexture(fontDefault.texture, 10, 10, BLACK);
}

if (currentFont == 1) DrawText("SDF!", 320, 20, 80, RED);
else DrawText("default font", 315, 40, 30, GRAY);

DrawText("FONT SIZE: 16.0", GetScreenWidth() - 240, 20, 20, DARKGRAY);
DrawText(FormatText("RENDER SIZE: %02.02f", fontSize), GetScreenWidth() - 240, 50, 20, DARKGRAY);
DrawText("Use MOUSE WHEEL to SCALE TEXT!", GetScreenWidth() - 240, 90, 10, DARKGRAY);

DrawText("HOLD SPACE to USE SDF FONT VERSION!", 340, GetScreenHeight() - 30, 20, MAROON);

EndDrawing();

}



UnloadFont(fontDefault); 
UnloadFont(fontSDF); 

UnloadShader(shader); 

CloseWindow(); 


return 0;
}