#include "server_setup.h"
#if defined(HAVE_SIGNAL_H)
#include <signal.h>
#endif
#if defined(HAVE_NETINET_IN_H)
#include <netinet/in.h>
#endif
#if defined(_XOPEN_SOURCE_EXTENDED)
#include <arpa/inet.h>
#endif
#if defined(HAVE_NETDB_H)
#include <netdb.h>
#endif
#if defined(HAVE_POLL_H)
#include <poll.h>
#elif defined(HAVE_SYS_POLL_H)
#include <sys/poll.h>
#endif
#if defined(__MINGW32__)
#include <w32api.h>
#endif
#define ENABLE_CURLX_PRINTF
#include "curlx.h" 
#include "getpart.h"
#include "util.h"
#include "timeval.h"
#if defined(USE_WINSOCK)
#undef EINTR
#define EINTR 4 
#undef EINVAL
#define EINVAL 22 
#endif
#if defined(ENABLE_IPV6) && defined(__MINGW32__)
#if (__W32API_MAJOR_VERSION < 3) || ((__W32API_MAJOR_VERSION == 3) && (__W32API_MINOR_VERSION < 6))
const struct in6_addr in6addr_any = {{ IN6ADDR_ANY_INIT }};
#endif 
#endif 
static struct timeval tvnow(void);
char *data_to_hex(char *data, size_t len)
{
static char buf[256*3];
size_t i;
char *optr = buf;
char *iptr = data;
if(len > 255)
len = 255;
for(i = 0; i < len; i++) {
if((data[i] >= 0x20) && (data[i] < 0x7f))
*optr++ = *iptr++;
else {
msnprintf(optr, 4, "%%%02x", *iptr++);
optr += 3;
}
}
*optr = 0; 
return buf;
}
void logmsg(const char *msg, ...)
{
va_list ap;
char buffer[2048 + 1];
FILE *logfp;
struct timeval tv;
time_t sec;
struct tm *now;
char timebuf[20];
static time_t epoch_offset;
static int known_offset;
if(!serverlogfile) {
fprintf(stderr, "Error: serverlogfile not set\n");
return;
}
tv = tvnow();
if(!known_offset) {
epoch_offset = time(NULL) - tv.tv_sec;
known_offset = 1;
}
sec = epoch_offset + tv.tv_sec;
now = localtime(&sec); 
msnprintf(timebuf, sizeof(timebuf), "%02d:%02d:%02d.%06ld",
(int)now->tm_hour, (int)now->tm_min, (int)now->tm_sec,
(long)tv.tv_usec);
va_start(ap, msg);
mvsnprintf(buffer, sizeof(buffer), msg, ap);
va_end(ap);
logfp = fopen(serverlogfile, "ab");
if(logfp) {
fprintf(logfp, "%s %s\n", timebuf, buffer);
fclose(logfp);
}
else {
int error = errno;
fprintf(stderr, "fopen() failed with error: %d %s\n",
error, strerror(error));
fprintf(stderr, "Error opening file: %s\n", serverlogfile);
fprintf(stderr, "Msg not logged: %s %s\n", timebuf, buffer);
}
}
#if defined(WIN32)
void win32_perror(const char *msg)
{
char buf[512];
DWORD err = SOCKERRNO;
if(!FormatMessageA((FORMAT_MESSAGE_FROM_SYSTEM |
FORMAT_MESSAGE_IGNORE_INSERTS), NULL, err,
LANG_NEUTRAL, buf, sizeof(buf), NULL))
msnprintf(buf, sizeof(buf), "Unknown error %lu (%#lx)", err, err);
if(msg)
fprintf(stderr, "%s: ", msg);
fprintf(stderr, "%s\n", buf);
}
#endif 
#if defined(USE_WINSOCK)
void win32_init(void)
{
WORD wVersionRequested;
WSADATA wsaData;
int err;
wVersionRequested = MAKEWORD(USE_WINSOCK, USE_WINSOCK);
err = WSAStartup(wVersionRequested, &wsaData);
if(err != 0) {
perror("Winsock init failed");
logmsg("Error initialising winsock -- aborting");
exit(1);
}
if(LOBYTE(wsaData.wVersion) != USE_WINSOCK ||
HIBYTE(wsaData.wVersion) != USE_WINSOCK) {
WSACleanup();
perror("Winsock init failed");
logmsg("No suitable winsock.dll found -- aborting");
exit(1);
}
}
void win32_cleanup(void)
{
WSACleanup();
}
#endif 
const char *path = ".";
char *test2file(long testno)
{
static char filename[256];
msnprintf(filename, sizeof(filename), TEST_DATA_PATH, path, testno);
return filename;
}
int wait_ms(int timeout_ms)
{
#if !defined(MSDOS) && !defined(USE_WINSOCK)
#if !defined(HAVE_POLL_FINE)
struct timeval pending_tv;
#endif
struct timeval initial_tv;
int pending_ms;
#endif
int r = 0;
if(!timeout_ms)
return 0;
if(timeout_ms < 0) {
errno = EINVAL;
return -1;
}
#if defined(MSDOS)
delay(timeout_ms);
#elif defined(USE_WINSOCK)
Sleep(timeout_ms);
#else
pending_ms = timeout_ms;
initial_tv = tvnow();
do {
int error;
#if defined(HAVE_POLL_FINE)
r = poll(NULL, 0, pending_ms);
#else
pending_tv.tv_sec = pending_ms / 1000;
pending_tv.tv_usec = (pending_ms % 1000) * 1000;
r = select(0, NULL, NULL, NULL, &pending_tv);
#endif 
if(r != -1)
break;
error = errno;
if(error && (error != EINTR))
break;
pending_ms = timeout_ms - (int)timediff(tvnow(), initial_tv);
if(pending_ms <= 0)
break;
} while(r == -1);
#endif 
if(r)
r = -1;
return r;
}
int write_pidfile(const char *filename)
{
FILE *pidfile;
long pid;
pid = (long)getpid();
pidfile = fopen(filename, "wb");
if(!pidfile) {
logmsg("Couldn't write pid file: %s %s", filename, strerror(errno));
return 0; 
}
fprintf(pidfile, "%ld\n", pid);
fclose(pidfile);
logmsg("Wrote pid %ld to %s", pid, filename);
return 1; 
}
void set_advisor_read_lock(const char *filename)
{
FILE *lockfile;
int error = 0;
int res;
do {
lockfile = fopen(filename, "wb");
} while((lockfile == NULL) && ((error = errno) == EINTR));
if(lockfile == NULL) {
logmsg("Error creating lock file %s error: %d %s",
filename, error, strerror(error));
return;
}
do {
res = fclose(lockfile);
} while(res && ((error = errno) == EINTR));
if(res)
logmsg("Error closing lock file %s error: %d %s",
filename, error, strerror(error));
}
void clear_advisor_read_lock(const char *filename)
{
int error = 0;
int res;
do {
res = unlink(filename);
} while(res && ((error = errno) == EINTR));
if(res)
logmsg("Error removing lock file %s error: %d %s",
filename, error, strerror(error));
}
static char raw_toupper(char in)
{
#if !defined(CURL_DOES_CONVERSIONS)
if(in >= 'a' && in <= 'z')
return (char)('A' + in - 'a');
#else
switch(in) {
case 'a':
return 'A';
case 'b':
return 'B';
case 'c':
return 'C';
case 'd':
return 'D';
case 'e':
return 'E';
case 'f':
return 'F';
case 'g':
return 'G';
case 'h':
return 'H';
case 'i':
return 'I';
case 'j':
return 'J';
case 'k':
return 'K';
case 'l':
return 'L';
case 'm':
return 'M';
case 'n':
return 'N';
case 'o':
return 'O';
case 'p':
return 'P';
case 'q':
return 'Q';
case 'r':
return 'R';
case 's':
return 'S';
case 't':
return 'T';
case 'u':
return 'U';
case 'v':
return 'V';
case 'w':
return 'W';
case 'x':
return 'X';
case 'y':
return 'Y';
case 'z':
return 'Z';
}
#endif
return in;
}
int strncasecompare(const char *first, const char *second, size_t max)
{
while(*first && *second && max) {
if(raw_toupper(*first) != raw_toupper(*second)) {
break;
}
max--;
first++;
second++;
}
if(0 == max)
return 1; 
return raw_toupper(*first) == raw_toupper(*second);
}
#if defined(WIN32) && !defined(MSDOS)
static struct timeval tvnow(void)
{
struct timeval now;
#if defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0600) && (!defined(__MINGW32__) || defined(__MINGW64_VERSION_MAJOR))
ULONGLONG milliseconds = GetTickCount64();
#else
DWORD milliseconds = GetTickCount();
#endif
now.tv_sec = (long)(milliseconds / 1000);
now.tv_usec = (long)((milliseconds % 1000) * 1000);
return now;
}
#elif defined(HAVE_CLOCK_GETTIME_MONOTONIC)
static struct timeval tvnow(void)
{
struct timeval now;
struct timespec tsnow;
if(0 == clock_gettime(CLOCK_MONOTONIC, &tsnow)) {
now.tv_sec = tsnow.tv_sec;
now.tv_usec = tsnow.tv_nsec / 1000;
}
#if defined(HAVE_GETTIMEOFDAY)
else
(void)gettimeofday(&now, NULL);
#else
else {
now.tv_sec = (long)time(NULL);
now.tv_usec = 0;
}
#endif
return now;
}
#elif defined(HAVE_GETTIMEOFDAY)
static struct timeval tvnow(void)
{
struct timeval now;
(void)gettimeofday(&now, NULL);
return now;
}
#else
static struct timeval tvnow(void)
{
struct timeval now;
now.tv_sec = (long)time(NULL);
now.tv_usec = 0;
return now;
}
#endif
long timediff(struct timeval newer, struct timeval older)
{
timediff_t diff = newer.tv_sec-older.tv_sec;
if(diff >= (LONG_MAX/1000))
return LONG_MAX;
else if(diff <= (LONG_MIN/1000))
return LONG_MIN;
return (long)(newer.tv_sec-older.tv_sec)*1000+
(long)(newer.tv_usec-older.tv_usec)/1000;
}
