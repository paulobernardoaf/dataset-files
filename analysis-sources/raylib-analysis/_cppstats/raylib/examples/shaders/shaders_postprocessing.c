#include "raylib.h"
#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION 330
#else 
#define GLSL_VERSION 100
#endif
#define MAX_POSTPRO_SHADERS 12
typedef enum {
FX_GRAYSCALE = 0,
FX_POSTERIZATION,
FX_DREAM_VISION,
FX_PIXELIZER,
FX_CROSS_HATCHING,
FX_CROSS_STITCHING,
FX_PREDATOR_VIEW,
FX_SCANLINES,
FX_FISHEYE,
FX_SOBEL,
FX_BLOOM,
FX_BLUR,
} PostproShader;
static const char *postproShaderText[] = {
"GRAYSCALE",
"POSTERIZATION",
"DREAM_VISION",
"PIXELIZER",
"CROSS_HATCHING",
"CROSS_STITCHING",
"PREDATOR_VIEW",
"SCANLINES",
"FISHEYE",
"SOBEL",
"BLOOM",
"BLUR",
};
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
SetConfigFlags(FLAG_MSAA_4X_HINT); 
InitWindow(screenWidth, screenHeight, "raylib [shaders] example - postprocessing shader");
Camera camera = { { 2.0f, 3.0f, 2.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 45.0f, 0 };
Model model = LoadModel("resources/models/church.obj"); 
Texture2D texture = LoadTexture("resources/models/church_diffuse.png"); 
model.materials[0].maps[MAP_DIFFUSE].texture = texture; 
Vector3 position = { 0.0f, 0.0f, 0.0f }; 
Shader shaders[MAX_POSTPRO_SHADERS] = { 0 };
shaders[FX_GRAYSCALE] = LoadShader(0, FormatText("resources/shaders/glsl%i/grayscale.fs", GLSL_VERSION));
shaders[FX_POSTERIZATION] = LoadShader(0, FormatText("resources/shaders/glsl%i/posterization.fs", GLSL_VERSION));
shaders[FX_DREAM_VISION] = LoadShader(0, FormatText("resources/shaders/glsl%i/dream_vision.fs", GLSL_VERSION));
shaders[FX_PIXELIZER] = LoadShader(0, FormatText("resources/shaders/glsl%i/pixelizer.fs", GLSL_VERSION));
shaders[FX_CROSS_HATCHING] = LoadShader(0, FormatText("resources/shaders/glsl%i/cross_hatching.fs", GLSL_VERSION));
shaders[FX_CROSS_STITCHING] = LoadShader(0, FormatText("resources/shaders/glsl%i/cross_stitching.fs", GLSL_VERSION));
shaders[FX_PREDATOR_VIEW] = LoadShader(0, FormatText("resources/shaders/glsl%i/predator.fs", GLSL_VERSION));
shaders[FX_SCANLINES] = LoadShader(0, FormatText("resources/shaders/glsl%i/scanlines.fs", GLSL_VERSION));
shaders[FX_FISHEYE] = LoadShader(0, FormatText("resources/shaders/glsl%i/fisheye.fs", GLSL_VERSION));
shaders[FX_SOBEL] = LoadShader(0, FormatText("resources/shaders/glsl%i/sobel.fs", GLSL_VERSION));
shaders[FX_BLOOM] = LoadShader(0, FormatText("resources/shaders/glsl%i/bloom.fs", GLSL_VERSION));
shaders[FX_BLUR] = LoadShader(0, FormatText("resources/shaders/glsl%i/blur.fs", GLSL_VERSION));
int currentShader = FX_GRAYSCALE;
RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);
SetCameraMode(camera, CAMERA_ORBITAL); 
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
UpdateCamera(&camera); 
if (IsKeyPressed(KEY_RIGHT)) currentShader++;
else if (IsKeyPressed(KEY_LEFT)) currentShader--;
if (currentShader >= MAX_POSTPRO_SHADERS) currentShader = 0;
else if (currentShader < 0) currentShader = MAX_POSTPRO_SHADERS - 1;
BeginDrawing();
ClearBackground(RAYWHITE);
BeginTextureMode(target); 
ClearBackground(RAYWHITE); 
BeginMode3D(camera); 
DrawModel(model, position, 0.1f, WHITE); 
DrawGrid(10, 1.0f); 
EndMode3D(); 
EndTextureMode(); 
BeginShaderMode(shaders[currentShader]);
DrawTextureRec(target.texture, (Rectangle){ 0, 0, target.texture.width, -target.texture.height }, (Vector2){ 0, 0 }, WHITE);
EndShaderMode();
DrawRectangle(0, 9, 580, 30, Fade(LIGHTGRAY, 0.7f));
DrawText("(c) Church 3D model by Alberto Cano", screenWidth - 200, screenHeight - 20, 10, GRAY);
DrawText("CURRENT POSTPRO SHADER:", 10, 15, 20, BLACK);
DrawText(postproShaderText[currentShader], 330, 15, 20, RED);
DrawText("< >", 540, 10, 30, DARKBLUE);
DrawFPS(700, 15);
EndDrawing();
}
for (int i = 0; i < MAX_POSTPRO_SHADERS; i++) UnloadShader(shaders[i]);
UnloadTexture(texture); 
UnloadModel(model); 
UnloadRenderTexture(target); 
CloseWindow(); 
return 0;
}
