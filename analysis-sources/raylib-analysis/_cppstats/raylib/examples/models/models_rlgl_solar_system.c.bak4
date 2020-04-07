












#include "raylib.h"
#include "rlgl.h"




void DrawSphereBasic(Color color); 




int main(void)
{


const int screenWidth = 800;
const int screenHeight = 450;

const float sunRadius = 4.0f;
const float earthRadius = 0.6f;
const float earthOrbitRadius = 8.0f;
const float moonRadius = 0.16f;
const float moonOrbitRadius = 1.5f;

InitWindow(screenWidth, screenHeight, "raylib [models] example - rlgl module usage with push/pop matrix transformations");


Camera camera = { 0 };
camera.position = (Vector3){ 16.0f, 16.0f, 16.0f };
camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
camera.fovy = 45.0f;
camera.type = CAMERA_PERSPECTIVE;

SetCameraMode(camera, CAMERA_FREE);

float rotationSpeed = 0.2f; 

float earthRotation = 0.0f; 
float earthOrbitRotation = 0.0f; 
float moonRotation = 0.0f; 
float moonOrbitRotation = 0.0f; 

SetTargetFPS(60); 



while (!WindowShouldClose()) 
{


UpdateCamera(&camera);

earthRotation += (5.0f*rotationSpeed);
earthOrbitRotation += (365/360.0f*(5.0f*rotationSpeed)*rotationSpeed);
moonRotation += (2.0f*rotationSpeed);
moonOrbitRotation += (8.0f*rotationSpeed);




BeginDrawing();

ClearBackground(RAYWHITE);

BeginMode3D(camera);

rlPushMatrix();
rlScalef(sunRadius, sunRadius, sunRadius); 
DrawSphereBasic(GOLD); 
rlPopMatrix();

rlPushMatrix();
rlRotatef(earthOrbitRotation, 0.0f, 1.0f, 0.0f); 
rlTranslatef(earthOrbitRadius, 0.0f, 0.0f); 
rlRotatef(-earthOrbitRotation, 0.0f, 1.0f, 0.0f); 

rlPushMatrix();
rlRotatef(earthRotation, 0.25, 1.0, 0.0); 
rlScalef(earthRadius, earthRadius, earthRadius);

DrawSphereBasic(BLUE); 
rlPopMatrix();

rlRotatef(moonOrbitRotation, 0.0f, 1.0f, 0.0f); 
rlTranslatef(moonOrbitRadius, 0.0f, 0.0f); 
rlRotatef(-moonOrbitRotation, 0.0f, 1.0f, 0.0f); 
rlRotatef(moonRotation, 0.0f, 1.0f, 0.0f); 
rlScalef(moonRadius, moonRadius, moonRadius); 

DrawSphereBasic(LIGHTGRAY); 
rlPopMatrix();


DrawCircle3D((Vector3){ 0.0f, 0.0f, 0.0f }, earthOrbitRadius, (Vector3){ 1, 0, 0 }, 90.0f, Fade(RED, 0.5f));
DrawGrid(20, 1.0f);

EndMode3D();

DrawText("EARTH ORBITING AROUND THE SUN!", 400, 10, 20, MAROON);
DrawFPS(10, 10);

EndDrawing();

}



CloseWindow(); 


return 0;
}







void DrawSphereBasic(Color color)
{
int rings = 16;
int slices = 16;

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
}
