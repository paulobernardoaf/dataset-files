#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif
#include <vlc_common.h>
#include <vlc_demux.h>
#include <vlc_block.h>
#include <vlc_network.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#if defined(HAVE_POLL)
#include <poll.h>
#endif
#if defined(HAVE_SYS_UIO_H)
#include <sys/uio.h>
#endif
#include "rtp.h"
#if defined(HAVE_SRTP)
#include "srtp.h"
#endif
#define DEFAULT_MRU (1500u - (20 + 8))
static void rtp_process (demux_t *demux, block_t *block)
{
demux_sys_t *sys = demux->p_sys;
if (block->i_buffer < 2)
goto drop;
const uint8_t ptype = rtp_ptype (block);
if (ptype >= 72 && ptype <= 76)
goto drop; 
#if defined(HAVE_SRTP)
if (sys->srtp != NULL)
{
size_t len = block->i_buffer;
if (srtp_recv (sys->srtp, block->p_buffer, &len))
{
msg_Dbg (demux, "SRTP authentication/decryption failed");
goto drop;
}
block->i_buffer = len;
}
#endif
if (unlikely(sys->autodetect))
{ 
rtp_autodetect (demux, sys->session, block);
sys->autodetect = false;
}
rtp_queue (demux, sys->session, block);
return;
drop:
block_Release (block);
}
static int rtp_timeout (vlc_tick_t deadline)
{
if (deadline == VLC_TICK_INVALID)
return -1; 
vlc_tick_t t = vlc_tick_now ();
if (t >= deadline)
return 0;
t = (deadline - t) / (CLOCK_FREQ / INT64_C(1000));
if (unlikely(t > INT_MAX))
return INT_MAX;
return t;
}
void *rtp_dgram_thread (void *opaque)
{
demux_t *demux = opaque;
demux_sys_t *sys = demux->p_sys;
vlc_tick_t deadline = VLC_TICK_INVALID;
int rtp_fd = sys->fd;
#if defined(__linux__)
const int trunc_flag = MSG_TRUNC;
#else
const int trunc_flag = 0;
#endif
struct iovec iov =
{
.iov_len = DEFAULT_MRU,
};
struct msghdr msg =
{
.msg_iov = &iov,
.msg_iovlen = 1,
};
struct pollfd ufd[1];
ufd[0].fd = rtp_fd;
ufd[0].events = POLLIN;
for (;;)
{
int n = poll (ufd, 1, rtp_timeout (deadline));
if (n == -1)
continue;
int canc = vlc_savecancel ();
if (n == 0)
goto dequeue;
if (ufd[0].revents)
{
n--;
if (unlikely(ufd[0].revents & POLLHUP))
break; 
block_t *block = block_Alloc (iov.iov_len);
if (unlikely(block == NULL))
{
if (iov.iov_len == DEFAULT_MRU)
break; 
iov.iov_len = DEFAULT_MRU;
continue; 
}
iov.iov_base = block->p_buffer;
msg.msg_flags = trunc_flag;
ssize_t len = recvmsg (rtp_fd, &msg, trunc_flag);
if (len != -1)
{
if (msg.msg_flags & trunc_flag)
{
msg_Err(demux, "%zd bytes packet truncated (MRU was %zu)",
len, iov.iov_len);
block->i_flags |= BLOCK_FLAG_CORRUPTED;
iov.iov_len = len;
}
else
block->i_buffer = len;
rtp_process (demux, block);
}
else
{
msg_Warn (demux, "RTP network error: %s",
vlc_strerror_c(errno));
block_Release (block);
}
}
dequeue:
if (!rtp_dequeue (demux, sys->session, &deadline))
deadline = VLC_TICK_INVALID;
vlc_restorecancel (canc);
}
return NULL;
}
void *rtp_stream_thread (void *opaque)
{
#if !defined(_WIN32)
demux_t *demux = opaque;
demux_sys_t *sys = demux->p_sys;
int fd = sys->fd;
for (;;)
{
ssize_t val;
uint16_t frame_len;
if (recv (fd, &frame_len, 2, MSG_WAITALL) != 2)
break;
block_t *block = block_Alloc (ntohs (frame_len));
if (unlikely(block == NULL))
break;
block_cleanup_push (block);
val = recv (fd, block->p_buffer, block->i_buffer, MSG_WAITALL);
vlc_cleanup_pop ();
if (val != (ssize_t)block->i_buffer)
{
block_Release (block);
break;
}
int canc = vlc_savecancel ();
rtp_process (demux, block);
rtp_dequeue_force (demux, sys->session);
vlc_restorecancel (canc);
}
#else
(void) opaque;
#endif
return NULL;
}
