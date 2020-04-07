#include "raylib.h"
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
InitWindow(screenWidth, screenHeight, "raylib [textures] example - image loading");
Image image = LoadImage("resources/raylib_logo.png"); 
Texture2D texture = LoadTextureFromImage(image); 
UnloadImage(image); 
while (!WindowShouldClose()) 
{
BeginDrawing();
ClearBackground(RAYWHITE);
DrawTexture(texture, screenWidth/2 - texture.width/2, screenHeight/2 - texture.height/2, WHITE);
DrawText("this IS a texture loaded from an image!", 300, 370, 10, GRAY);
EndDrawing();
}
UnloadTexture(texture); 
CloseWindow(); 
return 0;
}