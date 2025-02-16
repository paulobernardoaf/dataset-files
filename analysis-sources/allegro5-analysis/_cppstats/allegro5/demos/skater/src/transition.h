#include <allegro5/allegro.h>
#include "gamestate.h" 
typedef struct TRANSITION {
GAMESTATE *from;
GAMESTATE *to;
double duration;
double progress;
ALLEGRO_BITMAP *from_bmp;
ALLEGRO_BITMAP *to_bmp;
} TRANSITION;
TRANSITION *create_transition(GAMESTATE * from, GAMESTATE * to,
double duration);
void destroy_transition(TRANSITION * t);
int update_transition(TRANSITION * t);
void draw_transition(TRANSITION * t);
