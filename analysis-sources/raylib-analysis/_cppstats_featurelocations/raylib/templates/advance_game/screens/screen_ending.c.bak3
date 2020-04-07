
























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
PlaySound(fxCoin);
}
}


void DrawEndingScreen(void)
{

DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLUE);
DrawTextEx(font, "ENDING SCREEN", (Vector2){ 20, 10 }, font.baseSize*3, 4, DARKBLUE);
DrawText("PRESS ENTER or TAP to RETURN to TITLE SCREEN", 120, 220, 20, DARKBLUE);
}


void UnloadEndingScreen(void)
{

}


int FinishEndingScreen(void)
{
return finishScreen;
}