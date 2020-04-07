




















#include "raylib.h"

#define MAX_ENEMIES 10

typedef enum { TITLE, GAMEPLAY, ENDING } GameScreen;

int main()
{


const int screenWidth = 1280;
const int screenHeight = 720;


InitWindow(screenWidth, screenHeight, "Dr. Turtle & Mr. GAMERA");


GameScreen currentScreen = TITLE;

SetTargetFPS(60); 



while (!WindowShouldClose()) 
{




switch (currentScreen)
{
case TITLE:
{

if (IsKeyPressed(KEY_ENTER))
{
currentScreen = GAMEPLAY;
}

} break;
case GAMEPLAY:
{

if (IsKeyPressed(KEY_ENTER))
{
currentScreen = ENDING;
}

} break;
case ENDING:
{

if (IsKeyPressed(KEY_ENTER))
{
currentScreen = TITLE;
}

} break;
default: break;
}




BeginDrawing();

ClearBackground(RAYWHITE);

switch (currentScreen)
{
case TITLE:
{

DrawRectangle(0, 0, screenWidth, screenHeight, GREEN);
DrawText("TITLE SCREEN", 20, 20, 40, DARKGREEN);

} break;
case GAMEPLAY:
{

DrawRectangle(0, 0, screenWidth, screenHeight, RED);
DrawText("GAMEPLAY SCREEN", 20, 20, 40, MAROON);

} break;
case ENDING:
{

DrawRectangle(0, 0, screenWidth, screenHeight, BLUE);
DrawText("ENDING SCREEN", 20, 20, 40, DARKBLUE);

} break;
default: break;
}

EndDrawing();

}



CloseWindow(); 


return 0;
}