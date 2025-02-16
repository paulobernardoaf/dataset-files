#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include "raygui.h" 
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
InitWindow(screenWidth, screenHeight, "raylib [shapes] example - draw rectangle rounded");
float roundness = 0.2f;
int width = 200;
int height = 100;
int segments = 0;
int lineThick = 1;
bool drawRect = false;
bool drawRoundedRect = true;
bool drawRoundedLines = false;
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
Rectangle rec = { (GetScreenWidth() - width - 250)/2, (GetScreenHeight() - height)/2, width, height };
BeginDrawing();
ClearBackground(RAYWHITE);
DrawLine(560, 0, 560, GetScreenHeight(), Fade(LIGHTGRAY, 0.6f));
DrawRectangle(560, 0, GetScreenWidth() - 500, GetScreenHeight(), Fade(LIGHTGRAY, 0.3f));
if (drawRect) DrawRectangleRec(rec, Fade(GOLD, 0.6));
if (drawRoundedRect) DrawRectangleRounded(rec, roundness, segments, Fade(MAROON, 0.2));
if (drawRoundedLines) DrawRectangleRoundedLines(rec,roundness, segments, lineThick, Fade(MAROON, 0.4));
width = GuiSliderBar((Rectangle){ 640, 40, 105, 20 }, "Width", width, 0, GetScreenWidth() - 300, true );
height = GuiSliderBar((Rectangle){ 640, 70, 105, 20 }, "Height", height, 0, GetScreenHeight() - 50, true);
roundness = GuiSliderBar((Rectangle){ 640, 140, 105, 20 }, "Roundness", roundness, 0.0f, 1.0f, true);
lineThick = GuiSliderBar((Rectangle){ 640, 170, 105, 20 }, "Thickness", lineThick, 0, 20, true);
segments = GuiSliderBar((Rectangle){ 640, 240, 105, 20}, "Segments", segments, 0, 60, true);
drawRoundedRect = GuiCheckBox((Rectangle){ 640, 320, 20, 20 }, "DrawRoundedRect", drawRoundedRect);
drawRoundedLines = GuiCheckBox((Rectangle){ 640, 350, 20, 20 }, "DrawRoundedLines", drawRoundedLines);
drawRect = GuiCheckBox((Rectangle){ 640, 380, 20, 20}, "DrawRect", drawRect);
DrawText(FormatText("MODE: %s", (segments >= 4)? "MANUAL" : "AUTO"), 640, 280, 10, (segments >= 4)? MAROON : DARKGRAY);
DrawFPS(10, 10);
EndDrawing();
}
CloseWindow(); 
return 0;
}
