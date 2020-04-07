

















































#include "server_setup.h"

#if defined(HAVE_SYS_IOCTL_H)
#include <sys/ioctl.h>
#endif
#if defined(HAVE_SIGNAL_H)
#include <signal.h>
#endif
#if defined(HAVE_FCNTL_H)
#include <fcntl.h>
#endif
#if defined(HAVE_NETINET_IN_H)
#include <netinet/in.h>
#endif
#if defined(HAVE_ARPA_INET_H)
#include <arpa/inet.h>
#endif
#if defined(HAVE_ARPA_TFTP_H)
#include <arpa/tftp.h>
#else
#include "tftp.h"
#endif
#if defined(HAVE_NETDB_H)
#include <netdb.h>
#endif
#if defined(HAVE_SYS_FILIO_H)

#include <sys/filio.h>
#endif

#include <setjmp.h>

#if defined(HAVE_PWD_H)
#include <pwd.h>
#endif

#define ENABLE_CURLX_PRINTF


#include "curlx.h" 
#include "getpart.h"
#include "util.h"
#include "server_sockaddr.h"


#include "memdebug.h"





#if !defined(PKTSIZE)
#define PKTSIZE (SEGSIZE + 4) 
#endif

struct testcase {
char *buffer; 
size_t bufsize; 
char *rptr; 
size_t rcount; 
long testno; 
int ofile; 

int writedelay; 
};

struct formats {
const char *f_mode;
int f_convert;
};

struct errmsg {
int e_code;
const char *e_msg;
};

typedef union {
struct tftphdr hdr;
char storage[PKTSIZE];
} tftphdr_storage_t;






struct bf {
int counter; 
tftphdr_storage_t buf; 
};

#define BF_ALLOC -3 
#define BF_FREE -2 

#define opcode_RRQ 1
#define opcode_WRQ 2
#define opcode_DATA 3
#define opcode_ACK 4
#define opcode_ERROR 5

#define TIMEOUT 5

#undef MIN
#define MIN(x,y) ((x)<(y)?(x):(y))

#if !defined(DEFAULT_LOGFILE)
#define DEFAULT_LOGFILE "log/tftpd.log"
#endif

#define REQUEST_DUMP "log/server.input"

#define DEFAULT_PORT 8999 





static struct errmsg errmsgs[] = {
{ EUNDEF, "Undefined error code" },
{ ENOTFOUND, "File not found" },
{ EACCESS, "Access violation" },
{ ENOSPACE, "Disk full or allocation exceeded" },
{ EBADOP, "Illegal TFTP operation" },
{ EBADID, "Unknown transfer ID" },
{ EEXISTS, "File already exists" },
{ ENOUSER, "No such user" },
{ -1, 0 }
};

static struct formats formata[] = {
{ "netascii", 1 },
{ "octet", 0 },
{ NULL, 0 }
};

static struct bf bfs[2];

static int nextone; 
static int current; 


static int newline = 0; 
static int prevchar = -1; 

static tftphdr_storage_t buf;
static tftphdr_storage_t ackbuf;

static srvr_sockaddr_union_t from;
static curl_socklen_t fromlen;

static curl_socket_t peer = CURL_SOCKET_BAD;

static unsigned int timeout;
static unsigned int maxtimeout = 5 * TIMEOUT;

#if defined(ENABLE_IPV6)
static bool use_ipv6 = FALSE;
#endif
static const char *ipv_inuse = "IPv4";

const char *serverlogfile = DEFAULT_LOGFILE;
static const char *pidname = ".tftpd.pid";
static int serverlogslocked = 0;
static int wrotepidfile = 0;

#if defined(HAVE_SIGSETJMP)
static sigjmp_buf timeoutbuf;
#endif

#if defined(HAVE_ALARM) && defined(SIGALRM)
static const unsigned int rexmtval = TIMEOUT;
#endif



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





static struct tftphdr *rw_init(int);

static struct tftphdr *w_init(void);

static struct tftphdr *r_init(void);

static void read_ahead(struct testcase *test, int convert);

static ssize_t write_behind(struct testcase *test, int convert);

static int synchnet(curl_socket_t);

static int do_tftp(struct testcase *test, struct tftphdr *tp, ssize_t size);

static int validate_access(struct testcase *test, const char *fname, int mode);

static void sendtftp(struct testcase *test, struct formats *pf);

static void recvtftp(struct testcase *test, struct formats *pf);

static void nak(int error);

#if defined(HAVE_ALARM) && defined(SIGALRM)

static void mysignal(int sig, void (*handler)(int));

static void timer(int signum);

static void justtimeout(int signum);

#endif 

static RETSIGTYPE exit_signal_handler(int signum);

static void install_signal_handlers(void);

static void restore_signal_handlers(void);





#if defined(HAVE_ALARM) && defined(SIGALRM)




static void mysignal(int sig, void (*handler)(int))
{
struct sigaction sa;
memset(&sa, 0, sizeof(sa));
sa.sa_handler = handler;
sigaction(sig, &sa, NULL);
}

static void timer(int signum)
{
(void)signum;

logmsg("alarm!");

timeout += rexmtval;
if(timeout >= maxtimeout) {
if(wrotepidfile) {
wrotepidfile = 0;
unlink(pidname);
}
if(serverlogslocked) {
serverlogslocked = 0;
clear_advisor_read_lock(SERVERLOGS_LOCK);
}
exit(1);
}
#if defined(HAVE_SIGSETJMP)
siglongjmp(timeoutbuf, 1);
#endif
}

static void justtimeout(int signum)
{
(void)signum;
}

#endif 






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





static struct tftphdr *rw_init(int x)
{
newline = 0; 
prevchar = -1;
bfs[0].counter = BF_ALLOC; 
current = 0;
bfs[1].counter = BF_FREE;
nextone = x; 
return &bfs[0].buf.hdr;
}

static struct tftphdr *w_init(void)
{
return rw_init(0); 
}

static struct tftphdr *r_init(void)
{
return rw_init(1); 
}




static int readit(struct testcase *test, struct tftphdr **dpp,
int convert )
{
struct bf *b;

bfs[current].counter = BF_FREE; 
current = !current; 

b = &bfs[current]; 
if(b->counter == BF_FREE) 
read_ahead(test, convert); 

*dpp = &b->buf.hdr; 
return b->counter;
}





static void read_ahead(struct testcase *test,
int convert )
{
int i;
char *p;
int c;
struct bf *b;
struct tftphdr *dp;

b = &bfs[nextone]; 
if(b->counter != BF_FREE) 
return;
nextone = !nextone; 

dp = &b->buf.hdr;

if(convert == 0) {


size_t copy_n = MIN(SEGSIZE, test->rcount);
memcpy(dp->th_data, test->rptr, copy_n);


test->rcount -= copy_n;
test->rptr += copy_n;
b->counter = (int)copy_n;
return;
}

p = dp->th_data;
for(i = 0 ; i < SEGSIZE; i++) {
if(newline) {
if(prevchar == '\n')
c = '\n'; 
else
c = '\0'; 
newline = 0;
}
else {
if(test->rcount) {
c = test->rptr[0];
test->rptr++;
test->rcount--;
}
else
break;
if(c == '\n' || c == '\r') {
prevchar = c;
c = '\r';
newline = 1;
}
}
*p++ = (char)c;
}
b->counter = (int)(p - dp->th_data);
}




static int writeit(struct testcase *test, struct tftphdr * volatile *dpp,
int ct, int convert)
{
bfs[current].counter = ct; 
current = !current; 
if(bfs[current].counter != BF_FREE) 
write_behind(test, convert); 
bfs[current].counter = BF_ALLOC; 
*dpp = &bfs[current].buf.hdr;
return ct; 
}







static ssize_t write_behind(struct testcase *test, int convert)
{
char *writebuf;
int count;
int ct;
char *p;
int c; 
struct bf *b;
struct tftphdr *dp;

b = &bfs[nextone];
if(b->counter < -1) 
return 0; 

if(!test->ofile) {
char outfile[256];
msnprintf(outfile, sizeof(outfile), "log/upload.%ld", test->testno);
#if defined(WIN32)
test->ofile = open(outfile, O_CREAT|O_RDWR|O_BINARY, 0777);
#else
test->ofile = open(outfile, O_CREAT|O_RDWR, 0777);
#endif
if(test->ofile == -1) {
logmsg("Couldn't create and/or open file %s for upload!", outfile);
return -1; 
}
}

count = b->counter; 
b->counter = BF_FREE; 
dp = &b->buf.hdr;
nextone = !nextone; 
writebuf = dp->th_data;

if(count <= 0)
return -1; 

if(convert == 0)
return write(test->ofile, writebuf, count);

p = writebuf;
ct = count;
while(ct--) { 
c = *p++; 
if(prevchar == '\r') { 
if(c == '\n') 
lseek(test->ofile, -1, SEEK_CUR); 
else
if(c == '\0') 
goto skipit; 

}


if(1 != write(test->ofile, &c, 1))
break;
skipit:
prevchar = c;
}
return count;
}












static int synchnet(curl_socket_t f )
{

#if defined(HAVE_IOCTLSOCKET)
unsigned long i;
#else
int i;
#endif
int j = 0;
char rbuf[PKTSIZE];
srvr_sockaddr_union_t fromaddr;
curl_socklen_t fromaddrlen;

for(;;) {
#if defined(HAVE_IOCTLSOCKET)
(void) ioctlsocket(f, FIONREAD, &i);
#else
(void) ioctl(f, FIONREAD, &i);
#endif
if(i) {
j++;
#if defined(ENABLE_IPV6)
if(!use_ipv6)
#endif
fromaddrlen = sizeof(fromaddr.sa4);
#if defined(ENABLE_IPV6)
else
fromaddrlen = sizeof(fromaddr.sa6);
#endif
(void) recvfrom(f, rbuf, sizeof(rbuf), 0,
&fromaddr.sa, &fromaddrlen);
}
else
break;
}
return j;
}

int main(int argc, char **argv)
{
srvr_sockaddr_union_t me;
struct tftphdr *tp;
ssize_t n = 0;
int arg = 1;
unsigned short port = DEFAULT_PORT;
curl_socket_t sock = CURL_SOCKET_BAD;
int flag;
int rc;
int error;
long pid;
struct testcase test;
int result = 0;

memset(&test, 0, sizeof(test));

while(argc>arg) {
if(!strcmp("--version", argv[arg])) {
printf("tftpd IPv4%s\n",
#if defined(ENABLE_IPV6)
"/IPv6"
#else
""
#endif
);
return 0;
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
else if(!strcmp("--ipv4", argv[arg])) {
#if defined(ENABLE_IPV6)
ipv_inuse = "IPv4";
use_ipv6 = FALSE;
#endif
arg++;
}
else if(!strcmp("--ipv6", argv[arg])) {
#if defined(ENABLE_IPV6)
ipv_inuse = "IPv6";
use_ipv6 = TRUE;
#endif
arg++;
}
else if(!strcmp("--port", argv[arg])) {
arg++;
if(argc>arg) {
char *endptr;
unsigned long ulnum = strtoul(argv[arg], &endptr, 10);
if((endptr != argv[arg] + strlen(argv[arg])) ||
(ulnum < 1025UL) || (ulnum > 65535UL)) {
fprintf(stderr, "tftpd: invalid --port argument (%s)\n",
argv[arg]);
return 0;
}
port = curlx_ultous(ulnum);
arg++;
}
}
else if(!strcmp("--srcdir", argv[arg])) {
arg++;
if(argc>arg) {
path = argv[arg];
arg++;
}
}
else {
puts("Usage: tftpd [option]\n"
" --version\n"
" --logfile [file]\n"
" --pidfile [file]\n"
" --ipv4\n"
" --ipv6\n"
" --port [port]\n"
" --srcdir [path]");
return 0;
}
}

#if defined(WIN32)
win32_init();
atexit(win32_cleanup);
#endif

install_signal_handlers();

pid = (long)getpid();

#if defined(ENABLE_IPV6)
if(!use_ipv6)
#endif
sock = socket(AF_INET, SOCK_DGRAM, 0);
#if defined(ENABLE_IPV6)
else
sock = socket(AF_INET6, SOCK_DGRAM, 0);
#endif

if(CURL_SOCKET_BAD == sock) {
error = SOCKERRNO;
logmsg("Error creating socket: (%d) %s",
error, strerror(error));
result = 1;
goto tftpd_cleanup;
}

flag = 1;
if(0 != setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
(void *)&flag, sizeof(flag))) {
error = SOCKERRNO;
logmsg("setsockopt(SO_REUSEADDR) failed with error: (%d) %s",
error, strerror(error));
result = 1;
goto tftpd_cleanup;
}

#if defined(ENABLE_IPV6)
if(!use_ipv6) {
#endif
memset(&me.sa4, 0, sizeof(me.sa4));
me.sa4.sin_family = AF_INET;
me.sa4.sin_addr.s_addr = INADDR_ANY;
me.sa4.sin_port = htons(port);
rc = bind(sock, &me.sa, sizeof(me.sa4));
#if defined(ENABLE_IPV6)
}
else {
memset(&me.sa6, 0, sizeof(me.sa6));
me.sa6.sin6_family = AF_INET6;
me.sa6.sin6_addr = in6addr_any;
me.sa6.sin6_port = htons(port);
rc = bind(sock, &me.sa, sizeof(me.sa6));
}
#endif 
if(0 != rc) {
error = SOCKERRNO;
logmsg("Error binding socket on port %hu: (%d) %s",
port, error, strerror(error));
result = 1;
goto tftpd_cleanup;
}

wrotepidfile = write_pidfile(pidname);
if(!wrotepidfile) {
result = 1;
goto tftpd_cleanup;
}

logmsg("Running %s version on port UDP/%d", ipv_inuse, (int)port);

for(;;) {
fromlen = sizeof(from);
#if defined(ENABLE_IPV6)
if(!use_ipv6)
#endif
fromlen = sizeof(from.sa4);
#if defined(ENABLE_IPV6)
else
fromlen = sizeof(from.sa6);
#endif
n = (ssize_t)recvfrom(sock, &buf.storage[0], sizeof(buf.storage), 0,
&from.sa, &fromlen);
if(got_exit_signal)
break;
if(n < 0) {
logmsg("recvfrom");
result = 3;
break;
}

set_advisor_read_lock(SERVERLOGS_LOCK);
serverlogslocked = 1;

#if defined(ENABLE_IPV6)
if(!use_ipv6) {
#endif
from.sa4.sin_family = AF_INET;
peer = socket(AF_INET, SOCK_DGRAM, 0);
if(CURL_SOCKET_BAD == peer) {
logmsg("socket");
result = 2;
break;
}
if(connect(peer, &from.sa, sizeof(from.sa4)) < 0) {
logmsg("connect: fail");
result = 1;
break;
}
#if defined(ENABLE_IPV6)
}
else {
from.sa6.sin6_family = AF_INET6;
peer = socket(AF_INET6, SOCK_DGRAM, 0);
if(CURL_SOCKET_BAD == peer) {
logmsg("socket");
result = 2;
break;
}
if(connect(peer, &from.sa, sizeof(from.sa6)) < 0) {
logmsg("connect: fail");
result = 1;
break;
}
}
#endif

maxtimeout = 5*TIMEOUT;

tp = &buf.hdr;
tp->th_opcode = ntohs(tp->th_opcode);
if(tp->th_opcode == opcode_RRQ || tp->th_opcode == opcode_WRQ) {
memset(&test, 0, sizeof(test));
if(do_tftp(&test, tp, n) < 0)
break;
free(test.buffer);
}
sclose(peer);
peer = CURL_SOCKET_BAD;

if(test.ofile > 0) {
close(test.ofile);
test.ofile = 0;
}

if(got_exit_signal)
break;

if(serverlogslocked) {
serverlogslocked = 0;
clear_advisor_read_lock(SERVERLOGS_LOCK);
}

logmsg("end of one transfer");

}

tftpd_cleanup:

if(test.ofile > 0)
close(test.ofile);

if((peer != sock) && (peer != CURL_SOCKET_BAD))
sclose(peer);

if(sock != CURL_SOCKET_BAD)
sclose(sock);

if(got_exit_signal)
logmsg("signalled to die");

if(wrotepidfile)
unlink(pidname);

if(serverlogslocked) {
serverlogslocked = 0;
clear_advisor_read_lock(SERVERLOGS_LOCK);
}

restore_signal_handlers();

if(got_exit_signal) {
logmsg("========> %s tftpd (port: %d pid: %ld) exits with signal (%d)",
ipv_inuse, (int)port, pid, exit_signal);





raise(exit_signal);
}

logmsg("========> tftpd quits");
return result;
}




static int do_tftp(struct testcase *test, struct tftphdr *tp, ssize_t size)
{
char *cp;
int first = 1, ecode;
struct formats *pf;
char *filename, *mode = NULL;
#if defined(USE_WINSOCK)
DWORD recvtimeout, recvtimeoutbak;
#endif
const char *option = "mode"; 
int toggle = 1;


FILE *server = fopen(REQUEST_DUMP, "ab");
if(!server) {
int error = errno;
logmsg("fopen() failed with error: %d %s", error, strerror(error));
logmsg("Error opening file: %s", REQUEST_DUMP);
return -1;
}


fprintf(server, "opcode: %x\n", tp->th_opcode);

cp = (char *)&tp->th_stuff;
filename = cp;
do {
bool endofit = true;
while(cp < &buf.storage[size]) {
if(*cp == '\0') {
endofit = false;
break;
}
cp++;
}
if(endofit)

break;



if((cp + 1) < &buf.storage[size]) {
++cp;
if(first) {

mode = cp;
first = 0;
}
if(toggle)

fprintf(server, "%s: %s\n", option, cp);
else {

option = cp;
}
toggle ^= 1;
}
else

break;
} while(1);

if(*cp) {
nak(EBADOP);
fclose(server);
return 3;
}


fprintf(server, "filename: %s\n", filename);

for(cp = mode; cp && *cp; cp++)
if(ISUPPER(*cp))
*cp = (char)tolower((int)*cp);


fclose(server);

for(pf = formata; pf->f_mode; pf++)
if(strcmp(pf->f_mode, mode) == 0)
break;
if(!pf->f_mode) {
nak(EBADOP);
return 2;
}
ecode = validate_access(test, filename, tp->th_opcode);
if(ecode) {
nak(ecode);
return 1;
}

#if defined(USE_WINSOCK)
recvtimeout = sizeof(recvtimeoutbak);
getsockopt(peer, SOL_SOCKET, SO_RCVTIMEO,
(char *)&recvtimeoutbak, (int *)&recvtimeout);
recvtimeout = TIMEOUT*1000;
setsockopt(peer, SOL_SOCKET, SO_RCVTIMEO,
(const char *)&recvtimeout, sizeof(recvtimeout));
#endif

if(tp->th_opcode == opcode_WRQ)
recvtftp(test, pf);
else
sendtftp(test, pf);

#if defined(USE_WINSOCK)
recvtimeout = recvtimeoutbak;
setsockopt(peer, SOL_SOCKET, SO_RCVTIMEO,
(const char *)&recvtimeout, sizeof(recvtimeout));
#endif

return 0;
}


static int parse_servercmd(struct testcase *req)
{
FILE *stream;
char *filename;
int error;

filename = test2file(req->testno);

stream = fopen(filename, "rb");
if(!stream) {
error = errno;
logmsg("fopen() failed with error: %d %s", error, strerror(error));
logmsg(" [1] Error opening file: %s", filename);
logmsg(" Couldn't open test file %ld", req->testno);
return 1; 
}
else {
char *orgcmd = NULL;
char *cmd = NULL;
size_t cmdsize = 0;
int num = 0;


error = getpart(&orgcmd, &cmdsize, "reply", "servercmd", stream);
fclose(stream);
if(error) {
logmsg("getpart() failed with error: %d", error);
return 1; 
}

cmd = orgcmd;
while(cmd && cmdsize) {
char *check;
if(1 == sscanf(cmd, "writedelay: %d", &num)) {
logmsg("instructed to delay %d secs between packets", num);
req->writedelay = num;
}
else {
logmsg("Unknown <servercmd> instruction found: %s", cmd);
}

check = strchr(cmd, '\r');
if(!check)
check = strchr(cmd, '\n');

if(check) {

while((*check == '\r') || (*check == '\n'))
check++;

if(!*check)

break;
cmd = check;
}
else
break;
}
free(orgcmd);
}

return 0; 
}





static int validate_access(struct testcase *test,
const char *filename, int mode)
{
char *ptr;

logmsg("trying to get file: %s mode %x", filename, mode);

if(!strncmp("verifiedserver", filename, 14)) {
char weare[128];
size_t count = msnprintf(weare, sizeof(weare),
"WE ROOLZ: %ld\r\n", (long)getpid());

logmsg("Are-we-friendly question received");
test->buffer = strdup(weare);
test->rptr = test->buffer; 
test->bufsize = count; 
test->rcount = count; 
return 0; 
}


ptr = strrchr(filename, '/');

if(ptr) {
char partbuf[80]="data";
long partno;
long testno;
char *file;

ptr++; 


while(*ptr && !ISDIGIT(*ptr))
ptr++;


testno = strtol(ptr, &ptr, 10);

if(testno > 10000) {
partno = testno % 10000;
testno /= 10000;
}
else
partno = 0;


logmsg("requested test number %ld part %ld", testno, partno);

test->testno = testno;

(void)parse_servercmd(test);

file = test2file(testno);

if(0 != partno)
msnprintf(partbuf, sizeof(partbuf), "data%ld", partno);

if(file) {
FILE *stream = fopen(file, "rb");
if(!stream) {
int error = errno;
logmsg("fopen() failed with error: %d %s", error, strerror(error));
logmsg("Error opening file: %s", file);
logmsg("Couldn't open test file: %s", file);
return EACCESS;
}
else {
size_t count;
int error = getpart(&test->buffer, &count, "reply", partbuf, stream);
fclose(stream);
if(error) {
logmsg("getpart() failed with error: %d", error);
return EACCESS;
}
if(test->buffer) {
test->rptr = test->buffer; 
test->bufsize = count; 
test->rcount = count; 
}
else
return EACCESS;
}

}
else
return EACCESS;
}
else {
logmsg("no slash found in path");
return EACCESS; 
}

logmsg("file opened and all is good");
return 0;
}




static void sendtftp(struct testcase *test, struct formats *pf)
{
int size;
ssize_t n;

volatile unsigned short sendblock; 
struct tftphdr * volatile sdp = r_init(); 
struct tftphdr * const sap = &ackbuf.hdr; 

sendblock = 1;
#if defined(HAVE_ALARM) && defined(SIGALRM)
mysignal(SIGALRM, timer);
#endif
do {
size = readit(test, (struct tftphdr **)&sdp, pf->f_convert);
if(size < 0) {
nak(errno + 100);
return;
}
sdp->th_opcode = htons((unsigned short)opcode_DATA);
sdp->th_block = htons(sendblock);
timeout = 0;
#if defined(HAVE_SIGSETJMP)
(void) sigsetjmp(timeoutbuf, 1);
#endif
if(test->writedelay) {
logmsg("Pausing %d seconds before %d bytes", test->writedelay,
size);
wait_ms(1000*test->writedelay);
}

send_data:
if(swrite(peer, sdp, size + 4) != size + 4) {
logmsg("write");
return;
}
read_ahead(test, pf->f_convert);
for(;;) {
#if defined(HAVE_ALARM)
alarm(rexmtval); 
#endif
n = sread(peer, &ackbuf.storage[0], sizeof(ackbuf.storage));
#if defined(HAVE_ALARM)
alarm(0);
#endif
if(got_exit_signal)
return;
if(n < 0) {
logmsg("read: fail");
return;
}
sap->th_opcode = ntohs((unsigned short)sap->th_opcode);
sap->th_block = ntohs(sap->th_block);

if(sap->th_opcode == opcode_ERROR) {
logmsg("got ERROR");
return;
}

if(sap->th_opcode == opcode_ACK) {
if(sap->th_block == sendblock) {
break;
}

(void) synchnet(peer);
if(sap->th_block == (sendblock-1)) {
goto send_data;
}
}

}
sendblock++;
} while(size == SEGSIZE);
}




static void recvtftp(struct testcase *test, struct formats *pf)
{
ssize_t n, size;

volatile unsigned short recvblock; 
struct tftphdr * volatile rdp; 
struct tftphdr *rap; 

recvblock = 0;
rdp = w_init();
#if defined(HAVE_ALARM) && defined(SIGALRM)
mysignal(SIGALRM, timer);
#endif
rap = &ackbuf.hdr;
do {
timeout = 0;
rap->th_opcode = htons((unsigned short)opcode_ACK);
rap->th_block = htons(recvblock);
recvblock++;
#if defined(HAVE_SIGSETJMP)
(void) sigsetjmp(timeoutbuf, 1);
#endif
send_ack:
if(swrite(peer, &ackbuf.storage[0], 4) != 4) {
logmsg("write: fail\n");
goto abort;
}
write_behind(test, pf->f_convert);
for(;;) {
#if defined(HAVE_ALARM)
alarm(rexmtval);
#endif
n = sread(peer, rdp, PKTSIZE);
#if defined(HAVE_ALARM)
alarm(0);
#endif
if(got_exit_signal)
goto abort;
if(n < 0) { 
logmsg("read: fail\n");
goto abort;
}
rdp->th_opcode = ntohs((unsigned short)rdp->th_opcode);
rdp->th_block = ntohs(rdp->th_block);
if(rdp->th_opcode == opcode_ERROR)
goto abort;
if(rdp->th_opcode == opcode_DATA) {
if(rdp->th_block == recvblock) {
break; 
}

(void) synchnet(peer);
if(rdp->th_block == (recvblock-1))
goto send_ack; 
}
}

size = writeit(test, &rdp, (int)(n - 4), pf->f_convert);
if(size != (n-4)) { 
if(size < 0)
nak(errno + 100);
else
nak(ENOSPACE);
goto abort;
}
} while(size == SEGSIZE);
write_behind(test, pf->f_convert);

rap->th_opcode = htons((unsigned short)opcode_ACK); 

rap->th_block = htons(recvblock);
(void) swrite(peer, &ackbuf.storage[0], 4);
#if defined(HAVE_ALARM) && defined(SIGALRM)
mysignal(SIGALRM, justtimeout); 
alarm(rexmtval);
#endif

n = sread(peer, &buf.storage[0], sizeof(buf.storage));
#if defined(HAVE_ALARM)
alarm(0);
#endif
if(got_exit_signal)
goto abort;
if(n >= 4 && 
rdp->th_opcode == opcode_DATA && 
recvblock == rdp->th_block) { 
(void) swrite(peer, &ackbuf.storage[0], 4); 
}
abort:
return;
}





static void nak(int error)
{
struct tftphdr *tp;
int length;
struct errmsg *pe;

tp = &buf.hdr;
tp->th_opcode = htons((unsigned short)opcode_ERROR);
tp->th_code = htons((unsigned short)error);
for(pe = errmsgs; pe->e_code >= 0; pe++)
if(pe->e_code == error)
break;
if(pe->e_code < 0) {
pe->e_msg = strerror(error - 100);
tp->th_code = EUNDEF; 
}
length = (int)strlen(pe->e_msg);



memcpy(tp->th_msg, pe->e_msg, length + 1);
length += 5;
if(swrite(peer, &buf.storage[0], length) != length)
logmsg("nak: fail\n");
}
