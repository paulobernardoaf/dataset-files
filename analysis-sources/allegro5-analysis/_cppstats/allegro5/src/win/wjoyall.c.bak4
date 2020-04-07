









































#define ALLEGRO_NO_COMPATIBILITY


#define _WIN32_WINNT 0x0501

#include "allegro5/allegro.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/platform/aintwin.h"
#include "allegro5/internal/aintern_events.h"
#include "allegro5/internal/aintern_joystick.h"
#include "allegro5/internal/aintern_bitmap.h"


#if defined(ALLEGRO_CFG_XINPUT)



#if !defined(ALLEGRO_WINDOWS)
#error something is wrong with the makefile
#endif

#if defined(ALLEGRO_MINGW32)
#undef MAKEFOURCC
#endif

#include <stdio.h>

ALLEGRO_DEBUG_CHANNEL("wjoyall")

#include "allegro5/joystick.h"
#include "allegro5/internal/aintern_joystick.h"
#include "allegro5/internal/aintern_wjoyall.h"


static bool joyall_init_joystick(void);
static void joyall_exit_joystick(void);
static bool joyall_reconfigure_joysticks(void);
static int joyall_get_num_joysticks(void);
static ALLEGRO_JOYSTICK *joyall_get_joystick(int num);
static void joyall_release_joystick(ALLEGRO_JOYSTICK *joy);
static void joyall_get_joystick_state(ALLEGRO_JOYSTICK *joy, ALLEGRO_JOYSTICK_STATE *ret_state);
static const char *joyall_get_name(ALLEGRO_JOYSTICK *joy);
static bool joyall_get_active(ALLEGRO_JOYSTICK *joy);



ALLEGRO_JOYSTICK_DRIVER _al_joydrv_windows_all =
{
AL_JOY_TYPE_WINDOWS_ALL,
"",
"",
"Windows Joystick",
joyall_init_joystick,
joyall_exit_joystick,
joyall_reconfigure_joysticks,
joyall_get_num_joysticks,
joyall_get_joystick,
joyall_release_joystick,
joyall_get_joystick_state,
joyall_get_name,
joyall_get_active
};


static ALLEGRO_MUTEX *joyall_mutex = NULL;

static bool ok_xi = false;
static bool ok_di = false;


static void joyall_setup_joysticks(void)
{
int index;
int num_xinput = 0;
int num_dinput = 0;
if (ok_di)
num_dinput = _al_joydrv_directx.num_joysticks();
if (ok_xi)
num_xinput = _al_joydrv_xinput.num_joysticks();

for (index = 0; index < num_xinput; index++) {
ALLEGRO_JOYSTICK *joystick = _al_joydrv_xinput.get_joystick(index);
joystick->driver = &_al_joydrv_xinput;
}

for (index = 0; index < num_dinput; index++) {
ALLEGRO_JOYSTICK *joystick = _al_joydrv_directx.get_joystick(index);
joystick->driver = &_al_joydrv_directx;
}
}



static bool joyall_init_joystick(void)
{

joyall_mutex = al_create_mutex_recursive();
if (!joyall_mutex)
return false;

al_lock_mutex(joyall_mutex);

ok_xi = _al_joydrv_xinput.init_joystick();
ok_di = _al_joydrv_directx.init_joystick();
joyall_setup_joysticks();
al_unlock_mutex(joyall_mutex);
return ok_xi || ok_di;
}


static void joyall_exit_joystick(void)
{
al_lock_mutex(joyall_mutex);
_al_joydrv_xinput.exit_joystick();
_al_joydrv_directx.exit_joystick();
al_unlock_mutex(joyall_mutex);
al_destroy_mutex(joyall_mutex);
}

static bool joyall_reconfigure_joysticks(void)
{
al_lock_mutex(joyall_mutex);
if (ok_xi)
_al_joydrv_xinput.reconfigure_joysticks();
if (ok_di)
_al_joydrv_directx.reconfigure_joysticks();
joyall_setup_joysticks();
al_unlock_mutex(joyall_mutex);
return true;
}

static int joyall_get_num_joysticks(void)
{
int ret = 0;
if (ok_xi)
ret += _al_joydrv_xinput.num_joysticks();
if (ok_di)
ret += _al_joydrv_directx.num_joysticks();
return ret;
}

static ALLEGRO_JOYSTICK *joyall_get_joystick(int num)
{
int num_xinput = 0;
int num_dinput = 0;
if (ok_di)
num_dinput = _al_joydrv_directx.num_joysticks();
if (ok_xi)
num_xinput = _al_joydrv_xinput.num_joysticks();
if (num < 0) return NULL;

if (num < num_xinput) {
return _al_joydrv_xinput.get_joystick(num);
}
else if (num < (num_xinput + num_dinput)) {
return _al_joydrv_directx.get_joystick(num - num_xinput);
}
return NULL;
}

static void joyall_release_joystick(ALLEGRO_JOYSTICK *joy)
{




if (joy) {
joy->driver->release_joystick(joy);
}
}

static void joyall_get_joystick_state(ALLEGRO_JOYSTICK *joy, ALLEGRO_JOYSTICK_STATE *ret_state)
{
joy->driver->get_joystick_state(joy, ret_state);
}

static const char *joyall_get_name(ALLEGRO_JOYSTICK *joy)
{
return joy->driver->get_name(joy);
}

static bool joyall_get_active(ALLEGRO_JOYSTICK *joy)
{
return joy->driver->get_active(joy);
}


#endif 
