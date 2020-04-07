
















#if !defined(__al_included_allegro5_aintunix_h)
#define __al_included_allegro5_aintunix_h

#include "allegro5/altime.h"
#include "allegro5/path.h"
#include "allegro5/internal/aintern_driver.h"


#if defined(ALLEGRO_WITH_XWINDOWS)
#include <X11/Xlib.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif

ALLEGRO_PATH *_al_unix_get_path(int id);
double _al_unix_get_time(void);
void _al_unix_rest(double seconds);
void _al_unix_init_timeout(ALLEGRO_TIMEOUT *timeout, double seconds);


#if defined(__cplusplus)
}
#endif


#if defined(ALLEGRO_LINUX)
#include "allegro5/platform/aintlnx.h"
#endif











#include "allegro5/platform/aintuthr.h"


#if defined(__cplusplus)
extern "C" {
#endif


void _al_unix_init_time(void);


void _al_unix_start_watching_fd(int fd, void (*callback)(void *), void *cb_data);
void _al_unix_stop_watching_fd(int fd);



#define _ALLEGRO_JOYDRV_LINUX AL_ID('L','N','X','A')

#if defined(ALLEGRO_HAVE_LINUX_INPUT_H)
AL_VAR(struct ALLEGRO_JOYSTICK_DRIVER, _al_joydrv_linux);
#endif



#define _ALLEGRO_HAPDRV_LINUX AL_ID('L','N','X','H')

#if defined(ALLEGRO_HAVE_LINUX_INPUT_H)
AL_VAR(struct ALLEGRO_HAPTIC_DRIVER, _al_hapdrv_linux);
#endif


#if defined(__cplusplus)
}
#endif


#endif
