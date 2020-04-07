#include "raylib.h"
#include <stdlib.h> 
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
InitWindow(screenWidth, screenHeight, "raylib [shapes] example - collision area");
Rectangle boxA = { 10, GetScreenHeight()/2 - 50, 200, 100 };
int boxASpeedX = 4;
Rectangle boxB = { GetScreenWidth()/2 - 30, GetScreenHeight()/2 - 30, 60, 60 };
Rectangle boxCollision = { 0 }; 
int screenUpperLimit = 40; 
bool pause = false; 
bool collision = false; 
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
if (!pause) boxA.x += boxASpeedX;
if (((boxA.x + boxA.width) >= GetScreenWidth()) || (boxA.x <= 0)) boxASpeedX *= -1;
boxB.x = GetMouseX() - boxB.width/2;
boxB.y = GetMouseY() - boxB.height/2;
if ((boxB.x + boxB.width) >= GetScreenWidth()) boxB.x = GetScreenWidth() - boxB.width;
else if (boxB.x <= 0) boxB.x = 0;
if ((boxB.y + boxB.height) >= GetScreenHeight()) boxB.y = GetScreenHeight() - boxB.height;
else if (boxB.y <= screenUpperLimit) boxB.y = screenUpperLimit;
collision = CheckCollisionRecs(boxA, boxB);
if (collision) boxCollision = GetCollisionRec(boxA, boxB);
if (IsKeyPressed(KEY_SPACE)) pause = !pause;
BeginDrawing();
ClearBackground(RAYWHITE);
DrawRectangle(0, 0, screenWidth, screenUpperLimit, collision? RED : BLACK);
DrawRectangleRec(boxA, GOLD);
DrawRectangleRec(boxB, BLUE);
if (collision)
{
DrawRectangleRec(boxCollision, LIME);
DrawText("COLLISION!", GetScreenWidth()/2 - MeasureText("COLLISION!", 20)/2, screenUpperLimit/2 - 10, 20, BLACK);
DrawText(FormatText("Collision Area: %i", (int)boxCollision.width*(int)boxCollision.height), GetScreenWidth()/2 - 100, screenUpperLimit + 10, 20, BLACK);
}
DrawFPS(10, 10);
EndDrawing();
}
CloseWindow(); 
return 0;
}