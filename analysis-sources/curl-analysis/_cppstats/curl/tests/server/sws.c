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
#if defined(HAVE_NETINET_TCP_H)
#include <netinet/tcp.h> 
#endif
#define ENABLE_CURLX_PRINTF
#include "curlx.h" 
#include "getpart.h"
#include "inet_pton.h"
#include "util.h"
#include "server_sockaddr.h"
#include "memdebug.h"
#if defined(USE_WINSOCK)
#undef EINTR
#define EINTR 4 
#undef EAGAIN
#define EAGAIN 11 
#undef ERANGE
#define ERANGE 34 
#endif
static enum {
socket_domain_inet = AF_INET
#if defined(ENABLE_IPV6)
, socket_domain_inet6 = AF_INET6
#endif
#if defined(USE_UNIX_SOCKETS)
, socket_domain_unix = AF_UNIX
#endif
} socket_domain = AF_INET;
static bool use_gopher = FALSE;
static int serverlogslocked = 0;
static bool is_proxy = FALSE;
#define REQBUFSIZ 150000
#define REQBUFSIZ_TXT "149999"
static long prevtestno = -1; 
static long prevpartno = -1; 
static bool prevbounce = FALSE; 
#define RCMD_NORMALREQ 0 
#define RCMD_IDLE 1 
#define RCMD_STREAM 2 
struct httprequest {
char reqbuf[REQBUFSIZ]; 
bool connect_request; 
unsigned short connect_port; 
size_t checkindex; 
size_t offset; 
long testno; 
long partno; 
bool open; 
bool auth_req; 
bool auth; 
size_t cl; 
bool digest; 
bool ntlm; 
int writedelay; 
int skip; 
int rcmd; 
int prot_version; 
int callcount; 
bool skipall; 
bool noexpect; 
bool connmon; 
bool upgrade; 
bool upgrade_request; 
bool close; 
int done_processing;
};
#define MAX_SOCKETS 1024
static curl_socket_t all_sockets[MAX_SOCKETS];
static size_t num_sockets = 0;
static int ProcessRequest(struct httprequest *req);
static void storerequest(const char *reqbuf, size_t totalsize);
#define DEFAULT_PORT 8999
#if !defined(DEFAULT_LOGFILE)
#define DEFAULT_LOGFILE "log/sws.log"
#endif
const char *serverlogfile = DEFAULT_LOGFILE;
#define SWSVERSION "curl test suite HTTP server/0.1"
#define REQUEST_DUMP "log/server.input"
#define RESPONSE_DUMP "log/server.response"
#define REQUEST_PROXY_DUMP "log/proxy.input"
#define RESPONSE_PROXY_DUMP "log/proxy.response"
#define MAXDOCNAMELEN 140000
#define MAXDOCNAMELEN_TXT "139999"
#define REQUEST_KEYWORD_SIZE 256
#define REQUEST_KEYWORD_SIZE_TXT "255"
#define CMD_AUTH_REQUIRED "auth_required"
#define CMD_IDLE "idle"
#define CMD_STREAM "stream"
#define CMD_CONNECTIONMONITOR "connection-monitor"
#define CMD_UPGRADE "upgrade"
#define CMD_SWSCLOSE "swsclose"
#define CMD_NOEXPECT "no-expect"
#define END_OF_HEADERS "\r\n\r\n"
enum {
DOCNUMBER_NOTHING = -4,
DOCNUMBER_QUIT = -3,
DOCNUMBER_WERULEZ = -2,
DOCNUMBER_404 = -1
};
static const char *end_of_headers = END_OF_HEADERS;
static const char *docquit =
"HTTP/1.1 200 Goodbye" END_OF_HEADERS;
static const char *doc404 = "HTTP/1.1 404 Not Found\r\n"
"Server: " SWSVERSION "\r\n"
"Connection: close\r\n"
"Content-Type: text/html"
END_OF_HEADERS
"<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n"
"<HTML><HEAD>\n"
"<TITLE>404 Not Found</TITLE>\n"
"</HEAD><BODY>\n"
"<H1>Not Found</H1>\n"
"The requested URL was not found on this server.\n"
"<P><HR><ADDRESS>" SWSVERSION "</ADDRESS>\n" "</BODY></HTML>\n";
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
static int already_recv_zeroed_chunk = FALSE;
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
static bool socket_domain_is_ip(void)
{
switch(socket_domain) {
case AF_INET:
#if defined(ENABLE_IPV6)
case AF_INET6:
#endif
return true;
default:
return false;
}
}
static int parse_servercmd(struct httprequest *req)
{
FILE *stream;
char *filename;
int error;
filename = test2file(req->testno);
req->close = FALSE;
req->connmon = FALSE;
stream = fopen(filename, "rb");
if(!stream) {
error = errno;
logmsg("fopen() failed with error: %d %s", error, strerror(error));
logmsg(" [1] Error opening file: %s", filename);
logmsg(" Couldn't open test file %ld", req->testno);
req->open = FALSE; 
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
req->open = FALSE; 
return 1; 
}
cmd = orgcmd;
while(cmd && cmdsize) {
char *check;
if(!strncmp(CMD_AUTH_REQUIRED, cmd, strlen(CMD_AUTH_REQUIRED))) {
logmsg("instructed to require authorization header");
req->auth_req = TRUE;
}
else if(!strncmp(CMD_IDLE, cmd, strlen(CMD_IDLE))) {
logmsg("instructed to idle");
req->rcmd = RCMD_IDLE;
req->open = TRUE;
}
else if(!strncmp(CMD_STREAM, cmd, strlen(CMD_STREAM))) {
logmsg("instructed to stream");
req->rcmd = RCMD_STREAM;
}
else if(!strncmp(CMD_CONNECTIONMONITOR, cmd,
strlen(CMD_CONNECTIONMONITOR))) {
logmsg("enabled connection monitoring");
req->connmon = TRUE;
}
else if(!strncmp(CMD_UPGRADE, cmd, strlen(CMD_UPGRADE))) {
logmsg("enabled upgrade to http2");
req->upgrade = TRUE;
}
else if(!strncmp(CMD_SWSCLOSE, cmd, strlen(CMD_SWSCLOSE))) {
logmsg("swsclose: close this connection after response");
req->close = TRUE;
}
else if(1 == sscanf(cmd, "skip: %d", &num)) {
logmsg("instructed to skip this number of bytes %d", num);
req->skip = num;
}
else if(!strncmp(CMD_NOEXPECT, cmd, strlen(CMD_NOEXPECT))) {
logmsg("instructed to reject Expect: 100-continue");
req->noexpect = TRUE;
}
else if(1 == sscanf(cmd, "writedelay: %d", &num)) {
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
static int ProcessRequest(struct httprequest *req)
{
char *line = &req->reqbuf[req->checkindex];
bool chunked = FALSE;
static char request[REQUEST_KEYWORD_SIZE];
static char doc[MAXDOCNAMELEN];
char logbuf[456];
int prot_major, prot_minor;
char *end = strstr(line, end_of_headers);
req->callcount++;
logmsg("Process %d bytes request%s", req->offset,
req->callcount > 1?" [CONTINUED]":"");
if(use_gopher &&
(req->testno == DOCNUMBER_NOTHING) &&
!strncmp("/verifiedserver", line, 15)) {
logmsg("Are-we-friendly question received");
req->testno = DOCNUMBER_WERULEZ;
return 1; 
}
else if((req->testno == DOCNUMBER_NOTHING) &&
sscanf(line,
"%" REQUEST_KEYWORD_SIZE_TXT"s %" MAXDOCNAMELEN_TXT "s HTTP/%d.%d",
request,
doc,
&prot_major,
&prot_minor) == 4) {
char *ptr;
req->prot_version = prot_major*10 + prot_minor;
ptr = strrchr(doc, '/');
if(ptr) {
if((strlen(doc) + strlen(request)) < 400)
msnprintf(logbuf, sizeof(logbuf), "Got request: %s %s HTTP/%d.%d",
request, doc, prot_major, prot_minor);
else
msnprintf(logbuf, sizeof(logbuf), "Got a *HUGE* request HTTP/%d.%d",
prot_major, prot_minor);
logmsg("%s", logbuf);
if(!strncmp("/verifiedserver", ptr, 15)) {
logmsg("Are-we-friendly question received");
req->testno = DOCNUMBER_WERULEZ;
return 1; 
}
if(!strncmp("/quit", ptr, 5)) {
logmsg("Request-to-quit received");
req->testno = DOCNUMBER_QUIT;
return 1; 
}
ptr++; 
while(*ptr && !ISDIGIT(*ptr))
ptr++;
req->testno = strtol(ptr, &ptr, 10);
if(req->testno > 10000) {
req->partno = req->testno % 10000;
req->testno /= 10000;
}
else
req->partno = 0;
if(req->testno) {
msnprintf(logbuf, sizeof(logbuf), "Requested test number %ld part %ld",
req->testno, req->partno);
logmsg("%s", logbuf);
}
else {
logmsg("No test number");
req->testno = DOCNUMBER_NOTHING;
}
}
if(req->testno == DOCNUMBER_NOTHING) {
if(sscanf(req->reqbuf, "CONNECT %" MAXDOCNAMELEN_TXT "s HTTP/%d.%d",
doc, &prot_major, &prot_minor) == 3) {
char *portp = NULL;
msnprintf(logbuf, sizeof(logbuf),
"Received a CONNECT %s HTTP/%d.%d request",
doc, prot_major, prot_minor);
logmsg("%s", logbuf);
req->connect_request = TRUE;
if(req->prot_version == 10)
req->open = FALSE; 
if(doc[0] == '[') {
char *p = &doc[1];
unsigned long part = 0;
while(*p && (ISXDIGIT(*p) || (*p == ':') || (*p == '.'))) {
char *endp;
part = strtoul(p, &endp, 16);
if(ISXDIGIT(*p))
p = endp;
else
p++;
}
if(*p != ']')
logmsg("Invalid CONNECT IPv6 address format");
else if(*(p + 1) != ':')
logmsg("Invalid CONNECT IPv6 port format");
else
portp = p + 1;
req->testno = part;
}
else
portp = strchr(doc, ':');
if(portp && (*(portp + 1) != '\0') && ISDIGIT(*(portp + 1))) {
unsigned long ulnum = strtoul(portp + 1, NULL, 10);
if(!ulnum || (ulnum > 65535UL))
logmsg("Invalid CONNECT port received");
else
req->connect_port = curlx_ultous(ulnum);
}
logmsg("Port number: %d, test case number: %ld",
req->connect_port, req->testno);
}
}
if(req->testno == DOCNUMBER_NOTHING) {
ptr = strrchr(doc, '.');
if(ptr) {
ptr++; 
req->testno = strtol(ptr, &ptr, 10);
if(req->testno > 10000) {
req->partno = req->testno % 10000;
req->testno /= 10000;
logmsg("found test %d in requested host name", req->testno);
}
else
req->partno = 0;
msnprintf(logbuf, sizeof(logbuf),
"Requested test number %ld part %ld (from host name)",
req->testno, req->partno);
logmsg("%s", logbuf);
}
if(!req->testno) {
logmsg("Did not find test number in PATH");
req->testno = DOCNUMBER_404;
}
else
parse_servercmd(req);
}
}
else if((req->offset >= 3) && (req->testno == DOCNUMBER_NOTHING)) {
logmsg("** Unusual request. Starts with %02x %02x %02x (%c%c%c)",
line[0], line[1], line[2], line[0], line[1], line[2]);
}
if(!end) {
logmsg("request not complete yet");
return 0; 
}
logmsg("- request found to be complete (%d)", req->testno);
if(req->testno == DOCNUMBER_NOTHING) {
char *testno = strstr(line, "\nTestno: ");
if(testno) {
req->testno = strtol(&testno[9], NULL, 10);
logmsg("Found test number %d in Testno: header!", req->testno);
}
else {
logmsg("No Testno: header");
}
}
parse_servercmd(req);
if(use_gopher) {
char *ptr;
ptr = strrchr(line, '/');
if(ptr) {
ptr++; 
while(*ptr && !ISDIGIT(*ptr))
ptr++;
req->testno = strtol(ptr, &ptr, 10);
if(req->testno > 10000) {
req->partno = req->testno % 10000;
req->testno /= 10000;
}
else
req->partno = 0;
msnprintf(logbuf, sizeof(logbuf),
"Requested GOPHER test number %ld part %ld",
req->testno, req->partno);
logmsg("%s", logbuf);
}
}
do {
if(got_exit_signal)
return 1; 
if((req->cl == 0) && strncasecompare("Content-Length:", line, 15)) {
char *endptr;
char *ptr = line + 15;
unsigned long clen = 0;
while(*ptr && ISSPACE(*ptr))
ptr++;
endptr = ptr;
errno = 0;
clen = strtoul(ptr, &endptr, 10);
if((ptr == endptr) || !ISSPACE(*endptr) || (ERANGE == errno)) {
logmsg("Found invalid Content-Length: (%s) in the request", ptr);
req->open = FALSE; 
return 1; 
}
if(req->skipall)
req->cl = 0;
else
req->cl = clen - req->skip;
logmsg("Found Content-Length: %lu in the request", clen);
if(req->skip)
logmsg("... but will abort after %zu bytes", req->cl);
}
else if(strncasecompare("Transfer-Encoding: chunked", line,
strlen("Transfer-Encoding: chunked"))) {
chunked = TRUE;
}
else if(req->noexpect &&
strncasecompare("Expect: 100-continue", line,
strlen("Expect: 100-continue"))) {
if(req->cl)
req->cl = 0;
req->skipall = TRUE;
logmsg("Found Expect: 100-continue, ignore body");
}
if(chunked) {
if(strstr(req->reqbuf, "\r\n0\r\n\r\n")) {
return 1; 
}
else if(strstr(req->reqbuf, "\r\n0\r\n")) {
char *last_crlf_char = strstr(req->reqbuf, "\r\n\r\n");
while(TRUE) {
if(!strstr(last_crlf_char + 4, "\r\n\r\n"))
break;
last_crlf_char = strstr(last_crlf_char + 4, "\r\n\r\n");
}
if(last_crlf_char &&
last_crlf_char > strstr(req->reqbuf, "\r\n0\r\n"))
return 1;
already_recv_zeroed_chunk = TRUE;
return 0;
}
else if(already_recv_zeroed_chunk && strstr(req->reqbuf, "\r\n\r\n"))
return 1;
else
return 0; 
}
line = strchr(line, '\n');
if(line)
line++;
} while(line);
if(!req->auth && strstr(req->reqbuf, "Authorization:")) {
req->auth = TRUE; 
if(req->auth_req)
logmsg("Authorization header found, as required");
}
if(strstr(req->reqbuf, "Authorization: Negotiate")) {
static long prev_testno = -1;
static long prev_partno = -1;
logmsg("Negotiate: prev_testno: %d, prev_partno: %d",
prev_testno, prev_partno);
if(req->testno != prev_testno) {
prev_testno = req->testno;
prev_partno = req->partno;
}
prev_partno += 1;
req->partno = prev_partno;
}
else if(!req->digest && strstr(req->reqbuf, "Authorization: Digest")) {
req->partno += 1000;
req->digest = TRUE; 
logmsg("Received Digest request, sending back data %ld", req->partno);
}
else if(!req->ntlm &&
strstr(req->reqbuf, "Authorization: NTLM TlRMTVNTUAAD")) {
req->partno += 1002;
req->ntlm = TRUE; 
logmsg("Received NTLM type-3, sending back data %ld", req->partno);
if(req->cl) {
logmsg(" Expecting %zu POSTed bytes", req->cl);
}
}
else if(!req->ntlm &&
strstr(req->reqbuf, "Authorization: NTLM TlRMTVNTUAAB")) {
req->partno += 1001;
req->ntlm = TRUE; 
logmsg("Received NTLM type-1, sending back data %ld", req->partno);
}
else if((req->partno >= 1000) &&
strstr(req->reqbuf, "Authorization: Basic")) {
req->partno += 1;
logmsg("Received Basic request, sending back data %ld", req->partno);
}
if(strstr(req->reqbuf, "Connection: close"))
req->open = FALSE; 
if(req->open &&
req->prot_version >= 11 &&
end &&
req->reqbuf + req->offset > end + strlen(end_of_headers) &&
!req->cl &&
(!strncmp(req->reqbuf, "GET", strlen("GET")) ||
!strncmp(req->reqbuf, "HEAD", strlen("HEAD")))) {
req->checkindex = (end - req->reqbuf) + strlen(end_of_headers);
}
if(req->auth_req && !req->auth) {
logmsg("Return early due to auth requested by none provided");
return 1; 
}
if(req->upgrade && strstr(req->reqbuf, "Upgrade:")) {
logmsg("Found Upgrade: in request and allows it");
req->upgrade_request = TRUE;
}
if(req->cl > 0) {
if(req->cl <= req->offset - (end - req->reqbuf) - strlen(end_of_headers))
return 1; 
else
return 0; 
}
return 1; 
}
static void storerequest(const char *reqbuf, size_t totalsize)
{
int res;
int error = 0;
size_t written;
size_t writeleft;
FILE *dump;
const char *dumpfile = is_proxy?REQUEST_PROXY_DUMP:REQUEST_DUMP;
if(reqbuf == NULL)
return;
if(totalsize == 0)
return;
do {
dump = fopen(dumpfile, "ab");
} while((dump == NULL) && ((error = errno) == EINTR));
if(dump == NULL) {
logmsg("[2] Error opening file %s error: %d %s",
dumpfile, error, strerror(error));
logmsg("Failed to write request input ");
return;
}
writeleft = totalsize;
do {
written = fwrite(&reqbuf[totalsize-writeleft],
1, writeleft, dump);
if(got_exit_signal)
goto storerequest_cleanup;
if(written > 0)
writeleft -= written;
} while((writeleft > 0) && ((error = errno) == EINTR));
if(writeleft == 0)
logmsg("Wrote request (%zu bytes) input to %s", totalsize, dumpfile);
else if(writeleft > 0) {
logmsg("Error writing file %s error: %d %s",
dumpfile, error, strerror(error));
logmsg("Wrote only (%zu bytes) of (%zu bytes) request input to %s",
totalsize-writeleft, totalsize, dumpfile);
}
storerequest_cleanup:
do {
res = fclose(dump);
} while(res && ((error = errno) == EINTR));
if(res)
logmsg("Error closing file %s error: %d %s",
dumpfile, error, strerror(error));
}
static void init_httprequest(struct httprequest *req)
{
req->checkindex = 0;
req->offset = 0;
req->testno = DOCNUMBER_NOTHING;
req->partno = 0;
req->connect_request = FALSE;
req->open = TRUE;
req->auth_req = FALSE;
req->auth = FALSE;
req->cl = 0;
req->digest = FALSE;
req->ntlm = FALSE;
req->skip = 0;
req->skipall = FALSE;
req->noexpect = FALSE;
req->writedelay = 0;
req->rcmd = RCMD_NORMALREQ;
req->prot_version = 0;
req->callcount = 0;
req->connect_port = 0;
req->done_processing = 0;
req->upgrade = 0;
req->upgrade_request = 0;
}
static int get_request(curl_socket_t sock, struct httprequest *req)
{
int fail = 0;
char *reqbuf = req->reqbuf;
ssize_t got = 0;
int overflow = 0;
if(req->offset >= REQBUFSIZ-1) {
overflow = 1;
}
else {
if(req->skip)
got = sread(sock, reqbuf + req->offset, req->cl);
else
got = sread(sock, reqbuf + req->offset, REQBUFSIZ-1 - req->offset);
if(got_exit_signal)
return -1;
if(got == 0) {
logmsg("Connection closed by client");
fail = 1;
}
else if(got < 0) {
int error = SOCKERRNO;
if(EAGAIN == error || EWOULDBLOCK == error) {
return 0;
}
logmsg("recv() returned error: (%d) %s", error, strerror(error));
fail = 1;
}
if(fail) {
reqbuf[req->offset] = '\0';
storerequest(reqbuf, req->offset);
return -1;
}
logmsg("Read %zd bytes", got);
req->offset += (size_t)got;
reqbuf[req->offset] = '\0';
req->done_processing = ProcessRequest(req);
if(got_exit_signal)
return -1;
}
if(overflow || (req->offset == REQBUFSIZ-1 && got > 0)) {
logmsg("Request would overflow buffer, closing connection");
reqbuf[REQBUFSIZ-1] = '\0';
fail = 1;
}
else if(req->offset > REQBUFSIZ-1) {
logmsg("Request buffer overflow, closing connection");
reqbuf[REQBUFSIZ-1] = '\0';
fail = 1;
}
else
reqbuf[req->offset] = '\0';
if(fail || req->done_processing)
storerequest(reqbuf, req->offset);
if(got_exit_signal)
return -1;
return fail ? -1 : 1;
}
static int send_doc(curl_socket_t sock, struct httprequest *req)
{
ssize_t written;
size_t count;
const char *buffer;
char *ptr = NULL;
FILE *stream;
char *cmd = NULL;
size_t cmdsize = 0;
FILE *dump;
bool persistent = TRUE;
bool sendfailure = FALSE;
size_t responsesize;
int error = 0;
int res;
const char *responsedump = is_proxy?RESPONSE_PROXY_DUMP:RESPONSE_DUMP;
static char weare[256];
switch(req->rcmd) {
default:
case RCMD_NORMALREQ:
break; 
case RCMD_STREAM:
#define STREAMTHIS "a string to stream 01234567890\n"
count = strlen(STREAMTHIS);
for(;;) {
written = swrite(sock, STREAMTHIS, count);
if(got_exit_signal)
return -1;
if(written != (ssize_t)count) {
logmsg("Stopped streaming");
break;
}
}
return -1;
case RCMD_IDLE:
return 0;
}
req->open = FALSE;
if(req->testno < 0) {
size_t msglen;
char msgbuf[64];
switch(req->testno) {
case DOCNUMBER_QUIT:
logmsg("Replying to QUIT");
buffer = docquit;
break;
case DOCNUMBER_WERULEZ:
logmsg("Identifying ourselves as friends");
msnprintf(msgbuf, sizeof(msgbuf), "WE ROOLZ: %ld\r\n", (long)getpid());
msglen = strlen(msgbuf);
if(use_gopher)
msnprintf(weare, sizeof(weare), "%s", msgbuf);
else
msnprintf(weare, sizeof(weare),
"HTTP/1.1 200 OK\r\nContent-Length: %zu\r\n\r\n%s",
msglen, msgbuf);
buffer = weare;
break;
case DOCNUMBER_404:
default:
logmsg("Replying to with a 404");
buffer = doc404;
break;
}
count = strlen(buffer);
}
else {
char partbuf[80];
char *filename = test2file(req->testno);
const char *section = req->connect_request?"connect":"data";
if(req->partno)
msnprintf(partbuf, sizeof(partbuf), "%s%ld", section, req->partno);
else
msnprintf(partbuf, sizeof(partbuf), "%s", section);
logmsg("Send response test%ld section <%s>", req->testno, partbuf);
stream = fopen(filename, "rb");
if(!stream) {
error = errno;
logmsg("fopen() failed with error: %d %s", error, strerror(error));
logmsg(" [3] Error opening file: %s", filename);
return 0;
}
else {
error = getpart(&ptr, &count, "reply", partbuf, stream);
fclose(stream);
if(error) {
logmsg("getpart() failed with error: %d", error);
return 0;
}
buffer = ptr;
}
if(got_exit_signal) {
free(ptr);
return -1;
}
stream = fopen(filename, "rb");
if(!stream) {
error = errno;
logmsg("fopen() failed with error: %d %s", error, strerror(error));
logmsg(" [4] Error opening file: %s", filename);
free(ptr);
return 0;
}
else {
error = getpart(&cmd, &cmdsize, "reply", "postcmd", stream);
fclose(stream);
if(error) {
logmsg("getpart() failed with error: %d", error);
free(ptr);
return 0;
}
}
}
if(got_exit_signal) {
free(ptr);
free(cmd);
return -1;
}
if(strstr(buffer, "swsclose") || !count || req->close) {
persistent = FALSE;
logmsg("connection close instruction \"swsclose\" found in response");
}
if(strstr(buffer, "swsbounce")) {
prevbounce = TRUE;
logmsg("enable \"swsbounce\" in the next request");
}
else
prevbounce = FALSE;
dump = fopen(responsedump, "ab");
if(!dump) {
error = errno;
logmsg("fopen() failed with error: %d %s", error, strerror(error));
logmsg(" [5] Error opening file: %s", responsedump);
free(ptr);
free(cmd);
return -1;
}
responsesize = count;
do {
size_t num = count;
if(num > 20)
num = 20;
retry:
written = swrite(sock, buffer, num);
if(written < 0) {
if((EWOULDBLOCK == SOCKERRNO) || (EAGAIN == SOCKERRNO)) {
wait_ms(10);
goto retry;
}
sendfailure = TRUE;
break;
}
fwrite(buffer, 1, (size_t)written, dump);
count -= written;
buffer += written;
if(req->writedelay) {
int quarters = req->writedelay * 4;
logmsg("Pausing %d seconds", req->writedelay);
while((quarters > 0) && !got_exit_signal) {
quarters--;
wait_ms(250);
}
}
} while((count > 0) && !got_exit_signal);
do {
res = fclose(dump);
} while(res && ((error = errno) == EINTR));
if(res)
logmsg("Error closing file %s error: %d %s",
responsedump, error, strerror(error));
if(got_exit_signal) {
free(ptr);
free(cmd);
return -1;
}
if(sendfailure) {
logmsg("Sending response failed. Only (%zu bytes) of (%zu bytes) "
"were sent",
responsesize-count, responsesize);
prevtestno = req->testno;
prevpartno = req->partno;
free(ptr);
free(cmd);
return -1;
}
logmsg("Response sent (%zu bytes) and written to %s",
responsesize, responsedump);
free(ptr);
if(cmdsize > 0) {
char command[32];
int quarters;
int num;
ptr = cmd;
do {
if(2 == sscanf(ptr, "%31s %d", command, &num)) {
if(!strcmp("wait", command)) {
logmsg("Told to sleep for %d seconds", num);
quarters = num * 4;
while((quarters > 0) && !got_exit_signal) {
quarters--;
res = wait_ms(250);
if(res) {
error = errno;
logmsg("wait_ms() failed with error: (%d) %s",
error, strerror(error));
break;
}
}
if(!quarters)
logmsg("Continuing after sleeping %d seconds", num);
}
else
logmsg("Unknown command in reply command section");
}
ptr = strchr(ptr, '\n');
if(ptr)
ptr++;
else
ptr = NULL;
} while(ptr && *ptr);
}
free(cmd);
req->open = use_gopher?FALSE:persistent;
prevtestno = req->testno;
prevpartno = req->partno;
return 0;
}
static curl_socket_t connect_to(const char *ipaddr, unsigned short port)
{
srvr_sockaddr_union_t serveraddr;
curl_socket_t serverfd;
int error;
int rc = 0;
const char *op_br = "";
const char *cl_br = "";
#if defined(ENABLE_IPV6)
if(socket_domain == AF_INET6) {
op_br = "[";
cl_br = "]";
}
#endif
if(!ipaddr)
return CURL_SOCKET_BAD;
logmsg("about to connect to %s%s%s:%hu",
op_br, ipaddr, cl_br, port);
serverfd = socket(socket_domain, SOCK_STREAM, 0);
if(CURL_SOCKET_BAD == serverfd) {
error = SOCKERRNO;
logmsg("Error creating socket for server connection: (%d) %s",
error, strerror(error));
return CURL_SOCKET_BAD;
}
#if defined(TCP_NODELAY)
if(socket_domain_is_ip()) {
curl_socklen_t flag = 1;
if(0 != setsockopt(serverfd, IPPROTO_TCP, TCP_NODELAY,
(void *)&flag, sizeof(flag)))
logmsg("====> TCP_NODELAY for server connection failed");
}
#endif
switch(socket_domain) {
case AF_INET:
memset(&serveraddr.sa4, 0, sizeof(serveraddr.sa4));
serveraddr.sa4.sin_family = AF_INET;
serveraddr.sa4.sin_port = htons(port);
if(Curl_inet_pton(AF_INET, ipaddr, &serveraddr.sa4.sin_addr) < 1) {
logmsg("Error inet_pton failed AF_INET conversion of '%s'", ipaddr);
sclose(serverfd);
return CURL_SOCKET_BAD;
}
rc = connect(serverfd, &serveraddr.sa, sizeof(serveraddr.sa4));
break;
#if defined(ENABLE_IPV6)
case AF_INET6:
memset(&serveraddr.sa6, 0, sizeof(serveraddr.sa6));
serveraddr.sa6.sin6_family = AF_INET6;
serveraddr.sa6.sin6_port = htons(port);
if(Curl_inet_pton(AF_INET6, ipaddr, &serveraddr.sa6.sin6_addr) < 1) {
logmsg("Error inet_pton failed AF_INET6 conversion of '%s'", ipaddr);
sclose(serverfd);
return CURL_SOCKET_BAD;
}
rc = connect(serverfd, &serveraddr.sa, sizeof(serveraddr.sa6));
break;
#endif 
#if defined(USE_UNIX_SOCKETS)
case AF_UNIX:
logmsg("Proxying through Unix socket is not (yet?) supported.");
return CURL_SOCKET_BAD;
#endif 
}
if(got_exit_signal) {
sclose(serverfd);
return CURL_SOCKET_BAD;
}
if(rc) {
error = SOCKERRNO;
logmsg("Error connecting to server port %hu: (%d) %s",
port, error, strerror(error));
sclose(serverfd);
return CURL_SOCKET_BAD;
}
logmsg("connected fine to %s%s%s:%hu, now tunnel",
op_br, ipaddr, cl_br, port);
return serverfd;
}
#define data_or_ctrl(x) ((x)?"DATA":"CTRL")
#define CTRL 0
#define DATA 1
static void http_connect(curl_socket_t *infdp,
curl_socket_t rootfd,
const char *ipaddr,
unsigned short ipport)
{
curl_socket_t serverfd[2] = {CURL_SOCKET_BAD, CURL_SOCKET_BAD};
curl_socket_t clientfd[2] = {CURL_SOCKET_BAD, CURL_SOCKET_BAD};
ssize_t toc[2] = {0, 0}; 
ssize_t tos[2] = {0, 0}; 
char readclient[2][256];
char readserver[2][256];
bool poll_client_rd[2] = { TRUE, TRUE };
bool poll_server_rd[2] = { TRUE, TRUE };
bool poll_client_wr[2] = { TRUE, TRUE };
bool poll_server_wr[2] = { TRUE, TRUE };
bool primary = FALSE;
bool secondary = FALSE;
int max_tunnel_idx; 
int loop;
int i;
int timeout_count = 0;
clientfd[CTRL] = *infdp;
for(loop = 2; (loop > 0) && !got_exit_signal; loop--)
wait_ms(250);
if(got_exit_signal)
goto http_connect_cleanup;
serverfd[CTRL] = connect_to(ipaddr, ipport);
if(serverfd[CTRL] == CURL_SOCKET_BAD)
goto http_connect_cleanup;
max_tunnel_idx = CTRL;
primary = TRUE;
while(!got_exit_signal) {
fd_set input;
fd_set output;
struct timeval timeout = {1, 0}; 
ssize_t rc;
curl_socket_t maxfd = (curl_socket_t)-1;
FD_ZERO(&input);
FD_ZERO(&output);
if((clientfd[DATA] == CURL_SOCKET_BAD) &&
(serverfd[DATA] == CURL_SOCKET_BAD) &&
poll_client_rd[CTRL] && poll_client_wr[CTRL] &&
poll_server_rd[CTRL] && poll_server_wr[CTRL]) {
FD_SET(rootfd, &input);
maxfd = rootfd;
}
for(i = 0; i <= max_tunnel_idx; i++) {
if(clientfd[i] != CURL_SOCKET_BAD) {
if(poll_client_rd[i]) {
FD_SET(clientfd[i], &input);
if(clientfd[i] > maxfd)
maxfd = clientfd[i];
}
if(poll_client_wr[i] && toc[i]) {
FD_SET(clientfd[i], &output);
if(clientfd[i] > maxfd)
maxfd = clientfd[i];
}
}
if(serverfd[i] != CURL_SOCKET_BAD) {
if(poll_server_rd[i]) {
FD_SET(serverfd[i], &input);
if(serverfd[i] > maxfd)
maxfd = serverfd[i];
}
if(poll_server_wr[i] && tos[i]) {
FD_SET(serverfd[i], &output);
if(serverfd[i] > maxfd)
maxfd = serverfd[i];
}
}
}
if(got_exit_signal)
break;
do {
rc = select((int)maxfd + 1, &input, &output, NULL, &timeout);
} while(rc < 0 && errno == EINTR && !got_exit_signal);
if(got_exit_signal)
break;
if(rc > 0) {
bool tcp_fin_wr = FALSE;
timeout_count = 0;
if((clientfd[DATA] == CURL_SOCKET_BAD) &&
(serverfd[DATA] == CURL_SOCKET_BAD) && FD_ISSET(rootfd, &input)) {
curl_socket_t datafd = accept(rootfd, NULL, NULL);
if(datafd != CURL_SOCKET_BAD) {
struct httprequest req2;
int err = 0;
memset(&req2, 0, sizeof(req2));
logmsg("====> Client connect DATA");
#if defined(TCP_NODELAY)
if(socket_domain_is_ip()) {
curl_socklen_t flag = 1;
if(0 != setsockopt(datafd, IPPROTO_TCP, TCP_NODELAY,
(void *)&flag, sizeof(flag)))
logmsg("====> TCP_NODELAY for client DATA connection failed");
}
#endif
init_httprequest(&req2);
while(!req2.done_processing) {
err = get_request(datafd, &req2);
if(err < 0) {
break;
}
}
if(err >= 0) {
err = send_doc(datafd, &req2);
if(!err && req2.connect_request) {
for(loop = 2; (loop > 0) && !got_exit_signal; loop--)
wait_ms(250);
if(!got_exit_signal) {
serverfd[DATA] = connect_to(ipaddr, req2.connect_port);
if(serverfd[DATA] != CURL_SOCKET_BAD) {
poll_client_rd[DATA] = TRUE;
poll_client_wr[DATA] = TRUE;
poll_server_rd[DATA] = TRUE;
poll_server_wr[DATA] = TRUE;
max_tunnel_idx = DATA;
secondary = TRUE;
toc[DATA] = 0;
tos[DATA] = 0;
clientfd[DATA] = datafd;
datafd = CURL_SOCKET_BAD;
}
}
}
}
if(datafd != CURL_SOCKET_BAD) {
shutdown(datafd, SHUT_RDWR);
sclose(datafd);
}
}
if(got_exit_signal)
break;
}
for(i = 0; i <= max_tunnel_idx; i++) {
size_t len;
if(clientfd[i] != CURL_SOCKET_BAD) {
len = sizeof(readclient[i]) - tos[i];
if(len && FD_ISSET(clientfd[i], &input)) {
rc = sread(clientfd[i], &readclient[i][tos[i]], len);
if(rc <= 0) {
logmsg("[%s] got %zd, STOP READING client", data_or_ctrl(i), rc);
shutdown(clientfd[i], SHUT_RD);
poll_client_rd[i] = FALSE;
}
else {
logmsg("[%s] READ %zd bytes from client", data_or_ctrl(i), rc);
logmsg("[%s] READ \"%s\"", data_or_ctrl(i),
data_to_hex(&readclient[i][tos[i]], rc));
tos[i] += rc;
}
}
}
if(serverfd[i] != CURL_SOCKET_BAD) {
len = sizeof(readserver[i])-toc[i];
if(len && FD_ISSET(serverfd[i], &input)) {
rc = sread(serverfd[i], &readserver[i][toc[i]], len);
if(rc <= 0) {
logmsg("[%s] got %zd, STOP READING server", data_or_ctrl(i), rc);
shutdown(serverfd[i], SHUT_RD);
poll_server_rd[i] = FALSE;
}
else {
logmsg("[%s] READ %zd bytes from server", data_or_ctrl(i), rc);
logmsg("[%s] READ \"%s\"", data_or_ctrl(i),
data_to_hex(&readserver[i][toc[i]], rc));
toc[i] += rc;
}
}
}
if(clientfd[i] != CURL_SOCKET_BAD) {
if(toc[i] && FD_ISSET(clientfd[i], &output)) {
rc = swrite(clientfd[i], readserver[i], toc[i]);
if(rc <= 0) {
logmsg("[%s] got %zd, STOP WRITING client", data_or_ctrl(i), rc);
shutdown(clientfd[i], SHUT_WR);
poll_client_wr[i] = FALSE;
tcp_fin_wr = TRUE;
}
else {
logmsg("[%s] SENT %zd bytes to client", data_or_ctrl(i), rc);
logmsg("[%s] SENT \"%s\"", data_or_ctrl(i),
data_to_hex(readserver[i], rc));
if(toc[i] - rc)
memmove(&readserver[i][0], &readserver[i][rc], toc[i]-rc);
toc[i] -= rc;
}
}
}
if(serverfd[i] != CURL_SOCKET_BAD) {
if(tos[i] && FD_ISSET(serverfd[i], &output)) {
rc = swrite(serverfd[i], readclient[i], tos[i]);
if(rc <= 0) {
logmsg("[%s] got %zd, STOP WRITING server", data_or_ctrl(i), rc);
shutdown(serverfd[i], SHUT_WR);
poll_server_wr[i] = FALSE;
tcp_fin_wr = TRUE;
}
else {
logmsg("[%s] SENT %zd bytes to server", data_or_ctrl(i), rc);
logmsg("[%s] SENT \"%s\"", data_or_ctrl(i),
data_to_hex(readclient[i], rc));
if(tos[i] - rc)
memmove(&readclient[i][0], &readclient[i][rc], tos[i]-rc);
tos[i] -= rc;
}
}
}
}
if(got_exit_signal)
break;
for(i = 0; i <= max_tunnel_idx; i++) {
for(loop = 2; loop > 0; loop--) {
if(clientfd[i] != CURL_SOCKET_BAD) {
if(poll_client_rd[i] && !poll_server_wr[i]) {
logmsg("[%s] DISABLED READING client", data_or_ctrl(i));
shutdown(clientfd[i], SHUT_RD);
poll_client_rd[i] = FALSE;
}
if(poll_client_wr[i] && !poll_server_rd[i] && !toc[i]) {
logmsg("[%s] DISABLED WRITING client", data_or_ctrl(i));
shutdown(clientfd[i], SHUT_WR);
poll_client_wr[i] = FALSE;
tcp_fin_wr = TRUE;
}
}
if(serverfd[i] != CURL_SOCKET_BAD) {
if(poll_server_rd[i] && !poll_client_wr[i]) {
logmsg("[%s] DISABLED READING server", data_or_ctrl(i));
shutdown(serverfd[i], SHUT_RD);
poll_server_rd[i] = FALSE;
}
if(poll_server_wr[i] && !poll_client_rd[i] && !tos[i]) {
logmsg("[%s] DISABLED WRITING server", data_or_ctrl(i));
shutdown(serverfd[i], SHUT_WR);
poll_server_wr[i] = FALSE;
tcp_fin_wr = TRUE;
}
}
}
}
if(tcp_fin_wr)
wait_ms(250);
for(i = 0; i <= max_tunnel_idx; i++) {
for(loop = 2; loop > 0; loop--) {
if(clientfd[i] != CURL_SOCKET_BAD) {
if(!poll_client_wr[i] && !poll_client_rd[i]) {
logmsg("[%s] CLOSING client socket", data_or_ctrl(i));
sclose(clientfd[i]);
clientfd[i] = CURL_SOCKET_BAD;
if(serverfd[i] == CURL_SOCKET_BAD) {
logmsg("[%s] ENDING", data_or_ctrl(i));
if(i == DATA)
secondary = FALSE;
else
primary = FALSE;
}
}
}
if(serverfd[i] != CURL_SOCKET_BAD) {
if(!poll_server_wr[i] && !poll_server_rd[i]) {
logmsg("[%s] CLOSING server socket", data_or_ctrl(i));
sclose(serverfd[i]);
serverfd[i] = CURL_SOCKET_BAD;
if(clientfd[i] == CURL_SOCKET_BAD) {
logmsg("[%s] ENDING", data_or_ctrl(i));
if(i == DATA)
secondary = FALSE;
else
primary = FALSE;
}
}
}
}
}
max_tunnel_idx = secondary ? DATA : CTRL;
if(!primary)
break;
} 
else {
timeout_count++;
if(timeout_count > 5) {
logmsg("CONNECT proxy timeout after %d idle seconds!", timeout_count);
break;
}
}
}
http_connect_cleanup:
for(i = DATA; i >= CTRL; i--) {
if(serverfd[i] != CURL_SOCKET_BAD) {
logmsg("[%s] CLOSING server socket (cleanup)", data_or_ctrl(i));
shutdown(serverfd[i], SHUT_RDWR);
sclose(serverfd[i]);
}
if(clientfd[i] != CURL_SOCKET_BAD) {
logmsg("[%s] CLOSING client socket (cleanup)", data_or_ctrl(i));
shutdown(clientfd[i], SHUT_RDWR);
sclose(clientfd[i]);
}
if((serverfd[i] != CURL_SOCKET_BAD) ||
(clientfd[i] != CURL_SOCKET_BAD)) {
logmsg("[%s] ABORTING", data_or_ctrl(i));
}
}
*infdp = CURL_SOCKET_BAD;
}
static void http2(struct httprequest *req)
{
(void)req;
logmsg("switched to http2");
}
static curl_socket_t accept_connection(curl_socket_t sock)
{
curl_socket_t msgsock = CURL_SOCKET_BAD;
int error;
int flag = 1;
if(MAX_SOCKETS == num_sockets) {
logmsg("Too many open sockets!");
return CURL_SOCKET_BAD;
}
msgsock = accept(sock, NULL, NULL);
if(got_exit_signal) {
if(CURL_SOCKET_BAD != msgsock)
sclose(msgsock);
return CURL_SOCKET_BAD;
}
if(CURL_SOCKET_BAD == msgsock) {
error = SOCKERRNO;
if(EAGAIN == error || EWOULDBLOCK == error) {
return 0;
}
logmsg("MAJOR ERROR: accept() failed with error: (%d) %s",
error, strerror(error));
return CURL_SOCKET_BAD;
}
if(0 != curlx_nonblock(msgsock, TRUE)) {
error = SOCKERRNO;
logmsg("curlx_nonblock failed with error: (%d) %s",
error, strerror(error));
sclose(msgsock);
return CURL_SOCKET_BAD;
}
if(0 != setsockopt(msgsock, SOL_SOCKET, SO_KEEPALIVE,
(void *)&flag, sizeof(flag))) {
error = SOCKERRNO;
logmsg("setsockopt(SO_KEEPALIVE) failed with error: (%d) %s",
error, strerror(error));
sclose(msgsock);
return CURL_SOCKET_BAD;
}
if(!serverlogslocked)
set_advisor_read_lock(SERVERLOGS_LOCK);
serverlogslocked += 1;
logmsg("====> Client connect");
all_sockets[num_sockets] = msgsock;
num_sockets += 1;
#if defined(TCP_NODELAY)
if(socket_domain_is_ip()) {
if(0 != setsockopt(msgsock, IPPROTO_TCP, TCP_NODELAY,
(void *)&flag, sizeof(flag)))
logmsg("====> TCP_NODELAY failed");
}
#endif
return msgsock;
}
static int service_connection(curl_socket_t msgsock, struct httprequest *req,
curl_socket_t listensock,
const char *connecthost)
{
if(got_exit_signal)
return -1;
while(!req->done_processing) {
int rc = get_request(msgsock, req);
if(rc <= 0) {
return rc;
}
}
if(prevbounce) {
if((req->testno == prevtestno) &&
(req->partno == prevpartno)) {
req->partno++;
logmsg("BOUNCE part number to %ld", req->partno);
}
else {
prevbounce = FALSE;
prevtestno = -1;
prevpartno = -1;
}
}
send_doc(msgsock, req);
if(got_exit_signal)
return -1;
if(req->testno < 0) {
logmsg("special request received, no persistency");
return -1;
}
if(!req->open) {
logmsg("instructed to close connection after server-reply");
return -1;
}
if(req->connect_request) {
if(!is_proxy) {
logmsg("received CONNECT but isn't running as proxy!");
return 1;
}
else {
http_connect(&msgsock, listensock, connecthost, req->connect_port);
return -1;
}
}
if(req->upgrade_request) {
http2(req);
return -1;
}
if(req->open) {
logmsg("=> persistent connection request ended, awaits new request\n");
return 1;
}
return -1;
}
int main(int argc, char *argv[])
{
srvr_sockaddr_union_t me;
curl_socket_t sock = CURL_SOCKET_BAD;
int wrotepidfile = 0;
int flag;
unsigned short port = DEFAULT_PORT;
#if defined(USE_UNIX_SOCKETS)
const char *unix_socket = NULL;
bool unlink_socket = false;
#endif
const char *pidname = ".http.pid";
struct httprequest req;
int rc = 0;
int error;
int arg = 1;
long pid;
const char *connecthost = "127.0.0.1";
const char *socket_type = "IPv4";
char port_str[11];
const char *location_str = port_str;
size_t socket_idx;
memset(&req, 0, sizeof(req));
while(argc>arg) {
if(!strcmp("--version", argv[arg])) {
puts("sws IPv4"
#if defined(ENABLE_IPV6)
"/IPv6"
#endif
#if defined(USE_UNIX_SOCKETS)
"/unix"
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
else if(!strcmp("--gopher", argv[arg])) {
arg++;
use_gopher = TRUE;
end_of_headers = "\r\n"; 
}
else if(!strcmp("--ipv4", argv[arg])) {
socket_type = "IPv4";
socket_domain = AF_INET;
location_str = port_str;
arg++;
}
else if(!strcmp("--ipv6", argv[arg])) {
#if defined(ENABLE_IPV6)
socket_type = "IPv6";
socket_domain = AF_INET6;
location_str = port_str;
#endif
arg++;
}
else if(!strcmp("--unix-socket", argv[arg])) {
arg++;
if(argc>arg) {
#if defined(USE_UNIX_SOCKETS)
unix_socket = argv[arg];
if(strlen(unix_socket) >= sizeof(me.sau.sun_path)) {
fprintf(stderr, "sws: socket path must be shorter than %zu chars\n",
sizeof(me.sau.sun_path));
return 0;
}
socket_type = "unix";
socket_domain = AF_UNIX;
location_str = unix_socket;
#endif
arg++;
}
}
else if(!strcmp("--port", argv[arg])) {
arg++;
if(argc>arg) {
char *endptr;
unsigned long ulnum = strtoul(argv[arg], &endptr, 10);
if((endptr != argv[arg] + strlen(argv[arg])) ||
(ulnum < 1025UL) || (ulnum > 65535UL)) {
fprintf(stderr, "sws: invalid --port argument (%s)\n",
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
else if(!strcmp("--connect", argv[arg])) {
arg++;
if(argc>arg) {
connecthost = argv[arg];
arg++;
is_proxy = TRUE;
logmsg("Run as proxy, CONNECT to host %s", connecthost);
}
}
else {
puts("Usage: sws [option]\n"
" --version\n"
" --logfile [file]\n"
" --pidfile [file]\n"
" --ipv4\n"
" --ipv6\n"
" --unix-socket [file]\n"
" --port [port]\n"
" --srcdir [path]\n"
" --connect [ip4-addr]\n"
" --gopher");
return 0;
}
}
msnprintf(port_str, sizeof(port_str), "port %hu", port);
#if defined(WIN32)
win32_init();
atexit(win32_cleanup);
#endif
install_signal_handlers();
pid = (long)getpid();
sock = socket(socket_domain, SOCK_STREAM, 0);
all_sockets[0] = sock;
num_sockets = 1;
if(CURL_SOCKET_BAD == sock) {
error = SOCKERRNO;
logmsg("Error creating socket: (%d) %s",
error, strerror(error));
goto sws_cleanup;
}
flag = 1;
if(0 != setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
(void *)&flag, sizeof(flag))) {
error = SOCKERRNO;
logmsg("setsockopt(SO_REUSEADDR) failed with error: (%d) %s",
error, strerror(error));
goto sws_cleanup;
}
if(0 != curlx_nonblock(sock, TRUE)) {
error = SOCKERRNO;
logmsg("curlx_nonblock failed with error: (%d) %s",
error, strerror(error));
goto sws_cleanup;
}
switch(socket_domain) {
case AF_INET:
memset(&me.sa4, 0, sizeof(me.sa4));
me.sa4.sin_family = AF_INET;
me.sa4.sin_addr.s_addr = INADDR_ANY;
me.sa4.sin_port = htons(port);
rc = bind(sock, &me.sa, sizeof(me.sa4));
break;
#if defined(ENABLE_IPV6)
case AF_INET6:
memset(&me.sa6, 0, sizeof(me.sa6));
me.sa6.sin6_family = AF_INET6;
me.sa6.sin6_addr = in6addr_any;
me.sa6.sin6_port = htons(port);
rc = bind(sock, &me.sa, sizeof(me.sa6));
break;
#endif 
#if defined(USE_UNIX_SOCKETS)
case AF_UNIX:
memset(&me.sau, 0, sizeof(me.sau));
me.sau.sun_family = AF_UNIX;
strncpy(me.sau.sun_path, unix_socket, sizeof(me.sau.sun_path) - 1);
rc = bind(sock, &me.sa, sizeof(me.sau));
if(0 != rc && errno == EADDRINUSE) {
struct stat statbuf;
int unixfd = socket(AF_UNIX, SOCK_STREAM, 0);
if(CURL_SOCKET_BAD == unixfd) {
error = SOCKERRNO;
logmsg("Error binding socket, failed to create socket at %s: (%d) %s",
unix_socket, error, strerror(error));
goto sws_cleanup;
}
rc = connect(unixfd, &me.sa, sizeof(me.sau));
error = errno;
close(unixfd);
if(ECONNREFUSED != error) {
logmsg("Error binding socket, failed to connect to %s: (%d) %s",
unix_socket, error, strerror(error));
goto sws_cleanup;
}
rc = lstat(unix_socket, &statbuf);
if(0 != rc) {
logmsg("Error binding socket, failed to stat %s: (%d) %s",
unix_socket, errno, strerror(errno));
goto sws_cleanup;
}
if((statbuf.st_mode & S_IFSOCK) != S_IFSOCK) {
logmsg("Error binding socket, failed to stat %s: (%d) %s",
unix_socket, error, strerror(error));
goto sws_cleanup;
}
rc = unlink(unix_socket);
if(0 != rc) {
logmsg("Error binding socket, failed to unlink %s: (%d) %s",
unix_socket, errno, strerror(errno));
goto sws_cleanup;
}
rc = bind(sock, &me.sa, sizeof(me.sau));
}
break;
#endif 
}
if(0 != rc) {
error = SOCKERRNO;
logmsg("Error binding socket on %s: (%d) %s",
location_str, error, strerror(error));
goto sws_cleanup;
}
logmsg("Running %s %s version on %s",
use_gopher?"GOPHER":"HTTP", socket_type, location_str);
rc = listen(sock, 5);
if(0 != rc) {
error = SOCKERRNO;
logmsg("listen() failed with error: (%d) %s",
error, strerror(error));
goto sws_cleanup;
}
#if defined(USE_UNIX_SOCKETS)
unlink_socket = true;
#endif
wrotepidfile = write_pidfile(pidname);
if(!wrotepidfile)
goto sws_cleanup;
init_httprequest(&req);
for(;;) {
fd_set input;
fd_set output;
struct timeval timeout = {0, 250000L}; 
curl_socket_t maxfd = (curl_socket_t)-1;
for(socket_idx = num_sockets - 1; socket_idx >= 1; --socket_idx) {
if(CURL_SOCKET_BAD == all_sockets[socket_idx]) {
char *dst = (char *) (all_sockets + socket_idx);
char *src = (char *) (all_sockets + socket_idx + 1);
char *end = (char *) (all_sockets + num_sockets);
memmove(dst, src, end - src);
num_sockets -= 1;
}
}
if(got_exit_signal)
goto sws_cleanup;
FD_ZERO(&input);
FD_ZERO(&output);
for(socket_idx = 0; socket_idx < num_sockets; ++socket_idx) {
FD_SET(all_sockets[socket_idx], &input);
if(all_sockets[socket_idx] > maxfd)
maxfd = all_sockets[socket_idx];
}
if(got_exit_signal)
goto sws_cleanup;
do {
rc = select((int)maxfd + 1, &input, &output, NULL, &timeout);
} while(rc < 0 && errno == EINTR && !got_exit_signal);
if(got_exit_signal)
goto sws_cleanup;
if(rc < 0) {
error = SOCKERRNO;
logmsg("select() failed with error: (%d) %s",
error, strerror(error));
goto sws_cleanup;
}
if(rc == 0) {
continue;
}
if(FD_ISSET(all_sockets[0], &input)) {
curl_socket_t msgsock;
do {
msgsock = accept_connection(sock);
logmsg("accept_connection %d returned %d", sock, msgsock);
if(CURL_SOCKET_BAD == msgsock)
goto sws_cleanup;
} while(msgsock > 0);
}
for(socket_idx = 1; socket_idx < num_sockets; ++socket_idx) {
if(FD_ISSET(all_sockets[socket_idx], &input)) {
if(got_exit_signal)
goto sws_cleanup;
do {
rc = service_connection(all_sockets[socket_idx], &req, sock,
connecthost);
if(got_exit_signal)
goto sws_cleanup;
if(rc < 0) {
logmsg("====> Client disconnect %d", req.connmon);
if(req.connmon) {
const char *keepopen = "[DISCONNECT]\n";
storerequest(keepopen, strlen(keepopen));
}
if(!req.open)
wait_ms(50);
if(all_sockets[socket_idx] != CURL_SOCKET_BAD) {
sclose(all_sockets[socket_idx]);
all_sockets[socket_idx] = CURL_SOCKET_BAD;
}
serverlogslocked -= 1;
if(!serverlogslocked)
clear_advisor_read_lock(SERVERLOGS_LOCK);
if(req.testno == DOCNUMBER_QUIT)
goto sws_cleanup;
}
if(rc != 0)
init_httprequest(&req);
} while(rc > 0);
}
}
if(got_exit_signal)
goto sws_cleanup;
}
sws_cleanup:
for(socket_idx = 1; socket_idx < num_sockets; ++socket_idx)
if((all_sockets[socket_idx] != sock) &&
(all_sockets[socket_idx] != CURL_SOCKET_BAD))
sclose(all_sockets[socket_idx]);
if(sock != CURL_SOCKET_BAD)
sclose(sock);
#if defined(USE_UNIX_SOCKETS)
if(unlink_socket && socket_domain == AF_UNIX) {
rc = unlink(unix_socket);
logmsg("unlink(%s) = %d (%s)", unix_socket, rc, strerror(rc));
}
#endif
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
logmsg("========> %s sws (%s pid: %ld) exits with signal (%d)",
socket_type, location_str, pid, exit_signal);
raise(exit_signal);
}
logmsg("========> sws quits");
return 0;
}
