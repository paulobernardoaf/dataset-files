#define MAX_JOYSTICKS 4
#define MAX_BUTTONS 14
#define MAX_TRIGGERS 2
#define MAX_AXES 2
#define MAX_STICKS 4
ALLEGRO_STATIC_ASSERT(wjoydxnu, _AL_MAX_JOYSTICK_STICKS >= (2 + MAX_TRIGGERS + MAX_STICKS));
ALLEGRO_STATIC_ASSERT(wjoydxnu, _AL_MAX_JOYSTICK_BUTTONS >= MAX_BUTTONS);
typedef enum
{
STATE_UNUSED,
STATE_ACTIVE,
} CONFIG_STATE;
#define ACTIVE_STATE(st) ((st) == STATE_ACTIVE)
typedef struct ALLEGRO_JOYSTICK_XINPUT
{
ALLEGRO_JOYSTICK parent; 
bool active;
ALLEGRO_JOYSTICK_STATE joystate;
DWORD index;
XINPUT_STATE state;
XINPUT_CAPABILITIES capabilities;
XINPUT_VIBRATION vibration;
char name[80];
char all_names[512]; 
} ALLEGRO_JOYSTICK_XINPUT;
