#include "raylib.h"
#define NUM_FRAMES 8
#define NUM_LINES 6
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
InitWindow(screenWidth, screenHeight, "raylib [textures] example - sprite explosion");
InitAudioDevice();
Sound fxBoom = LoadSound("resources/boom.wav");
Texture2D explosion = LoadTexture("resources/explosion.png");
int frameWidth = explosion.width/NUM_FRAMES; 
int frameHeight = explosion.height/NUM_LINES; 
int currentFrame = 0;
int currentLine = 0;
Rectangle frameRec = { 0, 0, frameWidth, frameHeight };
Vector2 position = { 0.0f, 0.0f };
bool active = false;
int framesCounter = 0;
SetTargetFPS(120);
while (!WindowShouldClose()) 
{
if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !active)
{
position = GetMousePosition();
active = true;
position.x -= frameWidth/2;
position.y -= frameHeight/2;
PlaySound(fxBoom);
}
if (active)
{
framesCounter++;
if (framesCounter > 2)
{
currentFrame++;
if (currentFrame >= NUM_FRAMES)
{
currentFrame = 0;
currentLine++;
if (currentLine >= NUM_LINES)
{
currentLine = 0;
active = false;
}
}
framesCounter = 0;
}
}
frameRec.x = frameWidth*currentFrame;
frameRec.y = frameHeight*currentLine;
BeginDrawing();
ClearBackground(RAYWHITE);
if (active) DrawTextureRec(explosion, frameRec, position, WHITE);
EndDrawing();
}
UnloadTexture(explosion); 
UnloadSound(fxBoom); 
CloseAudioDevice();
CloseWindow(); 
return 0;
}