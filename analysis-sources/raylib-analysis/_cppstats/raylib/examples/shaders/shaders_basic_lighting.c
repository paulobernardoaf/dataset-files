#include "raylib.h"
#include "raymath.h"
#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"
#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION 330
#else 
#define GLSL_VERSION 100
#endif
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
SetConfigFlags(FLAG_MSAA_4X_HINT); 
InitWindow(screenWidth, screenHeight, "raylib [shaders] example - basic lighting");
Camera camera = { 0 };
camera.position = (Vector3){ 2.0f, 2.0f, 6.0f }; 
camera.target = (Vector3){ 0.0f, 0.5f, 0.0f }; 
camera.up = (Vector3){ 0.0f, 1.0f, 0.0f }; 
camera.fovy = 45.0f; 
camera.type = CAMERA_PERSPECTIVE; 
Model modelA = LoadModelFromMesh(GenMeshTorus(0.4f, 1.0f, 16, 32));
Model modelB = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
Model modelC = LoadModelFromMesh(GenMeshSphere(0.5f, 32, 32));
Texture texture = LoadTexture("resources/texel_checker.png");
modelA.materials[0].maps[MAP_DIFFUSE].texture = texture;
modelB.materials[0].maps[MAP_DIFFUSE].texture = texture;
modelC.materials[0].maps[MAP_DIFFUSE].texture = texture;
Shader shader = LoadShader(FormatText("resources/shaders/glsl%i/base_lighting.vs", GLSL_VERSION), 
FormatText("resources/shaders/glsl%i/lighting.fs", GLSL_VERSION));
shader.locs[LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");
shader.locs[LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
int ambientLoc = GetShaderLocation(shader, "ambient");
SetShaderValue(shader, ambientLoc, (float[4]){ 0.2f, 0.2f, 0.2f, 1.0f }, UNIFORM_VEC4);
float angle = 6.282f;
modelA.materials[0].shader = shader;
modelB.materials[0].shader = shader;
modelC.materials[0].shader = shader;
Light lights[MAX_LIGHTS] = { 0 };
lights[0] = CreateLight(LIGHT_POINT, (Vector3){ 4, 2, 4 }, Vector3Zero(), WHITE, shader);
lights[1] = CreateLight(LIGHT_POINT, (Vector3){ 4, 2, 4 }, Vector3Zero(), RED, shader);
lights[2] = CreateLight(LIGHT_POINT, (Vector3){ 0, 4, 2 }, Vector3Zero(), GREEN, shader);
lights[3] = CreateLight(LIGHT_POINT, (Vector3){ 0, 4, 2 }, Vector3Zero(), BLUE, shader);
SetCameraMode(camera, CAMERA_ORBITAL); 
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
if (IsKeyPressed(KEY_W)) { lights[0].enabled = !lights[0].enabled; }
if (IsKeyPressed(KEY_R)) { lights[1].enabled = !lights[1].enabled; }
if (IsKeyPressed(KEY_G)) { lights[2].enabled = !lights[2].enabled; }
if (IsKeyPressed(KEY_B)) { lights[3].enabled = !lights[3].enabled; }
UpdateCamera(&camera); 
angle -= 0.02f;
lights[0].position.x = cosf(angle)*4.0f;
lights[0].position.z = sinf(angle)*4.0f;
lights[1].position.x = cosf(-angle*0.6f)*4.0f;
lights[1].position.z = sinf(-angle*0.6f)*4.0f;
lights[2].position.y = cosf(angle*0.2f)*4.0f;
lights[2].position.z = sinf(angle*0.2f)*4.0f;
lights[3].position.y = cosf(-angle*0.35f)*4.0f;
lights[3].position.z = sinf(-angle*0.35f)*4.0f;
UpdateLightValues(shader, lights[0]);
UpdateLightValues(shader, lights[1]);
UpdateLightValues(shader, lights[2]);
UpdateLightValues(shader, lights[3]);
modelA.transform = MatrixMultiply(modelA.transform, MatrixRotateX(-0.025f));
modelA.transform = MatrixMultiply(modelA.transform, MatrixRotateZ(0.012f));
float cameraPos[3] = { camera.position.x, camera.position.y, camera.position.z };
SetShaderValue(shader, shader.locs[LOC_VECTOR_VIEW], cameraPos, UNIFORM_VEC3);
BeginDrawing();
ClearBackground(RAYWHITE);
BeginMode3D(camera);
DrawModel(modelA, Vector3Zero(), 1.0f, WHITE);
DrawModel(modelB, (Vector3){-1.6,0,0}, 1.0f, WHITE);
DrawModel(modelC, (Vector3){ 1.6,0,0}, 1.0f, WHITE);
if (lights[0].enabled) { DrawSphereEx(lights[0].position, 0.2f, 8, 8, WHITE); }
if (lights[1].enabled) { DrawSphereEx(lights[1].position, 0.2f, 8, 8, RED); }
if (lights[2].enabled) { DrawSphereEx(lights[2].position, 0.2f, 8, 8, GREEN); }
if (lights[3].enabled) { DrawSphereEx(lights[3].position, 0.2f, 8, 8, BLUE); }
DrawGrid(10, 1.0f);
EndMode3D();
DrawFPS(10, 10);
DrawText("Use keys RGBW to toggle lights", 10, 30, 20, DARKGRAY);
EndDrawing();
}
UnloadModel(modelA); 
UnloadModel(modelB); 
UnloadModel(modelC); 
UnloadTexture(texture); 
UnloadShader(shader); 
CloseWindow(); 
return 0;
}
