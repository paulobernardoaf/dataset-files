#if !defined(__al_included_allegro_aintern_ljoynu_h)
#define __al_included_allegro_aintern_ljoynu_h

#include "allegro5/joystick.h"
#include "allegro5/internal/aintern_joystick.h"












typedef enum {
LJOY_STATE_UNUSED,
LJOY_STATE_BORN,
LJOY_STATE_ALIVE,
LJOY_STATE_DYING
} CONFIG_STATE;

#define ACTIVE_STATE(st) ((st) == LJOY_STATE_ALIVE || (st) == LJOY_STATE_DYING)







#define TOTAL_JOYSTICK_AXES (0x28)

typedef struct {
int stick;
int axis;

int value;
int min;
int max;
int fuzz;
int flat;
} AXIS_MAPPING;



typedef struct {
int ev_code;
} BUTTON_MAPPING;


typedef struct ALLEGRO_JOYSTICK_LINUX
{
ALLEGRO_JOYSTICK parent;
int config_state;
bool marked;
int fd;
ALLEGRO_USTR *device_name;

AXIS_MAPPING axis_mapping[TOTAL_JOYSTICK_AXES];
BUTTON_MAPPING button_mapping[_AL_MAX_JOYSTICK_BUTTONS];
ALLEGRO_JOYSTICK_STATE joystate;
char name[100];
} ALLEGRO_JOYSTICK_LINUX;


#endif


