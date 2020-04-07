





































#include "raylib.h" 


#if !defined(EXTERNAL_CONFIG_FLAGS)
#include "config.h" 
#endif

#include "utils.h" 

#include <stdlib.h> 
#include <stdio.h> 
#include <string.h> 
#include <math.h> 

#if defined(_WIN32)
#include <direct.h> 
#define CHDIR _chdir
#else
#include <unistd.h> 
#define CHDIR chdir
#endif

#include "rlgl.h" 

#if defined(SUPPORT_FILEFORMAT_OBJ) || defined(SUPPORT_FILEFORMAT_MTL)
#define TINYOBJ_MALLOC RL_MALLOC
#define TINYOBJ_CALLOC RL_CALLOC
#define TINYOBJ_REALLOC RL_REALLOC
#define TINYOBJ_FREE RL_FREE

#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include "external/tinyobj_loader_c.h" 
#endif

#if defined(SUPPORT_FILEFORMAT_GLTF)
#define CGLTF_MALLOC RL_MALLOC
#define CGLTF_FREE RL_FREE

#define CGLTF_IMPLEMENTATION
#include "external/cgltf.h" 
#include "external/stb_image.h" 
#endif

#if defined(SUPPORT_MESH_GENERATION)
#define PAR_MALLOC(T, N) ((T*)RL_MALLOC(N*sizeof(T)))
#define PAR_CALLOC(T, N) ((T*)RL_CALLOC(N*sizeof(T), 1))
#define PAR_REALLOC(T, BUF, N) ((T*)RL_REALLOC(BUF, sizeof(T)*(N)))
#define PAR_FREE RL_FREE

#define PAR_SHAPES_IMPLEMENTATION
#include "external/par_shapes.h" 
#endif




#define MAX_MESH_VBO 7 














#if defined(SUPPORT_FILEFORMAT_OBJ)
static Model LoadOBJ(const char *fileName); 
#endif
#if defined(SUPPORT_FILEFORMAT_IQM)
static Model LoadIQM(const char *fileName); 
#endif
#if defined(SUPPORT_FILEFORMAT_GLTF)
static Model LoadGLTF(const char *fileName); 
#endif






void DrawLine3D(Vector3 startPos, Vector3 endPos, Color color)
{
rlBegin(RL_LINES);
rlColor4ub(color.r, color.g, color.b, color.a);
rlVertex3f(startPos.x, startPos.y, startPos.z);
rlVertex3f(endPos.x, endPos.y, endPos.z);
rlEnd();
}


void DrawPoint3D(Vector3 position, Color color)
{
if (rlCheckBufferLimit(8)) rlglDraw();

rlPushMatrix();
rlTranslatef(position.x, position.y, position.z);
rlBegin(RL_LINES);
rlColor4ub(color.r, color.g, color.b, color.a);
rlVertex3f(0.0,0.0,0.0);
rlVertex3f(0.0,0.0,0.1);
rlEnd();
rlPopMatrix();
}


void DrawCircle3D(Vector3 center, float radius, Vector3 rotationAxis, float rotationAngle, Color color)
{
if (rlCheckBufferLimit(2*36)) rlglDraw();

rlPushMatrix();
rlTranslatef(center.x, center.y, center.z);
rlRotatef(rotationAngle, rotationAxis.x, rotationAxis.y, rotationAxis.z);

rlBegin(RL_LINES);
for (int i = 0; i < 360; i += 10)
{
rlColor4ub(color.r, color.g, color.b, color.a);

rlVertex3f(sinf(DEG2RAD*i)*radius, cosf(DEG2RAD*i)*radius, 0.0f);
rlVertex3f(sinf(DEG2RAD*(i + 10))*radius, cosf(DEG2RAD*(i + 10))*radius, 0.0f);
}
rlEnd();
rlPopMatrix();
}



void DrawCube(Vector3 position, float width, float height, float length, Color color)
{
float x = 0.0f;
float y = 0.0f;
float z = 0.0f;

if (rlCheckBufferLimit(36)) rlglDraw();

rlPushMatrix();

rlTranslatef(position.x, position.y, position.z);



rlBegin(RL_TRIANGLES);
rlColor4ub(color.r, color.g, color.b, color.a);


rlVertex3f(x - width/2, y - height/2, z + length/2); 
rlVertex3f(x + width/2, y - height/2, z + length/2); 
rlVertex3f(x - width/2, y + height/2, z + length/2); 

rlVertex3f(x + width/2, y + height/2, z + length/2); 
rlVertex3f(x - width/2, y + height/2, z + length/2); 
rlVertex3f(x + width/2, y - height/2, z + length/2); 


rlVertex3f(x - width/2, y - height/2, z - length/2); 
rlVertex3f(x - width/2, y + height/2, z - length/2); 
rlVertex3f(x + width/2, y - height/2, z - length/2); 

rlVertex3f(x + width/2, y + height/2, z - length/2); 
rlVertex3f(x + width/2, y - height/2, z - length/2); 
rlVertex3f(x - width/2, y + height/2, z - length/2); 


rlVertex3f(x - width/2, y + height/2, z - length/2); 
rlVertex3f(x - width/2, y + height/2, z + length/2); 
rlVertex3f(x + width/2, y + height/2, z + length/2); 

rlVertex3f(x + width/2, y + height/2, z - length/2); 
rlVertex3f(x - width/2, y + height/2, z - length/2); 
rlVertex3f(x + width/2, y + height/2, z + length/2); 


rlVertex3f(x - width/2, y - height/2, z - length/2); 
rlVertex3f(x + width/2, y - height/2, z + length/2); 
rlVertex3f(x - width/2, y - height/2, z + length/2); 

rlVertex3f(x + width/2, y - height/2, z - length/2); 
rlVertex3f(x + width/2, y - height/2, z + length/2); 
rlVertex3f(x - width/2, y - height/2, z - length/2); 


rlVertex3f(x + width/2, y - height/2, z - length/2); 
rlVertex3f(x + width/2, y + height/2, z - length/2); 
rlVertex3f(x + width/2, y + height/2, z + length/2); 

rlVertex3f(x + width/2, y - height/2, z + length/2); 
rlVertex3f(x + width/2, y - height/2, z - length/2); 
rlVertex3f(x + width/2, y + height/2, z + length/2); 


rlVertex3f(x - width/2, y - height/2, z - length/2); 
rlVertex3f(x - width/2, y + height/2, z + length/2); 
rlVertex3f(x - width/2, y + height/2, z - length/2); 

rlVertex3f(x - width/2, y - height/2, z + length/2); 
rlVertex3f(x - width/2, y + height/2, z + length/2); 
rlVertex3f(x - width/2, y - height/2, z - length/2); 
rlEnd();
rlPopMatrix();
}


void DrawCubeV(Vector3 position, Vector3 size, Color color)
{
DrawCube(position, size.x, size.y, size.z, color);
}


void DrawCubeWires(Vector3 position, float width, float height, float length, Color color)
{
float x = 0.0f;
float y = 0.0f;
float z = 0.0f;

if (rlCheckBufferLimit(36)) rlglDraw();

rlPushMatrix();
rlTranslatef(position.x, position.y, position.z);

rlBegin(RL_LINES);
rlColor4ub(color.r, color.g, color.b, color.a);



rlVertex3f(x-width/2, y-height/2, z+length/2); 
rlVertex3f(x+width/2, y-height/2, z+length/2); 


rlVertex3f(x+width/2, y-height/2, z+length/2); 
rlVertex3f(x+width/2, y+height/2, z+length/2); 


rlVertex3f(x+width/2, y+height/2, z+length/2); 
rlVertex3f(x-width/2, y+height/2, z+length/2); 


rlVertex3f(x-width/2, y+height/2, z+length/2); 
rlVertex3f(x-width/2, y-height/2, z+length/2); 



rlVertex3f(x-width/2, y-height/2, z-length/2); 
rlVertex3f(x+width/2, y-height/2, z-length/2); 


rlVertex3f(x+width/2, y-height/2, z-length/2); 
rlVertex3f(x+width/2, y+height/2, z-length/2); 


rlVertex3f(x+width/2, y+height/2, z-length/2); 
rlVertex3f(x-width/2, y+height/2, z-length/2); 


rlVertex3f(x-width/2, y+height/2, z-length/2); 
rlVertex3f(x-width/2, y-height/2, z-length/2); 



rlVertex3f(x-width/2, y+height/2, z+length/2); 
rlVertex3f(x-width/2, y+height/2, z-length/2); 


rlVertex3f(x+width/2, y+height/2, z+length/2); 
rlVertex3f(x+width/2, y+height/2, z-length/2); 



rlVertex3f(x-width/2, y-height/2, z+length/2); 
rlVertex3f(x-width/2, y-height/2, z-length/2); 


rlVertex3f(x+width/2, y-height/2, z+length/2); 
rlVertex3f(x+width/2, y-height/2, z-length/2); 
rlEnd();
rlPopMatrix();
}


void DrawCubeWiresV(Vector3 position, Vector3 size, Color color)
{
DrawCubeWires(position, size.x, size.y, size.z, color);
}



void DrawCubeTexture(Texture2D texture, Vector3 position, float width, float height, float length, Color color)
{
float x = position.x;
float y = position.y;
float z = position.z;

if (rlCheckBufferLimit(36)) rlglDraw();

rlEnableTexture(texture.id);







rlBegin(RL_QUADS);
rlColor4ub(color.r, color.g, color.b, color.a);

rlNormal3f(0.0f, 0.0f, 1.0f); 
rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z + length/2); 
rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z + length/2); 
rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z + length/2); 
rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z + length/2); 

rlNormal3f(0.0f, 0.0f, - 1.0f); 
rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z - length/2); 
rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z - length/2); 
rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z - length/2); 
rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z - length/2); 

rlNormal3f(0.0f, 1.0f, 0.0f); 
rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z - length/2); 
rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width/2, y + height/2, z + length/2); 
rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width/2, y + height/2, z + length/2); 
rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z - length/2); 

rlNormal3f(0.0f, - 1.0f, 0.0f); 
rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width/2, y - height/2, z - length/2); 
rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width/2, y - height/2, z - length/2); 
rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z + length/2); 
rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z + length/2); 

rlNormal3f(1.0f, 0.0f, 0.0f); 
rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z - length/2); 
rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z - length/2); 
rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z + length/2); 
rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z + length/2); 

rlNormal3f( - 1.0f, 0.0f, 0.0f); 
rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z - length/2); 
rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z + length/2); 
rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z + length/2); 
rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z - length/2); 
rlEnd();


rlDisableTexture();
}


void DrawSphere(Vector3 centerPos, float radius, Color color)
{
DrawSphereEx(centerPos, radius, 16, 16, color);
}


void DrawSphereEx(Vector3 centerPos, float radius, int rings, int slices, Color color)
{
int numVertex = (rings + 2)*slices*6;
if (rlCheckBufferLimit(numVertex)) rlglDraw();

rlPushMatrix();

rlTranslatef(centerPos.x, centerPos.y, centerPos.z);
rlScalef(radius, radius, radius);

rlBegin(RL_TRIANGLES);
rlColor4ub(color.r, color.g, color.b, color.a);

for (int i = 0; i < (rings + 2); i++)
{
for (int j = 0; j < slices; j++)
{
rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*i))*sinf(DEG2RAD*(j*360/slices)),
sinf(DEG2RAD*(270+(180/(rings + 1))*i)),
cosf(DEG2RAD*(270+(180/(rings + 1))*i))*cosf(DEG2RAD*(j*360/slices)));
rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(DEG2RAD*((j+1)*360/slices)),
sinf(DEG2RAD*(270+(180/(rings + 1))*(i+1))),
cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(DEG2RAD*((j+1)*360/slices)));
rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(DEG2RAD*(j*360/slices)),
sinf(DEG2RAD*(270+(180/(rings + 1))*(i+1))),
cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(DEG2RAD*(j*360/slices)));

rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*i))*sinf(DEG2RAD*(j*360/slices)),
sinf(DEG2RAD*(270+(180/(rings + 1))*i)),
cosf(DEG2RAD*(270+(180/(rings + 1))*i))*cosf(DEG2RAD*(j*360/slices)));
rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*(i)))*sinf(DEG2RAD*((j+1)*360/slices)),
sinf(DEG2RAD*(270+(180/(rings + 1))*(i))),
cosf(DEG2RAD*(270+(180/(rings + 1))*(i)))*cosf(DEG2RAD*((j+1)*360/slices)));
rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(DEG2RAD*((j+1)*360/slices)),
sinf(DEG2RAD*(270+(180/(rings + 1))*(i+1))),
cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(DEG2RAD*((j+1)*360/slices)));
}
}
rlEnd();
rlPopMatrix();
}


void DrawSphereWires(Vector3 centerPos, float radius, int rings, int slices, Color color)
{
int numVertex = (rings + 2)*slices*6;
if (rlCheckBufferLimit(numVertex)) rlglDraw();

rlPushMatrix();

rlTranslatef(centerPos.x, centerPos.y, centerPos.z);
rlScalef(radius, radius, radius);

rlBegin(RL_LINES);
rlColor4ub(color.r, color.g, color.b, color.a);

for (int i = 0; i < (rings + 2); i++)
{
for (int j = 0; j < slices; j++)
{
rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*i))*sinf(DEG2RAD*(j*360/slices)),
sinf(DEG2RAD*(270+(180/(rings + 1))*i)),
cosf(DEG2RAD*(270+(180/(rings + 1))*i))*cosf(DEG2RAD*(j*360/slices)));
rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(DEG2RAD*((j+1)*360/slices)),
sinf(DEG2RAD*(270+(180/(rings + 1))*(i+1))),
cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(DEG2RAD*((j+1)*360/slices)));

rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(DEG2RAD*((j+1)*360/slices)),
sinf(DEG2RAD*(270+(180/(rings + 1))*(i+1))),
cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(DEG2RAD*((j+1)*360/slices)));
rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(DEG2RAD*(j*360/slices)),
sinf(DEG2RAD*(270+(180/(rings + 1))*(i+1))),
cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(DEG2RAD*(j*360/slices)));

rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(DEG2RAD*(j*360/slices)),
sinf(DEG2RAD*(270+(180/(rings + 1))*(i+1))),
cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(DEG2RAD*(j*360/slices)));
rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*i))*sinf(DEG2RAD*(j*360/slices)),
sinf(DEG2RAD*(270+(180/(rings + 1))*i)),
cosf(DEG2RAD*(270+(180/(rings + 1))*i))*cosf(DEG2RAD*(j*360/slices)));
}
}
rlEnd();
rlPopMatrix();
}



void DrawCylinder(Vector3 position, float radiusTop, float radiusBottom, float height, int sides, Color color)
{
if (sides < 3) sides = 3;

int numVertex = sides*6;
if (rlCheckBufferLimit(numVertex)) rlglDraw();

rlPushMatrix();
rlTranslatef(position.x, position.y, position.z);

rlBegin(RL_TRIANGLES);
rlColor4ub(color.r, color.g, color.b, color.a);

if (radiusTop > 0)
{

for (int i = 0; i < 360; i += 360/sides)
{
rlVertex3f(sinf(DEG2RAD*i)*radiusBottom, 0, cosf(DEG2RAD*i)*radiusBottom); 
rlVertex3f(sinf(DEG2RAD*(i + 360/sides))*radiusBottom, 0, cosf(DEG2RAD*(i + 360/sides))*radiusBottom); 
rlVertex3f(sinf(DEG2RAD*(i + 360/sides))*radiusTop, height, cosf(DEG2RAD*(i + 360/sides))*radiusTop); 

rlVertex3f(sinf(DEG2RAD*i)*radiusTop, height, cosf(DEG2RAD*i)*radiusTop); 
rlVertex3f(sinf(DEG2RAD*i)*radiusBottom, 0, cosf(DEG2RAD*i)*radiusBottom); 
rlVertex3f(sinf(DEG2RAD*(i + 360/sides))*radiusTop, height, cosf(DEG2RAD*(i + 360/sides))*radiusTop); 
}


for (int i = 0; i < 360; i += 360/sides)
{
rlVertex3f(0, height, 0);
rlVertex3f(sinf(DEG2RAD*i)*radiusTop, height, cosf(DEG2RAD*i)*radiusTop);
rlVertex3f(sinf(DEG2RAD*(i + 360/sides))*radiusTop, height, cosf(DEG2RAD*(i + 360/sides))*radiusTop);
}
}
else
{

for (int i = 0; i < 360; i += 360/sides)
{
rlVertex3f(0, height, 0);
rlVertex3f(sinf(DEG2RAD*i)*radiusBottom, 0, cosf(DEG2RAD*i)*radiusBottom);
rlVertex3f(sinf(DEG2RAD*(i + 360/sides))*radiusBottom, 0, cosf(DEG2RAD*(i + 360/sides))*radiusBottom);
}
}


for (int i = 0; i < 360; i += 360/sides)
{
rlVertex3f(0, 0, 0);
rlVertex3f(sinf(DEG2RAD*(i + 360/sides))*radiusBottom, 0, cosf(DEG2RAD*(i + 360/sides))*radiusBottom);
rlVertex3f(sinf(DEG2RAD*i)*radiusBottom, 0, cosf(DEG2RAD*i)*radiusBottom);
}
rlEnd();
rlPopMatrix();
}



void DrawCylinderWires(Vector3 position, float radiusTop, float radiusBottom, float height, int sides, Color color)
{
if (sides < 3) sides = 3;

int numVertex = sides*8;
if (rlCheckBufferLimit(numVertex)) rlglDraw();

rlPushMatrix();
rlTranslatef(position.x, position.y, position.z);

rlBegin(RL_LINES);
rlColor4ub(color.r, color.g, color.b, color.a);

for (int i = 0; i < 360; i += 360/sides)
{
rlVertex3f(sinf(DEG2RAD*i)*radiusBottom, 0, cosf(DEG2RAD*i)*radiusBottom);
rlVertex3f(sinf(DEG2RAD*(i + 360/sides))*radiusBottom, 0, cosf(DEG2RAD*(i + 360/sides))*radiusBottom);

rlVertex3f(sinf(DEG2RAD*(i + 360/sides))*radiusBottom, 0, cosf(DEG2RAD*(i + 360/sides))*radiusBottom);
rlVertex3f(sinf(DEG2RAD*(i + 360/sides))*radiusTop, height, cosf(DEG2RAD*(i + 360/sides))*radiusTop);

rlVertex3f(sinf(DEG2RAD*(i + 360/sides))*radiusTop, height, cosf(DEG2RAD*(i + 360/sides))*radiusTop);
rlVertex3f(sinf(DEG2RAD*i)*radiusTop, height, cosf(DEG2RAD*i)*radiusTop);

rlVertex3f(sinf(DEG2RAD*i)*radiusTop, height, cosf(DEG2RAD*i)*radiusTop);
rlVertex3f(sinf(DEG2RAD*i)*radiusBottom, 0, cosf(DEG2RAD*i)*radiusBottom);
}
rlEnd();
rlPopMatrix();
}


void DrawPlane(Vector3 centerPos, Vector2 size, Color color)
{
if (rlCheckBufferLimit(4)) rlglDraw();


rlPushMatrix();
rlTranslatef(centerPos.x, centerPos.y, centerPos.z);
rlScalef(size.x, 1.0f, size.y);

rlBegin(RL_QUADS);
rlColor4ub(color.r, color.g, color.b, color.a);
rlNormal3f(0.0f, 1.0f, 0.0f);

rlVertex3f(-0.5f, 0.0f, -0.5f);
rlVertex3f(-0.5f, 0.0f, 0.5f);
rlVertex3f(0.5f, 0.0f, 0.5f);
rlVertex3f(0.5f, 0.0f, -0.5f);
rlEnd();
rlPopMatrix();
}


void DrawRay(Ray ray, Color color)
{
float scale = 10000;

rlBegin(RL_LINES);
rlColor4ub(color.r, color.g, color.b, color.a);
rlColor4ub(color.r, color.g, color.b, color.a);

rlVertex3f(ray.position.x, ray.position.y, ray.position.z);
rlVertex3f(ray.position.x + ray.direction.x*scale, ray.position.y + ray.direction.y*scale, ray.position.z + ray.direction.z*scale);
rlEnd();
}


void DrawGrid(int slices, float spacing)
{
int halfSlices = slices/2;

if (rlCheckBufferLimit(slices*4)) rlglDraw();

rlBegin(RL_LINES);
for (int i = -halfSlices; i <= halfSlices; i++)
{
if (i == 0)
{
rlColor3f(0.5f, 0.5f, 0.5f);
rlColor3f(0.5f, 0.5f, 0.5f);
rlColor3f(0.5f, 0.5f, 0.5f);
rlColor3f(0.5f, 0.5f, 0.5f);
}
else
{
rlColor3f(0.75f, 0.75f, 0.75f);
rlColor3f(0.75f, 0.75f, 0.75f);
rlColor3f(0.75f, 0.75f, 0.75f);
rlColor3f(0.75f, 0.75f, 0.75f);
}

rlVertex3f((float)i*spacing, 0.0f, (float)-halfSlices*spacing);
rlVertex3f((float)i*spacing, 0.0f, (float)halfSlices*spacing);

rlVertex3f((float)-halfSlices*spacing, 0.0f, (float)i*spacing);
rlVertex3f((float)halfSlices*spacing, 0.0f, (float)i*spacing);
}
rlEnd();
}


void DrawGizmo(Vector3 position)
{

float length = 1.0f;

rlPushMatrix();
rlTranslatef(position.x, position.y, position.z);
rlScalef(length, length, length);

rlBegin(RL_LINES);
rlColor3f(1.0f, 0.0f, 0.0f); rlVertex3f(0.0f, 0.0f, 0.0f);
rlColor3f(1.0f, 0.0f, 0.0f); rlVertex3f(1.0f, 0.0f, 0.0f);

rlColor3f(0.0f, 1.0f, 0.0f); rlVertex3f(0.0f, 0.0f, 0.0f);
rlColor3f(0.0f, 1.0f, 0.0f); rlVertex3f(0.0f, 1.0f, 0.0f);

rlColor3f(0.0f, 0.0f, 1.0f); rlVertex3f(0.0f, 0.0f, 0.0f);
rlColor3f(0.0f, 0.0f, 1.0f); rlVertex3f(0.0f, 0.0f, 1.0f);
rlEnd();
rlPopMatrix();
}


Model LoadModel(const char *fileName)
{
Model model = { 0 };

#if defined(SUPPORT_FILEFORMAT_OBJ)
if (IsFileExtension(fileName, ".obj")) model = LoadOBJ(fileName);
#endif
#if defined(SUPPORT_FILEFORMAT_IQM)
if (IsFileExtension(fileName, ".iqm")) model = LoadIQM(fileName);
#endif
#if defined(SUPPORT_FILEFORMAT_GLTF)
if (IsFileExtension(fileName, ".gltf") || IsFileExtension(fileName, ".glb")) model = LoadGLTF(fileName);
#endif


model.transform = MatrixIdentity();

if (model.meshCount == 0)
{
model.meshCount = 1;
model.meshes = (Mesh *)RL_CALLOC(model.meshCount, sizeof(Mesh));
#if defined(SUPPORT_MESH_GENERATION)
TRACELOG(LOG_WARNING, "MESH: [%s] Failed to load mesh data, default to cube mesh", fileName);
model.meshes[0] = GenMeshCube(1.0f, 1.0f, 1.0f);
#else
TRACELOG(LOG_WARNING, "MESH: [%s] Failed to load mesh data", fileName);
#endif
}
else
{

for (int i = 0; i < model.meshCount; i++) rlLoadMesh(&model.meshes[i], false);
}

if (model.materialCount == 0)
{
TRACELOG(LOG_WARNING, "MATERIAL: [%s] Failed to load material data, default to white material", fileName);

model.materialCount = 1;
model.materials = (Material *)RL_CALLOC(model.materialCount, sizeof(Material));
model.materials[0] = LoadMaterialDefault();

if (model.meshMaterial == NULL) model.meshMaterial = (int *)RL_CALLOC(model.meshCount, sizeof(int));
}

return model;
}





Model LoadModelFromMesh(Mesh mesh)
{
Model model = { 0 };

model.transform = MatrixIdentity();

model.meshCount = 1;
model.meshes = (Mesh *)RL_CALLOC(model.meshCount, sizeof(Mesh));
model.meshes[0] = mesh;

model.materialCount = 1;
model.materials = (Material *)RL_CALLOC(model.materialCount, sizeof(Material));
model.materials[0] = LoadMaterialDefault();

model.meshMaterial = (int *)RL_CALLOC(model.meshCount, sizeof(int));
model.meshMaterial[0] = 0; 

return model;
}


void UnloadModel(Model model)
{
for (int i = 0; i < model.meshCount; i++) UnloadMesh(model.meshes[i]);




for (int i = 0; i < model.materialCount; i++) RL_FREE(model.materials[i].maps);

RL_FREE(model.meshes);
RL_FREE(model.materials);
RL_FREE(model.meshMaterial);


RL_FREE(model.bones);
RL_FREE(model.bindPose);

TRACELOG(LOG_INFO, "MODEL: Unloaded model from RAM and VRAM");
}


Mesh *LoadMeshes(const char *fileName, int *meshCount)
{
Mesh *meshes = NULL;
int count = 0;



*meshCount = count;
return meshes;
}


void UnloadMesh(Mesh mesh)
{
rlUnloadMesh(mesh);
RL_FREE(mesh.vboId);
}


void ExportMesh(Mesh mesh, const char *fileName)
{
bool success = false;

if (IsFileExtension(fileName, ".obj"))
{
FILE *objFile = fopen(fileName, "wt");

fprintf(objFile, "#//////////////////////////////////////////////////////////////////////////////////\n");
fprintf(objFile, "#// //\n");
fprintf(objFile, "#// rMeshOBJ exporter v1.0 - Mesh exported as triangle faces and not optimized //\n");
fprintf(objFile, "#// //\n");
fprintf(objFile, "#// more info and bugs-report: github.com/raysan5/raylib //\n");
fprintf(objFile, "#// feedback and support: ray[at]raylib.com //\n");
fprintf(objFile, "#// //\n");
fprintf(objFile, "#// Copyright (c) 2018 Ramon Santamaria (@raysan5) //\n");
fprintf(objFile, "#// //\n");
fprintf(objFile, "#//////////////////////////////////////////////////////////////////////////////////\n\n");
fprintf(objFile, "#Vertex Count: %i\n", mesh.vertexCount);
fprintf(objFile, "#Triangle Count: %i\n\n", mesh.triangleCount);

fprintf(objFile, "g mesh\n");

for (int i = 0, v = 0; i < mesh.vertexCount; i++, v += 3)
{
fprintf(objFile, "v %.2f %.2f %.2f\n", mesh.vertices[v], mesh.vertices[v + 1], mesh.vertices[v + 2]);
}

for (int i = 0, v = 0; i < mesh.vertexCount; i++, v += 2)
{
fprintf(objFile, "vt %.2f %.2f\n", mesh.texcoords[v], mesh.texcoords[v + 1]);
}

for (int i = 0, v = 0; i < mesh.vertexCount; i++, v += 3)
{
fprintf(objFile, "vn %.2f %.2f %.2f\n", mesh.normals[v], mesh.normals[v + 1], mesh.normals[v + 2]);
}

for (int i = 0; i < mesh.triangleCount; i += 3)
{
fprintf(objFile, "f %i/%i/%i %i/%i/%i %i/%i/%i\n", i, i, i, i + 1, i + 1, i + 1, i + 2, i + 2, i + 2);
}

fprintf(objFile, "\n");

fclose(objFile);

success = true;
}
else if (IsFileExtension(fileName, ".raw")) { } 

if (success) TRACELOG(LOG_INFO, "FILEIO: [%s] Mesh exported successfully", fileName);
else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to export mesh data", fileName);
}


Material *LoadMaterials(const char *fileName, int *materialCount)
{
Material *materials = NULL;
unsigned int count = 0;



#if defined(SUPPORT_FILEFORMAT_MTL)
if (IsFileExtension(fileName, ".mtl"))
{
tinyobj_material_t *mats;

int result = tinyobj_parse_mtl_file(&mats, &count, fileName);
if (result != TINYOBJ_SUCCESS) {
TRACELOG(LOG_WARNING, "MATERIAL: [%s] Failed to parse materials file", fileName);
}



tinyobj_materials_free(mats, count);
}
#else
TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to load material file", fileName);
#endif


for (int i = 0; i < count; i++) materials[i].shader = GetShaderDefault();

*materialCount = count;
return materials;
}


Material LoadMaterialDefault(void)
{
Material material = { 0 };
material.maps = (MaterialMap *)RL_CALLOC(MAX_MATERIAL_MAPS, sizeof(MaterialMap));

material.shader = GetShaderDefault();
material.maps[MAP_DIFFUSE].texture = GetTextureDefault(); 



material.maps[MAP_DIFFUSE].color = WHITE; 
material.maps[MAP_SPECULAR].color = WHITE; 

return material;
}


void UnloadMaterial(Material material)
{

if (material.shader.id != GetShaderDefault().id) UnloadShader(material.shader);


for (int i = 0; i < MAX_MATERIAL_MAPS; i++)
{
if (material.maps[i].texture.id != GetTextureDefault().id) rlDeleteTextures(material.maps[i].texture.id);
}

RL_FREE(material.maps);
}



void SetMaterialTexture(Material *material, int mapType, Texture2D texture)
{
material->maps[mapType].texture = texture;
}


void SetModelMeshMaterial(Model *model, int meshId, int materialId)
{
if (meshId >= model->meshCount) TRACELOG(LOG_WARNING, "MESH: Id greater than mesh count");
else if (materialId >= model->materialCount) TRACELOG(LOG_WARNING, "MATERIAL: Id greater than material count");
else model->meshMaterial[meshId] = materialId;
}


ModelAnimation *LoadModelAnimations(const char *filename, int *animCount)
{
#define IQM_MAGIC "INTERQUAKEMODEL" 
#define IQM_VERSION 2 

typedef struct IQMHeader {
char magic[16];
unsigned int version;
unsigned int filesize;
unsigned int flags;
unsigned int num_text, ofs_text;
unsigned int num_meshes, ofs_meshes;
unsigned int num_vertexarrays, num_vertexes, ofs_vertexarrays;
unsigned int num_triangles, ofs_triangles, ofs_adjacency;
unsigned int num_joints, ofs_joints;
unsigned int num_poses, ofs_poses;
unsigned int num_anims, ofs_anims;
unsigned int num_frames, num_framechannels, ofs_frames, ofs_bounds;
unsigned int num_comment, ofs_comment;
unsigned int num_extensions, ofs_extensions;
} IQMHeader;

typedef struct IQMPose {
int parent;
unsigned int mask;
float channeloffset[10];
float channelscale[10];
} IQMPose;

typedef struct IQMAnim {
unsigned int name;
unsigned int first_frame, num_frames;
float framerate;
unsigned int flags;
} IQMAnim;

FILE *iqmFile = NULL;
IQMHeader iqm;

iqmFile = fopen(filename,"rb");

if (!iqmFile)
{
TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to open file", filename);
return NULL;
}


fread(&iqm, sizeof(IQMHeader), 1, iqmFile);

if (strncmp(iqm.magic, IQM_MAGIC, sizeof(IQM_MAGIC)))
{
TRACELOG(LOG_WARNING, "MODEL: [%s] IQM file is not a valid model", filename);
fclose(iqmFile);
return NULL;
}

if (iqm.version != IQM_VERSION)
{
TRACELOG(LOG_WARNING, "MODEL: [%s] IQM file version incorrect", filename);
fclose(iqmFile);
return NULL;
}


IQMPose *poses = RL_MALLOC(iqm.num_poses*sizeof(IQMPose));
fseek(iqmFile, iqm.ofs_poses, SEEK_SET);
fread(poses, iqm.num_poses*sizeof(IQMPose), 1, iqmFile);


*animCount = iqm.num_anims;
IQMAnim *anim = RL_MALLOC(iqm.num_anims*sizeof(IQMAnim));
fseek(iqmFile, iqm.ofs_anims, SEEK_SET);
fread(anim, iqm.num_anims*sizeof(IQMAnim), 1, iqmFile);
ModelAnimation *animations = RL_MALLOC(iqm.num_anims*sizeof(ModelAnimation));


unsigned short *framedata = RL_MALLOC(iqm.num_frames*iqm.num_framechannels*sizeof(unsigned short));
fseek(iqmFile, iqm.ofs_frames, SEEK_SET);
fread(framedata, iqm.num_frames*iqm.num_framechannels*sizeof(unsigned short), 1, iqmFile);

for (int a = 0; a < iqm.num_anims; a++)
{
animations[a].frameCount = anim[a].num_frames;
animations[a].boneCount = iqm.num_poses;
animations[a].bones = RL_MALLOC(iqm.num_poses*sizeof(BoneInfo));
animations[a].framePoses = RL_MALLOC(anim[a].num_frames*sizeof(Transform *));


for (int j = 0; j < iqm.num_poses; j++)
{
strcpy(animations[a].bones[j].name, "ANIMJOINTNAME");
animations[a].bones[j].parent = poses[j].parent;
}

for (int j = 0; j < anim[a].num_frames; j++) animations[a].framePoses[j] = RL_MALLOC(iqm.num_poses*sizeof(Transform));

int dcounter = anim[a].first_frame*iqm.num_framechannels;

for (int frame = 0; frame < anim[a].num_frames; frame++)
{
for (int i = 0; i < iqm.num_poses; i++)
{
animations[a].framePoses[frame][i].translation.x = poses[i].channeloffset[0];

if (poses[i].mask & 0x01)
{
animations[a].framePoses[frame][i].translation.x += framedata[dcounter]*poses[i].channelscale[0];
dcounter++;
}

animations[a].framePoses[frame][i].translation.y = poses[i].channeloffset[1];

if (poses[i].mask & 0x02)
{
animations[a].framePoses[frame][i].translation.y += framedata[dcounter]*poses[i].channelscale[1];
dcounter++;
}

animations[a].framePoses[frame][i].translation.z = poses[i].channeloffset[2];

if (poses[i].mask & 0x04)
{
animations[a].framePoses[frame][i].translation.z += framedata[dcounter]*poses[i].channelscale[2];
dcounter++;
}

animations[a].framePoses[frame][i].rotation.x = poses[i].channeloffset[3];

if (poses[i].mask & 0x08)
{
animations[a].framePoses[frame][i].rotation.x += framedata[dcounter]*poses[i].channelscale[3];
dcounter++;
}

animations[a].framePoses[frame][i].rotation.y = poses[i].channeloffset[4];

if (poses[i].mask & 0x10)
{
animations[a].framePoses[frame][i].rotation.y += framedata[dcounter]*poses[i].channelscale[4];
dcounter++;
}

animations[a].framePoses[frame][i].rotation.z = poses[i].channeloffset[5];

if (poses[i].mask & 0x20)
{
animations[a].framePoses[frame][i].rotation.z += framedata[dcounter]*poses[i].channelscale[5];
dcounter++;
}

animations[a].framePoses[frame][i].rotation.w = poses[i].channeloffset[6];

if (poses[i].mask & 0x40)
{
animations[a].framePoses[frame][i].rotation.w += framedata[dcounter]*poses[i].channelscale[6];
dcounter++;
}

animations[a].framePoses[frame][i].scale.x = poses[i].channeloffset[7];

if (poses[i].mask & 0x80)
{
animations[a].framePoses[frame][i].scale.x += framedata[dcounter]*poses[i].channelscale[7];
dcounter++;
}

animations[a].framePoses[frame][i].scale.y = poses[i].channeloffset[8];

if (poses[i].mask & 0x100)
{
animations[a].framePoses[frame][i].scale.y += framedata[dcounter]*poses[i].channelscale[8];
dcounter++;
}

animations[a].framePoses[frame][i].scale.z = poses[i].channeloffset[9];

if (poses[i].mask & 0x200)
{
animations[a].framePoses[frame][i].scale.z += framedata[dcounter]*poses[i].channelscale[9];
dcounter++;
}

animations[a].framePoses[frame][i].rotation = QuaternionNormalize(animations[a].framePoses[frame][i].rotation);
}
}


for (int frame = 0; frame < anim[a].num_frames; frame++)
{
for (int i = 0; i < animations[a].boneCount; i++)
{
if (animations[a].bones[i].parent >= 0)
{
animations[a].framePoses[frame][i].rotation = QuaternionMultiply(animations[a].framePoses[frame][animations[a].bones[i].parent].rotation, animations[a].framePoses[frame][i].rotation);
animations[a].framePoses[frame][i].translation = Vector3RotateByQuaternion(animations[a].framePoses[frame][i].translation, animations[a].framePoses[frame][animations[a].bones[i].parent].rotation);
animations[a].framePoses[frame][i].translation = Vector3Add(animations[a].framePoses[frame][i].translation, animations[a].framePoses[frame][animations[a].bones[i].parent].translation);
animations[a].framePoses[frame][i].scale = Vector3Multiply(animations[a].framePoses[frame][i].scale, animations[a].framePoses[frame][animations[a].bones[i].parent].scale);
}
}
}
}

RL_FREE(framedata);
RL_FREE(poses);
RL_FREE(anim);

fclose(iqmFile);

return animations;
}



void UpdateModelAnimation(Model model, ModelAnimation anim, int frame)
{
if ((anim.frameCount > 0) && (anim.bones != NULL) && (anim.framePoses != NULL))
{
if (frame >= anim.frameCount) frame = frame%anim.frameCount;

for (int m = 0; m < model.meshCount; m++)
{
Vector3 animVertex = { 0 };
Vector3 animNormal = { 0 };

Vector3 inTranslation = { 0 };
Quaternion inRotation = { 0 };


Vector3 outTranslation = { 0 };
Quaternion outRotation = { 0 };
Vector3 outScale = { 0 };

int vCounter = 0;
int boneCounter = 0;
int boneId = 0;

for (int i = 0; i < model.meshes[m].vertexCount; i++)
{
boneId = model.meshes[m].boneIds[boneCounter];
inTranslation = model.bindPose[boneId].translation;
inRotation = model.bindPose[boneId].rotation;

outTranslation = anim.framePoses[frame][boneId].translation;
outRotation = anim.framePoses[frame][boneId].rotation;
outScale = anim.framePoses[frame][boneId].scale;



animVertex = (Vector3){ model.meshes[m].vertices[vCounter], model.meshes[m].vertices[vCounter + 1], model.meshes[m].vertices[vCounter + 2] };
animVertex = Vector3Multiply(animVertex, outScale);
animVertex = Vector3Subtract(animVertex, inTranslation);
animVertex = Vector3RotateByQuaternion(animVertex, QuaternionMultiply(outRotation, QuaternionInvert(inRotation)));
animVertex = Vector3Add(animVertex, outTranslation);
model.meshes[m].animVertices[vCounter] = animVertex.x;
model.meshes[m].animVertices[vCounter + 1] = animVertex.y;
model.meshes[m].animVertices[vCounter + 2] = animVertex.z;



animNormal = (Vector3){ model.meshes[m].normals[vCounter], model.meshes[m].normals[vCounter + 1], model.meshes[m].normals[vCounter + 2] };
animNormal = Vector3RotateByQuaternion(animNormal, QuaternionMultiply(outRotation, QuaternionInvert(inRotation)));
model.meshes[m].animNormals[vCounter] = animNormal.x;
model.meshes[m].animNormals[vCounter + 1] = animNormal.y;
model.meshes[m].animNormals[vCounter + 2] = animNormal.z;
vCounter += 3;

boneCounter += 4;
}


rlUpdateBuffer(model.meshes[m].vboId[0], model.meshes[m].animVertices, model.meshes[m].vertexCount*3*sizeof(float)); 
rlUpdateBuffer(model.meshes[m].vboId[2], model.meshes[m].animNormals, model.meshes[m].vertexCount*3*sizeof(float)); 
}
}
}


void UnloadModelAnimation(ModelAnimation anim)
{
for (int i = 0; i < anim.frameCount; i++) RL_FREE(anim.framePoses[i]);

RL_FREE(anim.bones);
RL_FREE(anim.framePoses);
}



bool IsModelAnimationValid(Model model, ModelAnimation anim)
{
int result = true;

if (model.boneCount != anim.boneCount) result = false;
else
{
for (int i = 0; i < model.boneCount; i++)
{
if (model.bones[i].parent != anim.bones[i].parent) { result = false; break; }
}
}

return result;
}

#if defined(SUPPORT_MESH_GENERATION)

Mesh GenMeshPoly(int sides, float radius)
{
Mesh mesh = { 0 };
mesh.vboId = (unsigned int *)RL_CALLOC(MAX_MESH_VBO, sizeof(unsigned int));
int vertexCount = sides*3;


Vector3 *vertices = (Vector3 *)RL_MALLOC(vertexCount*sizeof(Vector3));
for (int i = 0, v = 0; i < 360; i += 360/sides, v += 3)
{
vertices[v] = (Vector3){ 0.0f, 0.0f, 0.0f };
vertices[v + 1] = (Vector3){ sinf(DEG2RAD*i)*radius, 0.0f, cosf(DEG2RAD*i)*radius };
vertices[v + 2] = (Vector3){ sinf(DEG2RAD*(i + 360/sides))*radius, 0.0f, cosf(DEG2RAD*(i + 360/sides))*radius };
}


Vector3 *normals = (Vector3 *)RL_MALLOC(vertexCount*sizeof(Vector3));
for (int n = 0; n < vertexCount; n++) normals[n] = (Vector3){ 0.0f, 1.0f, 0.0f }; 


Vector2 *texcoords = (Vector2 *)RL_MALLOC(vertexCount*sizeof(Vector2));
for (int n = 0; n < vertexCount; n++) texcoords[n] = (Vector2){ 0.0f, 0.0f };

mesh.vertexCount = vertexCount;
mesh.triangleCount = sides;
mesh.vertices = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
mesh.texcoords = (float *)RL_MALLOC(mesh.vertexCount*2*sizeof(float));
mesh.normals = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));


for (int i = 0; i < mesh.vertexCount; i++)
{
mesh.vertices[3*i] = vertices[i].x;
mesh.vertices[3*i + 1] = vertices[i].y;
mesh.vertices[3*i + 2] = vertices[i].z;
}


for (int i = 0; i < mesh.vertexCount; i++)
{
mesh.texcoords[2*i] = texcoords[i].x;
mesh.texcoords[2*i + 1] = texcoords[i].y;
}


for (int i = 0; i < mesh.vertexCount; i++)
{
mesh.normals[3*i] = normals[i].x;
mesh.normals[3*i + 1] = normals[i].y;
mesh.normals[3*i + 2] = normals[i].z;
}

RL_FREE(vertices);
RL_FREE(normals);
RL_FREE(texcoords);


rlLoadMesh(&mesh, false);

return mesh;
}


Mesh GenMeshPlane(float width, float length, int resX, int resZ)
{
Mesh mesh = { 0 };
mesh.vboId = (unsigned int *)RL_CALLOC(MAX_MESH_VBO, sizeof(unsigned int));

#define CUSTOM_MESH_GEN_PLANE
#if defined(CUSTOM_MESH_GEN_PLANE)
resX++;
resZ++;


int vertexCount = resX*resZ; 

Vector3 *vertices = (Vector3 *)RL_MALLOC(vertexCount*sizeof(Vector3));
for (int z = 0; z < resZ; z++)
{

float zPos = ((float)z/(resZ - 1) - 0.5f)*length;
for (int x = 0; x < resX; x++)
{

float xPos = ((float)x/(resX - 1) - 0.5f)*width;
vertices[x + z*resX] = (Vector3){ xPos, 0.0f, zPos };
}
}


Vector3 *normals = (Vector3 *)RL_MALLOC(vertexCount*sizeof(Vector3));
for (int n = 0; n < vertexCount; n++) normals[n] = (Vector3){ 0.0f, 1.0f, 0.0f }; 


Vector2 *texcoords = (Vector2 *)RL_MALLOC(vertexCount*sizeof(Vector2));
for (int v = 0; v < resZ; v++)
{
for (int u = 0; u < resX; u++)
{
texcoords[u + v*resX] = (Vector2){ (float)u/(resX - 1), (float)v/(resZ - 1) };
}
}


int numFaces = (resX - 1)*(resZ - 1);
int *triangles = (int *)RL_MALLOC(numFaces*6*sizeof(int));
int t = 0;
for (int face = 0; face < numFaces; face++)
{

int i = face % (resX - 1) + (face/(resZ - 1)*resX);

triangles[t++] = i + resX;
triangles[t++] = i + 1;
triangles[t++] = i;

triangles[t++] = i + resX;
triangles[t++] = i + resX + 1;
triangles[t++] = i + 1;
}

mesh.vertexCount = vertexCount;
mesh.triangleCount = numFaces*2;
mesh.vertices = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
mesh.texcoords = (float *)RL_MALLOC(mesh.vertexCount*2*sizeof(float));
mesh.normals = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
mesh.indices = (unsigned short *)RL_MALLOC(mesh.triangleCount*3*sizeof(unsigned short));


for (int i = 0; i < mesh.vertexCount; i++)
{
mesh.vertices[3*i] = vertices[i].x;
mesh.vertices[3*i + 1] = vertices[i].y;
mesh.vertices[3*i + 2] = vertices[i].z;
}


for (int i = 0; i < mesh.vertexCount; i++)
{
mesh.texcoords[2*i] = texcoords[i].x;
mesh.texcoords[2*i + 1] = texcoords[i].y;
}


for (int i = 0; i < mesh.vertexCount; i++)
{
mesh.normals[3*i] = normals[i].x;
mesh.normals[3*i + 1] = normals[i].y;
mesh.normals[3*i + 2] = normals[i].z;
}


for (int i = 0; i < mesh.triangleCount*3; i++) mesh.indices[i] = triangles[i];

RL_FREE(vertices);
RL_FREE(normals);
RL_FREE(texcoords);
RL_FREE(triangles);

#else 

par_shapes_mesh *plane = par_shapes_create_plane(resX, resZ); 
par_shapes_scale(plane, width, length, 1.0f);
par_shapes_rotate(plane, -PI/2.0f, (float[]){ 1, 0, 0 });
par_shapes_translate(plane, -width/2, 0.0f, length/2);

mesh.vertices = (float *)RL_MALLOC(plane->ntriangles*3*3*sizeof(float));
mesh.texcoords = (float *)RL_MALLOC(plane->ntriangles*3*2*sizeof(float));
mesh.normals = (float *)RL_MALLOC(plane->ntriangles*3*3*sizeof(float));
mesh.vboId = (unsigned int *)RL_CALLOC(MAX_MESH_VBO, sizeof(unsigned int));

mesh.vertexCount = plane->ntriangles*3;
mesh.triangleCount = plane->ntriangles;

for (int k = 0; k < mesh.vertexCount; k++)
{
mesh.vertices[k*3] = plane->points[plane->triangles[k]*3];
mesh.vertices[k*3 + 1] = plane->points[plane->triangles[k]*3 + 1];
mesh.vertices[k*3 + 2] = plane->points[plane->triangles[k]*3 + 2];

mesh.normals[k*3] = plane->normals[plane->triangles[k]*3];
mesh.normals[k*3 + 1] = plane->normals[plane->triangles[k]*3 + 1];
mesh.normals[k*3 + 2] = plane->normals[plane->triangles[k]*3 + 2];

mesh.texcoords[k*2] = plane->tcoords[plane->triangles[k]*2];
mesh.texcoords[k*2 + 1] = plane->tcoords[plane->triangles[k]*2 + 1];
}

par_shapes_free_mesh(plane);
#endif


rlLoadMesh(&mesh, false);

return mesh;
}


Mesh GenMeshCube(float width, float height, float length)
{
Mesh mesh = { 0 };
mesh.vboId = (unsigned int *)RL_CALLOC(MAX_MESH_VBO, sizeof(unsigned int));

#define CUSTOM_MESH_GEN_CUBE
#if defined(CUSTOM_MESH_GEN_CUBE)
float vertices[] = {
-width/2, -height/2, length/2,
width/2, -height/2, length/2,
width/2, height/2, length/2,
-width/2, height/2, length/2,
-width/2, -height/2, -length/2,
-width/2, height/2, -length/2,
width/2, height/2, -length/2,
width/2, -height/2, -length/2,
-width/2, height/2, -length/2,
-width/2, height/2, length/2,
width/2, height/2, length/2,
width/2, height/2, -length/2,
-width/2, -height/2, -length/2,
width/2, -height/2, -length/2,
width/2, -height/2, length/2,
-width/2, -height/2, length/2,
width/2, -height/2, -length/2,
width/2, height/2, -length/2,
width/2, height/2, length/2,
width/2, -height/2, length/2,
-width/2, -height/2, -length/2,
-width/2, -height/2, length/2,
-width/2, height/2, length/2,
-width/2, height/2, -length/2
};

float texcoords[] = {
0.0f, 0.0f,
1.0f, 0.0f,
1.0f, 1.0f,
0.0f, 1.0f,
1.0f, 0.0f,
1.0f, 1.0f,
0.0f, 1.0f,
0.0f, 0.0f,
0.0f, 1.0f,
0.0f, 0.0f,
1.0f, 0.0f,
1.0f, 1.0f,
1.0f, 1.0f,
0.0f, 1.0f,
0.0f, 0.0f,
1.0f, 0.0f,
1.0f, 0.0f,
1.0f, 1.0f,
0.0f, 1.0f,
0.0f, 0.0f,
0.0f, 0.0f,
1.0f, 0.0f,
1.0f, 1.0f,
0.0f, 1.0f
};

float normals[] = {
0.0f, 0.0f, 1.0f,
0.0f, 0.0f, 1.0f,
0.0f, 0.0f, 1.0f,
0.0f, 0.0f, 1.0f,
0.0f, 0.0f,-1.0f,
0.0f, 0.0f,-1.0f,
0.0f, 0.0f,-1.0f,
0.0f, 0.0f,-1.0f,
0.0f, 1.0f, 0.0f,
0.0f, 1.0f, 0.0f,
0.0f, 1.0f, 0.0f,
0.0f, 1.0f, 0.0f,
0.0f,-1.0f, 0.0f,
0.0f,-1.0f, 0.0f,
0.0f,-1.0f, 0.0f,
0.0f,-1.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
1.0f, 0.0f, 0.0f,
-1.0f, 0.0f, 0.0f,
-1.0f, 0.0f, 0.0f,
-1.0f, 0.0f, 0.0f,
-1.0f, 0.0f, 0.0f
};

mesh.vertices = (float *)RL_MALLOC(24*3*sizeof(float));
memcpy(mesh.vertices, vertices, 24*3*sizeof(float));

mesh.texcoords = (float *)RL_MALLOC(24*2*sizeof(float));
memcpy(mesh.texcoords, texcoords, 24*2*sizeof(float));

mesh.normals = (float *)RL_MALLOC(24*3*sizeof(float));
memcpy(mesh.normals, normals, 24*3*sizeof(float));

mesh.indices = (unsigned short *)RL_MALLOC(36*sizeof(unsigned short));

int k = 0;


for (int i = 0; i < 36; i+=6)
{
mesh.indices[i] = 4*k;
mesh.indices[i+1] = 4*k+1;
mesh.indices[i+2] = 4*k+2;
mesh.indices[i+3] = 4*k;
mesh.indices[i+4] = 4*k+2;
mesh.indices[i+5] = 4*k+3;

k++;
}

mesh.vertexCount = 24;
mesh.triangleCount = 12;

#else 










par_shapes_mesh *cube = par_shapes_create_cube();
cube->tcoords = PAR_MALLOC(float, 2*cube->npoints);
for (int i = 0; i < 2*cube->npoints; i++) cube->tcoords[i] = 0.0f;
par_shapes_scale(cube, width, height, length);
par_shapes_translate(cube, -width/2, 0.0f, -length/2);
par_shapes_compute_normals(cube);

mesh.vertices = (float *)RL_MALLOC(cube->ntriangles*3*3*sizeof(float));
mesh.texcoords = (float *)RL_MALLOC(cube->ntriangles*3*2*sizeof(float));
mesh.normals = (float *)RL_MALLOC(cube->ntriangles*3*3*sizeof(float));

mesh.vertexCount = cube->ntriangles*3;
mesh.triangleCount = cube->ntriangles;

for (int k = 0; k < mesh.vertexCount; k++)
{
mesh.vertices[k*3] = cube->points[cube->triangles[k]*3];
mesh.vertices[k*3 + 1] = cube->points[cube->triangles[k]*3 + 1];
mesh.vertices[k*3 + 2] = cube->points[cube->triangles[k]*3 + 2];

mesh.normals[k*3] = cube->normals[cube->triangles[k]*3];
mesh.normals[k*3 + 1] = cube->normals[cube->triangles[k]*3 + 1];
mesh.normals[k*3 + 2] = cube->normals[cube->triangles[k]*3 + 2];

mesh.texcoords[k*2] = cube->tcoords[cube->triangles[k]*2];
mesh.texcoords[k*2 + 1] = cube->tcoords[cube->triangles[k]*2 + 1];
}

par_shapes_free_mesh(cube);
#endif


rlLoadMesh(&mesh, false);

return mesh;
}


RLAPI Mesh GenMeshSphere(float radius, int rings, int slices)
{
Mesh mesh = { 0 };
mesh.vboId = (unsigned int *)RL_CALLOC(MAX_MESH_VBO, sizeof(unsigned int));

par_shapes_mesh *sphere = par_shapes_create_parametric_sphere(slices, rings);
par_shapes_scale(sphere, radius, radius, radius);


mesh.vertices = (float *)RL_MALLOC(sphere->ntriangles*3*3*sizeof(float));
mesh.texcoords = (float *)RL_MALLOC(sphere->ntriangles*3*2*sizeof(float));
mesh.normals = (float *)RL_MALLOC(sphere->ntriangles*3*3*sizeof(float));

mesh.vertexCount = sphere->ntriangles*3;
mesh.triangleCount = sphere->ntriangles;

for (int k = 0; k < mesh.vertexCount; k++)
{
mesh.vertices[k*3] = sphere->points[sphere->triangles[k]*3];
mesh.vertices[k*3 + 1] = sphere->points[sphere->triangles[k]*3 + 1];
mesh.vertices[k*3 + 2] = sphere->points[sphere->triangles[k]*3 + 2];

mesh.normals[k*3] = sphere->normals[sphere->triangles[k]*3];
mesh.normals[k*3 + 1] = sphere->normals[sphere->triangles[k]*3 + 1];
mesh.normals[k*3 + 2] = sphere->normals[sphere->triangles[k]*3 + 2];

mesh.texcoords[k*2] = sphere->tcoords[sphere->triangles[k]*2];
mesh.texcoords[k*2 + 1] = sphere->tcoords[sphere->triangles[k]*2 + 1];
}

par_shapes_free_mesh(sphere);


rlLoadMesh(&mesh, false);

return mesh;
}


RLAPI Mesh GenMeshHemiSphere(float radius, int rings, int slices)
{
Mesh mesh = { 0 };
mesh.vboId = (unsigned int *)RL_CALLOC(MAX_MESH_VBO, sizeof(unsigned int));

par_shapes_mesh *sphere = par_shapes_create_hemisphere(slices, rings);
par_shapes_scale(sphere, radius, radius, radius);


mesh.vertices = (float *)RL_MALLOC(sphere->ntriangles*3*3*sizeof(float));
mesh.texcoords = (float *)RL_MALLOC(sphere->ntriangles*3*2*sizeof(float));
mesh.normals = (float *)RL_MALLOC(sphere->ntriangles*3*3*sizeof(float));

mesh.vertexCount = sphere->ntriangles*3;
mesh.triangleCount = sphere->ntriangles;

for (int k = 0; k < mesh.vertexCount; k++)
{
mesh.vertices[k*3] = sphere->points[sphere->triangles[k]*3];
mesh.vertices[k*3 + 1] = sphere->points[sphere->triangles[k]*3 + 1];
mesh.vertices[k*3 + 2] = sphere->points[sphere->triangles[k]*3 + 2];

mesh.normals[k*3] = sphere->normals[sphere->triangles[k]*3];
mesh.normals[k*3 + 1] = sphere->normals[sphere->triangles[k]*3 + 1];
mesh.normals[k*3 + 2] = sphere->normals[sphere->triangles[k]*3 + 2];

mesh.texcoords[k*2] = sphere->tcoords[sphere->triangles[k]*2];
mesh.texcoords[k*2 + 1] = sphere->tcoords[sphere->triangles[k]*2 + 1];
}

par_shapes_free_mesh(sphere);


rlLoadMesh(&mesh, false);

return mesh;
}


Mesh GenMeshCylinder(float radius, float height, int slices)
{
Mesh mesh = { 0 };
mesh.vboId = (unsigned int *)RL_CALLOC(MAX_MESH_VBO, sizeof(unsigned int));





par_shapes_mesh *cylinder = par_shapes_create_cylinder(slices, 8);
par_shapes_scale(cylinder, radius, radius, height);
par_shapes_rotate(cylinder, -PI/2.0f, (float[]){ 1, 0, 0 });
par_shapes_rotate(cylinder, PI/2.0f, (float[]){ 0, 1, 0 });


par_shapes_mesh *capTop = par_shapes_create_disk(radius, slices, (float[]){ 0, 0, 0 }, (float[]){ 0, 0, 1 });
capTop->tcoords = PAR_MALLOC(float, 2*capTop->npoints);
for (int i = 0; i < 2*capTop->npoints; i++) capTop->tcoords[i] = 0.0f;
par_shapes_rotate(capTop, -PI/2.0f, (float[]){ 1, 0, 0 });
par_shapes_translate(capTop, 0, height, 0);


par_shapes_mesh *capBottom = par_shapes_create_disk(radius, slices, (float[]){ 0, 0, 0 }, (float[]){ 0, 0, -1 });
capBottom->tcoords = PAR_MALLOC(float, 2*capBottom->npoints);
for (int i = 0; i < 2*capBottom->npoints; i++) capBottom->tcoords[i] = 0.95f;
par_shapes_rotate(capBottom, PI/2.0f, (float[]){ 1, 0, 0 });

par_shapes_merge_and_free(cylinder, capTop);
par_shapes_merge_and_free(cylinder, capBottom);

mesh.vertices = (float *)RL_MALLOC(cylinder->ntriangles*3*3*sizeof(float));
mesh.texcoords = (float *)RL_MALLOC(cylinder->ntriangles*3*2*sizeof(float));
mesh.normals = (float *)RL_MALLOC(cylinder->ntriangles*3*3*sizeof(float));

mesh.vertexCount = cylinder->ntriangles*3;
mesh.triangleCount = cylinder->ntriangles;

for (int k = 0; k < mesh.vertexCount; k++)
{
mesh.vertices[k*3] = cylinder->points[cylinder->triangles[k]*3];
mesh.vertices[k*3 + 1] = cylinder->points[cylinder->triangles[k]*3 + 1];
mesh.vertices[k*3 + 2] = cylinder->points[cylinder->triangles[k]*3 + 2];

mesh.normals[k*3] = cylinder->normals[cylinder->triangles[k]*3];
mesh.normals[k*3 + 1] = cylinder->normals[cylinder->triangles[k]*3 + 1];
mesh.normals[k*3 + 2] = cylinder->normals[cylinder->triangles[k]*3 + 2];

mesh.texcoords[k*2] = cylinder->tcoords[cylinder->triangles[k]*2];
mesh.texcoords[k*2 + 1] = cylinder->tcoords[cylinder->triangles[k]*2 + 1];
}

par_shapes_free_mesh(cylinder);


rlLoadMesh(&mesh, false);

return mesh;
}


Mesh GenMeshTorus(float radius, float size, int radSeg, int sides)
{
Mesh mesh = { 0 };
mesh.vboId = (unsigned int *)RL_CALLOC(MAX_MESH_VBO, sizeof(unsigned int));

if (radius > 1.0f) radius = 1.0f;
else if (radius < 0.1f) radius = 0.1f;



par_shapes_mesh *torus = par_shapes_create_torus(radSeg, sides, radius);
par_shapes_scale(torus, size/2, size/2, size/2);

mesh.vertices = (float *)RL_MALLOC(torus->ntriangles*3*3*sizeof(float));
mesh.texcoords = (float *)RL_MALLOC(torus->ntriangles*3*2*sizeof(float));
mesh.normals = (float *)RL_MALLOC(torus->ntriangles*3*3*sizeof(float));

mesh.vertexCount = torus->ntriangles*3;
mesh.triangleCount = torus->ntriangles;

for (int k = 0; k < mesh.vertexCount; k++)
{
mesh.vertices[k*3] = torus->points[torus->triangles[k]*3];
mesh.vertices[k*3 + 1] = torus->points[torus->triangles[k]*3 + 1];
mesh.vertices[k*3 + 2] = torus->points[torus->triangles[k]*3 + 2];

mesh.normals[k*3] = torus->normals[torus->triangles[k]*3];
mesh.normals[k*3 + 1] = torus->normals[torus->triangles[k]*3 + 1];
mesh.normals[k*3 + 2] = torus->normals[torus->triangles[k]*3 + 2];

mesh.texcoords[k*2] = torus->tcoords[torus->triangles[k]*2];
mesh.texcoords[k*2 + 1] = torus->tcoords[torus->triangles[k]*2 + 1];
}

par_shapes_free_mesh(torus);


rlLoadMesh(&mesh, false);

return mesh;
}


Mesh GenMeshKnot(float radius, float size, int radSeg, int sides)
{
Mesh mesh = { 0 };
mesh.vboId = (unsigned int *)RL_CALLOC(MAX_MESH_VBO, sizeof(unsigned int));

if (radius > 3.0f) radius = 3.0f;
else if (radius < 0.5f) radius = 0.5f;

par_shapes_mesh *knot = par_shapes_create_trefoil_knot(radSeg, sides, radius);
par_shapes_scale(knot, size, size, size);

mesh.vertices = (float *)RL_MALLOC(knot->ntriangles*3*3*sizeof(float));
mesh.texcoords = (float *)RL_MALLOC(knot->ntriangles*3*2*sizeof(float));
mesh.normals = (float *)RL_MALLOC(knot->ntriangles*3*3*sizeof(float));

mesh.vertexCount = knot->ntriangles*3;
mesh.triangleCount = knot->ntriangles;

for (int k = 0; k < mesh.vertexCount; k++)
{
mesh.vertices[k*3] = knot->points[knot->triangles[k]*3];
mesh.vertices[k*3 + 1] = knot->points[knot->triangles[k]*3 + 1];
mesh.vertices[k*3 + 2] = knot->points[knot->triangles[k]*3 + 2];

mesh.normals[k*3] = knot->normals[knot->triangles[k]*3];
mesh.normals[k*3 + 1] = knot->normals[knot->triangles[k]*3 + 1];
mesh.normals[k*3 + 2] = knot->normals[knot->triangles[k]*3 + 2];

mesh.texcoords[k*2] = knot->tcoords[knot->triangles[k]*2];
mesh.texcoords[k*2 + 1] = knot->tcoords[knot->triangles[k]*2 + 1];
}

par_shapes_free_mesh(knot);


rlLoadMesh(&mesh, false);

return mesh;
}



Mesh GenMeshHeightmap(Image heightmap, Vector3 size)
{
#define GRAY_VALUE(c) ((c.r+c.g+c.b)/3)

Mesh mesh = { 0 };
mesh.vboId = (unsigned int *)RL_CALLOC(MAX_MESH_VBO, sizeof(unsigned int));

int mapX = heightmap.width;
int mapZ = heightmap.height;

Color *pixels = GetImageData(heightmap);


mesh.triangleCount = (mapX-1)*(mapZ-1)*2; 

mesh.vertexCount = mesh.triangleCount*3;

mesh.vertices = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
mesh.normals = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
mesh.texcoords = (float *)RL_MALLOC(mesh.vertexCount*2*sizeof(float));
mesh.colors = NULL;

int vCounter = 0; 
int tcCounter = 0; 
int nCounter = 0; 

int trisCounter = 0;

Vector3 scaleFactor = { size.x/mapX, size.y/255.0f, size.z/mapZ };

Vector3 vA;
Vector3 vB;
Vector3 vC;
Vector3 vN;

for (int z = 0; z < mapZ-1; z++)
{
for (int x = 0; x < mapX-1; x++)
{




mesh.vertices[vCounter] = (float)x*scaleFactor.x;
mesh.vertices[vCounter + 1] = (float)GRAY_VALUE(pixels[x + z*mapX])*scaleFactor.y;
mesh.vertices[vCounter + 2] = (float)z*scaleFactor.z;

mesh.vertices[vCounter + 3] = (float)x*scaleFactor.x;
mesh.vertices[vCounter + 4] = (float)GRAY_VALUE(pixels[x + (z + 1)*mapX])*scaleFactor.y;
mesh.vertices[vCounter + 5] = (float)(z + 1)*scaleFactor.z;

mesh.vertices[vCounter + 6] = (float)(x + 1)*scaleFactor.x;
mesh.vertices[vCounter + 7] = (float)GRAY_VALUE(pixels[(x + 1) + z*mapX])*scaleFactor.y;
mesh.vertices[vCounter + 8] = (float)z*scaleFactor.z;


mesh.vertices[vCounter + 9] = mesh.vertices[vCounter + 6];
mesh.vertices[vCounter + 10] = mesh.vertices[vCounter + 7];
mesh.vertices[vCounter + 11] = mesh.vertices[vCounter + 8];

mesh.vertices[vCounter + 12] = mesh.vertices[vCounter + 3];
mesh.vertices[vCounter + 13] = mesh.vertices[vCounter + 4];
mesh.vertices[vCounter + 14] = mesh.vertices[vCounter + 5];

mesh.vertices[vCounter + 15] = (float)(x + 1)*scaleFactor.x;
mesh.vertices[vCounter + 16] = (float)GRAY_VALUE(pixels[(x + 1) + (z + 1)*mapX])*scaleFactor.y;
mesh.vertices[vCounter + 17] = (float)(z + 1)*scaleFactor.z;
vCounter += 18; 



mesh.texcoords[tcCounter] = (float)x/(mapX - 1);
mesh.texcoords[tcCounter + 1] = (float)z/(mapZ - 1);

mesh.texcoords[tcCounter + 2] = (float)x/(mapX - 1);
mesh.texcoords[tcCounter + 3] = (float)(z + 1)/(mapZ - 1);

mesh.texcoords[tcCounter + 4] = (float)(x + 1)/(mapX - 1);
mesh.texcoords[tcCounter + 5] = (float)z/(mapZ - 1);

mesh.texcoords[tcCounter + 6] = mesh.texcoords[tcCounter + 4];
mesh.texcoords[tcCounter + 7] = mesh.texcoords[tcCounter + 5];

mesh.texcoords[tcCounter + 8] = mesh.texcoords[tcCounter + 2];
mesh.texcoords[tcCounter + 9] = mesh.texcoords[tcCounter + 3];

mesh.texcoords[tcCounter + 10] = (float)(x + 1)/(mapX - 1);
mesh.texcoords[tcCounter + 11] = (float)(z + 1)/(mapZ - 1);
tcCounter += 12; 



for (int i = 0; i < 18; i += 9)
{
vA.x = mesh.vertices[nCounter + i];
vA.y = mesh.vertices[nCounter + i + 1];
vA.z = mesh.vertices[nCounter + i + 2];

vB.x = mesh.vertices[nCounter + i + 3];
vB.y = mesh.vertices[nCounter + i + 4];
vB.z = mesh.vertices[nCounter + i + 5];

vC.x = mesh.vertices[nCounter + i + 6];
vC.y = mesh.vertices[nCounter + i + 7];
vC.z = mesh.vertices[nCounter + i + 8];

vN = Vector3Normalize(Vector3CrossProduct(Vector3Subtract(vB, vA), Vector3Subtract(vC, vA)));

mesh.normals[nCounter + i] = vN.x;
mesh.normals[nCounter + i + 1] = vN.y;
mesh.normals[nCounter + i + 2] = vN.z;

mesh.normals[nCounter + i + 3] = vN.x;
mesh.normals[nCounter + i + 4] = vN.y;
mesh.normals[nCounter + i + 5] = vN.z;

mesh.normals[nCounter + i + 6] = vN.x;
mesh.normals[nCounter + i + 7] = vN.y;
mesh.normals[nCounter + i + 8] = vN.z;
}

nCounter += 18; 
trisCounter += 2;
}
}

RL_FREE(pixels);


rlLoadMesh(&mesh, false);

return mesh;
}



Mesh GenMeshCubicmap(Image cubicmap, Vector3 cubeSize)
{
Mesh mesh = { 0 };
mesh.vboId = (unsigned int *)RL_CALLOC(MAX_MESH_VBO, sizeof(unsigned int));

Color *cubicmapPixels = GetImageData(cubicmap);

int mapWidth = cubicmap.width;
int mapHeight = cubicmap.height;


int maxTriangles = cubicmap.width*cubicmap.height*12;

int vCounter = 0; 
int tcCounter = 0; 
int nCounter = 0; 

float w = cubeSize.x;
float h = cubeSize.z;
float h2 = cubeSize.y;

Vector3 *mapVertices = (Vector3 *)RL_MALLOC(maxTriangles*3*sizeof(Vector3));
Vector2 *mapTexcoords = (Vector2 *)RL_MALLOC(maxTriangles*3*sizeof(Vector2));
Vector3 *mapNormals = (Vector3 *)RL_MALLOC(maxTriangles*3*sizeof(Vector3));


Vector3 n1 = { 1.0f, 0.0f, 0.0f };
Vector3 n2 = { -1.0f, 0.0f, 0.0f };
Vector3 n3 = { 0.0f, 1.0f, 0.0f };
Vector3 n4 = { 0.0f, -1.0f, 0.0f };
Vector3 n5 = { 0.0f, 0.0f, 1.0f };
Vector3 n6 = { 0.0f, 0.0f, -1.0f };


typedef struct RectangleF {
float x;
float y;
float width;
float height;
} RectangleF;

RectangleF rightTexUV = { 0.0f, 0.0f, 0.5f, 0.5f };
RectangleF leftTexUV = { 0.5f, 0.0f, 0.5f, 0.5f };
RectangleF frontTexUV = { 0.0f, 0.0f, 0.5f, 0.5f };
RectangleF backTexUV = { 0.5f, 0.0f, 0.5f, 0.5f };
RectangleF topTexUV = { 0.0f, 0.5f, 0.5f, 0.5f };
RectangleF bottomTexUV = { 0.5f, 0.5f, 0.5f, 0.5f };

for (int z = 0; z < mapHeight; ++z)
{
for (int x = 0; x < mapWidth; ++x)
{

Vector3 v1 = { w*(x - 0.5f), h2, h*(z - 0.5f) };
Vector3 v2 = { w*(x - 0.5f), h2, h*(z + 0.5f) };
Vector3 v3 = { w*(x + 0.5f), h2, h*(z + 0.5f) };
Vector3 v4 = { w*(x + 0.5f), h2, h*(z - 0.5f) };
Vector3 v5 = { w*(x + 0.5f), 0, h*(z - 0.5f) };
Vector3 v6 = { w*(x - 0.5f), 0, h*(z - 0.5f) };
Vector3 v7 = { w*(x - 0.5f), 0, h*(z + 0.5f) };
Vector3 v8 = { w*(x + 0.5f), 0, h*(z + 0.5f) };


if ((cubicmapPixels[z*cubicmap.width + x].r == 255) &&
(cubicmapPixels[z*cubicmap.width + x].g == 255) &&
(cubicmapPixels[z*cubicmap.width + x].b == 255))
{




mapVertices[vCounter] = v1;
mapVertices[vCounter + 1] = v2;
mapVertices[vCounter + 2] = v3;
mapVertices[vCounter + 3] = v1;
mapVertices[vCounter + 4] = v3;
mapVertices[vCounter + 5] = v4;
vCounter += 6;

mapNormals[nCounter] = n3;
mapNormals[nCounter + 1] = n3;
mapNormals[nCounter + 2] = n3;
mapNormals[nCounter + 3] = n3;
mapNormals[nCounter + 4] = n3;
mapNormals[nCounter + 5] = n3;
nCounter += 6;

mapTexcoords[tcCounter] = (Vector2){ topTexUV.x, topTexUV.y };
mapTexcoords[tcCounter + 1] = (Vector2){ topTexUV.x, topTexUV.y + topTexUV.height };
mapTexcoords[tcCounter + 2] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y + topTexUV.height };
mapTexcoords[tcCounter + 3] = (Vector2){ topTexUV.x, topTexUV.y };
mapTexcoords[tcCounter + 4] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y + topTexUV.height };
mapTexcoords[tcCounter + 5] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y };
tcCounter += 6;


mapVertices[vCounter] = v6;
mapVertices[vCounter + 1] = v8;
mapVertices[vCounter + 2] = v7;
mapVertices[vCounter + 3] = v6;
mapVertices[vCounter + 4] = v5;
mapVertices[vCounter + 5] = v8;
vCounter += 6;

mapNormals[nCounter] = n4;
mapNormals[nCounter + 1] = n4;
mapNormals[nCounter + 2] = n4;
mapNormals[nCounter + 3] = n4;
mapNormals[nCounter + 4] = n4;
mapNormals[nCounter + 5] = n4;
nCounter += 6;

mapTexcoords[tcCounter] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y };
mapTexcoords[tcCounter + 1] = (Vector2){ bottomTexUV.x, bottomTexUV.y + bottomTexUV.height };
mapTexcoords[tcCounter + 2] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y + bottomTexUV.height };
mapTexcoords[tcCounter + 3] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y };
mapTexcoords[tcCounter + 4] = (Vector2){ bottomTexUV.x, bottomTexUV.y };
mapTexcoords[tcCounter + 5] = (Vector2){ bottomTexUV.x, bottomTexUV.y + bottomTexUV.height };
tcCounter += 6;

if (((z < cubicmap.height - 1) &&
(cubicmapPixels[(z + 1)*cubicmap.width + x].r == 0) &&
(cubicmapPixels[(z + 1)*cubicmap.width + x].g == 0) &&
(cubicmapPixels[(z + 1)*cubicmap.width + x].b == 0)) || (z == cubicmap.height - 1))
{


mapVertices[vCounter] = v2;
mapVertices[vCounter + 1] = v7;
mapVertices[vCounter + 2] = v3;
mapVertices[vCounter + 3] = v3;
mapVertices[vCounter + 4] = v7;
mapVertices[vCounter + 5] = v8;
vCounter += 6;

mapNormals[nCounter] = n6;
mapNormals[nCounter + 1] = n6;
mapNormals[nCounter + 2] = n6;
mapNormals[nCounter + 3] = n6;
mapNormals[nCounter + 4] = n6;
mapNormals[nCounter + 5] = n6;
nCounter += 6;

mapTexcoords[tcCounter] = (Vector2){ frontTexUV.x, frontTexUV.y };
mapTexcoords[tcCounter + 1] = (Vector2){ frontTexUV.x, frontTexUV.y + frontTexUV.height };
mapTexcoords[tcCounter + 2] = (Vector2){ frontTexUV.x + frontTexUV.width, frontTexUV.y };
mapTexcoords[tcCounter + 3] = (Vector2){ frontTexUV.x + frontTexUV.width, frontTexUV.y };
mapTexcoords[tcCounter + 4] = (Vector2){ frontTexUV.x, frontTexUV.y + frontTexUV.height };
mapTexcoords[tcCounter + 5] = (Vector2){ frontTexUV.x + frontTexUV.width, frontTexUV.y + frontTexUV.height };
tcCounter += 6;
}

if (((z > 0) &&
(cubicmapPixels[(z - 1)*cubicmap.width + x].r == 0) &&
(cubicmapPixels[(z - 1)*cubicmap.width + x].g == 0) &&
(cubicmapPixels[(z - 1)*cubicmap.width + x].b == 0)) || (z == 0))
{


mapVertices[vCounter] = v1;
mapVertices[vCounter + 1] = v5;
mapVertices[vCounter + 2] = v6;
mapVertices[vCounter + 3] = v1;
mapVertices[vCounter + 4] = v4;
mapVertices[vCounter + 5] = v5;
vCounter += 6;

mapNormals[nCounter] = n5;
mapNormals[nCounter + 1] = n5;
mapNormals[nCounter + 2] = n5;
mapNormals[nCounter + 3] = n5;
mapNormals[nCounter + 4] = n5;
mapNormals[nCounter + 5] = n5;
nCounter += 6;

mapTexcoords[tcCounter] = (Vector2){ backTexUV.x + backTexUV.width, backTexUV.y };
mapTexcoords[tcCounter + 1] = (Vector2){ backTexUV.x, backTexUV.y + backTexUV.height };
mapTexcoords[tcCounter + 2] = (Vector2){ backTexUV.x + backTexUV.width, backTexUV.y + backTexUV.height };
mapTexcoords[tcCounter + 3] = (Vector2){ backTexUV.x + backTexUV.width, backTexUV.y };
mapTexcoords[tcCounter + 4] = (Vector2){ backTexUV.x, backTexUV.y };
mapTexcoords[tcCounter + 5] = (Vector2){ backTexUV.x, backTexUV.y + backTexUV.height };
tcCounter += 6;
}

if (((x < cubicmap.width - 1) &&
(cubicmapPixels[z*cubicmap.width + (x + 1)].r == 0) &&
(cubicmapPixels[z*cubicmap.width + (x + 1)].g == 0) &&
(cubicmapPixels[z*cubicmap.width + (x + 1)].b == 0)) || (x == cubicmap.width - 1))
{


mapVertices[vCounter] = v3;
mapVertices[vCounter + 1] = v8;
mapVertices[vCounter + 2] = v4;
mapVertices[vCounter + 3] = v4;
mapVertices[vCounter + 4] = v8;
mapVertices[vCounter + 5] = v5;
vCounter += 6;

mapNormals[nCounter] = n1;
mapNormals[nCounter + 1] = n1;
mapNormals[nCounter + 2] = n1;
mapNormals[nCounter + 3] = n1;
mapNormals[nCounter + 4] = n1;
mapNormals[nCounter + 5] = n1;
nCounter += 6;

mapTexcoords[tcCounter] = (Vector2){ rightTexUV.x, rightTexUV.y };
mapTexcoords[tcCounter + 1] = (Vector2){ rightTexUV.x, rightTexUV.y + rightTexUV.height };
mapTexcoords[tcCounter + 2] = (Vector2){ rightTexUV.x + rightTexUV.width, rightTexUV.y };
mapTexcoords[tcCounter + 3] = (Vector2){ rightTexUV.x + rightTexUV.width, rightTexUV.y };
mapTexcoords[tcCounter + 4] = (Vector2){ rightTexUV.x, rightTexUV.y + rightTexUV.height };
mapTexcoords[tcCounter + 5] = (Vector2){ rightTexUV.x + rightTexUV.width, rightTexUV.y + rightTexUV.height };
tcCounter += 6;
}

if (((x > 0) &&
(cubicmapPixels[z*cubicmap.width + (x - 1)].r == 0) &&
(cubicmapPixels[z*cubicmap.width + (x - 1)].g == 0) &&
(cubicmapPixels[z*cubicmap.width + (x - 1)].b == 0)) || (x == 0))
{


mapVertices[vCounter] = v1;
mapVertices[vCounter + 1] = v7;
mapVertices[vCounter + 2] = v2;
mapVertices[vCounter + 3] = v1;
mapVertices[vCounter + 4] = v6;
mapVertices[vCounter + 5] = v7;
vCounter += 6;

mapNormals[nCounter] = n2;
mapNormals[nCounter + 1] = n2;
mapNormals[nCounter + 2] = n2;
mapNormals[nCounter + 3] = n2;
mapNormals[nCounter + 4] = n2;
mapNormals[nCounter + 5] = n2;
nCounter += 6;

mapTexcoords[tcCounter] = (Vector2){ leftTexUV.x, leftTexUV.y };
mapTexcoords[tcCounter + 1] = (Vector2){ leftTexUV.x + leftTexUV.width, leftTexUV.y + leftTexUV.height };
mapTexcoords[tcCounter + 2] = (Vector2){ leftTexUV.x + leftTexUV.width, leftTexUV.y };
mapTexcoords[tcCounter + 3] = (Vector2){ leftTexUV.x, leftTexUV.y };
mapTexcoords[tcCounter + 4] = (Vector2){ leftTexUV.x, leftTexUV.y + leftTexUV.height };
mapTexcoords[tcCounter + 5] = (Vector2){ leftTexUV.x + leftTexUV.width, leftTexUV.y + leftTexUV.height };
tcCounter += 6;
}
}

else if ((cubicmapPixels[z*cubicmap.width + x].r == 0) &&
(cubicmapPixels[z*cubicmap.width + x].g == 0) &&
(cubicmapPixels[z*cubicmap.width + x].b == 0))
{

mapVertices[vCounter] = v1;
mapVertices[vCounter + 1] = v3;
mapVertices[vCounter + 2] = v2;
mapVertices[vCounter + 3] = v1;
mapVertices[vCounter + 4] = v4;
mapVertices[vCounter + 5] = v3;
vCounter += 6;

mapNormals[nCounter] = n4;
mapNormals[nCounter + 1] = n4;
mapNormals[nCounter + 2] = n4;
mapNormals[nCounter + 3] = n4;
mapNormals[nCounter + 4] = n4;
mapNormals[nCounter + 5] = n4;
nCounter += 6;

mapTexcoords[tcCounter] = (Vector2){ topTexUV.x, topTexUV.y };
mapTexcoords[tcCounter + 1] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y + topTexUV.height };
mapTexcoords[tcCounter + 2] = (Vector2){ topTexUV.x, topTexUV.y + topTexUV.height };
mapTexcoords[tcCounter + 3] = (Vector2){ topTexUV.x, topTexUV.y };
mapTexcoords[tcCounter + 4] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y };
mapTexcoords[tcCounter + 5] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y + topTexUV.height };
tcCounter += 6;


mapVertices[vCounter] = v6;
mapVertices[vCounter + 1] = v7;
mapVertices[vCounter + 2] = v8;
mapVertices[vCounter + 3] = v6;
mapVertices[vCounter + 4] = v8;
mapVertices[vCounter + 5] = v5;
vCounter += 6;

mapNormals[nCounter] = n3;
mapNormals[nCounter + 1] = n3;
mapNormals[nCounter + 2] = n3;
mapNormals[nCounter + 3] = n3;
mapNormals[nCounter + 4] = n3;
mapNormals[nCounter + 5] = n3;
nCounter += 6;

mapTexcoords[tcCounter] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y };
mapTexcoords[tcCounter + 1] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y + bottomTexUV.height };
mapTexcoords[tcCounter + 2] = (Vector2){ bottomTexUV.x, bottomTexUV.y + bottomTexUV.height };
mapTexcoords[tcCounter + 3] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y };
mapTexcoords[tcCounter + 4] = (Vector2){ bottomTexUV.x, bottomTexUV.y + bottomTexUV.height };
mapTexcoords[tcCounter + 5] = (Vector2){ bottomTexUV.x, bottomTexUV.y };
tcCounter += 6;
}
}
}


mesh.vertexCount = vCounter;
mesh.triangleCount = vCounter/3;

mesh.vertices = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
mesh.normals = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
mesh.texcoords = (float *)RL_MALLOC(mesh.vertexCount*2*sizeof(float));
mesh.colors = NULL;

int fCounter = 0;


for (int i = 0; i < vCounter; i++)
{
mesh.vertices[fCounter] = mapVertices[i].x;
mesh.vertices[fCounter + 1] = mapVertices[i].y;
mesh.vertices[fCounter + 2] = mapVertices[i].z;
fCounter += 3;
}

fCounter = 0;


for (int i = 0; i < nCounter; i++)
{
mesh.normals[fCounter] = mapNormals[i].x;
mesh.normals[fCounter + 1] = mapNormals[i].y;
mesh.normals[fCounter + 2] = mapNormals[i].z;
fCounter += 3;
}

fCounter = 0;


for (int i = 0; i < tcCounter; i++)
{
mesh.texcoords[fCounter] = mapTexcoords[i].x;
mesh.texcoords[fCounter + 1] = mapTexcoords[i].y;
fCounter += 2;
}

RL_FREE(mapVertices);
RL_FREE(mapNormals);
RL_FREE(mapTexcoords);

RL_FREE(cubicmapPixels); 


rlLoadMesh(&mesh, false);

return mesh;
}
#endif 



BoundingBox MeshBoundingBox(Mesh mesh)
{

Vector3 minVertex = { 0 };
Vector3 maxVertex = { 0 };

if (mesh.vertices != NULL)
{
minVertex = (Vector3){ mesh.vertices[0], mesh.vertices[1], mesh.vertices[2] };
maxVertex = (Vector3){ mesh.vertices[0], mesh.vertices[1], mesh.vertices[2] };

for (int i = 1; i < mesh.vertexCount; i++)
{
minVertex = Vector3Min(minVertex, (Vector3){ mesh.vertices[i*3], mesh.vertices[i*3 + 1], mesh.vertices[i*3 + 2] });
maxVertex = Vector3Max(maxVertex, (Vector3){ mesh.vertices[i*3], mesh.vertices[i*3 + 1], mesh.vertices[i*3 + 2] });
}
}


BoundingBox box = { 0 };
box.min = minVertex;
box.max = maxVertex;

return box;
}




void MeshTangents(Mesh *mesh)
{
if (mesh->tangents == NULL) mesh->tangents = (float *)RL_MALLOC(mesh->vertexCount*4*sizeof(float));
else TRACELOG(LOG_WARNING, "MESH: Tangents data already available, re-writting");

Vector3 *tan1 = (Vector3 *)RL_MALLOC(mesh->vertexCount*sizeof(Vector3));
Vector3 *tan2 = (Vector3 *)RL_MALLOC(mesh->vertexCount*sizeof(Vector3));

for (int i = 0; i < mesh->vertexCount; i += 3)
{

Vector3 v1 = { mesh->vertices[(i + 0)*3 + 0], mesh->vertices[(i + 0)*3 + 1], mesh->vertices[(i + 0)*3 + 2] };
Vector3 v2 = { mesh->vertices[(i + 1)*3 + 0], mesh->vertices[(i + 1)*3 + 1], mesh->vertices[(i + 1)*3 + 2] };
Vector3 v3 = { mesh->vertices[(i + 2)*3 + 0], mesh->vertices[(i + 2)*3 + 1], mesh->vertices[(i + 2)*3 + 2] };


Vector2 uv1 = { mesh->texcoords[(i + 0)*2 + 0], mesh->texcoords[(i + 0)*2 + 1] };
Vector2 uv2 = { mesh->texcoords[(i + 1)*2 + 0], mesh->texcoords[(i + 1)*2 + 1] };
Vector2 uv3 = { mesh->texcoords[(i + 2)*2 + 0], mesh->texcoords[(i + 2)*2 + 1] };

float x1 = v2.x - v1.x;
float y1 = v2.y - v1.y;
float z1 = v2.z - v1.z;
float x2 = v3.x - v1.x;
float y2 = v3.y - v1.y;
float z2 = v3.z - v1.z;

float s1 = uv2.x - uv1.x;
float t1 = uv2.y - uv1.y;
float s2 = uv3.x - uv1.x;
float t2 = uv3.y - uv1.y;

float div = s1*t2 - s2*t1;
float r = (div == 0.0f)? 0.0f : 1.0f/div;

Vector3 sdir = { (t2*x1 - t1*x2)*r, (t2*y1 - t1*y2)*r, (t2*z1 - t1*z2)*r };
Vector3 tdir = { (s1*x2 - s2*x1)*r, (s1*y2 - s2*y1)*r, (s1*z2 - s2*z1)*r };

tan1[i + 0] = sdir;
tan1[i + 1] = sdir;
tan1[i + 2] = sdir;

tan2[i + 0] = tdir;
tan2[i + 1] = tdir;
tan2[i + 2] = tdir;
}


for (int i = 0; i < mesh->vertexCount; ++i)
{
Vector3 normal = { mesh->normals[i*3 + 0], mesh->normals[i*3 + 1], mesh->normals[i*3 + 2] };
Vector3 tangent = tan1[i];


#if defined(COMPUTE_TANGENTS_METHOD_01)
Vector3 tmp = Vector3Subtract(tangent, Vector3Scale(normal, Vector3DotProduct(normal, tangent)));
tmp = Vector3Normalize(tmp);
mesh->tangents[i*4 + 0] = tmp.x;
mesh->tangents[i*4 + 1] = tmp.y;
mesh->tangents[i*4 + 2] = tmp.z;
mesh->tangents[i*4 + 3] = 1.0f;
#else
Vector3OrthoNormalize(&normal, &tangent);
mesh->tangents[i*4 + 0] = tangent.x;
mesh->tangents[i*4 + 1] = tangent.y;
mesh->tangents[i*4 + 2] = tangent.z;
mesh->tangents[i*4 + 3] = (Vector3DotProduct(Vector3CrossProduct(normal, tangent), tan2[i]) < 0.0f)? -1.0f : 1.0f;
#endif
}

RL_FREE(tan1);
RL_FREE(tan2);


mesh->vboId[LOC_VERTEX_TANGENT] = rlLoadAttribBuffer(mesh->vaoId, LOC_VERTEX_TANGENT, mesh->tangents, mesh->vertexCount*4*sizeof(float), false);

TRACELOG(LOG_INFO, "MESH: Tangents data computed for provided mesh");
}


void MeshBinormals(Mesh *mesh)
{
for (int i = 0; i < mesh->vertexCount; i++)
{





}
}


void DrawModel(Model model, Vector3 position, float scale, Color tint)
{
Vector3 vScale = { scale, scale, scale };
Vector3 rotationAxis = { 0.0f, 1.0f, 0.0f };

DrawModelEx(model, position, rotationAxis, 0.0f, vScale, tint);
}


void DrawModelEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint)
{


Matrix matScale = MatrixScale(scale.x, scale.y, scale.z);
Matrix matRotation = MatrixRotate(rotationAxis, rotationAngle*DEG2RAD);
Matrix matTranslation = MatrixTranslate(position.x, position.y, position.z);

Matrix matTransform = MatrixMultiply(MatrixMultiply(matScale, matRotation), matTranslation);


model.transform = MatrixMultiply(model.transform, matTransform);

for (int i = 0; i < model.meshCount; i++)
{

Color color = model.materials[model.meshMaterial[i]].maps[MAP_DIFFUSE].color;

Color colorTint = WHITE;
colorTint.r = (((float)color.r/255.0)*((float)tint.r/255.0))*255;
colorTint.g = (((float)color.g/255.0)*((float)tint.g/255.0))*255;
colorTint.b = (((float)color.b/255.0)*((float)tint.b/255.0))*255;
colorTint.a = (((float)color.a/255.0)*((float)tint.a/255.0))*255;

model.materials[model.meshMaterial[i]].maps[MAP_DIFFUSE].color = colorTint;
rlDrawMesh(model.meshes[i], model.materials[model.meshMaterial[i]], model.transform);
model.materials[model.meshMaterial[i]].maps[MAP_DIFFUSE].color = color;
}
}


void DrawModelWires(Model model, Vector3 position, float scale, Color tint)
{
rlEnableWireMode();

DrawModel(model, position, scale, tint);

rlDisableWireMode();
}


void DrawModelWiresEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint)
{
rlEnableWireMode();

DrawModelEx(model, position, rotationAxis, rotationAngle, scale, tint);

rlDisableWireMode();
}


void DrawBillboard(Camera camera, Texture2D texture, Vector3 center, float size, Color tint)
{
Rectangle sourceRec = { 0.0f, 0.0f, (float)texture.width, (float)texture.height };

DrawBillboardRec(camera, texture, sourceRec, center, size, tint);
}


void DrawBillboardRec(Camera camera, Texture2D texture, Rectangle sourceRec, Vector3 center, float size, Color tint)
{

Vector2 sizeRatio = { size, size*(float)sourceRec.height/sourceRec.width };

Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);

Vector3 right = { matView.m0, matView.m4, matView.m8 };



Vector3 up = { 0.0f, 1.0f, 0.0f };







right = Vector3Scale(right, sizeRatio.x/2);
up = Vector3Scale(up, sizeRatio.y/2);

Vector3 p1 = Vector3Add(right, up);
Vector3 p2 = Vector3Subtract(right, up);

Vector3 a = Vector3Subtract(center, p2);
Vector3 b = Vector3Add(center, p1);
Vector3 c = Vector3Add(center, p2);
Vector3 d = Vector3Subtract(center, p1);

if (rlCheckBufferLimit(4)) rlglDraw();

rlEnableTexture(texture.id);

rlBegin(RL_QUADS);
rlColor4ub(tint.r, tint.g, tint.b, tint.a);


rlTexCoord2f((float)sourceRec.x/texture.width, (float)sourceRec.y/texture.height);
rlVertex3f(a.x, a.y, a.z);


rlTexCoord2f((float)sourceRec.x/texture.width, (float)(sourceRec.y + sourceRec.height)/texture.height);
rlVertex3f(d.x, d.y, d.z);


rlTexCoord2f((float)(sourceRec.x + sourceRec.width)/texture.width, (float)(sourceRec.y + sourceRec.height)/texture.height);
rlVertex3f(c.x, c.y, c.z);


rlTexCoord2f((float)(sourceRec.x + sourceRec.width)/texture.width, (float)sourceRec.y/texture.height);
rlVertex3f(b.x, b.y, b.z);
rlEnd();

rlDisableTexture();
}


void DrawBoundingBox(BoundingBox box, Color color)
{
Vector3 size;

size.x = fabsf(box.max.x - box.min.x);
size.y = fabsf(box.max.y - box.min.y);
size.z = fabsf(box.max.z - box.min.z);

Vector3 center = { box.min.x + size.x/2.0f, box.min.y + size.y/2.0f, box.min.z + size.z/2.0f };

DrawCubeWires(center, size.x, size.y, size.z, color);
}


bool CheckCollisionSpheres(Vector3 centerA, float radiusA, Vector3 centerB, float radiusB)
{
bool collision = false;














if (Vector3DotProduct(Vector3Subtract(centerB, centerA), Vector3Subtract(centerB, centerA)) <= (radiusA + radiusB)*(radiusA + radiusB)) collision = true;

return collision;
}



bool CheckCollisionBoxes(BoundingBox box1, BoundingBox box2)
{
bool collision = true;

if ((box1.max.x >= box2.min.x) && (box1.min.x <= box2.max.x))
{
if ((box1.max.y < box2.min.y) || (box1.min.y > box2.max.y)) collision = false;
if ((box1.max.z < box2.min.z) || (box1.min.z > box2.max.z)) collision = false;
}
else collision = false;

return collision;
}


bool CheckCollisionBoxSphere(BoundingBox box, Vector3 center, float radius)
{
bool collision = false;

float dmin = 0;

if (center.x < box.min.x) dmin += powf(center.x - box.min.x, 2);
else if (center.x > box.max.x) dmin += powf(center.x - box.max.x, 2);

if (center.y < box.min.y) dmin += powf(center.y - box.min.y, 2);
else if (center.y > box.max.y) dmin += powf(center.y - box.max.y, 2);

if (center.z < box.min.z) dmin += powf(center.z - box.min.z, 2);
else if (center.z > box.max.z) dmin += powf(center.z - box.max.z, 2);

if (dmin <= (radius*radius)) collision = true;

return collision;
}


bool CheckCollisionRaySphere(Ray ray, Vector3 center, float radius)
{
bool collision = false;

Vector3 raySpherePos = Vector3Subtract(center, ray.position);
float distance = Vector3Length(raySpherePos);
float vector = Vector3DotProduct(raySpherePos, ray.direction);
float d = radius*radius - (distance*distance - vector*vector);

if (d >= 0.0f) collision = true;

return collision;
}


bool CheckCollisionRaySphereEx(Ray ray, Vector3 center, float radius, Vector3 *collisionPoint)
{
bool collision = false;

Vector3 raySpherePos = Vector3Subtract(center, ray.position);
float distance = Vector3Length(raySpherePos);
float vector = Vector3DotProduct(raySpherePos, ray.direction);
float d = radius*radius - (distance*distance - vector*vector);

if (d >= 0.0f) collision = true;


float collisionDistance = 0;

if (distance < radius) collisionDistance = vector + sqrtf(d);
else collisionDistance = vector - sqrtf(d);


Vector3 cPoint = Vector3Add(ray.position, Vector3Scale(ray.direction, collisionDistance));

collisionPoint->x = cPoint.x;
collisionPoint->y = cPoint.y;
collisionPoint->z = cPoint.z;

return collision;
}


bool CheckCollisionRayBox(Ray ray, BoundingBox box)
{
bool collision = false;

float t[8];
t[0] = (box.min.x - ray.position.x)/ray.direction.x;
t[1] = (box.max.x - ray.position.x)/ray.direction.x;
t[2] = (box.min.y - ray.position.y)/ray.direction.y;
t[3] = (box.max.y - ray.position.y)/ray.direction.y;
t[4] = (box.min.z - ray.position.z)/ray.direction.z;
t[5] = (box.max.z - ray.position.z)/ray.direction.z;
t[6] = (float)fmax(fmax(fmin(t[0], t[1]), fmin(t[2], t[3])), fmin(t[4], t[5]));
t[7] = (float)fmin(fmin(fmax(t[0], t[1]), fmax(t[2], t[3])), fmax(t[4], t[5]));

collision = !(t[7] < 0 || t[6] > t[7]);

return collision;
}


RayHitInfo GetCollisionRayModel(Ray ray, Model model)
{
RayHitInfo result = { 0 };

for (int m = 0; m < model.meshCount; m++)
{

if (model.meshes[m].vertices != NULL)
{

int triangleCount = model.meshes[m].vertexCount/3;


for (int i = 0; i < triangleCount; i++)
{
Vector3 a, b, c;
Vector3 *vertdata = (Vector3 *)model.meshes[m].vertices;

if (model.meshes[m].indices)
{
a = vertdata[model.meshes[m].indices[i*3 + 0]];
b = vertdata[model.meshes[m].indices[i*3 + 1]];
c = vertdata[model.meshes[m].indices[i*3 + 2]];
}
else
{
a = vertdata[i*3 + 0];
b = vertdata[i*3 + 1];
c = vertdata[i*3 + 2];
}

a = Vector3Transform(a, model.transform);
b = Vector3Transform(b, model.transform);
c = Vector3Transform(c, model.transform);

RayHitInfo triHitInfo = GetCollisionRayTriangle(ray, a, b, c);

if (triHitInfo.hit)
{

if ((!result.hit) || (result.distance > triHitInfo.distance)) result = triHitInfo;
}
}
}
}

return result;
}



RayHitInfo GetCollisionRayTriangle(Ray ray, Vector3 p1, Vector3 p2, Vector3 p3)
{
#define EPSILON 0.000001 

Vector3 edge1, edge2;
Vector3 p, q, tv;
float det, invDet, u, v, t;
RayHitInfo result = {0};


edge1 = Vector3Subtract(p2, p1);
edge2 = Vector3Subtract(p3, p1);


p = Vector3CrossProduct(ray.direction, edge2);


det = Vector3DotProduct(edge1, p);


if ((det > -EPSILON) && (det < EPSILON)) return result;

invDet = 1.0f/det;


tv = Vector3Subtract(ray.position, p1);


u = Vector3DotProduct(tv, p)*invDet;


if ((u < 0.0f) || (u > 1.0f)) return result;


q = Vector3CrossProduct(tv, edge1);


v = Vector3DotProduct(ray.direction, q)*invDet;


if ((v < 0.0f) || ((u + v) > 1.0f)) return result;

t = Vector3DotProduct(edge2, q)*invDet;

if (t > EPSILON)
{

result.hit = true;
result.distance = t;
result.hit = true;
result.normal = Vector3Normalize(Vector3CrossProduct(edge1, edge2));
result.position = Vector3Add(ray.position, Vector3Scale(ray.direction, t));
}

return result;
}


RayHitInfo GetCollisionRayGround(Ray ray, float groundHeight)
{
#define EPSILON 0.000001 

RayHitInfo result = { 0 };

if (fabsf(ray.direction.y) > EPSILON)
{
float distance = (ray.position.y - groundHeight)/-ray.direction.y;

if (distance >= 0.0)
{
result.hit = true;
result.distance = distance;
result.normal = (Vector3){ 0.0, 1.0, 0.0 };
result.position = Vector3Add(ray.position, Vector3Scale(ray.direction, distance));
}
}

return result;
}





#if defined(SUPPORT_FILEFORMAT_OBJ)

static Model LoadOBJ(const char *fileName)
{
Model model = { 0 };

tinyobj_attrib_t attrib;
tinyobj_shape_t *meshes = NULL;
unsigned int meshCount = 0;

tinyobj_material_t *materials = NULL;
unsigned int materialCount = 0;

char *fileData = LoadFileText(fileName);

if (fileData != NULL)
{
int dataSize = strlen(fileData);
char currentDir[1024] = { 0 };
strcpy(currentDir, GetWorkingDirectory());
chdir(GetDirectoryPath(fileName));

unsigned int flags = TINYOBJ_FLAG_TRIANGULATE;
int ret = tinyobj_parse_obj(&attrib, &meshes, &meshCount, &materials, &materialCount, fileData, dataSize, flags);

if (ret != TINYOBJ_SUCCESS) TRACELOG(LOG_WARNING, "MODEL: [%s] Failed to load OBJ data", fileName);
else TRACELOG(LOG_INFO, "MODEL: [%s] OBJ data loaded successfully: %i meshes / %i materials", fileName, meshCount, materialCount);




model.meshCount = 1;
model.meshes = (Mesh *)RL_CALLOC(model.meshCount, sizeof(Mesh));


if (materialCount > 0)
{
model.materialCount = materialCount;
model.materials = (Material *)RL_CALLOC(model.materialCount, sizeof(Material));
}

model.meshMaterial = (int *)RL_CALLOC(model.meshCount, sizeof(int));












for (int m = 0; m < 1; m++)
{
Mesh mesh = { 0 };
memset(&mesh, 0, sizeof(Mesh));
mesh.vertexCount = attrib.num_faces*3;
mesh.triangleCount = attrib.num_faces;
mesh.vertices = (float *)RL_CALLOC(mesh.vertexCount*3, sizeof(float));
mesh.texcoords = (float *)RL_CALLOC(mesh.vertexCount*2, sizeof(float));
mesh.normals = (float *)RL_CALLOC(mesh.vertexCount*3, sizeof(float));
mesh.vboId = (unsigned int *)RL_CALLOC(MAX_MESH_VBO, sizeof(unsigned int));

int vCount = 0;
int vtCount = 0;
int vnCount = 0;

for (int f = 0; f < attrib.num_faces; f++)
{

tinyobj_vertex_index_t idx0 = attrib.faces[3*f + 0];
tinyobj_vertex_index_t idx1 = attrib.faces[3*f + 1];
tinyobj_vertex_index_t idx2 = attrib.faces[3*f + 2];


for (int v = 0; v < 3; v++) { mesh.vertices[vCount + v] = attrib.vertices[idx0.v_idx*3 + v]; } vCount +=3;
for (int v = 0; v < 3; v++) { mesh.vertices[vCount + v] = attrib.vertices[idx1.v_idx*3 + v]; } vCount +=3;
for (int v = 0; v < 3; v++) { mesh.vertices[vCount + v] = attrib.vertices[idx2.v_idx*3 + v]; } vCount +=3;



mesh.texcoords[vtCount + 0] = attrib.texcoords[idx0.vt_idx*2 + 0];
mesh.texcoords[vtCount + 1] = 1.0f - attrib.texcoords[idx0.vt_idx*2 + 1]; vtCount += 2;
mesh.texcoords[vtCount + 0] = attrib.texcoords[idx1.vt_idx*2 + 0];
mesh.texcoords[vtCount + 1] = 1.0f - attrib.texcoords[idx1.vt_idx*2 + 1]; vtCount += 2;
mesh.texcoords[vtCount + 0] = attrib.texcoords[idx2.vt_idx*2 + 0];
mesh.texcoords[vtCount + 1] = 1.0f - attrib.texcoords[idx2.vt_idx*2 + 1]; vtCount += 2;


for (int v = 0; v < 3; v++) { mesh.normals[vnCount + v] = attrib.normals[idx0.vn_idx*3 + v]; } vnCount +=3;
for (int v = 0; v < 3; v++) { mesh.normals[vnCount + v] = attrib.normals[idx1.vn_idx*3 + v]; } vnCount +=3;
for (int v = 0; v < 3; v++) { mesh.normals[vnCount + v] = attrib.normals[idx2.vn_idx*3 + v]; } vnCount +=3;
}

model.meshes[m] = mesh; 


model.meshMaterial[m] = attrib.material_ids[m];


if (model.meshMaterial[m] == -1) model.meshMaterial[m] = 0;
}


for (int m = 0; m < materialCount; m++)
{


model.materials[m] = LoadMaterialDefault();




























model.materials[m].maps[MAP_DIFFUSE].texture = GetTextureDefault(); 

if (materials[m].diffuse_texname != NULL) model.materials[m].maps[MAP_DIFFUSE].texture = LoadTexture(materials[m].diffuse_texname); 
model.materials[m].maps[MAP_DIFFUSE].color = (Color){ (float)(materials[m].diffuse[0]*255.0f), (float)(materials[m].diffuse[1]*255.0f), (float)(materials[m].diffuse[2]*255.0f), 255 }; 
model.materials[m].maps[MAP_DIFFUSE].value = 0.0f;

if (materials[m].specular_texname != NULL) model.materials[m].maps[MAP_SPECULAR].texture = LoadTexture(materials[m].specular_texname); 
model.materials[m].maps[MAP_SPECULAR].color = (Color){ (float)(materials[m].specular[0]*255.0f), (float)(materials[m].specular[1]*255.0f), (float)(materials[m].specular[2]*255.0f), 255 }; 
model.materials[m].maps[MAP_SPECULAR].value = 0.0f;

if (materials[m].bump_texname != NULL) model.materials[m].maps[MAP_NORMAL].texture = LoadTexture(materials[m].bump_texname); 
model.materials[m].maps[MAP_NORMAL].color = WHITE;
model.materials[m].maps[MAP_NORMAL].value = materials[m].shininess;

model.materials[m].maps[MAP_EMISSION].color = (Color){ (float)(materials[m].emission[0]*255.0f), (float)(materials[m].emission[1]*255.0f), (float)(materials[m].emission[2]*255.0f), 255 }; 

if (materials[m].displacement_texname != NULL) model.materials[m].maps[MAP_HEIGHT].texture = LoadTexture(materials[m].displacement_texname); 
}

tinyobj_attrib_free(&attrib);
tinyobj_shapes_free(meshes, meshCount);
tinyobj_materials_free(materials, materialCount);

RL_FREE(fileData);

chdir(currentDir);
}

return model;
}
#endif

#if defined(SUPPORT_FILEFORMAT_IQM)

static Model LoadIQM(const char *fileName)
{
#define IQM_MAGIC "INTERQUAKEMODEL" 
#define IQM_VERSION 2 

#define BONE_NAME_LENGTH 32 
#define MESH_NAME_LENGTH 32 



typedef struct IQMHeader {
char magic[16];
unsigned int version;
unsigned int filesize;
unsigned int flags;
unsigned int num_text, ofs_text;
unsigned int num_meshes, ofs_meshes;
unsigned int num_vertexarrays, num_vertexes, ofs_vertexarrays;
unsigned int num_triangles, ofs_triangles, ofs_adjacency;
unsigned int num_joints, ofs_joints;
unsigned int num_poses, ofs_poses;
unsigned int num_anims, ofs_anims;
unsigned int num_frames, num_framechannels, ofs_frames, ofs_bounds;
unsigned int num_comment, ofs_comment;
unsigned int num_extensions, ofs_extensions;
} IQMHeader;

typedef struct IQMMesh {
unsigned int name;
unsigned int material;
unsigned int first_vertex, num_vertexes;
unsigned int first_triangle, num_triangles;
} IQMMesh;

typedef struct IQMTriangle {
unsigned int vertex[3];
} IQMTriangle;

typedef struct IQMJoint {
unsigned int name;
int parent;
float translate[3], rotate[4], scale[3];
} IQMJoint;

typedef struct IQMVertexArray {
unsigned int type;
unsigned int flags;
unsigned int format;
unsigned int size;
unsigned int offset;
} IQMVertexArray;





























enum {
IQM_POSITION = 0,
IQM_TEXCOORD = 1,
IQM_NORMAL = 2,
IQM_TANGENT = 3, 
IQM_BLENDINDEXES = 4,
IQM_BLENDWEIGHTS = 5,
IQM_COLOR = 6, 
IQM_CUSTOM = 0x10 
};

Model model = { 0 };

FILE *iqmFile = NULL;
IQMHeader iqm;

IQMMesh *imesh;
IQMTriangle *tri;
IQMVertexArray *va;
IQMJoint *ijoint;

float *vertex = NULL;
float *normal = NULL;
float *text = NULL;
char *blendi = NULL;
unsigned char *blendw = NULL;

iqmFile = fopen(fileName, "rb");

if (iqmFile == NULL)
{
TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to open IQM file", fileName);
return model;
}

fread(&iqm, sizeof(IQMHeader), 1, iqmFile); 

if (strncmp(iqm.magic, IQM_MAGIC, sizeof(IQM_MAGIC)))
{
TRACELOG(LOG_WARNING, "MODEL: [%s] IQM file is not a valid model", fileName);
fclose(iqmFile);
return model;
}

if (iqm.version != IQM_VERSION)
{
TRACELOG(LOG_WARNING, "MODEL: [%s] IQM file version not supported (%i)", fileName, iqm.version);
fclose(iqmFile);
return model;
}


imesh = RL_MALLOC(sizeof(IQMMesh)*iqm.num_meshes);
fseek(iqmFile, iqm.ofs_meshes, SEEK_SET);
fread(imesh, sizeof(IQMMesh)*iqm.num_meshes, 1, iqmFile);

model.meshCount = iqm.num_meshes;
model.meshes = RL_CALLOC(model.meshCount, sizeof(Mesh));

char name[MESH_NAME_LENGTH] = { 0 };

for (int i = 0; i < model.meshCount; i++)
{
fseek(iqmFile, iqm.ofs_text + imesh[i].name, SEEK_SET);
fread(name, sizeof(char)*MESH_NAME_LENGTH, 1, iqmFile); 
model.meshes[i].vertexCount = imesh[i].num_vertexes;

model.meshes[i].vertices = RL_CALLOC(model.meshes[i].vertexCount*3, sizeof(float)); 
model.meshes[i].normals = RL_CALLOC(model.meshes[i].vertexCount*3, sizeof(float)); 
model.meshes[i].texcoords = RL_CALLOC(model.meshes[i].vertexCount*2, sizeof(float)); 

model.meshes[i].boneIds = RL_CALLOC(model.meshes[i].vertexCount*4, sizeof(float)); 
model.meshes[i].boneWeights = RL_CALLOC(model.meshes[i].vertexCount*4, sizeof(float)); 

model.meshes[i].triangleCount = imesh[i].num_triangles;
model.meshes[i].indices = RL_CALLOC(model.meshes[i].triangleCount*3, sizeof(unsigned short));



model.meshes[i].animVertices = RL_CALLOC(model.meshes[i].vertexCount*3, sizeof(float));
model.meshes[i].animNormals = RL_CALLOC(model.meshes[i].vertexCount*3, sizeof(float));

model.meshes[i].vboId = (unsigned int *)RL_CALLOC(MAX_MESH_VBO, sizeof(unsigned int));
}


tri = RL_MALLOC(iqm.num_triangles*sizeof(IQMTriangle));
fseek(iqmFile, iqm.ofs_triangles, SEEK_SET);
fread(tri, iqm.num_triangles*sizeof(IQMTriangle), 1, iqmFile);

for (int m = 0; m < model.meshCount; m++)
{
int tcounter = 0;

for (int i = imesh[m].first_triangle; i < (imesh[m].first_triangle + imesh[m].num_triangles); i++)
{

model.meshes[m].indices[tcounter + 2] = tri[i].vertex[0] - imesh[m].first_vertex;
model.meshes[m].indices[tcounter + 1] = tri[i].vertex[1] - imesh[m].first_vertex;
model.meshes[m].indices[tcounter] = tri[i].vertex[2] - imesh[m].first_vertex;
tcounter += 3;
}
}


va = RL_MALLOC(iqm.num_vertexarrays*sizeof(IQMVertexArray));
fseek(iqmFile, iqm.ofs_vertexarrays, SEEK_SET);
fread(va, iqm.num_vertexarrays*sizeof(IQMVertexArray), 1, iqmFile);

for (int i = 0; i < iqm.num_vertexarrays; i++)
{
switch (va[i].type)
{
case IQM_POSITION:
{
vertex = RL_MALLOC(iqm.num_vertexes*3*sizeof(float));
fseek(iqmFile, va[i].offset, SEEK_SET);
fread(vertex, iqm.num_vertexes*3*sizeof(float), 1, iqmFile);

for (int m = 0; m < iqm.num_meshes; m++)
{
int vCounter = 0;
for (int i = imesh[m].first_vertex*3; i < (imesh[m].first_vertex + imesh[m].num_vertexes)*3; i++)
{
model.meshes[m].vertices[vCounter] = vertex[i];
model.meshes[m].animVertices[vCounter] = vertex[i];
vCounter++;
}
}
} break;
case IQM_NORMAL:
{
normal = RL_MALLOC(iqm.num_vertexes*3*sizeof(float));
fseek(iqmFile, va[i].offset, SEEK_SET);
fread(normal, iqm.num_vertexes*3*sizeof(float), 1, iqmFile);

for (int m = 0; m < iqm.num_meshes; m++)
{
int vCounter = 0;
for (int i = imesh[m].first_vertex*3; i < (imesh[m].first_vertex + imesh[m].num_vertexes)*3; i++)
{
model.meshes[m].normals[vCounter] = normal[i];
model.meshes[m].animNormals[vCounter] = normal[i];
vCounter++;
}
}
} break;
case IQM_TEXCOORD:
{
text = RL_MALLOC(iqm.num_vertexes*2*sizeof(float));
fseek(iqmFile, va[i].offset, SEEK_SET);
fread(text, iqm.num_vertexes*2*sizeof(float), 1, iqmFile);

for (int m = 0; m < iqm.num_meshes; m++)
{
int vCounter = 0;
for (int i = imesh[m].first_vertex*2; i < (imesh[m].first_vertex + imesh[m].num_vertexes)*2; i++)
{
model.meshes[m].texcoords[vCounter] = text[i];
vCounter++;
}
}
} break;
case IQM_BLENDINDEXES:
{
blendi = RL_MALLOC(iqm.num_vertexes*4*sizeof(char));
fseek(iqmFile, va[i].offset, SEEK_SET);
fread(blendi, iqm.num_vertexes*4*sizeof(char), 1, iqmFile);

for (int m = 0; m < iqm.num_meshes; m++)
{
int boneCounter = 0;
for (int i = imesh[m].first_vertex*4; i < (imesh[m].first_vertex + imesh[m].num_vertexes)*4; i++)
{
model.meshes[m].boneIds[boneCounter] = blendi[i];
boneCounter++;
}
}
} break;
case IQM_BLENDWEIGHTS:
{
blendw = RL_MALLOC(iqm.num_vertexes*4*sizeof(unsigned char));
fseek(iqmFile, va[i].offset, SEEK_SET);
fread(blendw, iqm.num_vertexes*4*sizeof(unsigned char), 1, iqmFile);

for (int m = 0; m < iqm.num_meshes; m++)
{
int boneCounter = 0;
for (int i = imesh[m].first_vertex*4; i < (imesh[m].first_vertex + imesh[m].num_vertexes)*4; i++)
{
model.meshes[m].boneWeights[boneCounter] = blendw[i]/255.0f;
boneCounter++;
}
}
} break;
}
}


ijoint = RL_MALLOC(iqm.num_joints*sizeof(IQMJoint));
fseek(iqmFile, iqm.ofs_joints, SEEK_SET);
fread(ijoint, iqm.num_joints*sizeof(IQMJoint), 1, iqmFile);

model.boneCount = iqm.num_joints;
model.bones = RL_MALLOC(iqm.num_joints*sizeof(BoneInfo));
model.bindPose = RL_MALLOC(iqm.num_joints*sizeof(Transform));

for (int i = 0; i < iqm.num_joints; i++)
{

model.bones[i].parent = ijoint[i].parent;
fseek(iqmFile, iqm.ofs_text + ijoint[i].name, SEEK_SET);
fread(model.bones[i].name, BONE_NAME_LENGTH*sizeof(char), 1, iqmFile);


model.bindPose[i].translation.x = ijoint[i].translate[0];
model.bindPose[i].translation.y = ijoint[i].translate[1];
model.bindPose[i].translation.z = ijoint[i].translate[2];

model.bindPose[i].rotation.x = ijoint[i].rotate[0];
model.bindPose[i].rotation.y = ijoint[i].rotate[1];
model.bindPose[i].rotation.z = ijoint[i].rotate[2];
model.bindPose[i].rotation.w = ijoint[i].rotate[3];

model.bindPose[i].scale.x = ijoint[i].scale[0];
model.bindPose[i].scale.y = ijoint[i].scale[1];
model.bindPose[i].scale.z = ijoint[i].scale[2];
}


for (int i = 0; i < model.boneCount; i++)
{
if (model.bones[i].parent >= 0)
{
model.bindPose[i].rotation = QuaternionMultiply(model.bindPose[model.bones[i].parent].rotation, model.bindPose[i].rotation);
model.bindPose[i].translation = Vector3RotateByQuaternion(model.bindPose[i].translation, model.bindPose[model.bones[i].parent].rotation);
model.bindPose[i].translation = Vector3Add(model.bindPose[i].translation, model.bindPose[model.bones[i].parent].translation);
model.bindPose[i].scale = Vector3Multiply(model.bindPose[i].scale, model.bindPose[model.bones[i].parent].scale);
}
}

fclose(iqmFile);
RL_FREE(imesh);
RL_FREE(tri);
RL_FREE(va);
RL_FREE(vertex);
RL_FREE(normal);
RL_FREE(text);
RL_FREE(blendi);
RL_FREE(blendw);
RL_FREE(ijoint);

return model;
}
#endif

#if defined(SUPPORT_FILEFORMAT_GLTF)

static const unsigned char base64Table[] = {
0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 62, 0, 0, 0, 63, 52, 53,
54, 55, 56, 57, 58, 59, 60, 61, 0, 0,
0, 0, 0, 0, 0, 0, 1, 2, 3, 4,
5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
25, 0, 0, 0, 0, 0, 0, 26, 27, 28,
29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
49, 50, 51
};

static int GetSizeBase64(char *input)
{
int size = 0;

for (int i = 0; input[4*i] != 0; i++)
{
if (input[4*i + 3] == '=')
{
if (input[4*i + 2] == '=') size += 1;
else size += 2;
}
else size += 3;
}

return size;
}

static unsigned char *DecodeBase64(char *input, int *size)
{
*size = GetSizeBase64(input);

unsigned char *buf = (unsigned char *)RL_MALLOC(*size);
for (int i = 0; i < *size/3; i++)
{
unsigned char a = base64Table[(int)input[4*i]];
unsigned char b = base64Table[(int)input[4*i + 1]];
unsigned char c = base64Table[(int)input[4*i + 2]];
unsigned char d = base64Table[(int)input[4*i + 3]];

buf[3*i] = (a << 2) | (b >> 4);
buf[3*i + 1] = (b << 4) | (c >> 2);
buf[3*i + 2] = (c << 6) | d;
}

if (*size%3 == 1)
{
int n = *size/3;
unsigned char a = base64Table[(int)input[4*n]];
unsigned char b = base64Table[(int)input[4*n + 1]];
buf[*size - 1] = (a << 2) | (b >> 4);
}
else if (*size%3 == 2)
{
int n = *size/3;
unsigned char a = base64Table[(int)input[4*n]];
unsigned char b = base64Table[(int)input[4*n + 1]];
unsigned char c = base64Table[(int)input[4*n + 2]];
buf[*size - 2] = (a << 2) | (b >> 4);
buf[*size - 1] = (b << 4) | (c >> 2);
}
return buf;
}


static Image LoadImageFromCgltfImage(cgltf_image *image, const char *texPath, Color tint)
{
Image rimage = { 0 };

if (image->uri)
{
if ((strlen(image->uri) > 5) &&
(image->uri[0] == 'd') &&
(image->uri[1] == 'a') &&
(image->uri[2] == 't') &&
(image->uri[3] == 'a') &&
(image->uri[4] == ':'))
{




int i = 0;
while ((image->uri[i] != ',') && (image->uri[i] != 0)) i++;

if (image->uri[i] == 0) TRACELOG(LOG_WARNING, "IMAGE: glTF data URI is not a valid image");
else
{
int size;
unsigned char *data = DecodeBase64(image->uri + i + 1, &size);

int w, h;
unsigned char *raw = stbi_load_from_memory(data, size, &w, &h, NULL, 4);

rimage = LoadImagePro(raw, w, h, UNCOMPRESSED_R8G8B8A8);


ImageColorTint(&rimage, tint);
}
}
else
{
rimage = LoadImage(TextFormat("%s/%s", texPath, image->uri));


ImageColorTint(&rimage, tint);
}
}
else if (image->buffer_view)
{
unsigned char *data = RL_MALLOC(image->buffer_view->size);
int n = image->buffer_view->offset;
int stride = image->buffer_view->stride ? image->buffer_view->stride : 1;

for (int i = 0; i < image->buffer_view->size; i++)
{
data[i] = ((unsigned char *)image->buffer_view->buffer->data)[n];
n += stride;
}

int w, h;
unsigned char *raw = stbi_load_from_memory(data, image->buffer_view->size, &w, &h, NULL, 4);
free(data);

rimage = LoadImagePro(raw, w, h, UNCOMPRESSED_R8G8B8A8);
free(raw);


ImageColorTint(&rimage, tint);
}
else
{
rimage = LoadImageEx(&tint, 1, 1);
}

return rimage;
}


static Model LoadGLTF(const char *fileName)
{


















#define LOAD_ACCESSOR(type, nbcomp, acc, dst) { int n = 0; type* buf = (type*)acc->buffer_view->buffer->data+acc->buffer_view->offset/sizeof(type)+acc->offset/sizeof(type); for (int k = 0; k < acc->count; k++) {for (int l = 0; l < nbcomp; l++) {dst[nbcomp*k+l] = buf[n+l];}n += acc->stride/sizeof(type);}}











Model model = { 0 };


FILE *gltfFile = fopen(fileName, "rb");

if (gltfFile == NULL)
{
TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to open glTF file", fileName);
return model;
}

fseek(gltfFile, 0, SEEK_END);
int size = ftell(gltfFile);
fseek(gltfFile, 0, SEEK_SET);

void *buffer = RL_MALLOC(size);
fread(buffer, size, 1, gltfFile);

fclose(gltfFile);


cgltf_options options = { 0 };
cgltf_data *data = NULL;
cgltf_result result = cgltf_parse(&options, buffer, size, &data);

if (result == cgltf_result_success)
{
TRACELOG(LOG_INFO, "MODEL: [%s] glTF meshes (%s) count: %i", fileName, (data->file_type == 2)? "glb" : "gltf", data->meshes_count, data->materials_count);
TRACELOG(LOG_INFO, "MODEL: [%s] glTF materials (%s) count: %i", fileName, (data->file_type == 2)? "glb" : "gltf", data->meshes_count, data->materials_count);


result = cgltf_load_buffers(&options, data, fileName);
if (result != cgltf_result_success) TRACELOG(LOG_INFO, "MODEL: [%s] Failed to load mesh/material buffers", fileName);

int primitivesCount = 0;

for (int i = 0; i < data->meshes_count; i++) primitivesCount += (int)data->meshes[i].primitives_count;


model.meshCount = primitivesCount;
model.meshes = RL_CALLOC(model.meshCount, sizeof(Mesh));
model.materialCount = data->materials_count + 1;
model.materials = RL_MALLOC(model.materialCount*sizeof(Material));
model.meshMaterial = RL_MALLOC(model.meshCount*sizeof(int));

for (int i = 0; i < model.meshCount; i++) model.meshes[i].vboId = (unsigned int *)RL_CALLOC(MAX_MESH_VBO, sizeof(unsigned int));

for (int i = 0; i < model.materialCount - 1; i++)
{
model.materials[i] = LoadMaterialDefault();
Color tint = (Color){ 255, 255, 255, 255 };
const char *texPath = GetDirectoryPath(fileName);


if (data->materials[i].has_pbr_metallic_roughness)
{
tint.r = (unsigned char)(data->materials[i].pbr_metallic_roughness.base_color_factor[0] * 255);
tint.g = (unsigned char)(data->materials[i].pbr_metallic_roughness.base_color_factor[1] * 255);
tint.b = (unsigned char)(data->materials[i].pbr_metallic_roughness.base_color_factor[2] * 255);
tint.a = (unsigned char)(data->materials[i].pbr_metallic_roughness.base_color_factor[3] * 255);

model.materials[i].maps[MAP_ALBEDO].color = tint;

if (data->materials[i].pbr_metallic_roughness.base_color_texture.texture)
{
Image albedo = LoadImageFromCgltfImage(data->materials[i].pbr_metallic_roughness.base_color_texture.texture->image, texPath, tint);
model.materials[i].maps[MAP_ALBEDO].texture = LoadTextureFromImage(albedo);
UnloadImage(albedo);
}

tint = WHITE; 

if (data->materials[i].pbr_metallic_roughness.metallic_roughness_texture.texture)
{
Image metallicRoughness = LoadImageFromCgltfImage(data->materials[i].pbr_metallic_roughness.metallic_roughness_texture.texture->image, texPath, tint);
model.materials[i].maps[MAP_ROUGHNESS].texture = LoadTextureFromImage(metallicRoughness);

float roughness = data->materials[i].pbr_metallic_roughness.roughness_factor;
model.materials[i].maps[MAP_ROUGHNESS].value = roughness;

float metallic = data->materials[i].pbr_metallic_roughness.metallic_factor;
model.materials[i].maps[MAP_METALNESS].value = metallic;

UnloadImage(metallicRoughness);
}

if (data->materials[i].normal_texture.texture)
{
Image normalImage = LoadImageFromCgltfImage(data->materials[i].normal_texture.texture->image, texPath, tint);
model.materials[i].maps[MAP_NORMAL].texture = LoadTextureFromImage(normalImage);
UnloadImage(normalImage);
}

if (data->materials[i].occlusion_texture.texture)
{
Image occulsionImage = LoadImageFromCgltfImage(data->materials[i].occlusion_texture.texture->image, texPath, tint);
model.materials[i].maps[MAP_OCCLUSION].texture = LoadTextureFromImage(occulsionImage);
UnloadImage(occulsionImage);
}

if (data->materials[i].emissive_texture.texture)
{
Image emissiveImage = LoadImageFromCgltfImage(data->materials[i].emissive_texture.texture->image, texPath, tint);
model.materials[i].maps[MAP_EMISSION].texture = LoadTextureFromImage(emissiveImage);
tint.r = (unsigned char)(data->materials[i].emissive_factor[0]*255);
tint.g = (unsigned char)(data->materials[i].emissive_factor[1]*255);
tint.b = (unsigned char)(data->materials[i].emissive_factor[2]*255);
model.materials[i].maps[MAP_EMISSION].color = tint;
UnloadImage(emissiveImage);
}
}
}

model.materials[model.materialCount - 1] = LoadMaterialDefault();

int primitiveIndex = 0;

for (int i = 0; i < data->meshes_count; i++)
{
for (int p = 0; p < data->meshes[i].primitives_count; p++)
{
for (int j = 0; j < data->meshes[i].primitives[p].attributes_count; j++)
{
if (data->meshes[i].primitives[p].attributes[j].type == cgltf_attribute_type_position)
{
cgltf_accessor *acc = data->meshes[i].primitives[p].attributes[j].data;
model.meshes[primitiveIndex].vertexCount = acc->count;
model.meshes[primitiveIndex].vertices = RL_MALLOC(sizeof(float)*model.meshes[primitiveIndex].vertexCount*3);

LOAD_ACCESSOR(float, 3, acc, model.meshes[primitiveIndex].vertices)
}
else if (data->meshes[i].primitives[p].attributes[j].type == cgltf_attribute_type_normal)
{
cgltf_accessor *acc = data->meshes[i].primitives[p].attributes[j].data;
model.meshes[primitiveIndex].normals = RL_MALLOC(sizeof(float)*acc->count*3);

LOAD_ACCESSOR(float, 3, acc, model.meshes[primitiveIndex].normals)
}
else if (data->meshes[i].primitives[p].attributes[j].type == cgltf_attribute_type_texcoord)
{
cgltf_accessor *acc = data->meshes[i].primitives[p].attributes[j].data;

if (acc->component_type == cgltf_component_type_r_32f)
{
model.meshes[primitiveIndex].texcoords = RL_MALLOC(sizeof(float)*acc->count*2);
LOAD_ACCESSOR(float, 2, acc, model.meshes[primitiveIndex].texcoords)
}
else
{

TRACELOG(LOG_WARNING, "MODEL: [%s] glTF texture coordinates must be float", fileName);
}
}
}

cgltf_accessor *acc = data->meshes[i].primitives[p].indices;

if (acc)
{
if (acc->component_type == cgltf_component_type_r_16u)
{
model.meshes[primitiveIndex].triangleCount = acc->count/3;
model.meshes[primitiveIndex].indices = RL_MALLOC(sizeof(unsigned short)*model.meshes[primitiveIndex].triangleCount*3);
LOAD_ACCESSOR(unsigned short, 1, acc, model.meshes[primitiveIndex].indices)
}
else
{

TRACELOG(LOG_WARNING, "MODEL: [%s] glTF index data must be unsigned short", fileName);
}
}
else
{

model.meshes[primitiveIndex].triangleCount = model.meshes[primitiveIndex].vertexCount/3;
}

if (data->meshes[i].primitives[p].material)
{

model.meshMaterial[primitiveIndex] = data->meshes[i].primitives[p].material - data->materials;
}
else
{
model.meshMaterial[primitiveIndex] = model.materialCount - 1;;
}

primitiveIndex++;
}
}

cgltf_free(data);
}
else TRACELOG(LOG_WARNING, "MODEL: [%s] Failed to load glTF data", fileName);

RL_FREE(buffer);

return model;
}
#endif
