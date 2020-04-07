
























#include "raylib.h"
#include "screens.h"






static int framesCounter;
static int finishScreen;

static Rectangle leftColumnRec, middleColumnRec, rightColumnRec;
static Rectangle movingBox;
static int moveSpeed = 4;

static bool leftColumnActive, middleColumnActive, rightColumnActive;

static bool done = false;
static int levelTimeSec = 0;
static bool levelFinished = false;






void InitLevel10Screen(void)
{

framesCounter = 0;
finishScreen = 0;

movingBox = (Rectangle){ 20, GetScreenHeight()/2 - 20, 40, 40 };

leftColumnRec = (Rectangle){ 240, 0, 100, GetScreenHeight() };
middleColumnRec = (Rectangle){ GetScreenWidth()/2 - 50, 0, 100, GetScreenHeight() };
rightColumnRec = (Rectangle){ 920, 0, 100, GetScreenHeight() };

leftColumnActive = true;
middleColumnActive = false;
rightColumnActive = true;
}


void UpdateLevel10Screen(void)
{

framesCounter++;

if (!done)
{
movingBox.x += moveSpeed;

if (movingBox.x <= 0) moveSpeed *= -1;

if ((leftColumnActive && (CheckCollisionRecs(leftColumnRec, movingBox))) ||
(middleColumnActive && (CheckCollisionRecs(middleColumnRec, movingBox))) ||
(rightColumnActive && (CheckCollisionRecs(rightColumnRec, movingBox)))) moveSpeed *= -1;

if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
{
if (CheckCollisionPointRec(GetMousePosition(), leftColumnRec))
{
middleColumnActive = false;
rightColumnActive = true;
}
else if (CheckCollisionPointRec(GetMousePosition(), middleColumnRec))
{
rightColumnActive = false;
leftColumnActive = true;
}
else if (CheckCollisionPointRec(GetMousePosition(), rightColumnRec))
{
leftColumnActive = false;
middleColumnActive = true;
}
}
}

if (movingBox.x >= 1100) done = true;

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


void DrawLevel10Screen(void)
{

DrawRectangle(1100, GetScreenHeight()/2 - 20, 40, 40, GRAY);

DrawRectangleRec(movingBox, LIGHTGRAY);

if (leftColumnActive) DrawRectangleRec(leftColumnRec, GRAY);
if (middleColumnActive) DrawRectangleRec(middleColumnRec, GRAY);
if (rightColumnActive) DrawRectangleRec(rightColumnRec, GRAY);

if (levelFinished)
{
DrawRectangleBordersRec((Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()}, 0, 0, 60, Fade(LIGHTGRAY, 0.6f));
DrawText("LEVEL 08", GetScreenWidth()/2 - MeasureText("LEVEL 08", 30)/2, 20, 30, GRAY);
DrawText(FormatText("DONE! (Seconds: %03i)", levelTimeSec), GetScreenWidth()/2 - MeasureText("DONE! (Seconds: 000)", 30)/2, GetScreenHeight() - 40, 30, GRAY);
}
else DrawText("LEVEL 08", GetScreenWidth()/2 - MeasureText("LEVEL 08", 30)/2, 20, 30, LIGHTGRAY);
}


void UnloadLevel10Screen(void)
{

}


int FinishLevel10Screen(void)
{
return finishScreen;
}