
























#include "raylib.h"
#include "screens.h"






static int framesCounter = 0;
static int finishScreen;

static Texture2D logo;
static float logoAlpha = 0;

static int state = 0;






void InitLogoScreen(void)
{

finishScreen = 0;

logo = LoadTexture("resources/textures/skully_logo.png");
}


void UpdateLogoScreen(void)
{

if (state == 0)
{
logoAlpha += 0.04f;

if (logoAlpha >= 1.0f) state = 1;
}
else if (state == 1)
{
framesCounter++;

if (framesCounter > 180) state = 2;
}
else if (state == 2)
{
logoAlpha -= 0.04f;

if (logoAlpha <= 0.0f) 
{
framesCounter = 0;
state = 3;
}
}
else if (state == 3)
{
finishScreen = 1;
}
}


void DrawLogoScreen(void)
{
DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), RAYWHITE);

DrawTexture(logo, GetScreenWidth()/2 - logo.width/2, 130, Fade(WHITE, logoAlpha));

DrawText("GRAY TEAM", 340, 450, 100, Fade(DARKGRAY, logoAlpha));
}


void UnloadLogoScreen(void)
{

UnloadTexture(logo);
}


int FinishLogoScreen(void)
{
return finishScreen;
}