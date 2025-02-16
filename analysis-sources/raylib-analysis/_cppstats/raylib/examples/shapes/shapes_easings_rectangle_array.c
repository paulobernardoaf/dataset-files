#include "raylib.h"
#include "easings.h" 
#define RECS_WIDTH 50
#define RECS_HEIGHT 50
#define MAX_RECS_X 800/RECS_WIDTH
#define MAX_RECS_Y 450/RECS_HEIGHT
#define PLAY_TIME_IN_FRAMES 240 
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
InitWindow(screenWidth, screenHeight, "raylib [shapes] example - easings rectangle array");
Rectangle recs[MAX_RECS_X*MAX_RECS_Y] = { 0 };
for (int y = 0; y < MAX_RECS_Y; y++)
{
for (int x = 0; x < MAX_RECS_X; x++)
{
recs[y*MAX_RECS_X + x].x = RECS_WIDTH/2 + RECS_WIDTH*x;
recs[y*MAX_RECS_X + x].y = RECS_HEIGHT/2 + RECS_HEIGHT*y;
recs[y*MAX_RECS_X + x].width = RECS_WIDTH;
recs[y*MAX_RECS_X + x].height = RECS_HEIGHT;
}
}
float rotation = 0.0f;
int framesCounter = 0;
int state = 0; 
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
if (state == 0)
{
framesCounter++;
for (int i = 0; i < MAX_RECS_X*MAX_RECS_Y; i++)
{
recs[i].height = EaseCircOut(framesCounter, RECS_HEIGHT, -RECS_HEIGHT, PLAY_TIME_IN_FRAMES);
recs[i].width = EaseCircOut(framesCounter, RECS_WIDTH, -RECS_WIDTH, PLAY_TIME_IN_FRAMES);
if (recs[i].height < 0) recs[i].height = 0;
if (recs[i].width < 0) recs[i].width = 0;
if ((recs[i].height == 0) && (recs[i].width == 0)) state = 1; 
rotation = EaseLinearIn(framesCounter, 0.0f, 360.0f, PLAY_TIME_IN_FRAMES);
}
}
else if ((state == 1) && IsKeyPressed(KEY_SPACE))
{
framesCounter = 0;
for (int i = 0; i < MAX_RECS_X*MAX_RECS_Y; i++)
{
recs[i].height = RECS_HEIGHT;
recs[i].width = RECS_WIDTH;
}
state = 0;
}
BeginDrawing();
ClearBackground(RAYWHITE);
if (state == 0)
{
for (int i = 0; i < MAX_RECS_X*MAX_RECS_Y; i++)
{
DrawRectanglePro(recs[i], (Vector2){ recs[i].width/2, recs[i].height/2 }, rotation, RED);
}
}
else if (state == 1) DrawText("PRESS [SPACE] TO PLAY AGAIN!", 240, 200, 20, GRAY);
EndDrawing();
}
CloseWindow(); 
return 0;
}