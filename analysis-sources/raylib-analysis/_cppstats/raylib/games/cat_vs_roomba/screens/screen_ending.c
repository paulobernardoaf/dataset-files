#include "raylib.h"
#include "screens.h"
static int framesCounter;
static int finishScreen;
static int scrollPositionX;
void InitEndingScreen(void)
{
framesCounter = 0;
finishScreen = 0;
PlayMusicStream(music);
}
void UpdateEndingScreen(void)
{
framesCounter++;
scrollPositionX -= 5;
if (scrollPositionX < -GetScreenWidth()) scrollPositionX = 0;
if (IsKeyPressed(KEY_ENTER) || IsGestureDetected(GESTURE_TAP))
{
finishScreen = 1;
PlaySound(fxCoin);
}
}
void DrawEndingScreen(void)
{
for (int i = 0; i < 64*2*2; i++)
{
DrawRectangle(64*i + scrollPositionX, 0, 64, GetScreenHeight(), (i%2 == 0)? GetColor(0xf3726dff) : GetColor(0xffcf6bff));
}
if (result == 0) DrawTextEx(font2, "YOU LOOSE...", (Vector2){ 350, 200 }, font2.baseSize*2, 2, WHITE);
else if (result == 1) DrawTextEx(font, "YOU WIN!!!", (Vector2){ 380, 200 }, font.baseSize*2, 2, WHITE);
DrawTextEx(font, FormatText("FINAL SCORE: %i", score), (Vector2){ 400, 360 }, font2.baseSize, 2, WHITE);
if ((framesCounter/30)%2) DrawTextEx(font2, "PRESS ENTER to TITLE", (Vector2){ 340, 550 }, font2.baseSize, 2, WHITE);
}
void UnloadEndingScreen(void)
{
}
int FinishEndingScreen(void)
{
return finishScreen;
}