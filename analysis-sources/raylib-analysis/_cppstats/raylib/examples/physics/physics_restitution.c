#include "raylib.h"
#define PHYSAC_IMPLEMENTATION
#define PHYSAC_NO_THREADS
#include "physac.h"
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
SetConfigFlags(FLAG_MSAA_4X_HINT);
InitWindow(screenWidth, screenHeight, "Physac [raylib] - Physics restitution");
int logoX = screenWidth - MeasureText("Physac", 30) - 10;
int logoY = 15;
InitPhysics();
PhysicsBody floor = CreatePhysicsBodyRectangle((Vector2){ screenWidth/2, screenHeight }, screenWidth, 100, 10);
floor->enabled = false; 
floor->restitution = 1;
PhysicsBody circleA = CreatePhysicsBodyCircle((Vector2){ screenWidth*0.25f, screenHeight/2 }, 30, 10);
circleA->restitution = 0;
PhysicsBody circleB = CreatePhysicsBodyCircle((Vector2){ screenWidth*0.5f, screenHeight/2 }, 30, 10);
circleB->restitution = 0.5f;
PhysicsBody circleC = CreatePhysicsBodyCircle((Vector2){ screenWidth*0.75f, screenHeight/2 }, 30, 10);
circleC->restitution = 1;
SetPhysicsTimeStep(1.0/60.0/100*1000);
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
RunPhysicsStep();
if (IsKeyPressed('R')) 
{
circleA->position = (Vector2){ screenWidth*0.25f, screenHeight/2 };
circleA->velocity = (Vector2){ 0, 0 };
circleB->position = (Vector2){ screenWidth*0.5f, screenHeight/2 };
circleB->velocity = (Vector2){ 0, 0 };
circleC->position = (Vector2){ screenWidth*0.75f, screenHeight/2 };
circleC->velocity = (Vector2){ 0, 0 };
}
BeginDrawing();
ClearBackground(BLACK);
DrawFPS(screenWidth - 90, screenHeight - 30);
int bodiesCount = GetPhysicsBodiesCount();
for (int i = 0; i < bodiesCount; i++)
{
PhysicsBody body = GetPhysicsBody(i);
int vertexCount = GetPhysicsShapeVerticesCount(i);
for (int j = 0; j < vertexCount; j++)
{
Vector2 vertexA = GetPhysicsShapeVertex(body, j);
int jj = (((j + 1) < vertexCount) ? (j + 1) : 0); 
Vector2 vertexB = GetPhysicsShapeVertex(body, jj);
DrawLineV(vertexA, vertexB, GREEN); 
}
}
DrawText("Restitution amount", (screenWidth - MeasureText("Restitution amount", 30))/2, 75, 30, WHITE);
DrawText("0", circleA->position.x - MeasureText("0", 20)/2, circleA->position.y - 7, 20, WHITE);
DrawText("0.5", circleB->position.x - MeasureText("0.5", 20)/2, circleB->position.y - 7, 20, WHITE);
DrawText("1", circleC->position.x - MeasureText("1", 20)/2, circleC->position.y - 7, 20, WHITE);
DrawText("Press 'R' to reset example", 10, 10, 10, WHITE);
DrawText("Physac", logoX, logoY, 30, WHITE);
DrawText("Powered by", logoX + 50, logoY - 7, 10, WHITE);
EndDrawing();
}
DestroyPhysicsBody(circleA);
DestroyPhysicsBody(circleB);
DestroyPhysicsBody(circleC);
DestroyPhysicsBody(floor);
ClosePhysics(); 
CloseWindow(); 
return 0;
}
