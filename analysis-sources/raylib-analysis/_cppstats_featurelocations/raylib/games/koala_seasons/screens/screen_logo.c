
























#include "raylib.h"
#include "screens.h"

#include <string.h>






static int framesCounter;
static int finishScreen;

const char msgLogoA[64] = "A simple and easy-to-use library";
const char msgLogoB[64] = "to enjoy videogames programming";

int logoPositionX;
int logoPositionY;

int raylibLettersCount = 0;

int topSideRecWidth = 16;
int leftSideRecHeight = 16;

int bottomSideRecWidth = 16;
int rightSideRecHeight = 16;

char raylib[8] = " \0"; 

int logoScreenState = 0; 
bool msgLogoADone = false;
bool msgLogoBDone = false;

int lettersCounter = 0;
char msgBuffer[128] = { ' ' };






void InitLogoScreen(void)
{

framesCounter = 0;
finishScreen = 0;

logoPositionX = GetScreenWidth()/2 - 128;
logoPositionY = GetScreenHeight()/2 - 128;
}


void UpdateLogoScreen(void)
{

framesCounter++; 


if (logoScreenState == 0) 
{
framesCounter++;

if (framesCounter == 120)
{
logoScreenState = 1;
framesCounter = 0; 
}
}
else if (logoScreenState == 1) 
{
topSideRecWidth += 4;
leftSideRecHeight += 4;

if (topSideRecWidth == 256) logoScreenState = 2;
}
else if (logoScreenState == 2) 
{
bottomSideRecWidth += 4;
rightSideRecHeight += 4;

if (bottomSideRecWidth == 256)
{
lettersCounter = 0;
for (int i = 0; i < (int)strlen(msgBuffer); i++) msgBuffer[i] = ' ';

logoScreenState = 3;
}
}
else if (logoScreenState == 3) 
{
framesCounter++;


if ((framesCounter%12) == 0) raylibLettersCount++;

switch (raylibLettersCount)
{
case 1: raylib[0] = 'r'; break;
case 2: raylib[1] = 'a'; break;
case 3: raylib[2] = 'y'; break;
case 4: raylib[3] = 'l'; break;
case 5: raylib[4] = 'i'; break;
case 6: raylib[5] = 'b'; break;
default: break;
}

if (raylibLettersCount >= 10)
{

if ((framesCounter%2) == 0) lettersCounter++;

if (!msgLogoADone)
{
if (lettersCounter <= (int)strlen(msgLogoA)) strncpy(msgBuffer, msgLogoA, lettersCounter);
else
{
for (int i = 0; i < (int)strlen(msgBuffer); i++) msgBuffer[i] = ' ';

lettersCounter = 0;
msgLogoADone = true;
}
}
else if (!msgLogoBDone)
{
if (lettersCounter <= (int)strlen(msgLogoB)) strncpy(msgBuffer, msgLogoB, lettersCounter);
else
{
msgLogoBDone = true;
framesCounter = 0;

}
}
}
}


if (msgLogoBDone)
{
framesCounter++;

if (framesCounter > 150) finishScreen = true;
}
}


void DrawLogoScreen(void)
{

if (logoScreenState == 0)
{
if ((framesCounter/15)%2) DrawRectangle(logoPositionX, logoPositionY - 60, 16, 16, BLACK);
}
else if (logoScreenState == 1)
{
DrawRectangle(logoPositionX, logoPositionY - 60, topSideRecWidth, 16, BLACK);
DrawRectangle(logoPositionX, logoPositionY - 60, 16, leftSideRecHeight, BLACK);
}
else if (logoScreenState == 2)
{
DrawRectangle(logoPositionX, logoPositionY - 60, topSideRecWidth, 16, BLACK);
DrawRectangle(logoPositionX, logoPositionY - 60, 16, leftSideRecHeight, BLACK);

DrawRectangle(logoPositionX + 240, logoPositionY - 60, 16, rightSideRecHeight, BLACK);
DrawRectangle(logoPositionX, logoPositionY + 240 - 60, bottomSideRecWidth, 16, BLACK);
}
else if (logoScreenState == 3)
{
DrawRectangle(logoPositionX, logoPositionY - 60, topSideRecWidth, 16, BLACK);
DrawRectangle(logoPositionX, logoPositionY + 16 - 60, 16, leftSideRecHeight - 32, BLACK);

DrawRectangle(logoPositionX + 240, logoPositionY + 16 - 60, 16, rightSideRecHeight - 32, BLACK);
DrawRectangle(logoPositionX, logoPositionY + 240 - 60, bottomSideRecWidth, 16, BLACK);

DrawRectangle(GetScreenWidth()/2 - 112, GetScreenHeight()/2 - 112 - 60, 224, 224, RAYWHITE);

DrawText(raylib, GetScreenWidth()/2 - 44, GetScreenHeight()/2 + 48 - 60, 50, BLACK);

if (!msgLogoADone) DrawText(msgBuffer, GetScreenWidth()/2 - MeasureText(msgLogoA, 30)/2, logoPositionY + 230, 30, GRAY);
else
{
DrawText(msgLogoA, GetScreenWidth()/2 - MeasureText(msgLogoA, 30)/2, logoPositionY + 230, 30, GRAY);

if (!msgLogoBDone) DrawText(msgBuffer, GetScreenWidth()/2 - MeasureText(msgLogoB, 30)/2, logoPositionY + 280, 30, GRAY);
else
{
DrawText(msgLogoB, GetScreenWidth()/2 - MeasureText(msgLogoA, 30)/2, logoPositionY + 280, 30, GRAY);
}
}
}
}


void UnloadLogoScreen(void)
{

}


int FinishLogoScreen(void)
{
return finishScreen;
}