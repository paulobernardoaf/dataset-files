#define MAX_JOYSTICKS 8
#define MAX_SLIDERS 2
#define MAX_POVS 4
#define MAX_BUTTONS 32
#define DEVICE_BUFFER_SIZE 10
ALLEGRO_STATIC_ASSERT(wjoydxnu, _AL_MAX_JOYSTICK_STICKS >= (2 + MAX_SLIDERS + MAX_POVS));
ALLEGRO_STATIC_ASSERT(wjoydxnu, _AL_MAX_JOYSTICK_BUTTONS >= MAX_BUTTONS);
#define GUID_EQUAL(a, b) (0 == memcmp(&(a), &(b), sizeof(GUID)))
typedef enum
{
STATE_UNUSED,
STATE_BORN,
STATE_ALIVE,
STATE_DYING
} CONFIG_STATE;
#define ACTIVE_STATE(st) ((st) == STATE_ALIVE || (st) == STATE_DYING)
#define NAME_LEN 128
typedef struct
{
bool have_x; TCHAR name_x[NAME_LEN];
bool have_y; TCHAR name_y[NAME_LEN];
bool have_z; TCHAR name_z[NAME_LEN];
bool have_rx; TCHAR name_rx[NAME_LEN];
bool have_ry; TCHAR name_ry[NAME_LEN];
bool have_rz; TCHAR name_rz[NAME_LEN];
int num_sliders; TCHAR name_slider[MAX_SLIDERS][NAME_LEN];
int num_povs; TCHAR name_pov[MAX_POVS][NAME_LEN];
int num_buttons; TCHAR name_button[MAX_BUTTONS][NAME_LEN];
} CAPS_AND_NAMES;
typedef struct
{
int stick, axis;
} AXIS_MAPPING;
typedef struct ALLEGRO_JOYSTICK_DIRECTX
{
ALLEGRO_JOYSTICK parent; 
CONFIG_STATE config_state;
bool marked;
LPDIRECTINPUTDEVICE2 device;
GUID guid;
GUID product_guid;
HANDLE waker_event;
ALLEGRO_JOYSTICK_STATE joystate;
AXIS_MAPPING x_mapping;
AXIS_MAPPING y_mapping;
AXIS_MAPPING z_mapping;
AXIS_MAPPING rx_mapping;
AXIS_MAPPING ry_mapping;
AXIS_MAPPING rz_mapping;
AXIS_MAPPING slider_mapping[MAX_SLIDERS];
int pov_mapping_stick[MAX_POVS];
char name[80];
char all_names[512]; 
} ALLEGRO_JOYSTICK_DIRECTX;
