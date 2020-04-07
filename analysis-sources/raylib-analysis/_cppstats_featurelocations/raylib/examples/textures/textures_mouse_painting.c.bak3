












#include "raylib.h"

#define MAX_COLORS_COUNT 23 

int main(void) 
{


const int screenWidth = 800;
const int screenHeight = 450;

InitWindow(screenWidth, screenHeight, "raylib [textures] example - mouse painting");


Color colors[MAX_COLORS_COUNT] = {
RAYWHITE, YELLOW, GOLD, ORANGE, PINK, RED, MAROON, GREEN, LIME, DARKGREEN,
SKYBLUE, BLUE, DARKBLUE, PURPLE, VIOLET, DARKPURPLE, BEIGE, BROWN, DARKBROWN,
LIGHTGRAY, GRAY, DARKGRAY, BLACK };


Rectangle colorsRecs[MAX_COLORS_COUNT] = { 0 };

for (int i = 0; i < MAX_COLORS_COUNT; i++)
{
colorsRecs[i].x = 10 + 30*i + 2*i;
colorsRecs[i].y = 10;
colorsRecs[i].width = 30;
colorsRecs[i].height = 30;
}

int colorSelected = 0;
int colorSelectedPrev = colorSelected;
int colorMouseHover = 0;
int brushSize = 20;

Rectangle btnSaveRec = { 750, 10, 40, 30 };
bool btnSaveMouseHover = false;
bool showSaveMessage = false;
int saveMessageCounter = 0;


RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);


BeginTextureMode(target);
ClearBackground(colors[0]);
EndTextureMode();

SetTargetFPS(120); 



while (!WindowShouldClose()) 
{


Vector2 mousePos = GetMousePosition();


if (IsKeyPressed(KEY_RIGHT)) colorSelected++;
else if (IsKeyPressed(KEY_LEFT)) colorSelected--;

if (colorSelected >= MAX_COLORS_COUNT) colorSelected = MAX_COLORS_COUNT - 1;
else if (colorSelected < 0) colorSelected = 0;


for (int i = 0; i < MAX_COLORS_COUNT; i++)
{
if (CheckCollisionPointRec(mousePos, colorsRecs[i]))
{
colorMouseHover = i;
break;
}
else colorMouseHover = -1;
}

if ((colorMouseHover >= 0) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) 
{
colorSelected = colorMouseHover;
colorSelectedPrev = colorSelected;
}


brushSize += GetMouseWheelMove()*5;
if (brushSize < 2) brushSize = 2;
if (brushSize > 50) brushSize = 50;

if (IsKeyPressed(KEY_C)) 
{

BeginTextureMode(target);
ClearBackground(colors[0]);
EndTextureMode();
}

if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) || (GetGestureDetected() == GESTURE_DRAG))
{



BeginTextureMode(target);
if (mousePos.y > 50) DrawCircle(mousePos.x, mousePos.y, brushSize, colors[colorSelected]);
EndTextureMode();
}

if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
{
colorSelected = 0;


BeginTextureMode(target);
if (mousePos.y > 50) DrawCircle(mousePos.x, mousePos.y, brushSize, colors[0]);
EndTextureMode();
}
else colorSelected = colorSelectedPrev;


if (CheckCollisionPointRec(mousePos, btnSaveRec)) btnSaveMouseHover = true;
else btnSaveMouseHover = false;



if ((btnSaveMouseHover && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) || IsKeyPressed(KEY_S))
{
Image image = GetTextureData(target.texture);
ImageFlipVertical(&image);
ExportImage(image, "my_amazing_texture_painting.png");
UnloadImage(image);
showSaveMessage = true;
}

if (showSaveMessage)
{

saveMessageCounter++;
if (saveMessageCounter > 240)
{
showSaveMessage = false;
saveMessageCounter = 0;
}
}




BeginDrawing();

ClearBackground(RAYWHITE);


DrawTextureRec(target.texture, (Rectangle){ 0, 0, target.texture.width, -target.texture.height }, (Vector2){ 0, 0 }, WHITE);


if (mousePos.y > 50) 
{
if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) DrawCircleLines(mousePos.x, mousePos.y, brushSize, GRAY);
else DrawCircle(GetMouseX(), GetMouseY(), brushSize, colors[colorSelected]);
}


DrawRectangle(0, 0, GetScreenWidth(), 50, RAYWHITE);
DrawLine(0, 50, GetScreenWidth(), 50, LIGHTGRAY);


for (int i = 0; i < MAX_COLORS_COUNT; i++) DrawRectangleRec(colorsRecs[i], colors[i]);
DrawRectangleLines(10, 10, 30, 30, LIGHTGRAY);

if (colorMouseHover >= 0) DrawRectangleRec(colorsRecs[colorMouseHover], Fade(WHITE, 0.6f));

DrawRectangleLinesEx((Rectangle){ colorsRecs[colorSelected].x - 2, colorsRecs[colorSelected].y - 2, 
colorsRecs[colorSelected].width + 4, colorsRecs[colorSelected].height + 4 }, 2, BLACK);


DrawRectangleLinesEx(btnSaveRec, 2, btnSaveMouseHover? RED : BLACK);
DrawText("SAVE!", 755, 20, 10, btnSaveMouseHover? RED : BLACK);


if (showSaveMessage)
{
DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(RAYWHITE, 0.8f));
DrawRectangle(0, 150, GetScreenWidth(), 80, BLACK);
DrawText("IMAGE SAVED: my_amazing_texture_painting.png", 150, 180, 20, RAYWHITE);
}

EndDrawing();

}



UnloadRenderTexture(target); 

CloseWindow(); 


return 0;
}
