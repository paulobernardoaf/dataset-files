#include "raylib.h"
#define MAX_ENEMIES 10
typedef enum { TITLE, GAMEPLAY, ENDING } GameScreen;
int main()
{
const int screenWidth = 1280;
const int screenHeight = 720;
InitWindow(screenWidth, screenHeight, "Dr. Turtle & Mr. GAMERA");
Texture2D sky = LoadTexture("resources/sky.png");
Texture2D mountains = LoadTexture("resources/mountains.png");
Texture2D sea = LoadTexture("resources/sea.png");
Texture2D title = LoadTexture("resources/title.png");
Texture2D turtle = LoadTexture("resources/turtle.png");
Texture2D gamera = LoadTexture("resources/gamera.png");
int backScrolling = 0;
int seaScrolling = 0;
GameScreen currentScreen = TITLE;
int playerRail = 1;
Rectangle playerBounds = { 30 + 14, playerRail*120 + 90 + 14, 100, 100 };
bool gameraMode = false;
int framesCounter = 0;
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
framesCounter++;
switch (currentScreen)
{
case TITLE:
{
seaScrolling -= 2;
if (seaScrolling <= -screenWidth) seaScrolling = 0; 
if (IsKeyPressed(KEY_ENTER))
{
currentScreen = GAMEPLAY;
framesCounter = 0;
}
} break;
case GAMEPLAY:
{
backScrolling--;
if (backScrolling <= -screenWidth) backScrolling = 0; 
seaScrolling -= 8;
if (seaScrolling <= -screenWidth) seaScrolling = 0; 
if (IsKeyPressed(KEY_DOWN)) playerRail++;
else if (IsKeyPressed(KEY_UP)) playerRail--;
if (playerRail > 4) playerRail = 4;
else if (playerRail < 0) playerRail = 0;
playerBounds = (Rectangle){ 30 + 14, playerRail*120 + 90 + 14, 100, 100 };
if (IsKeyPressed(KEY_SPACE)) gameraMode = !gameraMode;
if (IsKeyPressed(KEY_ENTER)) currentScreen = ENDING;
} break;
case ENDING:
{
if (IsKeyPressed(KEY_ENTER))
{
currentScreen = GAMEPLAY;
playerRail = 1;
playerBounds = (Rectangle){ 30 + 14, playerRail*120 + 90 + 14, 100, 100 };
gameraMode = false;
framesCounter = 0;
}
} break;
default: break;
}
BeginDrawing();
ClearBackground(RAYWHITE);
DrawTexture(sky, 0, 0, WHITE);
DrawTexture(mountains, backScrolling, 0, WHITE);
DrawTexture(mountains, screenWidth + backScrolling, 0, WHITE);
if (!gameraMode)
{
DrawTexture(sea, seaScrolling, 0, BLUE);
DrawTexture(sea, screenWidth + seaScrolling, 0, BLUE);
}
else
{
DrawTexture(sea, seaScrolling, 0, RED);
DrawTexture(sea, screenWidth + seaScrolling, 0, RED);
}
switch (currentScreen)
{
case TITLE:
{
DrawRectangle(380, 140, 500, 300, GRAY);
if ((framesCounter/30) % 2) DrawText("PRESS ENTER", 480, 480, 40, BLACK);
} break;
case GAMEPLAY:
{ 
if (!gameraMode) DrawRectangleRec(playerBounds, GREEN);
else DrawRectangleRec(playerBounds, ORANGE);
} break;
case ENDING:
{
DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.4f));
DrawText("GAME OVER", 300, 200, 100, MAROON);
if ((framesCounter/30) % 2) DrawText("PRESS ENTER to REPLAY", 400, 420, 30, LIGHTGRAY);
} break;
default: break;
}
EndDrawing();
}
UnloadTexture(sky);
UnloadTexture(mountains);
UnloadTexture(sea);
UnloadTexture(title);
UnloadTexture(turtle);
UnloadTexture(gamera);
CloseWindow(); 
return 0;
}