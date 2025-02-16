#define MAX_CODING_WORDS 12
#define MAX_MISSION_WORDS 8
typedef enum GameScreen { LOGO = 0, TITLE, MISSION, GAMEPLAY, ENDING } GameScreen;
typedef struct Word {
int id;
Rectangle rec;
Rectangle iniRec;
bool hover;
bool picked;
char text[32]; 
} Word;
typedef struct Mission {
int id;
char brief[512]; 
char key[32]; 
char msg[256]; 
int wordsCount; 
int sols[10]; 
} Mission;
GameScreen currentScreen;
Music music;
Sound fxButton;
Rectangle recButton;
float fadeButton;
Color colorButton;
Texture2D texButton;
Vector2 textPositionButton;
int fontSizeButton;
Color textColorButton;
int currentMission;
int totalMissions;
Font fontMission;
Word messageWords[MAX_MISSION_WORDS];
#if defined(__cplusplus)
extern "C" { 
#endif
bool IsButtonPressed();
void DrawButton(const char *text);
Mission *LoadMissions(const char *fileName);
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
void InitMissionScreen(void);
void UpdateMissionScreen(void);
void DrawMissionScreen(void);
void UnloadMissionScreen(void);
int FinishMissionScreen(void);
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
