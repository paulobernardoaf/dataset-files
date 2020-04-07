typedef enum GameScreen { LOGO = 0, TITLE, OPTIONS, GAMEPLAY, ENDING } GameScreen;
extern GameScreen currentScreen;
extern Font font;
extern Font font2;
extern Music music;
extern Sound fxCoin;
int score;
int result; 
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
