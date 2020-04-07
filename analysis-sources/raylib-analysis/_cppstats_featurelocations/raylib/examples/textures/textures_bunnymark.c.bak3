










#include "raylib.h"

#include <stdlib.h> 

#define MAX_BUNNIES 50000 



#define MAX_BATCH_ELEMENTS 8192

typedef struct Bunny {
Vector2 position;
Vector2 speed;
Color color;
} Bunny;

int main(void)
{


const int screenWidth = 800;
const int screenHeight = 450;

InitWindow(screenWidth, screenHeight, "raylib [textures] example - bunnymark");


Texture2D texBunny = LoadTexture("resources/wabbit_alpha.png");

Bunny *bunnies = (Bunny *)malloc(MAX_BUNNIES*sizeof(Bunny)); 

int bunniesCount = 0; 

SetTargetFPS(60); 



while (!WindowShouldClose()) 
{


if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
{

for (int i = 0; i < 100; i++)
{
if (bunniesCount < MAX_BUNNIES)
{
bunnies[bunniesCount].position = GetMousePosition();
bunnies[bunniesCount].speed.x = (float)GetRandomValue(-250, 250)/60.0f;
bunnies[bunniesCount].speed.y = (float)GetRandomValue(-250, 250)/60.0f;
bunnies[bunniesCount].color = (Color){ GetRandomValue(50, 240),
GetRandomValue(80, 240),
GetRandomValue(100, 240), 255 };
bunniesCount++;
}
}
}


for (int i = 0; i < bunniesCount; i++)
{
bunnies[i].position.x += bunnies[i].speed.x;
bunnies[i].position.y += bunnies[i].speed.y;

if (((bunnies[i].position.x + texBunny.width/2) > GetScreenWidth()) ||
((bunnies[i].position.x + texBunny.width/2) < 0)) bunnies[i].speed.x *= -1;
if (((bunnies[i].position.y + texBunny.height/2) > GetScreenHeight()) ||
((bunnies[i].position.y + texBunny.height/2 - 40) < 0)) bunnies[i].speed.y *= -1;
}




BeginDrawing();

ClearBackground(RAYWHITE);

for (int i = 0; i < bunniesCount; i++)
{






DrawTexture(texBunny, bunnies[i].position.x, bunnies[i].position.y, bunnies[i].color);
}

DrawRectangle(0, 0, screenWidth, 40, BLACK);
DrawText(FormatText("bunnies: %i", bunniesCount), 120, 10, 20, GREEN);
DrawText(FormatText("batched draw calls: %i", 1 + bunniesCount/MAX_BATCH_ELEMENTS), 320, 10, 20, MAROON);

DrawFPS(10, 10);

EndDrawing();

}



free(bunnies); 

UnloadTexture(texBunny); 

CloseWindow(); 


return 0;
}
