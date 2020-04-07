#include "raylib.h"
#define PHYSAC_IMPLEMENTATION
#define PHYSAC_NO_THREADS
#include "physac.h"
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
SetConfigFlags(FLAG_MSAA_4X_HINT);
InitWindow(screenWidth, screenHeight, "Physac [raylib] - Physics demo");
int logoX = screenWidth - MeasureText("Physac", 30) - 10;
int logoY = 15;
bool needsReset = false;
InitPhysics();
PhysicsBody floor = CreatePhysicsBodyRectangle((Vector2){ screenWidth/2, screenHeight }, 500, 100, 10);
floor->enabled = false; 
PhysicsBody circle = CreatePhysicsBodyCircle((Vector2){ screenWidth/2, screenHeight/2 }, 45, 10);
circle->enabled = false; 
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
RunPhysicsStep();
if (needsReset)
{
floor = CreatePhysicsBodyRectangle((Vector2){ screenWidth/2, screenHeight }, 500, 100, 10);
floor->enabled = false;
circle = CreatePhysicsBodyCircle((Vector2){ screenWidth/2, screenHeight/2 }, 45, 10);
circle->enabled = false;
needsReset = false;
}
if (IsKeyPressed('R'))
{
ResetPhysics();
needsReset = true;
}
if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) CreatePhysicsBodyPolygon(GetMousePosition(), GetRandomValue(20, 80), GetRandomValue(3, 8), 10);
else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) CreatePhysicsBodyCircle(GetMousePosition(), GetRandomValue(10, 45), 10);
int bodiesCount = GetPhysicsBodiesCount();
for (int i = bodiesCount - 1; i >= 0; i--)
{
PhysicsBody body = GetPhysicsBody(i);
if (body != NULL && (body->position.y > screenHeight*2)) DestroyPhysicsBody(body);
}
BeginDrawing();
ClearBackground(BLACK);
DrawFPS(screenWidth - 90, screenHeight - 30);
bodiesCount = GetPhysicsBodiesCount();
for (int i = 0; i < bodiesCount; i++)
{
PhysicsBody body = GetPhysicsBody(i);
if (body != NULL)
{
int vertexCount = GetPhysicsShapeVerticesCount(i);
for (int j = 0; j < vertexCount; j++)
{
Vector2 vertexA = GetPhysicsShapeVertex(body, j);
int jj = (((j + 1) < vertexCount) ? (j + 1) : 0); 
Vector2 vertexB = GetPhysicsShapeVertex(body, jj);
DrawLineV(vertexA, vertexB, GREEN); 
}
}
}
DrawText("Left mouse button to create a polygon", 10, 10, 10, WHITE);
DrawText("Right mouse button to create a circle", 10, 25, 10, WHITE);
DrawText("Press 'R' to reset example", 10, 40, 10, WHITE);
DrawText("Physac", logoX, logoY, 30, WHITE);
DrawText("Powered by", logoX + 50, logoY - 7, 10, WHITE);
EndDrawing();
}
ClosePhysics(); 
CloseWindow(); 
return 0;
}
