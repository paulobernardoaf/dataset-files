#include "raylib.h"
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
InitWindow(screenWidth, screenHeight, "raylib [text] example - font filters");
const char msg[50] = "Loaded Font";
Font font = LoadFontEx("resources/KAISG.ttf", 96, 0, 0);
GenTextureMipmaps(&font.texture);
float fontSize = font.baseSize;
Vector2 fontPosition = { 40, screenHeight/2 - 80 };
Vector2 textSize = { 0.0f, 0.0f };
SetTextureFilter(font.texture, FILTER_POINT);
int currentFontFilter = 0; 
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
fontSize += GetMouseWheelMove()*4.0f;
if (IsKeyPressed(KEY_ONE))
{
SetTextureFilter(font.texture, FILTER_POINT);
currentFontFilter = 0;
}
else if (IsKeyPressed(KEY_TWO))
{
SetTextureFilter(font.texture, FILTER_BILINEAR);
currentFontFilter = 1;
}
else if (IsKeyPressed(KEY_THREE))
{
SetTextureFilter(font.texture, FILTER_TRILINEAR);
currentFontFilter = 2;
}
textSize = MeasureTextEx(font, msg, fontSize, 0);
if (IsKeyDown(KEY_LEFT)) fontPosition.x -= 10;
else if (IsKeyDown(KEY_RIGHT)) fontPosition.x += 10;
if (IsFileDropped())
{
int count = 0;
char **droppedFiles = GetDroppedFiles(&count);
if (IsFileExtension(droppedFiles[0], ".ttf"))
{
UnloadFont(font);
font = LoadFontEx(droppedFiles[0], fontSize, 0, 0);
ClearDroppedFiles();
}
}
BeginDrawing();
ClearBackground(RAYWHITE);
DrawText("Use mouse wheel to change font size", 20, 20, 10, GRAY);
DrawText("Use KEY_RIGHT and KEY_LEFT to move text", 20, 40, 10, GRAY);
DrawText("Use 1, 2, 3 to change texture filter", 20, 60, 10, GRAY);
DrawText("Drop a new TTF font for dynamic loading", 20, 80, 10, DARKGRAY);
DrawTextEx(font, msg, fontPosition, fontSize, 0, BLACK);
DrawRectangle(0, screenHeight - 80, screenWidth, 80, LIGHTGRAY);
DrawText(FormatText("Font size: %02.02f", fontSize), 20, screenHeight - 50, 10, DARKGRAY);
DrawText(FormatText("Text size: [%02.02f, %02.02f]", textSize.x, textSize.y), 20, screenHeight - 30, 10, DARKGRAY);
DrawText("CURRENT TEXTURE FILTER:", 250, 400, 20, GRAY);
if (currentFontFilter == 0) DrawText("POINT", 570, 400, 20, BLACK);
else if (currentFontFilter == 1) DrawText("BILINEAR", 570, 400, 20, BLACK);
else if (currentFontFilter == 2) DrawText("TRILINEAR", 570, 400, 20, BLACK);
EndDrawing();
}
ClearDroppedFiles(); 
UnloadFont(font); 
CloseWindow(); 
return 0;
}