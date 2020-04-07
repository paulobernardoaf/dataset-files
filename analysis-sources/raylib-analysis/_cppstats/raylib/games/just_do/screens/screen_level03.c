#include "raylib.h"
#include "screens.h"
static int framesCounter;
static int finishScreen;
static Rectangle holeRec, pieceRec;
static bool showPiece = false;
static bool pieceSelected = false;
static bool done = false;
static int levelTimeSec = 0;
static bool levelFinished = false;
void InitLevel03Screen(void)
{
framesCounter = 0;
finishScreen = 0;
holeRec = (Rectangle){ GetScreenWidth()/2 - 50, GetScreenHeight()/2 - 50, 100, 100 };
pieceRec = (Rectangle){ 200, 400, 100, 100 };
}
void UpdateLevel03Screen(void)
{
framesCounter++;
Vector2 mousePos = GetMousePosition();
if (!done)
{
if (CheckCollisionPointRec(mousePos, holeRec)) showPiece = true;
else showPiece = false;
if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
{
if (CheckCollisionPointRec(mousePos, pieceRec))
{
pieceSelected = true;
pieceRec.x = ((int)mousePos.x - 50);
pieceRec.y = ((int)mousePos.y - 50);
}
}
if ((pieceRec.x == holeRec.x) && !(CheckCollisionPointRec(mousePos, holeRec)))
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
void DrawLevel03Screen(void)
{
DrawRectangleRec(holeRec, GRAY);
DrawRectangleRec(pieceRec, RAYWHITE);
if (showPiece) DrawRectangleLines(pieceRec.x, pieceRec.y, pieceRec.width, pieceRec.height, Fade(LIGHTGRAY, 0.8f));
if (levelFinished)
{
DrawRectangleBordersRec((Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()}, 0, 0, 60, Fade(LIGHTGRAY, 0.6f));
DrawText("LEVEL 03", GetScreenWidth()/2 - MeasureText("LEVEL 03", 30)/2, 20, 30, GRAY);
DrawText(FormatText("DONE! (Seconds: %03i)", levelTimeSec), GetScreenWidth()/2 - MeasureText("DONE! (Seconds: 000)", 30)/2, GetScreenHeight() - 40, 30, GRAY);
}
else DrawText("LEVEL 03", GetScreenWidth()/2 - MeasureText("LEVEL 03", 30)/2, 20, 30, LIGHTGRAY);
}
void UnloadLevel03Screen(void)
{
}
int FinishLevel03Screen(void)
{
return finishScreen;
}