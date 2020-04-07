#include "raylib.h"
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
InitWindow(screenWidth, screenHeight, "raylib [core] example - input multitouch");
Vector2 ballPosition = { -100.0f, -100.0f };
Color ballColor = BEIGE;
int touchCounter = 0;
Vector2 touchPosition = { 0.0f };
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
ballPosition = GetMousePosition();
ballColor = BEIGE;
if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) ballColor = MAROON;
if (IsMouseButtonDown(MOUSE_MIDDLE_BUTTON)) ballColor = LIME;
if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) ballColor = DARKBLUE;
if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) touchCounter = 10;
if (IsMouseButtonPressed(MOUSE_MIDDLE_BUTTON)) touchCounter = 10;
if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) touchCounter = 10;
if (touchCounter > 0) touchCounter--;
BeginDrawing();
ClearBackground(RAYWHITE);
for (int i = 0; i < MAX_TOUCH_POINTS; ++i)
{
touchPosition = GetTouchPosition(i); 
if ((touchPosition.x >= 0) && (touchPosition.y >= 0)) 
{
DrawCircleV(touchPosition, 34, ORANGE);
DrawText(FormatText("%d", i), touchPosition.x - 10, touchPosition.y - 70, 40, BLACK);
}
}
DrawCircleV(ballPosition, 30 + (touchCounter*3), ballColor);
DrawText("move ball with mouse and click mouse button to change color", 10, 10, 20, DARKGRAY);
DrawText("touch the screen at multiple locations to get multiple balls", 10, 30, 20, DARKGRAY);
EndDrawing();
}
CloseWindow(); 
return 0;
}