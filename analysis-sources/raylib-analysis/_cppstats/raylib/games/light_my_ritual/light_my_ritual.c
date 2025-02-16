#include "raylib.h"
#include "screens/screens.h" 
#include <stdlib.h>
#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif
const int screenWidth = 1280;
const int screenHeight = 720;
float transAlpha = 0;
bool onTransition = false;
bool transFadeOut = false;
int transFromScreen = -1;
int transToScreen = -1;
static Music music;
void TransitionToScreen(int screen);
void ChangeToScreen(int screen); 
void UpdateTransition(void);
void DrawTransition(void);
void UpdateDrawFrame(void); 
int main(void)
{
InitWindow(screenWidth, screenHeight, "LIGHT MY RITUAL! [GGJ16]");
InitAudioDevice();
Image image = LoadImage("resources/lights_map.png"); 
lightsMap = GetImageData(image); 
lightsMapWidth = image.width;
lightsMapHeight = image.height;
UnloadImage(image); 
font = LoadFont("resources/font_arcadian.png");
music = LoadMusicStream("resources/audio/ambient.ogg");
PlayMusicStream(music);
SetMusicVolume(music, 1.0f);
currentScreen = LOGO_RL;
rlInitLogoScreen();
#if defined(PLATFORM_WEB)
emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
UpdateDrawFrame();
}
#endif
switch (currentScreen)
{
case LOGO_RL: rlUnloadLogoScreen(); break;
case TITLE: UnloadTitleScreen(); break;
case GAMEPLAY: UnloadGameplayScreen(); break;
default: break;
}
UnloadFont(font);
UnloadMusicStream(music);
free(lightsMap);
CloseAudioDevice();
CloseWindow(); 
return 0;
}
void TransitionToScreen(int screen)
{
onTransition = true;
transFromScreen = currentScreen;
transToScreen = screen;
}
void ChangeToScreen(int screen)
{
switch (currentScreen)
{
case LOGO_RL: rlUnloadLogoScreen(); break;
case TITLE: UnloadTitleScreen(); break;
case GAMEPLAY: UnloadGameplayScreen(); break;
default: break;
}
switch (screen)
{
case LOGO_RL: rlInitLogoScreen(); break;
case TITLE: InitTitleScreen(); break;
case GAMEPLAY: InitGameplayScreen(); break;
default: break;
}
currentScreen = screen;
}
void UpdateTransition(void)
{
if (!transFadeOut)
{
transAlpha += 0.05f;
if (transAlpha >= 1.0)
{
transAlpha = 1.0;
switch (transFromScreen)
{
case LOGO_RL: rlUnloadLogoScreen(); break;
case TITLE: UnloadTitleScreen(); break;
case GAMEPLAY: UnloadGameplayScreen(); break;
default: break;
}
switch (transToScreen)
{
case LOGO_RL:
{
rlInitLogoScreen();
currentScreen = LOGO_RL;
} break;
case TITLE: 
{
InitTitleScreen();
currentScreen = TITLE; 
} break;
case GAMEPLAY:
{
InitGameplayScreen(); 
currentScreen = GAMEPLAY;
} break;
default: break;
}
transFadeOut = true;
}
}
else 
{
transAlpha -= 0.05f;
if (transAlpha <= 0)
{
transAlpha = 0;
transFadeOut = false;
onTransition = false;
transFromScreen = -1;
transToScreen = -1;
}
}
}
void DrawTransition(void)
{
DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, transAlpha));
}
void UpdateDrawFrame(void)
{
if (!onTransition)
{
switch(currentScreen) 
{
case LOGO_RL: 
{
rlUpdateLogoScreen();
if (rlFinishLogoScreen()) TransitionToScreen(TITLE);
} break;
case TITLE: 
{
UpdateTitleScreen();
if (FinishTitleScreen() == 1)
{
StopMusicStream(music);
TransitionToScreen(GAMEPLAY);
}
} break;
case GAMEPLAY:
{ 
UpdateGameplayScreen();
if (FinishGameplayScreen() == 1) ChangeToScreen(LOGO_RL);
else if (FinishGameplayScreen() == 2) TransitionToScreen(TITLE);
} break;
default: break;
}
}
else
{
UpdateTransition();
}
if (currentScreen != GAMEPLAY) UpdateMusicStream(music);
BeginDrawing();
ClearBackground(RAYWHITE);
switch(currentScreen) 
{
case LOGO_RL: rlDrawLogoScreen(); break;
case TITLE: DrawTitleScreen(); break;
case GAMEPLAY: DrawGameplayScreen(); break;
default: break;
}
if (onTransition) DrawTransition();
EndDrawing();
}
