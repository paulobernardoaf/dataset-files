




















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
#include "util.h"
#include "server_sockaddr.h"


#include "memdebug.h"

#if defined(USE_WINSOCK)
#undef EINTR
#define EINTR 4 
#undef ERANGE
#define ERANGE 34 
#endif

#if defined(ENABLE_IPV6)
static bool use_ipv6 = FALSE;
#endif
static const char *ipv_inuse = "IPv4";
static int serverlogslocked = 0;

#define REQBUFSIZ 150000
#define REQBUFSIZ_TXT "149999"

static long prevtestno = -1; 
static long prevpartno = -1; 
static bool prevbounce = FALSE; 



#define RCMD_NORMALREQ 0 
#define RCMD_IDLE 1 
#define RCMD_STREAM 2 

typedef enum {
RPROT_NONE = 0,
RPROT_RTSP = 1,
RPROT_HTTP = 2
} reqprot_t;

#define SET_RTP_PKT_CHN(p,c) ((p)[1] = (unsigned char)((c) & 0xFF))

#define SET_RTP_PKT_LEN(p,l) (((p)[2] = (unsigned char)(((l) >> 8) & 0xFF)), ((p)[3] = (unsigned char)((l) & 0xFF)))


struct httprequest {
char reqbuf[REQBUFSIZ]; 
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
int pipe; 

int skip; 



int rcmd; 
reqprot_t protocol; 
int prot_version; 
bool pipelining; 
char *rtp_buffer;
size_t rtp_buffersize;
};

static int ProcessRequest(struct httprequest *req);
static void storerequest(char *reqbuf, size_t totalsize);

#define DEFAULT_PORT 8999

#if !defined(DEFAULT_LOGFILE)
#define DEFAULT_LOGFILE "log/rtspd.log"
#endif

const char *serverlogfile = DEFAULT_LOGFILE;

#define RTSPDVERSION "curl test suite RTSP server/0.1"

#define REQUEST_DUMP "log/server.input"
#define RESPONSE_DUMP "log/server.response"


#define MAXDOCNAMELEN 140000
#define MAXDOCNAMELEN_TXT "139999"

#define REQUEST_KEYWORD_SIZE 256
#define REQUEST_KEYWORD_SIZE_TXT "255"

#define CMD_AUTH_REQUIRED "auth_required"



#define CMD_IDLE "idle"


#define CMD_STREAM "stream"

#define END_OF_HEADERS "\r\n\r\n"

enum {
DOCNUMBER_NOTHING = -7,
DOCNUMBER_QUIT = -6,
DOCNUMBER_BADCONNECT = -5,
DOCNUMBER_INTERNAL = -4,
DOCNUMBER_CONNECT = -3,
DOCNUMBER_WERULEZ = -2,
DOCNUMBER_404 = -1
};



static const char *docquit =
"HTTP/1.1 200 Goodbye" END_OF_HEADERS;


static const char *docconnect =
"HTTP/1.1 200 Mighty fine indeed" END_OF_HEADERS;


static const char *docbadconnect =
"HTTP/1.1 501 Forbidden you fool" END_OF_HEADERS;


static const char *doc404_HTTP = "HTTP/1.1 404 Not Found\r\n"
"Server: " RTSPDVERSION "\r\n"
"Connection: close\r\n"
"Content-Type: text/html"
END_OF_HEADERS
"<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n"
"<HTML><HEAD>\n"
"<TITLE>404 Not Found</TITLE>\n"
"</HEAD><BODY>\n"
"<H1>Not Found</H1>\n"
"The requested URL was not found on this server.\n"
"<P><HR><ADDRESS>" RTSPDVERSION "</ADDRESS>\n" "</BODY></HTML>\n";


static const char *doc404_RTSP = "RTSP/1.0 404 Not Found\r\n"
"Server: " RTSPDVERSION
END_OF_HEADERS;


#define RTP_DATA_SIZE 12
static const char *RTP_DATA = "$_1234\n\0asdf";



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

static int ProcessRequest(struct httprequest *req)
{
char *line = &req->reqbuf[req->checkindex];
bool chunked = FALSE;
static char request[REQUEST_KEYWORD_SIZE];
static char doc[MAXDOCNAMELEN];
static char prot_str[5];
int prot_major, prot_minor;
char *end = strstr(line, END_OF_HEADERS);

logmsg("ProcessRequest() called with testno %ld and line [%s]",
req->testno, line);



if((req->testno == DOCNUMBER_NOTHING) &&
sscanf(line,
"%" REQUEST_KEYWORD_SIZE_TXT"s %" MAXDOCNAMELEN_TXT "s %4s/%d.%d",
request,
doc,
prot_str,
&prot_major,
&prot_minor) == 5) {
char *ptr;
char logbuf[256];

if(!strcmp(prot_str, "HTTP")) {
req->protocol = RPROT_HTTP;
}
else if(!strcmp(prot_str, "RTSP")) {
req->protocol = RPROT_RTSP;
}
else {
req->protocol = RPROT_NONE;
logmsg("got unknown protocol %s", prot_str);
return 1;
}

req->prot_version = prot_major*10 + prot_minor;


ptr = strrchr(doc, '/');


if(ptr) {
FILE *stream;
char *filename;

if((strlen(doc) + strlen(request)) < 200)
msnprintf(logbuf, sizeof(logbuf), "Got request: %s %s %s/%d.%d",
request, doc, prot_str, prot_major, prot_minor);
else
msnprintf(logbuf, sizeof(logbuf), "Got a *HUGE* request %s/%d.%d",
prot_str, prot_major, prot_minor);
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

msnprintf(logbuf, sizeof(logbuf), "Requested test number %ld part %ld",
req->testno, req->partno);
logmsg("%s", logbuf);

filename = test2file(req->testno);

stream = fopen(filename, "rb");
if(!stream) {
int error = errno;
logmsg("fopen() failed with error: %d %s", error, strerror(error));
logmsg("Error opening file: %s", filename);
logmsg("Couldn't open test file %ld", req->testno);
req->open = FALSE; 
return 1; 
}
else {
char *cmd = NULL;
size_t cmdsize = 0;
int num = 0;

int rtp_channel = 0;
int rtp_size = 0;
int rtp_partno = -1;
char *rtp_scratch = NULL;


int error = getpart(&cmd, &cmdsize, "reply", "servercmd", stream);
fclose(stream);
if(error) {
logmsg("getpart() failed with error: %d", error);
req->open = FALSE; 
return 1; 
}
ptr = cmd;

if(cmdsize) {
logmsg("Found a reply-servercmd section!");
do {
if(!strncmp(CMD_AUTH_REQUIRED, ptr, strlen(CMD_AUTH_REQUIRED))) {
logmsg("instructed to require authorization header");
req->auth_req = TRUE;
}
else if(!strncmp(CMD_IDLE, ptr, strlen(CMD_IDLE))) {
logmsg("instructed to idle");
req->rcmd = RCMD_IDLE;
req->open = TRUE;
}
else if(!strncmp(CMD_STREAM, ptr, strlen(CMD_STREAM))) {
logmsg("instructed to stream");
req->rcmd = RCMD_STREAM;
}
else if(1 == sscanf(ptr, "pipe: %d", &num)) {
logmsg("instructed to allow a pipe size of %d", num);
if(num < 0)
logmsg("negative pipe size ignored");
else if(num > 0)
req->pipe = num-1; 

}
else if(1 == sscanf(ptr, "skip: %d", &num)) {
logmsg("instructed to skip this number of bytes %d", num);
req->skip = num;
}
else if(3 == sscanf(ptr, "rtp: part %d channel %d size %d",
&rtp_partno, &rtp_channel, &rtp_size)) {

if(rtp_partno == req->partno) {
int i = 0;
logmsg("RTP: part %d channel %d size %d",
rtp_partno, rtp_channel, rtp_size);



rtp_scratch = malloc(rtp_size + 4 + RTP_DATA_SIZE);


rtp_scratch[0] = '$';


SET_RTP_PKT_CHN(rtp_scratch, rtp_channel);


SET_RTP_PKT_LEN(rtp_scratch, rtp_size);


for(i = 0; i < rtp_size; i += RTP_DATA_SIZE) {
memcpy(rtp_scratch + 4 + i, RTP_DATA, RTP_DATA_SIZE);
}

if(req->rtp_buffer == NULL) {
req->rtp_buffer = rtp_scratch;
req->rtp_buffersize = rtp_size + 4;
}
else {
req->rtp_buffer = realloc(req->rtp_buffer,
req->rtp_buffersize +
rtp_size + 4);
memcpy(req->rtp_buffer + req->rtp_buffersize, rtp_scratch,
rtp_size + 4);
req->rtp_buffersize += rtp_size + 4;
free(rtp_scratch);
}
logmsg("rtp_buffersize is %zu, rtp_size is %d.",
req->rtp_buffersize, rtp_size);
}
}
else {
logmsg("funny instruction found: %s", ptr);
}

ptr = strchr(ptr, '\n');
if(ptr)
ptr++;
else
ptr = NULL;
} while(ptr && *ptr);
logmsg("Done parsing server commands");
}
free(cmd);
}
}
else {
if(sscanf(req->reqbuf, "CONNECT %" MAXDOCNAMELEN_TXT "s HTTP/%d.%d",
doc, &prot_major, &prot_minor) == 3) {
msnprintf(logbuf, sizeof(logbuf),
"Received a CONNECT %s HTTP/%d.%d request",
doc, prot_major, prot_minor);
logmsg("%s", logbuf);

if(req->prot_version == 10)
req->open = FALSE; 

if(!strncmp(doc, "bad", 3))

req->testno = DOCNUMBER_BADCONNECT;
else if(!strncmp(doc, "test", 4)) {


char *portp = strchr(doc, ':');
if(portp && (*(portp + 1) != '\0') && ISDIGIT(*(portp + 1)))
req->testno = strtol(portp + 1, NULL, 10);
else
req->testno = DOCNUMBER_CONNECT;
}
else
req->testno = DOCNUMBER_CONNECT;
}
else {
logmsg("Did not find test number in PATH");
req->testno = DOCNUMBER_404;
}
}
}

if(!end) {

logmsg("ProcessRequest returned without a complete request");
return 0; 
}
logmsg("ProcessRequest found a complete request");

if(req->pipe)


req->checkindex += (end - line) + strlen(END_OF_HEADERS);












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
req->cl = clen - req->skip;

logmsg("Found Content-Length: %lu in the request", clen);
if(req->skip)
logmsg("... but will abort after %zu bytes", req->cl);
break;
}
else if(strncasecompare("Transfer-Encoding: chunked", line,
strlen("Transfer-Encoding: chunked"))) {

chunked = TRUE;
}

if(chunked) {
if(strstr(req->reqbuf, "\r\n0\r\n\r\n"))

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

if(!req->digest && strstr(req->reqbuf, "Authorization: Digest")) {



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

if(!req->pipe &&
req->open &&
req->prot_version >= 11 &&
end &&
req->reqbuf + req->offset > end + strlen(END_OF_HEADERS) &&
(!strncmp(req->reqbuf, "GET", strlen("GET")) ||
!strncmp(req->reqbuf, "HEAD", strlen("HEAD")))) {


req->checkindex = (end - req->reqbuf) + strlen(END_OF_HEADERS);
req->pipelining = TRUE;
}

while(req->pipe) {
if(got_exit_signal)
return 1; 

line = &req->reqbuf[req->checkindex];
end = strstr(line, END_OF_HEADERS);
if(!end)
break;
req->checkindex += (end - line) + strlen(END_OF_HEADERS);
req->pipe--;
}





if(req->auth_req && !req->auth)
return 1; 

if(req->cl > 0) {
if(req->cl <= req->offset - (end - req->reqbuf) - strlen(END_OF_HEADERS))
return 1; 
else
return 0; 
}

return 1; 
}


static void storerequest(char *reqbuf, size_t totalsize)
{
int res;
int error = 0;
size_t written;
size_t writeleft;
FILE *dump;

if(reqbuf == NULL)
return;
if(totalsize == 0)
return;

do {
dump = fopen(REQUEST_DUMP, "ab");
} while((dump == NULL) && ((error = errno) == EINTR));
if(dump == NULL) {
logmsg("Error opening file %s error: %d %s",
REQUEST_DUMP, error, strerror(error));
logmsg("Failed to write request input to " REQUEST_DUMP);
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
logmsg("Wrote request (%zu bytes) input to " REQUEST_DUMP, totalsize);
else if(writeleft > 0) {
logmsg("Error writing file %s error: %d %s",
REQUEST_DUMP, error, strerror(error));
logmsg("Wrote only (%zu bytes) of (%zu bytes) request input to %s",
totalsize-writeleft, totalsize, REQUEST_DUMP);
}

storerequest_cleanup:

do {
res = fclose(dump);
} while(res && ((error = errno) == EINTR));
if(res)
logmsg("Error closing file %s error: %d %s",
REQUEST_DUMP, error, strerror(error));
}


static int get_request(curl_socket_t sock, struct httprequest *req)
{
int error;
int fail = 0;
int done_processing = 0;
char *reqbuf = req->reqbuf;
ssize_t got = 0;

char *pipereq = NULL;
size_t pipereq_length = 0;

if(req->pipelining) {
pipereq = reqbuf + req->checkindex;
pipereq_length = req->offset - req->checkindex;
}



req->checkindex = 0;
req->offset = 0;
req->testno = DOCNUMBER_NOTHING;
req->partno = 0;
req->open = TRUE;
req->auth_req = FALSE;
req->auth = FALSE;
req->cl = 0;
req->digest = FALSE;
req->ntlm = FALSE;
req->pipe = 0;
req->skip = 0;
req->rcmd = RCMD_NORMALREQ;
req->protocol = RPROT_NONE;
req->prot_version = 0;
req->pipelining = FALSE;
req->rtp_buffer = NULL;
req->rtp_buffersize = 0;



while(!done_processing && (req->offset < REQBUFSIZ-1)) {
if(pipereq_length && pipereq) {
memmove(reqbuf, pipereq, pipereq_length);
got = curlx_uztosz(pipereq_length);
pipereq_length = 0;
}
else {
if(req->skip)



got = sread(sock, reqbuf + req->offset, req->cl);
else
got = sread(sock, reqbuf + req->offset, REQBUFSIZ-1 - req->offset);
}
if(got_exit_signal)
return 1;
if(got == 0) {
logmsg("Connection closed by client");
fail = 1;
}
else if(got < 0) {
error = SOCKERRNO;
logmsg("recv() returned error: (%d) %s", error, strerror(error));
fail = 1;
}
if(fail) {

reqbuf[req->offset] = '\0';
storerequest(reqbuf, req->offset);
return 1;
}

logmsg("Read %zd bytes", got);

req->offset += (size_t)got;
reqbuf[req->offset] = '\0';

done_processing = ProcessRequest(req);
if(got_exit_signal)
return 1;
if(done_processing && req->pipe) {
logmsg("Waiting for another piped request");
done_processing = 0;
req->pipe--;
}
}

if((req->offset == REQBUFSIZ-1) && (got > 0)) {
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


storerequest(reqbuf, req->pipelining ? req->checkindex : req->offset);
if(got_exit_signal)
return 1;

return fail; 
}


static int send_doc(curl_socket_t sock, struct httprequest *req)
{
ssize_t written;
size_t count;
const char *buffer;
char *ptr = NULL;
char *cmd = NULL;
size_t cmdsize = 0;
FILE *dump;
bool persistent = TRUE;
bool sendfailure = FALSE;
size_t responsesize;
int error = 0;
int res;

static char weare[256];

logmsg("Send response number %ld part %ld", req->testno, req->partno);

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
msnprintf(msgbuf, sizeof(msgbuf), "RTSP_SERVER WE ROOLZ: %ld\r\n",
(long)getpid());
msglen = strlen(msgbuf);
msnprintf(weare, sizeof(weare),
"HTTP/1.1 200 OK\r\nContent-Length: %zu\r\n\r\n%s",
msglen, msgbuf);
buffer = weare;
break;
case DOCNUMBER_INTERNAL:
logmsg("Bailing out due to internal error");
return -1;
case DOCNUMBER_CONNECT:
logmsg("Replying to CONNECT");
buffer = docconnect;
break;
case DOCNUMBER_BADCONNECT:
logmsg("Replying to a bad CONNECT");
buffer = docbadconnect;
break;
case DOCNUMBER_404:
default:
logmsg("Replying to with a 404");
if(req->protocol == RPROT_HTTP) {
buffer = doc404_HTTP;
}
else {
buffer = doc404_RTSP;
}
break;
}

count = strlen(buffer);
}
else {
char *filename = test2file(req->testno);
char partbuf[80]="data";
FILE *stream;
if(0 != req->partno)
msnprintf(partbuf, sizeof(partbuf), "data%ld", req->partno);

stream = fopen(filename, "rb");
if(!stream) {
error = errno;
logmsg("fopen() failed with error: %d %s", error, strerror(error));
logmsg("Error opening file: %s", filename);
logmsg("Couldn't open test file");
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
logmsg("Error opening file: %s", filename);
logmsg("Couldn't open test file");
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




if(strstr(buffer, "swsclose") || !count) {
persistent = FALSE;
logmsg("connection close instruction \"swsclose\" found in response");
}
if(strstr(buffer, "swsbounce")) {
prevbounce = TRUE;
logmsg("enable \"swsbounce\" in the next request");
}
else
prevbounce = FALSE;

dump = fopen(RESPONSE_DUMP, "ab");
if(!dump) {
error = errno;
logmsg("fopen() failed with error: %d %s", error, strerror(error));
logmsg("Error opening file: %s", RESPONSE_DUMP);
logmsg("couldn't create logfile: " RESPONSE_DUMP);
free(ptr);
free(cmd);
return -1;
}

responsesize = count;
do {



size_t num = count;
if(num > 200)
num = 200;
written = swrite(sock, buffer, num);
if(written < 0) {
sendfailure = TRUE;
break;
}
else {
logmsg("Sent off %zd bytes", written);
}

fwrite(buffer, 1, (size_t)written, dump);
if(got_exit_signal)
break;

count -= written;
buffer += written;
} while(count>0);


if(req->rtp_buffer) {
logmsg("About to write %zu RTP bytes", req->rtp_buffersize);
count = req->rtp_buffersize;
do {
size_t num = count;
if(num > 200)
num = 200;
written = swrite(sock, req->rtp_buffer + (req->rtp_buffersize - count),
num);
if(written < 0) {
sendfailure = TRUE;
break;
}
count -= written;
} while(count > 0);

free(req->rtp_buffer);
req->rtp_buffersize = 0;
}

do {
res = fclose(dump);
} while(res && ((error = errno) == EINTR));
if(res)
logmsg("Error closing file %s error: %d %s",
RESPONSE_DUMP, error, strerror(error));

if(got_exit_signal) {
free(ptr);
free(cmd);
return -1;
}

if(sendfailure) {
logmsg("Sending response failed. Only (%zu bytes) of "
"(%zu bytes) were sent",
responsesize-count, responsesize);
free(ptr);
free(cmd);
return -1;
}

logmsg("Response sent (%zu bytes) and written to " RESPONSE_DUMP,
responsesize);
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
while(quarters > 0) {
quarters--;
res = wait_ms(250);
if(got_exit_signal)
break;
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
req->open = persistent;

prevtestno = req->testno;
prevpartno = req->partno;

return 0;
}


int main(int argc, char *argv[])
{
srvr_sockaddr_union_t me;
curl_socket_t sock = CURL_SOCKET_BAD;
curl_socket_t msgsock = CURL_SOCKET_BAD;
int wrotepidfile = 0;
int flag;
unsigned short port = DEFAULT_PORT;
const char *pidname = ".rtsp.pid";
struct httprequest req;
int rc;
int error;
int arg = 1;
long pid;

memset(&req, 0, sizeof(req));

while(argc>arg) {
if(!strcmp("--version", argv[arg])) {
printf("rtspd IPv4%s"
"\n"
,
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
fprintf(stderr, "rtspd: invalid --port argument (%s)\n",
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
puts("Usage: rtspd [option]\n"
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
sock = socket(AF_INET, SOCK_STREAM, 0);
#if defined(ENABLE_IPV6)
else
sock = socket(AF_INET6, SOCK_STREAM, 0);
#endif

if(CURL_SOCKET_BAD == sock) {
error = SOCKERRNO;
logmsg("Error creating socket: (%d) %s",
error, strerror(error));
goto server_cleanup;
}

flag = 1;
if(0 != setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
(void *)&flag, sizeof(flag))) {
error = SOCKERRNO;
logmsg("setsockopt(SO_REUSEADDR) failed with error: (%d) %s",
error, strerror(error));
goto server_cleanup;
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
goto server_cleanup;
}

logmsg("Running %s version on port %d", ipv_inuse, (int)port);


rc = listen(sock, 5);
if(0 != rc) {
error = SOCKERRNO;
logmsg("listen() failed with error: (%d) %s",
error, strerror(error));
goto server_cleanup;
}






wrotepidfile = write_pidfile(pidname);
if(!wrotepidfile)
goto server_cleanup;

for(;;) {
msgsock = accept(sock, NULL, NULL);

if(got_exit_signal)
break;
if(CURL_SOCKET_BAD == msgsock) {
error = SOCKERRNO;
logmsg("MAJOR ERROR: accept() failed with error: (%d) %s",
error, strerror(error));
break;
}







set_advisor_read_lock(SERVERLOGS_LOCK);
serverlogslocked = 1;

logmsg("====> Client connect");

#if defined(TCP_NODELAY)




flag = 1;
if(setsockopt(msgsock, IPPROTO_TCP, TCP_NODELAY,
(void *)&flag, sizeof(flag)) == -1) {
logmsg("====> TCP_NODELAY failed");
}
#endif





req.pipelining = FALSE;

do {
if(got_exit_signal)
break;

if(get_request(msgsock, &req))

break;

if(prevbounce) {

if((req.testno == prevtestno) &&
(req.partno == prevpartno)) {
req.partno++;
logmsg("BOUNCE part number to %ld", req.partno);
}
else {
prevbounce = FALSE;
prevtestno = -1;
prevpartno = -1;
}
}

send_doc(msgsock, &req);
if(got_exit_signal)
break;

if((req.testno < 0) && (req.testno != DOCNUMBER_CONNECT)) {
logmsg("special request received, no persistency");
break;
}
if(!req.open) {
logmsg("instructed to close connection after server-reply");
break;
}

if(req.open)
logmsg("=> persistent connection request ended, awaits new request");

} while(req.open || (req.testno == DOCNUMBER_CONNECT));

if(got_exit_signal)
break;

logmsg("====> Client disconnect");
sclose(msgsock);
msgsock = CURL_SOCKET_BAD;

if(serverlogslocked) {
serverlogslocked = 0;
clear_advisor_read_lock(SERVERLOGS_LOCK);
}

if(req.testno == DOCNUMBER_QUIT)
break;
}

server_cleanup:

if((msgsock != sock) && (msgsock != CURL_SOCKET_BAD))
sclose(msgsock);

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
logmsg("========> %s rtspd (port: %d pid: %ld) exits with signal (%d)",
ipv_inuse, (int)port, pid, exit_signal);





raise(exit_signal);
}

logmsg("========> rtspd quits");
return 0;
}
