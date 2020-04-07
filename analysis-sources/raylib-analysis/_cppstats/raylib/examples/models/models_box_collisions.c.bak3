










#include "raylib.h"

int main(void)
{


const int screenWidth = 800;
const int screenHeight = 450;

InitWindow(screenWidth, screenHeight, "raylib [models] example - box collisions");


Camera camera = { { 0.0f, 10.0f, 10.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 45.0f, 0 };

Vector3 playerPosition = { 0.0f, 1.0f, 2.0f };
Vector3 playerSize = { 1.0f, 2.0f, 1.0f };
Color playerColor = GREEN;

Vector3 enemyBoxPos = { -4.0f, 1.0f, 0.0f };
Vector3 enemyBoxSize = { 2.0f, 2.0f, 2.0f };

Vector3 enemySpherePos = { 4.0f, 0.0f, 0.0f };
float enemySphereSize = 1.5f;

bool collision = false;

SetTargetFPS(60); 



while (!WindowShouldClose()) 
{




if (IsKeyDown(KEY_RIGHT)) playerPosition.x += 0.2f;
else if (IsKeyDown(KEY_LEFT)) playerPosition.x -= 0.2f;
else if (IsKeyDown(KEY_DOWN)) playerPosition.z += 0.2f;
else if (IsKeyDown(KEY_UP)) playerPosition.z -= 0.2f;

collision = false;


if (CheckCollisionBoxes(
(BoundingBox){(Vector3){ playerPosition.x - playerSize.x/2,
playerPosition.y - playerSize.y/2,
playerPosition.z - playerSize.z/2 },
(Vector3){ playerPosition.x + playerSize.x/2,
playerPosition.y + playerSize.y/2,
playerPosition.z + playerSize.z/2 }},
(BoundingBox){(Vector3){ enemyBoxPos.x - enemyBoxSize.x/2,
enemyBoxPos.y - enemyBoxSize.y/2,
enemyBoxPos.z - enemyBoxSize.z/2 },
(Vector3){ enemyBoxPos.x + enemyBoxSize.x/2,
enemyBoxPos.y + enemyBoxSize.y/2,
enemyBoxPos.z + enemyBoxSize.z/2 }})) collision = true;


if (CheckCollisionBoxSphere(
(BoundingBox){(Vector3){ playerPosition.x - playerSize.x/2,
playerPosition.y - playerSize.y/2,
playerPosition.z - playerSize.z/2 },
(Vector3){ playerPosition.x + playerSize.x/2,
playerPosition.y + playerSize.y/2,
playerPosition.z + playerSize.z/2 }},
enemySpherePos, enemySphereSize)) collision = true;

if (collision) playerColor = RED;
else playerColor = GREEN;




BeginDrawing();

ClearBackground(RAYWHITE);

BeginMode3D(camera);


DrawCube(enemyBoxPos, enemyBoxSize.x, enemyBoxSize.y, enemyBoxSize.z, GRAY);
DrawCubeWires(enemyBoxPos, enemyBoxSize.x, enemyBoxSize.y, enemyBoxSize.z, DARKGRAY);


DrawSphere(enemySpherePos, enemySphereSize, GRAY);
DrawSphereWires(enemySpherePos, enemySphereSize, 16, 16, DARKGRAY);


DrawCubeV(playerPosition, playerSize, playerColor);

DrawGrid(10, 1.0f); 

EndMode3D();

DrawText("Move player with cursors to collide", 220, 40, 20, GRAY);

DrawFPS(10, 10);

EndDrawing();

}



CloseWindow(); 


return 0;
}