#include "raylib.h"
#include "screens.h"
static int framesCounter;
static int finishScreen;
static Texture2D background;
static Texture2D title;
static float titleAlpha = 0.0f;
static Sound fxStart;
void InitTitleScreen(void)
{
framesCounter = 0;
finishScreen = 0;
background = LoadTexture("resources/textures/back_title.png");
title = LoadTexture("resources/textures/title.png");
fxStart = LoadSound("resources/audio/start.wav");
}
void UpdateTitleScreen(void)
{
framesCounter++;
titleAlpha += 0.005f;
if (titleAlpha >= 1.0f) titleAlpha = 1.0f;
if ((IsKeyPressed(KEY_ENTER)) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)))
{
PlaySound(fxStart);
finishScreen = 1;
}
}
void DrawTitleScreen(void)
{
DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){ 26, 26, 26, 255 });
DrawTexture(background, GetScreenWidth()/2 - background.width/2, 0, WHITE);
DrawTexture(title, GetScreenWidth()/2 - title.width/2, 30, Fade(WHITE, titleAlpha));
DrawText("(c) Developed by Ramon Santamaria (@raysan5)", 20, GetScreenHeight() - 40, 20, LIGHTGRAY); 
if ((framesCounter > 180) && ((framesCounter/40)%2)) DrawTextEx(font, "PRESS ENTER to START LIGHTING", (Vector2){ 230, 450 }, font.baseSize, -2, WHITE);
}
void UnloadTitleScreen(void)
{
UnloadTexture(background);
UnloadTexture(title);
UnloadSound(fxStart);
}
int FinishTitleScreen(void)
{
return finishScreen;
}
