#define RLGL_IMPLEMENTATION
#define RLGL_STANDALONE
#define RLGL_SUPPORT_TRACELOG
#include "rlgl.h" 
#if defined(__EMSCRIPTEN__)
#define GLFW_INCLUDE_ES2
#endif
#include <GLFW/glfw3.h> 
#include <stdio.h> 
#define RED (Color){ 230, 41, 55, 255 } 
#define RAYWHITE (Color){ 245, 245, 245, 255 } 
#define DARKGRAY (Color){ 80, 80, 80, 255 } 
static void ErrorCallback(int error, const char *description);
static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
static void DrawGrid(int slices, float spacing);
static void DrawCube(Vector3 position, float width, float height, float length, Color color);
static void DrawCubeWires(Vector3 position, float width, float height, float length, Color color);
static void DrawRectangleV(Vector2 position, Vector2 size, Color color);
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
glfwSetErrorCallback(ErrorCallback);
if (!glfwInit())
{
printf("GLFW3: Can not initialize GLFW\n");
return 1;
}
else printf("GLFW3: GLFW initialized successfully\n");
glfwWindowHint(GLFW_SAMPLES, 4);
glfwWindowHint(GLFW_DEPTH_BITS, 16);
glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
GLFWwindow *window = glfwCreateWindow(screenWidth, screenHeight, "rlgl standalone", NULL, NULL);
if (!window)
{
glfwTerminate();
return 2;
}
else printf("GLFW3: Window created successfully\n");
glfwSetWindowPos(window, 200, 200);
glfwSetKeyCallback(window, KeyCallback);
glfwMakeContextCurrent(window);
glfwSwapInterval(0);
rlLoadExtensions(glfwGetProcAddress);
rlglInit(screenWidth, screenHeight);
rlViewport(0, 0, screenWidth, screenHeight);
rlMatrixMode(RL_PROJECTION); 
rlLoadIdentity(); 
rlOrtho(0, screenWidth, screenHeight, 0, 0.0f, 1.0f); 
rlMatrixMode(RL_MODELVIEW); 
rlLoadIdentity(); 
rlClearColor(245, 245, 245, 255); 
rlEnableDepthTest(); 
Camera camera = { 0 };
camera.position = (Vector3){ 5.0f, 5.0f, 5.0f }; 
camera.target = (Vector3){ 0.0f, 0.0f, 0.0f }; 
camera.up = (Vector3){ 0.0f, 1.0f, 0.0f }; 
camera.fovy = 45.0f; 
Vector3 cubePosition = { 0.0f, 0.0f, 0.0f }; 
while (!glfwWindowShouldClose(window)) 
{
rlClearScreenBuffers(); 
Matrix matProj = MatrixPerspective(camera.fovy*DEG2RAD, (double)screenWidth/(double)screenHeight, 0.01, 1000.0);
Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);
SetMatrixModelview(matView); 
SetMatrixProjection(matProj); 
DrawCube(cubePosition, 2.0f, 2.0f, 2.0f, RED);
DrawCubeWires(cubePosition, 2.0f, 2.0f, 2.0f, RAYWHITE);
DrawGrid(10, 1.0f);
rlglDraw();
#define RLGL_CREATE_MATRIX_MANUALLY
#if defined(RLGL_CREATE_MATRIX_MANUALLY)
matProj = MatrixOrtho(0.0, screenWidth, screenHeight, 0.0, 0.0, 1.0);
matView = MatrixIdentity();
SetMatrixModelview(matView); 
SetMatrixProjection(matProj); 
#else 
rlMatrixMode(RL_PROJECTION); 
rlLoadIdentity(); 
rlOrtho(0.0, screenWidth, screenHeight, 0.0, 0.0, 1.0); 
rlMatrixMode(RL_MODELVIEW); 
rlLoadIdentity(); 
#endif
DrawRectangleV((Vector2){ 10.0f, 10.0f }, (Vector2){ 780.0f, 20.0f }, DARKGRAY);
rlglDraw();
glfwSwapBuffers(window);
glfwPollEvents();
}
rlglClose(); 
glfwDestroyWindow(window); 
glfwTerminate(); 
return 0;
}
static void ErrorCallback(int error, const char *description)
{
fprintf(stderr, description);
}
static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
{
glfwSetWindowShouldClose(window, GL_TRUE);
}
}
static void DrawRectangleV(Vector2 position, Vector2 size, Color color)
{
rlBegin(RL_TRIANGLES);
rlColor4ub(color.r, color.g, color.b, color.a);
rlVertex2i(position.x, position.y);
rlVertex2i(position.x, position.y + size.y);
rlVertex2i(position.x + size.x, position.y + size.y);
rlVertex2i(position.x, position.y);
rlVertex2i(position.x + size.x, position.y + size.y);
rlVertex2i(position.x + size.x, position.y);
rlEnd();
}
static void DrawGrid(int slices, float spacing)
{
int halfSlices = slices / 2;
rlBegin(RL_LINES);
for(int i = -halfSlices; i <= halfSlices; i++)
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
static void DrawCube(Vector3 position, float width, float height, float length, Color color)
{
float x = 0.0f;
float y = 0.0f;
float z = 0.0f;
rlPushMatrix();
rlTranslatef(position.x, position.y, position.z);
rlBegin(RL_TRIANGLES);
rlColor4ub(color.r, color.g, color.b, color.a);
rlVertex3f(x-width/2, y-height/2, z+length/2); 
rlVertex3f(x+width/2, y-height/2, z+length/2); 
rlVertex3f(x-width/2, y+height/2, z+length/2); 
rlVertex3f(x+width/2, y+height/2, z+length/2); 
rlVertex3f(x-width/2, y+height/2, z+length/2); 
rlVertex3f(x+width/2, y-height/2, z+length/2); 
rlVertex3f(x-width/2, y-height/2, z-length/2); 
rlVertex3f(x-width/2, y+height/2, z-length/2); 
rlVertex3f(x+width/2, y-height/2, z-length/2); 
rlVertex3f(x+width/2, y+height/2, z-length/2); 
rlVertex3f(x+width/2, y-height/2, z-length/2); 
rlVertex3f(x-width/2, y+height/2, z-length/2); 
rlVertex3f(x-width/2, y+height/2, z-length/2); 
rlVertex3f(x-width/2, y+height/2, z+length/2); 
rlVertex3f(x+width/2, y+height/2, z+length/2); 
rlVertex3f(x+width/2, y+height/2, z-length/2); 
rlVertex3f(x-width/2, y+height/2, z-length/2); 
rlVertex3f(x+width/2, y+height/2, z+length/2); 
rlVertex3f(x-width/2, y-height/2, z-length/2); 
rlVertex3f(x+width/2, y-height/2, z+length/2); 
rlVertex3f(x-width/2, y-height/2, z+length/2); 
rlVertex3f(x+width/2, y-height/2, z-length/2); 
rlVertex3f(x+width/2, y-height/2, z+length/2); 
rlVertex3f(x-width/2, y-height/2, z-length/2); 
rlVertex3f(x+width/2, y-height/2, z-length/2); 
rlVertex3f(x+width/2, y+height/2, z-length/2); 
rlVertex3f(x+width/2, y+height/2, z+length/2); 
rlVertex3f(x+width/2, y-height/2, z+length/2); 
rlVertex3f(x+width/2, y-height/2, z-length/2); 
rlVertex3f(x+width/2, y+height/2, z+length/2); 
rlVertex3f(x-width/2, y-height/2, z-length/2); 
rlVertex3f(x-width/2, y+height/2, z+length/2); 
rlVertex3f(x-width/2, y+height/2, z-length/2); 
rlVertex3f(x-width/2, y-height/2, z+length/2); 
rlVertex3f(x-width/2, y+height/2, z+length/2); 
rlVertex3f(x-width/2, y-height/2, z-length/2); 
rlEnd();
rlPopMatrix();
}
static void DrawCubeWires(Vector3 position, float width, float height, float length, Color color)
{
float x = 0.0f;
float y = 0.0f;
float z = 0.0f;
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
