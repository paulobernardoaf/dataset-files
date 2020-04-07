#error This driver has not been updated to the new mouse API.


















#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/time.h>

#include "allegro5/allegro.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/platform/aintunix.h"
#include "allegro5/linalleg.h"


#define ASCII_NAME "GPM repeater"
#define DEVICE_FILENAME "/dev/gpmdata"















static int processor (unsigned char *buf, int buf_size)
{
int r, m, l, x, y, z;

if (buf_size < 5)
return 0; 

if ((buf[0] & 0xf8) != 0x80)
return 1; 


r = !(buf[0] & 1);
m = !(buf[0] & 2);
l = !(buf[0] & 4);
x = (signed char)buf[1] + (signed char)buf[3];
y = (signed char)buf[2] + (signed char)buf[4];
z = 0;
__al_linux_mouse_handler(x, y, z, l+(r<<1)+(m<<2));
return 5; 
}

static INTERNAL_MOUSE_DRIVER intdrv = {
-1,
processor,
3
};





static void sync_mouse (int fd)
{
fd_set set;
int result;
struct timeval tv;
char bitbucket;

do {
FD_ZERO (&set);
FD_SET (fd, &set);
tv.tv_sec = tv.tv_usec = 0;
result = select (FD_SETSIZE, &set, NULL, NULL, &tv);
if (result > 0) read (fd, &bitbucket, 1);
} while (result > 0);
}





static int mouse_init (void)
{
char tmp1[128], tmp2[128], tmp3[128];
const char *udevice;


udevice = get_config_string (uconvert_ascii ("mouse", tmp1),
uconvert_ascii ("mouse_device", tmp2),
uconvert_ascii (DEVICE_FILENAME, tmp3));


intdrv.device = open (uconvert_toascii (udevice, tmp1), O_RDONLY | O_NONBLOCK);
if (intdrv.device < 0) {
uszprintf (allegro_error, ALLEGRO_ERROR_SIZE, get_config_text ("Unable to open %s: %s"),
udevice, ustrerror (errno));
return -1;
}


sync_mouse (intdrv.device);

return __al_linux_mouse_init (&intdrv);
}




static void mouse_exit (void)
{
__al_linux_mouse_exit();
close (intdrv.device);
}

MOUSE_DRIVER mousedrv_linux_gpmdata =
{
MOUSEDRV_LINUX_GPMDATA,
"",
"",
ASCII_NAME,
mouse_init,
mouse_exit,
NULL, 
NULL, 
__al_linux_mouse_position,
__al_linux_mouse_set_range,
__al_linux_mouse_set_speed,
__al_linux_mouse_get_mickeys,
NULL, 
NULL,
NULL
};

