















#include "raylib.h"
#include "screens/screens.h" 

#include <stdlib.h>

#include <stdio.h> 
#include <string.h> 

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif




const int screenWidth = 1280;
const int screenHeight = 720;


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




int main(int argc, char *argv[])
{


#if defined(PLATFORM_DESKTOP)



if (argc > 1)
{


if ((IsFileExtension(argv[1], ".ogg")) ||
(IsFileExtension(argv[1], ".wav")))
{
if (sampleFilename != NULL) free(sampleFilename);

sampleFilename = (char *)malloc(256);
strcpy(sampleFilename, argv[1]);

printf("Custom audio file: %s", sampleFilename);
}
}
#endif

#if !defined(PLATFORM_ANDROID)
SetConfigFlags(FLAG_MSAA_4X_HINT);
#endif

InitWindow(screenWidth, screenHeight, "WAVE COLLECTOR [GGJ17]");


InitAudioDevice();

font = LoadFont("resources/font.fnt");
music = LoadMusicStream("resources/audio/wave.ogg");

SetMusicVolume(music, 1.0f);


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



if ((int)transAlpha >= 1)
{
transAlpha = 1.0f;


switch (transFromScreen)
{
case LOGO: UnloadLogoScreen(); break;
case TITLE: UnloadTitleScreen(); break;
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
transAlpha -= 0.05f;

if ((int)transAlpha <= 0)
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
DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(RAYWHITE, transAlpha));
}


static void UpdateDrawFrame(void)
{


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

if (FinishTitleScreen() == 1) TransitionToScreen(GAMEPLAY);

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


if (currentScreen != ENDING) UpdateMusicStream(music);




BeginDrawing();

ClearBackground(RAYWHITE);

switch(currentScreen) 
{
case LOGO: DrawLogoScreen(); break;
case TITLE: DrawTitleScreen(); break;
case GAMEPLAY: DrawGameplayScreen(); break;
case ENDING: DrawEndingScreen(); break;
default: break;
}


if (onTransition) DrawTransition();

EndDrawing();

}
