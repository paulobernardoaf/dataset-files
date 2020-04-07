
















#include "raylib.h"

#define PHYSAC_IMPLEMENTATION
#define PHYSAC_NO_THREADS
#include "physac.h"

int main(void)
{


const int screenWidth = 800;
const int screenHeight = 450;

SetConfigFlags(FLAG_MSAA_4X_HINT);
InitWindow(screenWidth, screenHeight, "Physac [raylib] - Body shatter");


int logoX = screenWidth - MeasureText("Physac", 30) - 10;
int logoY = 15;
bool needsReset = false;


InitPhysics();
SetPhysicsGravity(0, 0);


CreatePhysicsBodyPolygon((Vector2){ screenWidth/2, screenHeight/2 }, GetRandomValue(80, 200), GetRandomValue(3, 8), 10);

SetTargetFPS(60); 



while (!WindowShouldClose()) 
{

RunPhysicsStep();



if (needsReset)
{

CreatePhysicsBodyPolygon((Vector2){ screenWidth/2, screenHeight/2 }, GetRandomValue(80, 200), GetRandomValue(3, 8), 10);
needsReset = false;
}

if (IsKeyPressed('R')) 
{
ResetPhysics();
needsReset = true;
}

if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) 
{

int count = GetPhysicsBodiesCount();
for (int i = count - 1; i >= 0; i--)
{
PhysicsBody currentBody = GetPhysicsBody(i);
if (currentBody != NULL) PhysicsShatter(currentBody, GetMousePosition(), 10/currentBody->inverseMass);
}
}




BeginDrawing();

ClearBackground(BLACK);


int bodiesCount = GetPhysicsBodiesCount();
for (int i = 0; i < bodiesCount; i++)
{
PhysicsBody currentBody = GetPhysicsBody(i);

int vertexCount = GetPhysicsShapeVerticesCount(i);
for (int j = 0; j < vertexCount; j++)
{


Vector2 vertexA = GetPhysicsShapeVertex(currentBody, j);

int jj = (((j + 1) < vertexCount) ? (j + 1) : 0); 
Vector2 vertexB = GetPhysicsShapeVertex(currentBody, jj);

DrawLineV(vertexA, vertexB, GREEN); 
}
}

DrawText("Left mouse button in polygon area to shatter body\nPress 'R' to reset example", 10, 10, 10, WHITE);

DrawText("Physac", logoX, logoY, 30, WHITE);
DrawText("Powered by", logoX + 50, logoY - 7, 10, WHITE);

EndDrawing();

}



ClosePhysics(); 

CloseWindow(); 


return 0;
}
