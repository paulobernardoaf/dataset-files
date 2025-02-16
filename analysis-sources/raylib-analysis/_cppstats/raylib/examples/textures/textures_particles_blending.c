#include "raylib.h"
#define MAX_PARTICLES 200
typedef struct {
Vector2 position;
Color color;
float alpha;
float size;
float rotation;
bool active; 
} Particle;
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
InitWindow(screenWidth, screenHeight, "raylib [textures] example - particles blending");
Particle mouseTail[MAX_PARTICLES] = { 0 };
for (int i = 0; i < MAX_PARTICLES; i++)
{
mouseTail[i].position = (Vector2){ 0, 0 };
mouseTail[i].color = (Color){ GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255), 255 };
mouseTail[i].alpha = 1.0f;
mouseTail[i].size = (float)GetRandomValue(1, 30)/20.0f;
mouseTail[i].rotation = (float)GetRandomValue(0, 360);
mouseTail[i].active = false;
}
float gravity = 3.0f;
Texture2D smoke = LoadTexture("resources/smoke.png");
int blending = BLEND_ALPHA;
SetTargetFPS(60);
while (!WindowShouldClose()) 
{
for (int i = 0; i < MAX_PARTICLES; i++)
{
if (!mouseTail[i].active)
{
mouseTail[i].active = true;
mouseTail[i].alpha = 1.0f;
mouseTail[i].position = GetMousePosition();
i = MAX_PARTICLES;
}
}
for (int i = 0; i < MAX_PARTICLES; i++)
{
if (mouseTail[i].active)
{
mouseTail[i].position.y += gravity;
mouseTail[i].alpha -= 0.01f;
if (mouseTail[i].alpha <= 0.0f) mouseTail[i].active = false;
mouseTail[i].rotation += 5.0f;
}
}
if (IsKeyPressed(KEY_SPACE))
{
if (blending == BLEND_ALPHA) blending = BLEND_ADDITIVE;
else blending = BLEND_ALPHA;
}
BeginDrawing();
ClearBackground(DARKGRAY);
BeginBlendMode(blending);
for (int i = 0; i < MAX_PARTICLES; i++)
{
if (mouseTail[i].active) DrawTexturePro(smoke, (Rectangle){ 0.0f, 0.0f, (float)smoke.width, (float)smoke.height },
(Rectangle){ mouseTail[i].position.x, mouseTail[i].position.y, smoke.width*mouseTail[i].size, smoke.height*mouseTail[i].size },
(Vector2){ (float)(smoke.width*mouseTail[i].size/2.0f), (float)(smoke.height*mouseTail[i].size/2.0f) }, mouseTail[i].rotation,
Fade(mouseTail[i].color, mouseTail[i].alpha));
}
EndBlendMode();
DrawText("PRESS SPACE to CHANGE BLENDING MODE", 180, 20, 20, BLACK);
if (blending == BLEND_ALPHA) DrawText("ALPHA BLENDING", 290, screenHeight - 40, 20, BLACK);
else DrawText("ADDITIVE BLENDING", 280, screenHeight - 40, 20, RAYWHITE);
EndDrawing();
}
UnloadTexture(smoke);
CloseWindow(); 
return 0;
}