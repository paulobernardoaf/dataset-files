#include "raylib.h"
#include "screens.h"
static int framesCounter;
static int finishScreen;
static Rectangle innerLeftRec, outerLeftRec;
static Rectangle innerRightRec, outerRightRec;
static bool done = false;
static int levelTimeSec = 0;
static bool levelFinished = false;
void InitLevel01Screen(void)
{
framesCounter = 0;
finishScreen = 0;
outerLeftRec = (Rectangle){ 0, 0, GetScreenWidth()/2, GetScreenHeight() };
outerRightRec = (Rectangle){ GetScreenWidth()/2, 0, GetScreenWidth()/2, GetScreenHeight() };
innerLeftRec = (Rectangle){ GetScreenWidth()/4 - 200, GetScreenHeight()/2 - 200, 400, 400};
innerRightRec = (Rectangle){ GetScreenWidth()/2 + GetScreenWidth()/4 - 200, GetScreenHeight()/2 - 200, 400, 400};
}
void UpdateLevel01Screen(void)
{
framesCounter++;
if (!done)
{
if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
{
if (CheckCollisionPointRec(GetMousePosition(), innerLeftRec))
{
if (innerRightRec.width > 0)
{
innerRightRec.x += 20;
innerRightRec.y += 20;
innerRightRec.width -= 40;
innerRightRec.height -= 40;
}
}
else if (CheckCollisionPointRec(GetMousePosition(), innerRightRec))
{
if (innerLeftRec.width > 0)
{
innerLeftRec.x += 20;
innerLeftRec.y += 20;
innerLeftRec.width -= 40;
innerLeftRec.height -= 40;
}
}
else if (CheckCollisionPointRec(GetMousePosition(), outerLeftRec))
{
innerLeftRec.x -= 20;
innerLeftRec.y -= 20;
innerLeftRec.width += 40;
innerLeftRec.height += 40;
}
else if (CheckCollisionPointRec(GetMousePosition(), outerRightRec))
{
innerRightRec.x -= 20;
innerRightRec.y -= 20;
innerRightRec.width += 40;
innerRightRec.height += 40;
}
}
if (((innerRightRec.width == 0) && (innerLeftRec.height >= GetScreenHeight())) ||
((innerLeftRec.width == 0) && (innerRightRec.height >= GetScreenHeight())))
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
void DrawLevel01Screen(void)
{
if (!levelFinished) DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), LIGHTGRAY);
else DrawRectangle(60, 60, GetScreenWidth() - 120, GetScreenHeight() - 120, LIGHTGRAY);
DrawRectangleRec(outerLeftRec, GRAY);
DrawRectangleRec(innerLeftRec, RAYWHITE);
DrawRectangleRec(outerRightRec, RAYWHITE);
DrawRectangleRec(innerRightRec, GRAY);
if (levelFinished)
{
DrawRectangleBordersRec((Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()}, 0, 0, 60, Fade(LIGHTGRAY, 0.6f));
DrawText("LEVEL 01", GetScreenWidth()/2 - MeasureText("LEVEL 01", 30)/2, 20, 30, GRAY);
DrawText(FormatText("DONE! (Seconds: %03i)", levelTimeSec), GetScreenWidth()/2 - MeasureText("DONE! (Seconds: 000)", 30)/2, GetScreenHeight() - 40, 30, GRAY);
}
else DrawText("LEVEL 01", GetScreenWidth()/2 - MeasureText("LEVEL 01", 30)/2, 20, 30, LIGHTGRAY);
}
void UnloadLevel01Screen(void)
{
}
int FinishLevel01Screen(void)
{
return finishScreen;
}