




















#include "git-compat-util.h"


#if (__GNUC__ == 4 && 3 <= __GNUC_MINOR__) || 4 < __GNUC__
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif

#if defined(WIN32)
#include <malloc.h>
#endif

#include <sys/types.h>

#include <errno.h>
#include <limits.h>
#include <assert.h>

#if (defined _WIN32 || defined __WIN32__) && ! defined __CYGWIN__
#define WIN32_NATIVE
#if defined (_MSC_VER) && !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x0502
#endif
#include <winsock2.h>
#include <windows.h>
#include <io.h>
#include <stdio.h>
#include <conio.h>
#else
#include <sys/time.h>
#include <sys/socket.h>
#if !defined(NO_SYS_SELECT_H)
#include <sys/select.h>
#endif
#include <unistd.h>
#endif


#include "poll.h"

#if defined(HAVE_SYS_IOCTL_H)
#include <sys/ioctl.h>
#endif
#if defined(HAVE_SYS_FILIO_H)
#include <sys/filio.h>
#endif

#include <time.h>

#if !defined(INFTIM)
#define INFTIM (-1)
#endif


#if !defined(MSG_PEEK)
#define MSG_PEEK 0
#endif

#if defined(WIN32_NATIVE)

#define IsConsoleHandle(h) (((long) (intptr_t) (h) & 3) == 3)

static BOOL
IsSocketHandle (HANDLE h)
{
WSANETWORKEVENTS ev;

if (IsConsoleHandle (h))
return FALSE;



ev.lNetworkEvents = 0xDEADBEEF;
WSAEnumNetworkEvents ((SOCKET) h, NULL, &ev);
return ev.lNetworkEvents != 0xDEADBEEF;
}


typedef struct _FILE_PIPE_LOCAL_INFORMATION {
ULONG NamedPipeType;
ULONG NamedPipeConfiguration;
ULONG MaximumInstances;
ULONG CurrentInstances;
ULONG InboundQuota;
ULONG ReadDataAvailable;
ULONG OutboundQuota;
ULONG WriteQuotaAvailable;
ULONG NamedPipeState;
ULONG NamedPipeEnd;
} FILE_PIPE_LOCAL_INFORMATION, *PFILE_PIPE_LOCAL_INFORMATION;

typedef struct _IO_STATUS_BLOCK
{
union {
DWORD Status;
PVOID Pointer;
} u;
ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef enum _FILE_INFORMATION_CLASS {
FilePipeLocalInformation = 24
} FILE_INFORMATION_CLASS, *PFILE_INFORMATION_CLASS;

typedef DWORD (WINAPI *PNtQueryInformationFile)
(HANDLE, IO_STATUS_BLOCK *, VOID *, ULONG, FILE_INFORMATION_CLASS);

#if !defined(PIPE_BUF)
#define PIPE_BUF 512
#endif




static int
win32_compute_revents (HANDLE h, int *p_sought)
{
int i, ret, happened;
INPUT_RECORD *irbuffer;
DWORD avail, nbuffer;
BOOL bRet;

switch (GetFileType (h))
{
case FILE_TYPE_PIPE:
happened = 0;
if (PeekNamedPipe (h, NULL, 0, NULL, &avail, NULL) != 0)
{
if (avail)
happened |= *p_sought & (POLLIN | POLLRDNORM);
}
else if (GetLastError () == ERROR_BROKEN_PIPE)
happened |= POLLHUP;

else
{



happened |= *p_sought & (POLLOUT | POLLWRNORM | POLLWRBAND);
}
return happened;

case FILE_TYPE_CHAR:
ret = WaitForSingleObject (h, 0);
if (!IsConsoleHandle (h))
return ret == WAIT_OBJECT_0 ? *p_sought & ~(POLLPRI | POLLRDBAND) : 0;

nbuffer = avail = 0;
bRet = GetNumberOfConsoleInputEvents (h, &nbuffer);
if (bRet)
{

*p_sought &= POLLIN | POLLRDNORM;
if (nbuffer == 0)
return POLLHUP;
if (!*p_sought)
return 0;

irbuffer = (INPUT_RECORD *) alloca (nbuffer * sizeof (INPUT_RECORD));
bRet = PeekConsoleInput (h, irbuffer, nbuffer, &avail);
if (!bRet || avail == 0)
return POLLHUP;

for (i = 0; i < avail; i++)
if (irbuffer[i].EventType == KEY_EVENT)
return *p_sought;
return 0;
}
else
{

*p_sought &= POLLOUT | POLLWRNORM | POLLWRBAND;
return *p_sought;
}

default:
ret = WaitForSingleObject (h, 0);
if (ret == WAIT_OBJECT_0)
return *p_sought & ~(POLLPRI | POLLRDBAND);

return *p_sought & (POLLOUT | POLLWRNORM | POLLWRBAND);
}
}



static int
win32_compute_revents_socket (SOCKET h, int sought, long lNetworkEvents)
{
int happened = 0;

if ((lNetworkEvents & (FD_READ | FD_ACCEPT | FD_CLOSE)) == FD_ACCEPT)
happened |= (POLLIN | POLLRDNORM) & sought;

else if (lNetworkEvents & (FD_READ | FD_ACCEPT | FD_CLOSE))
{
int r, error;

char data[64];
WSASetLastError (0);
r = recv (h, data, sizeof (data), MSG_PEEK);
error = WSAGetLastError ();
WSASetLastError (0);

if (r > 0 || error == WSAENOTCONN)
happened |= (POLLIN | POLLRDNORM) & sought;


else if (r == 0 || error == WSAESHUTDOWN || error == WSAECONNRESET
|| error == WSAECONNABORTED || error == WSAENETRESET)
happened |= POLLHUP;

else
happened |= POLLERR;
}

if (lNetworkEvents & (FD_WRITE | FD_CONNECT))
happened |= (POLLOUT | POLLWRNORM | POLLWRBAND) & sought;

if (lNetworkEvents & FD_OOB)
happened |= (POLLPRI | POLLRDBAND) & sought;

return happened;
}

#else 


static int
compute_revents (int fd, int sought, fd_set *rfds, fd_set *wfds, fd_set *efds)
{
int happened = 0;
if (FD_ISSET (fd, rfds))
{
int r;
int socket_errno;

#if defined __MACH__ && defined __APPLE__




r = recv (fd, NULL, 0, MSG_PEEK);
socket_errno = (r < 0) ? errno : 0;
if (r == 0 || socket_errno == ENOTSOCK)
ioctl (fd, FIONREAD, &r);
#else
char data[64];
r = recv (fd, data, sizeof (data), MSG_PEEK);
socket_errno = (r < 0) ? errno : 0;
#endif
if (r == 0)
happened |= POLLHUP;



else if (r > 0 || ( socket_errno == ENOTCONN))
happened |= (POLLIN | POLLRDNORM) & sought;


else if (socket_errno == ESHUTDOWN || socket_errno == ECONNRESET
|| socket_errno == ECONNABORTED || socket_errno == ENETRESET)
happened |= POLLHUP;


else if ( socket_errno == ENOTSOCK)
happened |= (POLLIN | POLLRDNORM) & sought;

else
happened |= POLLERR;
}

if (FD_ISSET (fd, wfds))
happened |= (POLLOUT | POLLWRNORM | POLLWRBAND) & sought;

if (FD_ISSET (fd, efds))
happened |= (POLLPRI | POLLRDBAND) & sought;

return happened;
}
#endif 

int
poll (struct pollfd *pfd, nfds_t nfd, int timeout)
{
#if !defined(WIN32_NATIVE)
fd_set rfds, wfds, efds;
struct timeval tv;
struct timeval *ptv;
int maxfd, rc;
nfds_t i;

#if defined(_SC_OPEN_MAX)
static int sc_open_max = -1;

if (nfd < 0
|| (nfd > sc_open_max
&& (sc_open_max != -1
|| nfd > (sc_open_max = sysconf (_SC_OPEN_MAX)))))
{
errno = EINVAL;
return -1;
}
#else 
#if defined(OPEN_MAX)
if (nfd < 0 || nfd > OPEN_MAX)
{
errno = EINVAL;
return -1;
}
#endif 
#endif 



if (!pfd && nfd)
{
errno = EFAULT;
return -1;
}


if (timeout == 0)
{
ptv = &tv;
ptv->tv_sec = 0;
ptv->tv_usec = 0;
}
else if (timeout > 0)
{
ptv = &tv;
ptv->tv_sec = timeout / 1000;
ptv->tv_usec = (timeout % 1000) * 1000;
}
else if (timeout == INFTIM)

ptv = NULL;
else
{
errno = EINVAL;
return -1;
}


maxfd = -1;
FD_ZERO (&rfds);
FD_ZERO (&wfds);
FD_ZERO (&efds);
for (i = 0; i < nfd; i++)
{
if (pfd[i].fd < 0)
continue;

if (pfd[i].events & (POLLIN | POLLRDNORM))
FD_SET (pfd[i].fd, &rfds);




if (pfd[i].events & (POLLOUT | POLLWRNORM | POLLWRBAND))
FD_SET (pfd[i].fd, &wfds);
if (pfd[i].events & (POLLPRI | POLLRDBAND))
FD_SET (pfd[i].fd, &efds);
if (pfd[i].fd >= maxfd
&& (pfd[i].events & (POLLIN | POLLOUT | POLLPRI
| POLLRDNORM | POLLRDBAND
| POLLWRNORM | POLLWRBAND)))
{
maxfd = pfd[i].fd;
if (maxfd > FD_SETSIZE)
{
errno = EOVERFLOW;
return -1;
}
}
}


rc = select (maxfd + 1, &rfds, &wfds, &efds, ptv);
if (rc < 0)
return rc;


rc = 0;
for (i = 0; i < nfd; i++)
if (pfd[i].fd < 0)
pfd[i].revents = 0;
else
{
int happened = compute_revents (pfd[i].fd, pfd[i].events,
&rfds, &wfds, &efds);
if (happened)
{
pfd[i].revents = happened;
rc++;
}
else
{
pfd[i].revents = 0;
}
}

return rc;
#else
static struct timeval tv0;
static HANDLE hEvent;
WSANETWORKEVENTS ev;
HANDLE h, handle_array[FD_SETSIZE + 2];
DWORD ret, wait_timeout, nhandles, orig_timeout = 0;
ULONGLONG start = 0;
fd_set rfds, wfds, xfds;
BOOL poll_again;
MSG msg;
int rc = 0;
nfds_t i;

if (nfd < 0 || timeout < -1)
{
errno = EINVAL;
return -1;
}

if (timeout != INFTIM)
{
orig_timeout = timeout;
start = GetTickCount64();
}

if (!hEvent)
hEvent = CreateEvent (NULL, FALSE, FALSE, NULL);

restart:
handle_array[0] = hEvent;
nhandles = 1;
FD_ZERO (&rfds);
FD_ZERO (&wfds);
FD_ZERO (&xfds);


for (i = 0; i < nfd; i++)
{
int sought = pfd[i].events;
pfd[i].revents = 0;
if (pfd[i].fd < 0)
continue;
if (!(sought & (POLLIN | POLLRDNORM | POLLOUT | POLLWRNORM | POLLWRBAND
| POLLPRI | POLLRDBAND)))
continue;

h = (HANDLE) _get_osfhandle (pfd[i].fd);
assert (h != NULL);
if (IsSocketHandle (h))
{
int requested = FD_CLOSE;


if (sought & (POLLIN | POLLRDNORM))
{
requested |= FD_READ | FD_ACCEPT;
FD_SET ((SOCKET) h, &rfds);
}
if (sought & (POLLOUT | POLLWRNORM | POLLWRBAND))
{
requested |= FD_WRITE | FD_CONNECT;
FD_SET ((SOCKET) h, &wfds);
}
if (sought & (POLLPRI | POLLRDBAND))
{
requested |= FD_OOB;
FD_SET ((SOCKET) h, &xfds);
}

if (requested)
WSAEventSelect ((SOCKET) h, hEvent, requested);
}
else
{




pfd[i].revents = win32_compute_revents (h, &sought);
if (sought)
handle_array[nhandles++] = h;
if (pfd[i].revents)
timeout = 0;
}
}

if (select (0, &rfds, &wfds, &xfds, &tv0) > 0)
{


poll_again = FALSE;
wait_timeout = 0;
}
else
{
poll_again = TRUE;
if (timeout == INFTIM)
wait_timeout = INFINITE;
else
wait_timeout = timeout;
}

for (;;)
{
ret = MsgWaitForMultipleObjects (nhandles, handle_array, FALSE,
wait_timeout, QS_ALLINPUT);

if (ret == WAIT_OBJECT_0 + nhandles)
{

BOOL bRet;
while ((bRet = PeekMessage (&msg, NULL, 0, 0, PM_REMOVE)) != 0)
{
TranslateMessage (&msg);
DispatchMessage (&msg);
}
}
else
break;
}

if (poll_again)
select (0, &rfds, &wfds, &xfds, &tv0);


handle_array[nhandles] = NULL;
nhandles = 1;
for (i = 0; i < nfd; i++)
{
int happened;

if (pfd[i].fd < 0)
continue;
if (!(pfd[i].events & (POLLIN | POLLRDNORM |
POLLOUT | POLLWRNORM | POLLWRBAND)))
continue;

h = (HANDLE) _get_osfhandle (pfd[i].fd);
if (h != handle_array[nhandles])
{

WSAEnumNetworkEvents ((SOCKET) h, NULL, &ev);
WSAEventSelect ((SOCKET) h, NULL, 0);



if (FD_ISSET ((SOCKET) h, &rfds)
&& !(ev.lNetworkEvents & (FD_READ | FD_ACCEPT)))
ev.lNetworkEvents |= FD_READ | FD_ACCEPT;
if (FD_ISSET ((SOCKET) h, &wfds))
ev.lNetworkEvents |= FD_WRITE | FD_CONNECT;
if (FD_ISSET ((SOCKET) h, &xfds))
ev.lNetworkEvents |= FD_OOB;

happened = win32_compute_revents_socket ((SOCKET) h, pfd[i].events,
ev.lNetworkEvents);
}
else
{

int sought = pfd[i].events;
happened = win32_compute_revents (h, &sought);
nhandles++;
}

if ((pfd[i].revents |= happened) != 0)
rc++;
}

if (!rc && orig_timeout && timeout != INFTIM)
{
ULONGLONG elapsed = GetTickCount64() - start;
timeout = elapsed >= orig_timeout ? 0 : (int)(orig_timeout - elapsed);
}

if (!rc && timeout)
{
SleepEx (1, TRUE);
goto restart;
}

return rc;
#endif
}
