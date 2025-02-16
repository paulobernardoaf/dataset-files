typedef enum GameScreen { LOGO, LEVEL00, LEVEL01, LEVEL02, LEVEL03, LEVEL04, LEVEL05, LEVEL06, LEVEL07, LEVEL08, LEVEL09 } GameScreen;
GameScreen currentScreen;
Sound levelWin;
#if defined(__cplusplus)
extern "C" { 
#endif
void InitLogoScreen(void);
void UpdateLogoScreen(void);
void DrawLogoScreen(void);
void UnloadLogoScreen(void);
int FinishLogoScreen(void);
void InitLevel00Screen(void);
void UpdateLevel00Screen(void);
void DrawLevel00Screen(void);
void UnloadLevel00Screen(void);
int FinishLevel00Screen(void);
void InitLevel01Screen(void);
void UpdateLevel01Screen(void);
void DrawLevel01Screen(void);
void UnloadLevel01Screen(void);
int FinishLevel01Screen(void);
void InitLevel02Screen(void);
void UpdateLevel02Screen(void);
void DrawLevel02Screen(void);
void UnloadLevel02Screen(void);
int FinishLevel02Screen(void);
void InitLevel03Screen(void);
void UpdateLevel03Screen(void);
void DrawLevel03Screen(void);
void UnloadLevel03Screen(void);
int FinishLevel03Screen(void);
void InitLevel04Screen(void);
void UpdateLevel04Screen(void);
void DrawLevel04Screen(void);
void UnloadLevel04Screen(void);
int FinishLevel04Screen(void);
void InitLevel05Screen(void);
void UpdateLevel05Screen(void);
void DrawLevel05Screen(void);
void UnloadLevel05Screen(void);
int FinishLevel05Screen(void);
void InitLevel06Screen(void);
void UpdateLevel06Screen(void);
void DrawLevel06Screen(void);
void UnloadLevel06Screen(void);
int FinishLevel06Screen(void);
void InitLevel07Screen(void);
void UpdateLevel07Screen(void);
void DrawLevel07Screen(void);
void UnloadLevel07Screen(void);
int FinishLevel07Screen(void);
void InitLevel08Screen(void);
void UpdateLevel08Screen(void);
void DrawLevel08Screen(void);
void UnloadLevel08Screen(void);
int FinishLevel08Screen(void);
void InitLevel09Screen(void);
void UpdateLevel09Screen(void);
void DrawLevel09Screen(void);
void UnloadLevel09Screen(void);
int FinishLevel09Screen(void);
void DrawRectangleBordersRec(Rectangle rec, int offsetX, int offsetY, int borderSize, Color col);
#if defined(__cplusplus)
}
#endif
