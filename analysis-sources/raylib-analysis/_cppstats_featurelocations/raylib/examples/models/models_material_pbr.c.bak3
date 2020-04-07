













#include "raylib.h"
#include "raymath.h"

#include <stdio.h>

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

#define CUBEMAP_SIZE 512 
#define IRRADIANCE_SIZE 32 
#define PREFILTERED_SIZE 256 
#define BRDF_SIZE 512 


static Material LoadMaterialPBR(Color albedo, float metalness, float roughness);

int main(void)
{


const int screenWidth = 800;
const int screenHeight = 450;

SetConfigFlags(FLAG_MSAA_4X_HINT); 
InitWindow(screenWidth, screenHeight, "raylib [models] example - pbr material");


Camera camera = { 0 };
camera.position = (Vector3){ 4.0f, 4.0f, 4.0f }; 
camera.target = (Vector3){ 0.0f, 0.5f, 0.0f }; 
camera.up = (Vector3){ 0.0f, 1.0f, 0.0f }; 
camera.fovy = 45.0f; 
camera.type = CAMERA_PERSPECTIVE; 


Model model = LoadModel("resources/pbr/trooper.obj");



MeshTangents(&model.meshes[0]);

UnloadMaterial(model.materials[0]); 
model.materials[0] = LoadMaterialPBR((Color){ 255, 255, 255, 255 }, 1.0f, 1.0f);



CreateLight(LIGHT_POINT, (Vector3){ LIGHT_DISTANCE, LIGHT_HEIGHT, 0.0f }, (Vector3){ 0.0f, 0.0f, 0.0f }, (Color){ 255, 0, 0, 255 }, model.materials[0].shader);
CreateLight(LIGHT_POINT, (Vector3){ 0.0f, LIGHT_HEIGHT, LIGHT_DISTANCE }, (Vector3){ 0.0f, 0.0f, 0.0f }, (Color){ 0, 255, 0, 255 }, model.materials[0].shader);
CreateLight(LIGHT_POINT, (Vector3){ -LIGHT_DISTANCE, LIGHT_HEIGHT, 0.0f }, (Vector3){ 0.0f, 0.0f, 0.0f }, (Color){ 0, 0, 255, 255 }, model.materials[0].shader);
CreateLight(LIGHT_DIRECTIONAL, (Vector3){ 0.0f, LIGHT_HEIGHT*2.0f, -LIGHT_DISTANCE }, (Vector3){ 0.0f, 0.0f, 0.0f }, (Color){ 255, 0, 255, 255 }, model.materials[0].shader);

SetCameraMode(camera, CAMERA_ORBITAL); 

SetTargetFPS(60); 



while (!WindowShouldClose()) 
{


UpdateCamera(&camera); 


float cameraPos[3] = { camera.position.x, camera.position.y, camera.position.z };
SetShaderValue(model.materials[0].shader, model.materials[0].shader.locs[LOC_VECTOR_VIEW], cameraPos, UNIFORM_VEC3);




BeginDrawing();

ClearBackground(RAYWHITE);

BeginMode3D(camera);

DrawModel(model, Vector3Zero(), 1.0f, WHITE);

DrawGrid(10, 1.0f);

EndMode3D();

DrawFPS(10, 10);

EndDrawing();

}






UnloadTexture(model.materials[0].maps[MAP_ALBEDO].texture);
UnloadTexture(model.materials[0].maps[MAP_NORMAL].texture);
UnloadTexture(model.materials[0].maps[MAP_METALNESS].texture);
UnloadTexture(model.materials[0].maps[MAP_ROUGHNESS].texture);
UnloadTexture(model.materials[0].maps[MAP_OCCLUSION].texture);
UnloadTexture(model.materials[0].maps[MAP_IRRADIANCE].texture);
UnloadTexture(model.materials[0].maps[MAP_PREFILTER].texture);
UnloadTexture(model.materials[0].maps[MAP_BRDF].texture);
UnloadShader(model.materials[0].shader);

UnloadModel(model); 

CloseWindow(); 


return 0;
}



static Material LoadMaterialPBR(Color albedo, float metalness, float roughness)
{
Material mat = LoadMaterialDefault(); 

#if defined(PLATFORM_DESKTOP)
mat.shader = LoadShader("resources/shaders/glsl330/pbr.vs", "resources/shaders/glsl330/pbr.fs");
#else 
mat.shader = LoadShader("resources/shaders/glsl100/pbr.vs", "resources/shaders/glsl100/pbr.fs");
#endif



mat.shader.locs[LOC_MAP_ALBEDO] = GetShaderLocation(mat.shader, "albedo.sampler");
mat.shader.locs[LOC_MAP_METALNESS] = GetShaderLocation(mat.shader, "metalness.sampler");
mat.shader.locs[LOC_MAP_NORMAL] = GetShaderLocation(mat.shader, "normals.sampler");
mat.shader.locs[LOC_MAP_ROUGHNESS] = GetShaderLocation(mat.shader, "roughness.sampler");
mat.shader.locs[LOC_MAP_OCCLUSION] = GetShaderLocation(mat.shader, "occlusion.sampler");


mat.shader.locs[LOC_MAP_IRRADIANCE] = GetShaderLocation(mat.shader, "irradianceMap");
mat.shader.locs[LOC_MAP_PREFILTER] = GetShaderLocation(mat.shader, "prefilterMap");
mat.shader.locs[LOC_MAP_BRDF] = GetShaderLocation(mat.shader, "brdfLUT");


mat.shader.locs[LOC_MATRIX_MODEL] = GetShaderLocation(mat.shader, "matModel");

mat.shader.locs[LOC_VECTOR_VIEW] = GetShaderLocation(mat.shader, "viewPos");


mat.maps[MAP_ALBEDO].texture = LoadTexture("resources/pbr/trooper_albedo.png");
mat.maps[MAP_NORMAL].texture = LoadTexture("resources/pbr/trooper_normals.png");
mat.maps[MAP_METALNESS].texture = LoadTexture("resources/pbr/trooper_metalness.png");
mat.maps[MAP_ROUGHNESS].texture = LoadTexture("resources/pbr/trooper_roughness.png");
mat.maps[MAP_OCCLUSION].texture = LoadTexture("resources/pbr/trooper_ao.png");


#if defined(PLATFORM_DESKTOP)
Shader shdrCubemap = LoadShader("resources/shaders/glsl330/cubemap.vs", "resources/shaders/glsl330/cubemap.fs");
#else 
Shader shdrCubemap = LoadShader("resources/shaders/glsl100/cubemap.vs", "resources/shaders/glsl100/cubemap.fs");
#endif


#if defined(PLATFORM_DESKTOP)
Shader shdrIrradiance = LoadShader("resources/shaders/glsl330/skybox.vs", "resources/shaders/glsl330/irradiance.fs");
#else 
Shader shdrIrradiance = LoadShader("resources/shaders/glsl100/skybox.vs", "resources/shaders/glsl100/irradiance.fs");
#endif


#if defined(PLATFORM_DESKTOP)
Shader shdrPrefilter = LoadShader("resources/shaders/glsl330/skybox.vs", "resources/shaders/glsl330/prefilter.fs");
#else
Shader shdrPrefilter = LoadShader("resources/shaders/glsl100/skybox.vs", "resources/shaders/glsl100/prefilter.fs");
#endif


#if defined(PLATFORM_DESKTOP)
Shader shdrBRDF = LoadShader("resources/shaders/glsl330/brdf.vs", "resources/shaders/glsl330/brdf.fs");
#else
Shader shdrBRDF = LoadShader("resources/shaders/glsl100/brdf.vs", "resources/shaders/glsl100/brdf.fs");
#endif


SetShaderValue(shdrCubemap, GetShaderLocation(shdrCubemap, "equirectangularMap"), (int[1]){ 0 }, UNIFORM_INT);
SetShaderValue(shdrIrradiance, GetShaderLocation(shdrIrradiance, "environmentMap"), (int[1]){ 0 }, UNIFORM_INT);
SetShaderValue(shdrPrefilter, GetShaderLocation(shdrPrefilter, "environmentMap"), (int[1]){ 0 }, UNIFORM_INT);

Texture2D texHDR = LoadTexture("resources/dresden_square.hdr");
Texture2D cubemap = GenTextureCubemap(shdrCubemap, texHDR, CUBEMAP_SIZE);
mat.maps[MAP_IRRADIANCE].texture = GenTextureIrradiance(shdrIrradiance, cubemap, IRRADIANCE_SIZE);
mat.maps[MAP_PREFILTER].texture = GenTexturePrefilter(shdrPrefilter, cubemap, PREFILTERED_SIZE);
mat.maps[MAP_BRDF].texture = GenTextureBRDF(shdrBRDF, BRDF_SIZE);
UnloadTexture(cubemap);
UnloadTexture(texHDR);


UnloadShader(shdrCubemap);
UnloadShader(shdrIrradiance);
UnloadShader(shdrPrefilter);
UnloadShader(shdrBRDF);


SetTextureFilter(mat.maps[MAP_ALBEDO].texture, FILTER_BILINEAR);
SetTextureFilter(mat.maps[MAP_NORMAL].texture, FILTER_BILINEAR);
SetTextureFilter(mat.maps[MAP_METALNESS].texture, FILTER_BILINEAR);
SetTextureFilter(mat.maps[MAP_ROUGHNESS].texture, FILTER_BILINEAR);
SetTextureFilter(mat.maps[MAP_OCCLUSION].texture, FILTER_BILINEAR);


SetShaderValue(mat.shader, GetShaderLocation(mat.shader, "albedo.useSampler"), (int[1]){ 1 }, UNIFORM_INT);
SetShaderValue(mat.shader, GetShaderLocation(mat.shader, "normals.useSampler"), (int[1]){ 1 }, UNIFORM_INT);
SetShaderValue(mat.shader, GetShaderLocation(mat.shader, "metalness.useSampler"), (int[1]){ 1 }, UNIFORM_INT);
SetShaderValue(mat.shader, GetShaderLocation(mat.shader, "roughness.useSampler"), (int[1]){ 1 }, UNIFORM_INT);
SetShaderValue(mat.shader, GetShaderLocation(mat.shader, "occlusion.useSampler"), (int[1]){ 1 }, UNIFORM_INT);

int renderModeLoc = GetShaderLocation(mat.shader, "renderMode");
SetShaderValue(mat.shader, renderModeLoc, (int[1]){ 0 }, UNIFORM_INT);


mat.maps[MAP_ALBEDO].color = albedo;
mat.maps[MAP_NORMAL].color = (Color){ 128, 128, 255, 255 };
mat.maps[MAP_METALNESS].value = metalness;
mat.maps[MAP_ROUGHNESS].value = roughness;
mat.maps[MAP_OCCLUSION].value = 1.0f;
mat.maps[MAP_EMISSION].value = 0.5f;
mat.maps[MAP_HEIGHT].value = 0.5f;

return mat;
}
