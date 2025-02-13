












#include "raylib.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif




#define NUM_SHOOTS 50
#define NUM_MAX_ENEMIES 50
#define FIRST_WAVE 10
#define SECOND_WAVE 20
#define THIRD_WAVE 50




typedef enum { FIRST = 0, SECOND, THIRD } EnemyWave;

typedef struct Player{
Rectangle rec;
Vector2 speed;
Color color;
} Player;

typedef struct Enemy{
Rectangle rec;
Vector2 speed;
bool active;
Color color;
} Enemy;

typedef struct Shoot{
Rectangle rec;
Vector2 speed;
bool active;
Color color;
} Shoot;




static const int screenWidth = 800;
static const int screenHeight = 450;

static bool gameOver = false;
static bool pause = false;
static int score = 0;
static bool victory = false;

static Player player = { 0 };
static Enemy enemy[NUM_MAX_ENEMIES] = { 0 };
static Shoot shoot[NUM_SHOOTS] = { 0 };
static EnemyWave wave = { 0 };

static int shootRate = 0;
static float alpha = 0.0f;

static int activeEnemies = 0;
static int enemiesKill = 0;
static bool smooth = false;




static void InitGame(void); 
static void UpdateGame(void); 
static void DrawGame(void); 
static void UnloadGame(void); 
static void UpdateDrawFrame(void); 




int main(void)
{


InitWindow(screenWidth, screenHeight, "sample game: space invaders");

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

shootRate = 0;
pause = false;
gameOver = false;
victory = false;
smooth = false;
wave = FIRST;
activeEnemies = FIRST_WAVE;
enemiesKill = 0;
score = 0;
alpha = 0;


player.rec.x = 20;
player.rec.y = 50;
player.rec.width = 20;
player.rec.height = 20;
player.speed.x = 5;
player.speed.y = 5;
player.color = BLACK;


for (int i = 0; i < NUM_MAX_ENEMIES; i++)
{
enemy[i].rec.width = 10;
enemy[i].rec.height = 10;
enemy[i].rec.x = GetRandomValue(screenWidth, screenWidth + 1000);
enemy[i].rec.y = GetRandomValue(0, screenHeight - enemy[i].rec.height);
enemy[i].speed.x = 5;
enemy[i].speed.y = 5;
enemy[i].active = true;
enemy[i].color = GRAY;
}


for (int i = 0; i < NUM_SHOOTS; i++)
{
shoot[i].rec.x = player.rec.x;
shoot[i].rec.y = player.rec.y + player.rec.height/4;
shoot[i].rec.width = 10;
shoot[i].rec.height = 5;
shoot[i].speed.x = 7;
shoot[i].speed.y = 0;
shoot[i].active = false;
shoot[i].color = MAROON;
}
}


void UpdateGame(void)
{
if (!gameOver)
{
if (IsKeyPressed('P')) pause = !pause;

if (!pause)
{
switch (wave)
{
case FIRST:
{
if (!smooth)
{
alpha += 0.02f;

if (alpha >= 1.0f) smooth = true;
}

if (smooth) alpha -= 0.02f;

if (enemiesKill == activeEnemies)
{
enemiesKill = 0;

for (int i = 0; i < activeEnemies; i++)
{
if (!enemy[i].active) enemy[i].active = true;
}

activeEnemies = SECOND_WAVE;
wave = SECOND;
smooth = false;
alpha = 0.0f;
}
} break;
case SECOND:
{
if (!smooth)
{
alpha += 0.02f;

if (alpha >= 1.0f) smooth = true;
}

if (smooth) alpha -= 0.02f;

if (enemiesKill == activeEnemies)
{
enemiesKill = 0;

for (int i = 0; i < activeEnemies; i++)
{
if (!enemy[i].active) enemy[i].active = true;
}

activeEnemies = THIRD_WAVE;
wave = THIRD;
smooth = false;
alpha = 0.0f;
}
} break;
case THIRD:
{
if (!smooth)
{
alpha += 0.02f;

if (alpha >= 1.0f) smooth = true;
}

if (smooth) alpha -= 0.02f;

if (enemiesKill == activeEnemies) victory = true;

} break;
default: break;
}


if (IsKeyDown(KEY_RIGHT)) player.rec.x += player.speed.x;
if (IsKeyDown(KEY_LEFT)) player.rec.x -= player.speed.x;
if (IsKeyDown(KEY_UP)) player.rec.y -= player.speed.y;
if (IsKeyDown(KEY_DOWN)) player.rec.y += player.speed.y;


for (int i = 0; i < activeEnemies; i++)
{
if (CheckCollisionRecs(player.rec, enemy[i].rec)) gameOver = true;
}


for (int i = 0; i < activeEnemies; i++)
{
if (enemy[i].active)
{
enemy[i].rec.x -= enemy[i].speed.x;

if (enemy[i].rec.x < 0)
{
enemy[i].rec.x = GetRandomValue(screenWidth, screenWidth + 1000);
enemy[i].rec.y = GetRandomValue(0, screenHeight - enemy[i].rec.height);
}
}
}


if (player.rec.x <= 0) player.rec.x = 0;
if (player.rec.x + player.rec.width >= screenWidth) player.rec.x = screenWidth - player.rec.width;
if (player.rec.y <= 0) player.rec.y = 0;
if (player.rec.y + player.rec.height >= screenHeight) player.rec.y = screenHeight - player.rec.height;


if (IsKeyDown(KEY_SPACE))
{
shootRate += 5;

for (int i = 0; i < NUM_SHOOTS; i++)
{
if (!shoot[i].active && shootRate%20 == 0)
{
shoot[i].rec.x = player.rec.x;
shoot[i].rec.y = player.rec.y + player.rec.height/4;
shoot[i].active = true;
break;
}
}
}


for (int i = 0; i < NUM_SHOOTS; i++)
{
if (shoot[i].active)
{

shoot[i].rec.x += shoot[i].speed.x;


for (int j = 0; j < activeEnemies; j++)
{
if (enemy[j].active)
{
if (CheckCollisionRecs(shoot[i].rec, enemy[j].rec))
{
shoot[i].active = false;
enemy[j].rec.x = GetRandomValue(screenWidth, screenWidth + 1000);
enemy[j].rec.y = GetRandomValue(0, screenHeight - enemy[j].rec.height);
shootRate = 0;
enemiesKill++;
score += 100;
}

if (shoot[i].rec.x + shoot[i].rec.width >= screenWidth)
{
shoot[i].active = false;
shootRate = 0;
}
}
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
DrawRectangleRec(player.rec, player.color);

if (wave == FIRST) DrawText("FIRST WAVE", screenWidth/2 - MeasureText("FIRST WAVE", 40)/2, screenHeight/2 - 40, 40, Fade(BLACK, alpha));
else if (wave == SECOND) DrawText("SECOND WAVE", screenWidth/2 - MeasureText("SECOND WAVE", 40)/2, screenHeight/2 - 40, 40, Fade(BLACK, alpha));
else if (wave == THIRD) DrawText("THIRD WAVE", screenWidth/2 - MeasureText("THIRD WAVE", 40)/2, screenHeight/2 - 40, 40, Fade(BLACK, alpha));

for (int i = 0; i < activeEnemies; i++)
{
if (enemy[i].active) DrawRectangleRec(enemy[i].rec, enemy[i].color);
}

for (int i = 0; i < NUM_SHOOTS; i++)
{
if (shoot[i].active) DrawRectangleRec(shoot[i].rec, shoot[i].color);
}

DrawText(TextFormat("%04i", score), 20, 20, 40, GRAY);

if (victory) DrawText("YOU WIN", screenWidth/2 - MeasureText("YOU WIN", 40)/2, screenHeight/2 - 40, 40, BLACK);

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
