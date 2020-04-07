#include "raylib.h"
#define PHYSAC_IMPLEMENTATION
#define PHYSAC_NO_THREADS
#include "physac.h"
#define VELOCITY 0.5f
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
SetConfigFlags(FLAG_MSAA_4X_HINT);
InitWindow(screenWidth, screenHeight, "Physac [raylib] - Physics movement");
int logoX = screenWidth - MeasureText("Physac", 30) - 10;
int logoY = 15;
InitPhysics();
PhysicsBody floor = CreatePhysicsBodyRectangle((Vector2){ screenWidth/2, screenHeight }, screenWidth, 100, 10);
PhysicsBody platformLeft = CreatePhysicsBodyRectangle((Vector2){ screenWidth*0.25f, screenHeight*0.6f }, screenWidth*0.25f, 10, 10);
PhysicsBody platformRight = CreatePhysicsBodyRectangle((Vector2){ screenWidth*0.75f, screenHeight*0.6f }, screenWidth*0.25f, 10, 10);
PhysicsBody wallLeft = CreatePhysicsBodyRectangle((Vector2){ -5, screenHeight/2 }, 10, screenHeight, 10);
PhysicsBody wallRight = CreatePhysicsBodyRectangle((Vector2){ screenWidth + 5, screenHeight/2 }, 10, screenHeight, 10);
floor->enabled = false;
platformLeft->enabled = false;
platformRight->enabled = false;
wallLeft->enabled = false;
wallRight->enabled = false;
PhysicsBody body = CreatePhysicsBodyRectangle((Vector2){ screenWidth/2, screenHeight/2 }, 50, 50, 1);
body->freezeOrient = true; 
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
RunPhysicsStep();
if (IsKeyPressed('R')) 
{
body->position = (Vector2){ screenWidth/2, screenHeight/2 };
body->velocity = (Vector2){ 0, 0 };
SetPhysicsBodyRotation(body, 0);
}
if (IsKeyDown(KEY_RIGHT)) body->velocity.x = VELOCITY;
else if (IsKeyDown(KEY_LEFT)) body->velocity.x = -VELOCITY;
if (IsKeyDown(KEY_UP) && body->isGrounded) body->velocity.y = -VELOCITY*4;
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
DrawText("Use 'ARROWS' to move player", 10, 10, 10, WHITE);
DrawText("Press 'R' to reset example", 10, 30, 10, WHITE);
DrawText("Physac", logoX, logoY, 30, WHITE);
DrawText("Powered by", logoX + 50, logoY - 7, 10, WHITE);
EndDrawing();
}
ClosePhysics(); 
CloseWindow(); 
return 0;
}
