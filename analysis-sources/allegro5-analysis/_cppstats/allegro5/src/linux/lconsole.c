#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif
#include "allegro5/allegro.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/platform/aintunix.h"
#include "allegro5/linalleg.h"
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <linux/vt.h>
#include <linux/kd.h>
#include <sys/mman.h>
#include <sys/stat.h>
int __al_linux_vt = -1;
int __al_linux_console_fd = -1;
int __al_linux_prev_vt = -1;
int __al_linux_got_text_message = false;
struct termios __al_linux_startup_termio;
struct termios __al_linux_work_termio;
static int get_tty (int fd)
{
char name[16];
int tty;
ino_t inode;
struct stat st;
if (fstat (fd, &st))
return -1;
inode = st.st_ino;
for (tty = 1; tty <= 24; tty++) {
snprintf (name, sizeof(name), "/dev/tty%d", tty);
name[sizeof(name)-1] = 0;
if (!stat (name, &st) && (inode == st.st_ino))
break;
}
return (tty <= 24) ? tty : 0;
}
static int init_console(void)
{
char tmp[256];
__al_linux_vt = get_tty(STDIN_FILENO);
if (__al_linux_vt < 0) {
uszprintf (allegro_error, ALLEGRO_ERROR_SIZE, get_config_text("Error finding our VT: %s"), ustrerror(errno));
return 1;
}
if (__al_linux_vt != 0) {
if ((__al_linux_console_fd = open("/dev/tty", O_RDWR)) < 0) {
uszprintf (allegro_error, ALLEGRO_ERROR_SIZE, get_config_text("Unable to open %s: %s"),
uconvert_ascii("/dev/tty", tmp), ustrerror(errno));
return 1;
}
} else {
int tty, console_fd, fd, child;
unsigned short mask;
char tty_name[16];
struct vt_stat vts; 
if ((console_fd = open ("/dev/console", O_WRONLY)) < 0) {
int n;
uszprintf (allegro_error, ALLEGRO_ERROR_SIZE, uconvert_ascii("%s /dev/console: %s", tmp),
get_config_text("Unable to open"), ustrerror (errno));
for (n = 1; n <= 24; n++) {
snprintf (tty_name, sizeof(tty_name), "/dev/tty%d", n);
tty_name[sizeof(tty_name)-1] = 0;
if ((console_fd = open (tty_name, O_WRONLY)) >= 0)
break;
}
if (n > 24)
return 1; 
}
if (ioctl (console_fd, VT_GETSTATE, &vts)) {
uszprintf (allegro_error, ALLEGRO_ERROR_SIZE, uconvert_ascii("VT_GETSTATE: %s", tmp), ustrerror (errno));
close (console_fd);
return 1;
}
__al_linux_prev_vt = vts.v_active;
seteuid(0);
fd = -1;
for (tty = 1, mask = 2; mask; tty++, mask <<= 1) {
if (!(vts.v_state & mask)) {
snprintf (tty_name, sizeof(tty_name), "/dev/tty%d", tty);
tty_name[sizeof(tty_name)-1] = 0;
if ((fd = open (tty_name, O_RDWR)) != -1) {
close (fd);
break;
}
}
}
seteuid (getuid());
if (!mask) {
ustrzcpy (allegro_error, ALLEGRO_ERROR_SIZE, get_config_text ("Unable to find a usable VT"));
close (console_fd);
return 1;
}
child = fork();
if (child < 0) {
uszprintf (allegro_error, ALLEGRO_ERROR_SIZE, uconvert_ascii ("fork: %s", tmp), ustrerror (errno));
close (console_fd);
return 1;
}
if (child) {
fprintf (stderr, "Allegro application is running on VT %d\n", tty);
exit (0);
}
close (console_fd);
ioctl (0, TIOCNOTTY, 0);
setsid();
seteuid(0);
fd = open (tty_name, O_RDWR);
seteuid(getuid());
if (fd == -1) {
ustrzcpy (allegro_error, ALLEGRO_ERROR_SIZE, get_config_text ("Unable to reopen new console"));
return 1;
}
ioctl (fd, VT_ACTIVATE, tty);
__al_linux_vt = tty;
__al_linux_console_fd = fd;
if (__al_linux_wait_for_display()) {
close (fd);
ustrzcpy (allegro_error, ALLEGRO_ERROR_SIZE, get_config_text ("VT_WAITACTIVE failure"));
return 1;
}
if (isatty(0)) dup2 (fd, 0);
if (isatty(1)) dup2 (fd, 1);
if (isatty(2)) dup2 (fd, 2);
}
tcgetattr(__al_linux_console_fd, &__al_linux_startup_termio);
__al_linux_work_termio = __al_linux_startup_termio;
return 0;
}
static int done_console (void)
{
char msg[256];
int ret;
if (__al_linux_prev_vt >= 0) {
if (__al_linux_got_text_message) {
snprintf(msg, sizeof(msg), "\nProgram finished: press %s+F%d to switch back to the previous console\n", 
(__al_linux_prev_vt > 12) ? "AltGR" : "Alt", 
__al_linux_prev_vt%12);
msg[sizeof(msg)-1] = 0;
do {
ret = write(STDERR_FILENO, msg, strlen(msg));
if ((ret < 0) && (errno != EINTR))
break;
} while (ret < (int)strlen(msg));
__al_linux_got_text_message = false;
}
else
ioctl (__al_linux_console_fd, VT_ACTIVATE, __al_linux_prev_vt);
__al_linux_prev_vt = -1;
}
tcsetattr (__al_linux_console_fd, TCSANOW, &__al_linux_startup_termio);
close (__al_linux_console_fd);
__al_linux_console_fd = -1;
return 0;
}
static int console_users = 0;
int __al_linux_use_console(void)
{
console_users++;
if (console_users > 1)
return 0;
if (init_console()) {
console_users--;
return 1;
}
set_display_switch_mode (SWITCH_PAUSE);
return __al_linux_init_vtswitch();
}
int __al_linux_leave_console(void)
{
ASSERT (console_users > 0);
console_users--;
if (console_users > 0)
return 0;
if (__al_linux_done_vtswitch())
return 1;
if (done_console())
return 1;
return 0;
}
static int graphics_mode = 0;
int __al_linux_console_graphics (void)
{
if (__al_linux_use_console())
return 1;
if (graphics_mode)
return 0; 
ioctl(__al_linux_console_fd, KDSETMODE, KD_GRAPHICS);
__al_linux_wait_for_display();
graphics_mode = 1;
return 0;
}
int __al_linux_console_text (void)
{
int ret;
if (!graphics_mode)
return 0; 
ioctl(__al_linux_console_fd, KDSETMODE, KD_TEXT);
do {
ret = write(__al_linux_console_fd, "\e[H\e[J\e[0m", 10);
if ((ret < 0) && (errno != EINTR))
break;
} while (ret < 10);
graphics_mode = 0;
__al_linux_leave_console();
return 0;
}
int __al_linux_wait_for_display (void)
{
int x;
do {
x = ioctl (__al_linux_console_fd, VT_WAITACTIVE, __al_linux_vt);
} while (x && errno != EINTR);
return x;
}
