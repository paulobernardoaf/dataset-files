#if !defined(__DEMO_DEFINES_H__)
#define __DEMO_DEFINES_H__

#include "demodata.h"


#define DEMO_OK 0
#define DEMO_ERROR_ALLEGRO 1
#define DEMO_ERROR_GFX 2
#define DEMO_ERROR_MEMORY 3
#define DEMO_ERROR_VIDEOMEMORY 4
#define DEMO_ERROR_TRIPLEBUFFER 5
#define DEMO_ERROR_DATA 6
#define DEMO_ERROR_GAMEDATA 7


#define DEMO_UPDATE_DIRECT 0
#define DEMO_DOUBLE_BUFFER 1
#define DEMO_PAGE_FLIPPING 2
#define DEMO_TRIPLE_BUFFER 3
#define DEMO_OGL_FLIPPING 4


#define DEMO_CONTROLLER_KEYBOARD 0
#define DEMO_CONTROLLER_GAMEPAD 1


#define DEMO_BUTTON_LEFT 0
#define DEMO_BUTTON_RIGHT 1
#define DEMO_BUTTON_JUMP 2





#define DEMO_STATE_MAIN_MENU 0
#define DEMO_STATE_NEW_GAME 1
#define DEMO_STATE_OPTIONS 2
#define DEMO_STATE_GFX 3
#define DEMO_STATE_SOUND 4
#define DEMO_STATE_CONTROLS 5
#define DEMO_STATE_MISC 6
#define DEMO_STATE_ABOUT 7
#define DEMO_STATE_HELP 8
#define DEMO_STATE_INTRO 9
#define DEMO_STATE_CONTINUE_GAME 10
#define DEMO_STATE_SUCCESS 11
#define DEMO_STATE_EXIT -1


#define DEMO_PATH_LENGTH 1024


#define DEMO_MAX_GAMESTATES 64




#if defined(DEMO_USE_ALLEGRO_GL)
#define DEMO_USE_ALLEGRO_GL_FONT
#endif

#endif 
