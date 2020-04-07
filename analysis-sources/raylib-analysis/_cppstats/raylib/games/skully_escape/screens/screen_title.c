#include "raylib.h"
#include "screens.h"
static int framesCounter;
static int finishScreen;
static Texture2D title;
static float titleAlpha = 0.0f;
void InitTitleScreen(void)
{
framesCounter = 0;
finishScreen = 0;
title = LoadTexture("resources/textures/title.png");
}
void UpdateTitleScreen(void)
{
framesCounter++;
titleAlpha += 0.005f;
if (titleAlpha >= 1.0f) titleAlpha = 1.0f;
if ((IsKeyPressed(KEY_ENTER)) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)))
{
finishScreen = 1;
}
}
void DrawTitleScreen(void)
{
DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), DARKGRAY);
DrawTexture(title, GetScreenWidth()/2 - title.width/2, 20, Fade(WHITE, titleAlpha));
if ((framesCounter > 180) && ((framesCounter/40)%2)) DrawText("PRESS ENTER to START", 380, 545, 40, BLACK);
}
void UnloadTitleScreen(void)
{
UnloadTexture(title);
}
int FinishTitleScreen(void)
{
return finishScreen;
}