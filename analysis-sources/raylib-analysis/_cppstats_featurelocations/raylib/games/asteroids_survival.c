












#include "raylib.h"

#include <math.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif




#define PLAYER_BASE_SIZE 20.0f
#define PLAYER_SPEED 6.0f
#define PLAYER_MAX_SHOOTS 10

#define METEORS_SPEED 2
#define MAX_MEDIUM_METEORS 8
#define MAX_SMALL_METEORS 16




typedef struct Player {
Vector2 position;
Vector2 speed;
float acceleration;
float rotation;
Vector3 collider;
Color color;
} Player;

typedef struct Meteor {
Vector2 position;
Vector2 speed;
float radius;
bool active;
Color color;
} Meteor;




static const int screenWidth = 800;
static const int screenHeight = 450;

static int framesCounter = 0;
static bool gameOver = false;
static bool pause = false;


static float shipHeight = 0.0f;

static Player player = { 0 };
static Meteor mediumMeteor[MAX_MEDIUM_METEORS] = { 0 };
static Meteor smallMeteor[MAX_SMALL_METEORS] = { 0 };




static void InitGame(void); 
static void UpdateGame(void); 
static void DrawGame(void); 
static void UnloadGame(void); 
static void UpdateDrawFrame(void); 




int main(void)
{


InitWindow(screenWidth, screenHeight, "sample game: asteroids survival");

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
int posx, posy;
int velx, vely;
bool correctRange = false;

pause = false;

framesCounter = 0;

shipHeight = (PLAYER_BASE_SIZE/2)/tanf(20*DEG2RAD);


player.position = (Vector2){screenWidth/2, screenHeight/2 - shipHeight/2};
player.speed = (Vector2){0, 0};
player.acceleration = 0;
player.rotation = 0;
player.collider = (Vector3){player.position.x + sin(player.rotation*DEG2RAD)*(shipHeight/2.5f), player.position.y - cos(player.rotation*DEG2RAD)*(shipHeight/2.5f), 12};
player.color = LIGHTGRAY;

for (int i = 0; i < MAX_MEDIUM_METEORS; i++)
{
posx = GetRandomValue(0, screenWidth);

while(!correctRange)
{
if (posx > screenWidth/2 - 150 && posx < screenWidth/2 + 150) posx = GetRandomValue(0, screenWidth);
else correctRange = true;
}

correctRange = false;

posy = GetRandomValue(0, screenHeight);

while(!correctRange)
{
if (posy > screenHeight/2 - 150 && posy < screenHeight/2 + 150) posy = GetRandomValue(0, screenHeight);
else correctRange = true;
}

correctRange = false;
velx = GetRandomValue(-METEORS_SPEED, METEORS_SPEED);
vely = GetRandomValue(-METEORS_SPEED, METEORS_SPEED);

while(!correctRange)
{
if (velx == 0 && vely == 0)
{
velx = GetRandomValue(-METEORS_SPEED, METEORS_SPEED);
vely = GetRandomValue(-METEORS_SPEED, METEORS_SPEED);
}
else correctRange = true;
}
mediumMeteor[i].position = (Vector2){posx, posy};
mediumMeteor[i].speed = (Vector2){velx, vely};
mediumMeteor[i].radius = 20;
mediumMeteor[i].active = true;
mediumMeteor[i].color = GREEN;
}

for (int i = 0; i < MAX_SMALL_METEORS; i++)
{
posx = GetRandomValue(0, screenWidth);

while(!correctRange)
{
if (posx > screenWidth/2 - 150 && posx < screenWidth/2 + 150) posx = GetRandomValue(0, screenWidth);
else correctRange = true;
}

correctRange = false;

posy = GetRandomValue(0, screenHeight);

while(!correctRange)
{
if (posy > screenHeight/2 - 150 && posy < screenHeight/2 + 150) posy = GetRandomValue(0, screenHeight);
else correctRange = true;
}

correctRange = false;
velx = GetRandomValue(-METEORS_SPEED, METEORS_SPEED);
vely = GetRandomValue(-METEORS_SPEED, METEORS_SPEED);

while(!correctRange)
{
if (velx == 0 && vely == 0)
{
velx = GetRandomValue(-METEORS_SPEED, METEORS_SPEED);
vely = GetRandomValue(-METEORS_SPEED, METEORS_SPEED);
}
else correctRange = true;
}
smallMeteor[i].position = (Vector2){posx, posy};
smallMeteor[i].speed = (Vector2){velx, vely};
smallMeteor[i].radius = 10;
smallMeteor[i].active = true;
smallMeteor[i].color = YELLOW;
}
}


void UpdateGame(void)
{
if (!gameOver)
{
if (IsKeyPressed('P')) pause = !pause;

if (!pause)
{
framesCounter++;




if (IsKeyDown(KEY_LEFT)) player.rotation -= 5;
if (IsKeyDown(KEY_RIGHT)) player.rotation += 5;


player.speed.x = sin(player.rotation*DEG2RAD)*PLAYER_SPEED;
player.speed.y = cos(player.rotation*DEG2RAD)*PLAYER_SPEED;


if (IsKeyDown(KEY_UP))
{
if (player.acceleration < 1) player.acceleration += 0.04f;
}
else
{
if (player.acceleration > 0) player.acceleration -= 0.02f;
else if (player.acceleration < 0) player.acceleration = 0;
}
if (IsKeyDown(KEY_DOWN))
{
if (player.acceleration > 0) player.acceleration -= 0.04f;
else if (player.acceleration < 0) player.acceleration = 0;
}


player.position.x += (player.speed.x*player.acceleration);
player.position.y -= (player.speed.y*player.acceleration);


if (player.position.x > screenWidth + shipHeight) player.position.x = -(shipHeight);
else if (player.position.x < -(shipHeight)) player.position.x = screenWidth + shipHeight;
if (player.position.y > (screenHeight + shipHeight)) player.position.y = -(shipHeight);
else if (player.position.y < -(shipHeight)) player.position.y = screenHeight + shipHeight;


player.collider = (Vector3){player.position.x + sin(player.rotation*DEG2RAD)*(shipHeight/2.5f), player.position.y - cos(player.rotation*DEG2RAD)*(shipHeight/2.5f), 12};

for (int a = 0; a < MAX_MEDIUM_METEORS; a++)
{
if (CheckCollisionCircles((Vector2){player.collider.x, player.collider.y}, player.collider.z, mediumMeteor[a].position, mediumMeteor[a].radius) && mediumMeteor[a].active) gameOver = true;
}

for (int a = 0; a < MAX_SMALL_METEORS; a++)
{
if (CheckCollisionCircles((Vector2){player.collider.x, player.collider.y}, player.collider.z, smallMeteor[a].position, smallMeteor[a].radius) && smallMeteor[a].active) gameOver = true;
}



for (int i = 0; i < MAX_MEDIUM_METEORS; i++)
{
if (mediumMeteor[i].active)
{

mediumMeteor[i].position.x += mediumMeteor[i].speed.x;
mediumMeteor[i].position.y += mediumMeteor[i].speed.y;


if (mediumMeteor[i].position.x > screenWidth + mediumMeteor[i].radius) mediumMeteor[i].position.x = -(mediumMeteor[i].radius);
else if (mediumMeteor[i].position.x < 0 - mediumMeteor[i].radius) mediumMeteor[i].position.x = screenWidth + mediumMeteor[i].radius;
if (mediumMeteor[i].position.y > screenHeight + mediumMeteor[i].radius) mediumMeteor[i].position.y = -(mediumMeteor[i].radius);
else if (mediumMeteor[i].position.y < 0 - mediumMeteor[i].radius) mediumMeteor[i].position.y = screenHeight + mediumMeteor[i].radius;
}
}

for (int i = 0; i < MAX_SMALL_METEORS; i++)
{
if (smallMeteor[i].active)
{

smallMeteor[i].position.x += smallMeteor[i].speed.x;
smallMeteor[i].position.y += smallMeteor[i].speed.y;


if (smallMeteor[i].position.x > screenWidth + smallMeteor[i].radius) smallMeteor[i].position.x = -(smallMeteor[i].radius);
else if (smallMeteor[i].position.x < 0 - smallMeteor[i].radius) smallMeteor[i].position.x = screenWidth + smallMeteor[i].radius;
if (smallMeteor[i].position.y > screenHeight + smallMeteor[i].radius) smallMeteor[i].position.y = -(smallMeteor[i].radius);
else if (smallMeteor[i].position.y < 0 - smallMeteor[i].radius) smallMeteor[i].position.y = screenHeight + smallMeteor[i].radius;
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

Vector2 v1 = { player.position.x + sinf(player.rotation*DEG2RAD)*(shipHeight), player.position.y - cosf(player.rotation*DEG2RAD)*(shipHeight) };
Vector2 v2 = { player.position.x - cosf(player.rotation*DEG2RAD)*(PLAYER_BASE_SIZE/2), player.position.y - sinf(player.rotation*DEG2RAD)*(PLAYER_BASE_SIZE/2) };
Vector2 v3 = { player.position.x + cosf(player.rotation*DEG2RAD)*(PLAYER_BASE_SIZE/2), player.position.y + sinf(player.rotation*DEG2RAD)*(PLAYER_BASE_SIZE/2) };
DrawTriangle(v1, v2, v3, MAROON);


for (int i = 0;i < MAX_MEDIUM_METEORS; i++)
{
if (mediumMeteor[i].active) DrawCircleV(mediumMeteor[i].position, mediumMeteor[i].radius, GRAY);
else DrawCircleV(mediumMeteor[i].position, mediumMeteor[i].radius, Fade(LIGHTGRAY, 0.3f));
}

for (int i = 0;i < MAX_SMALL_METEORS; i++)
{
if (smallMeteor[i].active) DrawCircleV(smallMeteor[i].position, smallMeteor[i].radius, DARKGRAY);
else DrawCircleV(smallMeteor[i].position, smallMeteor[i].radius, Fade(LIGHTGRAY, 0.3f));
}

DrawText(TextFormat("TIME: %.02f", (float)framesCounter/60), 10, 10, 20, BLACK);

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
