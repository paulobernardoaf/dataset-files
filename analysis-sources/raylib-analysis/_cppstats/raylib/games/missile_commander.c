#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif
#define MAX_MISSILES 100
#define MAX_INTERCEPTORS 30
#define MAX_EXPLOSIONS 100
#define LAUNCHERS_AMOUNT 3 
#define BUILDINGS_AMOUNT 6 
#define LAUNCHER_SIZE 80
#define BUILDING_SIZE 60
#define EXPLOSION_RADIUS 40
#define MISSILE_SPEED 1
#define MISSILE_LAUNCH_FRAMES 80
#define INTERCEPTOR_SPEED 10
#define EXPLOSION_INCREASE_TIME 90 
#define EXPLOSION_TOTAL_TIME 210 
#define EXPLOSION_COLOR (Color){ 125, 125, 125, 125 }
typedef struct Missile {
Vector2 origin;
Vector2 position;
Vector2 objective;
Vector2 speed;
bool active;
} Missile;
typedef struct Interceptor {
Vector2 origin;
Vector2 position;
Vector2 objective;
Vector2 speed;
bool active;
} Interceptor;
typedef struct Explosion {
Vector2 position;
float radiusMultiplier;
int frame;
bool active;
} Explosion;
typedef struct Launcher {
Vector2 position;
bool active;
} Launcher;
typedef struct Building {
Vector2 position;
bool active;
} Building;
static int screenWidth = 800;
static int screenHeight = 450;
static int framesCounter = 0;
static bool gameOver = false;
static bool pause = false;
static int score = 0;
static Missile missile[MAX_MISSILES] = { 0 };
static Interceptor interceptor[MAX_INTERCEPTORS] = { 0 };
static Explosion explosion[MAX_EXPLOSIONS] = { 0 };
static Launcher launcher[LAUNCHERS_AMOUNT] = { 0 };
static Building building[BUILDINGS_AMOUNT] = { 0 };
static int explosionIndex = 0;
static void InitGame(void); 
static void UpdateGame(void); 
static void DrawGame(void); 
static void UnloadGame(void); 
static void UpdateDrawFrame(void); 
static void UpdateOutgoingFire();
static void UpdateIncomingFire();
int main(void)
{
InitWindow(screenWidth, screenHeight, "sample game: missile commander");
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
for (int i = 0; i < MAX_MISSILES; i++)
{
missile[i].origin = (Vector2){ 0, 0 };
missile[i].speed = (Vector2){ 0, 0 };
missile[i].position = (Vector2){ 0, 0 };
missile[i].active = false;
}
for (int i = 0; i < MAX_INTERCEPTORS; i++)
{
interceptor[i].origin = (Vector2){ 0, 0 };
interceptor[i].speed = (Vector2){ 0, 0 };
interceptor[i].position = (Vector2){ 0, 0 };
interceptor[i].active = false;
}
for (int i = 0; i < MAX_EXPLOSIONS; i++)
{
explosion[i].position = (Vector2){ 0, 0 };
explosion[i].frame = 0;
explosion[i].active = false;
}
int sparcing = screenWidth/(LAUNCHERS_AMOUNT + BUILDINGS_AMOUNT + 1);
launcher[0].position = (Vector2){ 1*sparcing, screenHeight - LAUNCHER_SIZE/2 };
building[0].position = (Vector2){ 2*sparcing, screenHeight - BUILDING_SIZE/2 };
building[1].position = (Vector2){ 3*sparcing, screenHeight - BUILDING_SIZE/2 };
building[2].position = (Vector2){ 4*sparcing, screenHeight - BUILDING_SIZE/2 };
launcher[1].position = (Vector2){ 5*sparcing, screenHeight - LAUNCHER_SIZE/2 };
building[3].position = (Vector2){ 6*sparcing, screenHeight - BUILDING_SIZE/2 };
building[4].position = (Vector2){ 7*sparcing, screenHeight - BUILDING_SIZE/2 };
building[5].position = (Vector2){ 8*sparcing, screenHeight - BUILDING_SIZE/2 };
launcher[2].position = (Vector2){ 9*sparcing, screenHeight - LAUNCHER_SIZE/2 };
for (int i = 0; i < LAUNCHERS_AMOUNT; i++) launcher[i].active = true;
for (int i = 0; i < BUILDINGS_AMOUNT; i++) building[i].active = true;
score = 0;
}
void UpdateGame(void)
{
if (!gameOver)
{
if (IsKeyPressed('P')) pause = !pause;
if (!pause)
{
framesCounter++;
static 
float distance;
for (int i = 0; i < MAX_INTERCEPTORS; i++)
{
if (interceptor[i].active)
{
interceptor[i].position.x += interceptor[i].speed.x;
interceptor[i].position.y += interceptor[i].speed.y;
distance = sqrt( pow(interceptor[i].position.x - interceptor[i].objective.x, 2) +
pow(interceptor[i].position.y - interceptor[i].objective.y, 2));
if (distance < INTERCEPTOR_SPEED)
{
interceptor[i].active = false;
explosion[explosionIndex].position = interceptor[i].position;
explosion[explosionIndex].active = true;
explosion[explosionIndex].frame = 0;
explosionIndex++;
if (explosionIndex == MAX_EXPLOSIONS) explosionIndex = 0;
break;
}
}
}
for (int i = 0; i < MAX_MISSILES; i++)
{
if (missile[i].active)
{
missile[i].position.x += missile[i].speed.x;
missile[i].position.y += missile[i].speed.y;
if (missile[i].position.y > screenHeight) missile[i].active = false;
else
{
for (int j = 0; j < LAUNCHERS_AMOUNT; j++)
{
if (launcher[j].active)
{
if (CheckCollisionPointRec(missile[i].position, (Rectangle){ launcher[j].position.x - LAUNCHER_SIZE/2, launcher[j].position.y - LAUNCHER_SIZE/2,
LAUNCHER_SIZE, LAUNCHER_SIZE }))
{
missile[i].active = false;
launcher[j].active = false;
explosion[explosionIndex].position = missile[i].position;
explosion[explosionIndex].active = true;
explosion[explosionIndex].frame = 0;
explosionIndex++;
if (explosionIndex == MAX_EXPLOSIONS) explosionIndex = 0;
break;
}
}
}
for (int j = 0; j < BUILDINGS_AMOUNT; j++)
{
if (building[j].active)
{
if (CheckCollisionPointRec(missile[i].position, (Rectangle){ building[j].position.x - BUILDING_SIZE/2, building[j].position.y - BUILDING_SIZE/2,
BUILDING_SIZE, BUILDING_SIZE }))
{
missile[i].active = false;
building[j].active = false;
explosion[explosionIndex].position = missile[i].position;
explosion[explosionIndex].active = true;
explosion[explosionIndex].frame = 0;
explosionIndex++;
if (explosionIndex == MAX_EXPLOSIONS) explosionIndex = 0;
break;
}
}
}
for (int j = 0; j < MAX_EXPLOSIONS; j++)
{
if (explosion[j].active)
{
if (CheckCollisionPointCircle(missile[i].position, explosion[j].position, EXPLOSION_RADIUS*explosion[j].radiusMultiplier))
{
missile[i].active = false;
score += 100;
explosion[explosionIndex].position = missile[i].position;
explosion[explosionIndex].active = true;
explosion[explosionIndex].frame = 0;
explosionIndex++;
if (explosionIndex == MAX_EXPLOSIONS) explosionIndex = 0;
break;
}
}
}
}
}
}
for (int i = 0; i < MAX_EXPLOSIONS; i++)
{
if (explosion[i].active)
{
explosion[i].frame++;
if (explosion[i].frame <= EXPLOSION_INCREASE_TIME) explosion[i].radiusMultiplier = explosion[i].frame/(float)EXPLOSION_INCREASE_TIME;
else if (explosion[i].frame <= EXPLOSION_TOTAL_TIME) explosion[i].radiusMultiplier = 1 - (explosion[i].frame - (float)EXPLOSION_INCREASE_TIME)/(float)EXPLOSION_TOTAL_TIME;
else
{
explosion[i].frame = 0;
explosion[i].active = false;
}
}
}
UpdateOutgoingFire();
UpdateIncomingFire();
int checker = 0;
for (int i = 0; i < LAUNCHERS_AMOUNT; i++)
{
if (!launcher[i].active) checker++;
if (checker == LAUNCHERS_AMOUNT) gameOver = true;
}
checker = 0;
for (int i = 0; i < BUILDINGS_AMOUNT; i++)
{
if (!building[i].active) checker++;
if (checker == BUILDINGS_AMOUNT) gameOver = true;
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
for (int i = 0; i < MAX_MISSILES; i++)
{
if (missile[i].active)
{
DrawLine(missile[i].origin.x, missile[i].origin.y, missile[i].position.x, missile[i].position.y, RED);
if (framesCounter % 16 < 8) DrawCircle(missile[i].position.x, missile[i].position.y, 3, YELLOW);
}
}
for (int i = 0; i < MAX_INTERCEPTORS; i++)
{
if (interceptor[i].active)
{
DrawLine(interceptor[i].origin.x, interceptor[i].origin.y, interceptor[i].position.x, interceptor[i].position.y, GREEN);
if (framesCounter % 16 < 8) DrawCircle(interceptor[i].position.x, interceptor[i].position.y, 3, BLUE);
}
}
for (int i = 0; i < MAX_EXPLOSIONS; i++)
{
if (explosion[i].active) DrawCircle(explosion[i].position.x, explosion[i].position.y, EXPLOSION_RADIUS*explosion[i].radiusMultiplier, EXPLOSION_COLOR);
}
for (int i = 0; i < LAUNCHERS_AMOUNT; i++)
{
if (launcher[i].active) DrawRectangle(launcher[i].position.x - LAUNCHER_SIZE/2, launcher[i].position.y - LAUNCHER_SIZE/2, LAUNCHER_SIZE, LAUNCHER_SIZE, GRAY);
}
for (int i = 0; i < BUILDINGS_AMOUNT; i++)
{
if (building[i].active) DrawRectangle(building[i].position.x - BUILDING_SIZE/2, building[i].position.y - BUILDING_SIZE/2, BUILDING_SIZE, BUILDING_SIZE, LIGHTGRAY);
}
DrawText(TextFormat("SCORE %4i", score), 20, 20, 40, LIGHTGRAY);
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
static void UpdateOutgoingFire()
{
static int interceptorNumber = 0;
int launcherShooting = 0;
if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) launcherShooting = 1;
if (IsMouseButtonPressed(MOUSE_MIDDLE_BUTTON)) launcherShooting = 2;
if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) launcherShooting = 3;
if (launcherShooting > 0 && launcher[launcherShooting - 1].active)
{
float module;
float sideX;
float sideY;
interceptor[interceptorNumber].active = true;
interceptor[interceptorNumber].origin = launcher[launcherShooting - 1].position;
interceptor[interceptorNumber].position = interceptor[interceptorNumber].origin;
interceptor[interceptorNumber].objective = GetMousePosition();
module = sqrt( pow(interceptor[interceptorNumber].objective.x - interceptor[interceptorNumber].origin.x, 2) +
pow(interceptor[interceptorNumber].objective.y - interceptor[interceptorNumber].origin.y, 2));
sideX = (interceptor[interceptorNumber].objective.x - interceptor[interceptorNumber].origin.x)*INTERCEPTOR_SPEED/module;
sideY = (interceptor[interceptorNumber].objective.y - interceptor[interceptorNumber].origin.y)*INTERCEPTOR_SPEED/module;
interceptor[interceptorNumber].speed = (Vector2){ sideX, sideY };
interceptorNumber++;
if (interceptorNumber == MAX_INTERCEPTORS) interceptorNumber = 0;
}
}
static void UpdateIncomingFire()
{
static int missileIndex = 0;
if (framesCounter%MISSILE_LAUNCH_FRAMES == 0)
{
float module;
float sideX;
float sideY;
missile[missileIndex].active = true;
missile[missileIndex].origin = (Vector2){ GetRandomValue(20, screenWidth - 20), -10 };
missile[missileIndex].position = missile[missileIndex].origin;
missile[missileIndex].objective = (Vector2){ GetRandomValue(20, screenWidth - 20), screenHeight + 10 };
module = sqrt( pow(missile[missileIndex].objective.x - missile[missileIndex].origin.x, 2) +
pow(missile[missileIndex].objective.y - missile[missileIndex].origin.y, 2));
sideX = (missile[missileIndex].objective.x - missile[missileIndex].origin.x)*MISSILE_SPEED/module;
sideY = (missile[missileIndex].objective.y - missile[missileIndex].origin.y)*MISSILE_SPEED/module;
missile[missileIndex].speed = (Vector2){ sideX, sideY };
missileIndex++;
if (missileIndex == MAX_MISSILES) missileIndex = 0;
}
}
