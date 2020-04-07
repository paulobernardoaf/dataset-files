#include "raylib.h"
#include "screens.h"
static int framesCounter;
static int finishScreen;
void InitTitleScreen(void)
{
framesCounter = 0;
finishScreen = 0;
}
void UpdateTitleScreen(void)
{
if (IsKeyPressed(KEY_ENTER) || IsGestureDetected(GESTURE_TAP))
{
finishScreen = 2; 
PlaySound(fxCoin);
}
}
void DrawTitleScreen(void)
{
DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), GREEN);
DrawTextEx(font, "TITLE SCREEN", (Vector2){ 20, 10 }, font.baseSize*3, 4, DARKGREEN);
DrawText("PRESS ENTER or TAP to JUMP to GAMEPLAY SCREEN", 120, 220, 20, DARKGREEN);
}
void UnloadTitleScreen(void)
{
}
int FinishTitleScreen(void)
{
return finishScreen;
}