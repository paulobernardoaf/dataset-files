



















#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_stream.h>
#include <vlc_network.h>
#include <vlc_fs.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#if !defined(_POSIX_SPAWN)
#define _POSIX_SPAWN (-1)
#endif
#include <fcntl.h>
#if (_POSIX_SPAWN >= 0)
#include <spawn.h>
#endif
#include <sys/wait.h>
#include <sys/ioctl.h>
#if defined (__linux__) && defined (HAVE_VMSPLICE)
#include <sys/uio.h>
#include <sys/mman.h>
#else
#undef HAVE_VMSPLICE
#endif
#include <vlc_interrupt.h>

#include <signal.h>

static int OpenGzip (vlc_object_t *);
static int OpenBzip2 (vlc_object_t *);
static int OpenXZ (vlc_object_t *);
static void Close (vlc_object_t *);

vlc_module_begin ()
set_category (CAT_INPUT)
set_subcategory (SUBCAT_INPUT_STREAM_FILTER)
set_capability ("stream_filter", 320)

set_description (N_("LZMA decompression"))
set_callbacks (OpenXZ, Close)

add_submodule ()
set_description (N_("Burrows-Wheeler decompression"))
set_callbacks (OpenBzip2, Close)


add_submodule ()
set_description (N_("gzip decompression"))
set_callbacks (OpenGzip, Close)
vlc_module_end ()

typedef struct
{

int write_fd;


vlc_cond_t wait;
vlc_mutex_t lock;
bool paused;


vlc_thread_t thread;
pid_t pid;

int read_fd;
bool can_pace;
bool can_pause;
vlc_tick_t pts_delay;
} stream_sys_t;

extern char **environ;

static const size_t bufsize = 65536;
#if defined(HAVE_VMSPLICE)
static void cleanup_mmap (void *addr)
{
munmap (addr, bufsize);
}
#endif

static void *Thread (void *data)
{
stream_t *stream = data;
stream_sys_t *p_sys = stream->p_sys;
#if defined(HAVE_VMSPLICE)
const ssize_t page_mask = sysconf (_SC_PAGE_SIZE) - 1;
#endif
int fd = p_sys->write_fd;
bool error = false;
sigset_t set;

sigemptyset(&set);
sigaddset(&set, SIGPIPE);
pthread_sigmask(SIG_BLOCK, &set, NULL);

do
{
ssize_t len;
int canc = vlc_savecancel ();
#if defined(HAVE_VMSPLICE)
unsigned char *buf = mmap (NULL, bufsize, PROT_READ|PROT_WRITE,
MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
if (unlikely(buf == MAP_FAILED))
break;
vlc_cleanup_push (cleanup_mmap, buf);
#else
unsigned char *buf = malloc (bufsize);
if (unlikely(buf == NULL))
break;
vlc_cleanup_push (free, buf);
#endif

vlc_mutex_lock (&p_sys->lock);
while (p_sys->paused) 
vlc_cond_wait (&p_sys->wait, &p_sys->lock);
len = vlc_stream_Read (stream->s, buf, bufsize);
vlc_mutex_unlock (&p_sys->lock);

vlc_restorecancel (canc);
error = len <= 0;

for (ssize_t i = 0, j; i < len; i += j)
{
#if defined(HAVE_VMSPLICE)
if ((len - i) <= page_mask) 
j = write (fd, buf + i, len - i);
else
{
struct iovec iov = {
.iov_base = buf + i,
.iov_len = (len - i) & ~page_mask };

j = vmsplice (fd, &iov, 1, SPLICE_F_GIFT);
}
if (j == -1 && errno == ENOSYS) 
#endif
j = write (fd, buf + i, len - i);
if (j <= 0)
{
if (j == 0)
errno = EPIPE;
msg_Err (stream, "cannot write data: %s",
vlc_strerror_c(errno));
error = true;
break;
}
}
vlc_cleanup_pop ();
#if defined(HAVE_VMSPLICE)
munmap (buf, bufsize);
#else
free (buf);
#endif
}
while (!error);

msg_Dbg (stream, "compressed stream at EOF");

p_sys->write_fd = -1;
vlc_close (fd);
return NULL;
}


#define MIN_BLOCK (1 << 10)
#define MAX_BLOCK (1 << 20)




static ssize_t Read (stream_t *stream, void *buf, size_t buflen)
{
stream_sys_t *sys = stream->p_sys;
ssize_t val = vlc_read_i11e (sys->read_fd, buf, buflen);
return (val >= 0) ? val : 0;
}




static int Control (stream_t *stream, int query, va_list args)
{
stream_sys_t *p_sys = stream->p_sys;

switch (query)
{
case STREAM_CAN_SEEK:
case STREAM_CAN_FASTSEEK:
*(va_arg (args, bool *)) = false;
break;
case STREAM_CAN_PAUSE:
*(va_arg (args, bool *)) = p_sys->can_pause;
break;
case STREAM_CAN_CONTROL_PACE:
*(va_arg (args, bool *)) = p_sys->can_pace;
break;
case STREAM_GET_SIZE:
*(va_arg (args, uint64_t *)) = 0;
break;
case STREAM_GET_PTS_DELAY:
*va_arg (args, vlc_tick_t *) = p_sys->pts_delay;
break;
case STREAM_SET_PAUSE_STATE:
{
bool paused = va_arg (args, unsigned);

vlc_mutex_lock (&p_sys->lock);
vlc_stream_Control(stream->s, STREAM_SET_PAUSE_STATE, paused);
p_sys->paused = paused;
vlc_cond_signal (&p_sys->wait);
vlc_mutex_unlock (&p_sys->lock);
break;
}
default:
return VLC_EGENERIC;
}
return VLC_SUCCESS;
}






static int Open (stream_t *stream, const char *path)
{
stream_sys_t *p_sys = stream->p_sys = malloc (sizeof (*p_sys));
if (p_sys == NULL)
return VLC_ENOMEM;

vlc_cond_init (&p_sys->wait);
vlc_mutex_init (&p_sys->lock);
p_sys->paused = false;
p_sys->pid = -1;
vlc_stream_Control(stream->s, STREAM_CAN_PAUSE, &p_sys->can_pause);
vlc_stream_Control(stream->s, STREAM_CAN_CONTROL_PACE, &p_sys->can_pace);
vlc_stream_Control(stream->s, STREAM_GET_PTS_DELAY, &p_sys->pts_delay);



int ret = VLC_EGENERIC;
int comp[2];



if (vlc_pipe (comp) == 0)
{
p_sys->write_fd = comp[1];

int uncomp[2];
if (vlc_pipe (uncomp) == 0)
{
p_sys->read_fd = uncomp[0];

#if (_POSIX_SPAWN >= 0)
posix_spawn_file_actions_t actions;
if (posix_spawn_file_actions_init (&actions) == 0)
{
char *const argv[] = { (char *)path, NULL };

if (!posix_spawn_file_actions_adddup2 (&actions, comp[0], 0)
&& !posix_spawn_file_actions_adddup2 (&actions, uncomp[1], 1)
&& !posix_spawnp (&p_sys->pid, path, &actions, NULL, argv,
environ))
{
if (vlc_clone (&p_sys->thread, Thread, stream,
VLC_THREAD_PRIORITY_INPUT) == 0)
ret = VLC_SUCCESS;
}
else
{
msg_Err (stream, "cannot execute %s", path);
p_sys->pid = -1;
}
posix_spawn_file_actions_destroy (&actions);
}
#else 
switch (p_sys->pid = fork ())
{
case -1:
msg_Err (stream, "cannot fork: %s", vlc_strerror_c(errno));
break;
case 0:
dup2 (comp[0], 0);
dup2 (uncomp[1], 1);
execlp (path, path, (const char *)NULL);
exit (1); 
default:
if (vlc_clone (&p_sys->thread, Thread, stream,
VLC_THREAD_PRIORITY_INPUT) == 0)
ret = VLC_SUCCESS;
}
#endif 
vlc_close (uncomp[1]);
if (ret != VLC_SUCCESS)
vlc_close (uncomp[0]);
}
vlc_close (comp[0]);
if (ret != VLC_SUCCESS)
vlc_close (comp[1]);
}

if (ret != VLC_SUCCESS)
{
if (p_sys->pid != -1)
while (waitpid (p_sys->pid, &(int){ 0 }, 0) == -1);
free (p_sys);
return ret;
}

stream->pf_read = Read;
stream->pf_seek = NULL;
stream->pf_control = Control;
return VLC_SUCCESS;
}





static void Close (vlc_object_t *obj)
{
stream_t *stream = (stream_t *)obj;
stream_sys_t *p_sys = stream->p_sys;
int status;

vlc_cancel (p_sys->thread);
vlc_close (p_sys->read_fd);
vlc_join (p_sys->thread, NULL);
if (p_sys->write_fd != -1)

vlc_close (p_sys->write_fd);

msg_Dbg (obj, "waiting for PID %u", (unsigned)p_sys->pid);
while (waitpid (p_sys->pid, &status, 0) == -1);
msg_Dbg (obj, "exit status %d", status);

free (p_sys);
}





static int OpenGzip (vlc_object_t *obj)
{
stream_t *stream = (stream_t *)obj;
const uint8_t *peek;

if (vlc_stream_Peek (stream->s, &peek, 3) < 3)
return VLC_EGENERIC;

if (memcmp (peek, "\x1f\x8b\x08", 3))
return VLC_EGENERIC;

msg_Dbg (obj, "detected gzip compressed stream");
return Open (stream, "zcat");
}





static int OpenBzip2 (vlc_object_t *obj)
{
stream_t *stream = (stream_t *)obj;
const uint8_t *peek;


if (vlc_stream_Peek (stream->s, &peek, 10) < 10)
return VLC_EGENERIC;

if (memcmp (peek, "BZh", 3) || (peek[3] < '1') || (peek[3] > '9')
|| memcmp (peek + 4, "\x31\x41\x59\x26\x53\x59", 6))
return VLC_EGENERIC;

msg_Dbg (obj, "detected bzip2 compressed stream");
return Open (stream, "bzcat");
}




static int OpenXZ (vlc_object_t *obj)
{
stream_t *stream = (stream_t *)obj;
const uint8_t *peek;


if (vlc_stream_Peek (stream->s, &peek, 8) < 8)
return VLC_EGENERIC;

if (memcmp (peek, "\xfd\x37\x7a\x58\x5a", 6))
return VLC_EGENERIC;

msg_Dbg (obj, "detected xz compressed stream");
return Open (stream, "xzcat");
}
