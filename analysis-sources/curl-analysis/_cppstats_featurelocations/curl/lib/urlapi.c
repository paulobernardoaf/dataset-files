





















#include "curl_setup.h"

#include "urldata.h"
#include "urlapi-int.h"
#include "strcase.h"
#include "dotdot.h"
#include "url.h"
#include "escape.h"
#include "curl_ctype.h"
#include "inet_pton.h"


#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"


#define STARTS_WITH_DRIVE_PREFIX(str) ((('a' <= str[0] && str[0] <= 'z') || ('A' <= str[0] && str[0] <= 'Z')) && (str[1] == ':'))






#define STARTS_WITH_URL_DRIVE_PREFIX(str) ((('a' <= (str)[0] && (str)[0] <= 'z') || ('A' <= (str)[0] && (str)[0] <= 'Z')) && ((str)[1] == ':' || (str)[1] == '|') && ((str)[2] == '/' || (str)[2] == '\\' || (str)[2] == 0))






struct Curl_URL {
char *scheme;
char *user;
char *password;
char *options; 
char *host;
char *zoneid; 
char *port;
char *path;
char *query;
char *fragment;

char *scratch; 
char *temppath; 
long portnum; 
};

#define DEFAULT_SCHEME "https"

static void free_urlhandle(struct Curl_URL *u)
{
free(u->scheme);
free(u->user);
free(u->password);
free(u->options);
free(u->host);
free(u->zoneid);
free(u->port);
free(u->path);
free(u->query);
free(u->fragment);
free(u->scratch);
free(u->temppath);
}



static void mv_urlhandle(struct Curl_URL *from,
struct Curl_URL *to)
{
free_urlhandle(to);
*to = *from;
free(from);
}





static const char *find_host_sep(const char *url)
{
const char *sep;
const char *query;


sep = strstr(url, "//");
if(!sep)
sep = url;
else
sep += 2;

query = strchr(sep, '?');
sep = strchr(sep, '/');

if(!sep)
sep = url + strlen(url);

if(!query)
query = url + strlen(url);

return sep < query ? sep : query;
}






static bool urlchar_needs_escaping(int c)
{
return !(ISCNTRL(c) || ISSPACE(c) || ISGRAPH(c));
}







static size_t strlen_url(const char *url, bool relative)
{
const unsigned char *ptr;
size_t newlen = 0;
bool left = TRUE; 
const unsigned char *host_sep = (const unsigned char *) url;

if(!relative)
host_sep = (const unsigned char *) find_host_sep(url);

for(ptr = (unsigned char *)url; *ptr; ptr++) {

if(ptr < host_sep) {
++newlen;
continue;
}

switch(*ptr) {
case '?':
left = FALSE;

default:
if(urlchar_needs_escaping(*ptr))
newlen += 2;
newlen++;
break;
case ' ':
if(left)
newlen += 3;
else
newlen++;
break;
}
}
return newlen;
}






static void strcpy_url(char *output, const char *url, bool relative)
{

bool left = TRUE;
const unsigned char *iptr;
char *optr = output;
const unsigned char *host_sep = (const unsigned char *) url;

if(!relative)
host_sep = (const unsigned char *) find_host_sep(url);

for(iptr = (unsigned char *)url; 
*iptr; 
iptr++) {

if(iptr < host_sep) {
*optr++ = *iptr;
continue;
}

switch(*iptr) {
case '?':
left = FALSE;

default:
if(urlchar_needs_escaping(*iptr)) {
msnprintf(optr, 4, "%%%02x", *iptr);
optr += 3;
}
else
*optr++=*iptr;
break;
case ' ':
if(left) {
*optr++='%'; 
*optr++='2'; 
*optr++='0'; 
}
else
*optr++='+'; 
break;
}
}
*optr = 0; 

}






bool Curl_is_absolute_url(const char *url, char *buf, size_t buflen)
{
size_t i;
#if defined(WIN32)
if(STARTS_WITH_DRIVE_PREFIX(url))
return FALSE;
#endif
for(i = 0; i < buflen && url[i]; ++i) {
char s = url[i];
if((s == ':') && (url[i + 1] == '/')) {
if(buf)
buf[i] = 0;
return TRUE;
}



else if(ISALNUM(s) || (s == '+') || (s == '-') || (s == '.') ) {
if(buf)
buf[i] = (char)TOLOWER(s);
}
else
break;
}
return FALSE;
}







static char *concat_url(const char *base, const char *relurl)
{





char *newest;
char *protsep;
char *pathsep;
size_t newlen;
bool host_changed = FALSE;

const char *useurl = relurl;
size_t urllen;



char *url_clone = strdup(base);

if(!url_clone)
return NULL; 


protsep = strstr(url_clone, "//");
if(!protsep)
protsep = url_clone;
else
protsep += 2; 

if('/' != relurl[0]) {
int level = 0;



pathsep = strchr(protsep, '?');
if(pathsep)
*pathsep = 0;





if(useurl[0] != '?') {
pathsep = strrchr(protsep, '/');
if(pathsep)
*pathsep = 0;
}



pathsep = strchr(protsep, '/');
if(pathsep)
protsep = pathsep + 1;
else
protsep = NULL;




if((useurl[0] == '.') && (useurl[1] == '/'))
useurl += 2; 

while((useurl[0] == '.') &&
(useurl[1] == '.') &&
(useurl[2] == '/')) {
level++;
useurl += 3; 
}

if(protsep) {
while(level--) {

pathsep = strrchr(protsep, '/');
if(pathsep)
*pathsep = 0;
else {
*protsep = 0;
break;
}
}
}
}
else {


if(relurl[1] == '/') {


*protsep = 0;
useurl = &relurl[2]; 

host_changed = TRUE;
}
else {


pathsep = strchr(protsep, '/');
if(pathsep) {



char *sep = strchr(protsep, '?');
if(sep && (sep < pathsep))
pathsep = sep;
*pathsep = 0;
}
else {




pathsep = strchr(protsep, '?');
if(pathsep)
*pathsep = 0;
}
}
}






newlen = strlen_url(useurl, !host_changed);

urllen = strlen(url_clone);

newest = malloc(urllen + 1 + 
newlen + 1 );

if(!newest) {
free(url_clone); 
return NULL;
}


memcpy(newest, url_clone, urllen);


if(('/' == useurl[0]) || (protsep && !*protsep) || ('?' == useurl[0]))
;
else
newest[urllen++]='/';


strcpy_url(&newest[urllen], useurl, !host_changed);

free(url_clone);

return newest;
}








static CURLUcode parse_hostname_login(struct Curl_URL *u,
char **hostname,
unsigned int flags)
{
CURLUcode result = CURLUE_OK;
CURLcode ccode;
char *userp = NULL;
char *passwdp = NULL;
char *optionsp = NULL;
const struct Curl_handler *h = NULL;








char *ptr = strchr(*hostname, '@');
char *login = *hostname;

if(!ptr)
goto out;




*hostname = ++ptr;


if(u->scheme)
h = Curl_builtin_scheme(u->scheme);



ccode = Curl_parse_login_details(login, ptr - login - 1,
&userp, &passwdp,
(h && (h->flags & PROTOPT_URLOPTIONS)) ?
&optionsp:NULL);
if(ccode) {
result = CURLUE_MALFORMED_INPUT;
goto out;
}

if(userp) {
if(flags & CURLU_DISALLOW_USER) {

result = CURLUE_USER_NOT_ALLOWED;
goto out;
}

u->user = userp;
}

if(passwdp)
u->password = passwdp;

if(optionsp)
u->options = optionsp;

return CURLUE_OK;
out:

free(userp);
free(passwdp);
free(optionsp);

return result;
}

UNITTEST CURLUcode Curl_parse_port(struct Curl_URL *u, char *hostname)
{
char *portptr = NULL;
char endbracket;
int len;





if(1 == sscanf(hostname, "[%*45[0123456789abcdefABCDEF:.]%c%n",
&endbracket, &len)) {
if(']' == endbracket)
portptr = &hostname[len];
else if('%' == endbracket) {
int zonelen = len;
if(1 == sscanf(hostname + zonelen, "%*[^]]%c%n", &endbracket, &len)) {
if(']' != endbracket)
return CURLUE_MALFORMED_INPUT;
portptr = &hostname[--zonelen + len + 1];
}
else
return CURLUE_MALFORMED_INPUT;
}
else
return CURLUE_MALFORMED_INPUT;


if(portptr && *portptr) {
if(*portptr != ':')
return CURLUE_MALFORMED_INPUT;
}
else
portptr = NULL;
}
else
portptr = strchr(hostname, ':');

if(portptr) {
char *rest;
long port;
char portbuf[7];




if(!portptr[1]) {
*portptr = '\0';
return CURLUE_OK;
}

if(!ISDIGIT(portptr[1]))
return CURLUE_BAD_PORT_NUMBER;

port = strtol(portptr + 1, &rest, 10); 

if((port <= 0) || (port > 0xffff))


return CURLUE_BAD_PORT_NUMBER;

if(rest[0])
return CURLUE_BAD_PORT_NUMBER;

*portptr++ = '\0'; 
*rest = 0;

msnprintf(portbuf, sizeof(portbuf), "%ld", port);
u->portnum = port;
u->port = strdup(portbuf);
if(!u->port)
return CURLUE_OUT_OF_MEMORY;
}

return CURLUE_OK;
}


static CURLUcode junkscan(const char *part)
{
if(part) {
static const char badbytes[]={
0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
0x7f,
0x00 
};
size_t n = strlen(part);
size_t nfine = strcspn(part, badbytes);
if(nfine != n)


return CURLUE_MALFORMED_INPUT;
}
return CURLUE_OK;
}

static CURLUcode hostname_check(struct Curl_URL *u, char *hostname)
{
size_t len;
size_t hlen = strlen(hostname);

if(hostname[0] == '[') {
#if defined(ENABLE_IPV6)
char dest[16]; 
#endif
const char *l = "0123456789abcdefABCDEF:.";
if(hlen < 5) 
return CURLUE_MALFORMED_INPUT;
hostname++;
hlen -= 2;

if(hostname[hlen] != ']')
return CURLUE_MALFORMED_INPUT;


len = strspn(hostname, l);
if(hlen != len) {
hlen = len;
if(hostname[len] == '%') {

char zoneid[16];
int i = 0;
char *h = &hostname[len + 1];

if(!strncmp(h, "25", 2) && h[2] && (h[2] != ']'))
h += 2;
while(*h && (*h != ']') && (i < 15))
zoneid[i++] = *h++;
if(!i || (']' != *h))
return CURLUE_MALFORMED_INPUT;
zoneid[i] = 0;
u->zoneid = strdup(zoneid);
if(!u->zoneid)
return CURLUE_OUT_OF_MEMORY;
hostname[len] = ']'; 
hostname[len + 1] = 0; 
}
else
return CURLUE_MALFORMED_INPUT;

}
#if defined(ENABLE_IPV6)
hostname[hlen] = 0; 
if(1 != Curl_inet_pton(AF_INET6, hostname, dest))
return CURLUE_MALFORMED_INPUT;
hostname[hlen] = ']'; 
#endif
}
else {

len = strcspn(hostname, " ");
if(hlen != len)

return CURLUE_MALFORMED_INPUT;
}
if(!hostname[0])
return CURLUE_NO_HOST;
return CURLUE_OK;
}

#define HOSTNAME_END(x) (((x) == '/') || ((x) == '?') || ((x) == '#'))

static CURLUcode seturl(const char *url, CURLU *u, unsigned int flags)
{
char *path;
bool path_alloced = FALSE;
char *hostname;
char *query = NULL;
char *fragment = NULL;
CURLUcode result;
bool url_has_scheme = FALSE;
char schemebuf[MAX_SCHEME_LEN + 1];
const char *schemep = NULL;
size_t schemelen = 0;
size_t urllen;

if(!url)
return CURLUE_MALFORMED_INPUT;





urllen = strlen(url);
if(urllen > CURL_MAX_INPUT_LENGTH)

return CURLUE_MALFORMED_INPUT;

path = u->scratch = malloc(urllen * 2 + 2);
if(!path)
return CURLUE_OUT_OF_MEMORY;

hostname = &path[urllen + 1];
hostname[0] = 0;

if(Curl_is_absolute_url(url, schemebuf, sizeof(schemebuf))) {
url_has_scheme = TRUE;
schemelen = strlen(schemebuf);
}


if(url_has_scheme && strcasecompare(schemebuf, "file")) {

strcpy(path, &url[5]);

hostname = NULL; 
u->scheme = strdup("file");
if(!u->scheme)
return CURLUE_OUT_OF_MEMORY;







if(path[0] == '/' && path[1] == '/') {

char *ptr = &path[2];

















if(ptr[0] != '/' && !STARTS_WITH_URL_DRIVE_PREFIX(ptr)) {


if(!checkprefix("localhost/", ptr) &&
!checkprefix("127.0.0.1/", ptr)) {


return CURLUE_MALFORMED_INPUT;
}
ptr += 9; 
}

path = ptr;
}

#if !defined(MSDOS) && !defined(WIN32) && !defined(__CYGWIN__)


if(('/' == path[0] && STARTS_WITH_URL_DRIVE_PREFIX(&path[1])) ||
STARTS_WITH_URL_DRIVE_PREFIX(path)) {

return CURLUE_MALFORMED_INPUT;
}
#else

if('/' == path[0] && STARTS_WITH_URL_DRIVE_PREFIX(&path[1])) {

memmove(path, &path[1], strlen(&path[1]) + 1);
}
#endif

}
else {

const char *p;
const char *hostp;
size_t len;
path[0] = 0;

if(url_has_scheme) {
int i = 0;
p = &url[schemelen + 1];
while(p && (*p == '/') && (i < 4)) {
p++;
i++;
}
if((i < 1) || (i>3))

return CURLUE_MALFORMED_INPUT;

schemep = schemebuf;
if(!Curl_builtin_scheme(schemep) &&
!(flags & CURLU_NON_SUPPORT_SCHEME))
return CURLUE_UNSUPPORTED_SCHEME;

if(junkscan(schemep))
return CURLUE_MALFORMED_INPUT;

}
else {


if(!(flags & (CURLU_DEFAULT_SCHEME|CURLU_GUESS_SCHEME)))
return CURLUE_MALFORMED_INPUT;
if(flags & CURLU_DEFAULT_SCHEME)
schemep = DEFAULT_SCHEME;




p = url;
}
hostp = p; 

while(*p && !HOSTNAME_END(*p)) 
p++;

len = p - hostp;
if(len) {
memcpy(hostname, hostp, len);
hostname[len] = 0;
}
else {
if(!(flags & CURLU_NO_AUTHORITY))
return CURLUE_MALFORMED_INPUT;
}

len = strlen(p);
memcpy(path, p, len);
path[len] = 0;

if(schemep) {
u->scheme = strdup(schemep);
if(!u->scheme)
return CURLUE_OUT_OF_MEMORY;
}
}

if(junkscan(path))
return CURLUE_MALFORMED_INPUT;

if((flags & CURLU_URLENCODE) && path[0]) {

char *newp = malloc(strlen(path) * 3);
if(!newp)
return CURLUE_OUT_OF_MEMORY;
path_alloced = TRUE;
strcpy_url(newp, path, TRUE); 
u->temppath = path = newp;
}

fragment = strchr(path, '#');
if(fragment) {
*fragment++ = 0;
if(fragment[0]) {
u->fragment = strdup(fragment);
if(!u->fragment)
return CURLUE_OUT_OF_MEMORY;
}
}

query = strchr(path, '?');
if(query) {
*query++ = 0;

u->query = strdup(query);
if(!u->query)
return CURLUE_OUT_OF_MEMORY;
}

if(!path[0])

path = NULL;
else {
if(!(flags & CURLU_PATH_AS_IS)) {

char *newp = Curl_dedotdotify(path);
if(!newp)
return CURLUE_OUT_OF_MEMORY;

if(strcmp(newp, path)) {

if(path_alloced)
Curl_safefree(u->temppath);
u->temppath = path = newp;
path_alloced = TRUE;
}
else
free(newp);
}

u->path = path_alloced?path:strdup(path);
if(!u->path)
return CURLUE_OUT_OF_MEMORY;
u->temppath = NULL; 
}

if(hostname) {



if(junkscan(hostname))
return CURLUE_MALFORMED_INPUT;

result = parse_hostname_login(u, &hostname, flags);
if(result)
return result;

result = Curl_parse_port(u, hostname);
if(result)
return result;

if(0 == strlen(hostname) && (flags & CURLU_NO_AUTHORITY)) {

}
else {
result = hostname_check(u, hostname);
if(result)
return result;
}

u->host = strdup(hostname);
if(!u->host)
return CURLUE_OUT_OF_MEMORY;

if((flags & CURLU_GUESS_SCHEME) && !schemep) {

if(checkprefix("ftp.", hostname))
schemep = "ftp";
else if(checkprefix("dict.", hostname))
schemep = "dict";
else if(checkprefix("ldap.", hostname))
schemep = "ldap";
else if(checkprefix("imap.", hostname))
schemep = "imap";
else if(checkprefix("smtp.", hostname))
schemep = "smtp";
else if(checkprefix("pop3.", hostname))
schemep = "pop3";
else
schemep = "http";

u->scheme = strdup(schemep);
if(!u->scheme)
return CURLUE_OUT_OF_MEMORY;
}
}

Curl_safefree(u->scratch);
Curl_safefree(u->temppath);

return CURLUE_OK;
}




static CURLUcode parseurl(const char *url, CURLU *u, unsigned int flags)
{
CURLUcode result = seturl(url, u, flags);
if(result) {
free_urlhandle(u);
memset(u, 0, sizeof(struct Curl_URL));
}
return result;
}



CURLU *curl_url(void)
{
return calloc(sizeof(struct Curl_URL), 1);
}

void curl_url_cleanup(CURLU *u)
{
if(u) {
free_urlhandle(u);
free(u);
}
}

#define DUP(dest, src, name) if(src->name) { dest->name = strdup(src->name); if(!dest->name) goto fail; }






CURLU *curl_url_dup(CURLU *in)
{
struct Curl_URL *u = calloc(sizeof(struct Curl_URL), 1);
if(u) {
DUP(u, in, scheme);
DUP(u, in, user);
DUP(u, in, password);
DUP(u, in, options);
DUP(u, in, host);
DUP(u, in, port);
DUP(u, in, path);
DUP(u, in, query);
DUP(u, in, fragment);
u->portnum = in->portnum;
}
return u;
fail:
curl_url_cleanup(u);
return NULL;
}

CURLUcode curl_url_get(CURLU *u, CURLUPart what,
char **part, unsigned int flags)
{
char *ptr;
CURLUcode ifmissing = CURLUE_UNKNOWN_PART;
char portbuf[7];
bool urldecode = (flags & CURLU_URLDECODE)?1:0;
bool plusdecode = FALSE;
(void)flags;
if(!u)
return CURLUE_BAD_HANDLE;
if(!part)
return CURLUE_BAD_PARTPOINTER;
*part = NULL;

switch(what) {
case CURLUPART_SCHEME:
ptr = u->scheme;
ifmissing = CURLUE_NO_SCHEME;
urldecode = FALSE; 
break;
case CURLUPART_USER:
ptr = u->user;
ifmissing = CURLUE_NO_USER;
break;
case CURLUPART_PASSWORD:
ptr = u->password;
ifmissing = CURLUE_NO_PASSWORD;
break;
case CURLUPART_OPTIONS:
ptr = u->options;
ifmissing = CURLUE_NO_OPTIONS;
break;
case CURLUPART_HOST:
ptr = u->host;
ifmissing = CURLUE_NO_HOST;
break;
case CURLUPART_ZONEID:
ptr = u->zoneid;
break;
case CURLUPART_PORT:
ptr = u->port;
ifmissing = CURLUE_NO_PORT;
urldecode = FALSE; 
if(!ptr && (flags & CURLU_DEFAULT_PORT) && u->scheme) {


const struct Curl_handler *h =
Curl_builtin_scheme(u->scheme);
if(h) {
msnprintf(portbuf, sizeof(portbuf), "%ld", h->defport);
ptr = portbuf;
}
}
else if(ptr && u->scheme) {


const struct Curl_handler *h =
Curl_builtin_scheme(u->scheme);
if(h && (h->defport == u->portnum) &&
(flags & CURLU_NO_DEFAULT_PORT))
ptr = NULL;
}
break;
case CURLUPART_PATH:
ptr = u->path;
if(!ptr) {
ptr = u->path = strdup("/");
if(!u->path)
return CURLUE_OUT_OF_MEMORY;
}
break;
case CURLUPART_QUERY:
ptr = u->query;
ifmissing = CURLUE_NO_QUERY;
plusdecode = urldecode;
break;
case CURLUPART_FRAGMENT:
ptr = u->fragment;
ifmissing = CURLUE_NO_FRAGMENT;
break;
case CURLUPART_URL: {
char *url;
char *scheme;
char *options = u->options;
char *port = u->port;
char *allochost = NULL;
if(u->scheme && strcasecompare("file", u->scheme)) {
url = aprintf("file://%s%s%s",
u->path,
u->fragment? "#": "",
u->fragment? u->fragment : "");
}
else if(!u->host)
return CURLUE_NO_HOST;
else {
const struct Curl_handler *h = NULL;
if(u->scheme)
scheme = u->scheme;
else if(flags & CURLU_DEFAULT_SCHEME)
scheme = (char *) DEFAULT_SCHEME;
else
return CURLUE_NO_SCHEME;

h = Curl_builtin_scheme(scheme);
if(!port && (flags & CURLU_DEFAULT_PORT)) {


if(h) {
msnprintf(portbuf, sizeof(portbuf), "%ld", h->defport);
port = portbuf;
}
}
else if(port) {


if(h && (h->defport == u->portnum) &&
(flags & CURLU_NO_DEFAULT_PORT))
port = NULL;
}

if(h && !(h->flags & PROTOPT_URLOPTIONS))
options = NULL;

if((u->host[0] == '[') && u->zoneid) {

size_t hostlen = strlen(u->host);
size_t alen = hostlen + 3 + strlen(u->zoneid) + 1;
allochost = malloc(alen);
if(!allochost)
return CURLUE_OUT_OF_MEMORY;
memcpy(allochost, u->host, hostlen - 1);
msnprintf(&allochost[hostlen - 1], alen - hostlen + 1,
"%%25%s]", u->zoneid);
}

url = aprintf("%s://%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
scheme,
u->user ? u->user : "",
u->password ? ":": "",
u->password ? u->password : "",
options ? ";" : "",
options ? options : "",
(u->user || u->password || options) ? "@": "",
allochost ? allochost : u->host,
port ? ":": "",
port ? port : "",
(u->path && (u->path[0] != '/')) ? "/": "",
u->path ? u->path : "/",
(u->query && u->query[0]) ? "?": "",
(u->query && u->query[0]) ? u->query : "",
u->fragment? "#": "",
u->fragment? u->fragment : "");
free(allochost);
}
if(!url)
return CURLUE_OUT_OF_MEMORY;
*part = url;
return CURLUE_OK;
}
default:
ptr = NULL;
break;
}
if(ptr) {
*part = strdup(ptr);
if(!*part)
return CURLUE_OUT_OF_MEMORY;
if(plusdecode) {

char *plus;
for(plus = *part; *plus; ++plus) {
if(*plus == '+')
*plus = ' ';
}
}
if(urldecode) {
char *decoded;
size_t dlen;
CURLcode res = Curl_urldecode(NULL, *part, 0, &decoded, &dlen, TRUE);
free(*part);
if(res) {
*part = NULL;
return CURLUE_URLDECODE;
}
*part = decoded;
}
return CURLUE_OK;
}
else
return ifmissing;
}

CURLUcode curl_url_set(CURLU *u, CURLUPart what,
const char *part, unsigned int flags)
{
char **storep = NULL;
long port = 0;
bool urlencode = (flags & CURLU_URLENCODE)? 1 : 0;
bool plusencode = FALSE;
bool urlskipslash = FALSE;
bool appendquery = FALSE;
bool equalsencode = FALSE;

if(!u)
return CURLUE_BAD_HANDLE;
if(!part) {

switch(what) {
case CURLUPART_URL:
break;
case CURLUPART_SCHEME:
storep = &u->scheme;
break;
case CURLUPART_USER:
storep = &u->user;
break;
case CURLUPART_PASSWORD:
storep = &u->password;
break;
case CURLUPART_OPTIONS:
storep = &u->options;
break;
case CURLUPART_HOST:
storep = &u->host;
break;
case CURLUPART_ZONEID:
storep = &u->zoneid;
break;
case CURLUPART_PORT:
u->portnum = 0;
storep = &u->port;
break;
case CURLUPART_PATH:
storep = &u->path;
break;
case CURLUPART_QUERY:
storep = &u->query;
break;
case CURLUPART_FRAGMENT:
storep = &u->fragment;
break;
default:
return CURLUE_UNKNOWN_PART;
}
if(storep && *storep) {
free(*storep);
*storep = NULL;
}
return CURLUE_OK;
}

switch(what) {
case CURLUPART_SCHEME:
if(strlen(part) > MAX_SCHEME_LEN)

return CURLUE_MALFORMED_INPUT;
if(!(flags & CURLU_NON_SUPPORT_SCHEME) &&

!Curl_builtin_scheme(part))
return CURLUE_UNSUPPORTED_SCHEME;
storep = &u->scheme;
urlencode = FALSE; 
break;
case CURLUPART_USER:
storep = &u->user;
break;
case CURLUPART_PASSWORD:
storep = &u->password;
break;
case CURLUPART_OPTIONS:
storep = &u->options;
break;
case CURLUPART_HOST:
storep = &u->host;
free(u->zoneid);
u->zoneid = NULL;
break;
case CURLUPART_ZONEID:
storep = &u->zoneid;
break;
case CURLUPART_PORT:
{
char *endp;
urlencode = FALSE; 
port = strtol(part, &endp, 10); 
if((port <= 0) || (port > 0xffff))
return CURLUE_BAD_PORT_NUMBER;
if(*endp)

return CURLUE_MALFORMED_INPUT;
storep = &u->port;
}
break;
case CURLUPART_PATH:
urlskipslash = TRUE;
storep = &u->path;
break;
case CURLUPART_QUERY:
plusencode = urlencode;
appendquery = (flags & CURLU_APPENDQUERY)?1:0;
equalsencode = appendquery;
storep = &u->query;
break;
case CURLUPART_FRAGMENT:
storep = &u->fragment;
break;
case CURLUPART_URL: {






CURLUcode result;
char *oldurl;
char *redired_url;
CURLU *handle2;

if(Curl_is_absolute_url(part, NULL, MAX_SCHEME_LEN + 1)) {
handle2 = curl_url();
if(!handle2)
return CURLUE_OUT_OF_MEMORY;
result = parseurl(part, handle2, flags);
if(!result)
mv_urlhandle(handle2, u);
else
curl_url_cleanup(handle2);
return result;
}

result = curl_url_get(u, CURLUPART_URL, &oldurl, flags);
if(result) {

handle2 = curl_url();
if(!handle2)
return CURLUE_OUT_OF_MEMORY;
result = parseurl(part, handle2, flags);
if(!result)
mv_urlhandle(handle2, u);
else
curl_url_cleanup(handle2);
return result;
}


redired_url = concat_url(oldurl, part);
free(oldurl);
if(!redired_url)
return CURLUE_OUT_OF_MEMORY;


handle2 = curl_url();
if(!handle2) {
free(redired_url);
return CURLUE_OUT_OF_MEMORY;
}
result = parseurl(redired_url, handle2, flags);
free(redired_url);
if(!result)
mv_urlhandle(handle2, u);
else
curl_url_cleanup(handle2);
return result;
}
default:
return CURLUE_UNKNOWN_PART;
}
DEBUGASSERT(storep);
{
const char *newp = part;
size_t nalloc = strlen(part);

if(nalloc > CURL_MAX_INPUT_LENGTH)

return CURLUE_MALFORMED_INPUT;

if(urlencode) {
const unsigned char *i;
char *o;
bool free_part = FALSE;
char *enc = malloc(nalloc * 3 + 1); 
if(!enc)
return CURLUE_OUT_OF_MEMORY;
if(plusencode) {

i = (const unsigned char *)part;
for(o = enc; *i; ++o, ++i)
*o = (*i == ' ') ? '+' : *i;
*o = 0; 
part = strdup(enc);
if(!part) {
free(enc);
return CURLUE_OUT_OF_MEMORY;
}
free_part = TRUE;
}
for(i = (const unsigned char *)part, o = enc; *i; i++) {
if(Curl_isunreserved(*i) ||
((*i == '/') && urlskipslash) ||
((*i == '=') && equalsencode) ||
((*i == '+') && plusencode)) {
if((*i == '=') && equalsencode)

equalsencode = FALSE;
*o = *i;
o++;
}
else {
msnprintf(o, 4, "%%%02x", *i);
o += 3;
}
}
*o = 0; 
newp = enc;
if(free_part)
free((char *)part);
}
else {
char *p;
newp = strdup(part);
if(!newp)
return CURLUE_OUT_OF_MEMORY;
p = (char *)newp;
while(*p) {

if((*p == '%') && ISXDIGIT(p[1]) && ISXDIGIT(p[2]) &&
(ISUPPER(p[1]) || ISUPPER(p[2]))) {
p[1] = (char)TOLOWER(p[1]);
p[2] = (char)TOLOWER(p[2]);
p += 3;
}
else
p++;
}
}

if(appendquery) {


size_t querylen = u->query ? strlen(u->query) : 0;
bool addamperand = querylen && (u->query[querylen -1] != '&');
if(querylen) {
size_t newplen = strlen(newp);
char *p = malloc(querylen + addamperand + newplen + 1);
if(!p) {
free((char *)newp);
return CURLUE_OUT_OF_MEMORY;
}
strcpy(p, u->query); 
if(addamperand)
p[querylen] = '&'; 
strcpy(&p[querylen + addamperand], newp); 
free((char *)newp);
free(*storep);
*storep = p;
return CURLUE_OK;
}
}

if(what == CURLUPART_HOST) {
if(0 == strlen(newp) && (flags & CURLU_NO_AUTHORITY)) {

}
else {
if(hostname_check(u, (char *)newp)) {
free((char *)newp);
return CURLUE_MALFORMED_INPUT;
}
}
}

free(*storep);
*storep = (char *)newp;
}


if(port)
u->portnum = port;
return CURLUE_OK;
}
