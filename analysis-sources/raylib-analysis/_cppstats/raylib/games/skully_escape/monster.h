#define MONSTER_ANIM_FRAMES 7
#define MONSTER_ANIM_SEQ 2
typedef struct Monster {
Vector2 position;
Texture2D texture;
Rectangle bounds;
Rectangle frameRec;
Color color;
int framesCounter;
int currentFrame;
int currentSeq;
int numFrames;
bool active;
bool selected;
bool spooky;
} Monster;
#if defined(__cplusplus)
extern "C" { 
#endif
void UpdateMonster(Monster *monster);
void DrawMonster(Monster monster, int scroll);
void UnloadMonster(Monster monster);
#if defined(__cplusplus)
}
#endif
