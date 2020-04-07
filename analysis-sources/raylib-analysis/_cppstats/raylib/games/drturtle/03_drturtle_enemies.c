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
Texture2D shark = LoadTexture("resources/shark.png");
Texture2D orca = LoadTexture("resources/orca.png");
Texture2D swhale = LoadTexture("resources/swhale.png");
Texture2D fish = LoadTexture("resources/fish.png");
int backScrolling = 0;
int seaScrolling = 0;
GameScreen currentScreen = TITLE;
int playerRail = 1;
Rectangle playerBounds = { 30 + 14, playerRail*120 + 90 + 14, 100, 100 };
bool gameraMode = false;
Rectangle enemyBounds[MAX_ENEMIES];
int enemyRail[MAX_ENEMIES];
int enemyType[MAX_ENEMIES];
bool enemyActive[MAX_ENEMIES];
float enemySpeed = 10;
for (int i = 0; i < MAX_ENEMIES; i++)
{
enemyType[i] = GetRandomValue(0, 3);
enemyRail[i] = GetRandomValue(0, 4);
enemyBounds[i] = (Rectangle){ screenWidth + 14, 120*enemyRail[i] + 90 + 14, 100, 100 };
enemyActive[i] = false;
}
int foodBar = 0;
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
seaScrolling -= (enemySpeed - 2);
if (seaScrolling <= -screenWidth) seaScrolling = 0; 
if (IsKeyPressed(KEY_DOWN)) playerRail++;
else if (IsKeyPressed(KEY_UP)) playerRail--;
if (playerRail > 4) playerRail = 4;
else if (playerRail < 0) playerRail = 0;
playerBounds = (Rectangle){ 30 + 14, playerRail*120 + 90 + 14, 100, 100 };
if (framesCounter > 40)
{
for (int i = 0; i < MAX_ENEMIES; i++)
{
if (enemyActive[i] == false)
{
enemyActive[i] = true;
i = MAX_ENEMIES;
}
}
framesCounter = 0;
}
for (int i = 0; i < MAX_ENEMIES; i++)
{
if (enemyActive[i])
{
enemyBounds[i].x -= enemySpeed;
}
if (enemyBounds[i].x <= 0 - 128)
{
enemyType[i] = GetRandomValue(0, 3);
enemyRail[i] = GetRandomValue(0, 4); 
enemyBounds[i] = (Rectangle){ screenWidth + 14, 120*enemyRail[i] + 90 + 14, 100, 100 };
enemyActive[i] = false;
}
}
if (!gameraMode) enemySpeed += 0.005;
for (int i = 0; i < MAX_ENEMIES; i++)
{
if (enemyActive[i])
{
if (CheckCollisionRecs(playerBounds, enemyBounds[i]))
{ 
if (enemyType[i] < 3) 
{
if (gameraMode)
{
foodBar += 15;
enemyType[i] = GetRandomValue(0, 3);
enemyRail[i] = GetRandomValue(0, 4);
enemyBounds[i] = (Rectangle){ screenWidth + 14, 120*enemyRail[i] + 90 + 14, 100, 100 };
enemyActive[i] = false;
}
else
{
currentScreen = ENDING;
framesCounter = 0;
}
}
else 
{
enemyActive[i] = false;
enemyType[i] = GetRandomValue(0, 3);
enemyRail[i] = GetRandomValue(0, 4);
enemyBounds[i] = (Rectangle){ screenWidth + 14, 120*enemyRail[i] + 90 + 14, 100, 100 };
if (!gameraMode) foodBar += 80;
else foodBar += 25;
if (foodBar == 400)
{
gameraMode = true;
}
}
}
}
}
if (gameraMode)
{
foodBar--;
if (foodBar <= 0) 
{
gameraMode = false;
enemySpeed -= 2;
if (enemySpeed < 10) enemySpeed = 10;
}
}
} break;
case ENDING:
{
if (IsKeyPressed(KEY_ENTER))
{
currentScreen = GAMEPLAY;
playerRail = 1;
playerBounds = (Rectangle){ 30 + 14, playerRail*120 + 90 + 14, 100, 100 };
gameraMode = false;
for (int i = 0; i < MAX_ENEMIES; i++)
{ 
enemyType[i] = GetRandomValue(0, 3);
enemyRail[i] = GetRandomValue(0, 4);
enemyBounds[i] = (Rectangle){ screenWidth + 14, 120*enemyRail[i] + 90 + 14, 100, 100 };
enemyActive[i] = false;
}
enemySpeed = 10;
foodBar = 0;
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
DrawTexture(title, screenWidth/2 - title.width/2, screenHeight/2 - title.height/2 - 80, WHITE);
if ((framesCounter/30) % 2) DrawText("PRESS ENTER", 480, 480, 40, BLACK);
} break;
case GAMEPLAY:
{
for (int i = 0; i < 5; i++) DrawRectangle(0, i*120 + 120, screenWidth, 110, Fade(SKYBLUE, 0.1f));
if (!gameraMode) DrawTexture(turtle, playerBounds.x - 14, playerBounds.y - 14, WHITE);
else DrawTexture(gamera, playerBounds.x - 64, playerBounds.y - 64, WHITE);
for (int i = 0; i < MAX_ENEMIES; i++)
{
if (enemyActive[i]) 
{
switch(enemyType[i])
{
case 0: DrawRectangleRec(enemyBounds[i], RED); break;
case 1: DrawRectangleRec(enemyBounds[i], RED); break;
case 2: DrawRectangleRec(enemyBounds[i], RED); break;
case 3: DrawRectangleRec(enemyBounds[i], GREEN); break;
default: break;
}
}
}
DrawRectangle(20, 20, 400, 40, Fade(GRAY, 0.4f));
DrawRectangle(20, 20, foodBar, 40, ORANGE);
DrawRectangleLines(20, 20, 400, 40, BLACK);
if (gameraMode)
{
DrawText("GAMERA MODE", 60, 22, 40, GRAY);
}
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
UnloadTexture(shark);
UnloadTexture(orca);
UnloadTexture(swhale);
UnloadTexture(fish);
CloseWindow(); 
return 0;
}