#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif
#define PLAYER_MAX_LIFE 5
#define LINES_OF_BRICKS 5
#define BRICKS_PER_LINE 20
typedef enum GameScreen { LOGO, TITLE, GAMEPLAY, ENDING } GameScreen;
typedef struct Player {
Vector2 position;
Vector2 size;
int life;
} Player;
typedef struct Ball {
Vector2 position;
Vector2 speed;
int radius;
bool active;
} Ball;
typedef struct Brick {
Vector2 position;
bool active;
} Brick;
static const int screenWidth = 800;
static const int screenHeight = 450;
static bool gameOver = false;
static bool pause = false;
static Player player = { 0 };
static Ball ball = { 0 };
static Brick brick[LINES_OF_BRICKS][BRICKS_PER_LINE] = { 0 };
static Vector2 brickSize = { 0 };
static void InitGame(void); 
static void UpdateGame(void); 
static void DrawGame(void); 
static void UnloadGame(void); 
static void UpdateDrawFrame(void); 
int main(void)
{
InitWindow(screenWidth, screenHeight, "sample game: arkanoid");
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
brickSize = (Vector2){ GetScreenWidth()/BRICKS_PER_LINE, 40 };
player.position = (Vector2){ screenWidth/2, screenHeight*7/8 };
player.size = (Vector2){ screenWidth/10, 20 };
player.life = PLAYER_MAX_LIFE;
ball.position = (Vector2){ screenWidth/2, screenHeight*7/8 - 30 };
ball.speed = (Vector2){ 0, 0 };
ball.radius = 7;
ball.active = false;
int initialDownPosition = 50;
for (int i = 0; i < LINES_OF_BRICKS; i++)
{
for (int j = 0; j < BRICKS_PER_LINE; j++)
{
brick[i][j].position = (Vector2){ j*brickSize.x + brickSize.x/2, i*brickSize.y + initialDownPosition };
brick[i][j].active = true;
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
if (IsKeyDown(KEY_LEFT)) player.position.x -= 5;
if ((player.position.x - player.size.x/2) <= 0) player.position.x = player.size.x/2;
if (IsKeyDown(KEY_RIGHT)) player.position.x += 5;
if ((player.position.x + player.size.x/2) >= screenWidth) player.position.x = screenWidth - player.size.x/2;
if (!ball.active)
{
if (IsKeyPressed(KEY_SPACE))
{
ball.active = true;
ball.speed = (Vector2){ 0, -5 };
}
}
if (ball.active)
{
ball.position.x += ball.speed.x;
ball.position.y += ball.speed.y;
}
else
{
ball.position = (Vector2){ player.position.x, screenHeight*7/8 - 30 };
}
if (((ball.position.x + ball.radius) >= screenWidth) || ((ball.position.x - ball.radius) <= 0)) ball.speed.x *= -1;
if ((ball.position.y - ball.radius) <= 0) ball.speed.y *= -1;
if ((ball.position.y + ball.radius) >= screenHeight)
{
ball.speed = (Vector2){ 0, 0 };
ball.active = false;
player.life--;
}
if (CheckCollisionCircleRec(ball.position, ball.radius,
(Rectangle){ player.position.x - player.size.x/2, player.position.y - player.size.y/2, player.size.x, player.size.y}))
{
if (ball.speed.y > 0)
{
ball.speed.y *= -1;
ball.speed.x = (ball.position.x - player.position.x)/(player.size.x/2)*5;
}
}
for (int i = 0; i < LINES_OF_BRICKS; i++)
{
for (int j = 0; j < BRICKS_PER_LINE; j++)
{
if (brick[i][j].active)
{
if (((ball.position.y - ball.radius) <= (brick[i][j].position.y + brickSize.y/2)) &&
((ball.position.y - ball.radius) > (brick[i][j].position.y + brickSize.y/2 + ball.speed.y)) &&
((fabs(ball.position.x - brick[i][j].position.x)) < (brickSize.x/2 + ball.radius*2/3)) && (ball.speed.y < 0))
{
brick[i][j].active = false;
ball.speed.y *= -1;
}
else if (((ball.position.y + ball.radius) >= (brick[i][j].position.y - brickSize.y/2)) &&
((ball.position.y + ball.radius) < (brick[i][j].position.y - brickSize.y/2 + ball.speed.y)) &&
((fabs(ball.position.x - brick[i][j].position.x)) < (brickSize.x/2 + ball.radius*2/3)) && (ball.speed.y > 0))
{
brick[i][j].active = false;
ball.speed.y *= -1;
}
else if (((ball.position.x + ball.radius) >= (brick[i][j].position.x - brickSize.x/2)) &&
((ball.position.x + ball.radius) < (brick[i][j].position.x - brickSize.x/2 + ball.speed.x)) &&
((fabs(ball.position.y - brick[i][j].position.y)) < (brickSize.y/2 + ball.radius*2/3)) && (ball.speed.x > 0))
{
brick[i][j].active = false;
ball.speed.x *= -1;
}
else if (((ball.position.x - ball.radius) <= (brick[i][j].position.x + brickSize.x/2)) &&
((ball.position.x - ball.radius) > (brick[i][j].position.x + brickSize.x/2 + ball.speed.x)) &&
((fabs(ball.position.y - brick[i][j].position.y)) < (brickSize.y/2 + ball.radius*2/3)) && (ball.speed.x < 0))
{
brick[i][j].active = false;
ball.speed.x *= -1;
}
}
}
}
if (player.life <= 0) gameOver = true;
else
{
gameOver = true;
for (int i = 0; i < LINES_OF_BRICKS; i++)
{
for (int j = 0; j < BRICKS_PER_LINE; j++)
{
if (brick[i][j].active) gameOver = false;
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
DrawRectangle(player.position.x - player.size.x/2, player.position.y - player.size.y/2, player.size.x, player.size.y, BLACK);
for (int i = 0; i < player.life; i++) DrawRectangle(20 + 40*i, screenHeight - 30, 35, 10, LIGHTGRAY);
DrawCircleV(ball.position, ball.radius, MAROON);
for (int i = 0; i < LINES_OF_BRICKS; i++)
{
for (int j = 0; j < BRICKS_PER_LINE; j++)
{
if (brick[i][j].active)
{
if ((i + j) % 2 == 0) DrawRectangle(brick[i][j].position.x - brickSize.x/2, brick[i][j].position.y - brickSize.y/2, brickSize.x, brickSize.y, GRAY);
else DrawRectangle(brick[i][j].position.x - brickSize.x/2, brick[i][j].position.y - brickSize.y/2, brickSize.x, brickSize.y, DARKGRAY);
}
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
