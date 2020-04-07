










#include "raylib.h"

int main(void)
{


const int screenWidth = 800;
const int screenHeight = 450;

SetConfigFlags(FLAG_MSAA_4X_HINT);
InitWindow(screenWidth, screenHeight, "raylib [shapes] example - cubic-bezier lines");

Vector2 start = { 0, 0 };
Vector2 end = { screenWidth, screenHeight };

SetTargetFPS(60); 



while (!WindowShouldClose()) 
{


if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) start = GetMousePosition();
else if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) end = GetMousePosition();




BeginDrawing();

ClearBackground(RAYWHITE);

DrawText("USE MOUSE LEFT-RIGHT CLICK to DEFINE LINE START and END POINTS", 15, 20, 20, GRAY);

DrawLineBezier(start, end, 2.0f, RED);

EndDrawing();

}



CloseWindow(); 


return 0;
}
