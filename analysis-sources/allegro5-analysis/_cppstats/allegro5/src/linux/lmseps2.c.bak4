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


static int intellimouse;
static int packet_size;






















static int processor (unsigned char *buf, int buf_size)
{
int r, l, m, x, y, z;

if (buf_size < packet_size) return 0; 


if (intellimouse) {
if ((buf[0] & 0xc8) != 0x08) return 1; 
}
else {
if ((buf[0] & 0xc0) != 0x00) return 1; 
}


l = !!(buf[0] & 1);
r = !!(buf[0] & 2);
m = !!(buf[0] & 4);

x = buf[1];
y = buf[2];
if (buf[0] & 0x10) x -= 256;
if (buf[0] & 0x20) y -= 256;

if (intellimouse) {
z = buf[3] & 0xf;
if (z) 
z = (z-7) >> 3;
}
else
z = 0;

__al_linux_mouse_handler(x, y, z, l+(r<<1)+(m<<2));
return packet_size; 
}






static int analyse_data (const char *buffer, int size)
{
int pos = 0;
int packets = 0, errors = 0;

int step = 0;

for (pos = 0; pos < size; pos++)
switch (step) {
case 3:
packets++;
step = 0;
case 0:
if (buffer[pos] & 0xC0) {
errors++;
} else {
step++;
}
break;

case 1:
case 2:
step++;
break;
}

return (errors <= 5) || (errors < size / 20); 
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




static void wakeup_im (int fd)
{
unsigned char init[] = { 243, 200, 243, 100, 243, 80 };
int ret;
do {
ret = write (fd, init, sizeof (init));
if ((ret < 0) && (errno != EINTR))
break;
} while (ret < (int)sizeof (init));
}





static int mouse_init (void)
{
char tmp1[128], tmp2[128];
const char *udevice;
int flags;

static const char * const default_devices[] = {
"/dev/mouse",
"/dev/input/mice",
};


udevice = get_config_string (uconvert_ascii ("mouse", tmp1),
uconvert_ascii ("mouse_device", tmp2),
NULL);


flags = O_NONBLOCK | (intellimouse ? O_RDWR : O_RDONLY);
if (udevice) {
intdrv.device = open (uconvert_toascii (udevice, tmp1), flags);
}
else {
size_t n;
for (n = 0; n < sizeof(default_devices) / sizeof(default_devices[0]); n++) {
intdrv.device = open (default_devices[n], flags);
if (intdrv.device >= 0)
break;
}
}

if (intdrv.device < 0) {
uszprintf (allegro_error, ALLEGRO_ERROR_SIZE, get_config_text ("Unable to open %s: %s"),
udevice ? udevice : get_config_text("one of the default mice devices"),
ustrerror (errno));
return -1;
}


if (intellimouse)
wakeup_im (intdrv.device);


sync_mouse (intdrv.device);

return __al_linux_mouse_init (&intdrv);
}




static int ps2_mouse_init (void)
{
intellimouse = false;
packet_size = 3;
return mouse_init ();
}




static int ips2_mouse_init (void)
{
intellimouse = true;
packet_size = 4;
return mouse_init ();
}




static void mouse_exit (void)
{
__al_linux_mouse_exit();
close (intdrv.device);
}

MOUSE_DRIVER mousedrv_linux_ps2 =
{
MOUSEDRV_LINUX_PS2,
"",
"",
"Linux PS/2 mouse",
ps2_mouse_init,
mouse_exit,
NULL, 
NULL, 
__al_linux_mouse_position,
__al_linux_mouse_set_range,
__al_linux_mouse_set_speed,
__al_linux_mouse_get_mickeys,
analyse_data,
NULL, 
NULL
};

MOUSE_DRIVER mousedrv_linux_ips2 =
{
MOUSEDRV_LINUX_IPS2,
"",
"",
"Linux PS/2 Intellimouse",
ips2_mouse_init,
mouse_exit,
NULL, 
NULL, 
__al_linux_mouse_position,
__al_linux_mouse_set_range,
__al_linux_mouse_set_speed,
__al_linux_mouse_get_mickeys,
analyse_data,
NULL, 
NULL
};
