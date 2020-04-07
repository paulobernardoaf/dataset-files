#include "raylib.h"
#if defined(PLATFORM_RPI)
#define XBOX360_NAME_ID "Microsoft X-Box 360 pad"
#define PS3_NAME_ID "PLAYSTATION(R)3 Controller"
#else
#define XBOX360_NAME_ID "Xbox 360 Controller"
#define PS3_NAME_ID "PLAYSTATION(R)3 Controller"
#endif
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
SetConfigFlags(FLAG_MSAA_4X_HINT); 
InitWindow(screenWidth, screenHeight, "raylib [core] example - gamepad input");
Texture2D texPs3Pad = LoadTexture("resources/ps3.png");
Texture2D texXboxPad = LoadTexture("resources/xbox.png");
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
BeginDrawing();
ClearBackground(RAYWHITE);
if (IsGamepadAvailable(GAMEPAD_PLAYER1))
{
DrawText(FormatText("GP1: %s", GetGamepadName(GAMEPAD_PLAYER1)), 10, 10, 10, BLACK);
if (IsGamepadName(GAMEPAD_PLAYER1, XBOX360_NAME_ID))
{
DrawTexture(texXboxPad, 0, 0, DARKGRAY);
if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_BUTTON_MIDDLE)) DrawCircle(394, 89, 19, RED);
if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_BUTTON_MIDDLE_RIGHT)) DrawCircle(436, 150, 9, RED);
if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_BUTTON_MIDDLE_LEFT)) DrawCircle(352, 150, 9, RED);
if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_BUTTON_RIGHT_FACE_LEFT)) DrawCircle(501, 151, 15, BLUE);
if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) DrawCircle(536, 187, 15, LIME);
if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) DrawCircle(572, 151, 15, MAROON);
if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_BUTTON_RIGHT_FACE_UP)) DrawCircle(536, 115, 15, GOLD);
DrawRectangle(317, 202, 19, 71, BLACK);
DrawRectangle(293, 228, 69, 19, BLACK);
if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_BUTTON_LEFT_FACE_UP)) DrawRectangle(317, 202, 19, 26, RED);
if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_BUTTON_LEFT_FACE_DOWN)) DrawRectangle(317, 202 + 45, 19, 26, RED);
if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_BUTTON_LEFT_FACE_LEFT)) DrawRectangle(292, 228, 25, 19, RED);
if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) DrawRectangle(292 + 44, 228, 26, 19, RED);
if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_BUTTON_LEFT_TRIGGER_1)) DrawCircle(259, 61, 20, RED);
if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_BUTTON_RIGHT_TRIGGER_1)) DrawCircle(536, 61, 20, RED);
DrawCircle(259, 152, 39, BLACK);
DrawCircle(259, 152, 34, LIGHTGRAY);
DrawCircle(259 + (GetGamepadAxisMovement(GAMEPAD_PLAYER1, GAMEPAD_AXIS_LEFT_X)*20),
152 - (GetGamepadAxisMovement(GAMEPAD_PLAYER1, GAMEPAD_AXIS_LEFT_Y)*20), 25, BLACK);
DrawCircle(461, 237, 38, BLACK);
DrawCircle(461, 237, 33, LIGHTGRAY);
DrawCircle(461 + (GetGamepadAxisMovement(GAMEPAD_PLAYER1, GAMEPAD_AXIS_RIGHT_X)*20),
237 - (GetGamepadAxisMovement(GAMEPAD_PLAYER1, GAMEPAD_AXIS_RIGHT_Y)*20), 25, BLACK);
DrawRectangle(170, 30, 15, 70, GRAY);
DrawRectangle(604, 30, 15, 70, GRAY);
DrawRectangle(170, 30, 15, (((1.0f + GetGamepadAxisMovement(GAMEPAD_PLAYER1, GAMEPAD_AXIS_LEFT_TRIGGER))/2.0f)*70), RED);
DrawRectangle(604, 30, 15, (((1.0f + GetGamepadAxisMovement(GAMEPAD_PLAYER1, GAMEPAD_AXIS_RIGHT_TRIGGER))/2.0f)*70), RED);
}
else if (IsGamepadName(GAMEPAD_PLAYER1, PS3_NAME_ID))
{
DrawTexture(texPs3Pad, 0, 0, DARKGRAY);
if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_BUTTON_MIDDLE)) DrawCircle(396, 222, 13, RED);
if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_BUTTON_MIDDLE_LEFT)) DrawRectangle(328, 170, 32, 13, RED);
if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_BUTTON_MIDDLE_RIGHT)) DrawTriangle((Vector2){ 436, 168 }, (Vector2){ 436, 185 }, (Vector2){ 464, 177 }, RED);
if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_BUTTON_RIGHT_FACE_UP)) DrawCircle(557, 144, 13, LIME);
if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) DrawCircle(586, 173, 13, RED);
if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) DrawCircle(557, 203, 13, VIOLET);
if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_BUTTON_RIGHT_FACE_LEFT)) DrawCircle(527, 173, 13, PINK);
DrawRectangle(225, 132, 24, 84, BLACK);
DrawRectangle(195, 161, 84, 25, BLACK);
if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_BUTTON_LEFT_FACE_UP)) DrawRectangle(225, 132, 24, 29, RED);
if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_BUTTON_LEFT_FACE_DOWN)) DrawRectangle(225, 132 + 54, 24, 30, RED);
if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_BUTTON_LEFT_FACE_LEFT)) DrawRectangle(195, 161, 30, 25, RED);
if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) DrawRectangle(195 + 54, 161, 30, 25, RED);
if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_BUTTON_LEFT_TRIGGER_1)) DrawCircle(239, 82, 20, RED);
if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_BUTTON_RIGHT_TRIGGER_1)) DrawCircle(557, 82, 20, RED);
DrawCircle(319, 255, 35, BLACK);
DrawCircle(319, 255, 31, LIGHTGRAY);
DrawCircle(319 + (GetGamepadAxisMovement(GAMEPAD_PLAYER1, GAMEPAD_AXIS_LEFT_X)*20),
255 + (GetGamepadAxisMovement(GAMEPAD_PLAYER1, GAMEPAD_AXIS_LEFT_Y)*20), 25, BLACK);
DrawCircle(475, 255, 35, BLACK);
DrawCircle(475, 255, 31, LIGHTGRAY);
DrawCircle(475 + (GetGamepadAxisMovement(GAMEPAD_PLAYER1, GAMEPAD_AXIS_RIGHT_X)*20),
255 + (GetGamepadAxisMovement(GAMEPAD_PLAYER1, GAMEPAD_AXIS_RIGHT_Y)*20), 25, BLACK);
DrawRectangle(169, 48, 15, 70, GRAY);
DrawRectangle(611, 48, 15, 70, GRAY);
DrawRectangle(169, 48, 15, (((1.0f - GetGamepadAxisMovement(GAMEPAD_PLAYER1, GAMEPAD_AXIS_LEFT_TRIGGER))/2.0f)*70), RED);
DrawRectangle(611, 48, 15, (((1.0f - GetGamepadAxisMovement(GAMEPAD_PLAYER1, GAMEPAD_AXIS_RIGHT_TRIGGER))/2.0f)*70), RED);
}
else
{
DrawText("- GENERIC GAMEPAD -", 280, 180, 20, GRAY);
}
DrawText(FormatText("DETECTED AXIS [%i]:", GetGamepadAxisCount(GAMEPAD_PLAYER1)), 10, 50, 10, MAROON);
for (int i = 0; i < GetGamepadAxisCount(GAMEPAD_PLAYER1); i++)
{
DrawText(FormatText("AXIS %i: %.02f", i, GetGamepadAxisMovement(GAMEPAD_PLAYER1, i)), 20, 70 + 20*i, 10, DARKGRAY);
}
if (GetGamepadButtonPressed() != -1) DrawText(FormatText("DETECTED BUTTON: %i", GetGamepadButtonPressed()), 10, 430, 10, RED);
else DrawText("DETECTED BUTTON: NONE", 10, 430, 10, GRAY);
}
else
{
DrawText("GP1: NOT DETECTED", 10, 10, 10, GRAY);
DrawTexture(texXboxPad, 0, 0, LIGHTGRAY);
}
EndDrawing();
}
UnloadTexture(texPs3Pad);
UnloadTexture(texXboxPad);
CloseWindow(); 
return 0;
}