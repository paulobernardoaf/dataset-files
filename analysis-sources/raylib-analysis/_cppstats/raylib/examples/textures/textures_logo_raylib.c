#include "raylib.h"
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
InitWindow(screenWidth, screenHeight, "raylib [textures] example - texture loading and drawing");
Texture2D texture = LoadTexture("resources/raylib_logo.png"); 
while (!WindowShouldClose()) 
{
BeginDrawing();
ClearBackground(RAYWHITE);
DrawTexture(texture, screenWidth/2 - texture.width/2, screenHeight/2 - texture.height/2, WHITE);
DrawText("this IS a texture!", 360, 370, 10, GRAY);
EndDrawing();
}
UnloadTexture(texture); 
CloseWindow(); 
return 0;
}