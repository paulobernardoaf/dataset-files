#include "raylib.h"
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
InitWindow(screenWidth, screenHeight, "raylib [core] example - mouse input");
Vector2 ballPosition = { -100.0f, -100.0f };
Color ballColor = DARKBLUE;
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
ballPosition = GetMousePosition();
if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) ballColor = MAROON;
else if (IsMouseButtonPressed(MOUSE_MIDDLE_BUTTON)) ballColor = LIME;
else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) ballColor = DARKBLUE;
BeginDrawing();
ClearBackground(RAYWHITE);
DrawCircleV(ballPosition, 40, ballColor);
DrawText("move ball with mouse and click mouse button to change color", 10, 10, 20, DARKGRAY);
EndDrawing();
}
CloseWindow(); 
return 0;
}