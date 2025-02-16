#include "raylib.h"
#include "screens.h"
static int framesCounter;
static int finishScreen;
static Rectangle boundsU, boundsO;
static bool mouseOverU = false;
static bool mouseOverO = false;
static bool placedU = false;
static bool placedO = false;
static bool done = false;
static int levelTimeSec = 0;
static bool levelFinished = false;
void InitLevel00Screen(void)
{
framesCounter = 0;
finishScreen = 0;
boundsU = (Rectangle){GetScreenWidth()/2 - 265, -200, MeasureText("U", 160) + 40, 160 };
boundsO = (Rectangle){GetScreenWidth() - 370, -30, MeasureText("O", 160) + 40, 160 };
}
void UpdateLevel00Screen(void)
{
if (!done) framesCounter++;
if (!done)
{
if (!placedU) boundsU.y += 2;
if (boundsU.y >= GetScreenHeight()) boundsU.y = -boundsU.height;
Vector2 mousePos = GetMousePosition();
if (CheckCollisionPointRec(mousePos, boundsU))
{
mouseOverU = true;
if (!placedU && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
{
if ((boundsU.y > GetScreenHeight()/2 - 110) && ((boundsU.y + boundsU.height) < (GetScreenHeight()/2 + 100)))
{
placedU = true;
}
}
}
else mouseOverU = false;
if (CheckCollisionPointRec(mousePos, boundsO))
{
mouseOverO = true;
if (!placedO && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) boundsO.y += 100;
if (boundsO.y >= (GetScreenHeight()/2 - 130)) placedO = true;
}
else mouseOverO = false;
if (placedO && placedU)
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
if ((framesCounter > 30) && (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))) finishScreen = true;
}
}
void DrawLevel00Screen(void)
{
DrawText("U", boundsU.x, boundsU.y + 10, 160, GRAY);
DrawText("J", GetScreenWidth()/2 - MeasureText("JUST DO", 160)/2, GetScreenHeight()/2 - 80, 160, GRAY);
DrawText("ST D", GetScreenWidth()/2 - MeasureText("JUST DO", 160)/2 + 210, GetScreenHeight()/2 - 80, 160, GRAY);
DrawText("O", boundsO.x, boundsO.y + 10, 160, GRAY);
DrawText("by RAMON SANTAMARIA (@raysan5)", 370, GetScreenHeight()/2 + 100, 30, Fade(LIGHTGRAY, 0.4f));
if (mouseOverU && !placedU) DrawRectangleLines(boundsU.x - 20, boundsU.y, boundsU.width, boundsU.height, Fade(LIGHTGRAY, 0.8f));
if (mouseOverO && !placedO) DrawRectangleLines(boundsO.x - 20, boundsO.y, boundsO.width, boundsO.height, Fade(LIGHTGRAY, 0.8f));
if (levelFinished)
{
DrawRectangleBordersRec((Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()}, 0, 0, 60, Fade(LIGHTGRAY, 0.6f));
DrawText("LEVEL 00", GetScreenWidth()/2 - MeasureText("LEVEL 00", 30)/2, 20, 30, GRAY);
DrawText(FormatText("DONE! (Seconds: %03i)", levelTimeSec), GetScreenWidth()/2 - MeasureText("DONE! (Seconds: 000)", 30)/2, GetScreenHeight() - 40, 30, GRAY);
}
else DrawText("LEVEL 00", GetScreenWidth()/2 - MeasureText("LEVEL 00", 30)/2, 20, 30, LIGHTGRAY);
}
void UnloadLevel00Screen(void)
{
}
int FinishLevel00Screen(void)
{
return finishScreen;
}
void DrawRectangleBordersRec(Rectangle rec, int offsetX, int offsetY, int borderSize, Color col)
{
DrawRectangle(rec.x + offsetX, rec.y + offsetY, rec.width, borderSize, col);
DrawRectangle(rec.x + offsetX, rec.y + borderSize + offsetY, borderSize, rec.height - borderSize*2, col);
DrawRectangle(rec.x + rec.width - borderSize + offsetX, rec.y + borderSize + offsetY, borderSize, rec.height - borderSize*2, col);
DrawRectangle(rec.x + offsetX, rec.y + rec.height - borderSize + offsetY, rec.width, borderSize, col);
}