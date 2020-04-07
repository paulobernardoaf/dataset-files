#include "raylib.h"
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
InitWindow(screenWidth, screenHeight, "raylib [audio] example - music playing (streaming)");
InitAudioDevice(); 
Music music = LoadMusicStream("resources/guitar_noodling.ogg");
PlayMusicStream(music);
float timePlayed = 0.0f;
bool pause = false;
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
UpdateMusicStream(music); 
if (IsKeyPressed(KEY_SPACE))
{
StopMusicStream(music);
PlayMusicStream(music);
}
if (IsKeyPressed(KEY_P))
{
pause = !pause;
if (pause) PauseMusicStream(music);
else ResumeMusicStream(music);
}
timePlayed = GetMusicTimePlayed(music)/GetMusicTimeLength(music)*400;
if (timePlayed > 400) StopMusicStream(music);
BeginDrawing();
ClearBackground(RAYWHITE);
DrawText("MUSIC SHOULD BE PLAYING!", 255, 150, 20, LIGHTGRAY);
DrawRectangle(200, 200, 400, 12, LIGHTGRAY);
DrawRectangle(200, 200, (int)timePlayed, 12, MAROON);
DrawRectangleLines(200, 200, 400, 12, GRAY);
DrawText("PRESS SPACE TO RESTART MUSIC", 215, 250, 20, LIGHTGRAY);
DrawText("PRESS P TO PAUSE/RESUME MUSIC", 208, 280, 20, LIGHTGRAY);
EndDrawing();
}
UnloadMusicStream(music); 
CloseAudioDevice(); 
CloseWindow(); 
return 0;
}