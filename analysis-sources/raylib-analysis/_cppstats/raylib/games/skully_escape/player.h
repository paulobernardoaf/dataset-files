typedef enum { NONE, WALK_RIGHT, WALK_LEFT, SCARE_RIGHT, SCARE_LEFT, SEARCH, FIND_KEY } PlayerSequence;
typedef struct Player {
Vector2 position;
Rectangle bounds;
Texture2D texture;
Color color;
Rectangle frameRec;
int currentFrame;
int currentSeq;
bool key;
int numLifes;
bool dead;
} Player;
Player player;
#if defined(__cplusplus)
extern "C" { 
#endif
void InitPlayer(void);
void UpdatePlayer(void);
void DrawPlayer(void);
void UnloadPlayer(void);
void ResetPlayer(void);
void ScarePlayer(void);
void SearchKeyPlayer(void);
void FindKeyPlayer(void);
#if defined(__cplusplus)
}
#endif
