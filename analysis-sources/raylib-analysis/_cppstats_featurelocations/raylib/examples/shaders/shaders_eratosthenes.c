
























#include "raylib.h"

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION 330
#else 
#define GLSL_VERSION 100
#endif

int main(void)
{


const int screenWidth = 800;
const int screenHeight = 450;

InitWindow(screenWidth, screenHeight, "raylib [shaders] example - Sieve of Eratosthenes");

RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);



Shader shader = LoadShader(0, FormatText("resources/shaders/glsl%i/eratosthenes.fs", GLSL_VERSION));

SetTargetFPS(60); 



while (!WindowShouldClose()) 
{







BeginDrawing();

ClearBackground(RAYWHITE);

BeginTextureMode(target); 
ClearBackground(BLACK); 





DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLACK);
EndTextureMode(); 

BeginShaderMode(shader);

DrawTextureRec(target.texture, (Rectangle){ 0, 0, target.texture.width, -target.texture.height }, (Vector2){ 0.0f, 0.0f }, WHITE);
EndShaderMode();

EndDrawing();

}



UnloadShader(shader); 
UnloadRenderTexture(target); 

CloseWindow(); 


return 0;
}
