#include "raylib.h"
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
InitWindow(screenWidth, screenHeight, "raylib [text] example - text formatting");
int score = 100020;
int hiscore = 200450;
int lives = 5;
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
BeginDrawing();
ClearBackground(RAYWHITE);
DrawText(FormatText("Score: %08i", score), 200, 80, 20, RED);
DrawText(FormatText("HiScore: %08i", hiscore), 200, 120, 20, GREEN);
DrawText(FormatText("Lives: %02i", lives), 200, 160, 40, BLUE);
DrawText(FormatText("Elapsed Time: %02.02f ms", GetFrameTime()*1000), 200, 220, 20, BLACK);
EndDrawing();
}
CloseWindow(); 
return 0;
}