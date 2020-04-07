#include "raylib.h"
#include "screens.h"
#include <string.h>
#include <stdlib.h>
#define MAX_TITLE_CHAR 256
#define MAX_SUBTITLE_CHAR 256
static char *codingWords[MAX_CODING_WORDS] = {
"pollo\0",
"conejo\0",
"huevo\0",
"nido\0",
"aire\0",
"armario\0",
"agujero\0",
"platano\0",
"pastel\0",
"mercado\0",
"raton\0",
"melon\0",
};
static int framesCounter;
static int finishScreen;
static Texture2D texBackground;
static Texture2D texNewspaper;
static Texture2D texVignette;
static Sound fxNews;
static float rotation = 0.1f;
static float scale = 0.05f;
static int state = 0;
static Mission *missions = NULL;
static char headline[MAX_TITLE_CHAR] = "\0";
Font fontNews;
static char *StringReplace(char *orig, char *rep, char *with);
void InitEndingScreen(void)
{
framesCounter = 0;
finishScreen = 0;
rotation = 0.1f;
scale = 0.05f;
state = 0;
texBackground = LoadTexture("resources/textures/ending_background.png");
texVignette = LoadTexture("resources/textures/message_vignette.png");
fxNews = LoadSound("resources/audio/fx_batman.ogg");
missions = LoadMissions("resources/missions.txt");
int wordsCount = missions[currentMission].wordsCount;
strcpy(headline, missions[currentMission].msg); 
int len = strlen(headline);
for (int i = 0; i < len; i++)
{
if (headline[i] == '@') headline[i] = ' ';
}
for (int i = 0; i < wordsCount; i++)
{
if (messageWords[i].id != missions[currentMission].sols[i])
{
char *title = StringReplace(headline, messageWords[i].text, codingWords[messageWords[i].id]);
if (title != NULL)
{
strcpy(headline, title); 
free(title);
}
}
}
TraceLog(LOG_WARNING, "Titular: %s", headline);
Image imNewspaper = LoadImage("resources/textures/ending_newspaper.png");
fontNews = LoadFontEx("resources/fonts/Lora-Bold.ttf", 32, 0, 250);
ImageDrawTextEx(&imNewspaper, (Vector2){ 50, 220 }, fontNews, headline, fontNews.baseSize, 0, DARKGRAY);
texNewspaper = LoadTextureFromImage(imNewspaper);
UnloadImage(imNewspaper);
}
void UpdateEndingScreen(void)
{
framesCounter++;
if (framesCounter == 10) PlaySound(fxNews);
if (state == 0)
{
rotation += 18.0f;
scale += 0.0096f;
if (scale >= 1.0f)
{
scale = 1.0f;
state = 1;
}
}
if ((state == 1) && (IsKeyPressed(KEY_ENTER) || IsButtonPressed()))
{
currentMission++;
if (currentMission >= totalMissions) finishScreen = 2;
else finishScreen = 1;
}
}
void DrawEndingScreen(void)
{
DrawTexture(texBackground, 0, 0, WHITE);
DrawTexturePro(texNewspaper, (Rectangle){ 0, 0, texNewspaper.width, texNewspaper.height },
(Rectangle){ GetScreenWidth()/2, GetScreenHeight()/2, texNewspaper.width*scale, texNewspaper.height*scale },
(Vector2){ (float)texNewspaper.width*scale/2, (float)texNewspaper.height*scale/2 }, rotation, WHITE);
DrawTextureEx(texVignette, (Vector2){ 0, 0 }, 0.0f, 2.0f, WHITE);
DrawTextEx(fontNews, headline, (Vector2){ 10, 10 }, fontNews.baseSize, 0, RAYWHITE);
for (int i = 0; i < missions[currentMission].wordsCount; i++)
{
DrawText(codingWords[messageWords[i].id], 10, 60 + 30*i, 20, (messageWords[i].id == missions[currentMission].sols[i]) ? GREEN : RED);
}
if (state == 1) DrawButton("continuar");
}
void UnloadEndingScreen(void)
{
UnloadTexture(texBackground);
UnloadTexture(texNewspaper);
UnloadTexture(texVignette);
UnloadSound(fxNews);
free(missions);
}
int FinishEndingScreen(void)
{
return finishScreen;
}
static char *StringReplace(char *orig, char *rep, char *with)
{
char *result; 
char *ins; 
char *tmp; 
int len_rep; 
int len_with; 
int len_front; 
int count; 
if (!orig || !rep) return NULL;
len_rep = strlen(rep);
if (len_rep == 0) return NULL; 
if (!with) with = ""; 
len_with = strlen(with);
ins = orig;
for (count = 0; (tmp = strstr(ins, rep)); ++count)
{
ins = tmp + len_rep;
}
tmp = result = malloc(strlen(orig) + (len_with - len_rep)*count + 1);
if (!result) return NULL; 
while (count--)
{
ins = strstr(orig, rep);
len_front = ins - orig;
tmp = strncpy(tmp, orig, len_front) + len_front;
tmp = strcpy(tmp, with) + len_with;
orig += len_front + len_rep; 
}
strcpy(tmp, orig);
return result;
}
