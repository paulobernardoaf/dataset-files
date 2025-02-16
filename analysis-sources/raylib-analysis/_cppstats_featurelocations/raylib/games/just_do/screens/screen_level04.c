
























#include "raylib.h"
#include "screens.h"






static int framesCounter;
static int finishScreen;

static Vector2 circlesCenter;
static float innerCircleRadius = 40;
static float outerCircleRadius = 300;

static bool done = false;
static int levelTimeSec = 0;
static bool levelFinished = false;






void InitLevel04Screen(void)
{

framesCounter = 0;
finishScreen = 0;

circlesCenter = (Vector2){ GetScreenWidth()/2, GetScreenHeight()/2 };
}


void UpdateLevel04Screen(void)
{

framesCounter++;

if (!done)
{
if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) 
{
if (CheckCollisionPointCircle(GetMousePosition(), circlesCenter, innerCircleRadius))
{
innerCircleRadius += 2;
}
else if (CheckCollisionPointCircle(GetMousePosition(), circlesCenter, outerCircleRadius))
{
outerCircleRadius += 2;
}
else
{
outerCircleRadius -= 2;

if (outerCircleRadius <= 260) outerCircleRadius = 260; 
}
}
else
{
if (!done)
{
innerCircleRadius -= 2;
if (outerCircleRadius > 300) outerCircleRadius -= 2;
}
}

if (innerCircleRadius >= 270) innerCircleRadius = 270;
else if (innerCircleRadius <= 40) innerCircleRadius = 40;

if (outerCircleRadius >= 600) outerCircleRadius = 600;

if (innerCircleRadius >= outerCircleRadius)
{
done = true;
PlaySound(levelWin);
}
}

if (done && !levelFinished)
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


void DrawLevel04Screen(void)
{


DrawCircleV(circlesCenter, outerCircleRadius, GRAY);
DrawCircleV(circlesCenter, innerCircleRadius, RAYWHITE);

if (levelFinished)
{
DrawRectangleBordersRec((Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()}, 0, 0, 60, Fade(LIGHTGRAY, 0.6f));
DrawText("LEVEL 04", GetScreenWidth()/2 - MeasureText("LEVEL 04", 30)/2, 20, 30, GRAY);
DrawText(FormatText("DONE! (Seconds: %03i)", levelTimeSec), GetScreenWidth()/2 - MeasureText("DONE! (Seconds: 000)", 30)/2, GetScreenHeight() - 40, 30, GRAY);
}
else DrawText("LEVEL 04", GetScreenWidth()/2 - MeasureText("LEVEL 04", 30)/2, 20, 30, LIGHTGRAY);
}


void UnloadLevel04Screen(void)
{

}


int FinishLevel04Screen(void)
{
return finishScreen;
}