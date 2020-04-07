#include "raylib.h"
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
InitWindow(screenWidth, screenHeight, "raylib [textures] examples - texture source and destination rectangles");
Texture2D scarfy = LoadTexture("resources/scarfy.png"); 
int frameWidth = scarfy.width/6;
int frameHeight = scarfy.height;
Rectangle sourceRec = { 0.0f, 0.0f, frameWidth, frameHeight };
Rectangle destRec = { screenWidth/2, screenHeight/2, frameWidth*2, frameHeight*2 };
Vector2 origin = { frameWidth, frameHeight };
int rotation = 0;
SetTargetFPS(60);
while (!WindowShouldClose()) 
{
rotation++;
BeginDrawing();
ClearBackground(RAYWHITE);
DrawTexturePro(scarfy, sourceRec, destRec, origin, (float)rotation, WHITE);
DrawLine((int)destRec.x, 0, (int)destRec.x, screenHeight, GRAY);
DrawLine(0, (int)destRec.y, screenWidth, (int)destRec.y, GRAY);
DrawText("(c) Scarfy sprite by Eiden Marsal", screenWidth - 200, screenHeight - 20, 10, GRAY);
EndDrawing();
}
UnloadTexture(scarfy); 
CloseWindow(); 
return 0;
}