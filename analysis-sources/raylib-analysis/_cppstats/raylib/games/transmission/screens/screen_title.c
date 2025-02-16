#include "raylib.h"
#include "screens.h"
#include <string.h>
static int framesCounter;
static int finishScreen;
static Texture2D texBackground;
static Font fontTitle;
static Sound fxTyping;
static float titleSize;
static Vector2 transmissionPosition;
static Vector2 missionPositon;
static const char textTitle[20] = "transmissionmission";
static Color titleColor;
static int speedText;
static int transmissionLenght;
static int missionLenght;
static int transmissionMaxLenght;
static int missionMaxLenght;
static bool writeTransmission;
static bool writeMission;
static bool writeEnd;
static void MissionScreen();
void InitTitleScreen(void)
{
framesCounter = 0;
finishScreen = 0;
texBackground = LoadTexture("resources/textures/title_background.png");
fxTyping = LoadSound("resources/audio/fx_typing.ogg");
fontTitle = LoadFontEx("resources/fonts/mom_typewritter.ttf", 96, 0, 0);
titleSize = 44;
transmissionPosition = (Vector2){519, 221};
missionPositon = (Vector2){580, 261};
titleColor = BLACK;
speedText = 15;
missionLenght = 0;
transmissionLenght = 0;
missionMaxLenght = 7;
transmissionMaxLenght = 12;
writeTransmission = true;
writeMission = false;
writeEnd = false;
currentMission = 0;
}
void UpdateTitleScreen(void)
{
if (!writeEnd)
{
framesCounter ++;
if (framesCounter%speedText == 0)
{
framesCounter = 0;
if (writeTransmission)
{
transmissionLenght++; 
if (transmissionLenght == transmissionMaxLenght)
{
writeTransmission = false;
writeMission = true;
}
}
else if (writeMission)
{
missionLenght++;
if (missionLenght == missionMaxLenght)
{
writeMission = false;
writeEnd = true;
}
}
PlaySound(fxTyping);
}
}
if(IsButtonPressed())
{
MissionScreen();
} 
else if (IsKeyPressed(KEY_ENTER)) MissionScreen();
}
void DrawTitleScreen(void)
{
DrawTexture(texBackground, 0,0, WHITE);
DrawTextEx(fontTitle, TextSubtext(textTitle, 0, transmissionLenght), transmissionPosition, titleSize, 0, titleColor);
DrawTextEx(fontTitle, TextSubtext(textTitle, 12, missionLenght), missionPositon, titleSize, 0, titleColor); 
DrawButton("start");
}
void UnloadTitleScreen(void)
{
UnloadTexture(texBackground);
UnloadSound(fxTyping);
UnloadFont(fontTitle);
}
int FinishTitleScreen(void)
{
return finishScreen;
}
static void MissionScreen()
{
transmissionLenght = transmissionMaxLenght;
missionLenght = missionMaxLenght;
writeEnd = true;
finishScreen = true; 
}