
























#include "raylib.h"
#include "screens.h"
#include "../player.h"
#include "../monster.h"

#include <string.h>






static int framesCounter;
static int finishScreen;

static Texture2D background;


static Door doorCenter;
static Door doorLeft;


static Monster candle;
static Monster picture;
static Monster phone;

static bool monsterHover = false;
static int monsterCheck = -1; 

static const char message[256] = "WHEN WIND BLOWS, IT KNOWS THE DIRECTION\nLET IT GUIDE YOU!";
static int msgPosX = 100;

static int msgState = 0; 
static int lettersCounter = 0;
static char msgBuffer[256] = { '\0' };
static int msgCounter = 0;

static bool searching = false;






void InitLivingroomScreen(void)
{
ResetPlayer();


monsterHover = false;
monsterCheck = -1;
msgState = 0;
msgCounter = 0;
lettersCounter = 0;
for (int i = 0; i < 256; i++) msgBuffer[i] = '\0';

framesCounter = 0;
finishScreen = 0;

background = LoadTexture("resources/textures/background_livingroom.png");


doorLeft.position = (Vector2) { -45, 140};
doorLeft.facing = 0;
doorLeft.locked = true;
doorLeft.frameRec =(Rectangle) {((doors.width/3)*doorLeft.facing), doors.height/2, doors.width/3, doors.height/2};
doorLeft.bound = (Rectangle) { doorLeft.position.x, doorLeft.position.y, doors.width/3, doors.height/2};
doorLeft.selected = false;

doorCenter.position = (Vector2) { 830, 108 };
doorCenter.facing = 1;
doorCenter.locked = true;
doorCenter.frameRec =(Rectangle) {((doors.width/3)*doorCenter.facing), doors.height/2, doors.width/3, doors.height/2};
doorCenter.bound = (Rectangle) { doorCenter.position.x, doorCenter.position.y, doors.width/3, doors.height/2};
doorCenter.selected = false;


candle.position = (Vector2){ 154, 256 };
candle.texture = LoadTexture("resources/textures/monster_candle.png");
candle.currentFrame = 0;
candle.framesCounter = 0;
candle.numFrames = 4;
candle.bounds = (Rectangle){ candle.position.x + 90, candle.position.y + 30, 185, 340 };
candle.frameRec = (Rectangle) { 0, 0, candle.texture.width/candle.numFrames, candle.texture.height };
candle.selected = false;
candle.active = false;
candle.spooky = false;


picture.position = (Vector2){ 504, 164 };
picture.texture = LoadTexture("resources/textures/monster_picture.png");
picture.currentFrame = 0;
picture.framesCounter = 0;
picture.numFrames = 4;
picture.bounds = (Rectangle){ picture.position.x + 44, picture.position.y, 174, 264 };
picture.frameRec = (Rectangle) { 0, 0, picture.texture.width/picture.numFrames, picture.texture.height };
picture.selected = false;
picture.active = false;
picture.spooky = true;


phone.position = (Vector2){ 1054, 404 };
phone.texture = LoadTexture("resources/textures/monster_phone.png");
phone.currentFrame = 0;
phone.framesCounter = 0;
phone.numFrames = 4;
phone.bounds = (Rectangle){ phone.position.x + 64, phone.position.y +120, 100, 160 };
phone.frameRec = (Rectangle) { 0, 0, phone.texture.width/phone.numFrames, phone.texture.height };
phone.selected = false;
phone.active = false;
phone.spooky = true;
}


void UpdateLivingroomScreen(void)
{
if (player.key)
{ 

if ((CheckCollisionPointRec(GetMousePosition(), doorLeft.bound)) || 
(CheckCollisionRecs(player.bounds, doorLeft.bound))) doorLeft.selected = true; 
else doorLeft.selected = false;

if ((doorLeft.selected) && (CheckCollisionRecs(player.bounds, doorLeft.bound)))
{
if (((IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) && CheckCollisionPointRec(GetMousePosition(), doorLeft.bound)) || (IsKeyPressed(KEY_SPACE)))
{
if (doorLeft.locked)
{
doorLeft.frameRec.y = 0;
doorLeft.locked = false;
PlaySound(sndDoor);
}
else finishScreen = 1;
}
}


if ((CheckCollisionPointRec(GetMousePosition(), doorCenter.bound)) || 
(CheckCollisionRecs(player.bounds, doorCenter.bound))) doorCenter.selected = true; 
else doorCenter.selected = false;

if ((doorCenter.selected) && (CheckCollisionRecs(player.bounds, doorCenter.bound)))
{
if (((IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) && CheckCollisionPointRec(GetMousePosition(), doorCenter.bound)) || (IsKeyPressed(KEY_SPACE)))
{
if (doorCenter.locked)
{
doorCenter.frameRec.y = 0;
doorCenter.locked = false;
PlaySound(sndDoor);
}
else finishScreen = 2;
}
}
}

if (msgState > 2)
{
UpdatePlayer();


UpdateMonster(&candle);
UpdateMonster(&picture);
UpdateMonster(&phone);
}


if (((CheckCollisionRecs(player.bounds, candle.bounds)) && !candle.active) ||
((CheckCollisionRecs(player.bounds, picture.bounds)) && !picture.active) ||
((CheckCollisionRecs(player.bounds, phone.bounds)) && !phone.active)) monsterHover = true;
else monsterHover = false;


if ((CheckCollisionRecs(player.bounds, candle.bounds)) && !candle.active)
{
candle.selected = true;

if ((IsKeyPressed(KEY_SPACE)) || 
((IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) && (CheckCollisionPointRec(GetMousePosition(), candle.bounds))))
{
SearchKeyPlayer();
searching = true;
framesCounter = 0;

monsterCheck = 1;
}
}
else candle.selected = false;


if ((CheckCollisionRecs(player.bounds, picture.bounds)) && !picture.active)
{
picture.selected = true;

if ((IsKeyPressed(KEY_SPACE)) || 
((IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) && (CheckCollisionPointRec(GetMousePosition(), picture.bounds))))
{
SearchKeyPlayer();
searching = true;
framesCounter = 0;

monsterCheck = 2;
}
}
else picture.selected = false;


if ((CheckCollisionRecs(player.bounds, phone.bounds)) && !phone.active)
{
phone.selected = true;

if ((IsKeyPressed(KEY_SPACE)) || 
((IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) && (CheckCollisionPointRec(GetMousePosition(), phone.bounds))))
{
SearchKeyPlayer();
searching = true;
framesCounter = 0;

monsterCheck = 3;
}
}
else phone.selected = false;

if (searching)
{
framesCounter++;

if (framesCounter > 180)
{
if (monsterCheck == 1)
{
if (candle.spooky)
{
ScarePlayer();
PlaySound(sndScream);
}
else FindKeyPlayer();

candle.active = true;
candle.selected = false;
}
else if (monsterCheck == 2)
{
if (picture.spooky)
{
ScarePlayer();
PlaySound(sndScream);
}
else FindKeyPlayer();

picture.active = true;
picture.selected = false;
}
else if (monsterCheck == 3)
{
if (phone.spooky)
{
ScarePlayer();
PlaySound(sndScream);
}
else FindKeyPlayer();

phone.active = true;
phone.selected = false;
}

searching = false;
framesCounter = 0;
}
}


framesCounter++;

if ((framesCounter%2) == 0) lettersCounter++;

if (msgState == 0)
{
if (lettersCounter <= (int)strlen(message)) strncpy(msgBuffer, message, lettersCounter);
else
{
for (int i = 0; i < (int)strlen(msgBuffer); i++) msgBuffer[i] = '\0';

lettersCounter = 0;
msgState = 1;
}

if (IsKeyPressed(KEY_ENTER)) msgState = 1;
}
else if (msgState == 1)
{
msgCounter++;

if ((IsKeyPressed(KEY_ENTER)) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)))
{
msgState = 2;
msgCounter = 0;
}
}
else if (msgState == 2)
{
msgCounter++;

if (msgCounter > 180) msgState = 3;
}
else msgCounter++;
}


void DrawLivingroomScreen(void)
{
DrawTexture(background, 0, 0, WHITE);


DrawMonster(picture, 0);
DrawMonster(candle, 0);
DrawMonster(phone, 0);


if (doorCenter.selected) DrawTextureRec(doors, doorCenter.frameRec, doorCenter.position, GREEN);
else DrawTextureRec(doors, doorCenter.frameRec, doorCenter.position, WHITE);
if (doorLeft.selected) DrawTextureRec(doors, doorLeft.frameRec, doorLeft.position, GREEN);
else DrawTextureRec(doors, doorLeft.frameRec, doorLeft.position, WHITE);


if (msgState < 2) DrawRectangle(0, 40, GetScreenWidth(), 200, Fade(LIGHTGRAY, 0.5f));
else if (msgState == 2) DrawRectangle(0, 80, GetScreenWidth(), 100, Fade(LIGHTGRAY, 0.5f));

if (msgState == 0) 
{
DrawTextEx(font, msgBuffer, (Vector2){ msgPosX, 80 }, font.baseSize, 2, WHITE);
}
else if (msgState == 1)
{
DrawTextEx(font, message, (Vector2){ msgPosX, 80 }, font.baseSize, 2, WHITE);

if ((msgCounter/30)%2) DrawText("PRESS ENTER or CLICK", GetScreenWidth() - 280, 200, 20, BLACK);
}
else if (msgState == 2)
{
if ((msgCounter/30)%2)
{
DrawTextEx(font, "CHOOSE WISELY!", (Vector2){ 300, 95 }, font.baseSize*2, 2, WHITE);

DrawRectangleRec(candle.bounds, Fade(RED, 0.6f));
DrawRectangleRec(phone.bounds, Fade(RED, 0.6f));
DrawRectangleRec(picture.bounds, Fade(RED, 0.6f));
}
}
else
{
if ((monsterHover) && ((msgCounter/30)%2))
{
DrawRectangle(0, 0, GetScreenWidth(), 50, Fade(LIGHTGRAY, 0.5f));
DrawText("PRESS SPACE or CLICK to INTERACT", 420, 15, 20, BLACK);
}
}

DrawPlayer(); 
}


void UnloadLivingroomScreen(void)
{

UnloadTexture(background);

UnloadMonster(candle);
UnloadMonster(picture);
UnloadMonster(phone);
}


int FinishLivingroomScreen(void)
{
return finishScreen;
}
