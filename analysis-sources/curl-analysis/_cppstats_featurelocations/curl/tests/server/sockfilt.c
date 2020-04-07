




















#include "server_setup.h"































































#if defined(HAVE_SIGNAL_H)
#include <signal.h>
#endif
#if defined(HAVE_NETINET_IN_H)
#include <netinet/in.h>
#endif
#if defined(HAVE_NETINET_IN6_H)
#include <netinet/in6.h>
#endif
#if defined(HAVE_ARPA_INET_H)
#include <arpa/inet.h>
#endif
#if defined(HAVE_NETDB_H)
#include <netdb.h>
#endif

#define ENABLE_CURLX_PRINTF


#include "curlx.h" 
#include "getpart.h"
#include "inet_pton.h"
#include "util.h"
#include "server_sockaddr.h"
#include "warnless.h"


#include "memdebug.h"

#if defined(USE_WINSOCK)
#undef EINTR
#define EINTR 4 
#undef EAGAIN
#define EAGAIN 11 
#undef ENOMEM
#define ENOMEM 12 
#undef EINVAL
#define EINVAL 22 
#endif

#define DEFAULT_PORT 8999

#if !defined(DEFAULT_LOGFILE)
#define DEFAULT_LOGFILE "log/sockfilt.log"
#endif

const char *serverlogfile = DEFAULT_LOGFILE;

static bool verbose = FALSE;
static bool bind_only = FALSE;
#if defined(ENABLE_IPV6)
static bool use_ipv6 = FALSE;
#endif
static const char *ipv_inuse = "IPv4";
static unsigned short port = DEFAULT_PORT;
static unsigned short connectport = 0; 

enum sockmode {
PASSIVE_LISTEN, 
PASSIVE_CONNECT, 
ACTIVE, 
ACTIVE_DISCONNECT 
};



#if !defined(HAVE_SIGINTERRUPT)
#define siginterrupt(x,y) do {} while(0)
#endif



typedef RETSIGTYPE (*SIGHANDLER_T)(int);

#if defined(SIGHUP)
static SIGHANDLER_T old_sighup_handler = SIG_ERR;
#endif

#if defined(SIGPIPE)
static SIGHANDLER_T old_sigpipe_handler = SIG_ERR;
#endif

#if defined(SIGALRM)
static SIGHANDLER_T old_sigalrm_handler = SIG_ERR;
#endif

#if defined(SIGINT)
static SIGHANDLER_T old_sigint_handler = SIG_ERR;
#endif

#if defined(SIGTERM)
static SIGHANDLER_T old_sigterm_handler = SIG_ERR;
#endif

#if defined(SIGBREAK) && defined(WIN32)
static SIGHANDLER_T old_sigbreak_handler = SIG_ERR;
#endif



SIG_ATOMIC_T got_exit_signal = 0;



static volatile int exit_signal = 0;






static RETSIGTYPE exit_signal_handler(int signum)
{
int old_errno = errno;
if(got_exit_signal == 0) {
got_exit_signal = 1;
exit_signal = signum;
}
(void)signal(signum, exit_signal_handler);
errno = old_errno;
}

static void install_signal_handlers(void)
{
#if defined(SIGHUP)

old_sighup_handler = signal(SIGHUP, SIG_IGN);
if(old_sighup_handler == SIG_ERR)
logmsg("cannot install SIGHUP handler: %s", strerror(errno));
#endif
#if defined(SIGPIPE)

old_sigpipe_handler = signal(SIGPIPE, SIG_IGN);
if(old_sigpipe_handler == SIG_ERR)
logmsg("cannot install SIGPIPE handler: %s", strerror(errno));
#endif
#if defined(SIGALRM)

old_sigalrm_handler = signal(SIGALRM, SIG_IGN);
if(old_sigalrm_handler == SIG_ERR)
logmsg("cannot install SIGALRM handler: %s", strerror(errno));
#endif
#if defined(SIGINT)

old_sigint_handler = signal(SIGINT, exit_signal_handler);
if(old_sigint_handler == SIG_ERR)
logmsg("cannot install SIGINT handler: %s", strerror(errno));
else
siginterrupt(SIGINT, 1);
#endif
#if defined(SIGTERM)

old_sigterm_handler = signal(SIGTERM, exit_signal_handler);
if(old_sigterm_handler == SIG_ERR)
logmsg("cannot install SIGTERM handler: %s", strerror(errno));
else
siginterrupt(SIGTERM, 1);
#endif
#if defined(SIGBREAK) && defined(WIN32)

old_sigbreak_handler = signal(SIGBREAK, exit_signal_handler);
if(old_sigbreak_handler == SIG_ERR)
logmsg("cannot install SIGBREAK handler: %s", strerror(errno));
else
siginterrupt(SIGBREAK, 1);
#endif
}

static void restore_signal_handlers(void)
{
#if defined(SIGHUP)
if(SIG_ERR != old_sighup_handler)
(void)signal(SIGHUP, old_sighup_handler);
#endif
#if defined(SIGPIPE)
if(SIG_ERR != old_sigpipe_handler)
(void)signal(SIGPIPE, old_sigpipe_handler);
#endif
#if defined(SIGALRM)
if(SIG_ERR != old_sigalrm_handler)
(void)signal(SIGALRM, old_sigalrm_handler);
#endif
#if defined(SIGINT)
if(SIG_ERR != old_sigint_handler)
(void)signal(SIGINT, old_sigint_handler);
#endif
#if defined(SIGTERM)
if(SIG_ERR != old_sigterm_handler)
(void)signal(SIGTERM, old_sigterm_handler);
#endif
#if defined(SIGBREAK) && defined(WIN32)
if(SIG_ERR != old_sigbreak_handler)
(void)signal(SIGBREAK, old_sigbreak_handler);
#endif
}

#if defined(WIN32)



static ssize_t read_wincon(int fd, void *buf, size_t count)
{
HANDLE handle = NULL;
DWORD mode, rcount = 0;
BOOL success;

if(fd == fileno(stdin)) {
handle = GetStdHandle(STD_INPUT_HANDLE);
}
else {
return read(fd, buf, count);
}

if(GetConsoleMode(handle, &mode)) {
success = ReadConsole(handle, buf, curlx_uztoul(count), &rcount, NULL);
}
else {
success = ReadFile(handle, buf, curlx_uztoul(count), &rcount, NULL);
}
if(success) {
return rcount;
}

errno = GetLastError();
return -1;
}
#undef read
#define read(a,b,c) read_wincon(a,b,c)




static ssize_t write_wincon(int fd, const void *buf, size_t count)
{
HANDLE handle = NULL;
DWORD mode, wcount = 0;
BOOL success;

if(fd == fileno(stdout)) {
handle = GetStdHandle(STD_OUTPUT_HANDLE);
}
else if(fd == fileno(stderr)) {
handle = GetStdHandle(STD_ERROR_HANDLE);
}
else {
return write(fd, buf, count);
}

if(GetConsoleMode(handle, &mode)) {
success = WriteConsole(handle, buf, curlx_uztoul(count), &wcount, NULL);
}
else {
success = WriteFile(handle, buf, curlx_uztoul(count), &wcount, NULL);
}
if(success) {
return wcount;
}

errno = GetLastError();
return -1;
}
#undef write
#define write(a,b,c) write_wincon(a,b,c)
#endif








static ssize_t fullread(int filedes, void *buffer, size_t nbytes)
{
int error;
ssize_t nread = 0;

do {
ssize_t rc = read(filedes,
(unsigned char *)buffer + nread, nbytes - nread);

if(got_exit_signal) {
logmsg("signalled to die");
return -1;
}

if(rc < 0) {
error = errno;
if((error == EINTR) || (error == EAGAIN))
continue;
logmsg("reading from file descriptor: %d,", filedes);
logmsg("unrecoverable read() failure: (%d) %s",
error, strerror(error));
return -1;
}

if(rc == 0) {
logmsg("got 0 reading from stdin");
return 0;
}

nread += rc;

} while((size_t)nread < nbytes);

if(verbose)
logmsg("read %zd bytes", nread);

return nread;
}








static ssize_t fullwrite(int filedes, const void *buffer, size_t nbytes)
{
int error;
ssize_t nwrite = 0;

do {
ssize_t wc = write(filedes, (const unsigned char *)buffer + nwrite,
nbytes - nwrite);

if(got_exit_signal) {
logmsg("signalled to die");
return -1;
}

if(wc < 0) {
error = errno;
if((error == EINTR) || (error == EAGAIN))
continue;
logmsg("writing to file descriptor: %d,", filedes);
logmsg("unrecoverable write() failure: (%d) %s",
error, strerror(error));
return -1;
}

if(wc == 0) {
logmsg("put 0 writing to stdout");
return 0;
}

nwrite += wc;

} while((size_t)nwrite < nbytes);

if(verbose)
logmsg("wrote %zd bytes", nwrite);

return nwrite;
}








static bool read_stdin(void *buffer, size_t nbytes)
{
ssize_t nread = fullread(fileno(stdin), buffer, nbytes);
if(nread != (ssize_t)nbytes) {
logmsg("exiting...");
return FALSE;
}
return TRUE;
}








static bool write_stdout(const void *buffer, size_t nbytes)
{
ssize_t nwrite = fullwrite(fileno(stdout), buffer, nbytes);
if(nwrite != (ssize_t)nbytes) {
logmsg("exiting...");
return FALSE;
}
return TRUE;
}

static void lograw(unsigned char *buffer, ssize_t len)
{
char data[120];
ssize_t i;
unsigned char *ptr = buffer;
char *optr = data;
ssize_t width = 0;
int left = sizeof(data);

for(i = 0; i<len; i++) {
switch(ptr[i]) {
case '\n':
msnprintf(optr, left, "\\n");
width += 2;
optr += 2;
left -= 2;
break;
case '\r':
msnprintf(optr, left, "\\r");
width += 2;
optr += 2;
left -= 2;
break;
default:
msnprintf(optr, left, "%c", (ISGRAPH(ptr[i]) ||
ptr[i] == 0x20) ?ptr[i]:'.');
width++;
optr++;
left--;
break;
}

if(width>60) {
logmsg("'%s'", data);
width = 0;
optr = data;
left = sizeof(data);
}
}
if(width)
logmsg("'%s'", data);
}

#if defined(USE_WINSOCK)












struct select_ws_wait_data {
HANDLE handle; 
HANDLE signal; 
HANDLE abort; 
HANDLE mutex; 
};
static DWORD WINAPI select_ws_wait_thread(LPVOID lpParameter)
{
struct select_ws_wait_data *data;
HANDLE mutex, signal, handle, handles[2];
INPUT_RECORD inputrecord;
LARGE_INTEGER size, pos;
DWORD type, length;


data = (struct select_ws_wait_data *) lpParameter;
if(data) {
handle = data->handle;
handles[0] = data->abort;
handles[1] = handle;
signal = data->signal;
mutex = data->mutex;
free(data);
}
else
return (DWORD)-1;


type = GetFileType(handle);
switch(type) {
case FILE_TYPE_DISK:








while(WaitForMultipleObjectsEx(1, handles, FALSE, 0, FALSE)
== WAIT_TIMEOUT) {
length = WaitForSingleObjectEx(mutex, 0, FALSE);
if(length == WAIT_OBJECT_0) {

length = 0;
size.QuadPart = 0;
size.LowPart = GetFileSize(handle, &length);
if((size.LowPart != INVALID_FILE_SIZE) ||
(GetLastError() == NO_ERROR)) {
size.HighPart = length;

pos.QuadPart = 0;
pos.LowPart = SetFilePointer(handle, 0, &pos.HighPart,
FILE_CURRENT);
if((pos.LowPart != INVALID_SET_FILE_POINTER) ||
(GetLastError() == NO_ERROR)) {

if(size.QuadPart == pos.QuadPart) {

SleepEx(0, FALSE);
ReleaseMutex(mutex);
continue;
}
}
}

logmsg("[select_ws_wait_thread] data available, DISK: %p", handle);
SetEvent(signal);
ReleaseMutex(mutex);
break;
}
else if(length == WAIT_ABANDONED) {


break;
}
}
break;

case FILE_TYPE_CHAR:







while(WaitForMultipleObjectsEx(2, handles, FALSE, INFINITE, FALSE)
== WAIT_OBJECT_0 + 1) {
length = WaitForSingleObjectEx(mutex, 0, FALSE);
if(length == WAIT_OBJECT_0) {

length = 0;
if(GetConsoleMode(handle, &length)) {

length = 0;
if(PeekConsoleInput(handle, &inputrecord, 1, &length)) {

if(length == 1 && inputrecord.EventType != KEY_EVENT) {

ReadConsoleInput(handle, &inputrecord, 1, &length);
ReleaseMutex(mutex);
continue;
}
}
}

logmsg("[select_ws_wait_thread] data available, CHAR: %p", handle);
SetEvent(signal);
ReleaseMutex(mutex);
break;
}
else if(length == WAIT_ABANDONED) {


break;
}
}
break;

case FILE_TYPE_PIPE:







while(WaitForMultipleObjectsEx(1, handles, FALSE, 0, FALSE)
== WAIT_TIMEOUT) {
length = WaitForSingleObjectEx(mutex, 0, FALSE);
if(length == WAIT_OBJECT_0) {

length = 0;
if(PeekNamedPipe(handle, NULL, 0, NULL, &length, NULL)) {

if(length == 0) {
SleepEx(0, FALSE);
ReleaseMutex(mutex);
continue;
}
else {
logmsg("[select_ws_wait_thread] PeekNamedPipe len: %d", length);
}
}
else {

length = GetLastError();
logmsg("[select_ws_wait_thread] PeekNamedPipe error: %d", length);
if(length == ERROR_BROKEN_PIPE) {
SleepEx(0, FALSE);
ReleaseMutex(mutex);
continue;
}
}

logmsg("[select_ws_wait_thread] data available, PIPE: %p", handle);
SetEvent(signal);
ReleaseMutex(mutex);
break;
}
else if(length == WAIT_ABANDONED) {


break;
}
}
break;

default:

if(WaitForMultipleObjectsEx(2, handles, FALSE, INFINITE, FALSE)
== WAIT_OBJECT_0 + 1) {
if(WaitForSingleObjectEx(mutex, 0, FALSE) == WAIT_OBJECT_0) {
logmsg("[select_ws_wait_thread] data available, HANDLE: %p", handle);
SetEvent(signal);
ReleaseMutex(mutex);
}
}
break;
}

return 0;
}
static HANDLE select_ws_wait(HANDLE handle, HANDLE signal,
HANDLE abort, HANDLE mutex)
{
struct select_ws_wait_data *data;
HANDLE thread = NULL;


data = malloc(sizeof(struct select_ws_wait_data));
if(data) {
data->handle = handle;
data->signal = signal;
data->abort = abort;
data->mutex = mutex;


thread = CreateThread(NULL, 0,
&select_ws_wait_thread,
data, 0, NULL);


if(!thread) {
free(data);
}
}

return thread;
}
struct select_ws_data {
WSANETWORKEVENTS pre; 
curl_socket_t fd; 
curl_socket_t wsasock; 
WSAEVENT wsaevent; 
HANDLE signal; 
HANDLE thread; 
};
static int select_ws(int nfds, fd_set *readfds, fd_set *writefds,
fd_set *exceptfds, struct timeval *tv)
{
HANDLE abort, mutex, signal, handle, *handles;
fd_set readsock, writesock, exceptsock;
DWORD milliseconds, wait, idx;
WSANETWORKEVENTS wsanetevents;
struct select_ws_data *data;
WSAEVENT wsaevent;
int error, fds;
DWORD nfd = 0, thd = 0, wsa = 0;
int ret = 0;


if(nfds < 0) {
errno = EINVAL;
return -1;
}


if(tv) {
milliseconds = (tv->tv_sec*1000)+(DWORD)(((double)tv->tv_usec)/1000.0);
}
else {
milliseconds = INFINITE;
}


if(!nfds) {
SleepEx(milliseconds, FALSE);
return 0;
}


abort = CreateEvent(NULL, TRUE, FALSE, NULL);
if(!abort) {
errno = ENOMEM;
return -1;
}


mutex = CreateMutex(NULL, FALSE, NULL);
if(!mutex) {
CloseHandle(abort);
errno = ENOMEM;
return -1;
}


data = calloc(nfds, sizeof(struct select_ws_data));
if(data == NULL) {
CloseHandle(abort);
CloseHandle(mutex);
errno = ENOMEM;
return -1;
}


handles = calloc(nfds, sizeof(HANDLE));
if(handles == NULL) {
CloseHandle(abort);
CloseHandle(mutex);
free(data);
errno = ENOMEM;
return -1;
}


for(fds = 0; fds < nfds; fds++) {
long networkevents = 0;
handles[nfd] = 0;

FD_ZERO(&readsock);
FD_ZERO(&writesock);
FD_ZERO(&exceptsock);

if(FD_ISSET(fds, readfds)) {
FD_SET(fds, &readsock);
networkevents |= FD_READ|FD_ACCEPT|FD_CLOSE;
}

if(FD_ISSET(fds, writefds)) {
FD_SET(fds, &writesock);
networkevents |= FD_WRITE|FD_CONNECT;
}

if(FD_ISSET(fds, exceptfds)) {
FD_SET(fds, &exceptsock);
networkevents |= FD_OOB;
}


if(networkevents) {
data[nfd].fd = curlx_sitosk(fds);
if(fds == fileno(stdin)) {
handle = GetStdHandle(STD_INPUT_HANDLE);
signal = CreateEvent(NULL, TRUE, FALSE, NULL);
if(signal) {
handle = select_ws_wait(handle, signal, abort, mutex);
if(handle) {
handles[nfd] = signal;
data[thd].signal = signal;
data[thd].thread = handle;
thd++;
}
else {
CloseHandle(signal);
}
}
}
else if(fds == fileno(stdout)) {
handles[nfd] = GetStdHandle(STD_OUTPUT_HANDLE);
}
else if(fds == fileno(stderr)) {
handles[nfd] = GetStdHandle(STD_ERROR_HANDLE);
}
else {
wsaevent = WSACreateEvent();
if(wsaevent != WSA_INVALID_EVENT) {
error = WSAEventSelect(fds, wsaevent, networkevents);
if(error != SOCKET_ERROR) {
handle = (HANDLE) wsaevent;
handles[nfd] = handle;
data[wsa].wsasock = curlx_sitosk(fds);
data[wsa].wsaevent = wsaevent;
data[nfd].pre.lNetworkEvents = 0;
tv->tv_sec = 0;
tv->tv_usec = 0;

if(select(fds + 1, &readsock, &writesock, &exceptsock, tv) == 1) {
logmsg("[select_ws] socket %d is ready", fds);
WSASetEvent(wsaevent);
if(FD_ISSET(fds, &readsock))
data[nfd].pre.lNetworkEvents |= FD_READ;
if(FD_ISSET(fds, &writesock))
data[nfd].pre.lNetworkEvents |= FD_WRITE;
if(FD_ISSET(fds, &exceptsock))
data[nfd].pre.lNetworkEvents |= FD_OOB;
}
wsa++;
}
else {
curl_socket_t socket = curlx_sitosk(fds);
WSACloseEvent(wsaevent);
handle = (HANDLE) socket;
signal = CreateEvent(NULL, TRUE, FALSE, NULL);
if(signal) {
handle = select_ws_wait(handle, signal, abort, mutex);
if(handle) {
handles[nfd] = signal;
data[thd].signal = signal;
data[thd].thread = handle;
thd++;
}
else {
CloseHandle(signal);
}
}
}
}
}
nfd++;
}
}


wait = WaitForMultipleObjectsEx(nfd, handles, FALSE, milliseconds, FALSE);


WaitForSingleObjectEx(mutex, INFINITE, FALSE);


for(idx = 0; idx < nfd; idx++) {
curl_socket_t sock = data[idx].fd;
handle = handles[idx];
fds = curlx_sktosi(sock);


if(wait != WAIT_FAILED && (wait - WAIT_OBJECT_0) <= idx &&
WaitForSingleObjectEx(handle, 0, FALSE) == WAIT_OBJECT_0) {

if(fds == fileno(stdin)) {

FD_CLR(sock, writefds);
FD_CLR(sock, exceptfds);
}
else if(fds == fileno(stdout) || fds == fileno(stderr)) {

FD_CLR(sock, readfds);
FD_CLR(sock, exceptfds);
}
else {

wsanetevents.lNetworkEvents = 0;
error = WSAEnumNetworkEvents(fds, handle, &wsanetevents);
if(error != SOCKET_ERROR) {

wsanetevents.lNetworkEvents |= data[idx].pre.lNetworkEvents;


if(!(wsanetevents.lNetworkEvents & (FD_READ|FD_ACCEPT|FD_CLOSE)))
FD_CLR(sock, readfds);


if(!(wsanetevents.lNetworkEvents & (FD_WRITE|FD_CONNECT)))
FD_CLR(sock, writefds);


if(!(wsanetevents.lNetworkEvents & (FD_OOB)))
FD_CLR(sock, exceptfds);
}
}


if(FD_ISSET(sock, readfds) || FD_ISSET(sock, writefds) ||
FD_ISSET(sock, exceptfds)) {
ret++;
}
}
else {

FD_CLR(sock, readfds);
FD_CLR(sock, writefds);
FD_CLR(sock, exceptfds);
}
}


SetEvent(abort);

for(fds = 0; fds < nfds; fds++) {
if(FD_ISSET(fds, readfds))
logmsg("[select_ws] %d is readable", fds);

if(FD_ISSET(fds, writefds))
logmsg("[select_ws] %d is writable", fds);

if(FD_ISSET(fds, exceptfds))
logmsg("[select_ws] %d is exceptional", fds);
}

for(idx = 0; idx < wsa; idx++) {
WSAEventSelect(data[idx].wsasock, NULL, 0);
WSACloseEvent(data[idx].wsaevent);
}

for(idx = 0; idx < thd; idx++) {
WaitForSingleObjectEx(data[idx].thread, INFINITE, FALSE);
CloseHandle(data[idx].thread);
CloseHandle(data[idx].signal);
}

CloseHandle(abort);
CloseHandle(mutex);

free(handles);
free(data);

return ret;
}
#define select(a,b,c,d,e) select_ws(a,b,c,d,e)
#endif 







static bool juggle(curl_socket_t *sockfdp,
curl_socket_t listenfd,
enum sockmode *mode)
{
struct timeval timeout;
fd_set fds_read;
fd_set fds_write;
fd_set fds_err;
curl_socket_t sockfd = CURL_SOCKET_BAD;
int maxfd = -99;
ssize_t rc;
int error = 0;



unsigned char buffer[17010];
char data[16];

if(got_exit_signal) {
logmsg("signalled to die, exiting...");
return FALSE;
}

#if defined(HAVE_GETPPID)


if(getppid() <= 1) {
logmsg("process becomes orphan, exiting");
return FALSE;
}
#endif

timeout.tv_sec = 120;
timeout.tv_usec = 0;

FD_ZERO(&fds_read);
FD_ZERO(&fds_write);
FD_ZERO(&fds_err);

FD_SET((curl_socket_t)fileno(stdin), &fds_read);

switch(*mode) {

case PASSIVE_LISTEN:


sockfd = listenfd;

FD_SET(sockfd, &fds_read);
maxfd = (int)sockfd;
break;

case PASSIVE_CONNECT:

sockfd = *sockfdp;
if(CURL_SOCKET_BAD == sockfd) {

logmsg("socket is -1! on %s:%d", __FILE__, __LINE__);
maxfd = 0; 
}
else {

FD_SET(sockfd, &fds_read);
maxfd = (int)sockfd;
}
break;

case ACTIVE:

sockfd = *sockfdp;

if(CURL_SOCKET_BAD != sockfd) {
FD_SET(sockfd, &fds_read);
maxfd = (int)sockfd;
}
else {
logmsg("No socket to read on");
maxfd = 0;
}
break;

case ACTIVE_DISCONNECT:

logmsg("disconnected, no socket to read on");
maxfd = 0;
sockfd = CURL_SOCKET_BAD;
break;

} 


do {



rc = select(maxfd + 1, &fds_read, &fds_write, &fds_err, &timeout);

if(got_exit_signal) {
logmsg("signalled to die, exiting...");
return FALSE;
}

} while((rc == -1) && ((error = errno) == EINTR));

if(rc < 0) {
logmsg("select() failed with error: (%d) %s",
error, strerror(error));
return FALSE;
}

if(rc == 0)

return TRUE;


if(FD_ISSET(fileno(stdin), &fds_read)) {
ssize_t buffer_len;















if(!read_stdin(buffer, 5))
return FALSE;

logmsg("Received %c%c%c%c (on stdin)",
buffer[0], buffer[1], buffer[2], buffer[3]);

if(!memcmp("PING", buffer, 4)) {

if(!write_stdout("PONG\n", 5))
return FALSE;
}

else if(!memcmp("PORT", buffer, 4)) {


msnprintf((char *)buffer, sizeof(buffer), "%s/%hu\n", ipv_inuse, port);
buffer_len = (ssize_t)strlen((char *)buffer);
msnprintf(data, sizeof(data), "PORT\n%04zx\n", buffer_len);
if(!write_stdout(data, 10))
return FALSE;
if(!write_stdout(buffer, buffer_len))
return FALSE;
}
else if(!memcmp("QUIT", buffer, 4)) {

logmsg("quits");
return FALSE;
}
else if(!memcmp("DATA", buffer, 4)) {


if(!read_stdin(buffer, 5))
return FALSE;

buffer[5] = '\0';

buffer_len = (ssize_t)strtol((char *)buffer, NULL, 16);
if(buffer_len > (ssize_t)sizeof(buffer)) {
logmsg("ERROR: Buffer size (%zu bytes) too small for data size "
"(%zd bytes)", sizeof(buffer), buffer_len);
return FALSE;
}
logmsg("> %zd bytes data, server => client", buffer_len);

if(!read_stdin(buffer, buffer_len))
return FALSE;

lograw(buffer, buffer_len);

if(*mode == PASSIVE_LISTEN) {
logmsg("*** We are disconnected!");
if(!write_stdout("DISC\n", 5))
return FALSE;
}
else {

ssize_t bytes_written = swrite(sockfd, buffer, buffer_len);
if(bytes_written != buffer_len) {
logmsg("Not all data was sent. Bytes to send: %zd sent: %zd",
buffer_len, bytes_written);
}
}
}
else if(!memcmp("DISC", buffer, 4)) {

if(!write_stdout("DISC\n", 5))
return FALSE;
if(sockfd != CURL_SOCKET_BAD) {
logmsg("====> Client forcibly disconnected");
sclose(sockfd);
*sockfdp = CURL_SOCKET_BAD;
if(*mode == PASSIVE_CONNECT)
*mode = PASSIVE_LISTEN;
else
*mode = ACTIVE_DISCONNECT;
}
else
logmsg("attempt to close already dead connection");
return TRUE;
}
}


if((sockfd != CURL_SOCKET_BAD) && (FD_ISSET(sockfd, &fds_read)) ) {
ssize_t nread_socket;
if(*mode == PASSIVE_LISTEN) {


curl_socket_t newfd = accept(sockfd, NULL, NULL);
if(CURL_SOCKET_BAD == newfd) {
error = SOCKERRNO;
logmsg("accept(%d, NULL, NULL) failed with error: (%d) %s",
sockfd, error, strerror(error));
}
else {
logmsg("====> Client connect");
if(!write_stdout("CNCT\n", 5))
return FALSE;
*sockfdp = newfd; 
*mode = PASSIVE_CONNECT; 
}
return TRUE;
}


nread_socket = sread(sockfd, buffer, sizeof(buffer));

if(nread_socket > 0) {
msnprintf(data, sizeof(data), "DATA\n%04zx\n", nread_socket);
if(!write_stdout(data, 10))
return FALSE;
if(!write_stdout(buffer, nread_socket))
return FALSE;

logmsg("< %zd bytes data, client => server", nread_socket);
lograw(buffer, nread_socket);
}

if(nread_socket <= 0) {
logmsg("====> Client disconnect");
if(!write_stdout("DISC\n", 5))
return FALSE;
sclose(sockfd);
*sockfdp = CURL_SOCKET_BAD;
if(*mode == PASSIVE_CONNECT)
*mode = PASSIVE_LISTEN;
else
*mode = ACTIVE_DISCONNECT;
return TRUE;
}
}

return TRUE;
}

static curl_socket_t sockdaemon(curl_socket_t sock,
unsigned short *listenport)
{

srvr_sockaddr_union_t listener;
int flag;
int rc;
int totdelay = 0;
int maxretr = 10;
int delay = 20;
int attempt = 0;
int error = 0;

do {
attempt++;
flag = 1;
rc = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
(void *)&flag, sizeof(flag));
if(rc) {
error = SOCKERRNO;
logmsg("setsockopt(SO_REUSEADDR) failed with error: (%d) %s",
error, strerror(error));
if(maxretr) {
rc = wait_ms(delay);
if(rc) {

error = errno;
logmsg("wait_ms() failed with error: (%d) %s",
error, strerror(error));
sclose(sock);
return CURL_SOCKET_BAD;
}
if(got_exit_signal) {
logmsg("signalled to die, exiting...");
sclose(sock);
return CURL_SOCKET_BAD;
}
totdelay += delay;
delay *= 2; 
}
}
} while(rc && maxretr--);

if(rc) {
logmsg("setsockopt(SO_REUSEADDR) failed %d times in %d ms. Error: (%d) %s",
attempt, totdelay, error, strerror(error));
logmsg("Continuing anyway...");
}




#if defined(ENABLE_IPV6)
if(!use_ipv6) {
#endif
memset(&listener.sa4, 0, sizeof(listener.sa4));
listener.sa4.sin_family = AF_INET;
listener.sa4.sin_addr.s_addr = INADDR_ANY;
listener.sa4.sin_port = htons(*listenport);
rc = bind(sock, &listener.sa, sizeof(listener.sa4));
#if defined(ENABLE_IPV6)
}
else {
memset(&listener.sa6, 0, sizeof(listener.sa6));
listener.sa6.sin6_family = AF_INET6;
listener.sa6.sin6_addr = in6addr_any;
listener.sa6.sin6_port = htons(*listenport);
rc = bind(sock, &listener.sa, sizeof(listener.sa6));
}
#endif 
if(rc) {
error = SOCKERRNO;
logmsg("Error binding socket on port %hu: (%d) %s",
*listenport, error, strerror(error));
sclose(sock);
return CURL_SOCKET_BAD;
}

if(!*listenport) {


curl_socklen_t la_size;
srvr_sockaddr_union_t localaddr;
#if defined(ENABLE_IPV6)
if(!use_ipv6)
#endif
la_size = sizeof(localaddr.sa4);
#if defined(ENABLE_IPV6)
else
la_size = sizeof(localaddr.sa6);
#endif
memset(&localaddr.sa, 0, (size_t)la_size);
if(getsockname(sock, &localaddr.sa, &la_size) < 0) {
error = SOCKERRNO;
logmsg("getsockname() failed with error: (%d) %s",
error, strerror(error));
sclose(sock);
return CURL_SOCKET_BAD;
}
switch(localaddr.sa.sa_family) {
case AF_INET:
*listenport = ntohs(localaddr.sa4.sin_port);
break;
#if defined(ENABLE_IPV6)
case AF_INET6:
*listenport = ntohs(localaddr.sa6.sin6_port);
break;
#endif
default:
break;
}
if(!*listenport) {

logmsg("Apparently getsockname() succeeded, with listener port zero.");
logmsg("A valid reason for this failure is a binary built without");
logmsg("proper network library linkage. This might not be the only");
logmsg("reason, but double check it before anything else.");
sclose(sock);
return CURL_SOCKET_BAD;
}
}


if(bind_only) {
logmsg("instructed to bind port without listening");
return sock;
}


rc = listen(sock, 5);
if(0 != rc) {
error = SOCKERRNO;
logmsg("listen(%d, 5) failed with error: (%d) %s",
sock, error, strerror(error));
sclose(sock);
return CURL_SOCKET_BAD;
}

return sock;
}


int main(int argc, char *argv[])
{
srvr_sockaddr_union_t me;
curl_socket_t sock = CURL_SOCKET_BAD;
curl_socket_t msgsock = CURL_SOCKET_BAD;
int wrotepidfile = 0;
const char *pidname = ".sockfilt.pid";
bool juggle_again;
int rc;
int error;
int arg = 1;
enum sockmode mode = PASSIVE_LISTEN; 
const char *addr = NULL;

while(argc>arg) {
if(!strcmp("--version", argv[arg])) {
printf("sockfilt IPv4%s\n",
#if defined(ENABLE_IPV6)
"/IPv6"
#else
""
#endif
);
return 0;
}
else if(!strcmp("--verbose", argv[arg])) {
verbose = TRUE;
arg++;
}
else if(!strcmp("--pidfile", argv[arg])) {
arg++;
if(argc>arg)
pidname = argv[arg++];
}
else if(!strcmp("--logfile", argv[arg])) {
arg++;
if(argc>arg)
serverlogfile = argv[arg++];
}
else if(!strcmp("--ipv6", argv[arg])) {
#if defined(ENABLE_IPV6)
ipv_inuse = "IPv6";
use_ipv6 = TRUE;
#endif
arg++;
}
else if(!strcmp("--ipv4", argv[arg])) {

#if defined(ENABLE_IPV6)
ipv_inuse = "IPv4";
use_ipv6 = FALSE;
#endif
arg++;
}
else if(!strcmp("--bindonly", argv[arg])) {
bind_only = TRUE;
arg++;
}
else if(!strcmp("--port", argv[arg])) {
arg++;
if(argc>arg) {
char *endptr;
unsigned long ulnum = strtoul(argv[arg], &endptr, 10);
if((endptr != argv[arg] + strlen(argv[arg])) ||
((ulnum != 0UL) && ((ulnum < 1025UL) || (ulnum > 65535UL)))) {
fprintf(stderr, "sockfilt: invalid --port argument (%s)\n",
argv[arg]);
return 0;
}
port = curlx_ultous(ulnum);
arg++;
}
}
else if(!strcmp("--connect", argv[arg])) {


arg++;
if(argc>arg) {
char *endptr;
unsigned long ulnum = strtoul(argv[arg], &endptr, 10);
if((endptr != argv[arg] + strlen(argv[arg])) ||
(ulnum < 1025UL) || (ulnum > 65535UL)) {
fprintf(stderr, "sockfilt: invalid --connect argument (%s)\n",
argv[arg]);
return 0;
}
connectport = curlx_ultous(ulnum);
arg++;
}
}
else if(!strcmp("--addr", argv[arg])) {

arg++;
if(argc>arg) {
addr = argv[arg];
arg++;
}
}
else {
puts("Usage: sockfilt [option]\n"
" --version\n"
" --verbose\n"
" --logfile [file]\n"
" --pidfile [file]\n"
" --ipv4\n"
" --ipv6\n"
" --bindonly\n"
" --port [port]\n"
" --connect [port]\n"
" --addr [address]");
return 0;
}
}

#if defined(WIN32)
win32_init();
atexit(win32_cleanup);

setmode(fileno(stdin), O_BINARY);
setmode(fileno(stdout), O_BINARY);
setmode(fileno(stderr), O_BINARY);
#endif

install_signal_handlers();

#if defined(ENABLE_IPV6)
if(!use_ipv6)
#endif
sock = socket(AF_INET, SOCK_STREAM, 0);
#if defined(ENABLE_IPV6)
else
sock = socket(AF_INET6, SOCK_STREAM, 0);
#endif

if(CURL_SOCKET_BAD == sock) {
error = SOCKERRNO;
logmsg("Error creating socket: (%d) %s",
error, strerror(error));
write_stdout("FAIL\n", 5);
goto sockfilt_cleanup;
}

if(connectport) {

mode = ACTIVE;
#if defined(ENABLE_IPV6)
if(!use_ipv6) {
#endif
memset(&me.sa4, 0, sizeof(me.sa4));
me.sa4.sin_family = AF_INET;
me.sa4.sin_port = htons(connectport);
me.sa4.sin_addr.s_addr = INADDR_ANY;
if(!addr)
addr = "127.0.0.1";
Curl_inet_pton(AF_INET, addr, &me.sa4.sin_addr);

rc = connect(sock, &me.sa, sizeof(me.sa4));
#if defined(ENABLE_IPV6)
}
else {
memset(&me.sa6, 0, sizeof(me.sa6));
me.sa6.sin6_family = AF_INET6;
me.sa6.sin6_port = htons(connectport);
if(!addr)
addr = "::1";
Curl_inet_pton(AF_INET6, addr, &me.sa6.sin6_addr);

rc = connect(sock, &me.sa, sizeof(me.sa6));
}
#endif 
if(rc) {
error = SOCKERRNO;
logmsg("Error connecting to port %hu: (%d) %s",
connectport, error, strerror(error));
write_stdout("FAIL\n", 5);
goto sockfilt_cleanup;
}
logmsg("====> Client connect");
msgsock = sock; 
}
else {

sock = sockdaemon(sock, &port);
if(CURL_SOCKET_BAD == sock) {
write_stdout("FAIL\n", 5);
goto sockfilt_cleanup;
}
msgsock = CURL_SOCKET_BAD; 
}

logmsg("Running %s version", ipv_inuse);

if(connectport)
logmsg("Connected to port %hu", connectport);
else if(bind_only)
logmsg("Bound without listening on port %hu", port);
else
logmsg("Listening on port %hu", port);

wrotepidfile = write_pidfile(pidname);
if(!wrotepidfile) {
write_stdout("FAIL\n", 5);
goto sockfilt_cleanup;
}

do {
juggle_again = juggle(&msgsock, sock, &mode);
} while(juggle_again);

sockfilt_cleanup:

if((msgsock != sock) && (msgsock != CURL_SOCKET_BAD))
sclose(msgsock);

if(sock != CURL_SOCKET_BAD)
sclose(sock);

if(wrotepidfile)
unlink(pidname);

restore_signal_handlers();

if(got_exit_signal) {
logmsg("============> sockfilt exits with signal (%d)", exit_signal);





raise(exit_signal);
}

logmsg("============> sockfilt quits");
return 0;
}
