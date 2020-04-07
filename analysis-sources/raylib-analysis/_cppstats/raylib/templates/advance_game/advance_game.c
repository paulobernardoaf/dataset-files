#include "raylib.h"
#include "screens/screens.h" 
#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif
GameScreen currentScreen = 0;
Font font = { 0 };
Music music = { 0 };
Sound fxCoin = { 0 };
const int screenWidth = 800;
const int screenHeight = 450;
static float transAlpha = 0.0f;
static bool onTransition = false;
static bool transFadeOut = false;
static int transFromScreen = -1;
static int transToScreen = -1;
static void ChangeToScreen(int screen); 
static void TransitionToScreen(int screen);
static void UpdateTransition(void);
static void DrawTransition(void);
static void UpdateDrawFrame(void); 
int main(void)
{
InitWindow(screenWidth, screenHeight, "raylib template - advance game");
InitAudioDevice();
font = LoadFont("resources/mecha.png");
music = LoadMusicStream("resources/ambient.ogg");
fxCoin = LoadSound("resources/coin.wav");
SetMusicVolume(music, 1.0f);
PlayMusicStream(music);
currentScreen = LOGO;
InitLogoScreen();
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
case LOGO: UnloadLogoScreen(); break;
case TITLE: UnloadTitleScreen(); break;
case GAMEPLAY: UnloadGameplayScreen(); break;
case ENDING: UnloadEndingScreen(); break;
default: break;
}
UnloadFont(font);
UnloadMusicStream(music);
UnloadSound(fxCoin);
CloseAudioDevice(); 
CloseWindow(); 
return 0;
}
static void ChangeToScreen(int screen)
{
switch (currentScreen)
{
case LOGO: UnloadLogoScreen(); break;
case TITLE: UnloadTitleScreen(); break;
case GAMEPLAY: UnloadGameplayScreen(); break;
case ENDING: UnloadEndingScreen(); break;
default: break;
}
switch (screen)
{
case LOGO: InitLogoScreen(); break;
case TITLE: InitTitleScreen(); break;
case GAMEPLAY: InitGameplayScreen(); break;
case ENDING: InitEndingScreen(); break;
default: break;
}
currentScreen = screen;
}
static void TransitionToScreen(int screen)
{
onTransition = true;
transFadeOut = false;
transFromScreen = currentScreen;
transToScreen = screen;
transAlpha = 0.0f;
}
static void UpdateTransition(void)
{
if (!transFadeOut)
{
transAlpha += 0.05f;
if (transAlpha > 1.01f)
{
transAlpha = 1.0f;
switch (transFromScreen)
{
case LOGO: UnloadLogoScreen(); break;
case TITLE: UnloadTitleScreen(); break;
case OPTIONS: UnloadOptionsScreen(); break;
case GAMEPLAY: UnloadGameplayScreen(); break;
case ENDING: UnloadEndingScreen(); break;
default: break;
}
switch (transToScreen)
{
case LOGO: InitLogoScreen(); break;
case TITLE: InitTitleScreen(); break;
case GAMEPLAY: InitGameplayScreen(); break;
case ENDING: InitEndingScreen(); break;
default: break;
}
currentScreen = transToScreen;
transFadeOut = true;
}
}
else 
{
transAlpha -= 0.02f;
if (transAlpha < -0.01f)
{
transAlpha = 0.0f;
transFadeOut = false;
onTransition = false;
transFromScreen = -1;
transToScreen = -1;
}
}
}
static void DrawTransition(void)
{
DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, transAlpha));
}
static void UpdateDrawFrame(void)
{
UpdateMusicStream(music); 
if (!onTransition)
{
switch(currentScreen) 
{
case LOGO: 
{
UpdateLogoScreen();
if (FinishLogoScreen()) TransitionToScreen(TITLE);
} break;
case TITLE: 
{
UpdateTitleScreen();
if (FinishTitleScreen() == 1) TransitionToScreen(OPTIONS);
else if (FinishTitleScreen() == 2) TransitionToScreen(GAMEPLAY);
} break;
case OPTIONS:
{
UpdateOptionsScreen();
if (FinishOptionsScreen()) TransitionToScreen(TITLE);
} break;
case GAMEPLAY:
{
UpdateGameplayScreen();
if (FinishGameplayScreen() == 1) TransitionToScreen(ENDING);
} break;
case ENDING:
{ 
UpdateEndingScreen();
if (FinishEndingScreen() == 1) TransitionToScreen(TITLE);
} break;
default: break;
}
}
else UpdateTransition(); 
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
if (onTransition) DrawTransition();
EndDrawing();
}
