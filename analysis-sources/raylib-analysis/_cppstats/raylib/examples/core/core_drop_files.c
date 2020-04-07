#include "raylib.h"
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
InitWindow(screenWidth, screenHeight, "raylib [core] example - drop files");
int count = 0;
char **droppedFiles = { 0 };
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
if (IsFileDropped())
{
droppedFiles = GetDroppedFiles(&count);
}
BeginDrawing();
ClearBackground(RAYWHITE);
if (count == 0) DrawText("Drop your files to this window!", 100, 40, 20, DARKGRAY);
else
{
DrawText("Dropped files:", 100, 40, 20, DARKGRAY);
for (int i = 0; i < count; i++)
{
if (i%2 == 0) DrawRectangle(0, 85 + 40*i, screenWidth, 40, Fade(LIGHTGRAY, 0.5f));
else DrawRectangle(0, 85 + 40*i, screenWidth, 40, Fade(LIGHTGRAY, 0.3f));
DrawText(droppedFiles[i], 120, 100 + 40*i, 10, GRAY);
}
DrawText("Drop new files...", 100, 110 + 40*count, 20, DARKGRAY);
}
EndDrawing();
}
ClearDroppedFiles(); 
CloseWindow(); 
return 0;
}