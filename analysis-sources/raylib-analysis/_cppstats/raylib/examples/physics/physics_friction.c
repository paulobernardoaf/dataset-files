#include "raylib.h"
#define PHYSAC_IMPLEMENTATION
#define PHYSAC_NO_THREADS
#include "physac.h"
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
SetConfigFlags(FLAG_MSAA_4X_HINT);
InitWindow(screenWidth, screenHeight, "Physac [raylib] - Physics friction");
int logoX = screenWidth - MeasureText("Physac", 30) - 10;
int logoY = 15;
InitPhysics();
PhysicsBody floor = CreatePhysicsBodyRectangle((Vector2){ screenWidth/2, screenHeight }, screenWidth, 100, 10);
floor->enabled = false; 
PhysicsBody wall = CreatePhysicsBodyRectangle((Vector2){ screenWidth/2, screenHeight*0.8f }, 10, 80, 10);
wall->enabled = false; 
PhysicsBody rectLeft = CreatePhysicsBodyRectangle((Vector2){ 25, screenHeight - 5 }, 250, 250, 10);
rectLeft->enabled = false; 
SetPhysicsBodyRotation(rectLeft, 30*DEG2RAD);
PhysicsBody rectRight = CreatePhysicsBodyRectangle((Vector2){ screenWidth - 25, screenHeight - 5 }, 250, 250, 10);
rectRight->enabled = false; 
SetPhysicsBodyRotation(rectRight, 330*DEG2RAD);
PhysicsBody bodyA = CreatePhysicsBodyRectangle((Vector2){ 35, screenHeight*0.6f }, 40, 40, 10);
bodyA->staticFriction = 0.1f;
bodyA->dynamicFriction = 0.1f;
SetPhysicsBodyRotation(bodyA, 30*DEG2RAD);
PhysicsBody bodyB = CreatePhysicsBodyRectangle((Vector2){ screenWidth - 35, screenHeight*0.6f }, 40, 40, 10);
bodyB->staticFriction = 1.0f;
bodyB->dynamicFriction = 1.0f;
SetPhysicsBodyRotation(bodyB, 330*DEG2RAD);
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
RunPhysicsStep();
if (IsKeyPressed('R')) 
{
bodyA->position = (Vector2){ 35, screenHeight*0.6f };
bodyA->velocity = (Vector2){ 0, 0 };
bodyA->angularVelocity = 0;
SetPhysicsBodyRotation(bodyA, 30*DEG2RAD);
bodyB->position = (Vector2){ screenWidth - 35, screenHeight*0.6f };
bodyB->velocity = (Vector2){ 0, 0 };
bodyB->angularVelocity = 0;
SetPhysicsBodyRotation(bodyB, 330*DEG2RAD);
}
BeginDrawing();
ClearBackground(BLACK);
DrawFPS(screenWidth - 90, screenHeight - 30);
int bodiesCount = GetPhysicsBodiesCount();
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
DrawRectangle(0, screenHeight - 49, screenWidth, 49, BLACK);
DrawText("Friction amount", (screenWidth - MeasureText("Friction amount", 30))/2, 75, 30, WHITE);
DrawText("0.1", bodyA->position.x - MeasureText("0.1", 20)/2, bodyA->position.y - 7, 20, WHITE);
DrawText("1", bodyB->position.x - MeasureText("1", 20)/2, bodyB->position.y - 7, 20, WHITE);
DrawText("Press 'R' to reset example", 10, 10, 10, WHITE);
DrawText("Physac", logoX, logoY, 30, WHITE);
DrawText("Powered by", logoX + 50, logoY - 7, 10, WHITE);
EndDrawing();
}
ClosePhysics(); 
CloseWindow(); 
return 0;
}
