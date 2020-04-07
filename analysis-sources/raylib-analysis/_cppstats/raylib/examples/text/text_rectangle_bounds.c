#include "raylib.h"
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
InitWindow(screenWidth, screenHeight, "raylib [text] example - draw text inside a rectangle");
const char text[] = "Text cannot escape\tthis container\t...word wrap also works when active so here's \
a long text for testing.\n\nLorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod \
tempor incididunt ut labore et dolore magna aliqua. Nec ullamcorper sit amet risus nullam eget felis eget.";
bool resizing = false;
bool wordWrap = true;
Rectangle container = { 25, 25, screenWidth - 50, screenHeight - 250};
Rectangle resizer = { container.x + container.width - 17, container.y + container.height - 17, 14, 14 };
const int minWidth = 60;
const int minHeight = 60;
const int maxWidth = screenWidth - 50;
const int maxHeight = screenHeight - 160;
Vector2 lastMouse = { 0.0f, 0.0f }; 
Color borderColor = MAROON; 
Font font = GetFontDefault(); 
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
if (IsKeyPressed(KEY_SPACE)) wordWrap = !wordWrap;
Vector2 mouse = GetMousePosition();
if (CheckCollisionPointRec(mouse, container)) borderColor = Fade(MAROON, 0.4f);
else if (!resizing) borderColor = MAROON;
if (resizing)
{
if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) resizing = false;
int width = container.width + (mouse.x - lastMouse.x);
container.width = (width > minWidth)? ((width < maxWidth)? width : maxWidth) : minWidth;
int height = container.height + (mouse.y - lastMouse.y);
container.height = (height > minHeight)? ((height < maxHeight)? height : maxHeight) : minHeight;
}
else
{
if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, resizer)) resizing = true;
}
resizer.x = container.x + container.width - 17;
resizer.y = container.y + container.height - 17;
lastMouse = mouse; 
BeginDrawing();
ClearBackground(RAYWHITE);
DrawRectangleLinesEx(container, 3, borderColor); 
DrawTextRec(font, text,
(Rectangle){ container.x + 4, container.y + 4, container.width - 4, container.height - 4 },
20.0f, 2.0f, wordWrap, GRAY);
DrawRectangleRec(resizer, borderColor); 
DrawRectangle(0, screenHeight - 54, screenWidth, 54, GRAY);
DrawRectangleRec((Rectangle){ 382, screenHeight - 34, 12, 12 }, MAROON);
DrawText("Word Wrap: ", 313, screenHeight-115, 20, BLACK);
if (wordWrap) DrawText("ON", 447, screenHeight - 115, 20, RED);
else DrawText("OFF", 447, screenHeight - 115, 20, BLACK);
DrawText("Press [SPACE] to toggle word wrap", 218, screenHeight - 86, 20, GRAY);
DrawText("Click hold & drag the to resize the container", 155, screenHeight - 38, 20, RAYWHITE);
EndDrawing();
}
CloseWindow(); 
return 0;
}