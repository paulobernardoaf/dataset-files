
























#include "raylib.h"
#include "screens.h"
#include "../player.h"
#include "../monster.h"

#include <string.h>






static int framesCounter;
static int finishScreen;

static Texture2D background;


static Door doorLeft;
static Door doorRight;


static Monster closet;
static Monster chair;
static Monster window;

static bool monsterHover = false;
static int monsterCheck = -1; 

static const char message[256] = "QUITE BORING AROUND...\nANY BETTER ENTERTAINING?";
static int msgPosX = 100;

static int msgState = 0; 
static int lettersCounter = 0;
static char msgBuffer[256] = { '\0' };
static int msgCounter = 0;

static bool searching = false;






void InitKitchenScreen(void)
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

background = LoadTexture("resources/textures/background_kitchen.png");


doorLeft.position = (Vector2) { -45, 136 };
doorLeft.facing = 0;
doorLeft.locked = true;
doorLeft.frameRec =(Rectangle) {((doors.width/3)*doorLeft.facing), doors.height/2, doors.width/3, doors.height/2};
doorLeft.bound = (Rectangle) { doorLeft.position.x, doorLeft.position.y, doors.width/3, doors.height/2};
doorLeft.selected = false;

doorRight.position = (Vector2) { 1090, 148 };
doorRight.facing = 2;
doorRight.locked = true;
doorRight.frameRec =(Rectangle) {((doors.width/3)*doorRight.facing), doors.height/2, doors.width/3, doors.height/2};
doorRight.bound = (Rectangle) { doorRight.position.x, doorRight.position.y, doors.width/3, doors.height/2};
doorRight.selected = false;


closet.position = (Vector2){ 280, 260 };
closet.texture = LoadTexture("resources/textures/monster_closet.png");
closet.currentFrame = 0;
closet.framesCounter = 0;
closet.numFrames = 4;
closet.bounds = (Rectangle){ closet.position.x + 100, closet.position.y+25, 272,348 };
closet.frameRec = (Rectangle) { 0, 0, closet.texture.width/closet.numFrames, closet.texture.height };
closet.selected = false;
closet.active = false;
closet.spooky = true;


chair.position = (Vector2){ 230, 410 };
chair.texture = LoadTexture("resources/textures/monster_chair_left.png");
chair.currentFrame = 0;
chair.framesCounter = 0;
chair.numFrames = 4;
chair.bounds = (Rectangle){ chair.position.x + 30, chair.position.y + 60, 100, 160 };
chair.frameRec = (Rectangle) { 0, 0, chair.texture.width/chair.numFrames, chair.texture.height };
chair.selected = false;
chair.active = false;
chair.spooky = true;


window.position = (Vector2){ 715, 88 };
window.texture = LoadTexture("resources/textures/monster_window.png");
window.currentFrame = 0;
window.framesCounter = 0;
window.numFrames = 4;
window.bounds = (Rectangle){ window.position.x + 100, window.position.y + 10, 200, 370 };
window.frameRec = (Rectangle) { 0, 0, window.texture.width/window.numFrames, window.texture.height };
window.selected = false;
window.active = false;
window.spooky = false;
}


void UpdateKitchenScreen(void)
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


if ((CheckCollisionPointRec(GetMousePosition(), doorRight.bound)) || 
(CheckCollisionRecs(player.bounds, doorRight.bound))) doorRight.selected = true; 
else doorRight.selected = false;

if ((doorRight.selected) && (CheckCollisionRecs(player.bounds, doorRight.bound)))
{
if (((IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) && CheckCollisionPointRec(GetMousePosition(), doorRight.bound)) || (IsKeyPressed(KEY_SPACE)))
{
if (doorRight.locked)
{
doorRight.frameRec.y = 0;
doorRight.locked = false;
PlaySound(sndDoor);
}
else finishScreen = 2;
}
}
}

if (msgState > 2)
{
UpdatePlayer();


UpdateMonster(&closet);
UpdateMonster(&chair);
UpdateMonster(&window);
}


if (((CheckCollisionRecs(player.bounds, closet.bounds)) && !closet.active) ||
((CheckCollisionRecs(player.bounds, window.bounds)) && !window.active)) monsterHover = true;
else monsterHover = false;


if ((CheckCollisionRecs(player.bounds, closet.bounds)) && !closet.active)
{
closet.selected = true;

if ((IsKeyPressed(KEY_SPACE)) || 
((IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) && (CheckCollisionPointRec(GetMousePosition(), closet.bounds))))
{
SearchKeyPlayer();
searching = true;
framesCounter = 0;

monsterCheck = 1;
}
}
else closet.selected = false;


if ((CheckCollisionRecs(player.bounds, chair.bounds)) && !chair.active)
{
chair.selected = true;

if ((IsKeyPressed(KEY_SPACE)) || 
((IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) && (CheckCollisionPointRec(GetMousePosition(), chair.bounds))))
{
SearchKeyPlayer();
searching = true;
framesCounter = 0;

monsterCheck = 2;
}
}
else chair.selected = false;


if ((CheckCollisionRecs(player.bounds, window.bounds)) && !window.active)
{
window.selected = true;

if ((IsKeyPressed(KEY_SPACE)) || 
((IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) && (CheckCollisionPointRec(GetMousePosition(), window.bounds))))
{
SearchKeyPlayer();
searching = true;
framesCounter = 0;

monsterCheck = 3;
}
}
else window.selected = false;

if (searching)
{
framesCounter++;

if (framesCounter > 180)
{
if (monsterCheck == 1)
{
if (closet.spooky)
{
ScarePlayer();
PlaySound(sndScream);
}
else FindKeyPlayer();

closet.active = true;
closet.selected = false;
}
else if (monsterCheck == 2)
{
if (chair.spooky)
{
ScarePlayer();
PlaySound(sndScream);
}
else FindKeyPlayer();

chair.active = true;
chair.selected = false;
}
else if (monsterCheck == 3)
{
if (window.spooky)
{
ScarePlayer();
PlaySound(sndScream);
}
else FindKeyPlayer();

window.active = true;
window.selected = false;
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


void DrawKitchenScreen(void)
{
DrawTexture(background, 0, 0, WHITE);


DrawMonster(closet, 0);
DrawMonster(chair, 0);
DrawMonster(window, 0);


if (doorRight.selected) DrawTextureRec(doors, doorRight.frameRec, doorRight.position, GREEN);
else DrawTextureRec(doors, doorRight.frameRec, doorRight.position, WHITE);

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

DrawRectangleRec(closet.bounds, Fade(RED, 0.6f));
DrawRectangleRec(window.bounds, Fade(RED, 0.6f));
DrawRectangleRec(chair.bounds, Fade(RED, 0.6f));
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


void UnloadKitchenScreen(void)
{

UnloadTexture(background);

UnloadMonster(closet);
UnloadMonster(chair);
UnloadMonster(window);
}


int FinishKitchenScreen(void)
{
return finishScreen;
}
