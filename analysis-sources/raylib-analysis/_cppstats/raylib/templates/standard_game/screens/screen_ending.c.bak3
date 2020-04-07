
























#include "raylib.h"
#include "screens.h"






static int framesCounter;
static int finishScreen;






void InitEndingScreen(void)
{

framesCounter = 0;
finishScreen = 0;
}


void UpdateEndingScreen(void)
{



if (IsKeyPressed(KEY_ENTER) || IsGestureDetected(GESTURE_TAP))
{
finishScreen = 1;
}
}


void DrawEndingScreen(void)
{

DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLUE);
DrawText("ENDING SCREEN", 20, 20, 40, DARKBLUE);
DrawText("PRESS ENTER to RETURN to TITLE SCREEN", 120, 220, 20, DARKBLUE);
}


void UnloadEndingScreen(void)
{

}


int FinishEndingScreen(void)
{
return finishScreen;
}