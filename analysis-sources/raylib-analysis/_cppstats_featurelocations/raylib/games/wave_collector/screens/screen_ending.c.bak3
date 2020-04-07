
























#include "raylib.h"
#include "screens.h"






static int framesCounter;
static int finishScreen;

static Texture2D texBackground;
static Texture2D texWin;
static Texture2D texLose;
static Texture2D texLogo;






void InitEndingScreen(void)
{

framesCounter = 0;
finishScreen = 0;

texBackground = LoadTexture("resources/textures/background.png");
texWin = LoadTexture("resources/textures/win.png");
texLose = LoadTexture("resources/textures/lose.png");
texLogo = LoadTexture("resources/textures/logo_raylib.png");
}


void UpdateEndingScreen(void)
{

framesCounter++;


if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
{
finishScreen = 1;
}
}


void DrawEndingScreen(void)
{
DrawTexture(texBackground, 0, 0, WHITE);

if (endingStatus == 1) 
{
DrawTexture(texWin, GetScreenWidth()/2 - texWin.width/2, 90, WHITE);
DrawTextEx(font, "congrats, you got the wave!", (Vector2){ 200, 335 }, font.baseSize, 0, WHITE);
}
else if (endingStatus == 2) 
{
DrawTexture(texLose, GetScreenWidth()/2 - texWin.width/2, 90, WHITE);
DrawTextEx(font, "it seems you lose the wave...", (Vector2){ 205, 335 }, font.baseSize, 0, WHITE);
}

DrawRectangle(0, GetScreenHeight() - 70, 560, 40, Fade(RAYWHITE, 0.8f));
DrawText("(c) Developed by Ramon Santamaria (@raysan5)", 36, GetScreenHeight() - 60, 20, DARKBLUE);

DrawText("powered by", GetScreenWidth() - 162, GetScreenHeight() - 190, 20, DARKGRAY);
DrawTexture(texLogo, GetScreenWidth() - 128 - 34, GetScreenHeight() - 128 - 36, WHITE);

if ((framesCounter > 80) && ((framesCounter/40)%2)) DrawTextEx(font, "mouse click to return", (Vector2){ 300, 464 }, font.baseSize, 0, SKYBLUE);
}


void UnloadEndingScreen(void)
{

UnloadTexture(texBackground);
UnloadTexture(texWin);
UnloadTexture(texLose);
UnloadTexture(texLogo);
}


int FinishEndingScreen(void)
{
return finishScreen;
}