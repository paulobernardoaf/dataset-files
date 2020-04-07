










#include "raylib.h"

#define MAX_INPUT_CHARS 9

int main(void)
{


const int screenWidth = 800;
const int screenHeight = 450;

InitWindow(screenWidth, screenHeight, "raylib [text] example - input box");

char name[MAX_INPUT_CHARS + 1] = "\0"; 
int letterCount = 0;

Rectangle textBox = { screenWidth/2 - 100, 180, 225, 50 };
bool mouseOnText = false;

int framesCounter = 0;

SetTargetFPS(10); 



while (!WindowShouldClose()) 
{


if (CheckCollisionPointRec(GetMousePosition(), textBox)) mouseOnText = true;
else mouseOnText = false;

if (mouseOnText)
{

int key = GetKeyPressed();


while (key > 0)
{

if ((key >= 32) && (key <= 125) && (letterCount < MAX_INPUT_CHARS))
{
name[letterCount] = (char)key;
letterCount++;
}

key = GetKeyPressed(); 
}

if (IsKeyPressed(KEY_BACKSPACE))
{
letterCount--;
name[letterCount] = '\0';

if (letterCount < 0) letterCount = 0;
}
}

if (mouseOnText) framesCounter++;
else framesCounter = 0;




BeginDrawing();

ClearBackground(RAYWHITE);

DrawText("PLACE MOUSE OVER INPUT BOX!", 240, 140, 20, GRAY);

DrawRectangleRec(textBox, LIGHTGRAY);
if (mouseOnText) DrawRectangleLines(textBox.x, textBox.y, textBox.width, textBox.height, RED);
else DrawRectangleLines(textBox.x, textBox.y, textBox.width, textBox.height, DARKGRAY);

DrawText(name, textBox.x + 5, textBox.y + 8, 40, MAROON);

DrawText(FormatText("INPUT CHARS: %i/%i", letterCount, MAX_INPUT_CHARS), 315, 250, 20, DARKGRAY);

if (mouseOnText)
{
if (letterCount < MAX_INPUT_CHARS)
{

if (((framesCounter/20)%2) == 0) DrawText("_", textBox.x + 8 + MeasureText(name, 40), textBox.y + 12, 40, MAROON);
}
else DrawText("Press BACKSPACE to delete chars...", 230, 300, 20, GRAY);
}

EndDrawing();

}



CloseWindow(); 


return 0;
}



bool IsAnyKeyPressed()
{
bool keyPressed = false;
int key = GetKeyPressed();

if ((key >= 32) && (key <= 126)) keyPressed = true;

return keyPressed;
}