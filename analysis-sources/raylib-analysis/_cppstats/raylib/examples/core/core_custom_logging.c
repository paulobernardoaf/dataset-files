#include "raylib.h"
#include <stdio.h> 
#include <time.h> 
void LogCustom(int msgType, const char *text, va_list args)
{
char timeStr[64] = { 0 };
time_t now = time(NULL);
struct tm *tm_info = localtime(&now);
strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tm_info);
printf("[%s] ", timeStr);
switch (msgType)
{
case LOG_INFO: printf("[INFO] : "); break;
case LOG_ERROR: printf("[ERROR]: "); break;
case LOG_WARNING: printf("[WARN] : "); break;
case LOG_DEBUG: printf("[DEBUG]: "); break;
default: break;
}
vprintf(text, args);
printf("\n");
}
int main(int argc, char* argv[])
{
const int screenWidth = 800;
const int screenHeight = 450;
SetTraceLogCallback(LogCustom);
InitWindow(screenWidth, screenHeight, "raylib [core] example - custom logging");
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
BeginDrawing();
ClearBackground(RAYWHITE);
DrawText("Check out the console output to see the custom logger in action!", 60, 200, 20, LIGHTGRAY);
EndDrawing();
}
CloseWindow(); 
return 0;
}
