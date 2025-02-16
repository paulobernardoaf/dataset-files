#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif
#include "allegro5/allegro.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/platform/aintunix.h"
#include "allegro5/linalleg.h"
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
static SIGIO_HOOK user_sigio_hook = NULL;
#define DEFAULT_ASYNC_IO_MODE ASYNC_BSD
unsigned __al_linux_async_io_mode = 0;
SIGIO_HOOK al_linux_install_sigio_hook (SIGIO_HOOK hook)
{
SIGIO_HOOK ret = user_sigio_hook;
user_sigio_hook = hook;
return ret;
}
static void async_io_event(int signo)
{
if (__al_linux_std_drivers[STD_MOUSE]) __al_linux_std_drivers[STD_MOUSE]->update();
if (__al_linux_std_drivers[STD_KBD]) __al_linux_std_drivers[STD_KBD]->update();
if (user_sigio_hook) user_sigio_hook(SIGIO);
return;
}
int al_linux_set_async_mode (unsigned type)
{
static struct sigaction org_sigio;
struct sigaction sa;
if (type == ASYNC_DEFAULT)
type = DEFAULT_ASYNC_IO_MODE;
__al_linux_async_set_drivers (__al_linux_async_io_mode, 0);
switch (__al_linux_async_io_mode) {
case ASYNC_BSD:
sigaction (SIGIO, &org_sigio, NULL);
break;
}
__al_linux_async_io_mode = type;
switch (__al_linux_async_io_mode) {
case ASYNC_BSD:
sa.sa_flags = SA_RESTART;
sa.sa_handler = async_io_event;
sigfillset (&sa.sa_mask);
sigaction (SIGIO, &sa, &org_sigio);
break;
}
__al_linux_async_set_drivers (__al_linux_async_io_mode, 1);
return 0;
}
int al_linux_is_async_mode (void)
{
return (__al_linux_async_io_mode != ASYNC_OFF);
}
