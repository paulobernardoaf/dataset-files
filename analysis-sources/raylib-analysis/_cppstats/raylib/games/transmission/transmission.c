#include "raylib.h"
#include "screens/screens.h" 
#include <stdlib.h>
#include <stdio.h>
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
int main(void)
{
InitWindow(screenWidth, screenHeight, "transmission mission [GGJ18]");
InitAudioDevice();
music = LoadMusicStream("resources/audio/music_title.ogg");
fxButton = LoadSound("resources/audio/fx_newspaper.ogg");
SetMusicVolume(music, 1.0f);
PlayMusicStream(music);
fontMission = LoadFontEx("resources/fonts/traveling_typewriter.ttf", 64, 0, 250);
texButton = LoadTexture("resources/textures/title_ribbon.png");
recButton.width = texButton.width;
recButton.height = texButton.height;
recButton.x = screenWidth - recButton.width;
recButton.y = screenHeight - recButton.height - 50;
fadeButton = 0.8f;
colorButton = RED;
textPositionButton = (Vector2){recButton.x + recButton.width/2, recButton.y + recButton.height/2};
fontSizeButton = 30;
textColorButton = WHITE;
currentMission = 0;
totalMissions = 4;
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
case MISSION: UnloadMissionScreen(); break;
case GAMEPLAY: UnloadGameplayScreen(); break;
case ENDING: UnloadEndingScreen(); break;
default: break;
}
UnloadMusicStream(music);
UnloadSound(fxButton);
UnloadFont(fontMission);
UnloadTexture(texButton);
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
case MISSION: UnloadMissionScreen(); break;
case GAMEPLAY: UnloadGameplayScreen(); break;
case ENDING: UnloadEndingScreen(); break;
default: break;
}
switch (screen)
{
case LOGO: InitLogoScreen(); break;
case TITLE: InitTitleScreen(); break;
case MISSION: InitMissionScreen(); break;
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
transAlpha += 0.02f;
if (transAlpha > 1.01f)
{
transAlpha = 1.0f;
switch (transFromScreen)
{
case LOGO: UnloadLogoScreen(); break;
case TITLE: UnloadTitleScreen(); break;
case MISSION: UnloadMissionScreen(); break;
case GAMEPLAY: UnloadGameplayScreen(); break;
case ENDING: UnloadEndingScreen(); break;
default: break;
}
switch (transToScreen)
{
case LOGO: InitLogoScreen(); break;
case TITLE: InitTitleScreen(); break;
case MISSION: InitMissionScreen(); break;
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
if (FinishTitleScreen()) 
{
StopMusicStream(music);
TransitionToScreen(MISSION);
}
} break;
case MISSION:
{
UpdateMissionScreen();
if (FinishMissionScreen())
{
StopMusicStream(music);
TransitionToScreen(GAMEPLAY);
}
} break;
case GAMEPLAY:
{
UpdateGameplayScreen();
if (FinishGameplayScreen() == 1) TransitionToScreen(ENDING);
} break;
case ENDING:
{ 
UpdateEndingScreen();
if (FinishEndingScreen() == 1) 
{
TransitionToScreen(MISSION);
}
else if (FinishEndingScreen() == 2) 
{
PlayMusicStream(music);
TransitionToScreen(TITLE);
}
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
case MISSION: DrawMissionScreen(); break;
case GAMEPLAY: DrawGameplayScreen(); break;
case ENDING: DrawEndingScreen(); break;
default: break;
}
if (onTransition) DrawTransition();
EndDrawing();
}
Mission *LoadMissions(const char *fileName)
{
Mission *missions = NULL;
char buffer[512];
int missionsCount = 0;
FILE *misFile = fopen(fileName, "rt");
if (misFile == NULL) printf("[%s] Missions file could not be opened\n", fileName);
else
{
while (!feof(misFile))
{
fgets(buffer, 512, misFile);
switch (buffer[0])
{
case 't': sscanf(buffer, "t %i", &missionsCount); break;
default: break;
}
}
if (missionsCount > 0) missions = (Mission *)malloc(missionsCount*sizeof(Mission));
else return NULL;
rewind(misFile); 
int missionNum = 0;
while (!feof(misFile))
{
fgets(buffer, 512, misFile);
if (missionNum < missionsCount)
{
switch (buffer[0])
{
case 'b': 
{
missions[missionNum].id = missionNum;
sscanf(buffer, "b %[^\n]s", missions[missionNum].brief); 
} break;
case 'k': sscanf(buffer, "k %[^\n]s", missions[missionNum].key); break;
case 'm':
{
sscanf(buffer, "m %[^\n]s", missions[missionNum].msg);
} break;
case 's': 
{
sscanf(buffer, "s %i %i %i %i %i %i %i %i", 
&missions[missionNum].sols[0], 
&missions[missionNum].sols[1],
&missions[missionNum].sols[2],
&missions[missionNum].sols[3],
&missions[missionNum].sols[4],
&missions[missionNum].sols[5],
&missions[missionNum].sols[6],
&missions[missionNum].sols[7]);
missions[missionNum].wordsCount = 0;
for (int i = 0; i < 8; i++) 
{
if (missions[missionNum].sols[i] > -1)
{
missions[missionNum].wordsCount++;
}
}
TraceLog(LOG_WARNING, "Mission %i - Words count %i", missionNum, missions[missionNum].wordsCount);
missionNum++;
} break;
default: break;
}
}
}
if (missionsCount != missionNum) TraceLog(LOG_WARNING, "Missions count and loaded missions don't match!");
}
fclose(misFile);
if (missions != NULL) 
{
TraceLog(LOG_INFO, "Missions loaded: %i", missionsCount);
TraceLog(LOG_INFO, "Missions loaded successfully!");
}
return missions;
}
bool IsButtonPressed()
{
if (CheckCollisionPointRec(GetMousePosition(), recButton))
{
fadeButton = 1.0f;
if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsGestureDetected(GESTURE_TAP))
{
PlaySound(fxButton);
return true;
}
}
else fadeButton = 0.80f;
return false;
}
void DrawButton(const char *text)
{
DrawTexturePro(texButton, (Rectangle){0,0,texButton.width, texButton.height}, recButton, (Vector2){0,0},0, Fade(WHITE, fadeButton));
Vector2 measure = MeasureTextEx(fontMission, text, fontSizeButton, 0);
Vector2 textPos = {textPositionButton.x - measure.x/2 + 10, textPositionButton.y - measure.y/2 - 10};
DrawTextEx(fontMission, text, textPos , fontSizeButton, 0, textColorButton);
}
