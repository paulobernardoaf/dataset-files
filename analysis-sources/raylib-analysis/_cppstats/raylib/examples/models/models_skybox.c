#include "raylib.h"
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
InitWindow(screenWidth, screenHeight, "raylib [models] example - skybox loading and drawing");
Camera camera = { { 1.0f, 1.0f, 1.0f }, { 4.0f, 1.0f, 4.0f }, { 0.0f, 1.0f, 0.0f }, 45.0f, 0 };
Mesh cube = GenMeshCube(1.0f, 1.0f, 1.0f);
Model skybox = LoadModelFromMesh(cube);
#if defined(PLATFORM_DESKTOP)
skybox.materials[0].shader = LoadShader("resources/shaders/glsl330/skybox.vs", "resources/shaders/glsl330/skybox.fs");
#else 
skybox.materials[0].shader = LoadShader("resources/shaders/glsl100/skybox.vs", "resources/shaders/glsl100/skybox.fs");
#endif
SetShaderValue(skybox.materials[0].shader, GetShaderLocation(skybox.materials[0].shader, "environmentMap"), (int[1]){ MAP_CUBEMAP }, UNIFORM_INT);
#if defined(PLATFORM_DESKTOP)
Shader shdrCubemap = LoadShader("resources/shaders/glsl330/cubemap.vs", "resources/shaders/glsl330/cubemap.fs");
#else 
Shader shdrCubemap = LoadShader("resources/shaders/glsl100/cubemap.vs", "resources/shaders/glsl100/cubemap.fs");
#endif
SetShaderValue(shdrCubemap, GetShaderLocation(shdrCubemap, "equirectangularMap"), (int[1]){ 0 }, UNIFORM_INT);
Texture2D texHDR = LoadTexture("resources/dresden_square.hdr");
skybox.materials[0].maps[MAP_CUBEMAP].texture = GenTextureCubemap(shdrCubemap, texHDR, 512);
UnloadTexture(texHDR); 
UnloadShader(shdrCubemap); 
SetCameraMode(camera, CAMERA_FIRST_PERSON); 
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
UpdateCamera(&camera); 
BeginDrawing();
ClearBackground(RAYWHITE);
BeginMode3D(camera);
DrawModel(skybox, (Vector3){0, 0, 0}, 1.0f, WHITE);
DrawGrid(10, 1.0f);
EndMode3D();
DrawFPS(10, 10);
EndDrawing();
}
UnloadShader(skybox.materials[0].shader);
UnloadTexture(skybox.materials[0].maps[MAP_CUBEMAP].texture);
UnloadModel(skybox); 
CloseWindow(); 
return 0;
}
