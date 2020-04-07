#include "raylib.h"
#include <math.h> 
#define MAX_ENEMIES 10
typedef enum { TITLE, GAMEPLAY, ENDING } GameScreen;
int main()
{
const int screenWidth = 1280;
const int screenHeight = 720;
InitWindow(screenWidth, screenHeight, "Dr. Turtle & Mr. GAMERA");
InitAudioDevice(); 
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
Texture2D gframe = LoadTexture("resources/gframe.png");
Font font = LoadFont("resources/komika.png");
Sound eat = LoadSound("resources/eat.wav");
Sound die = LoadSound("resources/die.wav");
Sound growl = LoadSound("resources/gamera.wav");
Music music = LoadMusicStream("resources/speeding.ogg");
PlayMusicStream(music);
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
int enemyProb = GetRandomValue(0, 100);
if (enemyProb < 30) enemyType[i] = 0;
else if (enemyProb < 60) enemyType[i] = 1;
else if (enemyProb < 90) enemyType[i] = 2;
else enemyType[i] = 3;
enemyRail[i] = GetRandomValue(0, 4);
if (i > 0) while (enemyRail[i] == enemyRail[i - 1]) enemyRail[i] = GetRandomValue(0, 4);
enemyBounds[i] = (Rectangle){ screenWidth + 14, 120*enemyRail[i] + 90 + 14, 100, 100 };
enemyActive[i] = false;
}
int score = 0;
float distance = 0.0f;
int hiscore = 0;
float hidistance = 0.0f;
int foodBar = 0;
int framesCounter = 0;
unsigned char blue = 200;
float timeCounter = 0;
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
UpdateMusicStream(music); 
framesCounter++;
blue = 210 + 25 * sinf(timeCounter);
timeCounter += 0.01;
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
enemyActive[i] = false;
enemyType[i] = GetRandomValue(0, 3);
enemyRail[i] = GetRandomValue(0, 4);
if (i > 0) while (enemyRail[i] == enemyRail[i - 1]) enemyRail[i] = GetRandomValue(0, 4);
enemyBounds[i] = (Rectangle){ screenWidth + 14, 120*enemyRail[i] + 90 + 14, 100, 100 };
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
if (enemyType[i] == 0) score += 50;
else if (enemyType[i] == 1) score += 150;
else if (enemyType[i] == 2) score += 300;
foodBar += 15;
enemyActive[i] = false;
enemyType[i] = GetRandomValue(0, 3);
enemyRail[i] = GetRandomValue(0, 4);
if (i > 0) while (enemyRail[i] == enemyRail[i - 1]) enemyRail[i] = GetRandomValue(0, 4);
enemyBounds[i] = (Rectangle){ screenWidth + 14, 120*enemyRail[i] + 90 + 14, 100, 100 };
PlaySound(eat);
}
else
{
PlaySound(die);
currentScreen = ENDING;
framesCounter = 0;
if (score > hiscore) hiscore = score;
if (distance > hidistance) hidistance = distance;
}
}
else 
{
enemyActive[i] = false;
enemyType[i] = GetRandomValue(0, 3);
enemyRail[i] = GetRandomValue(0, 4);
if (i > 0) while (enemyRail[i] == enemyRail[i - 1]) enemyRail[i] = GetRandomValue(0, 4);
enemyBounds[i] = (Rectangle){ screenWidth + 14, 120*enemyRail[i] + 90 + 14, 100, 100 };
if (!gameraMode) foodBar += 80;
else foodBar += 25;
score += 10;
if (foodBar == 400)
{
gameraMode = true;
PlaySound(growl);
}
PlaySound(eat);
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
distance += 0.5f;
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
int enemyProb = GetRandomValue(0, 100);
if (enemyProb < 30) enemyType[i] = 0;
else if (enemyProb < 60) enemyType[i] = 1;
else if (enemyProb < 90) enemyType[i] = 2;
else enemyType[i] = 3;
enemyRail[i] = GetRandomValue(0, 4);
if (i > 0) while (enemyRail[i] == enemyRail[i - 1]) enemyRail[i] = GetRandomValue(0, 4);
enemyBounds[i] = (Rectangle){ screenWidth + 14, 120*enemyRail[i] + 90 + 14, 100, 100 };
enemyActive[i] = false;
}
enemySpeed = 10;
score = 0;
distance = 0.0;
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
DrawTexture(sea, seaScrolling, 0, (Color){ 16, 189, blue, 255});
DrawTexture(sea, screenWidth + seaScrolling, 0, (Color){ 16, 189, blue, 255});
}
else
{
DrawTexture(sea, seaScrolling, 0, (Color){ 255, 113, 66, 255});
DrawTexture(sea, screenWidth + seaScrolling, 0, (Color){ 255, 113, 66, 255});
}
switch (currentScreen)
{
case TITLE:
{
DrawTexture(title, screenWidth/2 - title.width/2, screenHeight/2 - title.height/2 - 80, WHITE);
if ((framesCounter/30) % 2) DrawTextEx(font, "PRESS ENTER", (Vector2){ screenWidth/2 - 150, 480 }, font.baseSize, 0, WHITE);
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
case 0: DrawTexture(shark, enemyBounds[i].x - 14, enemyBounds[i].y - 14, WHITE); break;
case 1: DrawTexture(orca, enemyBounds[i].x - 14, enemyBounds[i].y - 14, WHITE); break;
case 2: DrawTexture(swhale, enemyBounds[i].x - 14, enemyBounds[i].y - 14, WHITE); break;
case 3: DrawTexture(fish, enemyBounds[i].x - 14, enemyBounds[i].y - 14, WHITE); break;
default: break;
}
}
}
DrawRectangle(20, 20, 400, 40, Fade(GRAY, 0.4f));
DrawRectangle(20, 20, foodBar, 40, ORANGE);
DrawRectangleLines(20, 20, 400, 40, BLACK);
DrawTextEx(font, FormatText("SCORE: %04i", score), (Vector2){ screenWidth - 300, 20 }, font.baseSize, -2, ORANGE);
DrawTextEx(font, FormatText("DISTANCE: %04i", (int)distance), (Vector2){ 550, 20 }, font.baseSize, -2, ORANGE);
if (gameraMode)
{
DrawText("GAMERA MODE", 60, 22, 40, GRAY);
DrawTexture(gframe, 0, 0, Fade(WHITE, 0.5f));
}
} break;
case ENDING:
{
DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.4f));
DrawTextEx(font, "GAME OVER", (Vector2){ 300, 160 }, font.baseSize*3, -2, MAROON);
DrawTextEx(font, FormatText("SCORE: %04i", score), (Vector2){ 680, 350 }, font.baseSize, -2, GOLD);
DrawTextEx(font, FormatText("DISTANCE: %04i", (int)distance), (Vector2){ 290, 350 }, font.baseSize, -2, GOLD);
DrawTextEx(font, FormatText("HISCORE: %04i", hiscore), (Vector2){ 665, 400 }, font.baseSize, -2, ORANGE);
DrawTextEx(font, FormatText("HIDISTANCE: %04i", (int)hidistance), (Vector2){ 270, 400 }, font.baseSize, -2, ORANGE);
if ((framesCounter/30) % 2) DrawTextEx(font, "PRESS ENTER to REPLAY", (Vector2){ screenWidth/2 - 250, 520 }, font.baseSize, -2, LIGHTGRAY);
} break;
default: break;
}
EndDrawing();
}
UnloadTexture(sky);
UnloadTexture(mountains);
UnloadTexture(sea);
UnloadTexture(gframe);
UnloadTexture(title);
UnloadTexture(turtle);
UnloadTexture(shark);
UnloadTexture(orca);
UnloadTexture(swhale);
UnloadTexture(fish);
UnloadTexture(gamera);
UnloadFont(font);
UnloadSound(eat);
UnloadSound(die);
UnloadSound(growl);
UnloadMusicStream(music); 
CloseAudioDevice(); 
CloseWindow(); 
return 0;
}
