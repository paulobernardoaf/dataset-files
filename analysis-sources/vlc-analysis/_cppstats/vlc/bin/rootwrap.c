#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif
#define _XPG4_2 
#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/resource.h> 
#if defined(RLIMIT_RTPRIO)
#include <sched.h>
#endif
#include <errno.h>
#include <netinet/in.h>
#if defined (AF_INET6) && !defined (IPV6_V6ONLY)
#warning Uho, your IPv6 support is broken and has been disabled. Fix your C library.
#undef AF_INET6
#endif
#if !defined(AF_LOCAL)
#define AF_LOCAL AF_UNIX
#endif
#if !defined(MSG_NOSIGNAL) && defined(SO_NOSIGPIPE)
#define MSG_NOSIGNAL 0
#endif
#if !defined(CMSG_ALIGN)
#define CMSG_ALIGN(len) (((len) + sizeof(intptr_t)-1) & ~(sizeof(intptr_t)-1))
#endif
#if !defined(CMSG_SPACE)
#define CMSG_SPACE(len) (CMSG_ALIGN(sizeof(struct cmsghdr)) + CMSG_ALIGN(len))
#endif
#if !defined(CMSG_LEN)
#define CMSG_LEN(len) (CMSG_ALIGN(sizeof(struct cmsghdr)) + (len))
#endif
static inline int is_allowed_port (uint16_t port)
{
port = ntohs (port);
return (port == 80) || (port == 443) || (port == 554);
}
static inline int send_err (int fd, int err)
{
return send(fd, &err, sizeof (err), MSG_NOSIGNAL) == sizeof (err) ? 0 : -1;
}
static int send_fd (int p, int fd)
{
struct msghdr hdr;
struct iovec iov;
struct cmsghdr *cmsg;
char buf[CMSG_SPACE (sizeof (fd))];
int val = 0;
hdr.msg_name = NULL;
hdr.msg_namelen = 0;
hdr.msg_iov = &iov;
hdr.msg_iovlen = 1;
hdr.msg_control = buf;
hdr.msg_controllen = sizeof (buf);
iov.iov_base = &val;
iov.iov_len = sizeof (val);
cmsg = CMSG_FIRSTHDR (&hdr);
cmsg->cmsg_level = SOL_SOCKET;
cmsg->cmsg_type = SCM_RIGHTS;
cmsg->cmsg_len = CMSG_LEN (sizeof (fd));
memcpy (CMSG_DATA (cmsg), &fd, sizeof (fd));
hdr.msg_controllen = cmsg->cmsg_len;
return sendmsg(p, &hdr, MSG_NOSIGNAL) == sizeof (val) ? 0 : -1;
}
static void rootprocess (int fd)
{
union
{
struct sockaddr sa;
struct sockaddr_storage ss;
struct sockaddr_in sin;
#if defined(AF_INET6)
struct sockaddr_in6 sin6;
#endif
} addr;
while (recv (fd, &addr.ss, sizeof (addr.ss), 0) == sizeof (addr.ss))
{
unsigned len;
int sock;
int family;
switch (addr.sa.sa_family)
{
case AF_INET:
if (!is_allowed_port (addr.sin.sin_port))
{
if (send_err (fd, EACCES))
return;
continue;
}
len = sizeof (struct sockaddr_in);
family = PF_INET;
break;
#if defined(AF_INET6)
case AF_INET6:
if (!is_allowed_port (addr.sin6.sin6_port))
{
if (send_err (fd, EACCES))
return;
continue;
}
len = sizeof (struct sockaddr_in6);
family = PF_INET6;
break;
#endif
default:
if (send_err (fd, EAFNOSUPPORT))
return;
continue;
}
sock = socket (family, SOCK_STREAM, IPPROTO_TCP);
if (sock != -1)
{
const int val = 1;
setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof (val));
#if defined(AF_INET6)
if (addr.sa.sa_family == AF_INET6)
setsockopt (sock, IPPROTO_IPV6, IPV6_V6ONLY, &val, sizeof (val));
#endif
if (bind (sock, &addr.sa, len) == 0)
{
send_fd (fd, sock);
close (sock);
continue;
}
close (sock);
}
send_err (fd, errno);
}
}
int main (int argc, char *argv[])
{
int pair[2];
if (socketpair (AF_LOCAL, SOCK_STREAM, 0, pair))
return 1;
if (pair[0] < 3)
goto error; 
#if defined(SO_NOSIGPIPE)
setsockopt(pair[1], SOL_SOCKET, SO_NOSIGPIPE, &(int){ 1 }, sizeof (int));
#endif
pid_t pid = fork ();
switch (pid)
{
case -1:
goto error;
case 0:
{
int null = open ("/dev/null", O_RDWR);
if (null != -1)
{
dup2 (null, 0);
dup2 (null, 1);
dup2 (null, 2);
close (null);
}
close (pair[0]);
setsid ();
rootprocess (pair[1]);
exit (0);
}
}
close (pair[1]);
pair[1] = -1;
char buf[21];
snprintf (buf, sizeof (buf), "%d", pair[0]);
setenv ("VLC_ROOTWRAP_SOCK", buf, 1);
#if defined(RLIMIT_RTPRIO)
struct rlimit rlim;
rlim.rlim_max = rlim.rlim_cur = sched_get_priority_min (SCHED_RR) + 24;
setrlimit (RLIMIT_RTPRIO, &rlim);
#endif
uid_t uid = getuid ();
if (uid == 0)
{
const char *sudo = getenv ("SUDO_UID");
if (sudo)
uid = atoi (sudo);
}
if (uid == 0)
{
fputs("Cannot determine unprivileged user for VLC!\n", stderr);
exit (1);
}
setuid (uid);
if (!setuid (0)) 
exit (1);
if (strlen (argv[0]) < sizeof ("-wrapper"))
goto error;
argv[0][strlen (argv[0]) - strlen ("-wrapper")] = '\0';
(void)argc;
if (execvp (argv[0], argv))
perror (argv[0]);
error:
close (pair[0]);
if (pair[1] != -1)
close (pair[1]);
return 1;
}
