














#include "raylib.h"

int main(void)
{


const int screenWidth = 800;
const int screenHeight = 450;

InitWindow(screenWidth, screenHeight, "raylib [textures] example - N-patch drawing");


Texture2D nPatchTexture = LoadTexture("resources/ninepatch_button.png");

Vector2 mousePosition = { 0 };
Vector2 origin = { 0.0f, 0.0f };


Rectangle dstRec1 = { 480.0f, 160.0f, 32.0f, 32.0f };
Rectangle dstRec2 = { 160.0f, 160.0f, 32.0f, 32.0f };
Rectangle dstRecH = { 160.0f, 93.0f, 32.0f, 32.0f };
Rectangle dstRecV = { 92.0f, 160.0f, 32.0f, 32.0f };


NPatchInfo ninePatchInfo1 = { (Rectangle){ 0.0f, 0.0f, 64.0f, 64.0f }, 12, 40, 12, 12, NPT_9PATCH };
NPatchInfo ninePatchInfo2 = { (Rectangle){ 0.0f, 128.0f, 64.0f, 64.0f }, 16, 16, 16, 16, NPT_9PATCH };


NPatchInfo h3PatchInfo = { (Rectangle){ 0.0f, 64.0f, 64.0f, 64.0f }, 8, 8, 8, 8, NPT_3PATCH_HORIZONTAL };


NPatchInfo v3PatchInfo = { (Rectangle){ 0.0f, 192.0f, 64.0f, 64.0f }, 6, 6, 6, 6, NPT_3PATCH_VERTICAL };

SetTargetFPS(60);



while (!WindowShouldClose()) 
{


mousePosition = GetMousePosition();


dstRec1.width = mousePosition.x - dstRec1.x;
dstRec1.height = mousePosition.y - dstRec1.y;
dstRec2.width = mousePosition.x - dstRec2.x;
dstRec2.height = mousePosition.y - dstRec2.y;
dstRecH.width = mousePosition.x - dstRecH.x;
dstRecV.height = mousePosition.y - dstRecV.y;


if (dstRec1.width < 1.0f) dstRec1.width = 1.0f;
if (dstRec1.width > 300.0f) dstRec1.width = 300.0f;
if (dstRec1.height < 1.0f) dstRec1.height = 1.0f;
if (dstRec2.width < 1.0f) dstRec2.width = 1.0f;
if (dstRec2.width > 300.0f) dstRec2.width = 300.0f;
if (dstRec2.height < 1.0f) dstRec2.height = 1.0f;
if (dstRecH.width < 1.0f) dstRecH.width = 1.0f;
if (dstRecV.height < 1.0f) dstRecV.height = 1.0f;




BeginDrawing();

ClearBackground(RAYWHITE);


DrawTextureNPatch(nPatchTexture, ninePatchInfo2, dstRec2, origin, 0.0f, WHITE);
DrawTextureNPatch(nPatchTexture, ninePatchInfo1, dstRec1, origin, 0.0f, WHITE);
DrawTextureNPatch(nPatchTexture, h3PatchInfo, dstRecH, origin, 0.0f, WHITE);
DrawTextureNPatch(nPatchTexture, v3PatchInfo, dstRecV, origin, 0.0f, WHITE);


DrawRectangleLines(5, 88, 74, 266, BLUE);
DrawTexture(nPatchTexture, 10, 93, WHITE);
DrawText("TEXTURE", 15, 360, 10, DARKGRAY);

DrawText("Move the mouse to stretch or shrink the n-patches", 10, 20, 20, DARKGRAY);

EndDrawing();

}



UnloadTexture(nPatchTexture); 

CloseWindow(); 


return 0;
}
