



















#include "raylib.h"

int main(void)
{


const int screenWidth = 800;
const int screenHeight = 450;

InitWindow(screenWidth, screenHeight, "raylib [text] example - sprite font loading");

const char msg1[50] = "THIS IS A custom SPRITE FONT...";
const char msg2[50] = "...and this is ANOTHER CUSTOM font...";
const char msg3[50] = "...and a THIRD one! GREAT! :D";


Font font1 = LoadFont("resources/custom_mecha.png"); 
Font font2 = LoadFont("resources/custom_alagard.png"); 
Font font3 = LoadFont("resources/custom_jupiter_crash.png"); 

Vector2 fontPosition1 = { screenWidth/2 - MeasureTextEx(font1, msg1, font1.baseSize, -3).x/2,
screenHeight/2 - font1.baseSize/2 - 80 };

Vector2 fontPosition2 = { screenWidth/2 - MeasureTextEx(font2, msg2, font2.baseSize, -2).x/2,
screenHeight/2 - font2.baseSize/2 - 10 };

Vector2 fontPosition3 = { screenWidth/2 - MeasureTextEx(font3, msg3, font3.baseSize, 2).x/2,
screenHeight/2 - font3.baseSize/2 + 50 };

SetTargetFPS(60); 



while (!WindowShouldClose()) 
{







BeginDrawing();

ClearBackground(RAYWHITE);

DrawTextEx(font1, msg1, fontPosition1, font1.baseSize, -3, WHITE);
DrawTextEx(font2, msg2, fontPosition2, font2.baseSize, -2, WHITE);
DrawTextEx(font3, msg3, fontPosition3, font3.baseSize, 2, WHITE);

EndDrawing();

}



UnloadFont(font1); 
UnloadFont(font2); 
UnloadFont(font3); 

CloseWindow(); 


return 0;
}