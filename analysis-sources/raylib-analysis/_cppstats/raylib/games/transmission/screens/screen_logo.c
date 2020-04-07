#include "raylib.h"
#include "screens.h"
#define LOGO_RECS_SIDE 16
static int framesCounter = 0;
static int finishScreen = 0;
static int logoPositionX = 0;
static int logoPositionY = 0;
static int lettersCount = 0;
static int topSideRecWidth = 0;
static int leftSideRecHeight = 0;
static int bottomSideRecWidth = 0;
static int rightSideRecHeight = 0;
static char raylib[8] = { 0 }; 
static int state = 0; 
static float alpha = 1.0f; 
static Texture2D texLogoCW = { 0 }; 
void InitLogoScreen(void)
{
finishScreen = 0;
framesCounter = 0;
lettersCount = 0;
logoPositionX = GetScreenWidth()/2 - 128;
logoPositionY = GetScreenHeight()/2 - 128;
topSideRecWidth = LOGO_RECS_SIDE;
leftSideRecHeight = LOGO_RECS_SIDE;
bottomSideRecWidth = LOGO_RECS_SIDE;
rightSideRecHeight = LOGO_RECS_SIDE;
for (int i = 0; i < 8; i++) raylib[i] = '\0';
state = 0;
alpha = 1.0f;
texLogoCW = LoadTexture("resources/textures/cw_logo.png");
}
void UpdateLogoScreen(void)
{
if (state == 0) 
{
framesCounter++;
if (framesCounter == 80)
{
state = 1;
framesCounter = 0; 
}
}
else if (state == 1) 
{
topSideRecWidth += 8;
leftSideRecHeight += 8;
if (topSideRecWidth == 256) state = 2;
}
else if (state == 2) 
{
bottomSideRecWidth += 8;
rightSideRecHeight += 8;
if (bottomSideRecWidth == 256) state = 3;
}
else if (state == 3) 
{
framesCounter++;
if (framesCounter/10) 
{
lettersCount++;
framesCounter = 0;
}
switch (lettersCount)
{
case 1: raylib[0] = 'r'; break;
case 2: raylib[1] = 'a'; break;
case 3: raylib[2] = 'y'; break;
case 4: raylib[3] = 'l'; break;
case 5: raylib[4] = 'i'; break;
case 6: raylib[5] = 'b'; break;
default: break;
}
if (lettersCount >= 10)
{
state = 4;
framesCounter = 0;
}
}
else if (state == 4)
{
framesCounter++;
if (framesCounter > 100)
{
alpha -= 0.02f;
if (alpha <= 0.0f)
{
alpha = 0.0f;
framesCounter = 0;
state = 5;
}
}
}
else if (state == 5)
{
alpha += 0.02f;
if (alpha >= 1.0f) alpha = 1.0f;
framesCounter++;
if (framesCounter > 200)
{
framesCounter = 0;
state = 6;
}
}
else if (state == 6)
{
alpha -= 0.02f;
if (alpha >= 1.0f) alpha = 1.0f;
framesCounter++;
if (framesCounter > 100)
{
framesCounter = 0;
finishScreen = 1;
}
}
}
void DrawLogoScreen(void)
{
if (state == 0)
{
if ((framesCounter/10)%2) DrawRectangle(logoPositionX, logoPositionY, 16, 16, BLACK);
}
else if (state == 1)
{
DrawRectangle(logoPositionX, logoPositionY, topSideRecWidth, 16, BLACK);
DrawRectangle(logoPositionX, logoPositionY, 16, leftSideRecHeight, BLACK);
}
else if (state == 2)
{
DrawRectangle(logoPositionX, logoPositionY, topSideRecWidth, 16, BLACK);
DrawRectangle(logoPositionX, logoPositionY, 16, leftSideRecHeight, BLACK);
DrawRectangle(logoPositionX + 240, logoPositionY, 16, rightSideRecHeight, BLACK);
DrawRectangle(logoPositionX, logoPositionY + 240, bottomSideRecWidth, 16, BLACK);
}
else if (state == 3)
{
DrawRectangle(logoPositionX, logoPositionY, topSideRecWidth, 16, Fade(BLACK, alpha));
DrawRectangle(logoPositionX, logoPositionY + 16, 16, leftSideRecHeight - 32, Fade(BLACK, alpha));
DrawRectangle(logoPositionX + 240, logoPositionY + 16, 16, rightSideRecHeight - 32, Fade(BLACK, alpha));
DrawRectangle(logoPositionX, logoPositionY + 240, bottomSideRecWidth, 16, Fade(BLACK, alpha));
DrawRectangle(GetScreenWidth()/2 - 112, GetScreenHeight()/2 - 112, 224, 224, Fade(RAYWHITE, alpha));
DrawText(raylib, GetScreenWidth()/2 - 44, GetScreenHeight()/2 + 48, 50, Fade(BLACK, alpha));
}
else if (state == 4)
{
DrawRectangle(logoPositionX, logoPositionY, topSideRecWidth, 16, Fade(BLACK, alpha));
DrawRectangle(logoPositionX, logoPositionY + 16, 16, leftSideRecHeight - 32, Fade(BLACK, alpha));
DrawRectangle(logoPositionX + 240, logoPositionY + 16, 16, rightSideRecHeight - 32, Fade(BLACK, alpha));
DrawRectangle(logoPositionX, logoPositionY + 240, bottomSideRecWidth, 16, Fade(BLACK, alpha));
DrawRectangle(GetScreenWidth()/2 - 112, GetScreenHeight()/2 - 112, 224, 224, Fade(RAYWHITE, alpha));
DrawText(raylib, GetScreenWidth()/2 - 44, GetScreenHeight()/2 + 48, 50, Fade(BLACK, alpha));
if (framesCounter > 20) DrawText("powered by", logoPositionX, logoPositionY - 27, 20, Fade(DARKGRAY, alpha));
}
else if ((state == 5) || (state == 6)) DrawTexture(texLogoCW, GetScreenWidth()/2 - texLogoCW.width/2, GetScreenHeight()/2 - texLogoCW.height/2, Fade(WHITE, alpha));
}
void UnloadLogoScreen(void)
{
UnloadTexture(texLogoCW);
}
int FinishLogoScreen(void)
{
return finishScreen;
}
