#error This file is no longer used.


















#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#include "allegro5/allegro.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/platform/aintunix.h"
#include "allegro5/linalleg.h"

#include <unistd.h>



STD_DRIVER *__al_linux_std_drivers[N_STD_DRIVERS];

static int std_drivers_suspended = false;






int __al_linux_add_standard_driver (STD_DRIVER *spec)
{
if (!spec) return 1;
if (spec->type >= N_STD_DRIVERS) return 2;
if (!spec->update) return 3;
if (spec->fd < 0) return 4;

__al_linux_std_drivers[spec->type] = spec;

spec->resume();

return 0;
}





int __al_linux_remove_standard_driver (STD_DRIVER *spec)
{
if (!spec) return 1;
if (spec->type >= N_STD_DRIVERS) return 3;
if (!__al_linux_std_drivers[spec->type]) return 4; 
if (__al_linux_std_drivers[spec->type] != spec) return 5;

spec->suspend();

__al_linux_std_drivers[spec->type] = NULL;

return 0;
}





void __al_linux_update_standard_drivers (int threaded)
{
int i;
if (!std_drivers_suspended) {
for (i = 0; i < N_STD_DRIVERS; i++)
if (__al_linux_std_drivers[i])
__al_linux_std_drivers[i]->update();
}
}





void __al_linux_suspend_standard_drivers (void)
{
ASSERT(!std_drivers_suspended);
std_drivers_suspended = true;
}




void __al_linux_resume_standard_drivers (void)
{
ASSERT(std_drivers_suspended);
std_drivers_suspended = false;
}

