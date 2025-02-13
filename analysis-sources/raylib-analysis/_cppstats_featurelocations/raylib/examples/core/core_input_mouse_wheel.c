










#include "raylib.h"

int main(void)
{


const int screenWidth = 800;
const int screenHeight = 450;

InitWindow(screenWidth, screenHeight, "raylib [core] example - input mouse wheel");

int boxPositionY = screenHeight/2 - 40;
int scrollSpeed = 4; 

SetTargetFPS(60); 



while (!WindowShouldClose()) 
{


boxPositionY -= (GetMouseWheelMove()*scrollSpeed);




BeginDrawing();

ClearBackground(RAYWHITE);

DrawRectangle(screenWidth/2 - 40, boxPositionY, 80, 80, MAROON);

DrawText("Use mouse wheel to move the cube up and down!", 10, 10, 20, GRAY);
DrawText(FormatText("Box position Y: %03i", boxPositionY), 10, 40, 20, LIGHTGRAY);

EndDrawing();

}



CloseWindow(); 


return 0;
}