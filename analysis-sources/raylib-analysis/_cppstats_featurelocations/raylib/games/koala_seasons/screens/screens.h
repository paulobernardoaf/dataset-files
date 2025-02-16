
























#if !defined(SCREENS_H)
#define SCREENS_H

#define GAME_FPS 60.0
#define TIME_FACTOR 60.0/GAME_FPS

#define MAX_KILLS 128




typedef enum GameScreen { LOGO, TITLE, OPTIONS, GAMEPLAY, ENDING } GameScreen;




GameScreen currentScreen;


Font font;
Shader colorBlend;
Texture2D atlas01;
Texture2D atlas02;

Sound fxJump; 
Sound fxDash; 
Sound fxEatLeaves;
Sound fxDieSnake;
Sound fxDieDingo;
Sound fxDieOwl;
Sound fxHitResin;
Sound fxWind;


int score;
int hiscore;
int killHistory[MAX_KILLS];
int killer;
int seasons;
int years;
int currentLeaves;
int currentSeason;
int initSeason;
int initYears;
int rainChance;

#if defined(__cplusplus)
extern "C" { 
#endif




void InitLogoScreen(void);
void UpdateLogoScreen(void);
void DrawLogoScreen(void);
void UnloadLogoScreen(void);
int FinishLogoScreen(void);




void InitTitleScreen(void);
void UpdateTitleScreen(void);
void DrawTitleScreen(void);
void UnloadTitleScreen(void);
int FinishTitleScreen(void);




void InitOptionsScreen(void);
void UpdateOptionsScreen(void);
void DrawOptionsScreen(void);
void UnloadOptionsScreen(void);
int FinishOptionsScreen(void);




void InitGameplayScreen(void);
void UpdateGameplayScreen(void);
void DrawGameplayScreen(void);
void UnloadGameplayScreen(void);
int FinishGameplayScreen(void);




void InitEndingScreen(void);
void UpdateEndingScreen(void);
void DrawEndingScreen(void);
void UnloadEndingScreen(void);
int FinishEndingScreen(void);

#if defined(__cplusplus)
}
#endif

#endif