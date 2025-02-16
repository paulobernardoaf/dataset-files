
























#include "raylib.h"
#include "screens.h"






static int framesCounter;
static int finishScreen;

static Vector2 leftCirclePos, middleCirclePos, rightCirclePos;
static Vector2 leftBtnPos, middleBtnPos, rightBtnPos;
static float circleRadius = 100;
static float btnRadius = 80;

static bool leftCircleActive, middleCircleActive, rightCircleActive;
static Color leftCircleColor, middleCircleColor, rightCircleColor;

static bool done = false;
static int levelTimeSec = 0;
static bool levelFinished = false;




static bool CheckColor(Color col1, Color col2);


void InitLevel07Screen(void)
{

framesCounter = 0;
finishScreen = 0;

leftCirclePos = (Vector2){ GetScreenWidth()/2 - 340, GetScreenHeight()/2 - 100 };
middleCirclePos = (Vector2){ GetScreenWidth()/2, GetScreenHeight()/2 - 100 };
rightCirclePos = (Vector2){ GetScreenWidth()/2 + 340, GetScreenHeight()/2 - 100 };

leftBtnPos = (Vector2){ GetScreenWidth()/2 - 340, GetScreenHeight()/2 + 120 };
middleBtnPos = (Vector2){ GetScreenWidth()/2, GetScreenHeight()/2 + 120 };
rightBtnPos = (Vector2){ GetScreenWidth()/2 + 340, GetScreenHeight()/2 + 120 };

leftCircleActive = false;
middleCircleActive = true;
rightCircleActive = false;

leftCircleColor = GRAY;
middleCircleColor = GRAY;
rightCircleColor = GRAY;
}


void UpdateLevel07Screen(void)
{

framesCounter++;

if (!done)
{
if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
{
if (CheckCollisionPointCircle(GetMousePosition(), leftBtnPos, btnRadius)) leftCircleActive = !leftCircleActive;
else if (CheckCollisionPointCircle(GetMousePosition(), middleBtnPos, btnRadius)) middleCircleActive = !middleCircleActive;
else if (CheckCollisionPointCircle(GetMousePosition(), rightBtnPos, btnRadius)) rightCircleActive = !rightCircleActive;

if (rightCircleActive && CheckCollisionPointCircle(GetMousePosition(), leftCirclePos, circleRadius))
{
if (CheckColor(leftCircleColor, GRAY)) leftCircleColor = LIGHTGRAY;
else leftCircleColor = GRAY;
}

if (middleCircleActive && CheckCollisionPointCircle(GetMousePosition(), middleCirclePos, circleRadius))
{
if (CheckColor(middleCircleColor, GRAY)) middleCircleColor = LIGHTGRAY;
else middleCircleColor = GRAY;
}

if (rightCircleActive && leftCircleActive && CheckCollisionPointCircle(GetMousePosition(), rightCirclePos, circleRadius))
{
if (CheckColor(rightCircleColor, GRAY)) rightCircleColor = LIGHTGRAY;
else rightCircleColor = GRAY;
}
}


if (CheckColor(leftCircleColor, LIGHTGRAY) && CheckColor(middleCircleColor, LIGHTGRAY) && CheckColor(rightCircleColor, LIGHTGRAY) &&
!leftCircleActive && !middleCircleActive && !rightCircleActive)
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


void DrawLevel07Screen(void)
{

DrawCircleV(leftCirclePos, circleRadius, leftCircleColor);
DrawCircleV(middleCirclePos, circleRadius, middleCircleColor);
DrawCircleV(rightCirclePos, circleRadius, rightCircleColor);

if (leftCircleActive) DrawCircleV(leftBtnPos, btnRadius, GRAY);
else DrawCircleV(leftBtnPos, btnRadius, LIGHTGRAY);

if (middleCircleActive) DrawCircleV(middleBtnPos, btnRadius, GRAY);
else DrawCircleV(middleBtnPos, btnRadius, LIGHTGRAY);

if (rightCircleActive) DrawCircleV(rightBtnPos, btnRadius, GRAY);
else DrawCircleV(rightBtnPos, btnRadius, LIGHTGRAY);



if (levelFinished)
{
DrawRectangleBordersRec((Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()}, 0, 0, 60, Fade(LIGHTGRAY, 0.6f));
DrawText("LEVEL 07", GetScreenWidth()/2 - MeasureText("LEVEL 07", 30)/2, 20, 30, GRAY);
DrawText(FormatText("DONE! (Seconds: %03i)", levelTimeSec), GetScreenWidth()/2 - MeasureText("DONE! (Seconds: 000)", 30)/2, GetScreenHeight() - 40, 30, GRAY);
}
else DrawText("LEVEL 07", GetScreenWidth()/2 - MeasureText("LEVEL 07", 30)/2, 20, 30, LIGHTGRAY);
}


void UnloadLevel07Screen(void)
{

}


int FinishLevel07Screen(void)
{
return finishScreen;
}

static bool CheckColor(Color col1, Color col2)
{
return ((col1.r == col2.r) && (col1.g == col2.g) && (col1.b == col2.b) && (col1.a == col2.a));
}