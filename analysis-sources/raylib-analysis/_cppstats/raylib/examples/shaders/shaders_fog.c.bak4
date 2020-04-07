


























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
InitWindow(screenWidth, screenHeight, "raylib [shaders] example - fog");


Camera camera = { 
(Vector3){ 2.0f, 2.0f, 6.0f }, 
(Vector3){ 0.0f, 0.5f, 0.0f }, 
(Vector3){ 0.0f, 1.0f, 0.0f }, 
45.0f, CAMERA_PERSPECTIVE }; 


Model modelA = LoadModelFromMesh(GenMeshTorus(0.4f, 1.0f, 16, 32));
Model modelB = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
Model modelC = LoadModelFromMesh(GenMeshSphere(0.5f, 32, 32));
Texture texture = LoadTexture("resources/texel_checker.png");


modelA.materials[0].maps[MAP_DIFFUSE].texture = texture;
modelB.materials[0].maps[MAP_DIFFUSE].texture = texture;
modelC.materials[0].maps[MAP_DIFFUSE].texture = texture;


Shader shader = LoadShader(FormatText("resources/shaders/glsl%i/base_lighting.vs", GLSL_VERSION), 
FormatText("resources/shaders/glsl%i/fog.fs", GLSL_VERSION));
shader.locs[LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");
shader.locs[LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");


int ambientLoc = GetShaderLocation(shader, "ambient");
SetShaderValue(shader, ambientLoc, (float[4]){ 0.2f, 0.2f, 0.2f, 1.0f }, UNIFORM_VEC4);

float fogDensity = 0.15f;
int fogDensityLoc = GetShaderLocation(shader, "fogDensity");
SetShaderValue(shader, fogDensityLoc, &fogDensity, UNIFORM_FLOAT);


modelA.materials[0].shader = shader;
modelB.materials[0].shader = shader;
modelC.materials[0].shader = shader;


CreateLight(LIGHT_POINT, (Vector3){ 0, 2, 6 }, Vector3Zero(), WHITE, shader);

SetCameraMode(camera, CAMERA_ORBITAL); 

SetTargetFPS(60); 



while (!WindowShouldClose()) 
{


UpdateCamera(&camera); 

if (IsKeyDown(KEY_UP)) 
{
fogDensity += 0.001;
if (fogDensity > 1.0) fogDensity = 1.0;
}

if (IsKeyDown(KEY_DOWN))
{
fogDensity -= 0.001;
if (fogDensity < 0.0) fogDensity = 0.0;
}

SetShaderValue(shader, fogDensityLoc, &fogDensity, UNIFORM_FLOAT);


modelA.transform = MatrixMultiply(modelA.transform, MatrixRotateX(-0.025));
modelA.transform = MatrixMultiply(modelA.transform, MatrixRotateZ(0.012));


SetShaderValue(shader, shader.locs[LOC_VECTOR_VIEW], &camera.position.x, UNIFORM_VEC3);




BeginDrawing();

ClearBackground(GRAY);

BeginMode3D(camera);


DrawModel(modelA, Vector3Zero(), 1.0f, WHITE);
DrawModel(modelB, (Vector3){ -2.6, 0, 0 }, 1.0f, WHITE);
DrawModel(modelC, (Vector3){ 2.6, 0, 0 }, 1.0f, WHITE);

for (int i = -20; i < 20; i += 2) DrawModel(modelA,(Vector3){ i, 0, 2 }, 1.0f, WHITE);

EndMode3D();

DrawText(TextFormat("Use KEY_UP/KEY_DOWN to change fog density [%.2f]", fogDensity), 10, 10, 20, RAYWHITE);

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
