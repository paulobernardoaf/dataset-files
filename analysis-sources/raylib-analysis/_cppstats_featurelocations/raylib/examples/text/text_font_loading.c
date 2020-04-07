



















#include "raylib.h"

int main(void)
{


const int screenWidth = 800;
const int screenHeight = 450;

InitWindow(screenWidth, screenHeight, "raylib [text] example - font loading");



const char msg[256] = "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHI\nJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmn\nopqrstuvwxyz{|}~¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓ\nÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷\nøùúûüýþÿ";




Font fontBm = LoadFont("resources/pixantiqua.fnt");



Font fontTtf = LoadFontEx("resources/pixantiqua.ttf", 32, 0, 250);

bool useTtf = false;

SetTargetFPS(60); 



while (!WindowShouldClose()) 
{


if (IsKeyDown(KEY_SPACE)) useTtf = true;
else useTtf = false;




BeginDrawing();

ClearBackground(RAYWHITE);

DrawText("Hold SPACE to use TTF generated font", 20, 20, 20, LIGHTGRAY);

if (!useTtf)
{
DrawTextEx(fontBm, msg, (Vector2){ 20.0f, 100.0f }, fontBm.baseSize, 2, MAROON);
DrawText("Using BMFont (Angelcode) imported", 20, GetScreenHeight() - 30, 20, GRAY);
}
else
{
DrawTextEx(fontTtf, msg, (Vector2){ 20.0f, 100.0f }, fontTtf.baseSize, 2, LIME);
DrawText("Using TTF font generated", 20, GetScreenHeight() - 30, 20, GRAY);
}

EndDrawing();

}



UnloadFont(fontBm); 
UnloadFont(fontTtf); 

CloseWindow(); 


return 0;
}