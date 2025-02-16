
















#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#include <signal.h>
#include <sys/ioctl.h>
#include <linux/vt.h>

#include "allegro5/allegro.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/platform/aintunix.h"
#include "allegro5/linalleg.h"

#if defined(ALLEGRO_HAVE_MMAP)
#include <sys/mman.h>
#endif

#if !defined(_POSIX_MAPPED_FILES) || !defined(ALLEGRO_HAVE_MMAP)
#error "Sorry, mapped files are required for Allegro/Linux to work!"
#endif


static int switch_mode = SWITCH_PAUSE;

static int vtswitch_initialised = 0;
static struct vt_mode startup_vtmode;

volatile int __al_linux_switching_blocked = 0;

static volatile int console_active = 1; 
static volatile int console_should_be_active = 1; 



int __al_linux_set_display_switch_mode (int mode)
{

if (switch_mode == SWITCH_NONE) __al_linux_switching_blocked--;

switch_mode = mode;


if (switch_mode == SWITCH_NONE) __al_linux_switching_blocked++;

return 0;
}






static void go_away(void)
{
_switch_out();

_unix_bg_man->disable_interrupts();
if ((switch_mode == SWITCH_PAUSE) || (switch_mode == SWITCH_AMNESIA))
_al_suspend_old_timer_emulation();




_save_switch_state(switch_mode);

if (gfx_driver && gfx_driver->save_video_state)
gfx_driver->save_video_state();

ioctl (__al_linux_console_fd, VT_RELDISP, 1); 
console_active = 0; 

__al_linux_switching_blocked--;

if ((switch_mode == SWITCH_PAUSE) || (switch_mode == SWITCH_AMNESIA)) {
__al_linux_wait_for_display();
_al_resume_old_timer_emulation();
}

_unix_bg_man->enable_interrupts();
}






static void come_back(void)
{
_unix_bg_man->disable_interrupts();

if (gfx_driver && gfx_driver->restore_video_state)
gfx_driver->restore_video_state();

_restore_switch_state();

ioctl(__al_linux_console_fd, VT_RELDISP, VT_ACKACQ);
console_active = 1;



_unix_bg_man->enable_interrupts();

_switch_in();

__al_linux_switching_blocked--;
}







static void poll_console_switch (void)
{
if (console_active == console_should_be_active) return;
if (__al_linux_switching_blocked) return;

__al_linux_switching_blocked++;

if (console_should_be_active)
come_back();
else
go_away();
}







static void vt_switch_requested(int signo)
{
switch (signo) {
case SIGRELVT:
console_should_be_active = 0;
break;
case SIGACQVT:
console_should_be_active = 1;
break;
default:
return;
}
poll_console_switch();
}









void __al_linux_acquire_bitmap(BITMAP *bmp)
{
ASSERT(bmp);

__al_linux_switching_blocked++;
}






void __al_linux_release_bitmap(BITMAP *bmp)
{
ASSERT(bmp);

__al_linux_switching_blocked--;
poll_console_switch();
}







void __al_linux_display_switch_lock(int lock, int foreground)
{
if (__al_linux_console_fd == -1) {
return;
}

if (foreground) {
__al_linux_wait_for_display();
}

if (lock) {
__al_linux_switching_blocked++;
}
else {
__al_linux_switching_blocked--;
poll_console_switch();
}
}







int __al_linux_init_vtswitch(void)
{
struct sigaction sa;
struct vt_mode vtm;

if (vtswitch_initialised) return 0; 

__al_linux_switching_blocked = (switch_mode == SWITCH_NONE) ? 1 : 0;
console_active = console_should_be_active = 1;


sigemptyset(&sa.sa_mask);
sigaddset(&sa.sa_mask, SIGIO); 
sa.sa_flags = 0;
sa.sa_handler = vt_switch_requested;
if ((sigaction(SIGRELVT, &sa, NULL) < 0) || (sigaction(SIGACQVT, &sa, NULL) < 0)) {
ustrzcpy (allegro_error, ALLEGRO_ERROR_SIZE, get_config_text ("Unable to control VT switching"));
return 1;
}



ioctl(__al_linux_console_fd, VT_GETMODE, &startup_vtmode);
vtm = startup_vtmode;

vtm.mode = VT_PROCESS;
vtm.relsig = SIGRELVT;
vtm.acqsig = SIGACQVT;

ioctl(__al_linux_console_fd, VT_SETMODE, &vtm);

vtswitch_initialised = 1;

return 0;
}






int __al_linux_done_vtswitch(void)
{
struct sigaction sa;

if (!vtswitch_initialised) return 0; 


ioctl(__al_linux_console_fd, VT_SETMODE, &startup_vtmode);

sigemptyset (&sa.sa_mask);
sa.sa_handler = SIG_DFL;
sa.sa_flags = SA_RESTART;
sigaction (SIGRELVT, &sa, NULL);
sigaction (SIGACQVT, &sa, NULL);

vtswitch_initialised = 0;

return 0;
}


