










#include "raylib.h"

int main(void)
{


const int screenWidth = 800;
const int screenHeight = 450;

InitWindow(screenWidth, screenHeight, "raylib [core] example - generate random values");

int framesCounter = 0; 

int randValue = GetRandomValue(-8, 5); 

SetTargetFPS(60); 



while (!WindowShouldClose()) 
{


framesCounter++;


if (((framesCounter/120)%2) == 1)
{
randValue = GetRandomValue(-8, 5);
framesCounter = 0;
}




BeginDrawing();

ClearBackground(RAYWHITE);

DrawText("Every 2 seconds a new random value is generated:", 130, 100, 20, MAROON);

DrawText(FormatText("%i", randValue), 360, 180, 80, LIGHTGRAY);

EndDrawing();

}



CloseWindow(); 


return 0;
}