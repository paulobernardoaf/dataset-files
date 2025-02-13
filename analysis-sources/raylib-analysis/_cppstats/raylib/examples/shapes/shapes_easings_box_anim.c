#include "raylib.h"
#include "easings.h" 
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
InitWindow(screenWidth, screenHeight, "raylib [shapes] example - easings box anim");
Rectangle rec = { GetScreenWidth()/2, -100, 100, 100 };
float rotation = 0.0f;
float alpha = 1.0f;
int state = 0;
int framesCounter = 0;
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
switch (state)
{
case 0: 
{
framesCounter++;
rec.y = EaseElasticOut(framesCounter, -100, GetScreenHeight()/2 + 100, 120);
if (framesCounter >= 120)
{
framesCounter = 0;
state = 1;
}
} break;
case 1: 
{
framesCounter++;
rec.height = EaseBounceOut(framesCounter, 100, -90, 120);
rec.width = EaseBounceOut(framesCounter, 100, GetScreenWidth(), 120);
if (framesCounter >= 120)
{
framesCounter = 0;
state = 2;
}
} break;
case 2: 
{
framesCounter++;
rotation = EaseQuadOut(framesCounter, 0.0f, 270.0f, 240);
if (framesCounter >= 240)
{
framesCounter = 0;
state = 3;
}
} break;
case 3: 
{
framesCounter++;
rec.height = EaseCircOut(framesCounter, 10, GetScreenWidth(), 120);
if (framesCounter >= 120)
{
framesCounter = 0;
state = 4;
}
} break;
case 4: 
{
framesCounter++;
alpha = EaseSineOut(framesCounter, 1.0f, -1.0f, 160);
if (framesCounter >= 160)
{
framesCounter = 0;
state = 5;
}
} break;
default: break;
}
if (IsKeyPressed(KEY_SPACE))
{
rec = (Rectangle){ GetScreenWidth()/2, -100, 100, 100 };
rotation = 0.0f;
alpha = 1.0f;
state = 0;
framesCounter = 0;
}
BeginDrawing();
ClearBackground(RAYWHITE);
DrawRectanglePro(rec, (Vector2){ rec.width/2, rec.height/2 }, rotation, Fade(BLACK, alpha));
DrawText("PRESS [SPACE] TO RESET BOX ANIMATION!", 10, GetScreenHeight() - 25, 20, LIGHTGRAY);
EndDrawing();
}
CloseWindow(); 
return 0;
}