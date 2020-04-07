#include <float.h>
#include <termios.h>
#if !defined(PROTO)
#include <kernel/OS.h>
#endif
#include "vim.h"
#if USE_THREAD_FOR_INPUT_WITH_TIMEOUT
#if defined(PROTO)
#define sem_id int
#define thread_id int
#endif
char_u charbuf;
signed char charcount;
sem_id character_present;
sem_id character_wanted;
thread_id read_thread_id;
#define TRY_ABORT 0 
#if TRY_ABORT
static void
mostly_ignore(int sig)
{
}
#endif
static long
read_thread(void *dummy)
{
signal(SIGINT, SIG_IGN);
signal(SIGQUIT, SIG_IGN);
#if TRY_ABORT
signal(SIGUSR1, mostly_ignore);
#endif
for (;;) {
if (acquire_sem(character_wanted) != B_NO_ERROR)
break;
charcount = read(read_cmd_fd, &charbuf, 1);
release_sem(character_present);
}
return 0;
}
void
beos_cleanup_read_thread(void)
{
if (character_present > 0)
delete_sem(character_present);
character_present = 0;
if (read_thread_id > 0)
kill_thread(read_thread_id);
read_thread_id = 0;
}
#endif
int
beos_select(int nbits,
struct fd_set *rbits,
struct fd_set *wbits,
struct fd_set *ebits,
struct timeval *timeout)
{
bigtime_t tmo;
if (nbits == 0) {
snooze(timeout->tv_sec * 1e6 + timeout->tv_usec);
return 0;
}
#if 0
if (FD_ISSET(0, rbits)) {
char cbuf[1];
int count;
struct termios told;
struct termios tnew;
tcgetattr(0, &told);
tnew = told;
tnew.c_lflag &= ~ICANON;
tnew.c_cc[VMIN] = 0;
tnew.c_cc[VTIME] = timeout->tv_sec * 10 + timeout->tv_usec / 100000;
tcsetattr(0, TCSANOW, &tnew);
count = read(0, &cbuf, sizeof(cbuf));
tcsetattr(0, TCSANOW, &told);
if (count > 0) {
add_to_input_buf(&cbuf[0], count);
return 1;
}
return 0;
}
#endif
#if USE_THREAD_FOR_INPUT_WITH_TIMEOUT
if (FD_ISSET(read_cmd_fd, rbits))
{
int acquired;
if (character_present == 0) {
character_present = create_sem(0, "vim character_present");
character_wanted = create_sem(1, "vim character_wanted");
read_thread_id = spawn_thread(read_thread, "vim async read",
B_NORMAL_PRIORITY, NULL);
atexit(beos_cleanup_read_thread);
resume_thread(read_thread_id);
}
if (timeout) {
tmo = timeout->tv_sec * 1e6 + timeout->tv_usec;
if (tmo == 0)
tmo = 1.0;
}
#if TRY_ABORT
release_sem(character_wanted);
#endif
if (timeout)
acquired = acquire_sem_etc(character_present, 1, B_TIMEOUT, tmo);
else
acquired = acquire_sem(character_present);
if (acquired == B_NO_ERROR) {
if (charcount > 0) {
add_to_input_buf(&charbuf, 1);
#if !TRY_ABORT
release_sem(character_wanted);
#endif
return 1;
} else {
#if !TRY_ABORT
release_sem(character_wanted);
#endif
return 0;
}
}
#if TRY_ABORT
else {
kill(read_thread_id, SIGUSR1);
if (acquired == B_TIMED_OUT)
acquire_sem(character_present);
if (charcount > 0) {
add_to_input_buf(&charbuf, 1);
return 1;
}
return 0;
}
#endif
}
#endif
return 0;
}
