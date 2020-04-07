#include <vlc_threads.h>
#if !defined(_WIN32)
#include <sys/socket.h> 
#else
#include <ws2tcpip.h>
#endif
struct pollfd;
struct iovec;
struct sockaddr;
struct msghdr;
VLC_API int vlc_sem_wait_i11e(vlc_sem_t *);
VLC_API int vlc_mwait_i11e(vlc_tick_t);
static inline int vlc_msleep_i11e(vlc_tick_t delay)
{
return vlc_mwait_i11e(vlc_tick_now() + delay);
}
VLC_API int vlc_poll_i11e(struct pollfd *, unsigned, int);
VLC_API ssize_t vlc_readv_i11e(int fd, struct iovec *, int);
VLC_API ssize_t vlc_writev_i11e(int fd, const struct iovec *, int);
VLC_API ssize_t vlc_read_i11e(int fd, void *, size_t);
VLC_API ssize_t vlc_write_i11e(int fd, const void *, size_t);
VLC_API ssize_t vlc_recvmsg_i11e(int fd, struct msghdr *, int flags);
VLC_API ssize_t vlc_sendmsg_i11e(int fd, const struct msghdr *, int flags);
VLC_API ssize_t vlc_recvfrom_i11e(int fd, void *, size_t, int flags,
struct sockaddr *, socklen_t *);
VLC_API ssize_t vlc_sendto_i11e(int fd, const void *, size_t, int flags,
const struct sockaddr *, socklen_t);
static inline ssize_t vlc_recv_i11e(int fd, void *buf, size_t len, int flags)
{
return vlc_recvfrom_i11e(fd, buf, len, flags, NULL, NULL);
}
static inline
ssize_t vlc_send_i11e(int fd, const void *buf, size_t len, int flags)
{
return vlc_sendto_i11e(fd, buf, len, flags, NULL, 0);
}
VLC_API int vlc_accept_i11e(int fd, struct sockaddr *, socklen_t *, bool);
VLC_API void vlc_interrupt_register(void (*cb)(void *), void *opaque);
VLC_API int vlc_interrupt_unregister(void);
typedef struct vlc_interrupt vlc_interrupt_t;
VLC_API vlc_interrupt_t *vlc_interrupt_create(void) VLC_USED;
VLC_API void vlc_interrupt_destroy(vlc_interrupt_t *);
VLC_API vlc_interrupt_t *vlc_interrupt_set(vlc_interrupt_t *);
VLC_API void vlc_interrupt_raise(vlc_interrupt_t *);
VLC_API void vlc_interrupt_kill(vlc_interrupt_t *);
VLC_API bool vlc_killed(void) VLC_USED;
VLC_API void vlc_interrupt_forward_start(vlc_interrupt_t *to,
void *data[2]);
VLC_API int vlc_interrupt_forward_stop(void *const data[2]);
