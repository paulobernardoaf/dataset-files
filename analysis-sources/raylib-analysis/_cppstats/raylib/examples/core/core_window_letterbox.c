#include "raylib.h"
#define max(a, b) ((a)>(b)? (a) : (b))
#define min(a, b) ((a)<(b)? (a) : (b))
Vector2 ClampValue(Vector2 value, Vector2 min, Vector2 max)
{
Vector2 result = value;
result.x = (result.x > max.x)? max.x : result.x;
result.x = (result.x < min.x)? min.x : result.x;
result.y = (result.y > max.y)? max.y : result.y;
result.y = (result.y < min.y)? min.y : result.y;
return result;
}
int main(void)
{
const int windowWidth = 800;
const int windowHeight = 450;
SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
InitWindow(windowWidth, windowHeight, "raylib [core] example - window scale letterbox");
SetWindowMinSize(320, 240);
int gameScreenWidth = 640;
int gameScreenHeight = 480;
RenderTexture2D target = LoadRenderTexture(gameScreenWidth, gameScreenHeight);
SetTextureFilter(target.texture, FILTER_BILINEAR); 
Color colors[10] = { 0 };
for (int i = 0; i < 10; i++) colors[i] = (Color){ GetRandomValue(100, 250), GetRandomValue(50, 150), GetRandomValue(10, 100), 255 };
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
float scale = min((float)GetScreenWidth()/gameScreenWidth, (float)GetScreenHeight()/gameScreenHeight);
if (IsKeyPressed(KEY_SPACE))
{
for (int i = 0; i < 10; i++) colors[i] = (Color){ GetRandomValue(100, 250), GetRandomValue(50, 150), GetRandomValue(10, 100), 255 };
}
Vector2 mouse = GetMousePosition();
Vector2 virtualMouse = { 0 };
virtualMouse.x = (mouse.x - (GetScreenWidth() - (gameScreenWidth*scale))*0.5f)/scale;
virtualMouse.y = (mouse.y - (GetScreenHeight() - (gameScreenHeight*scale))*0.5f)/scale;
virtualMouse = ClampValue(virtualMouse, (Vector2){ 0, 0 }, (Vector2){ gameScreenWidth, gameScreenHeight }); 
BeginDrawing();
ClearBackground(BLACK);
BeginTextureMode(target);
ClearBackground(RAYWHITE); 
for (int i = 0; i < 10; i++) DrawRectangle(0, (gameScreenHeight/10)*i, gameScreenWidth, gameScreenHeight/10, colors[i]);
DrawText("If executed inside a window,\nyou can resize the window,\nand see the screen scaling!", 10, 25, 20, WHITE);
DrawText(TextFormat("Default Mouse: [%i , %i]", (int)mouse.x, (int)mouse.y), 350, 25, 20, GREEN);
DrawText(TextFormat("Virtual Mouse: [%i , %i]", (int)virtualMouse.x, (int)virtualMouse.y), 350, 55, 20, YELLOW);
EndTextureMode();
DrawTexturePro(target.texture, (Rectangle){ 0.0f, 0.0f, (float)target.texture.width, (float)-target.texture.height },
(Rectangle){ (GetScreenWidth() - ((float)gameScreenWidth*scale))*0.5, (GetScreenHeight() - ((float)gameScreenHeight*scale))*0.5,
(float)gameScreenWidth*scale, (float)gameScreenHeight*scale }, (Vector2){ 0, 0 }, 0.0f, WHITE);
EndDrawing();
}
UnloadRenderTexture(target); 
CloseWindow(); 
return 0;
}
