












#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif




#define SQUARE_SIZE 20

#define GRID_HORIZONTAL_SIZE 12
#define GRID_VERTICAL_SIZE 20

#define LATERAL_SPEED 10
#define TURNING_SPEED 12
#define FAST_FALL_AWAIT_COUNTER 30

#define FADING_TIME 33




typedef enum GridSquare { EMPTY, MOVING, FULL, BLOCK, FADING } GridSquare;




static const int screenWidth = 800;
static const int screenHeight = 450;

static bool gameOver = false;
static bool pause = false;


static GridSquare grid [GRID_HORIZONTAL_SIZE][GRID_VERTICAL_SIZE];
static GridSquare piece [4][4];
static GridSquare incomingPiece [4][4];


static int piecePositionX = 0;
static int piecePositionY = 0;


static Color fadingColor;


static bool beginPlay = true; 
static bool pieceActive = false;
static bool detection = false;
static bool lineToDelete = false;


static int level = 1;
static int lines = 0;


static int gravityMovementCounter = 0;
static int lateralMovementCounter = 0;
static int turnMovementCounter = 0;
static int fastFallMovementCounter = 0;

static int fadeLineCounter = 0;


static int gravitySpeed = 30;




static void InitGame(void); 
static void UpdateGame(void); 
static void DrawGame(void); 
static void UnloadGame(void); 
static void UpdateDrawFrame(void); 


static bool Createpiece();
static void GetRandompiece();
static void ResolveFallingMovement();
static bool ResolveLateralMovement();
static bool ResolveTurnMovement();
static void CheckDetection();
static void CheckCompletion();
static void DeleteCompleteLines();




int main(void)
{


InitWindow(screenWidth, screenHeight, "sample game: tetris");

InitGame();

#if defined(PLATFORM_WEB)
emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
SetTargetFPS(60);



while (!WindowShouldClose()) 
{


UpdateDrawFrame();

}
#endif


UnloadGame(); 

CloseWindow(); 


return 0;
}






void InitGame(void)
{

level = 1;
lines = 0;

fadingColor = GRAY;

piecePositionX = 0;
piecePositionY = 0;

pause = false;

beginPlay = true;
pieceActive = false;
detection = false;
lineToDelete = false;


gravityMovementCounter = 0;
lateralMovementCounter = 0;
turnMovementCounter = 0;
fastFallMovementCounter = 0;

fadeLineCounter = 0;
gravitySpeed = 30;


for (int i = 0; i < GRID_HORIZONTAL_SIZE; i++)
{
for (int j = 0; j < GRID_VERTICAL_SIZE; j++)
{
if ((j == GRID_VERTICAL_SIZE - 1) || (i == 0) || (i == GRID_HORIZONTAL_SIZE - 1)) grid[i][j] = BLOCK;
else grid[i][j] = EMPTY;
}
}


for (int i = 0; i < 4; i++)
{
for (int j = 0; j< 4; j++)
{
incomingPiece[i][j] = EMPTY;
}
}
}


void UpdateGame(void)
{
if (!gameOver)
{
if (IsKeyPressed('P')) pause = !pause;

if (!pause)
{
if (!lineToDelete)
{
if (!pieceActive)
{

pieceActive = Createpiece();


fastFallMovementCounter = 0;
}
else 
{

fastFallMovementCounter++;
gravityMovementCounter++;
lateralMovementCounter++;
turnMovementCounter++;


if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT)) lateralMovementCounter = LATERAL_SPEED;
if (IsKeyPressed(KEY_UP)) turnMovementCounter = TURNING_SPEED;


if (IsKeyDown(KEY_DOWN) && (fastFallMovementCounter >= FAST_FALL_AWAIT_COUNTER))
{

gravityMovementCounter += gravitySpeed;
}

if (gravityMovementCounter >= gravitySpeed)
{

CheckDetection(&detection);


ResolveFallingMovement(&detection, &pieceActive);


CheckCompletion(&lineToDelete);

gravityMovementCounter = 0;
}


if (lateralMovementCounter >= LATERAL_SPEED)
{

if (!ResolveLateralMovement()) lateralMovementCounter = 0;
}


if (turnMovementCounter >= TURNING_SPEED)
{

if (ResolveTurnMovement()) turnMovementCounter = 0;
}
}


for (int j = 0; j < 2; j++)
{
for (int i = 1; i < GRID_HORIZONTAL_SIZE - 1; i++)
{
if (grid[i][j] == FULL)
{
gameOver = true;
}
}
}
}
else
{

fadeLineCounter++;

if (fadeLineCounter%8 < 4) fadingColor = MAROON;
else fadingColor = GRAY;

if (fadeLineCounter >= FADING_TIME)
{
DeleteCompleteLines();
fadeLineCounter = 0;
lineToDelete = false;

lines++;
}
}
}
}
else
{
if (IsKeyPressed(KEY_ENTER))
{
InitGame();
gameOver = false;
}
}
}


void DrawGame(void)
{
BeginDrawing();

ClearBackground(RAYWHITE);

if (!gameOver)
{

Vector2 offset;
offset.x = screenWidth/2 - (GRID_HORIZONTAL_SIZE*SQUARE_SIZE/2) - 50;
offset.y = screenHeight/2 - ((GRID_VERTICAL_SIZE - 1)*SQUARE_SIZE/2) + SQUARE_SIZE*2;

offset.y -= 50; 

int controller = offset.x;

for (int j = 0; j < GRID_VERTICAL_SIZE; j++)
{
for (int i = 0; i < GRID_HORIZONTAL_SIZE; i++)
{

if (grid[i][j] == EMPTY)
{
DrawLine(offset.x, offset.y, offset.x + SQUARE_SIZE, offset.y, LIGHTGRAY );
DrawLine(offset.x, offset.y, offset.x, offset.y + SQUARE_SIZE, LIGHTGRAY );
DrawLine(offset.x + SQUARE_SIZE, offset.y, offset.x + SQUARE_SIZE, offset.y + SQUARE_SIZE, LIGHTGRAY );
DrawLine(offset.x, offset.y + SQUARE_SIZE, offset.x + SQUARE_SIZE, offset.y + SQUARE_SIZE, LIGHTGRAY );
offset.x += SQUARE_SIZE;
}
else if (grid[i][j] == FULL)
{
DrawRectangle(offset.x, offset.y, SQUARE_SIZE, SQUARE_SIZE, GRAY);
offset.x += SQUARE_SIZE;
}
else if (grid[i][j] == MOVING)
{
DrawRectangle(offset.x, offset.y, SQUARE_SIZE, SQUARE_SIZE, DARKGRAY);
offset.x += SQUARE_SIZE;
}
else if (grid[i][j] == BLOCK)
{
DrawRectangle(offset.x, offset.y, SQUARE_SIZE, SQUARE_SIZE, LIGHTGRAY);
offset.x += SQUARE_SIZE;
}
else if (grid[i][j] == FADING)
{
DrawRectangle(offset.x, offset.y, SQUARE_SIZE, SQUARE_SIZE, fadingColor);
offset.x += SQUARE_SIZE;
}
}

offset.x = controller;
offset.y += SQUARE_SIZE;
}


offset.x = 500;
offset.y = 45;

int controler = offset.x;

for (int j = 0; j < 4; j++)
{
for (int i = 0; i < 4; i++)
{
if (incomingPiece[i][j] == EMPTY)
{
DrawLine(offset.x, offset.y, offset.x + SQUARE_SIZE, offset.y, LIGHTGRAY );
DrawLine(offset.x, offset.y, offset.x, offset.y + SQUARE_SIZE, LIGHTGRAY );
DrawLine(offset.x + SQUARE_SIZE, offset.y, offset.x + SQUARE_SIZE, offset.y + SQUARE_SIZE, LIGHTGRAY );
DrawLine(offset.x, offset.y + SQUARE_SIZE, offset.x + SQUARE_SIZE, offset.y + SQUARE_SIZE, LIGHTGRAY );
offset.x += SQUARE_SIZE;
}
else if (incomingPiece[i][j] == MOVING)
{
DrawRectangle(offset.x, offset.y, SQUARE_SIZE, SQUARE_SIZE, GRAY);
offset.x += SQUARE_SIZE;
}
}

offset.x = controler;
offset.y += SQUARE_SIZE;
}

DrawText("INCOMING:", offset.x, offset.y - 100, 10, GRAY);
DrawText(TextFormat("LINES: %04i", lines), offset.x, offset.y + 20, 10, GRAY);

if (pause) DrawText("GAME PAUSED", screenWidth/2 - MeasureText("GAME PAUSED", 40)/2, screenHeight/2 - 40, 40, GRAY);
}
else DrawText("PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth()/2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20)/2, GetScreenHeight()/2 - 50, 20, GRAY);

EndDrawing();
}


void UnloadGame(void)
{

}


void UpdateDrawFrame(void)
{
UpdateGame();
DrawGame();
}




static bool Createpiece()
{
piecePositionX = (int)((GRID_HORIZONTAL_SIZE - 4)/2);
piecePositionY = 0;


if (beginPlay)
{
GetRandompiece();
beginPlay = false;
}


for (int i = 0; i < 4; i++)
{
for (int j = 0; j< 4; j++)
{
piece[i][j] = incomingPiece[i][j];
}
}


GetRandompiece();


for (int i = piecePositionX; i < piecePositionX + 4; i++)
{
for (int j = 0; j < 4; j++)
{
if (piece[i - (int)piecePositionX][j] == MOVING) grid[i][j] = MOVING;
}
}

return true;
}

static void GetRandompiece()
{
int random = GetRandomValue(0, 6);

for (int i = 0; i < 4; i++)
{
for (int j = 0; j < 4; j++)
{
incomingPiece[i][j] = EMPTY;
}
}

switch (random)
{
case 0: { incomingPiece[1][1] = MOVING; incomingPiece[2][1] = MOVING; incomingPiece[1][2] = MOVING; incomingPiece[2][2] = MOVING; } break; 
case 1: { incomingPiece[1][0] = MOVING; incomingPiece[1][1] = MOVING; incomingPiece[1][2] = MOVING; incomingPiece[2][2] = MOVING; } break; 
case 2: { incomingPiece[1][2] = MOVING; incomingPiece[2][0] = MOVING; incomingPiece[2][1] = MOVING; incomingPiece[2][2] = MOVING; } break; 
case 3: { incomingPiece[0][1] = MOVING; incomingPiece[1][1] = MOVING; incomingPiece[2][1] = MOVING; incomingPiece[3][1] = MOVING; } break; 
case 4: { incomingPiece[1][0] = MOVING; incomingPiece[1][1] = MOVING; incomingPiece[1][2] = MOVING; incomingPiece[2][1] = MOVING; } break; 
case 5: { incomingPiece[1][1] = MOVING; incomingPiece[2][1] = MOVING; incomingPiece[2][2] = MOVING; incomingPiece[3][2] = MOVING; } break; 
case 6: { incomingPiece[1][2] = MOVING; incomingPiece[2][2] = MOVING; incomingPiece[2][1] = MOVING; incomingPiece[3][1] = MOVING; } break; 
}
}

static void ResolveFallingMovement(bool *detection, bool *pieceActive)
{

if (*detection)
{
for (int j = GRID_VERTICAL_SIZE - 2; j >= 0; j--)
{
for (int i = 1; i < GRID_HORIZONTAL_SIZE - 1; i++)
{
if (grid[i][j] == MOVING)
{
grid[i][j] = FULL;
*detection = false;
*pieceActive = false;
}
}
}
}
else 
{
for (int j = GRID_VERTICAL_SIZE - 2; j >= 0; j--)
{
for (int i = 1; i < GRID_HORIZONTAL_SIZE - 1; i++)
{
if (grid[i][j] == MOVING)
{
grid[i][j+1] = MOVING;
grid[i][j] = EMPTY;
}
}
}

piecePositionY++;
}
}

static bool ResolveLateralMovement()
{
bool collision = false;


if (IsKeyDown(KEY_LEFT)) 
{

for (int j = GRID_VERTICAL_SIZE - 2; j >= 0; j--)
{
for (int i = 1; i < GRID_HORIZONTAL_SIZE - 1; i++)
{
if (grid[i][j] == MOVING)
{

if ((i-1 == 0) || (grid[i-1][j] == FULL)) collision = true;
}
}
}


if (!collision)
{
for (int j = GRID_VERTICAL_SIZE - 2; j >= 0; j--)
{
for (int i = 1; i < GRID_HORIZONTAL_SIZE - 1; i++) 
{

if (grid[i][j] == MOVING)
{
grid[i-1][j] = MOVING;
grid[i][j] = EMPTY;
}
}
}

piecePositionX--;
}
}
else if (IsKeyDown(KEY_RIGHT)) 
{

for (int j = GRID_VERTICAL_SIZE - 2; j >= 0; j--)
{
for (int i = 1; i < GRID_HORIZONTAL_SIZE - 1; i++)
{
if (grid[i][j] == MOVING)
{

if ((i+1 == GRID_HORIZONTAL_SIZE - 1) || (grid[i+1][j] == FULL))
{
collision = true;

}
}
}
}


if (!collision)
{
for (int j = GRID_VERTICAL_SIZE - 2; j >= 0; j--)
{
for (int i = GRID_HORIZONTAL_SIZE - 1; i >= 1; i--) 
{

if (grid[i][j] == MOVING)
{
grid[i+1][j] = MOVING;
grid[i][j] = EMPTY;
}
}
}

piecePositionX++;
}
}

return collision;
}

static bool ResolveTurnMovement()
{

if (IsKeyDown(KEY_UP))
{
int aux = 0;
bool checker = false;


if ((grid[piecePositionX + 3][piecePositionY] == MOVING) &&
(grid[piecePositionX][piecePositionY] != EMPTY) &&
(grid[piecePositionX][piecePositionY] != MOVING)) checker = true;

if ((grid[piecePositionX + 3][piecePositionY + 3] == MOVING) &&
(grid[piecePositionX + 3][piecePositionY] != EMPTY) &&
(grid[piecePositionX + 3][piecePositionY] != MOVING)) checker = true;

if ((grid[piecePositionX][piecePositionY + 3] == MOVING) &&
(grid[piecePositionX + 3][piecePositionY + 3] != EMPTY) &&
(grid[piecePositionX + 3][piecePositionY + 3] != MOVING)) checker = true;

if ((grid[piecePositionX][piecePositionY] == MOVING) &&
(grid[piecePositionX][piecePositionY + 3] != EMPTY) &&
(grid[piecePositionX][piecePositionY + 3] != MOVING)) checker = true;

if ((grid[piecePositionX + 1][piecePositionY] == MOVING) &&
(grid[piecePositionX][piecePositionY + 2] != EMPTY) &&
(grid[piecePositionX][piecePositionY + 2] != MOVING)) checker = true;

if ((grid[piecePositionX + 3][piecePositionY + 1] == MOVING) &&
(grid[piecePositionX + 1][piecePositionY] != EMPTY) &&
(grid[piecePositionX + 1][piecePositionY] != MOVING)) checker = true;

if ((grid[piecePositionX + 2][piecePositionY + 3] == MOVING) &&
(grid[piecePositionX + 3][piecePositionY + 1] != EMPTY) &&
(grid[piecePositionX + 3][piecePositionY + 1] != MOVING)) checker = true;

if ((grid[piecePositionX][piecePositionY + 2] == MOVING) &&
(grid[piecePositionX + 2][piecePositionY + 3] != EMPTY) &&
(grid[piecePositionX + 2][piecePositionY + 3] != MOVING)) checker = true;

if ((grid[piecePositionX + 2][piecePositionY] == MOVING) &&
(grid[piecePositionX][piecePositionY + 1] != EMPTY) &&
(grid[piecePositionX][piecePositionY + 1] != MOVING)) checker = true;

if ((grid[piecePositionX + 3][piecePositionY + 2] == MOVING) &&
(grid[piecePositionX + 2][piecePositionY] != EMPTY) &&
(grid[piecePositionX + 2][piecePositionY] != MOVING)) checker = true;

if ((grid[piecePositionX + 1][piecePositionY + 3] == MOVING) &&
(grid[piecePositionX + 3][piecePositionY + 2] != EMPTY) &&
(grid[piecePositionX + 3][piecePositionY + 2] != MOVING)) checker = true;

if ((grid[piecePositionX][piecePositionY + 1] == MOVING) &&
(grid[piecePositionX + 1][piecePositionY + 3] != EMPTY) &&
(grid[piecePositionX + 1][piecePositionY + 3] != MOVING)) checker = true;

if ((grid[piecePositionX + 1][piecePositionY + 1] == MOVING) &&
(grid[piecePositionX + 1][piecePositionY + 2] != EMPTY) &&
(grid[piecePositionX + 1][piecePositionY + 2] != MOVING)) checker = true;

if ((grid[piecePositionX + 2][piecePositionY + 1] == MOVING) &&
(grid[piecePositionX + 1][piecePositionY + 1] != EMPTY) &&
(grid[piecePositionX + 1][piecePositionY + 1] != MOVING)) checker = true;

if ((grid[piecePositionX + 2][piecePositionY + 2] == MOVING) &&
(grid[piecePositionX + 2][piecePositionY + 1] != EMPTY) &&
(grid[piecePositionX + 2][piecePositionY + 1] != MOVING)) checker = true;

if ((grid[piecePositionX + 1][piecePositionY + 2] == MOVING) &&
(grid[piecePositionX + 2][piecePositionY + 2] != EMPTY) &&
(grid[piecePositionX + 2][piecePositionY + 2] != MOVING)) checker = true;

if (!checker)
{
aux = piece[0][0];
piece[0][0] = piece[3][0];
piece[3][0] = piece[3][3];
piece[3][3] = piece[0][3];
piece[0][3] = aux;

aux = piece[1][0];
piece[1][0] = piece[3][1];
piece[3][1] = piece[2][3];
piece[2][3] = piece[0][2];
piece[0][2] = aux;

aux = piece[2][0];
piece[2][0] = piece[3][2];
piece[3][2] = piece[1][3];
piece[1][3] = piece[0][1];
piece[0][1] = aux;

aux = piece[1][1];
piece[1][1] = piece[2][1];
piece[2][1] = piece[2][2];
piece[2][2] = piece[1][2];
piece[1][2] = aux;
}

for (int j = GRID_VERTICAL_SIZE - 2; j >= 0; j--)
{
for (int i = 1; i < GRID_HORIZONTAL_SIZE - 1; i++)
{
if (grid[i][j] == MOVING)
{
grid[i][j] = EMPTY;
}
}
}

for (int i = piecePositionX; i < piecePositionX + 4; i++)
{
for (int j = piecePositionY; j < piecePositionY + 4; j++)
{
if (piece[i - piecePositionX][j - piecePositionY] == MOVING)
{
grid[i][j] = MOVING;
}
}
}

return true;
}

return false;
}

static void CheckDetection(bool *detection)
{
for (int j = GRID_VERTICAL_SIZE - 2; j >= 0; j--)
{
for (int i = 1; i < GRID_HORIZONTAL_SIZE - 1; i++)
{
if ((grid[i][j] == MOVING) && ((grid[i][j+1] == FULL) || (grid[i][j+1] == BLOCK))) *detection = true;
}
}
}

static void CheckCompletion(bool *lineToDelete)
{
int calculator = 0;

for (int j = GRID_VERTICAL_SIZE - 2; j >= 0; j--)
{
calculator = 0;
for (int i = 1; i < GRID_HORIZONTAL_SIZE - 1; i++)
{

if (grid[i][j] == FULL)
{
calculator++;
}


if (calculator == GRID_HORIZONTAL_SIZE - 2)
{
*lineToDelete = true;
calculator = 0;



for (int z = 1; z < GRID_HORIZONTAL_SIZE - 1; z++)
{
grid[z][j] = FADING;
}
}
}
}
}

static void DeleteCompleteLines()
{

for (int j = GRID_VERTICAL_SIZE - 2; j >= 0; j--)
{
while (grid[1][j] == FADING)
{
for (int i = 1; i < GRID_HORIZONTAL_SIZE - 1; i++)
{
grid[i][j] = EMPTY;
}

for (int j2 = j-1; j2 >= 0; j2--)
{
for (int i2 = 1; i2 < GRID_HORIZONTAL_SIZE - 1; i2++)
{
if (grid[i2][j2] == FULL)
{
grid[i2][j2+1] = FULL;
grid[i2][j2] = EMPTY;
}
else if (grid[i2][j2] == FADING)
{
grid[i2][j2+1] = FADING;
grid[i2][j2] = EMPTY;
}
}
}
}
}
}
