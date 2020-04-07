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
int backScrolling = 0;
int seaScrolling = 0;
GameScreen currentScreen = TITLE;
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
switch (currentScreen)
{
case TITLE:
{
seaScrolling -= 2;
if (seaScrolling <= -screenWidth) seaScrolling = 0; 
if (IsKeyPressed(KEY_ENTER))
{
currentScreen = GAMEPLAY;
}
} break;
case GAMEPLAY:
{
backScrolling--;
if (backScrolling <= -screenWidth) backScrolling = 0; 
seaScrolling -= 8;
if (seaScrolling <= -screenWidth) seaScrolling = 0;
if (IsKeyPressed(KEY_ENTER))
{
currentScreen = ENDING;
}
} break;
case ENDING:
{
if (IsKeyPressed(KEY_ENTER))
{
currentScreen = TITLE;
}
} break;
default: break;
}
BeginDrawing();
ClearBackground(RAYWHITE);
DrawTexture(sky, 0, 0, WHITE);
DrawTexture(mountains, backScrolling, 0, WHITE);
DrawTexture(mountains, screenWidth + backScrolling, 0, WHITE);
DrawTexture(sea, seaScrolling, 0, BLUE);
DrawTexture(sea, screenWidth + seaScrolling, 0, BLUE);
switch (currentScreen)
{
case TITLE:
{
DrawText("PRESS ENTER", 450, 420, 40, BLACK);
} break;
case GAMEPLAY:
{
DrawText("GAMEPLAY SCREEN", 20, 20, 40, MAROON);
} break;
case ENDING:
{
DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.4f));
DrawText("ENDING SCREEN", 20, 20, 40, DARKBLUE);
} break;
default: break;
}
EndDrawing();
}
UnloadTexture(sky);
UnloadTexture(mountains);
UnloadTexture(sea);
CloseWindow(); 
return 0;
}