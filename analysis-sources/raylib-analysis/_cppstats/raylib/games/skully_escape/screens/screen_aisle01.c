#include "raylib.h"
#include "screens.h"
#include "../player.h"
#include "../monster.h"
#include <string.h>
static int framesCounter;
static int finishScreen;
static Texture2D background;
static Door doorRight;
static Door doorCenter;
static Door doorLeft;
static Monster lamp;
static Monster picture;
static bool monsterHover = false;
static int monsterCheck = -1; 
static const char message[256] = "WHO IS THERE???\nANYBODY IN THE ROOM???";
static int msgPosX = 100;
static int msgState = 0; 
static int lettersCounter = 0;
static char msgBuffer[256] = { '\0' };
static int msgCounter = 0;
static bool searching = false;
static int scroll = 0;
void InitAisle01Screen(void)
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
background = LoadTexture("resources/textures/background_aisle01.png");
scroll = player.position.x - 200;
doorLeft.position = (Vector2) { -30, 135 };
doorLeft.facing = 0;
doorLeft.locked = true;
doorLeft.frameRec =(Rectangle) {((doors.width/3)*doorLeft.facing), doors.height/2, doors.width/3, doors.height/2};
doorLeft.bound = (Rectangle) { doorLeft.position.x, doorLeft.position.y, doors.width/3, doors.height/2};
doorLeft.selected = false;
doorCenter.position = (Vector2) { 1115, 104 };
doorCenter.facing = 1;
doorCenter.locked = true;
doorCenter.frameRec =(Rectangle) {((doors.width/3)*doorCenter.facing), doors.height/2, doors.width/3, doors.height/2};
doorCenter.bound = (Rectangle) { doorCenter.position.x, doorCenter.position.y, doors.width/3, doors.height/2};
doorCenter.selected = false;
doorRight.position = (Vector2) { 1710, 140 };
doorRight.facing = 2;
doorRight.locked = true;
doorRight.frameRec =(Rectangle) {((doors.width/3)*doorRight.facing), doors.height/2, doors.width/3, doors.height/2};
doorRight.bound = (Rectangle) { doorRight.position.x, doorRight.position.y, doors.width/3, doors.height/2};
lamp.position = (Vector2){ 187, 256 };
lamp.texture = LoadTexture("resources/textures/monster_lamp_left.png");
lamp.currentFrame = 0;
lamp.framesCounter = 0;
lamp.numFrames = 4;
lamp.bounds = (Rectangle){ lamp.position.x + 20, lamp.position.y, 90, 380 };
lamp.frameRec = (Rectangle) { 0, 0, lamp.texture.width/lamp.numFrames, lamp.texture.height };
lamp.selected = false;
lamp.active = false;
lamp.spooky = true;
picture.position = (Vector2){ 637, 178 };
picture.texture = LoadTexture("resources/textures/monster_picture.png");
picture.currentFrame = 0;
picture.framesCounter = 0;
picture.numFrames = 4;
picture.bounds = (Rectangle){ picture.position.x + 44, picture.position.y, 174, 256 };
picture.frameRec = (Rectangle) { 0, 0, picture.texture.width/picture.numFrames, picture.texture.height };
picture.selected = false;
picture.active = false;
picture.spooky = false;
}
void UpdateAisle01Screen(void)
{
doorLeft.bound.x = doorLeft.position.x - scroll;
doorCenter.bound.x = doorCenter.position.x - scroll;
doorRight.bound.x = doorRight.position.x - scroll;
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
else finishScreen = 3;
}
}
}
if (msgState > 2)
{
UpdatePlayer();
UpdateMonster(&lamp);
UpdateMonster(&picture);
}
lamp.bounds.x = lamp.position.x + 20 - scroll;
picture.bounds.x = picture.position.x + 44 - scroll;
if (((CheckCollisionRecs(player.bounds, lamp.bounds)) && !lamp.active) ||
((CheckCollisionRecs(player.bounds, picture.bounds)) && !picture.active)) monsterHover = true;
else monsterHover = false;
if ((CheckCollisionRecs(player.bounds, lamp.bounds)) && !lamp.active)
{
lamp.selected = true;
if ((IsKeyPressed(KEY_SPACE)) || 
((IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) && (CheckCollisionPointRec(GetMousePosition(), lamp.bounds))))
{
SearchKeyPlayer();
searching = true;
framesCounter = 0;
monsterCheck = 1;
}
}
else lamp.selected = false;
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
if (searching)
{
framesCounter++;
if (framesCounter > 180)
{
if (monsterCheck == 1)
{
if (lamp.spooky)
{
ScarePlayer();
PlaySound(sndScream);
}
else FindKeyPlayer();
lamp.active = true;
lamp.selected = false;
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
if (player.position.x > 200)
{
scroll = player.position.x - 200;
if (scroll > 620) scroll = 620;
}
}
void DrawAisle01Screen(void)
{
DrawTexture(background, -scroll, 0, WHITE);
DrawMonster(lamp, scroll);
DrawMonster(picture, scroll);
Vector2 doorScrollPos = { doorCenter.position.x - scroll, doorCenter.position.y };
if (doorCenter.selected) DrawTextureRec(doors, doorCenter.frameRec, doorScrollPos, GREEN);
else DrawTextureRec(doors, doorCenter.frameRec, doorScrollPos, WHITE);
doorScrollPos = (Vector2){ doorLeft.position.x - scroll, doorLeft.position.y };
if (doorLeft.selected) DrawTextureRec(doors, doorLeft.frameRec, doorScrollPos, GREEN);
else DrawTextureRec(doors, doorLeft.frameRec, doorScrollPos, WHITE);
doorScrollPos = (Vector2){ doorRight.position.x - scroll, doorRight.position.y };
if (doorRight.selected) DrawTextureRec(doors, doorRight.frameRec, doorScrollPos, GREEN);
else DrawTextureRec(doors, doorRight.frameRec, doorScrollPos, WHITE);
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
DrawRectangleRec(lamp.bounds, Fade(RED, 0.6f));
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
void UnloadAisle01Screen(void)
{
UnloadTexture(background);
UnloadMonster(lamp);
UnloadMonster(picture);
}
int FinishAisle01Screen(void)
{
return finishScreen;
}
