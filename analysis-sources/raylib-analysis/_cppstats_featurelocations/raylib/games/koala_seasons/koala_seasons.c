













#include "raylib.h"
#include "screens/screens.h" 

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif




static const int screenWidth = 1280;
static const int screenHeight = 720;

static float transAlpha = 0;
static bool onTransition = false;
static bool transFadeOut = false;
static int transFromScreen = -1;
static int transToScreen = -1;
static int framesCounter = 0;

static Music music;




void TransitionToScreen(int screen);
void UpdateTransition(void);
void DrawTransition(void);

void UpdateDrawFrame(void); 




int main(void) 
{


InitWindow(screenWidth, screenHeight, "KOALA SEASONS");


font = LoadFont("resources/graphics/mainfont.png");

atlas01 = LoadTexture("resources/graphics/atlas01.png");
atlas02 = LoadTexture("resources/graphics/atlas02.png");

#if defined(PLATFORM_WEB) || defined(PLATFORM_RPI) || defined(PLATFORM_ANDROID)
colorBlend = LoadShader(0, "resources/shaders/glsl100/blend_color.fs");
#else
colorBlend = LoadShader(0, "resources/shaders/glsl330/blend_color.fs");
#endif

InitAudioDevice();


fxJump = LoadSound("resources/audio/jump.ogg");
fxDash = LoadSound("resources/audio/dash.ogg");
fxEatLeaves = LoadSound("resources/audio/eat_leaves.ogg");
fxHitResin = LoadSound("resources/audio/resin_hit.ogg");
fxWind = LoadSound("resources/audio/wind_sound.ogg");
fxDieSnake = LoadSound("resources/audio/snake_die.ogg");
fxDieDingo = LoadSound("resources/audio/dingo_die.ogg");
fxDieOwl = LoadSound("resources/audio/owl_die.ogg");

music = LoadMusicStream("resources/audio/jngl.xm");
PlayMusicStream(music);
SetMusicVolume(music, 2.0f);



currentScreen = TITLE;
InitTitleScreen();

#if defined(PLATFORM_WEB)
emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
SetTargetFPS(60); 



while (!WindowShouldClose()) UpdateDrawFrame();
#endif



UnloadEndingScreen();
UnloadTitleScreen();
UnloadGameplayScreen();
UnloadLogoScreen();

UnloadTexture(atlas01);
UnloadTexture(atlas02);
UnloadFont(font);

UnloadShader(colorBlend); 

UnloadSound(fxJump);
UnloadSound(fxDash);
UnloadSound(fxEatLeaves);
UnloadSound(fxHitResin);
UnloadSound(fxWind);
UnloadSound(fxDieSnake);
UnloadSound(fxDieDingo);
UnloadSound(fxDieOwl);

UnloadMusicStream(music);

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

void UpdateTransition(void)
{
if (!transFadeOut)
{
transAlpha += 0.05f;

if (transAlpha >= 1.0)
{
transAlpha = 1.0;
currentScreen = transToScreen;
transFadeOut = true;
framesCounter = 0;
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
switch (currentScreen)
{
case LOGO:
{
UpdateLogoScreen();

if (FinishLogoScreen()) TransitionToScreen(TITLE);

} break;
case TITLE:
{
UpdateTitleScreen();


if (FinishTitleScreen() == 1)
{
UnloadTitleScreen();


}
else if (FinishTitleScreen() == 2)
{
UnloadTitleScreen();

InitGameplayScreen();
TransitionToScreen(GAMEPLAY);
}
} break;
case GAMEPLAY:
{
UpdateGameplayScreen();

if (FinishGameplayScreen())
{
UnloadGameplayScreen();

InitEndingScreen();
TransitionToScreen(ENDING); 
}
} break;
case ENDING:
{
UpdateEndingScreen();

if (FinishEndingScreen())
{
UnloadEndingScreen();

InitGameplayScreen();
TransitionToScreen(GAMEPLAY); 
}
} break;
default: break;
}
}
else UpdateTransition();

UpdateMusicStream(music);




BeginDrawing();

ClearBackground(WHITE);

switch (currentScreen)
{
case LOGO: DrawLogoScreen(); break;
case TITLE: DrawTitleScreen(); break;
case GAMEPLAY: DrawGameplayScreen(); break;
case ENDING: DrawEndingScreen(); break;
default: break;
}

if (onTransition) DrawTransition();

DrawRectangle(GetScreenWidth() - 200, GetScreenHeight() - 50, 200, 40, Fade(WHITE, 0.6f));
DrawText("ALPHA VERSION", GetScreenWidth() - 180, GetScreenHeight() - 40, 20, DARKGRAY);

EndDrawing();

}
