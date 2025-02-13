#include "raylib.h"
#include "screens/screens.h" 
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
InitWindow(screenWidth, screenHeight, "raylib template - standard game");
currentScreen = LOGO; 
InitLogoScreen();
SetTargetFPS(60);
while (!WindowShouldClose()) 
{
switch(currentScreen) 
{
case LOGO: 
{
UpdateLogoScreen();
if (FinishLogoScreen())
{
UnloadLogoScreen();
currentScreen = TITLE;
InitTitleScreen();
}
} break;
case TITLE: 
{
UpdateTitleScreen();
if (FinishTitleScreen() == 1)
{
UnloadTitleScreen();
currentScreen = OPTIONS;
InitOptionsScreen();
}
else if (FinishTitleScreen() == 2)
{
UnloadTitleScreen();
currentScreen = GAMEPLAY;
InitGameplayScreen();
}
} break;
case OPTIONS:
{
UpdateOptionsScreen();
if (FinishOptionsScreen())
{
UnloadOptionsScreen();
currentScreen = TITLE;
InitTitleScreen();
} 
} break;
case GAMEPLAY:
{ 
UpdateGameplayScreen();
if (FinishGameplayScreen())
{
UnloadGameplayScreen();
currentScreen = ENDING;
InitEndingScreen();
} 
} break;
case ENDING: 
{
UpdateEndingScreen();
if (FinishEndingScreen())
{
UnloadEndingScreen();
currentScreen = TITLE;
InitTitleScreen();
} 
} break;
default: break;
}
BeginDrawing();
ClearBackground(RAYWHITE);
switch(currentScreen) 
{
case LOGO: DrawLogoScreen(); break;
case TITLE: DrawTitleScreen(); break;
case OPTIONS: DrawOptionsScreen(); break;
case GAMEPLAY: DrawGameplayScreen(); break;
case ENDING: DrawEndingScreen(); break;
default: break;
}
EndDrawing();
}
CloseWindow(); 
return 0;
}
