#include "raylib.h"
#include "screens/screens.h" 
#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif
GameScreen currentScreen = 0;
Font font = { 0 };
Music music = { 0 };
Sound fxCoin = { 0 };
Texture2D background = { 0 };
Texture2D texNPatch = { 0 };
NPatchInfo npInfo = { 0 };
Texture2D texHead, texHair, texNose, texMouth, texEyes, texComp;
Character playerBase = { 0 };
Character datingBase = { 0 };
Character player = { 0 };
Character dating = { 0 };
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
InitWindow(screenWidth, screenHeight, "RE-PAIR [GGJ2020]");
InitAudioDevice();
font = LoadFont("resources/font.png");
SetTextureFilter(font.texture, FILTER_BILINEAR);
music = LoadMusicStream("resources/elevator_romance.ogg");
fxCoin = LoadSound("resources/coin.wav");
background = LoadTexture("resources/background.png");
texNPatch = LoadTexture("resources/npatch.png");
npInfo.sourceRec = (Rectangle){ 0, 0, 80, texNPatch.height },
npInfo.left = 24;
npInfo.top = 24;
npInfo.right = 24;
npInfo.bottom = 24;
texHead = LoadTexture("resources/head_models.png");
texHair = LoadTexture("resources/hair_models.png");
texNose = LoadTexture("resources/nose_models.png");
texMouth = LoadTexture("resources/mouth_models.png");
texEyes = LoadTexture("resources/eyes_models.png");
SetMusicVolume(music, 0.5f);
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
UnloadTexture(background);
UnloadTexture(texNPatch);
UnloadTexture(texHead);
UnloadTexture(texHair);
UnloadTexture(texNose);
UnloadTexture(texMouth);
UnloadTexture(texEyes);
CloseAudioDevice(); 
CloseWindow(); 
return 0;
}
Character GenerateCharacter(void)
{
Character character = { 0 };
character.head = GetRandomValue(0, texHead.width/BASE_HEAD_WIDTH - 1);
character.colHead = headColors[GetRandomValue(0, 5)];
character.hair = GetRandomValue(0, texHair.width/BASE_HAIR_WIDTH - 1);
character.colHair = hairColors[GetRandomValue(0, 9)];
character.eyes = GetRandomValue(0, texEyes.width/BASE_EYES_WIDTH - 1);
character.nose = GetRandomValue(0, texNose.width/BASE_NOSE_WIDTH - 1);
character.mouth = GetRandomValue(0, texMouth.width/BASE_MOUTH_WIDTH - 1);
return character;
}
void CustomizeCharacter(Character *character)
{
if (GetRandomValue(0, 1)) character->hair = GetRandomValue(0, texHair.width/BASE_HAIR_WIDTH - 1);
if (GetRandomValue(0, 1)) character->colHair = hairColors[GetRandomValue(0, 9)];
if (GetRandomValue(0, 1)) character->eyes = GetRandomValue(0, texEyes.width/BASE_EYES_WIDTH - 1);
if (GetRandomValue(0, 1)) character->nose = GetRandomValue(0, texNose.width/BASE_NOSE_WIDTH - 1);
if (GetRandomValue(0, 1)) character->mouth = GetRandomValue(0, texMouth.width/BASE_MOUTH_WIDTH - 1);
}
void DrawCharacter(Character character, Vector2 position)
{
DrawTextureRec(texHair, (Rectangle){ BASE_HAIR_WIDTH*character.hair, 240, BASE_HAIR_WIDTH, texHair.height - 240 }, (Vector2){ position.x + (250 - BASE_HAIR_WIDTH)/2, position.y + 240 }, GetColor(character.colHair));
DrawTextureRec(texHead, (Rectangle){ BASE_HEAD_WIDTH*character.head, 0, BASE_HEAD_WIDTH, texHead.height }, (Vector2){ position.x + (250 - BASE_HEAD_WIDTH)/2, position.y + 60 }, GetColor(character.colHead));
DrawTextureRec(texHair, (Rectangle){ BASE_HAIR_WIDTH*character.hair, 0, BASE_HAIR_WIDTH, 240 }, (Vector2){ position.x + (250 - BASE_HAIR_WIDTH)/2, position.y }, GetColor(character.colHair));
DrawTextureRec(texEyes, (Rectangle){ BASE_EYES_WIDTH*character.eyes, 0, BASE_EYES_WIDTH, texEyes.height }, (Vector2){ position.x + (250 - BASE_EYES_WIDTH)/2, position.y + 190 }, WHITE);
DrawTextureRec(texNose, (Rectangle){ BASE_NOSE_WIDTH*character.nose, 0, BASE_NOSE_WIDTH, texNose.height }, (Vector2){ position.x + (250 - BASE_NOSE_WIDTH)/2, position.y + 275 }, GetColor(character.colHead));
DrawTextureRec(texMouth, (Rectangle){ BASE_MOUTH_WIDTH*character.mouth, 0, BASE_MOUTH_WIDTH, texMouth.height }, (Vector2){ position.x + (250 - BASE_MOUTH_WIDTH)/2, position.y + 370 }, GetColor(character.colHead));
}
bool GuiButton(Rectangle bounds, const char *text, int forcedState)
{
static const int textColor[4] = { 0xeff6ffff, 0x78e782ff, 0xb04d5fff, 0xd6d6d6ff };
int state = (forcedState >= 0)? forcedState : 0; 
bool pressed = false;
Vector2 textSize = MeasureTextEx(font, text, font.baseSize, 1);
if ((state < 3) && (forcedState < 0))
{
Vector2 mousePoint = GetMousePosition();
if (CheckCollisionPointRec(mousePoint, bounds))
{
if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = 2; 
else state = 1; 
if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) || IsGestureDetected(GESTURE_TAP))
{
pressed = true;
PlaySound(fxCoin);
}
}
}
npInfo.sourceRec.x = 80*state;
DrawTextureNPatch(texNPatch, npInfo, bounds, (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
DrawTextEx(font, text, (Vector2){ bounds.x + bounds.width/2 - textSize.x/2, bounds.y + bounds.height/2 - textSize.y/2 + 4 }, font.baseSize, 1, GetColor(textColor[state]));
return pressed;
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
if (FinishLogoScreen())
{
TransitionToScreen(TITLE);
PlayMusicStream(music);
}
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
