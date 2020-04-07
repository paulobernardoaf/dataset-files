#include "raylib.h"
#include "screens.h"
static int framesCounter;
static int finishScreen;
void InitGameplayScreen(void)
{
framesCounter = 0;
finishScreen = 0;
}
void UpdateGameplayScreen(void)
{
if (IsKeyPressed(KEY_ENTER) || IsGestureDetected(GESTURE_TAP))
{
finishScreen = 1;
}
}
void DrawGameplayScreen(void)
{
DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), PURPLE);
DrawText("GAMEPLAY SCREEN", 20, 20, 40, MAROON);
DrawText("PRESS ENTER or TAP to JUMP to ENDING SCREEN", 130, 220, 20, MAROON);
}
void UnloadGameplayScreen(void)
{
}
int FinishGameplayScreen(void)
{
return finishScreen;
}