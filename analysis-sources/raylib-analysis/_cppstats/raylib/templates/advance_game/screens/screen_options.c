#include "raylib.h"
#include "screens.h"
static int framesCounter;
static int finishScreen;
void InitOptionsScreen(void)
{
framesCounter = 0;
finishScreen = 0;
}
void UpdateOptionsScreen(void)
{
}
void DrawOptionsScreen(void)
{
}
void UnloadOptionsScreen(void)
{
}
int FinishOptionsScreen(void)
{
return finishScreen;
}