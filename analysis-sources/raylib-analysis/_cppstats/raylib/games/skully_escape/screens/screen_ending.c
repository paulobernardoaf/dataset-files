#include "raylib.h"
#include "screens.h"
static int framesCounter;
static int finishScreen;
static float alpha = 0.0f;
void InitEndingScreen(void)
{
framesCounter = 0;
finishScreen = 0;
alpha = 0.0f;
}
void UpdateEndingScreen(void)
{
framesCounter++;
alpha += 0.005f;
if (alpha >= 1.0f) alpha = 1.0f;
if ((IsKeyPressed(KEY_ENTER)) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)))
{
finishScreen = 1;
}
}
void DrawEndingScreen(void)
{
DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), DARKGRAY);
DrawTextEx(font, "CONGRATULATIONS!", (Vector2){ 50, 160 }, font.baseSize*3, 2, Fade(WHITE, alpha));
DrawTextEx(font, "SKULLY ESCAPED!", (Vector2){ 100, 300 }, font.baseSize*3, 2, Fade(WHITE, alpha));
if ((framesCounter > 180) && ((framesCounter/40)%2)) DrawText("PRESS ENTER or CLICK", 380, 545, 40, BLACK);
}
void UnloadEndingScreen(void)
{
}
int FinishEndingScreen(void)
{
return finishScreen;
}
