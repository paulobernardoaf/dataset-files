#include "raylib.h"
#define MAX_FONTS 8
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
InitWindow(screenWidth, screenHeight, "raylib [text] example - raylib fonts");
Font fonts[MAX_FONTS] = { 0 };
fonts[0] = LoadFont("resources/fonts/alagard.png");
fonts[1] = LoadFont("resources/fonts/pixelplay.png");
fonts[2] = LoadFont("resources/fonts/mecha.png");
fonts[3] = LoadFont("resources/fonts/setback.png");
fonts[4] = LoadFont("resources/fonts/romulus.png");
fonts[5] = LoadFont("resources/fonts/pixantiqua.png");
fonts[6] = LoadFont("resources/fonts/alpha_beta.png");
fonts[7] = LoadFont("resources/fonts/jupiter_crash.png");
const char *messages[MAX_FONTS] = { "ALAGARD FONT designed by Hewett Tsoi",
"PIXELPLAY FONT designed by Aleksander Shevchuk",
"MECHA FONT designed by Captain Falcon",
"SETBACK FONT designed by Brian Kent (AEnigma)",
"ROMULUS FONT designed by Hewett Tsoi",
"PIXANTIQUA FONT designed by Gerhard Grossmann",
"ALPHA_BETA FONT designed by Brian Kent (AEnigma)",
"JUPITER_CRASH FONT designed by Brian Kent (AEnigma)" };
const int spacings[MAX_FONTS] = { 2, 4, 8, 4, 3, 4, 4, 1 };
Vector2 positions[MAX_FONTS] = { 0 };
for (int i = 0; i < MAX_FONTS; i++)
{
positions[i].x = screenWidth/2 - MeasureTextEx(fonts[i], messages[i], fonts[i].baseSize*2, spacings[i]).x/2;
positions[i].y = 60 + fonts[i].baseSize + 45*i;
}
positions[3].y += 8;
positions[4].y += 2;
positions[7].y -= 8;
Color colors[MAX_FONTS] = { MAROON, ORANGE, DARKGREEN, DARKBLUE, DARKPURPLE, LIME, GOLD, RED };
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
BeginDrawing();
ClearBackground(RAYWHITE);
DrawText("free fonts included with raylib", 250, 20, 20, DARKGRAY);
DrawLine(220, 50, 590, 50, DARKGRAY);
for (int i = 0; i < MAX_FONTS; i++)
{
DrawTextEx(fonts[i], messages[i], positions[i], fonts[i].baseSize*2, spacings[i], colors[i]);
}
EndDrawing();
}
for (int i = 0; i < MAX_FONTS; i++) UnloadFont(fonts[i]);
CloseWindow(); 
return 0;
}