
























#include "raylib.h"
#include "screens.h"

typedef struct {
int hair;
int colHair;
int eyes;
int nose;
int mouth;


} CharLikes;






static int framesCounter = 0;
static int finishScreen = 0;

static Texture2D texQmark = { 0 };
static Texture2D texMatch = { 0 };

static int state = 0;
static int matchValue = 0;

static CharLikes playerLikes = { 0 };
static CharLikes playerBaseLikes = { 0 };






void InitEndingScreen(void)
{
framesCounter = 0;
finishScreen = 0;
state = 0;

CustomizeCharacter(&dating);

texQmark = LoadTexture("resources/qmark.png");
texMatch = LoadTexture("resources/match.png");
}


void UpdateEndingScreen(void)
{
if (state == 0)
{
framesCounter++;

if (framesCounter > 200)
{
state = 1;


if (playerBase.hair == dating.hair) playerBaseLikes.hair = GetRandomValue(70, 100);
else if (playerBase.hair == datingBase.hair) playerBaseLikes.hair = GetRandomValue(0, 30);
else playerBaseLikes.hair = GetRandomValue(0, 100);

if (playerBase.colHair == dating.colHair) playerBaseLikes.colHair = GetRandomValue(70, 100);
else if (playerBase.colHair == datingBase.colHair) playerBaseLikes.colHair = GetRandomValue(0, 30);
else playerBaseLikes.colHair = GetRandomValue(0, 100);

if (playerBase.eyes == dating.eyes) playerBaseLikes.eyes = GetRandomValue(70, 100);
else if (playerBase.eyes == datingBase.eyes) playerBaseLikes.eyes = GetRandomValue(0, 30);
else playerBaseLikes.eyes = GetRandomValue(0, 100);

if (playerBase.nose == dating.nose) playerBaseLikes.nose = GetRandomValue(70, 100);
else if (playerBase.nose == datingBase.nose) playerBaseLikes.nose = GetRandomValue(0, 30);
else playerBaseLikes.nose = GetRandomValue(0, 100);

if (playerBase.mouth == dating.mouth) playerBaseLikes.mouth = GetRandomValue(70, 100);
else if (playerBase.mouth == datingBase.mouth) playerBaseLikes.mouth = GetRandomValue(0, 30);
else playerBaseLikes.mouth = GetRandomValue(0, 100);



if (player.hair == dating.hair) playerLikes.hair = GetRandomValue(70, 100);
else if (player.hair == datingBase.hair) playerLikes.hair = GetRandomValue(0, 30);
else playerLikes.hair = GetRandomValue(0, 100);

if (player.colHair == dating.colHair) playerLikes.colHair = GetRandomValue(70, 100);
else if (player.colHair == datingBase.colHair) playerLikes.colHair = GetRandomValue(0, 30);
else playerLikes.colHair = GetRandomValue(0, 100);

if (player.eyes == dating.eyes) playerLikes.eyes = GetRandomValue(70, 100);
else if (player.eyes == datingBase.eyes) playerLikes.eyes = GetRandomValue(0, 30);
else playerLikes.eyes = GetRandomValue(0, 100);

if (player.nose == dating.nose) playerLikes.nose = GetRandomValue(70, 100);
else if (player.nose == datingBase.nose) playerLikes.nose = GetRandomValue(0, 30);
else playerLikes.nose = GetRandomValue(0, 100);

if (player.mouth == dating.mouth) playerLikes.mouth = GetRandomValue(70, 100);
else if (player.mouth == datingBase.mouth) playerLikes.mouth = GetRandomValue(0, 30);
else playerLikes.mouth = GetRandomValue(0, 100);



matchValue = playerLikes.hair + playerLikes.colHair + playerLikes.eyes + playerLikes.nose + playerLikes.mouth;
}
}
else if (state == 1)
{

}


if (IsKeyPressed(KEY_ENTER))
{
finishScreen = 1;
PlaySound(fxCoin);
}
}


void DrawEndingScreen(void)
{

DrawTexture(background, 0, 0, GetColor(0xf6aa60ff));

DrawCharacter(player, (Vector2){ 180, 40 });

DrawCharacter(dating, (Vector2){ 820, 40 });

if (state == 0)
{
if ((framesCounter/15)%2 == 1) DrawTexture(texQmark, GetScreenWidth()/2 - texQmark.width/2, 180, WHITE);
}
else if (state == 1)
{
DrawTextEx(font, TextFormat("MATCH: %i%%", (int)(((float)matchValue/500.0f)*100.0f)), (Vector2){ 420, 40 }, font.baseSize*2, 1, SKYBLUE);

DrawTextureRec(texMatch, (Rectangle){ 0, (matchValue > 250)? 0 : texMatch.height/2, texMatch.width, texMatch.height/2 }, (Vector2){ GetScreenWidth()/2 - texMatch.width/2, 240 }, WHITE);

int barsPositionX = 80;





DrawTextEx(font, "HAIR:", (Vector2){ barsPositionX, 550 }, font.baseSize/2, 1, WHITE);
DrawRectangle(barsPositionX + 80, 550 + 6, 400, font.baseSize/4, GRAY);
DrawRectangle(barsPositionX + 80, 550 + 6, playerBaseLikes.hair*4, font.baseSize/4, RED);

DrawTextEx(font, "TINT:", (Vector2){ barsPositionX, 580 }, font.baseSize/2, 1, WHITE);
DrawRectangle(barsPositionX + 80, 580 + 6, 400, font.baseSize/4, GRAY);
DrawRectangle(barsPositionX + 80, 580 + 6, playerBaseLikes.colHair*4, font.baseSize/4, RED);

DrawTextEx(font, "EYES:", (Vector2){ barsPositionX, 610 }, font.baseSize/2, 1, WHITE);
DrawRectangle(barsPositionX + 80, 610 + 6, 400, font.baseSize/4, GRAY);
DrawRectangle(barsPositionX + 80, 610 + 6, playerBaseLikes.eyes*4, font.baseSize/4, RED);

DrawTextEx(font, "NOSE:", (Vector2){ barsPositionX, 640 }, font.baseSize/2, 1, WHITE);
DrawRectangle(barsPositionX + 80, 640 + 6, 400, font.baseSize/4, GRAY);
DrawRectangle(barsPositionX + 80, 640 + 6, playerBaseLikes.nose*4, font.baseSize/4, RED);

DrawTextEx(font, "LIPS:", (Vector2){ barsPositionX, 670 }, font.baseSize/2, 1, WHITE);
DrawRectangle(barsPositionX + 80, 670 + 6, 400, font.baseSize/4, GRAY);
DrawRectangle(barsPositionX + 80, 670 + 6, playerBaseLikes.mouth*4, font.baseSize/4, RED);


if (player.hair != playerBase.hair)
{
DrawTextEx(font, "after re-touch:", (Vector2){ barsPositionX + 80 + 400 + 20, 550 }, font.baseSize/2, 1, WHITE);
DrawRectangle(barsPositionX + 80 + 400 + 100 + 90, 550 + 6, 400, font.baseSize/4, GRAY);
DrawRectangle(barsPositionX + 80 + 400 + 100 + 90, 550 + 6, playerLikes.hair*4, font.baseSize/4, RED);
}

if (player.colHair != playerBase.colHair)
{
DrawTextEx(font, "after re-touch:", (Vector2){ barsPositionX + 80 + 400 + 20, 580 }, font.baseSize/2, 1, WHITE);
DrawRectangle(barsPositionX + 80 + 400 + 100 + 90, 580 + 6, 400, font.baseSize/4, GRAY);
DrawRectangle(barsPositionX + 80 + 400 + 100 + 90, 580 + 6, playerLikes.colHair*4, font.baseSize/4, RED);
}

if (player.eyes != playerBase.eyes)
{
DrawTextEx(font, "after re-touch:", (Vector2){ barsPositionX + 80 + 400 + 20, 610 }, font.baseSize/2, 1, WHITE);
DrawRectangle(barsPositionX + 80 + 400 + 100 + 90, 610 + 6, 400, font.baseSize/4, GRAY);
DrawRectangle(barsPositionX + 80 + 400 + 100 + 90, 610 + 6, playerLikes.eyes*4, font.baseSize/4, RED);
}

if (player.nose != playerBase.nose)
{
DrawTextEx(font, "after re-touch:", (Vector2){ barsPositionX + 80 + 400 + 20, 640 }, font.baseSize/2, 1, WHITE);
DrawRectangle(barsPositionX + 80 + 400 + 100 + 90, 640 + 6, 400, font.baseSize/4, GRAY);
DrawRectangle(barsPositionX + 80 + 400 + 100 + 90, 640 + 6, playerLikes.nose*4, font.baseSize/4, RED);
}

if (player.mouth != playerBase.mouth)
{
DrawTextEx(font, "after re-touch:", (Vector2){ barsPositionX + 80 + 400 + 20, 670 }, font.baseSize/2, 1, WHITE);
DrawRectangle(barsPositionX + 80 + 400 + 100 + 90, 670 + 6, 400, font.baseSize/4, GRAY);
DrawRectangle(barsPositionX + 80 + 400 + 100 + 90, 670 + 6, playerLikes.mouth*4, font.baseSize/4, RED);
}


if (GuiButton((Rectangle){ GetScreenWidth() - 280, 60, 260, 80 }, "AGAIN!", -1))
{
finishScreen = 1;
}
}
}


void UnloadEndingScreen(void)
{
UnloadTexture(texQmark);
UnloadTexture(texMatch);
}


int FinishEndingScreen(void)
{
return finishScreen;
}