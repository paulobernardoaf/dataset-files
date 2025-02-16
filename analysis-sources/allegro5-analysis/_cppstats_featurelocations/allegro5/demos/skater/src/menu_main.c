#include <allegro5/allegro.h>
#include "defines.h"
#include "global.h"
#include "menu.h"
#include "menus.h"
#include "music.h"


static int _id = DEMO_STATE_MAIN_MENU;
static int already_said_welcome = 0;

static int id(void)
{
return _id;
}


static DEMO_MENU menu[] = {
DEMO_MENU_ITEM4(demo_button_proc, "New Game", DEMO_MENU_SELECTABLE, DEMO_STATE_NEW_GAME),
DEMO_MENU_ITEM4(demo_text_proc, "Continue Game", 0, DEMO_STATE_CONTINUE_GAME),
DEMO_MENU_ITEM4(demo_button_proc, "Options", DEMO_MENU_SELECTABLE, DEMO_STATE_OPTIONS),
DEMO_MENU_ITEM4(demo_button_proc, "About", DEMO_MENU_SELECTABLE, DEMO_STATE_ABOUT),
DEMO_MENU_ITEM4(demo_button_proc, "Exit", DEMO_MENU_SELECTABLE, DEMO_STATE_EXIT),
DEMO_MENU_END
};

void enable_continue_game(void)
{
menu[1].proc = demo_button_proc;
menu[1].flags |= DEMO_MENU_SELECTABLE;
}



void disable_continue_game(void)
{
menu[1].proc = demo_text_proc;
menu[1].flags &= ~DEMO_MENU_SELECTABLE;


if (menu[1].flags & DEMO_MENU_SELECTED) {
menu[1].flags &= ~DEMO_MENU_SELECTED;
menu[0].flags |= DEMO_MENU_SELECTED;
}
}

static void init(void)
{
init_demo_menu(menu, true);

if (!already_said_welcome) {
play_sound(demo_data[DEMO_SAMPLE_WELCOME].dat, 255, 128, 1000, 0);
already_said_welcome = 1;
}
}


static int update(void)
{
int ret = update_demo_menu(menu);

switch (ret) {
case DEMO_MENU_CONTINUE:
return id();

case DEMO_MENU_BACK:
return DEMO_STATE_EXIT;

default:
return ret;
};
}


static void draw(void)
{
draw_demo_menu(menu);
}


void create_main_menu(GAMESTATE * state)
{
state->id = id;
state->init = init;
state->update = update;
state->draw = draw;
}
