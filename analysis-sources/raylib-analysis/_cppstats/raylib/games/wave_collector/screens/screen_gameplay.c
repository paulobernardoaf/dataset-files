#include "raylib.h"
#include "screens.h"
#include <stdio.h>
#include <stdlib.h> 
#include <math.h> 
#define MAX_SAMPLES_SPEED 7 
#define MIN_SAMPLES_SPEED 3 
#define SAMPLES_SPACING 100 
#define SAMPLES_MULTIPLIER 700 
#define MAX_GAME_HEIGHT 400 
typedef struct Player {
Vector2 position;
Vector2 speed;
int width;
int height;
Color color;
} Player;
typedef struct Sample {
Vector2 position;
float value; 
int radius;
bool active; 
bool collected; 
bool renderable; 
Color color;
} Sample;
static int framesCounter;
static int finishScreen;
static bool pause;
static Player player;
static Rectangle playerArea; 
static float warpCounter; 
static float synchro; 
static int combo;
static int maxCombo;
static Rectangle waveRec;
static Sample *samples; 
static int totalSamples; 
static int collectedSamples; 
static int currentSample; 
static float samplesSpeed; 
static float waveTime; 
static Texture2D texBackground;
static Texture2D texPlayer;
static Texture2D texSampleSmall;
static Texture2D texSampleMid;
static Texture2D texSampleBig;
static RenderTexture2D waveTarget;
static Sound fxSampleOn; 
static Sound fxSampleOff; 
static Sound fxPause; 
static void DrawSamplesMap(Sample *samples, int sampleCount, int playedSamples, Rectangle bounds, Color color);
void InitGameplayScreen(void)
{
framesCounter = 0;
finishScreen = 0;
pause = false;
endingStatus = 0;
texBackground = LoadTexture("resources/textures/background_gameplay.png");
texPlayer = LoadTexture("resources/textures/player.png");
texSampleSmall = LoadTexture("resources/textures/sample_small.png");
texSampleMid = LoadTexture("resources/textures/sample_mid.png");
texSampleBig = LoadTexture("resources/textures/sample_big.png");
waveRec = (Rectangle){ 32, 32, 1280 - 64, 105 };
waveTarget = LoadRenderTexture(waveRec.width, waveRec.height);
fxSampleOn = LoadSound("resources/audio/sample_on.wav");
fxSampleOff = LoadSound("resources/audio/sample_off.wav");
fxPause = LoadSound("resources/audio/pause.wav");
SetSoundVolume(fxSampleOn, 0.6f);
SetSoundVolume(fxPause, 0.5f);
playerArea = (Rectangle){ 200, 160, 80, 400 };
player.width = 20;
player.height = 60;
player.speed = (Vector2){ 15, 15 };
player.color = GOLD;
player.position = (Vector2){ playerArea.x + playerArea.width/2 - texPlayer.width/2, 
playerArea.y + playerArea.height/2 - texPlayer.height/2 };
warpCounter = 395;
synchro = 0.2f;
combo = 0;
maxCombo = 0;
Wave wave = LoadWave("resources/audio/wave.ogg");
float *waveData = GetWaveData(wave); 
waveTime = wave.sampleCount/wave.sampleRate; 
float requiredSamples = (MAX_SAMPLES_SPEED*waveTime*60 - 1000)/SAMPLES_SPACING;
int samplesDivision = (int)(wave.sampleCount/requiredSamples);
totalSamples = wave.sampleCount/samplesDivision;
UnloadWave(wave);
collectedSamples = 0;
samples = (Sample *)malloc(totalSamples*sizeof(Sample));
float minSampleValue = 0.0f;
float maxSampleValue = 0.0f;
for (int i = 0; i < totalSamples; i++)
{
if (waveData[i*samplesDivision] < minSampleValue) minSampleValue = waveData[i*samplesDivision];
if (waveData[i*samplesDivision] > maxSampleValue) maxSampleValue = waveData[i*samplesDivision];
}
float sampleScaleFactor = 1.0f/(maxSampleValue - minSampleValue); 
for (int i = 0; i < totalSamples; i++)
{
samples[i].value = waveData[i*samplesDivision]*sampleScaleFactor; 
samples[i].position.x = player.position.x + 1000 + i*SAMPLES_SPACING;
samples[i].position.y = GetScreenHeight()/2 + samples[i].value*SAMPLES_MULTIPLIER;
if (samples[i].position.y > GetScreenHeight()/2 + MAX_GAME_HEIGHT/2) samples[i].position.y = GetScreenHeight()/2 - MAX_GAME_HEIGHT/2;
else if (samples[i].position.y < GetScreenHeight()/2 - MAX_GAME_HEIGHT/2) samples[i].position.y = GetScreenHeight()/2 + MAX_GAME_HEIGHT/2;
samples[i].radius = 6;
samples[i].active = true;
samples[i].collected = false;
samples[i].color = RED;
samples[i].renderable = false;
}
samplesSpeed = MAX_SAMPLES_SPEED;
currentSample = 0;
free(waveData);
StopMusicStream(music);
PlayMusicStream(music);
}
void UpdateGameplayScreen(void)
{
if (IsKeyPressed('P'))
{
PlaySound(fxPause);
pause = !pause;
if (pause) PauseMusicStream(music);
else ResumeMusicStream(music);
}
if (!pause)
{
framesCounter++; 
player.position.y = GetMousePosition().y;
if (IsKeyDown(KEY_W)) player.position.y -= player.speed.y;
else if (IsKeyDown(KEY_S)) player.position.y += player.speed.y;
if (player.position.x < playerArea.x) player.position.x = playerArea.x;
else if ((player.position.x + player.width) > (playerArea.x + playerArea.width)) player.position.x = playerArea.x + playerArea.width - player.width;
if (player.position.y < playerArea.y) player.position.y = playerArea.y;
else if ((player.position.y + player.height) > (playerArea.y + playerArea.height)) player.position.y = playerArea.y + playerArea.height - player.height;
for (int i = 0; i < totalSamples; i++)
{
samples[i].position.x -= samplesSpeed;
if (((samples[i].position.x + samples[i].radius) > -SAMPLES_SPACING) && 
((samples[i].position.x - samples[i].radius) < GetScreenWidth())) samples[i].renderable = true;
else samples[i].renderable = false;
if (!samples[i].collected && CheckCollisionCircleRec(samples[i].position, samples[i].radius, (Rectangle){ (int)player.position.x, (int)player.position.y, player.width, player.height }))
{
samples[i].collected = true;
collectedSamples++;
synchro += 0.02;
combo++;
if (combo > maxCombo) maxCombo = combo;
if (synchro >= 1.0f) synchro = 1.0f;
SetSoundPitch(fxSampleOn, samples[i].value*1.4f + 0.7f);
PlaySound(fxSampleOn);
}
if ((samples[i].position.x - samples[i].radius) < player.position.x)
{
currentSample = i; 
if (samples[i].active)
{
samples[i].active = false;
if (!samples[i].collected)
{
synchro -= 0.05f;
PlaySound(fxSampleOff);
combo = 0;
}
}
}
}
if (IsKeyDown(KEY_SPACE) && (warpCounter > 0))
{
warpCounter--;
if (warpCounter < 0) warpCounter = 0;
samplesSpeed -= 0.1f;
if (samplesSpeed <= MIN_SAMPLES_SPEED) samplesSpeed = MIN_SAMPLES_SPEED;
SetMusicPitch(music, samplesSpeed/MAX_SAMPLES_SPEED);
}
else
{
warpCounter++;
if (warpCounter > 395) warpCounter = 395;
samplesSpeed += 0.1f;
if (samplesSpeed >= MAX_SAMPLES_SPEED) samplesSpeed = MAX_SAMPLES_SPEED;
SetMusicPitch(music, samplesSpeed/MAX_SAMPLES_SPEED);
}
if (currentSample >= totalSamples - 1)
{
endingStatus = 1; 
finishScreen = 1;
}
if (synchro <= 0.0f)
{
synchro = 0.0f;
endingStatus = 2; 
finishScreen = 1;
}
}
}
void DrawGameplayScreen(void)
{
DrawTexture(texBackground, 0, 0, WHITE);
DrawRectangle(0, GetScreenHeight()/2 - 1, GetScreenWidth(), 2, Fade(BLUE, 0.3f));
for (int i = 0; i < totalSamples - 1; i++)
{
if (samples[i].renderable)
{
Color col = samples[i].color;
if (i < (currentSample + 1)) col = Fade(DARKGRAY, 0.5f);
else col = WHITE;
if (!samples[i].collected) 
{
if (combo > 30) DrawTexture(texSampleSmall, samples[i].position.x - texSampleSmall.width/2, samples[i].position.y - texSampleSmall.height/2, col);
else if (combo > 15) DrawTexture(texSampleMid, samples[i].position.x - texSampleMid.width/2, samples[i].position.y - texSampleMid.height/2, col);
else DrawTexture(texSampleBig, samples[i].position.x - texSampleBig.width/2, samples[i].position.y - texSampleBig.height/2, col);
}
if (i < (currentSample + 1)) col = Fade(GRAY, 0.3f);
else col = Fade(RED, 0.5f);
DrawLineEx(samples[i].position, samples[i + 1].position, 3.0f, col);
}
}
DrawTexture(texPlayer, player.position.x - 32, player.position.y - 24, WHITE);
if (pause) DrawTextEx(font, "WAVE PAUSED", (Vector2){ 235, 400 }, font.baseSize*2, 0, WHITE);
DrawTextEx(font, FormatText("%05i / %05i", collectedSamples, totalSamples), (Vector2){810, 170}, font.baseSize, -2, SKYBLUE);
DrawTextEx(font, FormatText("Combo: %02i [max: %02i]", combo, maxCombo), (Vector2){200, 170}, font.baseSize/2, -2, SKYBLUE);
DrawRectangle(99, 622, 395, 32, Fade(RAYWHITE, 0.8f));
if (synchro <= 0.3f) DrawRectangle(99, 622, synchro*395, 32, Fade(RED, 0.8f));
else if (synchro <= 0.8f) DrawRectangle(99, 622, synchro*395, 32, Fade(ORANGE,0.8f));
else if (synchro < 1.0f) DrawRectangle(99, 622, synchro*395, 32, Fade(LIME,0.8f));
else DrawRectangle(99, 622, synchro*395, 32, Fade(GREEN, 0.9f));
DrawRectangleLines(99, 622, 395, 32, MAROON);
if (synchro == 1.0f) DrawTextEx(font, FormatText("%02i%%", (int)(synchro*100)), (Vector2){99 + 390, 600}, font.baseSize, -2, GREEN);
else DrawTextEx(font, FormatText("%02i%%", (int)(synchro*100)), (Vector2){99 + 390, 600}, font.baseSize, -2, SKYBLUE);
DrawRectangle(754, 622, 395, 32, Fade(RAYWHITE, 0.8f));
DrawRectangle(754, 622, warpCounter, 32, Fade(SKYBLUE, 0.8f));
DrawRectangleLines(754, 622, 395, 32, DARKGRAY);
DrawTextEx(font, FormatText("%02i%%", (int)((float)warpCounter/395.0f*100.0f)), (Vector2){754 + 390, 600}, font.baseSize, -2, SKYBLUE);
if (waveTarget.texture.id <= 0) 
{
DrawSamplesMap(samples, totalSamples, currentSample, waveRec, MAROON);
DrawRectangle(waveRec.x + (int)currentSample*1215/totalSamples, waveRec.y, 2, 99, DARKGRAY);
}
else
{
BeginTextureMode(waveTarget);
ClearBackground(BLANK);
DrawSamplesMap(samples, totalSamples, currentSample, (Rectangle){ 0, 0, waveTarget.texture.width, waveTarget.texture.height }, MAROON);
EndTextureMode();
DrawTextureEx(waveTarget.texture, (Vector2){ waveRec.x, waveRec.y }, 0.0f, 1.0f, WHITE);
DrawRectangle(waveRec.x + (int)currentSample*1215/totalSamples, waveRec.y, 2, 99, DARKGRAY);
}
}
void UnloadGameplayScreen(void)
{
StopMusicStream(music);
UnloadTexture(texBackground);
UnloadTexture(texPlayer);
UnloadTexture(texSampleSmall);
UnloadTexture(texSampleMid);
UnloadTexture(texSampleBig);
UnloadRenderTexture(waveTarget);
UnloadSound(fxSampleOn);
UnloadSound(fxSampleOff);
UnloadSound(fxPause);
free(samples); 
}
int FinishGameplayScreen(void)
{
return finishScreen;
}
static void DrawSamplesMap(Sample *samples, int sampleCount, int playedSamples, Rectangle bounds, Color color)
{
float sampleIncrementX = (float)bounds.width/sampleCount;
Color col = color;
for (int i = 0; i < sampleCount - 1; i++)
{
if (i < playedSamples) col = GRAY;
else col = color;
DrawLineV((Vector2){ (float)bounds.x + (float)i*sampleIncrementX, (float)(bounds.y + bounds.height/2) + samples[i].value*bounds.height }, 
(Vector2){ (float)bounds.x + (float)(i + 1)*sampleIncrementX, (float)(bounds.y + bounds.height/2) + + samples[i + 1].value*bounds.height }, col);
}
}