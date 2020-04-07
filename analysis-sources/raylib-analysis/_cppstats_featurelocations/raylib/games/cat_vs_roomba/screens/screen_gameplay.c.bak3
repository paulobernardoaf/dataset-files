
























#include "raylib.h"
#include "screens.h"

#include "raymath.h"

#include <stdio.h>

#define TILE_REQUIRED_CLEAN_TIME 2 
#define TILE_SCORE_BY_CLEANED_LEVEL 100 
#define TILE_REQUIRED_CLEAN_AREA 28*28 

#define TILE_SIZE 36 
#define MAX_TILES_X 32
#define MAX_TILES_Y 17

#define CAT_TARGET_RADIUS 3 
#define CAT_DIRT_CELL_RADIUS 2 

#define TIME_LIMIT_SECONDS 180 

#define MAX_SCORE_POPUPS 60 





typedef struct {
Vector2 position; 
int level; 
int state; 
int counter; 


bool cleaned; 
} Dirtile;


typedef struct {
Vector2 position;
int value;
float alpha;
bool enabled;
} ScorePopup;


typedef struct {
int id; 
int posX; 
int posY; 
int width; 
int height; 
} FurSet;


typedef struct {
int furId; 
int cellX; 
int cellY; 
int state; 
int counter; 
} Furniture;






static int framesCounter;
static int timeLevelSeconds;
static bool levelFinished;
static int finishScreen;

const Vector2 roomOffset = { 70, 70 };

static Texture2D roomba;
static Texture2D cat;
static Texture2D dirtiles;
static Texture2D furniture;

#if defined(TILE_VIEWER_MODE)
static Texture2D tracemap;
static Texture2D fursetid;
#endif

static Music catch;

static Sound fxCat[2];
static Sound fxRoomba[3];

static Vector2 roombaPosition = { 100, 100 };
static Vector2 roombaSpeed = { 4, 4 };
static int roombaTilePosX = 0, roombaTilePosY = 0;

static Vector2 catPosition = { 0, 0 };
static Vector2 catTargetPosition = { 0, 0 };
static Vector2 catSpeed = { 3, 3 };
static int catTilePosX = 0, catTilePosY = 0;
static bool catShouldMove = false;

static Vector2 mousePosition = { 0, 0 };
static int mouseTileX = -1, mouseTileY = -1;

static Dirtile tiles[MAX_TILES_X*MAX_TILES_Y] = { 0 };

static ScorePopup popup[MAX_SCORE_POPUPS] = { 0 };

static FurSet furset[32] = { -1 };
static Furniture furmap[40] = { -1 };
static int furnitureCount = 0;



static int furcolmap[MAX_TILES_X*MAX_TILES_Y] = {
1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,3,3,1,1,1,0,0,1,1,1,1,1,1,1,1,
1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,3,3,1,1,1,0,0,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
0,0,1,1,1,1,2,2,2,1,1,1,0,0,1,1,1,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,
0,0,1,1,1,1,2,2,2,1,1,1,0,0,1,1,1,2,2,2,1,1,1,1,3,3,1,1,1,1,0,0,
3,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,1,1,1,3,3,1,1,1,1,0,0,
3,3,1,1,2,2,2,2,2,2,2,2,1,1,1,1,2,2,2,2,2,1,1,1,0,0,0,0,0,0,0,0,
1,1,1,1,2,2,2,2,2,2,2,2,1,1,1,1,1,2,2,2,1,1,1,1,0,0,0,0,0,0,0,0,
1,1,1,1,2,2,2,2,2,2,2,2,1,1,1,1,1,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,0,0,0,0,0,0,0,0,3,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,0,0,0,0,0,0,0,0,3,3,1,1,1,1,1,1,1,0,0,1,2,2,2,2,2,2,1,1,
0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,2,2,2,2,2,2,1,1,
0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,1,1,
0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,1,1,
0,0,0,0,0,0,0,0,0,0,2,2,2,3,3,3,3,1,1,0,0,1,2,2,2,2,2,2,2,2,0,0,
0,0,0,0,0,0,0,0,0,0,2,2,2,3,3,3,3,1,1,0,0,1,2,2,2,2,2,2,2,2,0,0 };

static bool showObjective = false;




static float GetTileCleanPercent(void);






void InitGameplayScreen(void)
{

framesCounter = 0;
finishScreen = 0;
timeLevelSeconds = TIME_LIMIT_SECONDS;
levelFinished = false;

roomba = LoadTexture("resources/roomba.png");
cat = LoadTexture("resources/cat.png");
dirtiles = LoadTexture("resources/dirtiles.png");
furniture = LoadTexture("resources/furniture.png");

#if defined(TILE_VIEWER_MODE)
tracemap = LoadTexture("resources/tracemap.png");
fursetid = LoadTexture("resources/fursetid.png");
#endif

int furCount = 0;
FILE *fursetFile = fopen("resources/furset.txt", "rt");

if (fursetFile != NULL)
{
char buffer[512] = { 0 };

while (!feof(fursetFile))
{
fgets(buffer, 512, fursetFile);

switch (buffer[0])
{
case 'f': 
{
sscanf(buffer, "f %i %i %i %i %i", 
&furset[furCount].id, 
&furset[furCount].posX, 
&furset[furCount].posY, 
&furset[furCount].width, 
&furset[furCount].height); 
furCount++;
} break;
case '.': 
default: break;
}
}

fclose(fursetFile);
}


for (int i = 0; i < furCount; i++)
{
furset[i].posX *= TILE_SIZE;
furset[i].posY *= TILE_SIZE;
furset[i].width *= TILE_SIZE;
furset[i].height *= TILE_SIZE;
}

printf("Furniture SET elements read: %i\n", furCount);


FILE *furnitureFile = fopen("resources/furmap.txt", "rt");

if (furnitureFile != NULL)
{
char buffer[512] = { 0 };

while (!feof(furnitureFile))
{
fgets(buffer, 512, furnitureFile);

switch (buffer[0])
{
case 'f': 
{
sscanf(buffer, "f %i %i %i %i %i", 
&furmap[furnitureCount].furId, 
&furmap[furnitureCount].cellX, 
&furmap[furnitureCount].cellY, 
&furmap[furnitureCount].state, 
&furmap[furnitureCount].counter); 
furnitureCount++;
} break;
case '.': 
default: break;
}
}

fclose(furnitureFile);
}

printf("Furniture MAP elements read: %i\n", furnitureCount);


for (int y = 0; y < MAX_TILES_Y; y++)
{
for (int x = 0; x < MAX_TILES_X; x++)
{
tiles[y*MAX_TILES_X + x].position = (Vector2){ roomOffset.x + TILE_SIZE*x, roomOffset.y + TILE_SIZE*y };

if ((furcolmap[y*MAX_TILES_X + x] != 0) &&
(furcolmap[y*MAX_TILES_X + x] != 3))
{


int dirt = GetRandomValue(0, 100);

if (dirt < 50) tiles[y*MAX_TILES_X + x].level = 0; 
else if (dirt < 70) tiles[y*MAX_TILES_X + x].level = 1; 
else if (dirt < 90) tiles[y*MAX_TILES_X + x].level = 2; 
else if (dirt < 100) tiles[y*MAX_TILES_X + x].level = 3; 
}
else tiles[y*MAX_TILES_X + x].level = 0;

tiles[y*MAX_TILES_X + x].state = tiles[y*MAX_TILES_X + x].level;
tiles[y*MAX_TILES_X + x].counter = (tiles[y*MAX_TILES_X + x].level == 0)? 0 : TILE_REQUIRED_CLEAN_TIME;
tiles[y*MAX_TILES_X + x].cleaned = (tiles[y*MAX_TILES_X + x].level == 0)? true : false;
}
}


for (int i = 0; i < MAX_SCORE_POPUPS; i++)
{
popup[i].position = (Vector2){ 0, 0 };
popup[i].value = TILE_SCORE_BY_CLEANED_LEVEL;
popup[i].enabled = false;
popup[i].alpha = 1.0f;
}


catPosition = (Vector2){ 30*TILE_SIZE + roomOffset.x, TILE_SIZE + roomOffset.y };
catTargetPosition = catPosition;

showObjective = true;


fxCat[0] = LoadSound("resources/fxcat01.wav");
fxCat[1] = LoadSound("resources/fxcat02.wav");
fxRoomba[0] = LoadSound("resources/fxrobot01.wav");
fxRoomba[1] = LoadSound("resources/fxrobot02.wav");
fxRoomba[2] = LoadSound("resources/fxrobot03.wav");

catch = LoadMusicStream("resources/catch22.mod");

StopMusicStream(music);
SetMusicVolume(catch, 0.6f);
PlayMusicStream(catch);

result = 0; 
}


void UpdateGameplayScreen(void)
{
UpdateMusicStream(catch);

if (showObjective)
{
if (IsKeyPressed(KEY_ENTER)) 
{
showObjective = false;
PlaySound(fxCoin);
}

return;
}

framesCounter++;

if (framesCounter == 60)
{
timeLevelSeconds--;

if (timeLevelSeconds == 0)
{
levelFinished = true;
finishScreen = 1;
PlaySound(fxCoin);

if (GetTileCleanPercent() >= 80) result = 1;
}

framesCounter = 0;
}

mousePosition = GetMousePosition();
mouseTileX = (int)floorf((mousePosition.x - roomOffset.x)/TILE_SIZE);
mouseTileY = (int)floorf((mousePosition.y - roomOffset.y)/TILE_SIZE);


Vector2 prevPosition = roombaPosition;

if (IsKeyDown(KEY_D)) roombaPosition.x += roombaSpeed.x;
else if (IsKeyDown(KEY_A)) roombaPosition.x -= roombaSpeed.x;
if (IsKeyDown(KEY_W)) roombaPosition.y -= roombaSpeed.y;
else if (IsKeyDown(KEY_S)) roombaPosition.y += roombaSpeed.y;


roombaTilePosX = (int)(floorf(roombaPosition.x - roomOffset.x)/TILE_SIZE);
roombaTilePosY = (int)(floorf(roombaPosition.y - roomOffset.y)/TILE_SIZE);
if ((roombaPosition.x - roomba.width/2 < roomOffset.x) || 
((roombaPosition.x + roomba.width/2) >= (roomOffset.x + MAX_TILES_X*TILE_SIZE)) ||
(roombaPosition.y - roomba.height/2 < roomOffset.y) || 
((roombaPosition.y + roomba.height/2) >= (roomOffset.y + MAX_TILES_Y*TILE_SIZE)) ||
(furcolmap[roombaTilePosY*MAX_TILES_X + roombaTilePosX] == 0) || 
(furcolmap[roombaTilePosY*MAX_TILES_X + roombaTilePosX] == 3)) roombaPosition = prevPosition;









for (int y = 0; y < MAX_TILES_Y; y++)
{
for (int x = 0; x < MAX_TILES_X; x++)
{

if (tiles[y*MAX_TILES_X + x].state > 0)
{

Rectangle cleanRec = GetCollisionRec((Rectangle){ tiles[y*MAX_TILES_X + x].position.x, tiles[y*MAX_TILES_X + x].position.y, 36, 36 },
(Rectangle){ roombaPosition.x - roomba.width/2, roombaPosition.y - roomba.height/2, roomba.width, roomba.height });


if ((cleanRec.width*cleanRec.height) > TILE_REQUIRED_CLEAN_AREA)
{

tiles[y*MAX_TILES_X + x].counter--;

if (tiles[y*MAX_TILES_X + x].counter < 0)
{
tiles[y*MAX_TILES_X + x].state--;

if (tiles[y*MAX_TILES_X + x].state == 0) 
{
tiles[y*MAX_TILES_X + x].counter = 0;
score += tiles[y*MAX_TILES_X + x].level*TILE_SCORE_BY_CLEANED_LEVEL;


for (int i = 0; i < MAX_SCORE_POPUPS; i++)
{
if (!popup[i].enabled)
{
popup[i].position = tiles[y*MAX_TILES_X + x].position;
popup[i].value = TILE_SCORE_BY_CLEANED_LEVEL*tiles[y*MAX_TILES_X + x].level;
popup[i].enabled = true;
popup[i].alpha = 1.0f;
break;
}
}
}
else tiles[y*MAX_TILES_X + x].counter = TILE_REQUIRED_CLEAN_TIME;
}
}
}
}
}


for (int i = 0; i < MAX_SCORE_POPUPS; i++)
{
if (popup[i].enabled)
{
popup[i].position.y -= 2;
popup[i].alpha -= 0.015f;

if (popup[i].alpha < 0.0f) popup[i].enabled = false;
}
}


if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
{

if ((mousePosition.x > roomOffset.x) && (mousePosition.x < (roomOffset.x + MAX_TILES_X*TILE_SIZE)) && 
(mousePosition.y > roomOffset.y) && (mousePosition.y < (roomOffset.y + MAX_TILES_Y*TILE_SIZE)) && 
furcolmap[mouseTileY*MAX_TILES_X + mouseTileX] != 0)
{
catTargetPosition = GetMousePosition();
catShouldMove = true;
}
}

if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) PlaySound(fxCat[GetRandomValue(0,1)]);
if (IsKeyPressed(KEY_SPACE)) PlaySound(fxRoomba[GetRandomValue(0,2)]);


if (catShouldMove)
{
if (CheckCollisionPointCircle(catPosition, catTargetPosition, CAT_TARGET_RADIUS))
{
catShouldMove = false;



catTilePosX = (int)floorf((catPosition.x - cat.width/2 - roomOffset.x)/TILE_SIZE) + 1;
catTilePosY = (int)floorf((catPosition.y - cat.height/2 - 10 - roomOffset.y)/TILE_SIZE) + 1;


if (furcolmap[mouseTileY*MAX_TILES_X + mouseTileX] == 3)
{
for (int y = (catTilePosY - CAT_DIRT_CELL_RADIUS); y < (catTilePosY + CAT_DIRT_CELL_RADIUS + 1); y++)
{
for (int x = (catTilePosX - CAT_DIRT_CELL_RADIUS); x < (catTilePosX + CAT_DIRT_CELL_RADIUS + 1); x++)
{
if (((y >= 0) && (y < MAX_TILES_Y) && (x >= 0) && (x < MAX_TILES_X)) && 
(tiles[y*MAX_TILES_X + x].state == 0) && 
(furcolmap[y*MAX_TILES_X + x] != 0) &&
(furcolmap[y*MAX_TILES_X + x] != 3))
{
int dirt = GetRandomValue(0, 100);

if (dirt < 50) tiles[y*MAX_TILES_X + x].level = 0; 
else if (dirt < 70) tiles[y*MAX_TILES_X + x].level = 1; 
else if (dirt < 90) tiles[y*MAX_TILES_X + x].level = 2; 
else if (dirt < 100) tiles[y*MAX_TILES_X + x].level = 3; 

tiles[y*MAX_TILES_X + x].state = tiles[y*MAX_TILES_X + x].level;
tiles[y*MAX_TILES_X + x].counter = (tiles[y*MAX_TILES_X + x].level == 0)? 0 : TILE_REQUIRED_CLEAN_TIME;
tiles[y*MAX_TILES_X + x].cleaned = (tiles[y*MAX_TILES_X + x].level == 0)? true : false;
}
}
}
}
}
else
{
Vector2 dir = Vector2Subtract(catTargetPosition, catPosition);
Vector2 dirnorm = Vector2Normalize(dir);

catPosition.x += catSpeed.x*dirnorm.x;
catPosition.y += catSpeed.y*dirnorm.y;
}
}

if (levelFinished)
{

}
}


void DrawGameplayScreen(void)
{
DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), GetColor(0x57374cff));


for (int y = 0; y < MAX_TILES_Y; y++)
{
for (int x = 0; x < MAX_TILES_X; x++)
{

DrawTextureRec(dirtiles, (Rectangle){ tiles[y*MAX_TILES_X + x].state*TILE_SIZE, 0, TILE_SIZE, TILE_SIZE }, 
(Vector2){ roomOffset.x + TILE_SIZE*x, roomOffset.y + TILE_SIZE*y }, WHITE);


}
}


DrawTextureRec(furniture, (Rectangle){ furset[30].posX, furset[30].posY, furset[30].width, furset[30].height }, roomOffset, WHITE);
DrawTextureRec(furniture, (Rectangle){ furset[29].posX, furset[29].posY, furset[29].width, furset[29].height }, (Vector2){ roomOffset.x + 29*36, roomOffset.y }, WHITE);

DrawTexture(roomba, roombaPosition.x - roomba.width/2, roombaPosition.y - roomba.height/2, WHITE);
DrawTexture(cat, catPosition.x - cat.width/2, catPosition.y - cat.height/2 - 10, WHITE);

float furAlpha = 1.0f;


for (int i = 0; i < furnitureCount; i++)
{
if (CheckCollisionCircleRec((Vector2){ roombaPosition.x - roomba.width/2, roombaPosition.y - roomba.height/2 }, roomba.width,
(Rectangle){ roomOffset.x + furmap[i].cellX*TILE_SIZE, roomOffset.y + furmap[i].cellY*TILE_SIZE,
furset[furmap[i].furId].width, furset[furmap[i].furId].height}) && (furmap[i].state == 1))
{
DrawTextureRec(furniture, (Rectangle){ furset[furmap[i].furId].posX, furset[furmap[i].furId].posY, furset[furmap[i].furId].width, furset[furmap[i].furId].height }, 
(Vector2){ roomOffset.x + furmap[i].cellX*TILE_SIZE, roomOffset.y + furmap[i].cellY*TILE_SIZE }, Fade(WHITE, 0.5f));
}
else
{
DrawTextureRec(furniture, (Rectangle){ furset[furmap[i].furId].posX, furset[furmap[i].furId].posY, furset[furmap[i].furId].width, furset[furmap[i].furId].height }, 
(Vector2){ roomOffset.x + furmap[i].cellX*TILE_SIZE, roomOffset.y + furmap[i].cellY*TILE_SIZE }, Fade(WHITE, furAlpha));
}
}

#if defined(TILE_VIEWER_MODE)
DrawTexture(tracemap, roomOffset.x, roomOffset.y, Fade(WHITE, 0.5f));
DrawTexture(fursetid, 0, 720, WHITE);
#endif





DrawTextEx(font2, "SCORE:", (Vector2){ 80, 10 }, font2.baseSize, 2, WHITE);
DrawTextEx(font, FormatText("%i", score), (Vector2){ 260, 10 }, font.baseSize, 2, WHITE);
DrawTextEx(font2, "CLEAN:", (Vector2){ 500, 10 }, font2.baseSize, 2, WHITE);
DrawTextEx(font, FormatText("%.2f%%", GetTileCleanPercent()), (Vector2){ 690, 10 }, font.baseSize, 2, WHITE);
DrawTextEx(font2, "TIME:", (Vector2){ 950, 10 }, font2.baseSize, 2, WHITE);
DrawTextEx(font, FormatText("%i:%02is", timeLevelSeconds/60, timeLevelSeconds%60), (Vector2){ 1100, 10 }, font.baseSize, 2, WHITE);






if ((mouseTileY >= 0) && (mouseTileY < MAX_TILES_Y) && (mouseTileX >= 0) && (mouseTileX < MAX_TILES_X))
{
DrawRectangleLinesEx((Rectangle){ tiles[mouseTileY*MAX_TILES_X + mouseTileX].position.x, 
tiles[mouseTileY*MAX_TILES_X + mouseTileX].position.y, TILE_SIZE, TILE_SIZE }, 2, RED);
}


for (int i = 0; i < MAX_SCORE_POPUPS; i++)
{
if (popup[i].enabled) DrawText(FormatText("+%i", popup[i].value), popup[i].position.x, popup[i].position.y, 20, Fade(RED, popup[i].alpha)); 
}


if (showObjective)
{
DrawRectangle(0, 150, GetScreenWidth(), GetScreenHeight() - 300, Fade(DARKGRAY, 0.7f));
DrawTextEx(font2, "OBJECTIVE:", (Vector2){ 500, 240 }, font2.baseSize, 2, WHITE);
DrawTextEx(font, "CLEAN 80% OF THE ROOM", (Vector2){ 300, 320 }, font.baseSize, 2, WHITE);
}

}


void UnloadGameplayScreen(void)
{

UnloadTexture(roomba);
UnloadTexture(cat);
UnloadTexture(dirtiles);
UnloadTexture(furniture);

UnloadSound(fxCat[0]);
UnloadSound(fxCat[1]);
UnloadSound(fxRoomba[0]);
UnloadSound(fxRoomba[1]);
UnloadSound(fxRoomba[2]);

StopMusicStream(catch);
UnloadMusicStream(catch);
}


int FinishGameplayScreen(void)
{
return finishScreen;
}






static float GetTileCleanPercent(void)
{
float value = 0;

int tileLevelsToClean = 0;
int tileLevelsCleaned = 0;

for (int y = 0; y < MAX_TILES_Y; y++)
{
for (int x = 0; x < MAX_TILES_X; x++)
{
if (tiles[y*MAX_TILES_X + x].level > 0)
{
tileLevelsToClean += tiles[y*MAX_TILES_X + x].level;
tileLevelsCleaned += tiles[y*MAX_TILES_X + x].state;
}
}
}

value = ((float)(tileLevelsToClean - tileLevelsCleaned)/tileLevelsToClean)*100.0f;

return value;
}