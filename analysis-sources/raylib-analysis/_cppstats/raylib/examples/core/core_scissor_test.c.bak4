












#include "raylib.h"

int main(void)
{


const int screenWidth = 800;
const int screenHeight = 450;

InitWindow(screenWidth, screenHeight, "raylib [core] example - scissor test");

Rectangle scissorArea = { 0, 0, 300, 300 };
bool scissorMode = true;

SetTargetFPS(60); 



while (!WindowShouldClose()) 
{


if (IsKeyPressed(KEY_S)) scissorMode = !scissorMode;


scissorArea.x = GetMouseX() - scissorArea.width/2;
scissorArea.y = GetMouseY() - scissorArea.height/2;




BeginDrawing();

ClearBackground(RAYWHITE);

if (scissorMode) BeginScissorMode(scissorArea.x, scissorArea.y, scissorArea.width, scissorArea.height);



DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), RED);
DrawText("Move the mouse around to reveal this text!", 190, 200, 20, LIGHTGRAY);

if (scissorMode) EndScissorMode();

DrawRectangleLinesEx(scissorArea, 1, BLACK);
DrawText("Press S to toggle scissor test", 10, 10, 20, BLACK);

EndDrawing();

}



CloseWindow(); 


return 0;
}
