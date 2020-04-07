



























































#if !defined(RLGL_H)
#define RLGL_H

#if defined(RLGL_STANDALONE)
#define RAYMATH_STANDALONE
#define RAYMATH_HEADER_ONLY

#define RLAPI 

#if defined(_WIN32)
#if defined(BUILD_LIBTYPE_SHARED)
#define RLAPI __declspec(dllexport) 
#elif defined(USE_LIBTYPE_SHARED)
#define RLAPI __declspec(dllimport) 
#endif
#endif


#if !defined(TRACELOG)
#define TRACELOG(level, ...) (void)0
#define TRACELOGD(...) (void)0
#endif


#if !defined(RL_MALLOC)
#define RL_MALLOC(sz) malloc(sz)
#endif
#if !defined(RL_CALLOC)
#define RL_CALLOC(n,sz) calloc(n,sz)
#endif
#if !defined(RL_REALLOC)
#define RL_REALLOC(n,sz) realloc(n,sz)
#endif
#if !defined(RL_FREE)
#define RL_FREE(p) free(p)
#endif
#else
#include "raylib.h" 
#endif

#include "raymath.h" 


#if !defined(GRAPHICS_API_OPENGL_11) && !defined(GRAPHICS_API_OPENGL_21) && !defined(GRAPHICS_API_OPENGL_33) && !defined(GRAPHICS_API_OPENGL_ES2)



#define GRAPHICS_API_OPENGL_33
#endif


#if defined(GRAPHICS_API_OPENGL_11)
#if defined(GRAPHICS_API_OPENGL_21)
#undef GRAPHICS_API_OPENGL_21
#endif
#if defined(GRAPHICS_API_OPENGL_33)
#undef GRAPHICS_API_OPENGL_33
#endif
#if defined(GRAPHICS_API_OPENGL_ES2)
#undef GRAPHICS_API_OPENGL_ES2
#endif
#endif

#if defined(GRAPHICS_API_OPENGL_21)
#define GRAPHICS_API_OPENGL_33
#endif




#if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)


#define MAX_BATCH_ELEMENTS 8192
#elif defined(GRAPHICS_API_OPENGL_ES2)


#define MAX_BATCH_ELEMENTS 2048
#endif

#if !defined(MAX_BATCH_BUFFERING)
#define MAX_BATCH_BUFFERING 1 
#endif
#define MAX_MATRIX_STACK_SIZE 32 
#define MAX_DRAWCALL_REGISTERED 256 

#if !defined(DEFAULT_NEAR_CULL_DISTANCE)
#define DEFAULT_NEAR_CULL_DISTANCE 0.01 
#endif
#if !defined(DEFAULT_FAR_CULL_DISTANCE)
#define DEFAULT_FAR_CULL_DISTANCE 1000.0 
#endif


#define MAX_SHADER_LOCATIONS 32 
#define MAX_MATERIAL_MAPS 12 


#define RL_TEXTURE_WRAP_S 0x2802 
#define RL_TEXTURE_WRAP_T 0x2803 
#define RL_TEXTURE_MAG_FILTER 0x2800 
#define RL_TEXTURE_MIN_FILTER 0x2801 
#define RL_TEXTURE_ANISOTROPIC_FILTER 0x3000 

#define RL_FILTER_NEAREST 0x2600 
#define RL_FILTER_LINEAR 0x2601 
#define RL_FILTER_MIP_NEAREST 0x2700 
#define RL_FILTER_NEAREST_MIP_LINEAR 0x2702 
#define RL_FILTER_LINEAR_MIP_NEAREST 0x2701 
#define RL_FILTER_MIP_LINEAR 0x2703 

#define RL_WRAP_REPEAT 0x2901 
#define RL_WRAP_CLAMP 0x812F 
#define RL_WRAP_MIRROR_REPEAT 0x8370 
#define RL_WRAP_MIRROR_CLAMP 0x8742 


#define RL_MODELVIEW 0x1700 
#define RL_PROJECTION 0x1701 
#define RL_TEXTURE 0x1702 


#define RL_LINES 0x0001 
#define RL_TRIANGLES 0x0004 
#define RL_QUADS 0x0007 




typedef enum { OPENGL_11 = 1, OPENGL_21, OPENGL_33, OPENGL_ES_20 } GlVersion;

typedef unsigned char byte;

#if defined(RLGL_STANDALONE)
#if !defined(__cplusplus)

typedef enum { false, true } bool;
#endif


typedef struct Color {
unsigned char r;
unsigned char g;
unsigned char b;
unsigned char a;
} Color;


typedef struct Rectangle {
float x;
float y;
float width;
float height;
} Rectangle;



typedef struct Texture2D {
unsigned int id; 
int width; 
int height; 
int mipmaps; 
int format; 
} Texture2D;


typedef Texture2D Texture;


typedef Texture2D TextureCubemap;


typedef struct RenderTexture2D {
unsigned int id; 
Texture2D texture; 
Texture2D depth; 
bool depthTexture; 
} RenderTexture2D;


typedef RenderTexture2D RenderTexture;


typedef struct Mesh {
int vertexCount; 
int triangleCount; 
float *vertices; 
float *texcoords; 
float *texcoords2; 
float *normals; 
float *tangents; 
unsigned char *colors; 
unsigned short *indices;


float *animVertices; 
float *animNormals; 
int *boneIds; 
float *boneWeights; 


unsigned int vaoId; 
unsigned int *vboId; 
} Mesh;


#define MAX_SHADER_LOCATIONS 32
#define MAX_MATERIAL_MAPS 12


typedef struct Shader {
unsigned int id; 
int *locs; 
} Shader;


typedef struct MaterialMap {
Texture2D texture; 
Color color; 
float value; 
} MaterialMap;


typedef struct Material {
Shader shader; 
MaterialMap *maps; 
float *params; 
} Material;


typedef struct Camera {
Vector3 position; 
Vector3 target; 
Vector3 up; 
float fovy; 
} Camera;


typedef struct VrDeviceInfo {
int hResolution; 
int vResolution; 
float hScreenSize; 
float vScreenSize; 
float vScreenCenter; 
float eyeToScreenDistance; 
float lensSeparationDistance; 
float interpupillaryDistance; 
float lensDistortionValues[4]; 
float chromaAbCorrection[4]; 
} VrDeviceInfo;


typedef struct VrStereoConfig {
Shader distortionShader; 
Matrix eyesProjection[2]; 
Matrix eyesViewOffset[2]; 
int eyeViewportRight[4]; 
int eyeViewportLeft[4]; 
} VrStereoConfig;



typedef enum {
LOG_ALL,
LOG_TRACE,
LOG_DEBUG,
LOG_INFO,
LOG_WARNING,
LOG_ERROR,
LOG_FATAL,
LOG_NONE
} TraceLogType;


typedef enum {
UNCOMPRESSED_GRAYSCALE = 1, 
UNCOMPRESSED_GRAY_ALPHA,
UNCOMPRESSED_R5G6B5, 
UNCOMPRESSED_R8G8B8, 
UNCOMPRESSED_R5G5B5A1, 
UNCOMPRESSED_R4G4B4A4, 
UNCOMPRESSED_R8G8B8A8, 
UNCOMPRESSED_R32, 
UNCOMPRESSED_R32G32B32, 
UNCOMPRESSED_R32G32B32A32, 
COMPRESSED_DXT1_RGB, 
COMPRESSED_DXT1_RGBA, 
COMPRESSED_DXT3_RGBA, 
COMPRESSED_DXT5_RGBA, 
COMPRESSED_ETC1_RGB, 
COMPRESSED_ETC2_RGB, 
COMPRESSED_ETC2_EAC_RGBA, 
COMPRESSED_PVRT_RGB, 
COMPRESSED_PVRT_RGBA, 
COMPRESSED_ASTC_4x4_RGBA, 
COMPRESSED_ASTC_8x8_RGBA 
} PixelFormat;




typedef enum {
FILTER_POINT = 0, 
FILTER_BILINEAR, 
FILTER_TRILINEAR, 
FILTER_ANISOTROPIC_4X, 
FILTER_ANISOTROPIC_8X, 
FILTER_ANISOTROPIC_16X, 
} TextureFilterMode;


typedef enum {
BLEND_ALPHA = 0,
BLEND_ADDITIVE,
BLEND_MULTIPLIED
} BlendMode;


typedef enum {
LOC_VERTEX_POSITION = 0,
LOC_VERTEX_TEXCOORD01,
LOC_VERTEX_TEXCOORD02,
LOC_VERTEX_NORMAL,
LOC_VERTEX_TANGENT,
LOC_VERTEX_COLOR,
LOC_MATRIX_MVP,
LOC_MATRIX_MODEL,
LOC_MATRIX_VIEW,
LOC_MATRIX_PROJECTION,
LOC_VECTOR_VIEW,
LOC_COLOR_DIFFUSE,
LOC_COLOR_SPECULAR,
LOC_COLOR_AMBIENT,
LOC_MAP_ALBEDO, 
LOC_MAP_METALNESS, 
LOC_MAP_NORMAL,
LOC_MAP_ROUGHNESS,
LOC_MAP_OCCLUSION,
LOC_MAP_EMISSION,
LOC_MAP_HEIGHT,
LOC_MAP_CUBEMAP,
LOC_MAP_IRRADIANCE,
LOC_MAP_PREFILTER,
LOC_MAP_BRDF
} ShaderLocationIndex;


typedef enum {
UNIFORM_FLOAT = 0,
UNIFORM_VEC2,
UNIFORM_VEC3,
UNIFORM_VEC4,
UNIFORM_INT,
UNIFORM_IVEC2,
UNIFORM_IVEC3,
UNIFORM_IVEC4,
UNIFORM_SAMPLER2D
} ShaderUniformDataType;

#define LOC_MAP_DIFFUSE LOC_MAP_ALBEDO
#define LOC_MAP_SPECULAR LOC_MAP_METALNESS


typedef enum {
MAP_ALBEDO = 0, 
MAP_METALNESS = 1, 
MAP_NORMAL = 2,
MAP_ROUGHNESS = 3,
MAP_OCCLUSION,
MAP_EMISSION,
MAP_HEIGHT,
MAP_CUBEMAP, 
MAP_IRRADIANCE, 
MAP_PREFILTER, 
MAP_BRDF
} MaterialMapType;

#define MAP_DIFFUSE MAP_ALBEDO
#define MAP_SPECULAR MAP_METALNESS
#endif

#if defined(__cplusplus)
extern "C" { 
#endif




RLAPI void rlMatrixMode(int mode); 
RLAPI void rlPushMatrix(void); 
RLAPI void rlPopMatrix(void); 
RLAPI void rlLoadIdentity(void); 
RLAPI void rlTranslatef(float x, float y, float z); 
RLAPI void rlRotatef(float angleDeg, float x, float y, float z); 
RLAPI void rlScalef(float x, float y, float z); 
RLAPI void rlMultMatrixf(float *matf); 
RLAPI void rlFrustum(double left, double right, double bottom, double top, double znear, double zfar);
RLAPI void rlOrtho(double left, double right, double bottom, double top, double znear, double zfar);
RLAPI void rlViewport(int x, int y, int width, int height); 




RLAPI void rlBegin(int mode); 
RLAPI void rlEnd(void); 
RLAPI void rlVertex2i(int x, int y); 
RLAPI void rlVertex2f(float x, float y); 
RLAPI void rlVertex3f(float x, float y, float z); 
RLAPI void rlTexCoord2f(float x, float y); 
RLAPI void rlNormal3f(float x, float y, float z); 
RLAPI void rlColor4ub(byte r, byte g, byte b, byte a); 
RLAPI void rlColor3f(float x, float y, float z); 
RLAPI void rlColor4f(float x, float y, float z, float w); 





RLAPI void rlEnableTexture(unsigned int id); 
RLAPI void rlDisableTexture(void); 
RLAPI void rlTextureParameters(unsigned int id, int param, int value); 
RLAPI void rlEnableRenderTexture(unsigned int id); 
RLAPI void rlDisableRenderTexture(void); 
RLAPI void rlEnableDepthTest(void); 
RLAPI void rlDisableDepthTest(void); 
RLAPI void rlEnableBackfaceCulling(void); 
RLAPI void rlDisableBackfaceCulling(void); 
RLAPI void rlEnableScissorTest(void); 
RLAPI void rlDisableScissorTest(void); 
RLAPI void rlScissor(int x, int y, int width, int height); 
RLAPI void rlEnableWireMode(void); 
RLAPI void rlDisableWireMode(void); 
RLAPI void rlDeleteTextures(unsigned int id); 
RLAPI void rlDeleteRenderTextures(RenderTexture2D target); 
RLAPI void rlDeleteShader(unsigned int id); 
RLAPI void rlDeleteVertexArrays(unsigned int id); 
RLAPI void rlDeleteBuffers(unsigned int id); 
RLAPI void rlClearColor(byte r, byte g, byte b, byte a); 
RLAPI void rlClearScreenBuffers(void); 
RLAPI void rlUpdateBuffer(int bufferId, void *data, int dataSize); 
RLAPI unsigned int rlLoadAttribBuffer(unsigned int vaoId, int shaderLoc, void *buffer, int size, bool dynamic); 




RLAPI void rlglInit(int width, int height); 
RLAPI void rlglClose(void); 
RLAPI void rlglDraw(void); 

RLAPI int rlGetVersion(void); 
RLAPI bool rlCheckBufferLimit(int vCount); 
RLAPI void rlSetDebugMarker(const char *text); 
RLAPI void rlLoadExtensions(void *loader); 
RLAPI Vector3 rlUnproject(Vector3 source, Matrix proj, Matrix view); 


RLAPI unsigned int rlLoadTexture(void *data, int width, int height, int format, int mipmapCount); 
RLAPI unsigned int rlLoadTextureDepth(int width, int height, int bits, bool useRenderBuffer); 
RLAPI unsigned int rlLoadTextureCubemap(void *data, int size, int format); 
RLAPI void rlUpdateTexture(unsigned int id, int width, int height, int format, const void *data); 
RLAPI void rlGetGlTextureFormats(int format, unsigned int *glInternalFormat, unsigned int *glFormat, unsigned int *glType); 
RLAPI void rlUnloadTexture(unsigned int id); 

RLAPI void rlGenerateMipmaps(Texture2D *texture); 
RLAPI void *rlReadTexturePixels(Texture2D texture); 
RLAPI unsigned char *rlReadScreenPixels(int width, int height); 


RLAPI RenderTexture2D rlLoadRenderTexture(int width, int height, int format, int depthBits, bool useDepthTexture); 
RLAPI void rlRenderTextureAttach(RenderTexture target, unsigned int id, int attachType); 
RLAPI bool rlRenderTextureComplete(RenderTexture target); 


RLAPI void rlLoadMesh(Mesh *mesh, bool dynamic); 
RLAPI void rlUpdateMesh(Mesh mesh, int buffer, int num); 
RLAPI void rlUpdateMeshAt(Mesh mesh, int buffer, int num, int index); 
RLAPI void rlDrawMesh(Mesh mesh, Material material, Matrix transform); 
RLAPI void rlUnloadMesh(Mesh mesh); 




#if defined(RLGL_STANDALONE)





RLAPI Shader LoadShader(const char *vsFileName, const char *fsFileName); 
RLAPI Shader LoadShaderCode(const char *vsCode, const char *fsCode); 
RLAPI void UnloadShader(Shader shader); 

RLAPI Shader GetShaderDefault(void); 
RLAPI Texture2D GetTextureDefault(void); 
RLAPI Texture2D GetShapesTexture(void); 
RLAPI Rectangle GetShapesTextureRec(void); 


RLAPI int GetShaderLocation(Shader shader, const char *uniformName); 
RLAPI void SetShaderValue(Shader shader, int uniformLoc, const void *value, int uniformType); 
RLAPI void SetShaderValueV(Shader shader, int uniformLoc, const void *value, int uniformType, int count); 
RLAPI void SetShaderValueMatrix(Shader shader, int uniformLoc, Matrix mat); 
RLAPI void SetMatrixProjection(Matrix proj); 
RLAPI void SetMatrixModelview(Matrix view); 
RLAPI Matrix GetMatrixModelview(void); 



RLAPI Texture2D GenTextureCubemap(Shader shader, Texture2D map, int size); 
RLAPI Texture2D GenTextureIrradiance(Shader shader, Texture2D cubemap, int size); 
RLAPI Texture2D GenTexturePrefilter(Shader shader, Texture2D cubemap, int size); 
RLAPI Texture2D GenTextureBRDF(Shader shader, int size); 


RLAPI void BeginShaderMode(Shader shader); 
RLAPI void EndShaderMode(void); 
RLAPI void BeginBlendMode(int mode); 
RLAPI void EndBlendMode(void); 


RLAPI void InitVrSimulator(void); 
RLAPI void CloseVrSimulator(void); 
RLAPI void UpdateVrTracking(Camera *camera); 
RLAPI void SetVrConfiguration(VrDeviceInfo info, Shader distortion); 
RLAPI bool IsVrSimulatorReady(void); 
RLAPI void ToggleVrMode(void); 
RLAPI void BeginVrDrawing(void); 
RLAPI void EndVrDrawing(void); 

RLAPI char *LoadFileText(const char *fileName); 
RLAPI int GetPixelDataSize(int width, int height, int format);
#endif

#if defined(__cplusplus)
}
#endif

#endif 







#if defined(RLGL_IMPLEMENTATION)

#if defined(RLGL_STANDALONE)
#include <stdio.h> 
#else

#if !defined(EXTERNAL_CONFIG_FLAGS)
#include "config.h" 
#endif
#include "raymath.h" 
#endif

#include <stdlib.h> 
#include <string.h> 
#include <math.h> 

#if defined(GRAPHICS_API_OPENGL_11)
#if defined(__APPLE__)
#include <OpenGL/gl.h> 
#include <OpenGL/glext.h>
#else

#if !defined(APIENTRY)
#if defined(_WIN32)
#define APIENTRY __stdcall
#else
#define APIENTRY
#endif
#endif

#if !defined(WINGDIAPI) && defined(_WIN32)
#define WINGDIAPI __declspec(dllimport)
#endif

#include <GL/gl.h> 
#endif
#endif

#if defined(GRAPHICS_API_OPENGL_21)
#define GRAPHICS_API_OPENGL_33 
#endif

#if defined(GRAPHICS_API_OPENGL_33)
#if defined(__APPLE__)
#include <OpenGL/gl3.h> 
#include <OpenGL/gl3ext.h> 
#else
#define GLAD_REALLOC RL_REALLOC
#define GLAD_FREE RL_FREE

#define GLAD_IMPLEMENTATION
#if defined(RLGL_STANDALONE)
#include "glad.h" 
#else
#include "external/glad.h" 
#endif
#endif
#endif

#if defined(GRAPHICS_API_OPENGL_ES2)
#include <EGL/egl.h> 
#include <GLES2/gl2.h> 
#include <GLES2/gl2ext.h> 
#endif




#if !defined(GL_SHADING_LANGUAGE_VERSION)
#define GL_SHADING_LANGUAGE_VERSION 0x8B8C
#endif

#if !defined(GL_COMPRESSED_RGB_S3TC_DXT1_EXT)
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0x83F0
#endif
#if !defined(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#endif
#if !defined(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT)
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#endif
#if !defined(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#endif
#if !defined(GL_ETC1_RGB8_OES)
#define GL_ETC1_RGB8_OES 0x8D64
#endif
#if !defined(GL_COMPRESSED_RGB8_ETC2)
#define GL_COMPRESSED_RGB8_ETC2 0x9274
#endif
#if !defined(GL_COMPRESSED_RGBA8_ETC2_EAC)
#define GL_COMPRESSED_RGBA8_ETC2_EAC 0x9278
#endif
#if !defined(GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG)
#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG 0x8C00
#endif
#if !defined(GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG)
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG 0x8C02
#endif
#if !defined(GL_COMPRESSED_RGBA_ASTC_4x4_KHR)
#define GL_COMPRESSED_RGBA_ASTC_4x4_KHR 0x93b0
#endif
#if !defined(GL_COMPRESSED_RGBA_ASTC_8x8_KHR)
#define GL_COMPRESSED_RGBA_ASTC_8x8_KHR 0x93b7
#endif

#if !defined(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT)
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif

#if !defined(GL_TEXTURE_MAX_ANISOTROPY_EXT)
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#endif

#if defined(GRAPHICS_API_OPENGL_11)
#define GL_UNSIGNED_SHORT_5_6_5 0x8363
#define GL_UNSIGNED_SHORT_5_5_5_1 0x8034
#define GL_UNSIGNED_SHORT_4_4_4_4 0x8033
#endif

#if defined(GRAPHICS_API_OPENGL_21)
#define GL_LUMINANCE 0x1909
#define GL_LUMINANCE_ALPHA 0x190A
#endif

#if defined(GRAPHICS_API_OPENGL_ES2)
#define glClearDepth glClearDepthf
#define GL_READ_FRAMEBUFFER GL_FRAMEBUFFER
#define GL_DRAW_FRAMEBUFFER GL_FRAMEBUFFER
#endif


#define DEFAULT_ATTRIB_POSITION_NAME "vertexPosition" 
#define DEFAULT_ATTRIB_TEXCOORD_NAME "vertexTexCoord" 
#define DEFAULT_ATTRIB_NORMAL_NAME "vertexNormal" 
#define DEFAULT_ATTRIB_COLOR_NAME "vertexColor" 
#define DEFAULT_ATTRIB_TANGENT_NAME "vertexTangent" 
#define DEFAULT_ATTRIB_TEXCOORD2_NAME "vertexTexCoord2" 






typedef struct DynamicBuffer {
int vCounter; 
int tcCounter; 
int cCounter; 

float *vertices; 
float *texcoords; 
unsigned char *colors; 
#if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)
unsigned int *indices; 
#elif defined(GRAPHICS_API_OPENGL_ES2)
unsigned short *indices; 
#endif
unsigned int vaoId; 
unsigned int vboId[4]; 
} DynamicBuffer;


typedef struct DrawCall {
int mode; 
int vertexCount; 
int vertexAlignment; 


unsigned int textureId; 



} DrawCall;

#if defined(SUPPORT_VR_SIMULATOR)

typedef struct VrStereoConfig {
Shader distortionShader; 
Matrix eyesProjection[2]; 
Matrix eyesViewOffset[2]; 
int eyeViewportRight[4]; 
int eyeViewportLeft[4]; 
} VrStereoConfig;
#endif

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
typedef struct rlglData {
struct {
int currentMatrixMode; 
Matrix *currentMatrix; 
Matrix modelview; 
Matrix projection; 
Matrix transform; 
bool doTransform; 
Matrix stack[MAX_MATRIX_STACK_SIZE];
int stackCounter; 

DynamicBuffer vertexData[MAX_BATCH_BUFFERING];
int currentBuffer; 
DrawCall *draws; 
int drawsCounter; 

Texture2D shapesTexture; 
Rectangle shapesTextureRec; 
unsigned int defaultTextureId; 
unsigned int defaultVShaderId; 
unsigned int defaultFShaderId; 
Shader defaultShader; 
Shader currentShader; 
float currentDepth; 

int framebufferWidth; 
int framebufferHeight; 

} State;
struct {
bool vao; 
bool texNPOT; 
bool texDepth; 
bool texFloat32; 
bool texCompDXT; 
bool texCompETC1; 
bool texCompETC2; 
bool texCompPVRT; 
bool texCompASTC; 
bool texMirrorClamp; 
bool texAnisoFilter; 
bool debugMarker; 

float maxAnisotropicLevel; 
int maxDepthBits; 

} ExtSupported; 
#if defined(SUPPORT_VR_SIMULATOR)
struct {
VrStereoConfig config; 
RenderTexture2D stereoFbo; 
bool simulatorReady; 
bool stereoRender; 
} Vr;
#endif 
} rlglData;
#endif 




#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
static rlglData RLGL = { 0 };
#endif 

#if defined(GRAPHICS_API_OPENGL_ES2)

static PFNGLGENVERTEXARRAYSOESPROC glGenVertexArrays; 
static PFNGLBINDVERTEXARRAYOESPROC glBindVertexArray; 
static PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArrays; 
#endif




#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
static unsigned int CompileShader(const char *shaderStr, int type); 
static unsigned int LoadShaderProgram(unsigned int vShaderId, unsigned int fShaderId); 

static Shader LoadShaderDefault(void); 
static void SetShaderDefaultLocations(Shader *shader); 
static void UnloadShaderDefault(void); 

static void LoadBuffersDefault(void); 
static void UpdateBuffersDefault(void); 
static void DrawBuffersDefault(void); 
static void UnloadBuffersDefault(void); 

static void GenDrawCube(void); 
static void GenDrawQuad(void); 

#if defined(SUPPORT_VR_SIMULATOR)
static void SetStereoView(int eye, Matrix matProjection, Matrix matModelView); 
#endif

#endif 

#if defined(GRAPHICS_API_OPENGL_11)
static int GenerateMipmaps(unsigned char *data, int baseWidth, int baseHeight);
static Color *GenNextMipmap(Color *srcData, int srcWidth, int srcHeight);
#endif





#if defined(GRAPHICS_API_OPENGL_11)



void rlMatrixMode(int mode)
{
switch (mode)
{
case RL_PROJECTION: glMatrixMode(GL_PROJECTION); break;
case RL_MODELVIEW: glMatrixMode(GL_MODELVIEW); break;
case RL_TEXTURE: glMatrixMode(GL_TEXTURE); break;
default: break;
}
}

void rlFrustum(double left, double right, double bottom, double top, double znear, double zfar)
{
glFrustum(left, right, bottom, top, znear, zfar);
}

void rlOrtho(double left, double right, double bottom, double top, double znear, double zfar)
{
glOrtho(left, right, bottom, top, znear, zfar);
}

void rlPushMatrix(void) { glPushMatrix(); }
void rlPopMatrix(void) { glPopMatrix(); }
void rlLoadIdentity(void) { glLoadIdentity(); }
void rlTranslatef(float x, float y, float z) { glTranslatef(x, y, z); }
void rlRotatef(float angleDeg, float x, float y, float z) { glRotatef(angleDeg, x, y, z); }
void rlScalef(float x, float y, float z) { glScalef(x, y, z); }
void rlMultMatrixf(float *matf) { glMultMatrixf(matf); }

#elif defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)


void rlMatrixMode(int mode)
{
if (mode == RL_PROJECTION) RLGL.State.currentMatrix = &RLGL.State.projection;
else if (mode == RL_MODELVIEW) RLGL.State.currentMatrix = &RLGL.State.modelview;


RLGL.State.currentMatrixMode = mode;
}


void rlPushMatrix(void)
{
if (RLGL.State.stackCounter >= MAX_MATRIX_STACK_SIZE) TRACELOG(LOG_ERROR, "RLGL: Matrix stack overflow (MAX_MATRIX_STACK_SIZE)");

if (RLGL.State.currentMatrixMode == RL_MODELVIEW)
{
RLGL.State.doTransform = true;
RLGL.State.currentMatrix = &RLGL.State.transform;
}

RLGL.State.stack[RLGL.State.stackCounter] = *RLGL.State.currentMatrix;
RLGL.State.stackCounter++;
}


void rlPopMatrix(void)
{
if (RLGL.State.stackCounter > 0)
{
Matrix mat = RLGL.State.stack[RLGL.State.stackCounter - 1];
*RLGL.State.currentMatrix = mat;
RLGL.State.stackCounter--;
}

if ((RLGL.State.stackCounter == 0) && (RLGL.State.currentMatrixMode == RL_MODELVIEW))
{
RLGL.State.currentMatrix = &RLGL.State.modelview;
RLGL.State.doTransform = false;
}
}


void rlLoadIdentity(void)
{
*RLGL.State.currentMatrix = MatrixIdentity();
}


void rlTranslatef(float x, float y, float z)
{
Matrix matTranslation = MatrixTranslate(x, y, z);


*RLGL.State.currentMatrix = MatrixMultiply(matTranslation, *RLGL.State.currentMatrix);
}


void rlRotatef(float angleDeg, float x, float y, float z)
{
Matrix matRotation = MatrixIdentity();

Vector3 axis = (Vector3){ x, y, z };
matRotation = MatrixRotate(Vector3Normalize(axis), angleDeg*DEG2RAD);


*RLGL.State.currentMatrix = MatrixMultiply(matRotation, *RLGL.State.currentMatrix);
}


void rlScalef(float x, float y, float z)
{
Matrix matScale = MatrixScale(x, y, z);


*RLGL.State.currentMatrix = MatrixMultiply(matScale, *RLGL.State.currentMatrix);
}


void rlMultMatrixf(float *matf)
{

Matrix mat = { matf[0], matf[4], matf[8], matf[12],
matf[1], matf[5], matf[9], matf[13],
matf[2], matf[6], matf[10], matf[14],
matf[3], matf[7], matf[11], matf[15] };

*RLGL.State.currentMatrix = MatrixMultiply(*RLGL.State.currentMatrix, mat);
}


void rlFrustum(double left, double right, double bottom, double top, double znear, double zfar)
{
Matrix matPerps = MatrixFrustum(left, right, bottom, top, znear, zfar);

*RLGL.State.currentMatrix = MatrixMultiply(*RLGL.State.currentMatrix, matPerps);
}


void rlOrtho(double left, double right, double bottom, double top, double znear, double zfar)
{
Matrix matOrtho = MatrixOrtho(left, right, bottom, top, znear, zfar);

*RLGL.State.currentMatrix = MatrixMultiply(*RLGL.State.currentMatrix, matOrtho);
}

#endif



void rlViewport(int x, int y, int width, int height)
{
glViewport(x, y, width, height);
}




#if defined(GRAPHICS_API_OPENGL_11)



void rlBegin(int mode)
{
switch (mode)
{
case RL_LINES: glBegin(GL_LINES); break;
case RL_TRIANGLES: glBegin(GL_TRIANGLES); break;
case RL_QUADS: glBegin(GL_QUADS); break;
default: break;
}
}

void rlEnd() { glEnd(); }
void rlVertex2i(int x, int y) { glVertex2i(x, y); }
void rlVertex2f(float x, float y) { glVertex2f(x, y); }
void rlVertex3f(float x, float y, float z) { glVertex3f(x, y, z); }
void rlTexCoord2f(float x, float y) { glTexCoord2f(x, y); }
void rlNormal3f(float x, float y, float z) { glNormal3f(x, y, z); }
void rlColor4ub(byte r, byte g, byte b, byte a) { glColor4ub(r, g, b, a); }
void rlColor3f(float x, float y, float z) { glColor3f(x, y, z); }
void rlColor4f(float x, float y, float z, float w) { glColor4f(x, y, z, w); }

#elif defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)


void rlBegin(int mode)
{


if (RLGL.State.draws[RLGL.State.drawsCounter - 1].mode != mode)
{
if (RLGL.State.draws[RLGL.State.drawsCounter - 1].vertexCount > 0)
{





if (RLGL.State.draws[RLGL.State.drawsCounter - 1].mode == RL_LINES) RLGL.State.draws[RLGL.State.drawsCounter - 1].vertexAlignment = ((RLGL.State.draws[RLGL.State.drawsCounter - 1].vertexCount < 4)? RLGL.State.draws[RLGL.State.drawsCounter - 1].vertexCount : RLGL.State.draws[RLGL.State.drawsCounter - 1].vertexCount%4);
else if (RLGL.State.draws[RLGL.State.drawsCounter - 1].mode == RL_TRIANGLES) RLGL.State.draws[RLGL.State.drawsCounter - 1].vertexAlignment = ((RLGL.State.draws[RLGL.State.drawsCounter - 1].vertexCount < 4)? 1 : (4 - (RLGL.State.draws[RLGL.State.drawsCounter - 1].vertexCount%4)));

else RLGL.State.draws[RLGL.State.drawsCounter - 1].vertexAlignment = 0;

if (rlCheckBufferLimit(RLGL.State.draws[RLGL.State.drawsCounter - 1].vertexAlignment)) rlglDraw();
else
{
RLGL.State.vertexData[RLGL.State.currentBuffer].vCounter += RLGL.State.draws[RLGL.State.drawsCounter - 1].vertexAlignment;
RLGL.State.vertexData[RLGL.State.currentBuffer].cCounter += RLGL.State.draws[RLGL.State.drawsCounter - 1].vertexAlignment;
RLGL.State.vertexData[RLGL.State.currentBuffer].tcCounter += RLGL.State.draws[RLGL.State.drawsCounter - 1].vertexAlignment;

RLGL.State.drawsCounter++;
}
}

if (RLGL.State.drawsCounter >= MAX_DRAWCALL_REGISTERED) rlglDraw();

RLGL.State.draws[RLGL.State.drawsCounter - 1].mode = mode;
RLGL.State.draws[RLGL.State.drawsCounter - 1].vertexCount = 0;
RLGL.State.draws[RLGL.State.drawsCounter - 1].textureId = RLGL.State.defaultTextureId;
}
}


void rlEnd(void)
{




if (RLGL.State.vertexData[RLGL.State.currentBuffer].vCounter != RLGL.State.vertexData[RLGL.State.currentBuffer].cCounter)
{
int addColors = RLGL.State.vertexData[RLGL.State.currentBuffer].vCounter - RLGL.State.vertexData[RLGL.State.currentBuffer].cCounter;

for (int i = 0; i < addColors; i++)
{
RLGL.State.vertexData[RLGL.State.currentBuffer].colors[4*RLGL.State.vertexData[RLGL.State.currentBuffer].cCounter] = RLGL.State.vertexData[RLGL.State.currentBuffer].colors[4*RLGL.State.vertexData[RLGL.State.currentBuffer].cCounter - 4];
RLGL.State.vertexData[RLGL.State.currentBuffer].colors[4*RLGL.State.vertexData[RLGL.State.currentBuffer].cCounter + 1] = RLGL.State.vertexData[RLGL.State.currentBuffer].colors[4*RLGL.State.vertexData[RLGL.State.currentBuffer].cCounter - 3];
RLGL.State.vertexData[RLGL.State.currentBuffer].colors[4*RLGL.State.vertexData[RLGL.State.currentBuffer].cCounter + 2] = RLGL.State.vertexData[RLGL.State.currentBuffer].colors[4*RLGL.State.vertexData[RLGL.State.currentBuffer].cCounter - 2];
RLGL.State.vertexData[RLGL.State.currentBuffer].colors[4*RLGL.State.vertexData[RLGL.State.currentBuffer].cCounter + 3] = RLGL.State.vertexData[RLGL.State.currentBuffer].colors[4*RLGL.State.vertexData[RLGL.State.currentBuffer].cCounter - 1];
RLGL.State.vertexData[RLGL.State.currentBuffer].cCounter++;
}
}


if (RLGL.State.vertexData[RLGL.State.currentBuffer].vCounter != RLGL.State.vertexData[RLGL.State.currentBuffer].tcCounter)
{
int addTexCoords = RLGL.State.vertexData[RLGL.State.currentBuffer].vCounter - RLGL.State.vertexData[RLGL.State.currentBuffer].tcCounter;

for (int i = 0; i < addTexCoords; i++)
{
RLGL.State.vertexData[RLGL.State.currentBuffer].texcoords[2*RLGL.State.vertexData[RLGL.State.currentBuffer].tcCounter] = 0.0f;
RLGL.State.vertexData[RLGL.State.currentBuffer].texcoords[2*RLGL.State.vertexData[RLGL.State.currentBuffer].tcCounter + 1] = 0.0f;
RLGL.State.vertexData[RLGL.State.currentBuffer].tcCounter++;
}
}






RLGL.State.currentDepth += (1.0f/20000.0f);



if ((RLGL.State.vertexData[RLGL.State.currentBuffer].vCounter) >= (MAX_BATCH_ELEMENTS*4 - 4))
{



for (int i = RLGL.State.stackCounter; i >= 0; i--) rlPopMatrix();
rlglDraw();
}
}



void rlVertex3f(float x, float y, float z)
{
Vector3 vec = { x, y, z };


if (RLGL.State.doTransform) vec = Vector3Transform(vec, RLGL.State.transform);


if (RLGL.State.vertexData[RLGL.State.currentBuffer].vCounter < (MAX_BATCH_ELEMENTS*4))
{
RLGL.State.vertexData[RLGL.State.currentBuffer].vertices[3*RLGL.State.vertexData[RLGL.State.currentBuffer].vCounter] = vec.x;
RLGL.State.vertexData[RLGL.State.currentBuffer].vertices[3*RLGL.State.vertexData[RLGL.State.currentBuffer].vCounter + 1] = vec.y;
RLGL.State.vertexData[RLGL.State.currentBuffer].vertices[3*RLGL.State.vertexData[RLGL.State.currentBuffer].vCounter + 2] = vec.z;
RLGL.State.vertexData[RLGL.State.currentBuffer].vCounter++;

RLGL.State.draws[RLGL.State.drawsCounter - 1].vertexCount++;
}
else TRACELOG(LOG_ERROR, "RLGL: Batch elements overflow (MAX_BATCH_ELEMENTS)");
}


void rlVertex2f(float x, float y)
{
rlVertex3f(x, y, RLGL.State.currentDepth);
}


void rlVertex2i(int x, int y)
{
rlVertex3f((float)x, (float)y, RLGL.State.currentDepth);
}



void rlTexCoord2f(float x, float y)
{
RLGL.State.vertexData[RLGL.State.currentBuffer].texcoords[2*RLGL.State.vertexData[RLGL.State.currentBuffer].tcCounter] = x;
RLGL.State.vertexData[RLGL.State.currentBuffer].texcoords[2*RLGL.State.vertexData[RLGL.State.currentBuffer].tcCounter + 1] = y;
RLGL.State.vertexData[RLGL.State.currentBuffer].tcCounter++;
}



void rlNormal3f(float x, float y, float z)
{

}


void rlColor4ub(byte x, byte y, byte z, byte w)
{
RLGL.State.vertexData[RLGL.State.currentBuffer].colors[4*RLGL.State.vertexData[RLGL.State.currentBuffer].cCounter] = x;
RLGL.State.vertexData[RLGL.State.currentBuffer].colors[4*RLGL.State.vertexData[RLGL.State.currentBuffer].cCounter + 1] = y;
RLGL.State.vertexData[RLGL.State.currentBuffer].colors[4*RLGL.State.vertexData[RLGL.State.currentBuffer].cCounter + 2] = z;
RLGL.State.vertexData[RLGL.State.currentBuffer].colors[4*RLGL.State.vertexData[RLGL.State.currentBuffer].cCounter + 3] = w;
RLGL.State.vertexData[RLGL.State.currentBuffer].cCounter++;
}


void rlColor4f(float r, float g, float b, float a)
{
rlColor4ub((byte)(r*255), (byte)(g*255), (byte)(b*255), (byte)(a*255));
}


void rlColor3f(float x, float y, float z)
{
rlColor4ub((byte)(x*255), (byte)(y*255), (byte)(z*255), 255);
}

#endif






void rlEnableTexture(unsigned int id)
{
#if defined(GRAPHICS_API_OPENGL_11)
glEnable(GL_TEXTURE_2D);
glBindTexture(GL_TEXTURE_2D, id);
#endif

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
if (RLGL.State.draws[RLGL.State.drawsCounter - 1].textureId != id)
{
if (RLGL.State.draws[RLGL.State.drawsCounter - 1].vertexCount > 0)
{





if (RLGL.State.draws[RLGL.State.drawsCounter - 1].mode == RL_LINES) RLGL.State.draws[RLGL.State.drawsCounter - 1].vertexAlignment = ((RLGL.State.draws[RLGL.State.drawsCounter - 1].vertexCount < 4)? RLGL.State.draws[RLGL.State.drawsCounter - 1].vertexCount : RLGL.State.draws[RLGL.State.drawsCounter - 1].vertexCount%4);
else if (RLGL.State.draws[RLGL.State.drawsCounter - 1].mode == RL_TRIANGLES) RLGL.State.draws[RLGL.State.drawsCounter - 1].vertexAlignment = ((RLGL.State.draws[RLGL.State.drawsCounter - 1].vertexCount < 4)? 1 : (4 - (RLGL.State.draws[RLGL.State.drawsCounter - 1].vertexCount%4)));

else RLGL.State.draws[RLGL.State.drawsCounter - 1].vertexAlignment = 0;

if (rlCheckBufferLimit(RLGL.State.draws[RLGL.State.drawsCounter - 1].vertexAlignment)) rlglDraw();
else
{
RLGL.State.vertexData[RLGL.State.currentBuffer].vCounter += RLGL.State.draws[RLGL.State.drawsCounter - 1].vertexAlignment;
RLGL.State.vertexData[RLGL.State.currentBuffer].cCounter += RLGL.State.draws[RLGL.State.drawsCounter - 1].vertexAlignment;
RLGL.State.vertexData[RLGL.State.currentBuffer].tcCounter += RLGL.State.draws[RLGL.State.drawsCounter - 1].vertexAlignment;

RLGL.State.drawsCounter++;
}
}

if (RLGL.State.drawsCounter >= MAX_DRAWCALL_REGISTERED) rlglDraw();

RLGL.State.draws[RLGL.State.drawsCounter - 1].textureId = id;
RLGL.State.draws[RLGL.State.drawsCounter - 1].vertexCount = 0;
}
#endif
}


void rlDisableTexture(void)
{
#if defined(GRAPHICS_API_OPENGL_11)
glDisable(GL_TEXTURE_2D);
glBindTexture(GL_TEXTURE_2D, 0);
#else


if (RLGL.State.vertexData[RLGL.State.currentBuffer].vCounter >= (MAX_BATCH_ELEMENTS*4)) rlglDraw();
#endif
}


void rlTextureParameters(unsigned int id, int param, int value)
{
glBindTexture(GL_TEXTURE_2D, id);

switch (param)
{
case RL_TEXTURE_WRAP_S:
case RL_TEXTURE_WRAP_T:
{
if (value == RL_WRAP_MIRROR_CLAMP)
{
#if !defined(GRAPHICS_API_OPENGL_11)
if (RLGL.ExtSupported.texMirrorClamp) glTexParameteri(GL_TEXTURE_2D, param, value);
else TRACELOG(LOG_WARNING, "GL: Clamp mirror wrap mode not supported (GL_MIRROR_CLAMP_EXT)");
#endif
}
else glTexParameteri(GL_TEXTURE_2D, param, value);

} break;
case RL_TEXTURE_MAG_FILTER:
case RL_TEXTURE_MIN_FILTER: glTexParameteri(GL_TEXTURE_2D, param, value); break;
case RL_TEXTURE_ANISOTROPIC_FILTER:
{
#if !defined(GRAPHICS_API_OPENGL_11)
if (value <= RLGL.ExtSupported.maxAnisotropicLevel) glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)value);
else if (RLGL.ExtSupported.maxAnisotropicLevel > 0.0f)
{
TRACELOG(LOG_WARNING, "GL: Maximum anisotropic filter level supported is %iX", id, RLGL.ExtSupported.maxAnisotropicLevel);
glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)value);
}
else TRACELOG(LOG_WARNING, "GL: Anisotropic filtering not supported");
#endif
} break;
default: break;
}

glBindTexture(GL_TEXTURE_2D, 0);
}


void rlEnableRenderTexture(unsigned int id)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
glBindFramebuffer(GL_FRAMEBUFFER, id);



#endif
}


void rlDisableRenderTexture(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
glBindFramebuffer(GL_FRAMEBUFFER, 0);



#endif
}


void rlEnableDepthTest(void) { glEnable(GL_DEPTH_TEST); }


void rlDisableDepthTest(void) { glDisable(GL_DEPTH_TEST); }


void rlEnableBackfaceCulling(void) { glEnable(GL_CULL_FACE); }


void rlDisableBackfaceCulling(void) { glDisable(GL_CULL_FACE); }


RLAPI void rlEnableScissorTest(void) { glEnable(GL_SCISSOR_TEST); }


RLAPI void rlDisableScissorTest(void) { glDisable(GL_SCISSOR_TEST); }


RLAPI void rlScissor(int x, int y, int width, int height) { glScissor(x, y, width, height); }


void rlEnableWireMode(void)
{
#if defined (GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)

glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif
}


void rlDisableWireMode(void)
{
#if defined (GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)

glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif
}


void rlDeleteTextures(unsigned int id)
{
if (id > 0) glDeleteTextures(1, &id);
}


void rlDeleteRenderTextures(RenderTexture2D target)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
if (target.texture.id > 0) glDeleteTextures(1, &target.texture.id);
if (target.depth.id > 0)
{
if (target.depthTexture) glDeleteTextures(1, &target.depth.id);
else glDeleteRenderbuffers(1, &target.depth.id);
}

if (target.id > 0) glDeleteFramebuffers(1, &target.id);

TRACELOG(LOG_INFO, "FBO: [ID %i] Unloaded render texture data from VRAM (GPU)", target.id);
#endif
}


void rlDeleteShader(unsigned int id)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
if (id != 0) glDeleteProgram(id);
#endif
}


void rlDeleteVertexArrays(unsigned int id)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
if (RLGL.ExtSupported.vao)
{
if (id != 0) glDeleteVertexArrays(1, &id);
TRACELOG(LOG_INFO, "VAO: [ID %i] Unloaded vertex data from VRAM (GPU)", id);
}
#endif
}


void rlDeleteBuffers(unsigned int id)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
if (id != 0)
{
glDeleteBuffers(1, &id);
if (!RLGL.ExtSupported.vao) TRACELOG(LOG_INFO, "VBO: [ID %i] Unloaded vertex data from VRAM (GPU)", id);
}
#endif
}


void rlClearColor(byte r, byte g, byte b, byte a)
{

float cr = (float)r/255;
float cg = (float)g/255;
float cb = (float)b/255;
float ca = (float)a/255;

glClearColor(cr, cg, cb, ca);
}


void rlClearScreenBuffers(void)
{
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

}


void rlUpdateBuffer(int bufferId, void *data, int dataSize)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
glBindBuffer(GL_ARRAY_BUFFER, bufferId);
glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, data);
#endif
}






void rlglInit(int width, int height)
{



TRACELOG(LOG_INFO, "GL: OpenGL device information:");
TRACELOG(LOG_INFO, " > Vendor: %s", glGetString(GL_VENDOR));
TRACELOG(LOG_INFO, " > Renderer: %s", glGetString(GL_RENDERER));
TRACELOG(LOG_INFO, " > Version: %s", glGetString(GL_VERSION));
TRACELOG(LOG_INFO, " > GLSL: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

























#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)

GLint numExt = 0;

#if defined(GRAPHICS_API_OPENGL_33)

RLGL.ExtSupported.vao = true;


RLGL.ExtSupported.texNPOT = true;
RLGL.ExtSupported.texFloat32 = true;
RLGL.ExtSupported.texDepth = true;



glGetIntegerv(GL_NUM_EXTENSIONS, &numExt);


const char **extList = RL_MALLOC(sizeof(const char *)*numExt);


for (int i = 0; i < numExt; i++) extList[i] = (const char *)glGetStringi(GL_EXTENSIONS, i);

#elif defined(GRAPHICS_API_OPENGL_ES2)

const char **extList = RL_MALLOC(sizeof(const char *)*512);

const char *extensions = (const char *)glGetString(GL_EXTENSIONS); 


int len = strlen(extensions) + 1;
char *extensionsDup = (char *)RL_CALLOC(len, sizeof(char));
strcpy(extensionsDup, extensions);

extList[numExt] = extensionsDup;

for (int i = 0; i < len; i++)
{
if (extensionsDup[i] == ' ')
{
extensionsDup[i] = '\0';

numExt++;
extList[numExt] = &extensionsDup[i + 1];
}
}


#endif

TRACELOG(LOG_INFO, "GL: Supported extensions count: %i", numExt);





for (int i = 0; i < numExt; i++)
{
#if defined(GRAPHICS_API_OPENGL_ES2)


if (strcmp(extList[i], (const char *)"GL_OES_vertex_array_object") == 0)
{


glGenVertexArrays = (PFNGLGENVERTEXARRAYSOESPROC)eglGetProcAddress("glGenVertexArraysOES");
glBindVertexArray = (PFNGLBINDVERTEXARRAYOESPROC)eglGetProcAddress("glBindVertexArrayOES");
glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSOESPROC)eglGetProcAddress("glDeleteVertexArraysOES");


if ((glGenVertexArrays != NULL) && (glBindVertexArray != NULL) && (glDeleteVertexArrays != NULL)) RLGL.ExtSupported.vao = true;
}



if (strcmp(extList[i], (const char *)"GL_OES_texture_npot") == 0) RLGL.ExtSupported.texNPOT = true;


if (strcmp(extList[i], (const char *)"GL_OES_texture_float") == 0) RLGL.ExtSupported.texFloat32 = true;


if ((strcmp(extList[i], (const char *)"GL_OES_depth_texture") == 0) ||
(strcmp(extList[i], (const char *)"GL_WEBGL_depth_texture") == 0)) RLGL.ExtSupported.texDepth = true;

if (strcmp(extList[i], (const char *)"GL_OES_depth24") == 0) RLGL.ExtSupported.maxDepthBits = 24;
if (strcmp(extList[i], (const char *)"GL_OES_depth32") == 0) RLGL.ExtSupported.maxDepthBits = 32;
#endif

if ((strcmp(extList[i], (const char *)"GL_EXT_texture_compression_s3tc") == 0) ||
(strcmp(extList[i], (const char *)"GL_WEBGL_compressed_texture_s3tc") == 0) ||
(strcmp(extList[i], (const char *)"GL_WEBKIT_WEBGL_compressed_texture_s3tc") == 0)) RLGL.ExtSupported.texCompDXT = true;


if ((strcmp(extList[i], (const char *)"GL_OES_compressed_ETC1_RGB8_texture") == 0) ||
(strcmp(extList[i], (const char *)"GL_WEBGL_compressed_texture_etc1") == 0)) RLGL.ExtSupported.texCompETC1 = true;


if (strcmp(extList[i], (const char *)"GL_ARB_ES3_compatibility") == 0) RLGL.ExtSupported.texCompETC2 = true;


if (strcmp(extList[i], (const char *)"GL_IMG_texture_compression_pvrtc") == 0) RLGL.ExtSupported.texCompPVRT = true;


if (strcmp(extList[i], (const char *)"GL_KHR_texture_compression_astc_hdr") == 0) RLGL.ExtSupported.texCompASTC = true;


if (strcmp(extList[i], (const char *)"GL_EXT_texture_filter_anisotropic") == 0)
{
RLGL.ExtSupported.texAnisoFilter = true;
glGetFloatv(0x84FF, &RLGL.ExtSupported.maxAnisotropicLevel); 
}


if (strcmp(extList[i], (const char *)"GL_EXT_texture_mirror_clamp") == 0) RLGL.ExtSupported.texMirrorClamp = true;


if (strcmp(extList[i], (const char *)"GL_EXT_debug_marker") == 0) RLGL.ExtSupported.debugMarker = true;
}


RL_FREE(extList);

#if defined(GRAPHICS_API_OPENGL_ES2)
RL_FREE(extensionsDup); 

if (RLGL.ExtSupported.vao) TRACELOG(LOG_INFO, "GL: VAO extension detected, VAO functions initialized successfully");
else TRACELOG(LOG_WARNING, "GL: VAO extension not found, VAO usage not supported");

if (RLGL.ExtSupported.texNPOT) TRACELOG(LOG_INFO, "GL: NPOT textures extension detected, full NPOT textures supported");
else TRACELOG(LOG_WARNING, "GL: NPOT textures extension not found, limited NPOT support (no-mipmaps, no-repeat)");
#endif

if (RLGL.ExtSupported.texCompDXT) TRACELOG(LOG_INFO, "GL: DXT compressed textures supported");
if (RLGL.ExtSupported.texCompETC1) TRACELOG(LOG_INFO, "GL: ETC1 compressed textures supported");
if (RLGL.ExtSupported.texCompETC2) TRACELOG(LOG_INFO, "GL: ETC2/EAC compressed textures supported");
if (RLGL.ExtSupported.texCompPVRT) TRACELOG(LOG_INFO, "GL: PVRT compressed textures supported");
if (RLGL.ExtSupported.texCompASTC) TRACELOG(LOG_INFO, "GL: ASTC compressed textures supported");

if (RLGL.ExtSupported.texAnisoFilter) TRACELOG(LOG_INFO, "GL: Anisotropic textures filtering supported (max: %.0fX)", RLGL.ExtSupported.maxAnisotropicLevel);
if (RLGL.ExtSupported.texMirrorClamp) TRACELOG(LOG_INFO, "GL: Mirror clamp wrap texture mode supported");

if (RLGL.ExtSupported.debugMarker) TRACELOG(LOG_INFO, "GL: Debug Marker supported");




unsigned char pixels[4] = { 255, 255, 255, 255 }; 
RLGL.State.defaultTextureId = rlLoadTexture(pixels, 1, 1, UNCOMPRESSED_R8G8B8A8, 1);

if (RLGL.State.defaultTextureId != 0) TRACELOG(LOG_INFO, "TEXTURE: [ID %i] Default texture loaded successfully", RLGL.State.defaultTextureId);
else TRACELOG(LOG_WARNING, "TEXTURE: Failed to load default texture");


RLGL.State.defaultShader = LoadShaderDefault();
RLGL.State.currentShader = RLGL.State.defaultShader;


LoadBuffersDefault();


RLGL.State.transform = MatrixIdentity();


RLGL.State.draws = (DrawCall *)RL_MALLOC(sizeof(DrawCall)*MAX_DRAWCALL_REGISTERED);

for (int i = 0; i < MAX_DRAWCALL_REGISTERED; i++)
{
RLGL.State.draws[i].mode = RL_QUADS;
RLGL.State.draws[i].vertexCount = 0;
RLGL.State.draws[i].vertexAlignment = 0;


RLGL.State.draws[i].textureId = RLGL.State.defaultTextureId;


}

RLGL.State.drawsCounter = 1;


for (int i = 0; i < MAX_MATRIX_STACK_SIZE; i++) RLGL.State.stack[i] = MatrixIdentity();


RLGL.State.projection = MatrixIdentity();
RLGL.State.modelview = MatrixIdentity();
RLGL.State.currentMatrix = &RLGL.State.modelview;
#endif 




glDepthFunc(GL_LEQUAL); 
glDisable(GL_DEPTH_TEST); 


glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
glEnable(GL_BLEND); 



glCullFace(GL_BACK); 
glFrontFace(GL_CCW); 
glEnable(GL_CULL_FACE); 

#if defined(GRAPHICS_API_OPENGL_11)

glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); 
glShadeModel(GL_SMOOTH); 
#endif


glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 
glClearDepth(1.0f); 
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 


RLGL.State.framebufferWidth = width;
RLGL.State.framebufferHeight = height;


RLGL.State.shapesTexture = GetTextureDefault();
RLGL.State.shapesTextureRec = (Rectangle){ 0.0f, 0.0f, 1.0f, 1.0f };

TRACELOG(LOG_INFO, "RLGL: Default state initialized successfully");
}


void rlglClose(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
UnloadShaderDefault(); 
UnloadBuffersDefault(); 
glDeleteTextures(1, &RLGL.State.defaultTextureId); 

TRACELOG(LOG_INFO, "TEXTURE: [ID %i] Unloaded default texture data from VRAM (GPU)", RLGL.State.defaultTextureId);

RL_FREE(RLGL.State.draws);
#endif
}


void rlglDraw(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)

if (RLGL.State.vertexData[RLGL.State.currentBuffer].vCounter > 0)
{
UpdateBuffersDefault();
DrawBuffersDefault(); 
}
#endif
}


int rlGetVersion(void)
{
#if defined(GRAPHICS_API_OPENGL_11)
return OPENGL_11;
#elif defined(GRAPHICS_API_OPENGL_21)
#if defined(__APPLE__)
return OPENGL_33; 
#else
return OPENGL_21;
#endif
#elif defined(GRAPHICS_API_OPENGL_33)
return OPENGL_33;
#elif defined(GRAPHICS_API_OPENGL_ES2)
return OPENGL_ES_20;
#endif
}


bool rlCheckBufferLimit(int vCount)
{
bool overflow = false;
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
if ((RLGL.State.vertexData[RLGL.State.currentBuffer].vCounter + vCount) >= (MAX_BATCH_ELEMENTS*4)) overflow = true;
#endif
return overflow;
}


void rlSetDebugMarker(const char *text)
{
#if defined(GRAPHICS_API_OPENGL_33)
if (RLGL.ExtSupported.debugMarker) glInsertEventMarkerEXT(0, text);
#endif
}



void rlLoadExtensions(void *loader)
{
#if defined(GRAPHICS_API_OPENGL_33)

#if !defined(__APPLE__)
if (!gladLoadGLLoader((GLADloadproc)loader)) TRACELOG(LOG_WARNING, "GLAD: Cannot load OpenGL extensions");
else TRACELOG(LOG_INFO, "GLAD: OpenGL extensions loaded successfully");

#if defined(GRAPHICS_API_OPENGL_21)
if (GLAD_GL_VERSION_2_1) TRACELOG(LOG_INFO, "GL: OpenGL 2.1 profile supported");
#elif defined(GRAPHICS_API_OPENGL_33)
if (GLAD_GL_VERSION_3_3) TRACELOG(LOG_INFO, "GL: OpenGL 3.3 Core profile supported");
else TRACELOG(LOG_ERROR, "GL: OpenGL 3.3 Core profile not supported");
#endif
#endif



#endif
}


Vector3 rlUnproject(Vector3 source, Matrix proj, Matrix view)
{
Vector3 result = { 0.0f, 0.0f, 0.0f };


Matrix matViewProj = MatrixMultiply(view, proj);
matViewProj = MatrixInvert(matViewProj);


Quaternion quat = { source.x, source.y, source.z, 1.0f };


quat = QuaternionTransform(quat, matViewProj);


result.x = quat.x/quat.w;
result.y = quat.y/quat.w;
result.z = quat.z/quat.w;

return result;
}


unsigned int rlLoadTexture(void *data, int width, int height, int format, int mipmapCount)
{
glBindTexture(GL_TEXTURE_2D, 0); 

unsigned int id = 0;


#if defined(GRAPHICS_API_OPENGL_11)
if (format >= COMPRESSED_DXT1_RGB)
{
TRACELOG(LOG_WARNING, "GL: OpenGL 1.1 does not support GPU compressed texture formats");
return id;
}
#else
if ((!RLGL.ExtSupported.texCompDXT) && ((format == COMPRESSED_DXT1_RGB) || (format == COMPRESSED_DXT1_RGBA) ||
(format == COMPRESSED_DXT3_RGBA) || (format == COMPRESSED_DXT5_RGBA)))
{
TRACELOG(LOG_WARNING, "GL: DXT compressed texture format not supported");
return id;
}
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
if ((!RLGL.ExtSupported.texCompETC1) && (format == COMPRESSED_ETC1_RGB))
{
TRACELOG(LOG_WARNING, "GL: ETC1 compressed texture format not supported");
return id;
}

if ((!RLGL.ExtSupported.texCompETC2) && ((format == COMPRESSED_ETC2_RGB) || (format == COMPRESSED_ETC2_EAC_RGBA)))
{
TRACELOG(LOG_WARNING, "GL: ETC2 compressed texture format not supported");
return id;
}

if ((!RLGL.ExtSupported.texCompPVRT) && ((format == COMPRESSED_PVRT_RGB) || (format == COMPRESSED_PVRT_RGBA)))
{
TRACELOG(LOG_WARNING, "GL: PVRT compressed texture format not supported");
return id;
}

if ((!RLGL.ExtSupported.texCompASTC) && ((format == COMPRESSED_ASTC_4x4_RGBA) || (format == COMPRESSED_ASTC_8x8_RGBA)))
{
TRACELOG(LOG_WARNING, "GL: ASTC compressed texture format not supported");
return id;
}
#endif
#endif 

glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

glGenTextures(1, &id); 

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)

#endif

glBindTexture(GL_TEXTURE_2D, id);

int mipWidth = width;
int mipHeight = height;
int mipOffset = 0; 

TRACELOGD("TEXTURE: Load texture from data memory address: 0x%x", data);


for (int i = 0; i < mipmapCount; i++)
{
unsigned int mipSize = GetPixelDataSize(mipWidth, mipHeight, format);

unsigned int glInternalFormat, glFormat, glType;
rlGetGlTextureFormats(format, &glInternalFormat, &glFormat, &glType);

TRACELOGD("TEXTURE: Load mipmap level %i (%i x %i), size: %i, offset: %i", i, mipWidth, mipHeight, mipSize, mipOffset);

if (glInternalFormat != -1)
{
if (format < COMPRESSED_DXT1_RGB) glTexImage2D(GL_TEXTURE_2D, i, glInternalFormat, mipWidth, mipHeight, 0, glFormat, glType, (unsigned char *)data + mipOffset);
#if !defined(GRAPHICS_API_OPENGL_11)
else glCompressedTexImage2D(GL_TEXTURE_2D, i, glInternalFormat, mipWidth, mipHeight, 0, mipSize, (unsigned char *)data + mipOffset);
#endif

#if defined(GRAPHICS_API_OPENGL_33)
if (format == UNCOMPRESSED_GRAYSCALE)
{
GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
}
else if (format == UNCOMPRESSED_GRAY_ALPHA)
{
#if defined(GRAPHICS_API_OPENGL_21)
GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ALPHA };
#elif defined(GRAPHICS_API_OPENGL_33)
GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_GREEN };
#endif
glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
}
#endif
}

mipWidth /= 2;
mipHeight /= 2;
mipOffset += mipSize;


if (mipWidth < 1) mipWidth = 1;
if (mipHeight < 1) mipHeight = 1;
}



#if defined(GRAPHICS_API_OPENGL_ES2)

if (RLGL.ExtSupported.texNPOT)
{
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
}
else
{

glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
}
#else
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
#endif


glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 

#if defined(GRAPHICS_API_OPENGL_33)
if (mipmapCount > 1)
{

glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
}
#endif






glBindTexture(GL_TEXTURE_2D, 0);

if (id > 0) TRACELOG(LOG_INFO, "TEXTURE: [ID %i] Texture created successfully (%ix%i - %i mipmaps)", id, width, height, mipmapCount);
else TRACELOG(LOG_WARNING, "TEXTURE: Failed to load texture");

return id;
}



unsigned int rlLoadTextureDepth(int width, int height, int bits, bool useRenderBuffer)
{
unsigned int id = 0;

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
unsigned int glInternalFormat = GL_DEPTH_COMPONENT16;

if ((bits != 16) && (bits != 24) && (bits != 32)) bits = 16;

if (bits == 24)
{
#if defined(GRAPHICS_API_OPENGL_33)
glInternalFormat = GL_DEPTH_COMPONENT24;
#elif defined(GRAPHICS_API_OPENGL_ES2)
if (RLGL.ExtSupported.maxDepthBits >= 24) glInternalFormat = GL_DEPTH_COMPONENT24_OES;
#endif
}

if (bits == 32)
{
#if defined(GRAPHICS_API_OPENGL_33)
glInternalFormat = GL_DEPTH_COMPONENT32;
#elif defined(GRAPHICS_API_OPENGL_ES2)
if (RLGL.ExtSupported.maxDepthBits == 32) glInternalFormat = GL_DEPTH_COMPONENT32_OES;
#endif
}

if (!useRenderBuffer && RLGL.ExtSupported.texDepth)
{
glGenTextures(1, &id);
glBindTexture(GL_TEXTURE_2D, id);
glTexImage2D(GL_TEXTURE_2D, 0, glInternalFormat, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);

glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

glBindTexture(GL_TEXTURE_2D, 0);
}
else
{


glGenRenderbuffers(1, &id);
glBindRenderbuffer(GL_RENDERBUFFER, id);
glRenderbufferStorage(GL_RENDERBUFFER, glInternalFormat, width, height);

glBindRenderbuffer(GL_RENDERBUFFER, 0);
}
#endif

return id;
}




unsigned int rlLoadTextureCubemap(void *data, int size, int format)
{
unsigned int cubemapId = 0;

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
unsigned int dataSize = GetPixelDataSize(size, size, format);

glGenTextures(1, &cubemapId);
glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapId);

unsigned int glInternalFormat, glFormat, glType;
rlGetGlTextureFormats(format, &glInternalFormat, &glFormat, &glType);

if (glInternalFormat != -1)
{

for (unsigned int i = 0; i < 6; i++)
{
if (format < COMPRESSED_DXT1_RGB) glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, glInternalFormat, size, size, 0, glFormat, glType, (unsigned char *)data + i*dataSize);
else glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, glInternalFormat, size, size, 0, dataSize, (unsigned char *)data + i*dataSize);

#if defined(GRAPHICS_API_OPENGL_33)
if (format == UNCOMPRESSED_GRAYSCALE)
{
GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
glTexParameteriv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
}
else if (format == UNCOMPRESSED_GRAY_ALPHA)
{
#if defined(GRAPHICS_API_OPENGL_21)
GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ALPHA };
#elif defined(GRAPHICS_API_OPENGL_33)
GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_GREEN };
#endif
glTexParameteriv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
}
#endif
}
}


glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#if defined(GRAPHICS_API_OPENGL_33)
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); 
#endif

glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
#endif

return cubemapId;
}



void rlUpdateTexture(unsigned int id, int width, int height, int format, const void *data)
{
glBindTexture(GL_TEXTURE_2D, id);

unsigned int glInternalFormat, glFormat, glType;
rlGetGlTextureFormats(format, &glInternalFormat, &glFormat, &glType);

if ((glInternalFormat != -1) && (format < COMPRESSED_DXT1_RGB))
{
glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, glFormat, glType, (unsigned char *)data);
}
else TRACELOG(LOG_WARNING, "TEXTURE: [ID %i] Failed to update for current texture format (%i)", id, format);
}


void rlGetGlTextureFormats(int format, unsigned int *glInternalFormat, unsigned int *glFormat, unsigned int *glType)
{
*glInternalFormat = -1;
*glFormat = -1;
*glType = -1;

switch (format)
{
#if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_21) || defined(GRAPHICS_API_OPENGL_ES2)

case UNCOMPRESSED_GRAYSCALE: *glInternalFormat = GL_LUMINANCE; *glFormat = GL_LUMINANCE; *glType = GL_UNSIGNED_BYTE; break;
case UNCOMPRESSED_GRAY_ALPHA: *glInternalFormat = GL_LUMINANCE_ALPHA; *glFormat = GL_LUMINANCE_ALPHA; *glType = GL_UNSIGNED_BYTE; break;
case UNCOMPRESSED_R5G6B5: *glInternalFormat = GL_RGB; *glFormat = GL_RGB; *glType = GL_UNSIGNED_SHORT_5_6_5; break;
case UNCOMPRESSED_R8G8B8: *glInternalFormat = GL_RGB; *glFormat = GL_RGB; *glType = GL_UNSIGNED_BYTE; break;
case UNCOMPRESSED_R5G5B5A1: *glInternalFormat = GL_RGBA; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_SHORT_5_5_5_1; break;
case UNCOMPRESSED_R4G4B4A4: *glInternalFormat = GL_RGBA; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_SHORT_4_4_4_4; break;
case UNCOMPRESSED_R8G8B8A8: *glInternalFormat = GL_RGBA; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_BYTE; break;
#if !defined(GRAPHICS_API_OPENGL_11)
case UNCOMPRESSED_R32: if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_LUMINANCE; *glFormat = GL_LUMINANCE; *glType = GL_FLOAT; break; 
case UNCOMPRESSED_R32G32B32: if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_RGB; *glFormat = GL_RGB; *glType = GL_FLOAT; break; 
case UNCOMPRESSED_R32G32B32A32: if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_RGBA; *glFormat = GL_RGBA; *glType = GL_FLOAT; break; 
#endif
#elif defined(GRAPHICS_API_OPENGL_33)
case UNCOMPRESSED_GRAYSCALE: *glInternalFormat = GL_R8; *glFormat = GL_RED; *glType = GL_UNSIGNED_BYTE; break;
case UNCOMPRESSED_GRAY_ALPHA: *glInternalFormat = GL_RG8; *glFormat = GL_RG; *glType = GL_UNSIGNED_BYTE; break;
case UNCOMPRESSED_R5G6B5: *glInternalFormat = GL_RGB565; *glFormat = GL_RGB; *glType = GL_UNSIGNED_SHORT_5_6_5; break;
case UNCOMPRESSED_R8G8B8: *glInternalFormat = GL_RGB8; *glFormat = GL_RGB; *glType = GL_UNSIGNED_BYTE; break;
case UNCOMPRESSED_R5G5B5A1: *glInternalFormat = GL_RGB5_A1; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_SHORT_5_5_5_1; break;
case UNCOMPRESSED_R4G4B4A4: *glInternalFormat = GL_RGBA4; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_SHORT_4_4_4_4; break;
case UNCOMPRESSED_R8G8B8A8: *glInternalFormat = GL_RGBA8; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_BYTE; break;
case UNCOMPRESSED_R32: if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_R32F; *glFormat = GL_RED; *glType = GL_FLOAT; break;
case UNCOMPRESSED_R32G32B32: if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_RGB32F; *glFormat = GL_RGB; *glType = GL_FLOAT; break;
case UNCOMPRESSED_R32G32B32A32: if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_RGBA32F; *glFormat = GL_RGBA; *glType = GL_FLOAT; break;
#endif
#if !defined(GRAPHICS_API_OPENGL_11)
case COMPRESSED_DXT1_RGB: if (RLGL.ExtSupported.texCompDXT) *glInternalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT; break;
case COMPRESSED_DXT1_RGBA: if (RLGL.ExtSupported.texCompDXT) *glInternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; break;
case COMPRESSED_DXT3_RGBA: if (RLGL.ExtSupported.texCompDXT) *glInternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT; break;
case COMPRESSED_DXT5_RGBA: if (RLGL.ExtSupported.texCompDXT) *glInternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; break;
case COMPRESSED_ETC1_RGB: if (RLGL.ExtSupported.texCompETC1) *glInternalFormat = GL_ETC1_RGB8_OES; break; 
case COMPRESSED_ETC2_RGB: if (RLGL.ExtSupported.texCompETC2) *glInternalFormat = GL_COMPRESSED_RGB8_ETC2; break; 
case COMPRESSED_ETC2_EAC_RGBA: if (RLGL.ExtSupported.texCompETC2) *glInternalFormat = GL_COMPRESSED_RGBA8_ETC2_EAC; break; 
case COMPRESSED_PVRT_RGB: if (RLGL.ExtSupported.texCompPVRT) *glInternalFormat = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG; break; 
case COMPRESSED_PVRT_RGBA: if (RLGL.ExtSupported.texCompPVRT) *glInternalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG; break; 
case COMPRESSED_ASTC_4x4_RGBA: if (RLGL.ExtSupported.texCompASTC) *glInternalFormat = GL_COMPRESSED_RGBA_ASTC_4x4_KHR; break; 
case COMPRESSED_ASTC_8x8_RGBA: if (RLGL.ExtSupported.texCompASTC) *glInternalFormat = GL_COMPRESSED_RGBA_ASTC_8x8_KHR; break; 
#endif
default: TRACELOG(LOG_WARNING, "TEXTURE: Current format not supported (%i)", format); break;
}
}


void rlUnloadTexture(unsigned int id)
{
if (id > 0) glDeleteTextures(1, &id);
}



RenderTexture2D rlLoadRenderTexture(int width, int height, int format, int depthBits, bool useDepthTexture)
{
RenderTexture2D target = { 0 };

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
if (useDepthTexture && RLGL.ExtSupported.texDepth) target.depthTexture = true;


glGenFramebuffers(1, &target.id);
glBindFramebuffer(GL_FRAMEBUFFER, target.id);



if ((format != -1) && (format < COMPRESSED_DXT1_RGB))
{

target.texture.id = rlLoadTexture(NULL, width, height, format, 1);
target.texture.width = width;
target.texture.height = height;
target.texture.format = format;
target.texture.mipmaps = 1;
}




if (depthBits > 0)
{
target.depth.id = rlLoadTextureDepth(width, height, depthBits, !useDepthTexture);
target.depth.width = width;
target.depth.height = height;
target.depth.format = 19; 
target.depth.mipmaps = 1;
}




rlRenderTextureAttach(target, target.texture.id, 0); 
rlRenderTextureAttach(target, target.depth.id, 1); 




if (rlRenderTextureComplete(target)) TRACELOG(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully", target.id);


glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif

return target;
}



void rlRenderTextureAttach(RenderTexture2D target, unsigned int id, int attachType)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
glBindFramebuffer(GL_FRAMEBUFFER, target.id);

if (attachType == 0) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, id, 0);
else if (attachType == 1)
{
if (target.depthTexture) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, id, 0);
else glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, id);
}

glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
}


bool rlRenderTextureComplete(RenderTexture target)
{
bool result = false;

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
glBindFramebuffer(GL_FRAMEBUFFER, target.id);

GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

if (status != GL_FRAMEBUFFER_COMPLETE)
{
switch (status)
{
case GL_FRAMEBUFFER_UNSUPPORTED: TRACELOG(LOG_WARNING, "FBO: [ID %i] Framebuffer is unsupported", target.id); break;
case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: TRACELOG(LOG_WARNING, "FBO: [ID %i] Framebuffer has incomplete attachment", target.id); break;
#if defined(GRAPHICS_API_OPENGL_ES2)
case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS: TRACELOG(LOG_WARNING, "FBO: [ID %i] Framebuffer has incomplete dimensions", target.id); break;
#endif
case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: TRACELOG(LOG_WARNING, "FBO: [ID %i] Framebuffer has a missing attachment", target.id); break;
default: break;
}
}

glBindFramebuffer(GL_FRAMEBUFFER, 0);

result = (status == GL_FRAMEBUFFER_COMPLETE);
#endif

return result;
}


void rlGenerateMipmaps(Texture2D *texture)
{
glBindTexture(GL_TEXTURE_2D, texture->id);


bool texIsPOT = false;

if (((texture->width > 0) && ((texture->width & (texture->width - 1)) == 0)) &&
((texture->height > 0) && ((texture->height & (texture->height - 1)) == 0))) texIsPOT = true;

#if defined(GRAPHICS_API_OPENGL_11)
if (texIsPOT)
{

if (texture->format == UNCOMPRESSED_R8G8B8A8)
{

void *data = rlReadTexturePixels(*texture);



int mipmapCount = GenerateMipmaps(data, texture->width, texture->height);

int size = texture->width*texture->height*4;
int offset = size;

int mipWidth = texture->width/2;
int mipHeight = texture->height/2;


for (int level = 1; level < mipmapCount; level++)
{
glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA8, mipWidth, mipHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char *)data + offset);

size = mipWidth*mipHeight*4;
offset += size;

mipWidth /= 2;
mipHeight /= 2;
}

texture->mipmaps = mipmapCount + 1;
RL_FREE(data); 

TRACELOG(LOG_WARNING, "TEXTURE: [ID %i] Mipmaps generated manually on CPU side, total: %i", texture->id, texture->mipmaps);
}
else TRACELOG(LOG_WARNING, "TEXTURE: [ID %i] Failed to generate mipmaps for provided texture format", texture->id);
}
#elif defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
if ((texIsPOT) || (RLGL.ExtSupported.texNPOT))
{

glGenerateMipmap(GL_TEXTURE_2D); 

glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

texture->mipmaps = 1 + (int)floor(log(MAX(texture->width, texture->height))/log(2));
TRACELOG(LOG_INFO, "TEXTURE: [ID %i] Mipmaps generated automatically, total: %i", texture->id, texture->mipmaps);
}
#endif
else TRACELOG(LOG_WARNING, "TEXTURE: [ID %i] Failed to generate mipmaps", texture->id);

glBindTexture(GL_TEXTURE_2D, 0);
}


void rlLoadMesh(Mesh *mesh, bool dynamic)
{
if (mesh->vaoId > 0)
{

TRACELOG(LOG_WARNING, "VAO: [ID %i] Trying to re-load an already loaded mesh", mesh->vaoId);
return;
}

mesh->vaoId = 0; 
mesh->vboId[0] = 0; 
mesh->vboId[1] = 0; 
mesh->vboId[2] = 0; 
mesh->vboId[3] = 0; 
mesh->vboId[4] = 0; 
mesh->vboId[5] = 0; 
mesh->vboId[6] = 0; 

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
int drawHint = GL_STATIC_DRAW;
if (dynamic) drawHint = GL_DYNAMIC_DRAW;

if (RLGL.ExtSupported.vao)
{

glGenVertexArrays(1, &mesh->vaoId);
glBindVertexArray(mesh->vaoId);
}




glGenBuffers(1, &mesh->vboId[0]);
glBindBuffer(GL_ARRAY_BUFFER, mesh->vboId[0]);
glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*mesh->vertexCount, mesh->vertices, drawHint);
glVertexAttribPointer(0, 3, GL_FLOAT, 0, 0, 0);
glEnableVertexAttribArray(0);


glGenBuffers(1, &mesh->vboId[1]);
glBindBuffer(GL_ARRAY_BUFFER, mesh->vboId[1]);
glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*mesh->vertexCount, mesh->texcoords, drawHint);
glVertexAttribPointer(1, 2, GL_FLOAT, 0, 0, 0);
glEnableVertexAttribArray(1);


if (mesh->normals != NULL)
{
glGenBuffers(1, &mesh->vboId[2]);
glBindBuffer(GL_ARRAY_BUFFER, mesh->vboId[2]);
glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*mesh->vertexCount, mesh->normals, drawHint);
glVertexAttribPointer(2, 3, GL_FLOAT, 0, 0, 0);
glEnableVertexAttribArray(2);
}
else
{

glVertexAttrib3f(2, 1.0f, 1.0f, 1.0f);
glDisableVertexAttribArray(2);
}


if (mesh->colors != NULL)
{
glGenBuffers(1, &mesh->vboId[3]);
glBindBuffer(GL_ARRAY_BUFFER, mesh->vboId[3]);
glBufferData(GL_ARRAY_BUFFER, sizeof(unsigned char)*4*mesh->vertexCount, mesh->colors, drawHint);
glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);
glEnableVertexAttribArray(3);
}
else
{

glVertexAttrib4f(3, 1.0f, 1.0f, 1.0f, 1.0f);
glDisableVertexAttribArray(3);
}


if (mesh->tangents != NULL)
{
glGenBuffers(1, &mesh->vboId[4]);
glBindBuffer(GL_ARRAY_BUFFER, mesh->vboId[4]);
glBufferData(GL_ARRAY_BUFFER, sizeof(float)*4*mesh->vertexCount, mesh->tangents, drawHint);
glVertexAttribPointer(4, 4, GL_FLOAT, 0, 0, 0);
glEnableVertexAttribArray(4);
}
else
{

glVertexAttrib4f(4, 0.0f, 0.0f, 0.0f, 0.0f);
glDisableVertexAttribArray(4);
}


if (mesh->texcoords2 != NULL)
{
glGenBuffers(1, &mesh->vboId[5]);
glBindBuffer(GL_ARRAY_BUFFER, mesh->vboId[5]);
glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*mesh->vertexCount, mesh->texcoords2, drawHint);
glVertexAttribPointer(5, 2, GL_FLOAT, 0, 0, 0);
glEnableVertexAttribArray(5);
}
else
{

glVertexAttrib2f(5, 0.0f, 0.0f);
glDisableVertexAttribArray(5);
}

if (mesh->indices != NULL)
{
glGenBuffers(1, &mesh->vboId[6]);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->vboId[6]);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short)*mesh->triangleCount*3, mesh->indices, drawHint);
}

if (RLGL.ExtSupported.vao)
{
if (mesh->vaoId > 0) TRACELOG(LOG_INFO, "VAO: [ID %i] Mesh uploaded successfully to VRAM (GPU)", mesh->vaoId);
else TRACELOG(LOG_WARNING, "VAO: Failed to load mesh to VRAM (GPU)");
}
else
{
TRACELOG(LOG_INFO, "VBO: Mesh uploaded successfully to VRAM (GPU)");
}
#endif
}


unsigned int rlLoadAttribBuffer(unsigned int vaoId, int shaderLoc, void *buffer, int size, bool dynamic)
{
unsigned int id = 0;

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
int drawHint = GL_STATIC_DRAW;
if (dynamic) drawHint = GL_DYNAMIC_DRAW;

if (RLGL.ExtSupported.vao) glBindVertexArray(vaoId);

glGenBuffers(1, &id);
glBindBuffer(GL_ARRAY_BUFFER, id);
glBufferData(GL_ARRAY_BUFFER, size, buffer, drawHint);
glVertexAttribPointer(shaderLoc, 2, GL_FLOAT, 0, 0, 0);
glEnableVertexAttribArray(shaderLoc);

if (RLGL.ExtSupported.vao) glBindVertexArray(0);
#endif

return id;
}


void rlUpdateMesh(Mesh mesh, int buffer, int num)
{
rlUpdateMeshAt(mesh, buffer, num, 0);
}




void rlUpdateMeshAt(Mesh mesh, int buffer, int num, int index)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)

if (RLGL.ExtSupported.vao) glBindVertexArray(mesh.vaoId);

switch (buffer)
{
case 0: 
{
glBindBuffer(GL_ARRAY_BUFFER, mesh.vboId[0]);
if (index == 0 && num >= mesh.vertexCount) glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*num, mesh.vertices, GL_DYNAMIC_DRAW);
else if (index + num >= mesh.vertexCount) break;
else glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*3*index, sizeof(float)*3*num, mesh.vertices);

} break;
case 1: 
{
glBindBuffer(GL_ARRAY_BUFFER, mesh.vboId[1]);
if (index == 0 && num >= mesh.vertexCount) glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*num, mesh.texcoords, GL_DYNAMIC_DRAW);
else if (index + num >= mesh.vertexCount) break;
else glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*2*index, sizeof(float)*2*num, mesh.texcoords);

} break;
case 2: 
{
glBindBuffer(GL_ARRAY_BUFFER, mesh.vboId[2]);
if (index == 0 && num >= mesh.vertexCount) glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*num, mesh.normals, GL_DYNAMIC_DRAW);
else if (index + num >= mesh.vertexCount) break;
else glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*3*index, sizeof(float)*3*num, mesh.normals);

} break;
case 3: 
{
glBindBuffer(GL_ARRAY_BUFFER, mesh.vboId[3]);
if (index == 0 && num >= mesh.vertexCount) glBufferData(GL_ARRAY_BUFFER, sizeof(float)*4*num, mesh.colors, GL_DYNAMIC_DRAW);
else if (index + num >= mesh.vertexCount) break;
else glBufferSubData(GL_ARRAY_BUFFER, sizeof(unsigned char)*4*index, sizeof(unsigned char)*4*num, mesh.colors);

} break;
case 4: 
{
glBindBuffer(GL_ARRAY_BUFFER, mesh.vboId[4]);
if (index == 0 && num >= mesh.vertexCount) glBufferData(GL_ARRAY_BUFFER, sizeof(float)*4*num, mesh.tangents, GL_DYNAMIC_DRAW);
else if (index + num >= mesh.vertexCount) break;
else glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*4*index, sizeof(float)*4*num, mesh.tangents);
} break;
case 5: 
{
glBindBuffer(GL_ARRAY_BUFFER, mesh.vboId[5]);
if (index == 0 && num >= mesh.vertexCount) glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*num, mesh.texcoords2, GL_DYNAMIC_DRAW);
else if (index + num >= mesh.vertexCount) break;
else glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*2*index, sizeof(float)*2*num, mesh.texcoords2);
} break;
case 6: 
{

unsigned short *indices = mesh.indices;
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vboId[6]);
if (index == 0 && num >= mesh.triangleCount)
glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*indices)*num*3, indices, GL_DYNAMIC_DRAW);
else if (index + num >= mesh.triangleCount)
break;
else
glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*indices)*index*3, sizeof(*indices)*num*3, indices);
} break;
default: break;
}


if (RLGL.ExtSupported.vao) glBindVertexArray(0);





#endif
}


void rlDrawMesh(Mesh mesh, Material material, Matrix transform)
{
#if defined(GRAPHICS_API_OPENGL_11)
glEnable(GL_TEXTURE_2D);
glBindTexture(GL_TEXTURE_2D, material.maps[MAP_DIFFUSE].texture.id);


glEnableClientState(GL_VERTEX_ARRAY); 
glEnableClientState(GL_TEXTURE_COORD_ARRAY); 
if (mesh.normals != NULL) glEnableClientState(GL_NORMAL_ARRAY); 
if (mesh.colors != NULL) glEnableClientState(GL_COLOR_ARRAY); 

glVertexPointer(3, GL_FLOAT, 0, mesh.vertices); 
glTexCoordPointer(2, GL_FLOAT, 0, mesh.texcoords); 
if (mesh.normals != NULL) glNormalPointer(GL_FLOAT, 0, mesh.normals); 
if (mesh.colors != NULL) glColorPointer(4, GL_UNSIGNED_BYTE, 0, mesh.colors); 

rlPushMatrix();
rlMultMatrixf(MatrixToFloat(transform));
rlColor4ub(material.maps[MAP_DIFFUSE].color.r, material.maps[MAP_DIFFUSE].color.g, material.maps[MAP_DIFFUSE].color.b, material.maps[MAP_DIFFUSE].color.a);

if (mesh.indices != NULL) glDrawElements(GL_TRIANGLES, mesh.triangleCount*3, GL_UNSIGNED_SHORT, mesh.indices);
else glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);
rlPopMatrix();

glDisableClientState(GL_VERTEX_ARRAY); 
glDisableClientState(GL_TEXTURE_COORD_ARRAY); 
if (mesh.normals != NULL) glDisableClientState(GL_NORMAL_ARRAY); 
if (mesh.colors != NULL) glDisableClientState(GL_NORMAL_ARRAY); 

glDisable(GL_TEXTURE_2D);
glBindTexture(GL_TEXTURE_2D, 0);
#endif

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)

glUseProgram(material.shader.id);




if (material.shader.locs[LOC_MATRIX_MODEL] != -1) SetShaderValueMatrix(material.shader, material.shader.locs[LOC_MATRIX_MODEL], transform);


if (material.shader.locs[LOC_COLOR_DIFFUSE] != -1)
glUniform4f(material.shader.locs[LOC_COLOR_DIFFUSE], (float)material.maps[MAP_DIFFUSE].color.r/255.0f,
(float)material.maps[MAP_DIFFUSE].color.g/255.0f,
(float)material.maps[MAP_DIFFUSE].color.b/255.0f,
(float)material.maps[MAP_DIFFUSE].color.a/255.0f);


if (material.shader.locs[LOC_COLOR_SPECULAR] != -1)
glUniform4f(material.shader.locs[LOC_COLOR_SPECULAR], (float)material.maps[MAP_SPECULAR].color.r/255.0f,
(float)material.maps[MAP_SPECULAR].color.g/255.0f,
(float)material.maps[MAP_SPECULAR].color.b/255.0f,
(float)material.maps[MAP_SPECULAR].color.a/255.0f);

if (material.shader.locs[LOC_MATRIX_VIEW] != -1) SetShaderValueMatrix(material.shader, material.shader.locs[LOC_MATRIX_VIEW], RLGL.State.modelview);
if (material.shader.locs[LOC_MATRIX_PROJECTION] != -1) SetShaderValueMatrix(material.shader, material.shader.locs[LOC_MATRIX_PROJECTION], RLGL.State.projection);



Matrix matView = RLGL.State.modelview; 
Matrix matProjection = RLGL.State.projection; 







Matrix matModelView = MatrixMultiply(transform, MatrixMultiply(RLGL.State.transform, matView));



for (int i = 0; i < MAX_MATERIAL_MAPS; i++)
{
if (material.maps[i].texture.id > 0)
{
glActiveTexture(GL_TEXTURE0 + i);
if ((i == MAP_IRRADIANCE) || (i == MAP_PREFILTER) || (i == MAP_CUBEMAP)) glBindTexture(GL_TEXTURE_CUBE_MAP, material.maps[i].texture.id);
else glBindTexture(GL_TEXTURE_2D, material.maps[i].texture.id);

glUniform1i(material.shader.locs[LOC_MAP_DIFFUSE + i], i);
}
}


if (RLGL.ExtSupported.vao) glBindVertexArray(mesh.vaoId);
else
{

glBindBuffer(GL_ARRAY_BUFFER, mesh.vboId[0]);
glVertexAttribPointer(material.shader.locs[LOC_VERTEX_POSITION], 3, GL_FLOAT, 0, 0, 0);
glEnableVertexAttribArray(material.shader.locs[LOC_VERTEX_POSITION]);


glBindBuffer(GL_ARRAY_BUFFER, mesh.vboId[1]);
glVertexAttribPointer(material.shader.locs[LOC_VERTEX_TEXCOORD01], 2, GL_FLOAT, 0, 0, 0);
glEnableVertexAttribArray(material.shader.locs[LOC_VERTEX_TEXCOORD01]);


if (material.shader.locs[LOC_VERTEX_NORMAL] != -1)
{
glBindBuffer(GL_ARRAY_BUFFER, mesh.vboId[2]);
glVertexAttribPointer(material.shader.locs[LOC_VERTEX_NORMAL], 3, GL_FLOAT, 0, 0, 0);
glEnableVertexAttribArray(material.shader.locs[LOC_VERTEX_NORMAL]);
}


if (material.shader.locs[LOC_VERTEX_COLOR] != -1)
{
if (mesh.vboId[3] != 0)
{
glBindBuffer(GL_ARRAY_BUFFER, mesh.vboId[3]);
glVertexAttribPointer(material.shader.locs[LOC_VERTEX_COLOR], 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);
glEnableVertexAttribArray(material.shader.locs[LOC_VERTEX_COLOR]);
}
else
{


glVertexAttrib4f(material.shader.locs[LOC_VERTEX_COLOR], 1.0f, 1.0f, 1.0f, 1.0f);
glDisableVertexAttribArray(material.shader.locs[LOC_VERTEX_COLOR]);
}
}


if (material.shader.locs[LOC_VERTEX_TANGENT] != -1)
{
glBindBuffer(GL_ARRAY_BUFFER, mesh.vboId[4]);
glVertexAttribPointer(material.shader.locs[LOC_VERTEX_TANGENT], 4, GL_FLOAT, 0, 0, 0);
glEnableVertexAttribArray(material.shader.locs[LOC_VERTEX_TANGENT]);
}


if (material.shader.locs[LOC_VERTEX_TEXCOORD02] != -1)
{
glBindBuffer(GL_ARRAY_BUFFER, mesh.vboId[5]);
glVertexAttribPointer(material.shader.locs[LOC_VERTEX_TEXCOORD02], 2, GL_FLOAT, 0, 0, 0);
glEnableVertexAttribArray(material.shader.locs[LOC_VERTEX_TEXCOORD02]);
}

if (mesh.indices != NULL) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vboId[6]);
}

int eyesCount = 1;
#if defined(SUPPORT_VR_SIMULATOR)
if (RLGL.Vr.stereoRender) eyesCount = 2;
#endif

for (int eye = 0; eye < eyesCount; eye++)
{
if (eyesCount == 1) RLGL.State.modelview = matModelView;
#if defined(SUPPORT_VR_SIMULATOR)
else SetStereoView(eye, matProjection, matModelView);
#endif


Matrix matMVP = MatrixMultiply(RLGL.State.modelview, RLGL.State.projection); 


glUniformMatrix4fv(material.shader.locs[LOC_MATRIX_MVP], 1, false, MatrixToFloat(matMVP));


if (mesh.indices != NULL) glDrawElements(GL_TRIANGLES, mesh.triangleCount*3, GL_UNSIGNED_SHORT, 0); 
else glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);
}


for (int i = 0; i < MAX_MATERIAL_MAPS; i++)
{
glActiveTexture(GL_TEXTURE0 + i); 
if ((i == MAP_IRRADIANCE) || (i == MAP_PREFILTER) || (i == MAP_CUBEMAP)) glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
else glBindTexture(GL_TEXTURE_2D, 0); 
}


if (RLGL.ExtSupported.vao) glBindVertexArray(0);
else
{
glBindBuffer(GL_ARRAY_BUFFER, 0);
if (mesh.indices != NULL) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


glUseProgram(0);



RLGL.State.projection = matProjection;
RLGL.State.modelview = matView;
#endif
}


void rlUnloadMesh(Mesh mesh)
{
RL_FREE(mesh.vertices);
RL_FREE(mesh.texcoords);
RL_FREE(mesh.normals);
RL_FREE(mesh.colors);
RL_FREE(mesh.tangents);
RL_FREE(mesh.texcoords2);
RL_FREE(mesh.indices);

RL_FREE(mesh.animVertices);
RL_FREE(mesh.animNormals);
RL_FREE(mesh.boneWeights);
RL_FREE(mesh.boneIds);

rlDeleteBuffers(mesh.vboId[0]); 
rlDeleteBuffers(mesh.vboId[1]); 
rlDeleteBuffers(mesh.vboId[2]); 
rlDeleteBuffers(mesh.vboId[3]); 
rlDeleteBuffers(mesh.vboId[4]); 
rlDeleteBuffers(mesh.vboId[5]); 
rlDeleteBuffers(mesh.vboId[6]); 

rlDeleteVertexArrays(mesh.vaoId);
}


unsigned char *rlReadScreenPixels(int width, int height)
{
unsigned char *screenData = (unsigned char *)RL_CALLOC(width*height*4, sizeof(unsigned char));



glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, screenData);


unsigned char *imgData = (unsigned char *)RL_MALLOC(width*height*sizeof(unsigned char)*4);

for (int y = height - 1; y >= 0; y--)
{
for (int x = 0; x < (width*4); x++)
{
imgData[((height - 1) - y)*width*4 + x] = screenData[(y*width*4) + x]; 



if (((x + 1)%4) == 0) imgData[((height - 1) - y)*width*4 + x] = 255;
}
}

RL_FREE(screenData);

return imgData; 
}


void *rlReadTexturePixels(Texture2D texture)
{
void *pixels = NULL;

#if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)
glBindTexture(GL_TEXTURE_2D, texture.id);












glPixelStorei(GL_PACK_ALIGNMENT, 1);

unsigned int glInternalFormat, glFormat, glType;
rlGetGlTextureFormats(texture.format, &glInternalFormat, &glFormat, &glType);
unsigned int size = GetPixelDataSize(texture.width, texture.height, texture.format);

if ((glInternalFormat != -1) && (texture.format < COMPRESSED_DXT1_RGB))
{
pixels = (unsigned char *)RL_MALLOC(size);
glGetTexImage(GL_TEXTURE_2D, 0, glFormat, glType, pixels);
}
else TRACELOG(LOG_WARNING, "TEXTURE: [ID %i] Data retrieval not suported for pixel format (%i)", texture.id, texture.format);

glBindTexture(GL_TEXTURE_2D, 0);
#endif

#if defined(GRAPHICS_API_OPENGL_ES2)







RenderTexture2D fbo = rlLoadRenderTexture(texture.width, texture.height, UNCOMPRESSED_R8G8B8A8, 16, false);

glBindFramebuffer(GL_FRAMEBUFFER, fbo.id);
glBindTexture(GL_TEXTURE_2D, 0);



glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.id, 0);


pixels = (unsigned char *)RL_MALLOC(GetPixelDataSize(texture.width, texture.height, UNCOMPRESSED_R8G8B8A8));
glReadPixels(0, 0, texture.width, texture.height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);


glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo.texture.id, 0);

glBindFramebuffer(GL_FRAMEBUFFER, 0);


rlDeleteRenderTextures(fbo);
#endif

return pixels;
}







Texture2D GetTextureDefault(void)
{
Texture2D texture = { 0 };
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
texture.id = RLGL.State.defaultTextureId;
texture.width = 1;
texture.height = 1;
texture.mipmaps = 1;
texture.format = UNCOMPRESSED_R8G8B8A8;
#endif
return texture;
}


Texture2D GetShapesTexture(void)
{
return RLGL.State.shapesTexture;
}


Rectangle GetShapesTextureRec(void)
{
return RLGL.State.shapesTextureRec;
}


void SetShapesTexture(Texture2D texture, Rectangle source)
{
RLGL.State.shapesTexture = texture;
RLGL.State.shapesTextureRec = source;
}


Shader GetShaderDefault(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
return RLGL.State.defaultShader;
#else
Shader shader = { 0 };
return shader;
#endif
}



Shader LoadShader(const char *vsFileName, const char *fsFileName)
{
Shader shader = { 0 };



char *vShaderStr = NULL;
char *fShaderStr = NULL;

if (vsFileName != NULL) vShaderStr = LoadFileText(vsFileName);
if (fsFileName != NULL) fShaderStr = LoadFileText(fsFileName);

shader = LoadShaderCode(vShaderStr, fShaderStr);

if (vShaderStr != NULL) RL_FREE(vShaderStr);
if (fShaderStr != NULL) RL_FREE(fShaderStr);

return shader;
}



Shader LoadShaderCode(const char *vsCode, const char *fsCode)
{
Shader shader = { 0 };
shader.locs = (int *)RL_CALLOC(MAX_SHADER_LOCATIONS, sizeof(int));


for (int i = 0; i < MAX_SHADER_LOCATIONS; i++) shader.locs[i] = -1;

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
unsigned int vertexShaderId = RLGL.State.defaultVShaderId;
unsigned int fragmentShaderId = RLGL.State.defaultFShaderId;

if (vsCode != NULL) vertexShaderId = CompileShader(vsCode, GL_VERTEX_SHADER);
if (fsCode != NULL) fragmentShaderId = CompileShader(fsCode, GL_FRAGMENT_SHADER);

if ((vertexShaderId == RLGL.State.defaultVShaderId) && (fragmentShaderId == RLGL.State.defaultFShaderId)) shader = RLGL.State.defaultShader;
else
{
shader.id = LoadShaderProgram(vertexShaderId, fragmentShaderId);

if (vertexShaderId != RLGL.State.defaultVShaderId) glDeleteShader(vertexShaderId);
if (fragmentShaderId != RLGL.State.defaultFShaderId) glDeleteShader(fragmentShaderId);

if (shader.id == 0)
{
TRACELOG(LOG_WARNING, "SHADER: Failed to load custom shader code");
shader = RLGL.State.defaultShader;
}


if (shader.id > 0) SetShaderDefaultLocations(&shader);
}



int uniformCount = -1;

glGetProgramiv(shader.id, GL_ACTIVE_UNIFORMS, &uniformCount);

for (int i = 0; i < uniformCount; i++)
{
int namelen = -1;
int num = -1;
char name[256]; 
GLenum type = GL_ZERO;


glGetActiveUniform(shader.id, i,sizeof(name) - 1, &namelen, &num, &type, name);

name[namelen] = 0;

TRACELOGD("SHADER: [ID %i] Active uniform (%s) set at location: %i", shader.id, name, glGetUniformLocation(shader.id, name));
}
#endif

return shader;
}


void UnloadShader(Shader shader)
{
if (shader.id > 0)
{
rlDeleteShader(shader.id);
TRACELOG(LOG_INFO, "SHADER: [ID %i] Unloaded shader program data from VRAM (GPU)", shader.id);
}

RL_FREE(shader.locs);
}


void BeginShaderMode(Shader shader)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
if (RLGL.State.currentShader.id != shader.id)
{
rlglDraw();
RLGL.State.currentShader = shader;
}
#endif
}


void EndShaderMode(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
BeginShaderMode(RLGL.State.defaultShader);
#endif
}


int GetShaderLocation(Shader shader, const char *uniformName)
{
int location = -1;
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
location = glGetUniformLocation(shader.id, uniformName);

if (location == -1) TRACELOG(LOG_WARNING, "SHADER: [ID %i] Failed to find shader uniform: %s", shader.id, uniformName);
else TRACELOG(LOG_INFO, "SHADER: [ID %i] Shader uniform (%s) set at location: %i", shader.id, uniformName, location);
#endif
return location;
}


void SetShaderValue(Shader shader, int uniformLoc, const void *value, int uniformType)
{
SetShaderValueV(shader, uniformLoc, value, uniformType, 1);
}


void SetShaderValueV(Shader shader, int uniformLoc, const void *value, int uniformType, int count)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
glUseProgram(shader.id);

switch (uniformType)
{
case UNIFORM_FLOAT: glUniform1fv(uniformLoc, count, (float *)value); break;
case UNIFORM_VEC2: glUniform2fv(uniformLoc, count, (float *)value); break;
case UNIFORM_VEC3: glUniform3fv(uniformLoc, count, (float *)value); break;
case UNIFORM_VEC4: glUniform4fv(uniformLoc, count, (float *)value); break;
case UNIFORM_INT: glUniform1iv(uniformLoc, count, (int *)value); break;
case UNIFORM_IVEC2: glUniform2iv(uniformLoc, count, (int *)value); break;
case UNIFORM_IVEC3: glUniform3iv(uniformLoc, count, (int *)value); break;
case UNIFORM_IVEC4: glUniform4iv(uniformLoc, count, (int *)value); break;
case UNIFORM_SAMPLER2D: glUniform1iv(uniformLoc, count, (int *)value); break;
default: TRACELOG(LOG_WARNING, "SHADER: [ID %i] Failed to set uniform, data type not recognized", shader.id);
}


#endif
}



void SetShaderValueMatrix(Shader shader, int uniformLoc, Matrix mat)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
glUseProgram(shader.id);

glUniformMatrix4fv(uniformLoc, 1, false, MatrixToFloat(mat));


#endif
}


void SetShaderValueTexture(Shader shader, int uniformLoc, Texture2D texture)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
glUseProgram(shader.id);

glUniform1i(uniformLoc, texture.id);


#endif
}


void SetMatrixProjection(Matrix projection)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
RLGL.State.projection = projection;
#endif
}


Matrix GetMatrixProjection(void) {
#if defined(GRAPHICS_API_OPENGL_11)
float mat[16];
glGetFloatv(GL_PROJECTION_MATRIX,mat);
Matrix m;
m.m0 = mat[0]; m.m1 = mat[1]; m.m2 = mat[2]; m.m3 = mat[3];
m.m4 = mat[4]; m.m5 = mat[5]; m.m6 = mat[6]; m.m7 = mat[7];
m.m8 = mat[8]; m.m9 = mat[9]; m.m10 = mat[10]; m.m11 = mat[11];
m.m12 = mat[12]; m.m13 = mat[13]; m.m14 = mat[14]; m.m15 = mat[15];
return m;
#else
return RLGL.State.projection;
#endif
#
}


void SetMatrixModelview(Matrix view)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
RLGL.State.modelview = view;
#endif
}


Matrix GetMatrixModelview(void)
{
Matrix matrix = MatrixIdentity();
#if defined(GRAPHICS_API_OPENGL_11)
float mat[16];
glGetFloatv(GL_MODELVIEW_MATRIX, mat);
matrix.m0 = mat[0]; matrix.m1 = mat[1]; matrix.m2 = mat[2]; matrix.m3 = mat[3];
matrix.m4 = mat[4]; matrix.m5 = mat[5]; matrix.m6 = mat[6]; matrix.m7 = mat[7];
matrix.m8 = mat[8]; matrix.m9 = mat[9]; matrix.m10 = mat[10]; matrix.m11 = mat[11];
matrix.m12 = mat[12]; matrix.m13 = mat[13]; matrix.m14 = mat[14]; matrix.m15 = mat[15];
#else
matrix = RLGL.State.modelview;
#endif
return matrix;
}



Texture2D GenTextureCubemap(Shader shader, Texture2D map, int size)
{
Texture2D cubemap = { 0 };
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)




glDisable(GL_CULL_FACE);
#if defined(GRAPHICS_API_OPENGL_33)
glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); 
#endif


unsigned int fbo, rbo;
glGenFramebuffers(1, &fbo);
glGenRenderbuffers(1, &rbo);
glBindFramebuffer(GL_FRAMEBUFFER, fbo);
glBindRenderbuffer(GL_RENDERBUFFER, rbo);
#if defined(GRAPHICS_API_OPENGL_33)
glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size, size);
#elif defined(GRAPHICS_API_OPENGL_ES2)
glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, size, size);
#endif
glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);



glGenTextures(1, &cubemap.id);
glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap.id);
for (unsigned int i = 0; i < 6; i++)
{
#if defined(GRAPHICS_API_OPENGL_33)
glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, size, size, 0, GL_RGB, GL_FLOAT, NULL);
#elif defined(GRAPHICS_API_OPENGL_ES2)
if (RLGL.ExtSupported.texFloat32) glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, size, size, 0, GL_RGB, GL_FLOAT, NULL);
#endif
}

glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#if defined(GRAPHICS_API_OPENGL_33)
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); 
#endif
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


Matrix fboProjection = MatrixPerspective(90.0*DEG2RAD, 1.0, DEFAULT_NEAR_CULL_DISTANCE, DEFAULT_FAR_CULL_DISTANCE);
Matrix fboViews[6] = {
MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){ 1.0f, 0.0f, 0.0f }, (Vector3){ 0.0f, -1.0f, 0.0f }),
MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){ -1.0f, 0.0f, 0.0f }, (Vector3){ 0.0f, -1.0f, 0.0f }),
MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){ 0.0f, 1.0f, 0.0f }, (Vector3){ 0.0f, 0.0f, 1.0f }),
MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){ 0.0f, -1.0f, 0.0f }, (Vector3){ 0.0f, 0.0f, -1.0f }),
MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){ 0.0f, 0.0f, 1.0f }, (Vector3){ 0.0f, -1.0f, 0.0f }),
MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){ 0.0f, 0.0f, -1.0f }, (Vector3){ 0.0f, -1.0f, 0.0f })
};


glUseProgram(shader.id);
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_2D, map.id);
SetShaderValueMatrix(shader, shader.locs[LOC_MATRIX_PROJECTION], fboProjection);


glViewport(0, 0, size, size);
glBindFramebuffer(GL_FRAMEBUFFER, fbo);

for (int i = 0; i < 6; i++)
{
SetShaderValueMatrix(shader, shader.locs[LOC_MATRIX_VIEW], fboViews[i]);
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemap.id, 0);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
GenDrawCube();
}


glBindFramebuffer(GL_FRAMEBUFFER, 0);


glViewport(0, 0, RLGL.State.framebufferWidth, RLGL.State.framebufferHeight);



cubemap.width = size;
cubemap.height = size;
cubemap.mipmaps = 1;
cubemap.format = UNCOMPRESSED_R32G32B32;
#endif
return cubemap;
}



Texture2D GenTextureIrradiance(Shader shader, Texture2D cubemap, int size)
{
Texture2D irradiance = { 0 };

#if defined(GRAPHICS_API_OPENGL_33) 




unsigned int fbo, rbo;
glGenFramebuffers(1, &fbo);
glGenRenderbuffers(1, &rbo);
glBindFramebuffer(GL_FRAMEBUFFER, fbo);
glBindRenderbuffer(GL_RENDERBUFFER, rbo);
glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size, size);
glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);


glGenTextures(1, &irradiance.id);
glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance.id);
for (unsigned int i = 0; i < 6; i++)
{
glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, size, size, 0, GL_RGB, GL_FLOAT, NULL);
}

glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


Matrix fboProjection = MatrixPerspective(90.0*DEG2RAD, 1.0, DEFAULT_NEAR_CULL_DISTANCE, DEFAULT_FAR_CULL_DISTANCE);
Matrix fboViews[6] = {
MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){ 1.0f, 0.0f, 0.0f }, (Vector3){ 0.0f, -1.0f, 0.0f }),
MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){ -1.0f, 0.0f, 0.0f }, (Vector3){ 0.0f, -1.0f, 0.0f }),
MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){ 0.0f, 1.0f, 0.0f }, (Vector3){ 0.0f, 0.0f, 1.0f }),
MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){ 0.0f, -1.0f, 0.0f }, (Vector3){ 0.0f, 0.0f, -1.0f }),
MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){ 0.0f, 0.0f, 1.0f }, (Vector3){ 0.0f, -1.0f, 0.0f }),
MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){ 0.0f, 0.0f, -1.0f }, (Vector3){ 0.0f, -1.0f, 0.0f })
};


glUseProgram(shader.id);
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap.id);
SetShaderValueMatrix(shader, shader.locs[LOC_MATRIX_PROJECTION], fboProjection);


glViewport(0, 0, size, size);
glBindFramebuffer(GL_FRAMEBUFFER, fbo);

for (int i = 0; i < 6; i++)
{
SetShaderValueMatrix(shader, shader.locs[LOC_MATRIX_VIEW], fboViews[i]);
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradiance.id, 0);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
GenDrawCube();
}


glBindFramebuffer(GL_FRAMEBUFFER, 0);


glViewport(0, 0, RLGL.State.framebufferWidth, RLGL.State.framebufferHeight);

irradiance.width = size;
irradiance.height = size;
irradiance.mipmaps = 1;

#endif
return irradiance;
}



Texture2D GenTexturePrefilter(Shader shader, Texture2D cubemap, int size)
{
Texture2D prefilter = { 0 };

#if defined(GRAPHICS_API_OPENGL_33) 



int roughnessLoc = GetShaderLocation(shader, "roughness");


unsigned int fbo, rbo;
glGenFramebuffers(1, &fbo);
glGenRenderbuffers(1, &rbo);
glBindFramebuffer(GL_FRAMEBUFFER, fbo);
glBindRenderbuffer(GL_RENDERBUFFER, rbo);
glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size, size);
glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);


glGenTextures(1, &prefilter.id);
glBindTexture(GL_TEXTURE_CUBE_MAP, prefilter.id);
for (unsigned int i = 0; i < 6; i++)
{
glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, size, size, 0, GL_RGB, GL_FLOAT, NULL);
}

glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


glGenerateMipmap(GL_TEXTURE_CUBE_MAP);


Matrix fboProjection = MatrixPerspective(90.0*DEG2RAD, 1.0, DEFAULT_NEAR_CULL_DISTANCE, DEFAULT_FAR_CULL_DISTANCE);
Matrix fboViews[6] = {
MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){ 1.0f, 0.0f, 0.0f }, (Vector3){ 0.0f, -1.0f, 0.0f }),
MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){ -1.0f, 0.0f, 0.0f }, (Vector3){ 0.0f, -1.0f, 0.0f }),
MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){ 0.0f, 1.0f, 0.0f }, (Vector3){ 0.0f, 0.0f, 1.0f }),
MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){ 0.0f, -1.0f, 0.0f }, (Vector3){ 0.0f, 0.0f, -1.0f }),
MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){ 0.0f, 0.0f, 1.0f }, (Vector3){ 0.0f, -1.0f, 0.0f }),
MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){ 0.0f, 0.0f, -1.0f }, (Vector3){ 0.0f, -1.0f, 0.0f })
};


glUseProgram(shader.id);
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap.id);
SetShaderValueMatrix(shader, shader.locs[LOC_MATRIX_PROJECTION], fboProjection);

glBindFramebuffer(GL_FRAMEBUFFER, fbo);

#define MAX_MIPMAP_LEVELS 5 

for (int mip = 0; mip < MAX_MIPMAP_LEVELS; mip++)
{

unsigned int mipWidth = size*(int)powf(0.5f, (float)mip);
unsigned int mipHeight = size*(int)powf(0.5f, (float)mip);

glBindRenderbuffer(GL_RENDERBUFFER, rbo);
glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
glViewport(0, 0, mipWidth, mipHeight);

float roughness = (float)mip/(float)(MAX_MIPMAP_LEVELS - 1);
glUniform1f(roughnessLoc, roughness);

for (int i = 0; i < 6; i++)
{
SetShaderValueMatrix(shader, shader.locs[LOC_MATRIX_VIEW], fboViews[i]);
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilter.id, mip);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
GenDrawCube();
}
}


glBindFramebuffer(GL_FRAMEBUFFER, 0);


glViewport(0, 0, RLGL.State.framebufferWidth, RLGL.State.framebufferHeight);

prefilter.width = size;
prefilter.height = size;


#endif
return prefilter;
}




Texture2D GenTextureBRDF(Shader shader, int size)
{
Texture2D brdf = { 0 };
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)

glGenTextures(1, &brdf.id);
glBindTexture(GL_TEXTURE_2D, brdf.id);
#if defined(GRAPHICS_API_OPENGL_33)
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, size, size, 0, GL_RGB, GL_FLOAT, NULL);
#elif defined(GRAPHICS_API_OPENGL_ES2)
if (RLGL.ExtSupported.texFloat32) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size, size, 0, GL_RGB, GL_FLOAT, NULL);
#endif

glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


unsigned int fbo, rbo;
glGenFramebuffers(1, &fbo);
glGenRenderbuffers(1, &rbo);
glBindFramebuffer(GL_FRAMEBUFFER, fbo);
glBindRenderbuffer(GL_RENDERBUFFER, rbo);
#if defined(GRAPHICS_API_OPENGL_33)
glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size, size);
#elif defined(GRAPHICS_API_OPENGL_ES2)
glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, size, size);
#endif
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdf.id, 0);

glViewport(0, 0, size, size);
glUseProgram(shader.id);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
GenDrawQuad();


glBindFramebuffer(GL_FRAMEBUFFER, 0);


glDeleteRenderbuffers(1, &rbo);
glDeleteFramebuffers(1, &fbo);


glViewport(0, 0, RLGL.State.framebufferWidth, RLGL.State.framebufferHeight);

brdf.width = size;
brdf.height = size;
brdf.mipmaps = 1;
brdf.format = UNCOMPRESSED_R32G32B32;
#endif
return brdf;
}



void BeginBlendMode(int mode)
{
static int blendMode = 0; 

if ((blendMode != mode) && (mode < 3))
{
rlglDraw();

switch (mode)
{
case BLEND_ALPHA: glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); break;
case BLEND_ADDITIVE: glBlendFunc(GL_SRC_ALPHA, GL_ONE); break; 
case BLEND_MULTIPLIED: glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA); break;
default: break;
}

blendMode = mode;
}
}


void EndBlendMode(void)
{
BeginBlendMode(BLEND_ALPHA);
}

#if defined(SUPPORT_VR_SIMULATOR)


void InitVrSimulator(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)


RLGL.Vr.stereoFbo = rlLoadRenderTexture(RLGL.State.framebufferWidth, RLGL.State.framebufferHeight, UNCOMPRESSED_R8G8B8A8, 24, false);

RLGL.Vr.simulatorReady = true;
#else
TRACELOG(LOG_WARNING, "RLGL: VR Simulator not supported on OpenGL 1.1");
#endif
}



void UpdateVrTracking(Camera *camera)
{

}


void CloseVrSimulator(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
if (RLGL.Vr.simulatorReady) rlDeleteRenderTextures(RLGL.Vr.stereoFbo); 
#endif
}


void SetVrConfiguration(VrDeviceInfo hmd, Shader distortion)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)

memset(&RLGL.Vr.config, 0, sizeof(RLGL.Vr.config));


RLGL.Vr.config.distortionShader = distortion;


float aspect = ((float)hmd.hResolution*0.5f)/(float)hmd.vResolution;


float lensShift = (hmd.hScreenSize*0.25f - hmd.lensSeparationDistance*0.5f)/hmd.hScreenSize;
float leftLensCenter[2] = { 0.25f + lensShift, 0.5f };
float rightLensCenter[2] = { 0.75f - lensShift, 0.5f };
float leftScreenCenter[2] = { 0.25f, 0.5f };
float rightScreenCenter[2] = { 0.75f, 0.5f };



float lensRadius = fabsf(-1.0f - 4.0f*lensShift);
float lensRadiusSq = lensRadius*lensRadius;
float distortionScale = hmd.lensDistortionValues[0] +
hmd.lensDistortionValues[1]*lensRadiusSq +
hmd.lensDistortionValues[2]*lensRadiusSq*lensRadiusSq +
hmd.lensDistortionValues[3]*lensRadiusSq*lensRadiusSq*lensRadiusSq;

TRACELOGD("RLGL: VR device configuration:");
TRACELOGD(" > Distortion Scale: %f", distortionScale);

float normScreenWidth = 0.5f;
float normScreenHeight = 1.0f;
float scaleIn[2] = { 2.0f/normScreenWidth, 2.0f/normScreenHeight/aspect };
float scale[2] = { normScreenWidth*0.5f/distortionScale, normScreenHeight*0.5f*aspect/distortionScale };

TRACELOGD(" > Distortion Shader: LeftLensCenter = { %f, %f }", leftLensCenter[0], leftLensCenter[1]);
TRACELOGD(" > Distortion Shader: RightLensCenter = { %f, %f }", rightLensCenter[0], rightLensCenter[1]);
TRACELOGD(" > Distortion Shader: Scale = { %f, %f }", scale[0], scale[1]);
TRACELOGD(" > Distortion Shader: ScaleIn = { %f, %f }", scaleIn[0], scaleIn[1]);




float fovy = 2.0f*(float)atan2f(hmd.vScreenSize*0.5f, hmd.eyeToScreenDistance);


float projOffset = 4.0f*lensShift; 
Matrix proj = MatrixPerspective(fovy, aspect, DEFAULT_NEAR_CULL_DISTANCE, DEFAULT_FAR_CULL_DISTANCE);
RLGL.Vr.config.eyesProjection[0] = MatrixMultiply(proj, MatrixTranslate(projOffset, 0.0f, 0.0f));
RLGL.Vr.config.eyesProjection[1] = MatrixMultiply(proj, MatrixTranslate(-projOffset, 0.0f, 0.0f));





RLGL.Vr.config.eyesViewOffset[0] = MatrixTranslate(-hmd.interpupillaryDistance*0.5f, 0.075f, 0.045f);
RLGL.Vr.config.eyesViewOffset[1] = MatrixTranslate(hmd.interpupillaryDistance*0.5f, 0.075f, 0.045f);


RLGL.Vr.config.eyeViewportRight[2] = hmd.hResolution/2;
RLGL.Vr.config.eyeViewportRight[3] = hmd.vResolution;

RLGL.Vr.config.eyeViewportLeft[0] = hmd.hResolution/2;
RLGL.Vr.config.eyeViewportLeft[1] = 0;
RLGL.Vr.config.eyeViewportLeft[2] = hmd.hResolution/2;
RLGL.Vr.config.eyeViewportLeft[3] = hmd.vResolution;

if (RLGL.Vr.config.distortionShader.id > 0)
{

SetShaderValue(RLGL.Vr.config.distortionShader, GetShaderLocation(RLGL.Vr.config.distortionShader, "leftLensCenter"), leftLensCenter, UNIFORM_VEC2);
SetShaderValue(RLGL.Vr.config.distortionShader, GetShaderLocation(RLGL.Vr.config.distortionShader, "rightLensCenter"), rightLensCenter, UNIFORM_VEC2);
SetShaderValue(RLGL.Vr.config.distortionShader, GetShaderLocation(RLGL.Vr.config.distortionShader, "leftScreenCenter"), leftScreenCenter, UNIFORM_VEC2);
SetShaderValue(RLGL.Vr.config.distortionShader, GetShaderLocation(RLGL.Vr.config.distortionShader, "rightScreenCenter"), rightScreenCenter, UNIFORM_VEC2);

SetShaderValue(RLGL.Vr.config.distortionShader, GetShaderLocation(RLGL.Vr.config.distortionShader, "scale"), scale, UNIFORM_VEC2);
SetShaderValue(RLGL.Vr.config.distortionShader, GetShaderLocation(RLGL.Vr.config.distortionShader, "scaleIn"), scaleIn, UNIFORM_VEC2);
SetShaderValue(RLGL.Vr.config.distortionShader, GetShaderLocation(RLGL.Vr.config.distortionShader, "hmdWarpParam"), hmd.lensDistortionValues, UNIFORM_VEC4);
SetShaderValue(RLGL.Vr.config.distortionShader, GetShaderLocation(RLGL.Vr.config.distortionShader, "chromaAbParam"), hmd.chromaAbCorrection, UNIFORM_VEC4);
}
#endif
}


bool IsVrSimulatorReady(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
return RLGL.Vr.simulatorReady;
#else
return false;
#endif
}


void ToggleVrMode(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
RLGL.Vr.simulatorReady = !RLGL.Vr.simulatorReady;

if (!RLGL.Vr.simulatorReady)
{
RLGL.Vr.stereoRender = false;


rlViewport(0, 0, RLGL.State.framebufferWidth, RLGL.State.framebufferHeight);
RLGL.State.projection = MatrixOrtho(0.0, RLGL.State.framebufferWidth, RLGL.State.framebufferHeight, 0.0, 0.0, 1.0);
RLGL.State.modelview = MatrixIdentity();
}
else RLGL.Vr.stereoRender = true;
#endif
}


void BeginVrDrawing(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
if (RLGL.Vr.simulatorReady)
{
rlEnableRenderTexture(RLGL.Vr.stereoFbo.id); 



rlClearScreenBuffers(); 

RLGL.Vr.stereoRender = true;
}
#endif
}


void EndVrDrawing(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
if (RLGL.Vr.simulatorReady)
{
RLGL.Vr.stereoRender = false; 

rlDisableRenderTexture(); 

rlClearScreenBuffers(); 


rlViewport(0, 0, RLGL.State.framebufferWidth, RLGL.State.framebufferHeight);


rlMatrixMode(RL_PROJECTION); 
rlLoadIdentity(); 
rlOrtho(0.0, RLGL.State.framebufferWidth, RLGL.State.framebufferHeight, 0.0, 0.0, 1.0); 
rlMatrixMode(RL_MODELVIEW); 
rlLoadIdentity(); 


if (RLGL.Vr.config.distortionShader.id > 0) RLGL.State.currentShader = RLGL.Vr.config.distortionShader;
else RLGL.State.currentShader = GetShaderDefault();

rlEnableTexture(RLGL.Vr.stereoFbo.texture.id);

rlPushMatrix();
rlBegin(RL_QUADS);
rlColor4ub(255, 255, 255, 255);
rlNormal3f(0.0f, 0.0f, 1.0f);


rlTexCoord2f(0.0f, 1.0f);
rlVertex2f(0.0f, 0.0f);


rlTexCoord2f(0.0f, 0.0f);
rlVertex2f(0.0f, (float)RLGL.Vr.stereoFbo.texture.height);


rlTexCoord2f(1.0f, 0.0f);
rlVertex2f((float)RLGL.Vr.stereoFbo.texture.width, (float)RLGL.Vr.stereoFbo.texture.height);


rlTexCoord2f(1.0f, 1.0f);
rlVertex2f((float)RLGL.Vr.stereoFbo.texture.width, 0.0f);
rlEnd();
rlPopMatrix();

rlDisableTexture();


UpdateBuffersDefault();
DrawBuffersDefault();


RLGL.State.currentShader = RLGL.State.defaultShader;


rlViewport(0, 0, RLGL.State.framebufferWidth, RLGL.State.framebufferHeight);
RLGL.State.projection = MatrixOrtho(0.0, RLGL.State.framebufferWidth, RLGL.State.framebufferHeight, 0.0, 0.0, 1.0);
RLGL.State.modelview = MatrixIdentity();

rlDisableDepthTest();
}
#endif
}
#endif 





#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)

static unsigned int CompileShader(const char *shaderStr, int type)
{
unsigned int shader = glCreateShader(type);
glShaderSource(shader, 1, &shaderStr, NULL);

GLint success = 0;
glCompileShader(shader);
glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

if (success != GL_TRUE)
{
TRACELOG(LOG_WARNING, "SHADER: [ID %i] Failed to compile shader code", shader);
int maxLength = 0;
int length;
glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

#if defined(_MSC_VER)
char *log = RL_MALLOC(maxLength);
#else
char log[maxLength];
#endif
glGetShaderInfoLog(shader, maxLength, &length, log);

TRACELOG(LOG_WARNING, "SHADER: [ID %i] Compile error: %s", shader, log);

#if defined(_MSC_VER)
RL_FREE(log);
#endif
}
else TRACELOG(LOG_INFO, "SHADER: [ID %i] Compiled successfully", shader);

return shader;
}


static unsigned int LoadShaderProgram(unsigned int vShaderId, unsigned int fShaderId)
{
unsigned int program = 0;

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)

GLint success = 0;
program = glCreateProgram();

glAttachShader(program, vShaderId);
glAttachShader(program, fShaderId);


glBindAttribLocation(program, 0, DEFAULT_ATTRIB_POSITION_NAME);
glBindAttribLocation(program, 1, DEFAULT_ATTRIB_TEXCOORD_NAME);
glBindAttribLocation(program, 2, DEFAULT_ATTRIB_NORMAL_NAME);
glBindAttribLocation(program, 3, DEFAULT_ATTRIB_COLOR_NAME);
glBindAttribLocation(program, 4, DEFAULT_ATTRIB_TANGENT_NAME);
glBindAttribLocation(program, 5, DEFAULT_ATTRIB_TEXCOORD2_NAME);



glLinkProgram(program);



glGetProgramiv(program, GL_LINK_STATUS, &success);

if (success == GL_FALSE)
{
TRACELOG(LOG_WARNING, "SHADER: [ID %i] Failed to link shader program", program);

int maxLength = 0;
int length;

glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

#if defined(_MSC_VER)
char *log = RL_MALLOC(maxLength);
#else
char log[maxLength];
#endif
glGetProgramInfoLog(program, maxLength, &length, log);

TRACELOG(LOG_WARNING, "SHADER: [ID %i] Link error: %s", program, log);

#if defined(_MSC_VER)
RL_FREE(log);
#endif
glDeleteProgram(program);

program = 0;
}
else TRACELOG(LOG_INFO, "SHADER: [ID %i] Program loaded successfully", program);
#endif
return program;
}




static Shader LoadShaderDefault(void)
{
Shader shader = { 0 };
shader.locs = (int *)RL_CALLOC(MAX_SHADER_LOCATIONS, sizeof(int));


for (int i = 0; i < MAX_SHADER_LOCATIONS; i++) shader.locs[i] = -1;


const char *defaultVShaderStr =
#if defined(GRAPHICS_API_OPENGL_21)
"#version 120 \n"
#elif defined(GRAPHICS_API_OPENGL_ES2)
"#version 100 \n"
#endif
#if defined(GRAPHICS_API_OPENGL_ES2) || defined(GRAPHICS_API_OPENGL_21)
"attribute vec3 vertexPosition; \n"
"attribute vec2 vertexTexCoord; \n"
"attribute vec4 vertexColor; \n"
"varying vec2 fragTexCoord; \n"
"varying vec4 fragColor; \n"
#elif defined(GRAPHICS_API_OPENGL_33)
"#version 330 \n"
"in vec3 vertexPosition; \n"
"in vec2 vertexTexCoord; \n"
"in vec4 vertexColor; \n"
"out vec2 fragTexCoord; \n"
"out vec4 fragColor; \n"
#endif
"uniform mat4 mvp; \n"
"void main() \n"
"{ \n"
" fragTexCoord = vertexTexCoord; \n"
" fragColor = vertexColor; \n"
" gl_Position = mvp*vec4(vertexPosition, 1.0); \n"
"} \n";


const char *defaultFShaderStr =
#if defined(GRAPHICS_API_OPENGL_21)
"#version 120 \n"
#elif defined(GRAPHICS_API_OPENGL_ES2)
"#version 100 \n"
"precision mediump float; \n" 
#endif
#if defined(GRAPHICS_API_OPENGL_ES2) || defined(GRAPHICS_API_OPENGL_21)
"varying vec2 fragTexCoord; \n"
"varying vec4 fragColor; \n"
#elif defined(GRAPHICS_API_OPENGL_33)
"#version 330 \n"
"in vec2 fragTexCoord; \n"
"in vec4 fragColor; \n"
"out vec4 finalColor; \n"
#endif
"uniform sampler2D texture0; \n"
"uniform vec4 colDiffuse; \n"
"void main() \n"
"{ \n"
#if defined(GRAPHICS_API_OPENGL_ES2) || defined(GRAPHICS_API_OPENGL_21)
" vec4 texelColor = texture2D(texture0, fragTexCoord); \n" 
" gl_FragColor = texelColor*colDiffuse*fragColor; \n"
#elif defined(GRAPHICS_API_OPENGL_33)
" vec4 texelColor = texture(texture0, fragTexCoord); \n"
" finalColor = texelColor*colDiffuse*fragColor; \n"
#endif
"} \n";


RLGL.State.defaultVShaderId = CompileShader(defaultVShaderStr, GL_VERTEX_SHADER); 
RLGL.State.defaultFShaderId = CompileShader(defaultFShaderStr, GL_FRAGMENT_SHADER); 

shader.id = LoadShaderProgram(RLGL.State.defaultVShaderId, RLGL.State.defaultFShaderId);

if (shader.id > 0)
{
TRACELOG(LOG_INFO, "SHADER: [ID %i] Default shader loaded successfully", shader.id);


shader.locs[LOC_VERTEX_POSITION] = glGetAttribLocation(shader.id, "vertexPosition");
shader.locs[LOC_VERTEX_TEXCOORD01] = glGetAttribLocation(shader.id, "vertexTexCoord");
shader.locs[LOC_VERTEX_COLOR] = glGetAttribLocation(shader.id, "vertexColor");


shader.locs[LOC_MATRIX_MVP] = glGetUniformLocation(shader.id, "mvp");
shader.locs[LOC_COLOR_DIFFUSE] = glGetUniformLocation(shader.id, "colDiffuse");
shader.locs[LOC_MAP_DIFFUSE] = glGetUniformLocation(shader.id, "texture0");




}
else TRACELOG(LOG_WARNING, "SHADER: [ID %i] Failed to load default shader", shader.id);

return shader;
}



static void SetShaderDefaultLocations(Shader *shader)
{









shader->locs[LOC_VERTEX_POSITION] = glGetAttribLocation(shader->id, DEFAULT_ATTRIB_POSITION_NAME);
shader->locs[LOC_VERTEX_TEXCOORD01] = glGetAttribLocation(shader->id, DEFAULT_ATTRIB_TEXCOORD_NAME);
shader->locs[LOC_VERTEX_TEXCOORD02] = glGetAttribLocation(shader->id, DEFAULT_ATTRIB_TEXCOORD2_NAME);
shader->locs[LOC_VERTEX_NORMAL] = glGetAttribLocation(shader->id, DEFAULT_ATTRIB_NORMAL_NAME);
shader->locs[LOC_VERTEX_TANGENT] = glGetAttribLocation(shader->id, DEFAULT_ATTRIB_TANGENT_NAME);
shader->locs[LOC_VERTEX_COLOR] = glGetAttribLocation(shader->id, DEFAULT_ATTRIB_COLOR_NAME);


shader->locs[LOC_MATRIX_MVP] = glGetUniformLocation(shader->id, "mvp");
shader->locs[LOC_MATRIX_PROJECTION] = glGetUniformLocation(shader->id, "projection");
shader->locs[LOC_MATRIX_VIEW] = glGetUniformLocation(shader->id, "view");


shader->locs[LOC_COLOR_DIFFUSE] = glGetUniformLocation(shader->id, "colDiffuse");
shader->locs[LOC_MAP_DIFFUSE] = glGetUniformLocation(shader->id, "texture0");
shader->locs[LOC_MAP_SPECULAR] = glGetUniformLocation(shader->id, "texture1");
shader->locs[LOC_MAP_NORMAL] = glGetUniformLocation(shader->id, "texture2");
}


static void UnloadShaderDefault(void)
{
glUseProgram(0);

glDetachShader(RLGL.State.defaultShader.id, RLGL.State.defaultVShaderId);
glDetachShader(RLGL.State.defaultShader.id, RLGL.State.defaultFShaderId);
glDeleteShader(RLGL.State.defaultVShaderId);
glDeleteShader(RLGL.State.defaultFShaderId);

glDeleteProgram(RLGL.State.defaultShader.id);
}


static void LoadBuffersDefault(void)
{


for (int i = 0; i < MAX_BATCH_BUFFERING; i++)
{
RLGL.State.vertexData[i].vertices = (float *)RL_MALLOC(sizeof(float)*3*4*MAX_BATCH_ELEMENTS); 
RLGL.State.vertexData[i].texcoords = (float *)RL_MALLOC(sizeof(float)*2*4*MAX_BATCH_ELEMENTS); 
RLGL.State.vertexData[i].colors = (unsigned char *)RL_MALLOC(sizeof(unsigned char)*4*4*MAX_BATCH_ELEMENTS); 
#if defined(GRAPHICS_API_OPENGL_33)
RLGL.State.vertexData[i].indices = (unsigned int *)RL_MALLOC(sizeof(unsigned int)*6*MAX_BATCH_ELEMENTS); 
#elif defined(GRAPHICS_API_OPENGL_ES2)
RLGL.State.vertexData[i].indices = (unsigned short *)RL_MALLOC(sizeof(unsigned short)*6*MAX_BATCH_ELEMENTS); 
#endif

for (int j = 0; j < (3*4*MAX_BATCH_ELEMENTS); j++) RLGL.State.vertexData[i].vertices[j] = 0.0f;
for (int j = 0; j < (2*4*MAX_BATCH_ELEMENTS); j++) RLGL.State.vertexData[i].texcoords[j] = 0.0f;
for (int j = 0; j < (4*4*MAX_BATCH_ELEMENTS); j++) RLGL.State.vertexData[i].colors[j] = 0;

int k = 0;


for (int j = 0; j < (6*MAX_BATCH_ELEMENTS); j += 6)
{
RLGL.State.vertexData[i].indices[j] = 4*k;
RLGL.State.vertexData[i].indices[j + 1] = 4*k + 1;
RLGL.State.vertexData[i].indices[j + 2] = 4*k + 2;
RLGL.State.vertexData[i].indices[j + 3] = 4*k;
RLGL.State.vertexData[i].indices[j + 4] = 4*k + 2;
RLGL.State.vertexData[i].indices[j + 5] = 4*k + 3;

k++;
}

RLGL.State.vertexData[i].vCounter = 0;
RLGL.State.vertexData[i].tcCounter = 0;
RLGL.State.vertexData[i].cCounter = 0;
}

TRACELOG(LOG_INFO, "RLGL: Internal vertex buffers initialized successfully in RAM (CPU)");




for (int i = 0; i < MAX_BATCH_BUFFERING; i++)
{
if (RLGL.ExtSupported.vao)
{

glGenVertexArrays(1, &RLGL.State.vertexData[i].vaoId);
glBindVertexArray(RLGL.State.vertexData[i].vaoId);
}



glGenBuffers(1, &RLGL.State.vertexData[i].vboId[0]);
glBindBuffer(GL_ARRAY_BUFFER, RLGL.State.vertexData[i].vboId[0]);
glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*4*MAX_BATCH_ELEMENTS, RLGL.State.vertexData[i].vertices, GL_DYNAMIC_DRAW);
glEnableVertexAttribArray(RLGL.State.currentShader.locs[LOC_VERTEX_POSITION]);
glVertexAttribPointer(RLGL.State.currentShader.locs[LOC_VERTEX_POSITION], 3, GL_FLOAT, 0, 0, 0);


glGenBuffers(1, &RLGL.State.vertexData[i].vboId[1]);
glBindBuffer(GL_ARRAY_BUFFER, RLGL.State.vertexData[i].vboId[1]);
glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*4*MAX_BATCH_ELEMENTS, RLGL.State.vertexData[i].texcoords, GL_DYNAMIC_DRAW);
glEnableVertexAttribArray(RLGL.State.currentShader.locs[LOC_VERTEX_TEXCOORD01]);
glVertexAttribPointer(RLGL.State.currentShader.locs[LOC_VERTEX_TEXCOORD01], 2, GL_FLOAT, 0, 0, 0);


glGenBuffers(1, &RLGL.State.vertexData[i].vboId[2]);
glBindBuffer(GL_ARRAY_BUFFER, RLGL.State.vertexData[i].vboId[2]);
glBufferData(GL_ARRAY_BUFFER, sizeof(unsigned char)*4*4*MAX_BATCH_ELEMENTS, RLGL.State.vertexData[i].colors, GL_DYNAMIC_DRAW);
glEnableVertexAttribArray(RLGL.State.currentShader.locs[LOC_VERTEX_COLOR]);
glVertexAttribPointer(RLGL.State.currentShader.locs[LOC_VERTEX_COLOR], 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);


glGenBuffers(1, &RLGL.State.vertexData[i].vboId[3]);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RLGL.State.vertexData[i].vboId[3]);
#if defined(GRAPHICS_API_OPENGL_33)
glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*6*MAX_BATCH_ELEMENTS, RLGL.State.vertexData[i].indices, GL_STATIC_DRAW);
#elif defined(GRAPHICS_API_OPENGL_ES2)
glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(short)*6*MAX_BATCH_ELEMENTS, RLGL.State.vertexData[i].indices, GL_STATIC_DRAW);
#endif
}

TRACELOG(LOG_INFO, "RLGL: Internal vertex buffers uploaded successfully to VRAM (GPU)");


if (RLGL.ExtSupported.vao) glBindVertexArray(0);

}




static void UpdateBuffersDefault(void)
{

if (RLGL.State.vertexData[RLGL.State.currentBuffer].vCounter > 0)
{

if (RLGL.ExtSupported.vao) glBindVertexArray(RLGL.State.vertexData[RLGL.State.currentBuffer].vaoId);


glBindBuffer(GL_ARRAY_BUFFER, RLGL.State.vertexData[RLGL.State.currentBuffer].vboId[0]);
glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*3*RLGL.State.vertexData[RLGL.State.currentBuffer].vCounter, RLGL.State.vertexData[RLGL.State.currentBuffer].vertices);



glBindBuffer(GL_ARRAY_BUFFER, RLGL.State.vertexData[RLGL.State.currentBuffer].vboId[1]);
glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*2*RLGL.State.vertexData[RLGL.State.currentBuffer].vCounter, RLGL.State.vertexData[RLGL.State.currentBuffer].texcoords);



glBindBuffer(GL_ARRAY_BUFFER, RLGL.State.vertexData[RLGL.State.currentBuffer].vboId[2]);
glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(unsigned char)*4*RLGL.State.vertexData[RLGL.State.currentBuffer].vCounter, RLGL.State.vertexData[RLGL.State.currentBuffer].colors);


















if (RLGL.ExtSupported.vao) glBindVertexArray(0);
}
}


static void DrawBuffersDefault(void)
{
Matrix matProjection = RLGL.State.projection;
Matrix matModelView = RLGL.State.modelview;

int eyesCount = 1;
#if defined(SUPPORT_VR_SIMULATOR)
if (RLGL.Vr.stereoRender) eyesCount = 2;
#endif

for (int eye = 0; eye < eyesCount; eye++)
{
#if defined(SUPPORT_VR_SIMULATOR)
if (eyesCount == 2) SetStereoView(eye, matProjection, matModelView);
#endif


if (RLGL.State.vertexData[RLGL.State.currentBuffer].vCounter > 0)
{

glUseProgram(RLGL.State.currentShader.id);


Matrix matMVP = MatrixMultiply(RLGL.State.modelview, RLGL.State.projection);

glUniformMatrix4fv(RLGL.State.currentShader.locs[LOC_MATRIX_MVP], 1, false, MatrixToFloat(matMVP));
glUniform4f(RLGL.State.currentShader.locs[LOC_COLOR_DIFFUSE], 1.0f, 1.0f, 1.0f, 1.0f);
glUniform1i(RLGL.State.currentShader.locs[LOC_MAP_DIFFUSE], 0); 







int vertexOffset = 0;

if (RLGL.ExtSupported.vao) glBindVertexArray(RLGL.State.vertexData[RLGL.State.currentBuffer].vaoId);
else
{

glBindBuffer(GL_ARRAY_BUFFER, RLGL.State.vertexData[RLGL.State.currentBuffer].vboId[0]);
glVertexAttribPointer(RLGL.State.currentShader.locs[LOC_VERTEX_POSITION], 3, GL_FLOAT, 0, 0, 0);
glEnableVertexAttribArray(RLGL.State.currentShader.locs[LOC_VERTEX_POSITION]);


glBindBuffer(GL_ARRAY_BUFFER, RLGL.State.vertexData[RLGL.State.currentBuffer].vboId[1]);
glVertexAttribPointer(RLGL.State.currentShader.locs[LOC_VERTEX_TEXCOORD01], 2, GL_FLOAT, 0, 0, 0);
glEnableVertexAttribArray(RLGL.State.currentShader.locs[LOC_VERTEX_TEXCOORD01]);


glBindBuffer(GL_ARRAY_BUFFER, RLGL.State.vertexData[RLGL.State.currentBuffer].vboId[2]);
glVertexAttribPointer(RLGL.State.currentShader.locs[LOC_VERTEX_COLOR], 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);
glEnableVertexAttribArray(RLGL.State.currentShader.locs[LOC_VERTEX_COLOR]);

glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RLGL.State.vertexData[RLGL.State.currentBuffer].vboId[3]);
}

glActiveTexture(GL_TEXTURE0);

for (int i = 0; i < RLGL.State.drawsCounter; i++)
{
glBindTexture(GL_TEXTURE_2D, RLGL.State.draws[i].textureId);





if ((RLGL.State.draws[i].mode == RL_LINES) || (RLGL.State.draws[i].mode == RL_TRIANGLES)) glDrawArrays(RLGL.State.draws[i].mode, vertexOffset, RLGL.State.draws[i].vertexCount);
else
{
#if defined(GRAPHICS_API_OPENGL_33)



glDrawElements(GL_TRIANGLES, RLGL.State.draws[i].vertexCount/4*6, GL_UNSIGNED_INT, (GLvoid *)(sizeof(GLuint)*vertexOffset/4*6));
#elif defined(GRAPHICS_API_OPENGL_ES2)
glDrawElements(GL_TRIANGLES, RLGL.State.draws[i].vertexCount/4*6, GL_UNSIGNED_SHORT, (GLvoid *)(sizeof(GLushort)*vertexOffset/4*6));
#endif
}

vertexOffset += (RLGL.State.draws[i].vertexCount + RLGL.State.draws[i].vertexAlignment);
}

if (!RLGL.ExtSupported.vao)
{
glBindBuffer(GL_ARRAY_BUFFER, 0);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

glBindTexture(GL_TEXTURE_2D, 0); 
}

if (RLGL.ExtSupported.vao) glBindVertexArray(0); 

glUseProgram(0); 
}


RLGL.State.vertexData[RLGL.State.currentBuffer].vCounter = 0;
RLGL.State.vertexData[RLGL.State.currentBuffer].tcCounter = 0;
RLGL.State.vertexData[RLGL.State.currentBuffer].cCounter = 0;


RLGL.State.currentDepth = -1.0f;


RLGL.State.projection = matProjection;
RLGL.State.modelview = matModelView;


for (int i = 0; i < MAX_DRAWCALL_REGISTERED; i++)
{
RLGL.State.draws[i].mode = RL_QUADS;
RLGL.State.draws[i].vertexCount = 0;
RLGL.State.draws[i].textureId = RLGL.State.defaultTextureId;
}

RLGL.State.drawsCounter = 1;


RLGL.State.currentBuffer++;
if (RLGL.State.currentBuffer >= MAX_BATCH_BUFFERING) RLGL.State.currentBuffer = 0;
}


static void UnloadBuffersDefault(void)
{

if (RLGL.ExtSupported.vao) glBindVertexArray(0);
glDisableVertexAttribArray(0);
glDisableVertexAttribArray(1);
glDisableVertexAttribArray(2);
glDisableVertexAttribArray(3);
glBindBuffer(GL_ARRAY_BUFFER, 0);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

for (int i = 0; i < MAX_BATCH_BUFFERING; i++)
{

glDeleteBuffers(1, &RLGL.State.vertexData[i].vboId[0]);
glDeleteBuffers(1, &RLGL.State.vertexData[i].vboId[1]);
glDeleteBuffers(1, &RLGL.State.vertexData[i].vboId[2]);
glDeleteBuffers(1, &RLGL.State.vertexData[i].vboId[3]);


if (RLGL.ExtSupported.vao) glDeleteVertexArrays(1, &RLGL.State.vertexData[i].vaoId);


RL_FREE(RLGL.State.vertexData[i].vertices);
RL_FREE(RLGL.State.vertexData[i].texcoords);
RL_FREE(RLGL.State.vertexData[i].colors);
RL_FREE(RLGL.State.vertexData[i].indices);
}
}


static void GenDrawQuad(void)
{
unsigned int quadVAO = 0;
unsigned int quadVBO = 0;

float vertices[] = {

-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
};


glGenVertexArrays(1, &quadVAO);
glGenBuffers(1, &quadVBO);
glBindVertexArray(quadVAO);


glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);


glEnableVertexAttribArray(0);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void *)0);
glEnableVertexAttribArray(1);
glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void *)(3*sizeof(float)));


glBindVertexArray(quadVAO);
glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
glBindVertexArray(0);

glDeleteBuffers(1, &quadVBO);
glDeleteVertexArrays(1, &quadVAO);
}


static void GenDrawCube(void)
{
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;

float vertices[] = {
-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
-1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
-1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
-1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
-1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
-1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
-1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
-1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
-1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
-1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
-1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
-1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
1.0f, 1.0f , 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
-1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f
};


glGenVertexArrays(1, &cubeVAO);
glGenBuffers(1, &cubeVBO);


glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);


glBindVertexArray(cubeVAO);
glEnableVertexAttribArray(0);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void *)0);
glEnableVertexAttribArray(1);
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void *)(3*sizeof(float)));
glEnableVertexAttribArray(2);
glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void *)(6*sizeof(float)));
glBindBuffer(GL_ARRAY_BUFFER, 0);
glBindVertexArray(0);


glBindVertexArray(cubeVAO);
glDrawArrays(GL_TRIANGLES, 0, 36);
glBindVertexArray(0);

glDeleteBuffers(1, &cubeVBO);
glDeleteVertexArrays(1, &cubeVAO);
}

#if defined(SUPPORT_VR_SIMULATOR)

static void SetStereoView(int eye, Matrix matProjection, Matrix matModelView)
{
Matrix eyeProjection = matProjection;
Matrix eyeModelView = matModelView;


rlViewport(eye*RLGL.State.framebufferWidth/2, 0, RLGL.State.framebufferWidth/2, RLGL.State.framebufferHeight);


eyeModelView = MatrixMultiply(matModelView, RLGL.Vr.config.eyesViewOffset[eye]);


eyeProjection = RLGL.Vr.config.eyesProjection[eye];

SetMatrixModelview(eyeModelView);
SetMatrixProjection(eyeProjection);
}
#endif 

#endif 

#if defined(GRAPHICS_API_OPENGL_11)


static int GenerateMipmaps(unsigned char *data, int baseWidth, int baseHeight)
{
int mipmapCount = 1; 
int width = baseWidth;
int height = baseHeight;
int size = baseWidth*baseHeight*4; 


while ((width != 1) && (height != 1))
{
if (width != 1) width /= 2;
if (height != 1) height /= 2;

TRACELOGD("TEXTURE: Next mipmap size: %i x %i", width, height);

mipmapCount++;

size += (width*height*4); 
}

TRACELOGD("TEXTURE: Total mipmaps required: %i", mipmapCount);
TRACELOGD("TEXTURE: Total size of data required: %i", size);

unsigned char *temp = RL_REALLOC(data, size);

if (temp != NULL) data = temp;
else TRACELOG(LOG_WARNING, "TEXTURE: Failed to allocate required mipmaps memory");

width = baseWidth;
height = baseHeight;
size = (width*height*4);



Color *image = (Color *)RL_MALLOC(width*height*sizeof(Color));
Color *mipmap = NULL;
int offset = 0;
int j = 0;

for (int i = 0; i < size; i += 4)
{
image[j].r = data[i];
image[j].g = data[i + 1];
image[j].b = data[i + 2];
image[j].a = data[i + 3];
j++;
}

TRACELOGD("TEXTURE: Mipmap base size (%ix%i)", width, height);

for (int mip = 1; mip < mipmapCount; mip++)
{
mipmap = GenNextMipmap(image, width, height);

offset += (width*height*4); 
j = 0;

width /= 2;
height /= 2;
size = (width*height*4); 


for (int i = 0; i < size; i += 4)
{
data[offset + i] = mipmap[j].r;
data[offset + i + 1] = mipmap[j].g;
data[offset + i + 2] = mipmap[j].b;
data[offset + i + 3] = mipmap[j].a;
j++;
}

RL_FREE(image);

image = mipmap;
mipmap = NULL;
}

RL_FREE(mipmap); 

return mipmapCount;
}


static Color *GenNextMipmap(Color *srcData, int srcWidth, int srcHeight)
{
int x2, y2;
Color prow, pcol;

int width = srcWidth/2;
int height = srcHeight/2;

Color *mipmap = (Color *)RL_MALLOC(width*height*sizeof(Color));


for (int y = 0; y < height; y++)
{
y2 = 2*y;

for (int x = 0; x < width; x++)
{
x2 = 2*x;

prow.r = (srcData[y2*srcWidth + x2].r + srcData[y2*srcWidth + x2 + 1].r)/2;
prow.g = (srcData[y2*srcWidth + x2].g + srcData[y2*srcWidth + x2 + 1].g)/2;
prow.b = (srcData[y2*srcWidth + x2].b + srcData[y2*srcWidth + x2 + 1].b)/2;
prow.a = (srcData[y2*srcWidth + x2].a + srcData[y2*srcWidth + x2 + 1].a)/2;

pcol.r = (srcData[(y2+1)*srcWidth + x2].r + srcData[(y2+1)*srcWidth + x2 + 1].r)/2;
pcol.g = (srcData[(y2+1)*srcWidth + x2].g + srcData[(y2+1)*srcWidth + x2 + 1].g)/2;
pcol.b = (srcData[(y2+1)*srcWidth + x2].b + srcData[(y2+1)*srcWidth + x2 + 1].b)/2;
pcol.a = (srcData[(y2+1)*srcWidth + x2].a + srcData[(y2+1)*srcWidth + x2 + 1].a)/2;

mipmap[y*width + x].r = (prow.r + pcol.r)/2;
mipmap[y*width + x].g = (prow.g + pcol.g)/2;
mipmap[y*width + x].b = (prow.b + pcol.b)/2;
mipmap[y*width + x].a = (prow.a + pcol.a)/2;
}
}

TRACELOGD("TEXTURE: Mipmap generated successfully (%ix%i)", width, height);

return mipmap;
}
#endif

#if defined(RLGL_STANDALONE)


char *LoadFileText(const char *fileName)
{
char *text = NULL;

if (fileName != NULL)
{
FILE *textFile = fopen(fileName, "rt");

if (textFile != NULL)
{



fseek(textFile, 0, SEEK_END);
int size = ftell(textFile);
fseek(textFile, 0, SEEK_SET);

if (size > 0)
{
text = (char *)RL_MALLOC(sizeof(char)*(size + 1));
int count = fread(text, sizeof(char), size, textFile);



if (count < size) text = RL_REALLOC(text, count + 1);


text[count] = '\0';

TRACELOG(LOG_INFO, "FILEIO: [%s] Text file loaded successfully", fileName);
}
else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to read text file", fileName);

fclose(textFile);
}
else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to open text file", fileName);
}
else TRACELOG(LOG_WARNING, "FILEIO: File name provided is not valid");

return text;
}



int GetPixelDataSize(int width, int height, int format)
{
int dataSize = 0; 
int bpp = 0; 

switch (format)
{
case UNCOMPRESSED_GRAYSCALE: bpp = 8; break;
case UNCOMPRESSED_GRAY_ALPHA:
case UNCOMPRESSED_R5G6B5:
case UNCOMPRESSED_R5G5B5A1:
case UNCOMPRESSED_R4G4B4A4: bpp = 16; break;
case UNCOMPRESSED_R8G8B8A8: bpp = 32; break;
case UNCOMPRESSED_R8G8B8: bpp = 24; break;
case UNCOMPRESSED_R32: bpp = 32; break;
case UNCOMPRESSED_R32G32B32: bpp = 32*3; break;
case UNCOMPRESSED_R32G32B32A32: bpp = 32*4; break;
case COMPRESSED_DXT1_RGB:
case COMPRESSED_DXT1_RGBA:
case COMPRESSED_ETC1_RGB:
case COMPRESSED_ETC2_RGB:
case COMPRESSED_PVRT_RGB:
case COMPRESSED_PVRT_RGBA: bpp = 4; break;
case COMPRESSED_DXT3_RGBA:
case COMPRESSED_DXT5_RGBA:
case COMPRESSED_ETC2_EAC_RGBA:
case COMPRESSED_ASTC_4x4_RGBA: bpp = 8; break;
case COMPRESSED_ASTC_8x8_RGBA: bpp = 2; break;
default: break;
}

dataSize = width*height*bpp/8; 



if ((width < 4) && (height < 4))
{
if ((format >= COMPRESSED_DXT1_RGB) && (format < COMPRESSED_DXT3_RGBA)) dataSize = 8;
else if ((format >= COMPRESSED_DXT3_RGBA) && (format < COMPRESSED_ASTC_8x8_RGBA)) dataSize = 16;
}

return dataSize;
}
#endif 

#endif 
