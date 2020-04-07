#include "raylib.h"
#define MAX_COLORS_COUNT 21 
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
InitWindow(screenWidth, screenHeight, "raylib [shapes] example - colors palette");
Color colors[MAX_COLORS_COUNT] = {
DARKGRAY, MAROON, ORANGE, DARKGREEN, DARKBLUE, DARKPURPLE, DARKBROWN,
GRAY, RED, GOLD, LIME, BLUE, VIOLET, BROWN, LIGHTGRAY, PINK, YELLOW,
GREEN, SKYBLUE, PURPLE, BEIGE };
const char *colorNames[MAX_COLORS_COUNT] = {
"DARKGRAY", "MAROON", "ORANGE", "DARKGREEN", "DARKBLUE", "DARKPURPLE",
"DARKBROWN", "GRAY", "RED", "GOLD", "LIME", "BLUE", "VIOLET", "BROWN",
"LIGHTGRAY", "PINK", "YELLOW", "GREEN", "SKYBLUE", "PURPLE", "BEIGE" };
Rectangle colorsRecs[MAX_COLORS_COUNT] = { 0 }; 
for (int i = 0; i < MAX_COLORS_COUNT; i++)
{
colorsRecs[i].x = 20 + 100*(i%7) + 10*(i%7);
colorsRecs[i].y = 80 + 100*(i/7) + 10*(i/7);
colorsRecs[i].width = 100;
colorsRecs[i].height = 100;
}
int colorState[MAX_COLORS_COUNT] = { 0 }; 
Vector2 mousePoint = { 0.0f, 0.0f };
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
mousePoint = GetMousePosition();
for (int i = 0; i < MAX_COLORS_COUNT; i++)
{
if (CheckCollisionPointRec(mousePoint, colorsRecs[i])) colorState[i] = 1;
else colorState[i] = 0;
}
BeginDrawing();
ClearBackground(RAYWHITE);
DrawText("raylib colors palette", 28, 42, 20, BLACK);
DrawText("press SPACE to see all colors", GetScreenWidth() - 180, GetScreenHeight() - 40, 10, GRAY);
for (int i = 0; i < MAX_COLORS_COUNT; i++) 
{
DrawRectangleRec(colorsRecs[i], Fade(colors[i], colorState[i]? 0.6f : 1.0f));
if (IsKeyDown(KEY_SPACE) || colorState[i])
{
DrawRectangle(colorsRecs[i].x, colorsRecs[i].y + colorsRecs[i].height - 26, colorsRecs[i].width, 20, BLACK);
DrawRectangleLinesEx(colorsRecs[i], 6, Fade(BLACK, 0.3f));
DrawText(colorNames[i], colorsRecs[i].x + colorsRecs[i].width - MeasureText(colorNames[i], 10) - 12,
colorsRecs[i].y + colorsRecs[i].height - 20, 10, colors[i]);
}
}
EndDrawing();
}
CloseWindow(); 
return 0;
}