































#if !defined(RLIGHTS_H)
#define RLIGHTS_H

#include "raylib.h"




#define MAX_LIGHTS 4 
#define LIGHT_DISTANCE 3.5f 
#define LIGHT_HEIGHT 1.0f 




typedef enum {
LIGHT_DIRECTIONAL,
LIGHT_POINT
} LightType;

typedef struct {
bool enabled;
LightType type;
Vector3 position;
Vector3 target;
Color color;
int enabledLoc;
int typeLoc;
int posLoc;
int targetLoc;
int colorLoc;
} Light;

#if defined(__cplusplus)
extern "C" { 
#endif




void CreateLight(int type, Vector3 pos, Vector3 targ, Color color, Shader shader); 
void UpdateLightValues(Shader shader, Light light); 

#if defined(__cplusplus)
}
#endif

#endif 








#if defined(RLIGHTS_IMPLEMENTATION)

#include "raylib.h"














static Light lights[MAX_LIGHTS] = { 0 };
static int lightsCount = 0; 











void CreateLight(int type, Vector3 pos, Vector3 targ, Color color, Shader shader)
{
Light light = { 0 };

if (lightsCount < MAX_LIGHTS)
{
light.enabled = true;
light.type = type;
light.position = pos;
light.target = targ;
light.color = color;

char enabledName[32] = "lights[x].enabled\0";
char typeName[32] = "lights[x].type\0";
char posName[32] = "lights[x].position\0";
char targetName[32] = "lights[x].target\0";
char colorName[32] = "lights[x].color\0";
enabledName[7] = '0' + lightsCount;
typeName[7] = '0' + lightsCount;
posName[7] = '0' + lightsCount;
targetName[7] = '0' + lightsCount;
colorName[7] = '0' + lightsCount;

light.enabledLoc = GetShaderLocation(shader, enabledName);
light.typeLoc = GetShaderLocation(shader, typeName);
light.posLoc = GetShaderLocation(shader, posName);
light.targetLoc = GetShaderLocation(shader, targetName);
light.colorLoc = GetShaderLocation(shader, colorName);

UpdateLightValues(shader, light);

lights[lightsCount] = light;
lightsCount++;
}
}


void UpdateLightValues(Shader shader, Light light)
{

SetShaderValue(shader, light.enabledLoc, &light.enabled, UNIFORM_INT);
SetShaderValue(shader, light.typeLoc, &light.type, UNIFORM_INT);


float position[3] = { light.position.x, light.position.y, light.position.z };
SetShaderValue(shader, light.posLoc, position, UNIFORM_VEC3);


float target[3] = { light.target.x, light.target.y, light.target.z };
SetShaderValue(shader, light.targetLoc, target, UNIFORM_VEC3);


float diff[4] = { (float)light.color.r/(float)255, (float)light.color.g/(float)255, (float)light.color.b/(float)255, (float)light.color.a/(float)255 };
SetShaderValue(shader, light.colorLoc, diff, UNIFORM_VEC4);
}

#endif