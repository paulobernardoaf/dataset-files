#include <allegro5/allegro.h>
#define SCREEN_W 640
#define SCREEN_H 480
#define FPS 30 
#define PLAYER_SIZE 16 
#define PLAYER_SPEED 200 
#define MAX_PLAYER_COUNT 32
#define DEFAULT_PORT 9234
typedef enum
{
PLAYER_JOIN,
PLAYER_LEAVE,
POSITION_UPDATE,
} MESSAGE_TYPE;
typedef struct
{
int x; 
int y; 
} ClientMessage;
typedef struct
{
int player_id;
MESSAGE_TYPE type;
int x; 
int y; 
ALLEGRO_COLOR color; 
} ServerMessage;
struct
{
bool active;
int x, y; 
int dx, dy; 
ALLEGRO_COLOR color;
} players[MAX_PLAYER_COUNT];
