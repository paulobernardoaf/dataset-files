#include "raylib.h"
typedef enum {
STORAGE_POSITION_SCORE = 0,
STORAGE_POSITION_HISCORE = 1
} StorageData;
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
InitWindow(screenWidth, screenHeight, "raylib [core] example - storage save/load values");
int score = 0;
int hiscore = 0;
int framesCounter = 0;
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
if (IsKeyPressed(KEY_R))
{
score = GetRandomValue(1000, 2000);
hiscore = GetRandomValue(2000, 4000);
}
if (IsKeyPressed(KEY_ENTER))
{
SaveStorageValue(STORAGE_POSITION_SCORE, score);
SaveStorageValue(STORAGE_POSITION_HISCORE, hiscore);
}
else if (IsKeyPressed(KEY_SPACE))
{
score = LoadStorageValue(STORAGE_POSITION_SCORE);
hiscore = LoadStorageValue(STORAGE_POSITION_HISCORE);
}
framesCounter++;
BeginDrawing();
ClearBackground(RAYWHITE);
DrawText(FormatText("SCORE: %i", score), 280, 130, 40, MAROON);
DrawText(FormatText("HI-SCORE: %i", hiscore), 210, 200, 50, BLACK);
DrawText(FormatText("frames: %i", framesCounter), 10, 10, 20, LIME);
DrawText("Press R to generate random numbers", 220, 40, 20, LIGHTGRAY);
DrawText("Press ENTER to SAVE values", 250, 310, 20, LIGHTGRAY);
DrawText("Press SPACE to LOAD values", 252, 350, 20, LIGHTGRAY);
EndDrawing();
}
CloseWindow(); 
return 0;
}