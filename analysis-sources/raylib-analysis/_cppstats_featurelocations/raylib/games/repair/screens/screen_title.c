
























#include "raylib.h"
#include "screens.h"






static int framesCounter = 0;
static int finishScreen = 0;

static Texture2D texTitle = { 0 };
static Texture2D texLogo = { 0 };

static int titlePositionY = 0;
static int titleCounter = 0;






void InitTitleScreen(void)
{
framesCounter = 0;
finishScreen = 0;

texTitle = LoadTexture("resources/title.png");
texLogo = LoadTexture("resources/raylib_logo.png");

player = GenerateCharacter();

titlePositionY = -200;
}


void UpdateTitleScreen(void)
{
framesCounter++;

if (framesCounter > 5)
{
int partToChange = GetRandomValue(0, 4);

if (partToChange == 0)
{
player.head = GetRandomValue(0, texHead.width/BASE_HEAD_WIDTH - 1);
player.colHead = headColors[GetRandomValue(0, 5)];
}
else if (partToChange == 1) player.eyes = GetRandomValue(0, texEyes.width/BASE_EYES_WIDTH - 1);
else if (partToChange == 2) player.nose = GetRandomValue(0, texNose.width/BASE_NOSE_WIDTH - 1);
else if (partToChange == 3) player.mouth = GetRandomValue(0, texMouth.width/BASE_MOUTH_WIDTH - 1);
else if (partToChange == 4)
{
player.hair = GetRandomValue(0, texHair.width/BASE_HAIR_WIDTH - 1);
player.colHair = hairColors[GetRandomValue(0, 9)];
}

framesCounter = 0;
}

titlePositionY += 3;
if (titlePositionY > 40) titlePositionY = 40;

titleCounter++;

if (IsKeyPressed(KEY_ENTER)) finishScreen = 1;
}


void DrawTitleScreen(void)
{
DrawTexture(background, 0, 0, GetColor(0xf6aa60ff));


DrawCharacter(player, (Vector2){ GetScreenWidth()/2 - 125, 80 });






DrawTexture(texTitle, GetScreenWidth()/2 - texTitle.width/2, titlePositionY, WHITE);

if (titleCounter > 180)
{
if (GuiButton((Rectangle){ GetScreenWidth()/2 - 440/2, 580, 440, 80 }, "START DATE!", -1)) finishScreen = 1; 
}

DrawText("powered by", 20, GetScreenHeight() - texLogo.height - 35, 10, BLACK);
DrawTexture(texLogo, 20, GetScreenHeight() - texLogo.height - 20, WHITE);
}


void UnloadTitleScreen(void)
{
UnloadTexture(texTitle);
UnloadTexture(texLogo);
}


int FinishTitleScreen(void)
{
return finishScreen;
}