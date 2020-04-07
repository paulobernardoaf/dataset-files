#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include "raygui.h" 
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
InitWindow(screenWidth, screenHeight, "raylib [shapes] example - draw circle sector");
Vector2 center = {(GetScreenWidth() - 300)/2, GetScreenHeight()/2 };
float outerRadius = 180.f;
int startAngle = 0;
int endAngle = 180;
int segments = 0;
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
BeginDrawing();
ClearBackground(RAYWHITE);
DrawLine(500, 0, 500, GetScreenHeight(), Fade(LIGHTGRAY, 0.6f));
DrawRectangle(500, 0, GetScreenWidth() - 500, GetScreenHeight(), Fade(LIGHTGRAY, 0.3f));
DrawCircleSector(center, outerRadius, startAngle, endAngle, segments, Fade(MAROON, 0.3));
DrawCircleSectorLines(center, outerRadius, startAngle, endAngle, segments, Fade(MAROON, 0.6));
startAngle = GuiSliderBar((Rectangle){ 600, 40, 120, 20}, "StartAngle", startAngle, 0, 720, true );
endAngle = GuiSliderBar((Rectangle){ 600, 70, 120, 20}, "EndAngle", endAngle, 0, 720, true);
outerRadius = GuiSliderBar((Rectangle){ 600, 140, 120, 20}, "Radius", outerRadius, 0, 200, true);
segments = GuiSliderBar((Rectangle){ 600, 170, 120, 20}, "Segments", segments, 0, 100, true);
DrawText(FormatText("MODE: %s", (segments >= 4)? "MANUAL" : "AUTO"), 600, 200, 10, (segments >= 4)? MAROON : DARKGRAY);
DrawFPS(10, 10);
EndDrawing();
}
CloseWindow(); 
return 0;
}