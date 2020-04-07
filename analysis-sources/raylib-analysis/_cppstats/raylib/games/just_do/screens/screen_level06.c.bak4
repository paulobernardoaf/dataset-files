
























#include "raylib.h"
#include "screens.h"






static int framesCounter;
static int finishScreen;

static Rectangle centerRec;

static Rectangle movingRecs[4];
static int speedRecs[4];
static bool stoppedRec[4];
static int mouseOverNum = -1;

static bool done = false;
static int levelTimeSec = 0;
static bool levelFinished = false;






void InitLevel06Screen(void)
{

framesCounter = 0;
finishScreen = 0;

centerRec = (Rectangle){ GetScreenWidth()/2 - 100, 0, 200, GetScreenHeight() };

for (int i = 0; i < 4; i++)
{
movingRecs[i] = (Rectangle){ GetRandomValue(0, 5)*150, (i*150) + 90, 100, 100 };
stoppedRec[i] = false;
speedRecs[i] = GetRandomValue(4, 8);
}
}


void UpdateLevel06Screen(void)
{

framesCounter++;

if (!done)
{
for (int i = 0; i < 4; i++)
{
if (!stoppedRec[i]) movingRecs[i].x += speedRecs[i];

if (movingRecs[i].x >= GetScreenWidth()) movingRecs[i].x = -movingRecs[i].width;

if (CheckCollisionPointRec(GetMousePosition(), movingRecs[i]))
{
mouseOverNum = i;

if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
{
if (i == 0) stoppedRec[3] = !stoppedRec[3];
else if (i == 1) stoppedRec[2] = !stoppedRec[2];
else if (i == 2) stoppedRec[0] = !stoppedRec[0];
else if (i == 3) stoppedRec[1] = !stoppedRec[1];
}
}
}


if (((movingRecs[0].x > centerRec.x) && ((movingRecs[0].x + movingRecs[0].width) < (centerRec.x + centerRec.width))) &&
((movingRecs[1].x > centerRec.x) && ((movingRecs[1].x + movingRecs[1].width) < (centerRec.x + centerRec.width))) &&
((movingRecs[2].x > centerRec.x) && ((movingRecs[2].x + movingRecs[2].width) < (centerRec.x + centerRec.width))) &&
((movingRecs[3].x > centerRec.x) && ((movingRecs[3].x + movingRecs[3].width) < (centerRec.x + centerRec.width))))
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


void DrawLevel06Screen(void)
{

DrawRectangleRec(centerRec, LIGHTGRAY);

for (int i = 0; i < 4; i++)
{
DrawRectangleRec(movingRecs[i], GRAY);
}

if (!done && (mouseOverNum >= 0)) DrawRectangleLines(movingRecs[mouseOverNum].x - 5, movingRecs[mouseOverNum].y - 5, movingRecs[mouseOverNum].width + 10, movingRecs[mouseOverNum].height + 10, Fade(LIGHTGRAY, 0.8f));

if (levelFinished)
{
DrawRectangleBordersRec((Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()}, 0, 0, 60, Fade(LIGHTGRAY, 0.6f));
DrawText("LEVEL 06", GetScreenWidth()/2 - MeasureText("LEVEL 06", 30)/2, 20, 30, GRAY);
DrawText(FormatText("DONE! (Seconds: %03i)", levelTimeSec), GetScreenWidth()/2 - MeasureText("DONE! (Seconds: 000)", 30)/2, GetScreenHeight() - 40, 30, GRAY);
}
else DrawText("LEVEL 06", GetScreenWidth()/2 - MeasureText("LEVEL 06", 30)/2, 20, 30, LIGHTGRAY);
}


void UnloadLevel06Screen(void)
{

}


int FinishLevel06Screen(void)
{
return finishScreen;
}