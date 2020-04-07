typedef enum GameScreen { LOGO_RL = 0, TITLE, GAMEPLAY } GameScreen;
GameScreen currentScreen;
Font font;
Color *lightsMap;
int lightsMapWidth, lightsMapHeight;
#if defined(__cplusplus)
extern "C" { 
#endif
void rlInitLogoScreen(void);
void rlUpdateLogoScreen(void);
void rlDrawLogoScreen(void);
void rlUnloadLogoScreen(void);
int rlFinishLogoScreen(void);
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
#if defined(__cplusplus)
}
#endif
