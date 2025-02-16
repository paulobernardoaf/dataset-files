

















#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#include "allegro5/allegro.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/internal/aintern_driver.h"
#include "allegro5/platform/aintunix.h"
#include "allegro5/platform/aintlnx.h"

#include "allegro5/linalleg.h"

#if !defined(ALLEGRO_LINUX)
#error Something is wrong with the makefile
#endif


static int sys_linux_init(void);
static void sys_linux_exit(void);
static void sys_linux_message (const char *msg);
static void sys_linux_save_console_state(void);
static void sys_linux_restore_console_state(void);



#define make_getter(x,y) static _AL_DRIVER_INFO *get_##y##_driver_list (void) { return x##_##y##_driver_list; }
make_getter (_unix, gfx)
make_getter (_unix, digi)
make_getter (_unix, midi)
make_getter (_al_linux, keyboard)
make_getter (_al_linux, mouse)
make_getter (_al_linux, joystick)
#undef make_getter



SYSTEM_DRIVER system_linux =
{
SYSTEM_LINUX,
"",
"",
"Linux console",
sys_linux_init,
sys_linux_exit,
_unix_get_executable_name,
_unix_get_path,
_unix_find_resource,
NULL, 
NULL, 
sys_linux_message,
NULL, 
sys_linux_save_console_state,
sys_linux_restore_console_state,
NULL, 
NULL, 
NULL, 
NULL, 
NULL, 
NULL, 
NULL, 
__al_linux_set_display_switch_mode,
__al_linux_display_switch_lock,
NULL, 
NULL, 
NULL, 
_unix_yield_timeslice,
get_gfx_driver_list,
get_digi_driver_list,
get_midi_driver_list,
get_keyboard_driver_list,
get_mouse_driver_list,
get_joystick_driver_list
};


int __al_linux_have_ioperms = 0;


typedef void (*temp_sighandler_t)(int);
static temp_sighandler_t old_sig_abrt, old_sig_fpe, old_sig_ill, old_sig_segv, old_sig_term, old_sig_int, old_sig_quit;





static void signal_handler (int num)
{
al_uninstall_system();
fprintf (stderr, "Shutting down Allegro due to signal #%d\n", num);
raise (num);
}





static int __al_linux_bgman_init (void)
{
_unix_bg_man = &_bg_man_pthreads;
if (_unix_bg_man->init())
return -1;




return 0;
}





static void __al_linux_bgman_exit (void)
{
_unix_bg_man->exit();
}






static int sys_linux_init (void)
{

_unix_read_os_type();
if (os_type != OSTYPE_LINUX) return -1; 




__al_linux_have_ioperms = !seteuid (0);
__al_linux_have_ioperms &= !__al_linux_init_memory();


seteuid (getuid());


old_sig_abrt = signal(SIGABRT, signal_handler);
old_sig_fpe = signal(SIGFPE, signal_handler);
old_sig_ill = signal(SIGILL, signal_handler);
old_sig_segv = signal(SIGSEGV, signal_handler);
old_sig_term = signal(SIGTERM, signal_handler);
old_sig_int = signal(SIGINT, signal_handler);
#if defined(SIGQUIT)
old_sig_quit = signal(SIGQUIT, signal_handler);
#endif


if (__al_linux_bgman_init()) {

sys_linux_exit();
return -1;
}


_al_unix_init_time();

return 0;
}






static void sys_linux_exit (void)
{

__al_linux_bgman_exit();


signal(SIGABRT, old_sig_abrt);
signal(SIGFPE, old_sig_fpe);
signal(SIGILL, old_sig_ill);
signal(SIGSEGV, old_sig_segv);
signal(SIGTERM, old_sig_term);
signal(SIGINT, old_sig_int);
#if defined(SIGQUIT)
signal(SIGQUIT, old_sig_quit);
#endif

__al_linux_shutdown_memory();
}



static void sys_linux_save_console_state(void)
{
__al_linux_use_console();
}



static void sys_linux_restore_console_state(void)
{
__al_linux_leave_console();
}






static void sys_linux_message (const char *msg)
{
char *tmp;
int ret;
ASSERT(msg);

tmp = al_malloc(ALLEGRO_MESSAGE_SIZE);
msg = uconvert(msg, U_UTF8, tmp, U_ASCII, ALLEGRO_MESSAGE_SIZE);

do {
ret = write(STDERR_FILENO, msg, strlen(msg));
if ((ret < 0) && (errno != EINTR))
break;
} while (ret < (int)strlen(msg));

__al_linux_got_text_message = true;

al_free(tmp);
}



