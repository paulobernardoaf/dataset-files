typedef struct ALLEGRO_JOYSTICK_WINDOWS_ALL {
ALLEGRO_JOYSTICK parent; 
bool active;
int index;
ALLEGRO_JOYSTICK * handle;
ALLEGRO_JOYSTICK_DRIVER * driver;
} ALLEGRO_JOYSTICK_WINDOWS_ALL;
