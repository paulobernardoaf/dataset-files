



















#include "raylib.h"
#include "raymath.h"

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION 330
#else 
#define GLSL_VERSION 100
#endif

int main(void)
{


const int screenWidth = 800;
const int screenHeight = 450;

InitWindow(screenWidth, screenHeight, "raylib - simple shader mask");


Camera camera = { 0 };
camera.position = (Vector3){ 0.0f, 1.0f, 2.0f };
camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
camera.fovy = 45.0f;
camera.type = CAMERA_PERSPECTIVE;


Mesh torus = GenMeshTorus(.3, 1, 16, 32);
Model model1 = LoadModelFromMesh(torus);

Mesh cube = GenMeshCube(.8,.8,.8);
Model model2 = LoadModelFromMesh(cube);


Mesh sphere = GenMeshSphere(1, 16, 16);
Model model3 = LoadModelFromMesh(sphere);


Shader shader = LoadShader(0, FormatText("resources/shaders/glsl%i/mask.fs", GLSL_VERSION));


Texture texDiffuse = LoadTexture("resources/plasma.png");
model1.materials[0].maps[MAP_DIFFUSE].texture = texDiffuse;
model2.materials[0].maps[MAP_DIFFUSE].texture = texDiffuse;




Texture texMask = LoadTexture("resources/mask.png");
model1.materials[0].maps[MAP_EMISSION].texture = texMask;
model2.materials[0].maps[MAP_EMISSION].texture = texMask;
shader.locs[LOC_MAP_EMISSION] = GetShaderLocation(shader, "mask");


int shaderFrame = GetShaderLocation(shader, "frame");


model1.materials[0].shader = shader;
model2.materials[0].shader = shader;

int framesCounter = 0;
Vector3 rotation = { 0 }; 

SetTargetFPS(60); 



while (!WindowShouldClose()) 
{


framesCounter++;
rotation.x += 0.01f;
rotation.y += 0.005f;
rotation.z -= 0.0025f;


SetShaderValue(shader, shaderFrame, &framesCounter, UNIFORM_INT);


model1.transform = MatrixRotateXYZ(rotation);

UpdateCamera(&camera);




BeginDrawing();

ClearBackground(DARKBLUE);

BeginMode3D(camera);

DrawModel(model1, (Vector3){0.5,0,0}, 1, WHITE);
DrawModelEx(model2, (Vector3){-.5,0,0}, (Vector3){1,1,0}, 50, (Vector3){1,1,1}, WHITE);
DrawModel(model3,(Vector3){0,0,-1.5}, 1, WHITE);
DrawGrid(10, 1.0f); 

EndMode3D();

DrawRectangle(16, 698, MeasureText(FormatText("Frame: %i", framesCounter), 20) + 8, 42, BLUE);
DrawText(FormatText("Frame: %i", framesCounter), 20, 700, 20, WHITE);

DrawFPS(10, 10);

EndDrawing();

}



UnloadModel(model1);
UnloadModel(model2);
UnloadModel(model3);

UnloadTexture(texDiffuse); 
UnloadTexture(texMask); 

UnloadShader(shader); 

CloseWindow(); 


return 0;
}
