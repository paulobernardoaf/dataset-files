#include "raylib.h"
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
InitWindow(screenWidth, screenHeight, "raylib [texture] example - image text drawing");
Image parrots = LoadImage("resources/parrots.png"); 
Font font = LoadFontEx("resources/KAISG.ttf", 64, 0, 0);
ImageDrawTextEx(&parrots, (Vector2){ 20.0f, 20.0f }, font, "[Parrots font drawing]", (float)font.baseSize, 0.0f, RED);
Texture2D texture = LoadTextureFromImage(parrots); 
UnloadImage(parrots); 
Vector2 position = { (float)(screenWidth/2 - texture.width/2), (float)(screenHeight/2 - texture.height/2 - 20) };
bool showFont = false;
SetTargetFPS(60);
while (!WindowShouldClose()) 
{
if (IsKeyDown(KEY_SPACE)) showFont = true;
else showFont = false;
BeginDrawing();
ClearBackground(RAYWHITE);
if (!showFont)
{
DrawTextureV(texture, position, WHITE);
DrawTextEx(font, "[Parrots font drawing]", (Vector2){ position.x + 20,
position.y + 20 + 280 }, (float)font.baseSize, 0.0f, WHITE);
}
else DrawTexture(font.texture, screenWidth/2 - font.texture.width/2, 50, BLACK);
DrawText("PRESS SPACE to SEE USED SPRITEFONT ", 290, 420, 10, DARKGRAY);
EndDrawing();
}
UnloadTexture(texture); 
UnloadFont(font); 
CloseWindow(); 
return 0;
}