#error This driver has not been updated to the new mouse API.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/time.h>
#include <termios.h>
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
if (!(buf[0] & 0x40)) return 1; 
if (buf[1] & 0x40) return 1; 
if (buf[2] & 0x40) return 2; 
l = !!(buf[0] & 0x20);
r = !!(buf[0] & 0x10);
if (intellimouse) {
m = !!(buf[3] & 0x10);
z = (buf[3] & 0x0f);
if (z) 
z = (z-7) >> 3;
}
else {
m = 0;
z = 0;
}
x = (signed char) (((buf[0] & 0x03) << 6) | (buf[1] & 0x3F));
y = -(signed char) (((buf[0] & 0x0C) << 4) | (buf[2] & 0x3F));
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
if (!(buffer[pos] & 0x40)) {
errors++;
} else {
step++;
}
break;
case 1:
case 2:
if (buffer[pos] & 0x40) {
errors++;
step = 0;
pos--;
} else {
step++;
}
break;
}
return (errors <= 5) || (errors < size / 20); 
}
static INTERNAL_MOUSE_DRIVER intdrv = {
-1,
processor,
2
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
struct termios t;
udevice = get_config_string (uconvert_ascii ("mouse", tmp1),
uconvert_ascii ("mouse_device", tmp2),
uconvert_ascii ("/dev/mouse", tmp3));
intdrv.device = open (uconvert_toascii (udevice, tmp1), O_RDONLY | O_NONBLOCK);
if (intdrv.device < 0) {
uszprintf (allegro_error, ALLEGRO_ERROR_SIZE, get_config_text ("Unable to open %s: %s"),
udevice, ustrerror (errno));
return -1;
}
tcgetattr (intdrv.device, &t);
t.c_iflag = IGNBRK | IGNPAR;
t.c_oflag = t.c_lflag = t.c_line = 0;
t.c_cflag = CS7 | CREAD | CLOCAL | HUPCL | B1200;
tcsetattr (intdrv.device, TCSAFLUSH, &t);
sync_mouse (intdrv.device);
return __al_linux_mouse_init (&intdrv);
}
static int ms_mouse_init (void)
{
intellimouse = false;
packet_size = 3;
intdrv.num_buttons = 2;
return mouse_init ();
}
static int ims_mouse_init (void)
{
intellimouse = true;
packet_size = 4;
intdrv.num_buttons = 3;
return mouse_init ();
}
static void mouse_exit (void)
{
__al_linux_mouse_exit();
close (intdrv.device);
}
MOUSE_DRIVER mousedrv_linux_ms =
{
MOUSEDRV_LINUX_MS,
"",
"",
"Linux MS mouse",
ms_mouse_init,
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
MOUSE_DRIVER mousedrv_linux_ims =
{
MOUSEDRV_LINUX_IMS,
"",
"",
"Linux MS Intellimouse",
ims_mouse_init,
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
