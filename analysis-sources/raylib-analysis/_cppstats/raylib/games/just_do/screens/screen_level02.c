#include "raylib.h"
#include "screens.h"
#include <math.h>
static int framesCounter;
static int finishScreen;
static Vector2 bouncingBallPos;
static float bouncingBallRadius = 40;
static Vector2 bouncingBallSpeed;
static Vector2 holeCirclePos;
static float holeCircleRadius = 50;
static bool ballOnHole = false;
static int levelTimeSec = 0;
static bool levelFinished = false;
float Vector2Distance(Vector2 v1, Vector2 v2);
void InitLevel02Screen(void)
{
framesCounter = 0;
finishScreen = 0;
bouncingBallPos = (Vector2){ 120, 80 };
bouncingBallSpeed = (Vector2){ 6, 8 };
holeCirclePos = (Vector2){ GetScreenWidth()/2, GetScreenHeight()/2 };
}
void UpdateLevel02Screen(void)
{
framesCounter++;
if (!ballOnHole)
{
bouncingBallPos.x += bouncingBallSpeed.x;
bouncingBallPos.y += bouncingBallSpeed.y;
if (((bouncingBallPos.x - bouncingBallRadius) <= 0) || ((bouncingBallPos.x + bouncingBallRadius) >= GetScreenWidth())) bouncingBallSpeed.x *= -1;
if (((bouncingBallPos.y - bouncingBallRadius) <= 0) || ((bouncingBallPos.y + bouncingBallRadius) >= GetScreenHeight())) bouncingBallSpeed.y *= -1;
Vector2 mousePos = GetMousePosition();
if (CheckCollisionPointCircle(mousePos, bouncingBallPos, 120))
{
bouncingBallPos.x = GetRandomValue(80, 1200);
bouncingBallPos.y = GetRandomValue(80, 650);
}
if (CheckCollisionPointCircle(mousePos, holeCirclePos, 120))
{
if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
{
holeCirclePos = mousePos;
if ((holeCirclePos.x - holeCircleRadius) <= 0) holeCirclePos.x = holeCircleRadius;
else if ((holeCirclePos.x + holeCircleRadius) >= GetScreenWidth()) holeCirclePos.x = GetScreenWidth() - holeCircleRadius;
if ((holeCirclePos.y - holeCircleRadius) <= 0) holeCirclePos.y = holeCircleRadius;
else if ((holeCirclePos.y + holeCircleRadius) >= GetScreenHeight()) holeCirclePos.y = GetScreenHeight() - holeCircleRadius;
}
}
if (Vector2Distance(bouncingBallPos, holeCirclePos) < 20)
{
ballOnHole = true;
PlaySound(levelWin);
}
}
if (ballOnHole && !levelFinished)
{
levelTimeSec = framesCounter/60;
levelFinished = true;
framesCounter = 0;
}
if (levelFinished)
{
framesCounter++;
if ((framesCounter > 90) && (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))) finishScreen = true;
}
}
void DrawLevel02Screen(void)
{
DrawCircleV(holeCirclePos, holeCircleRadius, LIGHTGRAY);
DrawCircleV(bouncingBallPos, bouncingBallRadius, DARKGRAY);
DrawCircleLines(bouncingBallPos.x, bouncingBallPos.y, 120, Fade(LIGHTGRAY, 0.8f));
if (levelFinished)
{
DrawRectangleBordersRec((Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()}, 0, 0, 60, Fade(LIGHTGRAY, 0.6f));
DrawText("LEVEL 02", GetScreenWidth()/2 - MeasureText("LEVEL 02", 30)/2, 20, 30, GRAY);
DrawText(FormatText("DONE! (Seconds: %03i)", levelTimeSec), GetScreenWidth()/2 - MeasureText("DONE! (Seconds: 000)", 30)/2, GetScreenHeight() - 40, 30, GRAY);
}
else DrawText("LEVEL 02", GetScreenWidth()/2 - MeasureText("LEVEL 02", 30)/2, 20, 30, LIGHTGRAY);
}
void UnloadLevel02Screen(void)
{
}
int FinishLevel02Screen(void)
{
return finishScreen;
}