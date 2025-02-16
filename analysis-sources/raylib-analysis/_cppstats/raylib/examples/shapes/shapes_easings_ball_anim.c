#include "raylib.h"
#include "easings.h" 
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
InitWindow(screenWidth, screenHeight, "raylib [shapes] example - easings ball anim");
int ballPositionX = -100;
int ballRadius = 20;
float ballAlpha = 0.0f;
int state = 0;
int framesCounter = 0;
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
if (state == 0) 
{
framesCounter++;
ballPositionX = EaseElasticOut(framesCounter, -100, screenWidth/2 + 100, 120);
if (framesCounter >= 120)
{
framesCounter = 0;
state = 1;
}
}
else if (state == 1) 
{
framesCounter++;
ballRadius = EaseElasticIn(framesCounter, 20, 500, 200);
if (framesCounter >= 200)
{
framesCounter = 0;
state = 2;
}
}
else if (state == 2) 
{
framesCounter++;
ballAlpha = EaseCubicOut(framesCounter, 0.0f, 1.0f, 200);
if (framesCounter >= 200)
{
framesCounter = 0;
state = 3;
}
}
else if (state == 3) 
{
if (IsKeyPressed(KEY_ENTER))
{
ballPositionX = -100;
ballRadius = 20;
ballAlpha = 0.0f;
state = 0;
}
}
if (IsKeyPressed(KEY_R)) framesCounter = 0;
BeginDrawing();
ClearBackground(RAYWHITE);
if (state >= 2) DrawRectangle(0, 0, screenWidth, screenHeight, GREEN);
DrawCircle(ballPositionX, 200, ballRadius, Fade(RED, 1.0f - ballAlpha));
if (state == 3) DrawText("PRESS [ENTER] TO PLAY AGAIN!", 240, 200, 20, BLACK);
EndDrawing();
}
CloseWindow(); 
return 0;
}