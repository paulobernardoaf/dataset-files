









#include "rubysocket.h"

static VALUE sym_wait_writable;

static VALUE sock_s_unpack_sockaddr_in(VALUE, VALUE);

void
rsock_sys_fail_host_port(const char *mesg, VALUE host, VALUE port)
{
rsock_syserr_fail_host_port(errno, mesg, host, port);
}

void
rsock_syserr_fail_host_port(int err, const char *mesg, VALUE host, VALUE port)
{
VALUE message;

message = rb_sprintf("%s for %+"PRIsVALUE" port % "PRIsVALUE"",
mesg, host, port);

rb_syserr_fail_str(err, message);
}

void
rsock_sys_fail_path(const char *mesg, VALUE path)
{
rsock_syserr_fail_path(errno, mesg, path);
}

void
rsock_syserr_fail_path(int err, const char *mesg, VALUE path)
{
VALUE message;

if (RB_TYPE_P(path, T_STRING)) {
message = rb_sprintf("%s for % "PRIsVALUE"", mesg, path);
rb_syserr_fail_str(err, message);
}
else {
rb_syserr_fail(err, mesg);
}
}

void
rsock_sys_fail_sockaddr(const char *mesg, struct sockaddr *addr, socklen_t len)
{
rsock_syserr_fail_sockaddr(errno, mesg, addr, len);
}

void
rsock_syserr_fail_sockaddr(int err, const char *mesg, struct sockaddr *addr, socklen_t len)
{
VALUE rai;

rai = rsock_addrinfo_new(addr, len, PF_UNSPEC, 0, 0, Qnil, Qnil);

rsock_syserr_fail_raddrinfo(err, mesg, rai);
}

void
rsock_sys_fail_raddrinfo(const char *mesg, VALUE rai)
{
rsock_syserr_fail_raddrinfo(errno, mesg, rai);
}

void
rsock_syserr_fail_raddrinfo(int err, const char *mesg, VALUE rai)
{
VALUE str, message;

str = rsock_addrinfo_inspect_sockaddr(rai);
message = rb_sprintf("%s for %"PRIsVALUE"", mesg, str);

rb_syserr_fail_str(err, message);
}

void
rsock_sys_fail_raddrinfo_or_sockaddr(const char *mesg, VALUE addr, VALUE rai)
{
rsock_syserr_fail_raddrinfo_or_sockaddr(errno, mesg, addr, rai);
}

void
rsock_syserr_fail_raddrinfo_or_sockaddr(int err, const char *mesg, VALUE addr, VALUE rai)
{
if (NIL_P(rai)) {
StringValue(addr);

rsock_syserr_fail_sockaddr(err, mesg,
(struct sockaddr *)RSTRING_PTR(addr),
(socklen_t)RSTRING_LEN(addr)); 
}
else
rsock_syserr_fail_raddrinfo(err, mesg, rai);
}

static void
setup_domain_and_type(VALUE domain, int *dv, VALUE type, int *tv)
{
*dv = rsock_family_arg(domain);
*tv = rsock_socktype_arg(type);
}



















static VALUE
sock_initialize(int argc, VALUE *argv, VALUE sock)
{
VALUE domain, type, protocol;
int fd;
int d, t;

rb_scan_args(argc, argv, "21", &domain, &type, &protocol);
if (NIL_P(protocol))
protocol = INT2FIX(0);

setup_domain_and_type(domain, &d, type, &t);
fd = rsock_socket(d, t, NUM2INT(protocol));
if (fd < 0) rb_sys_fail("socket(2)");

return rsock_init_sock(sock, fd);
}

#if defined HAVE_SOCKETPAIR
static VALUE
io_call_close(VALUE io)
{
return rb_funcallv(io, rb_intern("close"), 0, 0);
}

static VALUE
io_close(VALUE io)
{
return rb_rescue(io_call_close, io, 0, 0);
}

static VALUE
pair_yield(VALUE pair)
{
return rb_ensure(rb_yield, pair, io_close, rb_ary_entry(pair, 1));
}
#endif

#if defined HAVE_SOCKETPAIR

#if defined(SOCK_CLOEXEC)
static int
rsock_socketpair0(int domain, int type, int protocol, int sv[2])
{
int ret;
static int cloexec_state = -1; 
static const int default_flags = SOCK_CLOEXEC|RSOCK_NONBLOCK_DEFAULT;

if (cloexec_state > 0) { 
ret = socketpair(domain, type|default_flags, protocol, sv);
if (ret == 0 && (sv[0] <= 2 || sv[1] <= 2)) {
goto fix_cloexec; 
}
goto update_max_fd;
}
else if (cloexec_state < 0) { 
ret = socketpair(domain, type|default_flags, protocol, sv);
if (ret == 0) {
cloexec_state = rsock_detect_cloexec(sv[0]);
if ((cloexec_state == 0) || (sv[0] <= 2 || sv[1] <= 2))
goto fix_cloexec;
goto update_max_fd;
}
else if (ret == -1 && errno == EINVAL) {

ret = socketpair(domain, type, protocol, sv);
if (ret != -1) {




cloexec_state = 0;
}
}
}
else { 
ret = socketpair(domain, type, protocol, sv);
}
if (ret == -1) {
return -1;
}

fix_cloexec:
rb_maygvl_fd_fix_cloexec(sv[0]);
rb_maygvl_fd_fix_cloexec(sv[1]);
if (RSOCK_NONBLOCK_DEFAULT) {
rsock_make_fd_nonblock(sv[0]);
rsock_make_fd_nonblock(sv[1]);
}

update_max_fd:
rb_update_max_fd(sv[0]);
rb_update_max_fd(sv[1]);

return ret;
}
#else 
static int
rsock_socketpair0(int domain, int type, int protocol, int sv[2])
{
int ret = socketpair(domain, type, protocol, sv);

if (ret == -1)
return -1;

rb_fd_fix_cloexec(sv[0]);
rb_fd_fix_cloexec(sv[1]);
if (RSOCK_NONBLOCK_DEFAULT) {
rsock_make_fd_nonblock(sv[0]);
rsock_make_fd_nonblock(sv[1]);
}
return ret;
}
#endif 

static int
rsock_socketpair(int domain, int type, int protocol, int sv[2])
{
int ret;

ret = rsock_socketpair0(domain, type, protocol, sv);
if (ret < 0 && rb_gc_for_fd(errno)) {
ret = rsock_socketpair0(domain, type, protocol, sv);
}

return ret;
}






























VALUE
rsock_sock_s_socketpair(int argc, VALUE *argv, VALUE klass)
{
VALUE domain, type, protocol;
int d, t, p, sp[2];
int ret;
VALUE s1, s2, r;

rb_scan_args(argc, argv, "21", &domain, &type, &protocol);
if (NIL_P(protocol))
protocol = INT2FIX(0);

setup_domain_and_type(domain, &d, type, &t);
p = NUM2INT(protocol);
ret = rsock_socketpair(d, t, p, sp);
if (ret < 0) {
rb_sys_fail("socketpair(2)");
}

s1 = rsock_init_sock(rb_obj_alloc(klass), sp[0]);
s2 = rsock_init_sock(rb_obj_alloc(klass), sp[1]);
r = rb_assoc_new(s1, s2);
if (rb_block_given_p()) {
return rb_ensure(pair_yield, r, io_close, s1);
}
return r;
}
#else
#define rsock_sock_s_socketpair rb_f_notimplement
#endif
















































































































static VALUE
sock_connect(VALUE sock, VALUE addr)
{
VALUE rai;
rb_io_t *fptr;
int fd, n;

SockAddrStringValueWithAddrinfo(addr, rai);
addr = rb_str_new4(addr);
GetOpenFile(sock, fptr);
fd = fptr->fd;
n = rsock_connect(fd, (struct sockaddr*)RSTRING_PTR(addr), RSTRING_SOCKLEN(addr), 0);
if (n < 0) {
rsock_sys_fail_raddrinfo_or_sockaddr("connect(2)", addr, rai);
}

return INT2FIX(n);
}


static VALUE
sock_connect_nonblock(VALUE sock, VALUE addr, VALUE ex)
{
VALUE rai;
rb_io_t *fptr;
int n;

SockAddrStringValueWithAddrinfo(addr, rai);
addr = rb_str_new4(addr);
GetOpenFile(sock, fptr);
rb_io_set_nonblock(fptr);
n = connect(fptr->fd, (struct sockaddr*)RSTRING_PTR(addr), RSTRING_SOCKLEN(addr));
if (n < 0) {
int e = errno;
if (e == EINPROGRESS) {
if (ex == Qfalse) {
return sym_wait_writable;
}
rb_readwrite_syserr_fail(RB_IO_WAIT_WRITABLE, e, "connect(2) would block");
}
if (e == EISCONN) {
if (ex == Qfalse) {
return INT2FIX(0);
}
}
rsock_syserr_fail_raddrinfo_or_sockaddr(e, "connect(2)", addr, rai);
}

return INT2FIX(n);
}























































































static VALUE
sock_bind(VALUE sock, VALUE addr)
{
VALUE rai;
rb_io_t *fptr;

SockAddrStringValueWithAddrinfo(addr, rai);
GetOpenFile(sock, fptr);
if (bind(fptr->fd, (struct sockaddr*)RSTRING_PTR(addr), RSTRING_SOCKLEN(addr)) < 0)
rsock_sys_fail_raddrinfo_or_sockaddr("bind(2)", addr, rai);

return INT2FIX(0);
}







































































VALUE
rsock_sock_listen(VALUE sock, VALUE log)
{
rb_io_t *fptr;
int backlog;

backlog = NUM2INT(log);
GetOpenFile(sock, fptr);
if (listen(fptr->fd, backlog) < 0)
rb_sys_fail("listen(2)");

return INT2FIX(0);
}










































































































static VALUE
sock_recvfrom(int argc, VALUE *argv, VALUE sock)
{
return rsock_s_recvfrom(sock, argc, argv, RECV_SOCKET);
}


static VALUE
sock_recvfrom_nonblock(VALUE sock, VALUE len, VALUE flg, VALUE str, VALUE ex)
{
return rsock_s_recvfrom_nonblock(sock, len, flg, str, ex, RECV_SOCKET);
}















static VALUE
sock_accept(VALUE sock)
{
rb_io_t *fptr;
VALUE sock2;
union_sockaddr buf;
socklen_t len = (socklen_t)sizeof buf;

GetOpenFile(sock, fptr);
sock2 = rsock_s_accept(rb_cSocket,fptr->fd,&buf.addr,&len);

return rb_assoc_new(sock2, rsock_io_socket_addrinfo(sock2, &buf.addr, len));
}


static VALUE
sock_accept_nonblock(VALUE sock, VALUE ex)
{
rb_io_t *fptr;
VALUE sock2;
union_sockaddr buf;
struct sockaddr *addr = &buf.addr;
socklen_t len = (socklen_t)sizeof buf;

GetOpenFile(sock, fptr);
sock2 = rsock_s_accept_nonblock(rb_cSocket, ex, fptr, addr, &len);

if (SYMBOL_P(sock2)) 
return sock2;
return rb_assoc_new(sock2, rsock_io_socket_addrinfo(sock2, &buf.addr, len));
}







































static VALUE
sock_sysaccept(VALUE sock)
{
rb_io_t *fptr;
VALUE sock2;
union_sockaddr buf;
socklen_t len = (socklen_t)sizeof buf;

GetOpenFile(sock, fptr);
sock2 = rsock_s_accept(0,fptr->fd,&buf.addr,&len);

return rb_assoc_new(sock2, rsock_io_socket_addrinfo(sock2, &buf.addr, len));
}

#if defined(HAVE_GETHOSTNAME)











static VALUE
sock_gethostname(VALUE obj)
{
#if defined(NI_MAXHOST)
#define RUBY_MAX_HOST_NAME_LEN NI_MAXHOST
#elif defined(HOST_NAME_MAX)
#define RUBY_MAX_HOST_NAME_LEN HOST_NAME_MAX
#else
#define RUBY_MAX_HOST_NAME_LEN 1024
#endif

long len = RUBY_MAX_HOST_NAME_LEN;
VALUE name;

name = rb_str_new(0, len);
while (gethostname(RSTRING_PTR(name), len) < 0) {
int e = errno;
switch (e) {
case ENAMETOOLONG:
#if defined(__linux__)
case EINVAL:

#endif
break;
default:
rb_syserr_fail(e, "gethostname(3)");
}
rb_str_modify_expand(name, len);
len += len;
}
rb_str_resize(name, strlen(RSTRING_PTR(name)));
return name;
}
#else
#if defined(HAVE_UNAME)

#include <sys/utsname.h>

static VALUE
sock_gethostname(VALUE obj)
{
struct utsname un;

uname(&un);
return rb_str_new2(un.nodename);
}
#else
#define sock_gethostname rb_f_notimplement
#endif
#endif

static VALUE
make_addrinfo(struct rb_addrinfo *res0, int norevlookup)
{
VALUE base, ary;
struct addrinfo *res;

if (res0 == NULL) {
rb_raise(rb_eSocket, "host not found");
}
base = rb_ary_new();
for (res = res0->ai; res; res = res->ai_next) {
ary = rsock_ipaddr(res->ai_addr, res->ai_addrlen, norevlookup);
if (res->ai_canonname) {
RARRAY_ASET(ary, 2, rb_str_new2(res->ai_canonname));
}
rb_ary_push(ary, INT2FIX(res->ai_family));
rb_ary_push(ary, INT2FIX(res->ai_socktype));
rb_ary_push(ary, INT2FIX(res->ai_protocol));
rb_ary_push(base, ary);
}
return base;
}

static VALUE
sock_sockaddr(struct sockaddr *addr, socklen_t len)
{
char *ptr;

switch (addr->sa_family) {
case AF_INET:
ptr = (char*)&((struct sockaddr_in*)addr)->sin_addr.s_addr;
len = (socklen_t)sizeof(((struct sockaddr_in*)addr)->sin_addr.s_addr);
break;
#if defined(AF_INET6)
case AF_INET6:
ptr = (char*)&((struct sockaddr_in6*)addr)->sin6_addr.s6_addr;
len = (socklen_t)sizeof(((struct sockaddr_in6*)addr)->sin6_addr.s6_addr);
break;
#endif
default:
rb_raise(rb_eSocket, "unknown socket family:%d", addr->sa_family);
break;
}
return rb_str_new(ptr, len);
}





















static VALUE
sock_s_gethostbyname(VALUE obj, VALUE host)
{
struct rb_addrinfo *res =
rsock_addrinfo(host, Qnil, AF_UNSPEC, SOCK_STREAM, AI_CANONNAME);
return rsock_make_hostent(host, res, sock_sockaddr);
}


























static VALUE
sock_s_gethostbyaddr(int argc, VALUE *argv, VALUE _)
{
VALUE addr, family;
struct hostent *h;
char **pch;
VALUE ary, names;
int t = AF_INET;

rb_scan_args(argc, argv, "11", &addr, &family);
StringValue(addr);
if (!NIL_P(family)) {
t = rsock_family_arg(family);
}
#if defined(AF_INET6)
else if (RSTRING_LEN(addr) == 16) {
t = AF_INET6;
}
#endif
h = gethostbyaddr(RSTRING_PTR(addr), RSTRING_SOCKLEN(addr), t);
if (h == NULL) {
#if defined(HAVE_HSTRERROR)
extern int h_errno;
rb_raise(rb_eSocket, "%s", (char*)hstrerror(h_errno));
#else
rb_raise(rb_eSocket, "host not found");
#endif
}
ary = rb_ary_new();
rb_ary_push(ary, rb_str_new2(h->h_name));
names = rb_ary_new();
rb_ary_push(ary, names);
if (h->h_aliases != NULL) {
for (pch = h->h_aliases; *pch; pch++) {
rb_ary_push(names, rb_str_new2(*pch));
}
}
rb_ary_push(ary, INT2NUM(h->h_addrtype));
#if defined(h_addr)
for (pch = h->h_addr_list; *pch; pch++) {
rb_ary_push(ary, rb_str_new(*pch, h->h_length));
}
#else
rb_ary_push(ary, rb_str_new(h->h_addr, h->h_length));
#endif

return ary;
}














static VALUE
sock_s_getservbyname(int argc, VALUE *argv, VALUE _)
{
VALUE service, proto;
struct servent *sp;
long port;
const char *servicename, *protoname = "tcp";

rb_scan_args(argc, argv, "11", &service, &proto);
StringValue(service);
if (!NIL_P(proto)) StringValue(proto);
servicename = StringValueCStr(service);
if (!NIL_P(proto)) protoname = StringValueCStr(proto);
sp = getservbyname(servicename, protoname);
if (sp) {
port = ntohs(sp->s_port);
}
else {
char *end;

port = STRTOUL(servicename, &end, 0);
if (*end != '\0') {
rb_raise(rb_eSocket, "no such service %s/%s", servicename, protoname);
}
}
return INT2FIX(port);
}














static VALUE
sock_s_getservbyport(int argc, VALUE *argv, VALUE _)
{
VALUE port, proto;
struct servent *sp;
long portnum;
const char *protoname = "tcp";

rb_scan_args(argc, argv, "11", &port, &proto);
portnum = NUM2LONG(port);
if (portnum != (uint16_t)portnum) {
const char *s = portnum > 0 ? "big" : "small";
rb_raise(rb_eRangeError, "integer %ld too %s to convert into `int16_t'", portnum, s);
}
if (!NIL_P(proto)) protoname = StringValueCStr(proto);

sp = getservbyport((int)htons((uint16_t)portnum), protoname);
if (!sp) {
rb_raise(rb_eSocket, "no such service for port %d/%s", (int)portnum, protoname);
}
return rb_str_new2(sp->s_name);
}




































static VALUE
sock_s_getaddrinfo(int argc, VALUE *argv, VALUE _)
{
VALUE host, port, family, socktype, protocol, flags, ret, revlookup;
struct addrinfo hints;
struct rb_addrinfo *res;
int norevlookup;

rb_scan_args(argc, argv, "25", &host, &port, &family, &socktype, &protocol, &flags, &revlookup);

MEMZERO(&hints, struct addrinfo, 1);
hints.ai_family = NIL_P(family) ? PF_UNSPEC : rsock_family_arg(family);

if (!NIL_P(socktype)) {
hints.ai_socktype = rsock_socktype_arg(socktype);
}
if (!NIL_P(protocol)) {
hints.ai_protocol = NUM2INT(protocol);
}
if (!NIL_P(flags)) {
hints.ai_flags = NUM2INT(flags);
}
if (NIL_P(revlookup) || !rsock_revlookup_flag(revlookup, &norevlookup)) {
norevlookup = rsock_do_not_reverse_lookup;
}
res = rsock_getaddrinfo(host, port, &hints, 0);

ret = make_addrinfo(res, norevlookup);
rb_freeaddrinfo(res);
return ret;
}























static VALUE
sock_s_getnameinfo(int argc, VALUE *argv, VALUE _)
{
VALUE sa, af = Qnil, host = Qnil, port = Qnil, flags, tmp;
char *hptr, *pptr;
char hbuf[1024], pbuf[1024];
int fl;
struct rb_addrinfo *res = NULL;
struct addrinfo hints, *r;
int error, saved_errno;
union_sockaddr ss;
struct sockaddr *sap;
socklen_t salen;

sa = flags = Qnil;
rb_scan_args(argc, argv, "11", &sa, &flags);

fl = 0;
if (!NIL_P(flags)) {
fl = NUM2INT(flags);
}
tmp = rb_check_sockaddr_string_type(sa);
if (!NIL_P(tmp)) {
sa = tmp;
if (sizeof(ss) < (size_t)RSTRING_LEN(sa)) {
rb_raise(rb_eTypeError, "sockaddr length too big");
}
memcpy(&ss, RSTRING_PTR(sa), RSTRING_LEN(sa));
if (!VALIDATE_SOCKLEN(&ss.addr, RSTRING_LEN(sa))) {
rb_raise(rb_eTypeError, "sockaddr size differs - should not happen");
}
sap = &ss.addr;
salen = RSTRING_SOCKLEN(sa);
goto call_nameinfo;
}
tmp = rb_check_array_type(sa);
if (!NIL_P(tmp)) {
sa = tmp;
MEMZERO(&hints, struct addrinfo, 1);
if (RARRAY_LEN(sa) == 3) {
af = RARRAY_AREF(sa, 0);
port = RARRAY_AREF(sa, 1);
host = RARRAY_AREF(sa, 2);
}
else if (RARRAY_LEN(sa) >= 4) {
af = RARRAY_AREF(sa, 0);
port = RARRAY_AREF(sa, 1);
host = RARRAY_AREF(sa, 3);
if (NIL_P(host)) {
host = RARRAY_AREF(sa, 2);
}
else {




#if defined(AI_NUMERICHOST)
hints.ai_flags |= AI_NUMERICHOST;
#endif
}
}
else {
rb_raise(rb_eArgError, "array size should be 3 or 4, %ld given",
RARRAY_LEN(sa));
}

if (NIL_P(host)) {
hptr = NULL;
}
else {
strncpy(hbuf, StringValueCStr(host), sizeof(hbuf));
hbuf[sizeof(hbuf) - 1] = '\0';
hptr = hbuf;
}

if (NIL_P(port)) {
strcpy(pbuf, "0");
pptr = NULL;
}
else if (FIXNUM_P(port)) {
snprintf(pbuf, sizeof(pbuf), "%ld", NUM2LONG(port));
pptr = pbuf;
}
else {
strncpy(pbuf, StringValueCStr(port), sizeof(pbuf));
pbuf[sizeof(pbuf) - 1] = '\0';
pptr = pbuf;
}
hints.ai_socktype = (fl & NI_DGRAM) ? SOCK_DGRAM : SOCK_STREAM;

hints.ai_family = NIL_P(af) ? PF_UNSPEC : rsock_family_arg(af);
error = rb_getaddrinfo(hptr, pptr, &hints, &res);
if (error) goto error_exit_addr;
sap = res->ai->ai_addr;
salen = res->ai->ai_addrlen;
}
else {
rb_raise(rb_eTypeError, "expecting String or Array");
}

call_nameinfo:
error = rb_getnameinfo(sap, salen, hbuf, sizeof(hbuf),
pbuf, sizeof(pbuf), fl);
if (error) goto error_exit_name;
if (res) {
for (r = res->ai->ai_next; r; r = r->ai_next) {
char hbuf2[1024], pbuf2[1024];

sap = r->ai_addr;
salen = r->ai_addrlen;
error = rb_getnameinfo(sap, salen, hbuf2, sizeof(hbuf2),
pbuf2, sizeof(pbuf2), fl);
if (error) goto error_exit_name;
if (strcmp(hbuf, hbuf2) != 0|| strcmp(pbuf, pbuf2) != 0) {
rb_freeaddrinfo(res);
rb_raise(rb_eSocket, "sockaddr resolved to multiple nodename");
}
}
rb_freeaddrinfo(res);
}
return rb_assoc_new(rb_str_new2(hbuf), rb_str_new2(pbuf));

error_exit_addr:
saved_errno = errno;
if (res) rb_freeaddrinfo(res);
errno = saved_errno;
rsock_raise_socket_error("getaddrinfo", error);

error_exit_name:
saved_errno = errno;
if (res) rb_freeaddrinfo(res);
errno = saved_errno;
rsock_raise_socket_error("getnameinfo", error);

UNREACHABLE_RETURN(Qnil);
}















static VALUE
sock_s_pack_sockaddr_in(VALUE self, VALUE port, VALUE host)
{
struct rb_addrinfo *res = rsock_addrinfo(host, port, AF_UNSPEC, 0, 0);
VALUE addr = rb_str_new((char*)res->ai->ai_addr, res->ai->ai_addrlen);

rb_freeaddrinfo(res);

return addr;
}














static VALUE
sock_s_unpack_sockaddr_in(VALUE self, VALUE addr)
{
struct sockaddr_in * sockaddr;
VALUE host;

sockaddr = (struct sockaddr_in*)SockAddrStringValuePtr(addr);
if (RSTRING_LEN(addr) <
(char*)&((struct sockaddr *)sockaddr)->sa_family +
sizeof(((struct sockaddr *)sockaddr)->sa_family) -
(char*)sockaddr)
rb_raise(rb_eArgError, "too short sockaddr");
if (((struct sockaddr *)sockaddr)->sa_family != AF_INET
#if defined(INET6)
&& ((struct sockaddr *)sockaddr)->sa_family != AF_INET6
#endif
) {
#if defined(INET6)
rb_raise(rb_eArgError, "not an AF_INET/AF_INET6 sockaddr");
#else
rb_raise(rb_eArgError, "not an AF_INET sockaddr");
#endif
}
host = rsock_make_ipaddr((struct sockaddr*)sockaddr, RSTRING_SOCKLEN(addr));
return rb_assoc_new(INT2NUM(ntohs(sockaddr->sin_port)), host);
}

#if defined(HAVE_SYS_UN_H)











static VALUE
sock_s_pack_sockaddr_un(VALUE self, VALUE path)
{
struct sockaddr_un sockaddr;
VALUE addr;

StringValue(path);
INIT_SOCKADDR_UN(&sockaddr, sizeof(struct sockaddr_un));
if (sizeof(sockaddr.sun_path) < (size_t)RSTRING_LEN(path)) {
rb_raise(rb_eArgError, "too long unix socket path (%"PRIuSIZE" bytes given but %"PRIuSIZE" bytes max)",
(size_t)RSTRING_LEN(path), sizeof(sockaddr.sun_path));
}
memcpy(sockaddr.sun_path, RSTRING_PTR(path), RSTRING_LEN(path));
addr = rb_str_new((char*)&sockaddr, rsock_unix_sockaddr_len(path));

return addr;
}













static VALUE
sock_s_unpack_sockaddr_un(VALUE self, VALUE addr)
{
struct sockaddr_un * sockaddr;
VALUE path;

sockaddr = (struct sockaddr_un*)SockAddrStringValuePtr(addr);
if (RSTRING_LEN(addr) <
(char*)&((struct sockaddr *)sockaddr)->sa_family +
sizeof(((struct sockaddr *)sockaddr)->sa_family) -
(char*)sockaddr)
rb_raise(rb_eArgError, "too short sockaddr");
if (((struct sockaddr *)sockaddr)->sa_family != AF_UNIX) {
rb_raise(rb_eArgError, "not an AF_UNIX sockaddr");
}
if (sizeof(struct sockaddr_un) < (size_t)RSTRING_LEN(addr)) {
rb_raise(rb_eTypeError, "too long sockaddr_un - %ld longer than %d",
RSTRING_LEN(addr), (int)sizeof(struct sockaddr_un));
}
path = rsock_unixpath_str(sockaddr, RSTRING_SOCKLEN(addr));
return path;
}
#endif

#if defined(HAVE_GETIFADDRS) || defined(SIOCGLIFCONF) || defined(SIOCGIFCONF) || defined(_WIN32)

static socklen_t
sockaddr_len(struct sockaddr *addr)
{
if (addr == NULL)
return 0;

#if defined(HAVE_STRUCT_SOCKADDR_SA_LEN)
if (addr->sa_len != 0)
return addr->sa_len;
#endif

switch (addr->sa_family) {
case AF_INET:
return (socklen_t)sizeof(struct sockaddr_in);

#if defined(AF_INET6)
case AF_INET6:
return (socklen_t)sizeof(struct sockaddr_in6);
#endif

#if defined(HAVE_SYS_UN_H)
case AF_UNIX:
return (socklen_t)sizeof(struct sockaddr_un);
#endif

#if defined(AF_PACKET)
case AF_PACKET:
return (socklen_t)(offsetof(struct sockaddr_ll, sll_addr) + ((struct sockaddr_ll *)addr)->sll_halen);
#endif

default:
return (socklen_t)(offsetof(struct sockaddr, sa_family) + sizeof(addr->sa_family));
}
}

socklen_t
rsock_sockaddr_len(struct sockaddr *addr)
{
return sockaddr_len(addr);
}

static VALUE
sockaddr_obj(struct sockaddr *addr, socklen_t len)
{
#if defined(AF_INET6) && defined(__KAME__)
struct sockaddr_in6 addr6;
#endif

if (addr == NULL)
return Qnil;

len = sockaddr_len(addr);

#if defined(__KAME__) && defined(AF_INET6)
if (addr->sa_family == AF_INET6) {



len = (socklen_t)sizeof(struct sockaddr_in6);
memcpy(&addr6, addr, len);
addr = (struct sockaddr *)&addr6;
if (IN6_IS_ADDR_LINKLOCAL(&addr6.sin6_addr) &&
addr6.sin6_scope_id == 0 &&
(addr6.sin6_addr.s6_addr[2] || addr6.sin6_addr.s6_addr[3])) {
addr6.sin6_scope_id = (addr6.sin6_addr.s6_addr[2] << 8) | addr6.sin6_addr.s6_addr[3];
addr6.sin6_addr.s6_addr[2] = 0;
addr6.sin6_addr.s6_addr[3] = 0;
}
}
#endif

return rsock_addrinfo_new(addr, len, addr->sa_family, 0, 0, Qnil, Qnil);
}

VALUE
rsock_sockaddr_obj(struct sockaddr *addr, socklen_t len)
{
return sockaddr_obj(addr, len);
}

#endif

#if defined(HAVE_GETIFADDRS) || (defined(SIOCGLIFCONF) && defined(SIOCGLIFNUM) && !defined(__hpux)) || defined(SIOCGIFCONF) || defined(_WIN32)















static VALUE
socket_s_ip_address_list(VALUE self)
{
#if defined(HAVE_GETIFADDRS)
struct ifaddrs *ifp = NULL;
struct ifaddrs *p;
int ret;
VALUE list;

ret = getifaddrs(&ifp);
if (ret == -1) {
rb_sys_fail("getifaddrs");
}

list = rb_ary_new();
for (p = ifp; p; p = p->ifa_next) {
if (p->ifa_addr != NULL && IS_IP_FAMILY(p->ifa_addr->sa_family)) {
struct sockaddr *addr = p->ifa_addr;
#if defined(AF_INET6) && defined(__sun)





struct sockaddr_in6 addr6;
if (addr->sa_family == AF_INET6) {
socklen_t len = (socklen_t)sizeof(struct sockaddr_in6);
memcpy(&addr6, addr, len);
addr = (struct sockaddr *)&addr6;
if (IN6_IS_ADDR_LINKLOCAL(&addr6.sin6_addr) &&
addr6.sin6_scope_id == 0) {
unsigned int ifindex = if_nametoindex(p->ifa_name);
if (ifindex != 0) {
addr6.sin6_scope_id = ifindex;
}
}
}
#endif
rb_ary_push(list, sockaddr_obj(addr, sockaddr_len(addr)));
}
}

freeifaddrs(ifp);

return list;
#elif defined(SIOCGLIFCONF) && defined(SIOCGLIFNUM) && !defined(__hpux)


int fd = -1;
int ret;
struct lifnum ln;
struct lifconf lc;
const char *reason = NULL;
int save_errno;
int i;
VALUE list = Qnil;

lc.lifc_buf = NULL;

fd = socket(AF_INET, SOCK_DGRAM, 0);
if (fd == -1)
rb_sys_fail("socket(2)");

memset(&ln, 0, sizeof(ln));
ln.lifn_family = AF_UNSPEC;

ret = ioctl(fd, SIOCGLIFNUM, &ln);
if (ret == -1) {
reason = "SIOCGLIFNUM";
goto finish;
}

memset(&lc, 0, sizeof(lc));
lc.lifc_family = AF_UNSPEC;
lc.lifc_flags = 0;
lc.lifc_len = sizeof(struct lifreq) * ln.lifn_count;
lc.lifc_req = xmalloc(lc.lifc_len);

ret = ioctl(fd, SIOCGLIFCONF, &lc);
if (ret == -1) {
reason = "SIOCGLIFCONF";
goto finish;
}

list = rb_ary_new();
for (i = 0; i < ln.lifn_count; i++) {
struct lifreq *req = &lc.lifc_req[i];
if (IS_IP_FAMILY(req->lifr_addr.ss_family)) {
if (req->lifr_addr.ss_family == AF_INET6 &&
IN6_IS_ADDR_LINKLOCAL(&((struct sockaddr_in6 *)(&req->lifr_addr))->sin6_addr) &&
((struct sockaddr_in6 *)(&req->lifr_addr))->sin6_scope_id == 0) {
struct lifreq req2;
memcpy(req2.lifr_name, req->lifr_name, LIFNAMSIZ);
ret = ioctl(fd, SIOCGLIFINDEX, &req2);
if (ret == -1) {
reason = "SIOCGLIFINDEX";
goto finish;
}
((struct sockaddr_in6 *)(&req->lifr_addr))->sin6_scope_id = req2.lifr_index;
}
rb_ary_push(list, sockaddr_obj((struct sockaddr *)&req->lifr_addr, req->lifr_addrlen));
}
}

finish:
save_errno = errno;
if (lc.lifc_buf != NULL)
xfree(lc.lifc_req);
if (fd != -1)
close(fd);
errno = save_errno;

if (reason)
rb_syserr_fail(save_errno, reason);
return list;

#elif defined(SIOCGIFCONF)
int fd = -1;
int ret;
#define EXTRA_SPACE ((int)(sizeof(struct ifconf) + sizeof(union_sockaddr)))
char initbuf[4096+EXTRA_SPACE];
char *buf = initbuf;
int bufsize;
struct ifconf conf;
struct ifreq *req;
VALUE list = Qnil;
const char *reason = NULL;
int save_errno;

fd = socket(AF_INET, SOCK_DGRAM, 0);
if (fd == -1)
rb_sys_fail("socket(2)");

bufsize = sizeof(initbuf);
buf = initbuf;

retry:
conf.ifc_len = bufsize;
conf.ifc_req = (struct ifreq *)buf;



ret = ioctl(fd, SIOCGIFCONF, &conf);
if (ret == -1) {
reason = "SIOCGIFCONF";
goto finish;
}



if (bufsize - EXTRA_SPACE < conf.ifc_len) {
if (bufsize < conf.ifc_len) {

bufsize = conf.ifc_len + EXTRA_SPACE;
}
else {
bufsize = bufsize << 1;
}
if (buf == initbuf)
buf = NULL;
buf = xrealloc(buf, bufsize);
goto retry;
}

close(fd);
fd = -1;

list = rb_ary_new();
req = conf.ifc_req;
while ((char*)req < (char*)conf.ifc_req + conf.ifc_len) {
struct sockaddr *addr = &req->ifr_addr;
if (IS_IP_FAMILY(addr->sa_family)) {
rb_ary_push(list, sockaddr_obj(addr, sockaddr_len(addr)));
}
#if defined(HAVE_STRUCT_SOCKADDR_SA_LEN)
#if !defined(_SIZEOF_ADDR_IFREQ)
#define _SIZEOF_ADDR_IFREQ(r) (sizeof(struct ifreq) + (sizeof(struct sockaddr) < (r).ifr_addr.sa_len ? (r).ifr_addr.sa_len - sizeof(struct sockaddr) : 0))




#endif
req = (struct ifreq *)((char*)req + _SIZEOF_ADDR_IFREQ(*req));
#else
req = (struct ifreq *)((char*)req + sizeof(struct ifreq));
#endif
}

finish:

save_errno = errno;
if (buf != initbuf)
xfree(buf);
if (fd != -1)
close(fd);
errno = save_errno;

if (reason)
rb_syserr_fail(save_errno, reason);
return list;

#undef EXTRA_SPACE
#elif defined(_WIN32)
typedef struct ip_adapter_unicast_address_st {
unsigned LONG_LONG dummy0;
struct ip_adapter_unicast_address_st *Next;
struct {
struct sockaddr *lpSockaddr;
int iSockaddrLength;
} Address;
int dummy1;
int dummy2;
int dummy3;
long dummy4;
long dummy5;
long dummy6;
} ip_adapter_unicast_address_t;
typedef struct ip_adapter_anycast_address_st {
unsigned LONG_LONG dummy0;
struct ip_adapter_anycast_address_st *Next;
struct {
struct sockaddr *lpSockaddr;
int iSockaddrLength;
} Address;
} ip_adapter_anycast_address_t;
typedef struct ip_adapter_addresses_st {
unsigned LONG_LONG dummy0;
struct ip_adapter_addresses_st *Next;
void *dummy1;
ip_adapter_unicast_address_t *FirstUnicastAddress;
ip_adapter_anycast_address_t *FirstAnycastAddress;
void *dummy2;
void *dummy3;
void *dummy4;
void *dummy5;
void *dummy6;
BYTE dummy7[8];
DWORD dummy8;
DWORD dummy9;
DWORD dummy10;
DWORD IfType;
int OperStatus;
DWORD dummy12;
DWORD dummy13[16];
void *dummy14;
} ip_adapter_addresses_t;
typedef ULONG (WINAPI *GetAdaptersAddresses_t)(ULONG, ULONG, PVOID, ip_adapter_addresses_t *, PULONG);
HMODULE h;
GetAdaptersAddresses_t pGetAdaptersAddresses;
ULONG len;
DWORD ret;
ip_adapter_addresses_t *adapters;
VALUE list;

h = LoadLibrary("iphlpapi.dll");
if (!h)
rb_notimplement();
pGetAdaptersAddresses = (GetAdaptersAddresses_t)GetProcAddress(h, "GetAdaptersAddresses");
if (!pGetAdaptersAddresses) {
FreeLibrary(h);
rb_notimplement();
}

ret = pGetAdaptersAddresses(AF_UNSPEC, 0, NULL, NULL, &len);
if (ret != ERROR_SUCCESS && ret != ERROR_BUFFER_OVERFLOW) {
errno = rb_w32_map_errno(ret);
FreeLibrary(h);
rb_sys_fail("GetAdaptersAddresses");
}
adapters = (ip_adapter_addresses_t *)ALLOCA_N(BYTE, len);
ret = pGetAdaptersAddresses(AF_UNSPEC, 0, NULL, adapters, &len);
if (ret != ERROR_SUCCESS) {
errno = rb_w32_map_errno(ret);
FreeLibrary(h);
rb_sys_fail("GetAdaptersAddresses");
}

list = rb_ary_new();
for (; adapters; adapters = adapters->Next) {
ip_adapter_unicast_address_t *uni;
ip_adapter_anycast_address_t *any;
if (adapters->OperStatus != 1) 
continue;
for (uni = adapters->FirstUnicastAddress; uni; uni = uni->Next) {
#if !defined(INET6)
if (uni->Address.lpSockaddr->sa_family == AF_INET)
#else
if (IS_IP_FAMILY(uni->Address.lpSockaddr->sa_family))
#endif
rb_ary_push(list, sockaddr_obj(uni->Address.lpSockaddr, uni->Address.iSockaddrLength));
}
for (any = adapters->FirstAnycastAddress; any; any = any->Next) {
#if !defined(INET6)
if (any->Address.lpSockaddr->sa_family == AF_INET)
#else
if (IS_IP_FAMILY(any->Address.lpSockaddr->sa_family))
#endif
rb_ary_push(list, sockaddr_obj(any->Address.lpSockaddr, any->Address.iSockaddrLength));
}
}

FreeLibrary(h);
return list;
#endif
}
#else
#define socket_s_ip_address_list rb_f_notimplement
#endif

void
Init_socket(void)
{
rsock_init_basicsocket();






















































































































rb_cSocket = rb_define_class("Socket", rb_cBasicSocket);

rsock_init_socket_init();

rb_define_method(rb_cSocket, "initialize", sock_initialize, -1);
rb_define_method(rb_cSocket, "connect", sock_connect, 1);


rb_define_private_method(rb_cSocket,
"__connect_nonblock", sock_connect_nonblock, 2);

rb_define_method(rb_cSocket, "bind", sock_bind, 1);
rb_define_method(rb_cSocket, "listen", rsock_sock_listen, 1);
rb_define_method(rb_cSocket, "accept", sock_accept, 0);


rb_define_private_method(rb_cSocket,
"__accept_nonblock", sock_accept_nonblock, 1);

rb_define_method(rb_cSocket, "sysaccept", sock_sysaccept, 0);

rb_define_method(rb_cSocket, "recvfrom", sock_recvfrom, -1);


rb_define_private_method(rb_cSocket,
"__recvfrom_nonblock", sock_recvfrom_nonblock, 4);

rb_define_singleton_method(rb_cSocket, "socketpair", rsock_sock_s_socketpair, -1);
rb_define_singleton_method(rb_cSocket, "pair", rsock_sock_s_socketpair, -1);
rb_define_singleton_method(rb_cSocket, "gethostname", sock_gethostname, 0);
rb_define_singleton_method(rb_cSocket, "gethostbyname", sock_s_gethostbyname, 1);
rb_define_singleton_method(rb_cSocket, "gethostbyaddr", sock_s_gethostbyaddr, -1);
rb_define_singleton_method(rb_cSocket, "getservbyname", sock_s_getservbyname, -1);
rb_define_singleton_method(rb_cSocket, "getservbyport", sock_s_getservbyport, -1);
rb_define_singleton_method(rb_cSocket, "getaddrinfo", sock_s_getaddrinfo, -1);
rb_define_singleton_method(rb_cSocket, "getnameinfo", sock_s_getnameinfo, -1);
rb_define_singleton_method(rb_cSocket, "sockaddr_in", sock_s_pack_sockaddr_in, 2);
rb_define_singleton_method(rb_cSocket, "pack_sockaddr_in", sock_s_pack_sockaddr_in, 2);
rb_define_singleton_method(rb_cSocket, "unpack_sockaddr_in", sock_s_unpack_sockaddr_in, 1);
#if defined(HAVE_SYS_UN_H)
rb_define_singleton_method(rb_cSocket, "sockaddr_un", sock_s_pack_sockaddr_un, 1);
rb_define_singleton_method(rb_cSocket, "pack_sockaddr_un", sock_s_pack_sockaddr_un, 1);
rb_define_singleton_method(rb_cSocket, "unpack_sockaddr_un", sock_s_unpack_sockaddr_un, 1);
#endif

rb_define_singleton_method(rb_cSocket, "ip_address_list", socket_s_ip_address_list, 0);

#undef rb_intern
sym_wait_writable = ID2SYM(rb_intern("wait_writable"));
}
