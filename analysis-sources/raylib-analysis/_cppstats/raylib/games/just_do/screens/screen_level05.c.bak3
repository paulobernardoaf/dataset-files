
























#include "raylib.h"
#include "screens.h"

#define NUM_CIRCLES 10






static int framesCounter;
static int finishScreen;

static Vector2 circleCenter;
static float circleRadius[NUM_CIRCLES];
static bool circleLocked[NUM_CIRCLES];
static Color circleColor[NUM_CIRCLES];

static bool done = false;
static int levelTimeSec = 0;
static bool levelFinished = false;




static bool CheckColor(Color col1, Color col2);


void InitLevel05Screen(void)
{

framesCounter = 0;
finishScreen = 0;

circleCenter = (Vector2){ GetScreenWidth()/2, GetScreenHeight()/2 };

for (int i = 0; i < NUM_CIRCLES; i++)
{
circleRadius[i] = 760/NUM_CIRCLES*(NUM_CIRCLES - i);
circleLocked[i] = false;
}


circleColor[9] = GRAY;
circleColor[8] = RAYWHITE;
circleColor[7] = RAYWHITE;
circleColor[6] = GRAY;
circleColor[5] = RAYWHITE;
circleColor[4] = GRAY;
circleColor[3] = GRAY;
circleColor[2] = GRAY;
circleColor[1] = RAYWHITE;
circleColor[0] = GRAY;
}


void UpdateLevel05Screen(void)
{

framesCounter++;

if (!done)
{
if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
{
for (int i = NUM_CIRCLES - 1; i >= 0; i--)
{
if (CheckCollisionPointCircle(GetMousePosition(), circleCenter, circleRadius[i]))
{
if (i == 0)
{
if (CheckColor(circleColor[8], GRAY)) circleColor[8] = RAYWHITE;
else circleColor[8] = GRAY;
}
else if (i == 2)
{
if (CheckColor(circleColor[5], GRAY)) circleColor[5] = RAYWHITE;
else circleColor[5] = GRAY;
}
else if (i == 3)
{
if (CheckColor(circleColor[6], GRAY)) circleColor[6] = RAYWHITE;
else circleColor[6] = GRAY;
}
else
{
if (CheckColor(circleColor[i], GRAY)) circleColor[i] = RAYWHITE;
else circleColor[i] = GRAY;
}
return;
}
}
}


for (int i = 0; i < NUM_CIRCLES; i++)
{
done = true;

if (CheckColor(circleColor[i], RAYWHITE))
{
done = false;
return;
}


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


void DrawLevel05Screen(void)
{

for (int i = 0; i < NUM_CIRCLES; i++)
{
DrawPoly(circleCenter, 64, circleRadius[i], 0.0f, circleColor[i]);
}



if (levelFinished)
{
DrawRectangleBordersRec((Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()}, 0, 0, 60, Fade(LIGHTGRAY, 0.6f));
DrawText("LEVEL 05", GetScreenWidth()/2 - MeasureText("LEVEL 05", 30)/2, 20, 30, GRAY);
DrawText(FormatText("DONE! (Seconds: %03i)", levelTimeSec), GetScreenWidth()/2 - MeasureText("DONE! (Seconds: 000)", 30)/2, GetScreenHeight() - 40, 30, GRAY);
}
else DrawText("LEVEL 05", GetScreenWidth()/2 - MeasureText("LEVEL 05", 30)/2, 20, 30, LIGHTGRAY);
}


void UnloadLevel05Screen(void)
{

}


int FinishLevel05Screen(void)
{
return finishScreen;
}

static bool CheckColor(Color col1, Color col2)
{
return ((col1.r == col2.r) && (col1.g == col2.g) && (col1.b == col2.b) && (col1.a == col2.a));
}