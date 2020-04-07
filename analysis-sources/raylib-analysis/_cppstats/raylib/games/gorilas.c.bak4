












#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif




#define MAX_BUILDINGS 15
#define MAX_EXPLOSIONS 200
#define MAX_PLAYERS 2

#define BUILDING_RELATIVE_ERROR 30 
#define BUILDING_MIN_RELATIVE_HEIGHT 20 
#define BUILDING_MAX_RELATIVE_HEIGHT 60 
#define BUILDING_MIN_GRAYSCALE_COLOR 120 
#define BUILDING_MAX_GRAYSCALE_COLOR 200 

#define MIN_PLAYER_POSITION 5 
#define MAX_PLAYER_POSITION 20 

#define GRAVITY 9.81f
#define DELTA_FPS 60




typedef struct Player {
Vector2 position;
Vector2 size;

Vector2 aimingPoint;
int aimingAngle;
int aimingPower;

Vector2 previousPoint;
int previousAngle;
int previousPower;

Vector2 impactPoint;

bool isLeftTeam; 
bool isPlayer; 
bool isAlive;
} Player;

typedef struct Building {
Rectangle rectangle;
Color color;
} Building;

typedef struct Explosion {
Vector2 position;
int radius;
bool active;
} Explosion;

typedef struct Ball {
Vector2 position;
Vector2 speed;
int radius;
bool active;
} Ball;




static const int screenWidth = 800;
static const int screenHeight = 450;

static bool gameOver = false;
static bool pause = false;

static Player player[MAX_PLAYERS] = { 0 };
static Building building[MAX_BUILDINGS] = { 0 };
static Explosion explosion[MAX_EXPLOSIONS] = { 0 };
static Ball ball = { 0 };

static int playerTurn = 0;
static bool ballOnAir = false;




static void InitGame(void); 
static void UpdateGame(void); 
static void DrawGame(void); 
static void UnloadGame(void); 
static void UpdateDrawFrame(void); 


static void InitBuildings(void);
static void InitPlayers(void);
static bool UpdatePlayer(int playerTurn);
static bool UpdateBall(int playerTurn);




int main(void)
{


InitWindow(screenWidth, screenHeight, "sample game: gorilas");

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

ball.radius = 10;
ballOnAir = false;
ball.active = false;

InitBuildings();
InitPlayers();


for (int i = 0; i < MAX_EXPLOSIONS; i++)
{
explosion[i].position = (Vector2){ 0.0f, 0.0f };
explosion[i].radius = 30;
explosion[i].active = false;
}
}


void UpdateGame(void)
{
if (!gameOver)
{
if (IsKeyPressed('P')) pause = !pause;

if (!pause)
{
if (!ballOnAir) ballOnAir = UpdatePlayer(playerTurn); 
else
{
if (UpdateBall(playerTurn)) 
{

bool leftTeamAlive = false;
bool rightTeamAlive = false;

for (int i = 0; i < MAX_PLAYERS; i++)
{
if (player[i].isAlive)
{
if (player[i].isLeftTeam) leftTeamAlive = true;
if (!player[i].isLeftTeam) rightTeamAlive = true;
}
}

if (leftTeamAlive && rightTeamAlive)
{
ballOnAir = false;
ball.active = false;

playerTurn++;

if (playerTurn == MAX_PLAYERS) playerTurn = 0;
}
else
{
gameOver = true;



}
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

for (int i = 0; i < MAX_BUILDINGS; i++) DrawRectangleRec(building[i].rectangle, building[i].color);


for (int i = 0; i < MAX_EXPLOSIONS; i++)
{
if (explosion[i].active) DrawCircle(explosion[i].position.x, explosion[i].position.y, explosion[i].radius, RAYWHITE);
}


for (int i = 0; i < MAX_PLAYERS; i++)
{
if (player[i].isAlive)
{
if (player[i].isLeftTeam) DrawRectangle(player[i].position.x - player[i].size.x/2, player[i].position.y - player[i].size.y/2,
player[i].size.x, player[i].size.y, BLUE);
else DrawRectangle(player[i].position.x - player[i].size.x/2, player[i].position.y - player[i].size.y/2,
player[i].size.x, player[i].size.y, RED);
}
}


if (ball.active) DrawCircle(ball.position.x, ball.position.y, ball.radius, MAROON);


if (!ballOnAir)
{























if (player[playerTurn].isLeftTeam)
{

DrawTriangle((Vector2){ player[playerTurn].position.x - player[playerTurn].size.x/4, player[playerTurn].position.y - player[playerTurn].size.y/4 },
(Vector2){ player[playerTurn].position.x + player[playerTurn].size.x/4, player[playerTurn].position.y + player[playerTurn].size.y/4 },
player[playerTurn].previousPoint, GRAY);


DrawTriangle((Vector2){ player[playerTurn].position.x - player[playerTurn].size.x/4, player[playerTurn].position.y - player[playerTurn].size.y/4 },
(Vector2){ player[playerTurn].position.x + player[playerTurn].size.x/4, player[playerTurn].position.y + player[playerTurn].size.y/4 },
player[playerTurn].aimingPoint, DARKBLUE);
}
else
{

DrawTriangle((Vector2){ player[playerTurn].position.x - player[playerTurn].size.x/4, player[playerTurn].position.y + player[playerTurn].size.y/4 },
(Vector2){ player[playerTurn].position.x + player[playerTurn].size.x/4, player[playerTurn].position.y - player[playerTurn].size.y/4 },
player[playerTurn].previousPoint, GRAY);


DrawTriangle((Vector2){ player[playerTurn].position.x - player[playerTurn].size.x/4, player[playerTurn].position.y + player[playerTurn].size.y/4 },
(Vector2){ player[playerTurn].position.x + player[playerTurn].size.x/4, player[playerTurn].position.y - player[playerTurn].size.y/4 },
player[playerTurn].aimingPoint, MAROON);
}
}

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




static void InitBuildings(void)
{

int currentWidth = 0;




float relativeWidth = 100/(100 - BUILDING_RELATIVE_ERROR);
float buildingWidthMean = (screenWidth*relativeWidth/MAX_BUILDINGS) + 1; 


int currentHeighth = 0;
int grayLevel;


for (int i = 0; i < MAX_BUILDINGS; i++)
{

building[i].rectangle.x = currentWidth;
building[i].rectangle.width = GetRandomValue(buildingWidthMean*(100 - BUILDING_RELATIVE_ERROR/2)/100 + 1, buildingWidthMean*(100 + BUILDING_RELATIVE_ERROR)/100);

currentWidth += building[i].rectangle.width;


currentHeighth = GetRandomValue(BUILDING_MIN_RELATIVE_HEIGHT, BUILDING_MAX_RELATIVE_HEIGHT);
building[i].rectangle.y = screenHeight - (screenHeight*currentHeighth/100);
building[i].rectangle.height = screenHeight*currentHeighth/100 + 1;


grayLevel = GetRandomValue(BUILDING_MIN_GRAYSCALE_COLOR, BUILDING_MAX_GRAYSCALE_COLOR);
building[i].color = (Color){ grayLevel, grayLevel, grayLevel, 255 };
}
}

static void InitPlayers(void)
{
for (int i = 0; i < MAX_PLAYERS; i++)
{
player[i].isAlive = true;


if (i % 2 == 0) player[i].isLeftTeam = true;
else player[i].isLeftTeam = false;


player[i].isPlayer = true;


player[i].size = (Vector2){ 40, 40 };


if (player[i].isLeftTeam) player[i].position.x = GetRandomValue(screenWidth*MIN_PLAYER_POSITION/100, screenWidth*MAX_PLAYER_POSITION/100);
else player[i].position.x = screenWidth - GetRandomValue(screenWidth*MIN_PLAYER_POSITION/100, screenWidth*MAX_PLAYER_POSITION/100);

for (int j = 0; j < MAX_BUILDINGS; j++)
{
if (building[j].rectangle.x > player[i].position.x)
{

player[i].position.x = building[j-1].rectangle.x + building[j-1].rectangle.width/2;

player[i].position.y = building[j-1].rectangle.y - player[i].size.y/2;
break;
}
}


player[i].aimingPoint = player[i].position;
player[i].previousAngle = 0;
player[i].previousPower = 0;
player[i].previousPoint = player[i].position;
player[i].aimingAngle = 0;
player[i].aimingPower = 0;

player[i].impactPoint = (Vector2){ -100, -100 };
}
}

static bool UpdatePlayer(int playerTurn)
{

if (GetMousePosition().y <= player[playerTurn].position.y)
{

if (player[playerTurn].isLeftTeam && GetMousePosition().x >= player[playerTurn].position.x)
{

player[playerTurn].aimingPower = sqrt(pow(player[playerTurn].position.x - GetMousePosition().x, 2) + pow(player[playerTurn].position.y - GetMousePosition().y, 2));

player[playerTurn].aimingAngle = asin((player[playerTurn].position.y - GetMousePosition().y)/player[playerTurn].aimingPower)*RAD2DEG;

player[playerTurn].aimingPoint = GetMousePosition();


if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
{
player[playerTurn].previousPoint = player[playerTurn].aimingPoint;
player[playerTurn].previousPower = player[playerTurn].aimingPower;
player[playerTurn].previousAngle = player[playerTurn].aimingAngle;
ball.position = player[playerTurn].position;

return true;
}
}

else if (!player[playerTurn].isLeftTeam && GetMousePosition().x <= player[playerTurn].position.x)
{

player[playerTurn].aimingPower = sqrt(pow(player[playerTurn].position.x - GetMousePosition().x, 2) + pow(player[playerTurn].position.y - GetMousePosition().y, 2));

player[playerTurn].aimingAngle = asin((player[playerTurn].position.y - GetMousePosition().y)/player[playerTurn].aimingPower)*RAD2DEG;

player[playerTurn].aimingPoint = GetMousePosition();


if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
{
player[playerTurn].previousPoint = player[playerTurn].aimingPoint;
player[playerTurn].previousPower = player[playerTurn].aimingPower;
player[playerTurn].previousAngle = player[playerTurn].aimingAngle;
ball.position = player[playerTurn].position;

return true;
}
}
else
{
player[playerTurn].aimingPoint = player[playerTurn].position;
player[playerTurn].aimingPower = 0;
player[playerTurn].aimingAngle = 0;
}
}
else
{
player[playerTurn].aimingPoint = player[playerTurn].position;
player[playerTurn].aimingPower = 0;
player[playerTurn].aimingAngle = 0;
}

return false;
}

static bool UpdateBall(int playerTurn)
{
static int explosionNumber = 0;


if (!ball.active)
{
if (player[playerTurn].isLeftTeam)
{
ball.speed.x = cos(player[playerTurn].previousAngle*DEG2RAD)*player[playerTurn].previousPower*3/DELTA_FPS;
ball.speed.y = -sin(player[playerTurn].previousAngle*DEG2RAD)*player[playerTurn].previousPower*3/DELTA_FPS;
ball.active = true;
}
else
{
ball.speed.x = -cos(player[playerTurn].previousAngle*DEG2RAD)*player[playerTurn].previousPower*3/DELTA_FPS;
ball.speed.y = -sin(player[playerTurn].previousAngle*DEG2RAD)*player[playerTurn].previousPower*3/DELTA_FPS;
ball.active = true;
}
}

ball.position.x += ball.speed.x;
ball.position.y += ball.speed.y;
ball.speed.y += GRAVITY/DELTA_FPS;


if (ball.position.x + ball.radius < 0) return true;
else if (ball.position.x - ball.radius > screenWidth) return true;
else
{

for (int i = 0; i < MAX_PLAYERS; i++)
{
if (CheckCollisionCircleRec(ball.position, ball.radius, (Rectangle){ player[i].position.x - player[i].size.x/2, player[i].position.y - player[i].size.y/2,
player[i].size.x, player[i].size.y }))
{

if (i == playerTurn) return false;
else
{

player[playerTurn].impactPoint.x = ball.position.x;
player[playerTurn].impactPoint.y = ball.position.y + ball.radius;


player[i].isAlive = false;
return true;
}
}
}



for (int i = 0; i < MAX_BUILDINGS; i++)
{
if (CheckCollisionCircles(ball.position, ball.radius, explosion[i].position, explosion[i].radius - ball.radius))
{
return false;
}
}

for (int i = 0; i < MAX_BUILDINGS; i++)
{
if (CheckCollisionCircleRec(ball.position, ball.radius, building[i].rectangle))
{

player[playerTurn].impactPoint.x = ball.position.x;
player[playerTurn].impactPoint.y = ball.position.y + ball.radius;


explosion[explosionNumber].position = player[playerTurn].impactPoint;
explosion[explosionNumber].active = true;
explosionNumber++;

return true;
}
}
}

return false;
}
