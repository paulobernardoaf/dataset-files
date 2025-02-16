#include "rubysocket.h"
struct inetsock_arg
{
VALUE sock;
struct {
VALUE host, serv;
struct rb_addrinfo *res;
} remote, local;
int type;
int fd;
};
static VALUE
inetsock_cleanup(VALUE v)
{
struct inetsock_arg *arg = (void *)v;
if (arg->remote.res) {
rb_freeaddrinfo(arg->remote.res);
arg->remote.res = 0;
}
if (arg->local.res) {
rb_freeaddrinfo(arg->local.res);
arg->local.res = 0;
}
if (arg->fd >= 0) {
close(arg->fd);
}
return Qnil;
}
static VALUE
init_inetsock_internal(VALUE v)
{
struct inetsock_arg *arg = (void *)v;
int error = 0;
int type = arg->type;
struct addrinfo *res, *lres;
int fd, status = 0, local = 0;
int family = AF_UNSPEC;
const char *syscall = 0;
arg->remote.res = rsock_addrinfo(arg->remote.host, arg->remote.serv,
family, SOCK_STREAM,
(type == INET_SERVER) ? AI_PASSIVE : 0);
if (type != INET_SERVER && (!NIL_P(arg->local.host) || !NIL_P(arg->local.serv))) {
arg->local.res = rsock_addrinfo(arg->local.host, arg->local.serv,
family, SOCK_STREAM, 0);
}
arg->fd = fd = -1;
for (res = arg->remote.res->ai; res; res = res->ai_next) {
#if !defined(INET6) && defined(AF_INET6)
if (res->ai_family == AF_INET6)
continue;
#endif
lres = NULL;
if (arg->local.res) {
for (lres = arg->local.res->ai; lres; lres = lres->ai_next) {
if (lres->ai_family == res->ai_family)
break;
}
if (!lres) {
if (res->ai_next || status < 0)
continue;
lres = arg->local.res->ai;
}
}
status = rsock_socket(res->ai_family,res->ai_socktype,res->ai_protocol);
syscall = "socket(2)";
fd = status;
if (fd < 0) {
error = errno;
continue;
}
arg->fd = fd;
if (type == INET_SERVER) {
#if !defined(_WIN32) && !defined(__CYGWIN__)
status = 1;
setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
(char*)&status, (socklen_t)sizeof(status));
#endif
status = bind(fd, res->ai_addr, res->ai_addrlen);
syscall = "bind(2)";
}
else {
if (lres) {
#if !defined(_WIN32) && !defined(__CYGWIN__)
status = 1;
setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
(char*)&status, (socklen_t)sizeof(status));
#endif
status = bind(fd, lres->ai_addr, lres->ai_addrlen);
local = status;
syscall = "bind(2)";
}
if (status >= 0) {
status = rsock_connect(fd, res->ai_addr, res->ai_addrlen,
(type == INET_SOCKS));
syscall = "connect(2)";
}
}
if (status < 0) {
error = errno;
close(fd);
arg->fd = fd = -1;
continue;
} else
break;
}
if (status < 0) {
VALUE host, port;
if (local < 0) {
host = arg->local.host;
port = arg->local.serv;
} else {
host = arg->remote.host;
port = arg->remote.serv;
}
rsock_syserr_fail_host_port(error, syscall, host, port);
}
arg->fd = -1;
if (type == INET_SERVER) {
status = listen(fd, SOMAXCONN);
if (status < 0) {
error = errno;
close(fd);
rb_syserr_fail(error, "listen(2)");
}
}
return rsock_init_sock(arg->sock, fd);
}
VALUE
rsock_init_inetsock(VALUE sock, VALUE remote_host, VALUE remote_serv,
VALUE local_host, VALUE local_serv, int type)
{
struct inetsock_arg arg;
arg.sock = sock;
arg.remote.host = remote_host;
arg.remote.serv = remote_serv;
arg.remote.res = 0;
arg.local.host = local_host;
arg.local.serv = local_serv;
arg.local.res = 0;
arg.type = type;
arg.fd = -1;
return rb_ensure(init_inetsock_internal, (VALUE)&arg,
inetsock_cleanup, (VALUE)&arg);
}
static ID id_numeric, id_hostname;
int
rsock_revlookup_flag(VALUE revlookup, int *norevlookup)
{
#define return_norevlookup(x) {*norevlookup = (x); return 1;}
ID id;
switch (revlookup) {
case Qtrue: return_norevlookup(0);
case Qfalse: return_norevlookup(1);
case Qnil: break;
default:
Check_Type(revlookup, T_SYMBOL);
id = SYM2ID(revlookup);
if (id == id_numeric) return_norevlookup(1);
if (id == id_hostname) return_norevlookup(0);
rb_raise(rb_eArgError, "invalid reverse_lookup flag: :%s", rb_id2name(id));
}
return 0;
#undef return_norevlookup
}
static VALUE
ip_inspect(VALUE sock)
{
VALUE str = rb_call_super(0, 0);
rb_io_t *fptr = RFILE(sock)->fptr;
union_sockaddr addr;
socklen_t len = (socklen_t)sizeof addr;
ID id;
if (fptr && fptr->fd >= 0 &&
getsockname(fptr->fd, &addr.addr, &len) >= 0 &&
(id = rsock_intern_family(addr.addr.sa_family)) != 0) {
VALUE family = rb_id2str(id);
char hbuf[1024], pbuf[1024];
long slen = RSTRING_LEN(str);
const char last = (slen > 1 && RSTRING_PTR(str)[slen - 1] == '>') ?
(--slen, '>') : 0;
str = rb_str_subseq(str, 0, slen);
rb_str_cat_cstr(str, ", ");
rb_str_append(str, family);
if (!rb_getnameinfo(&addr.addr, len, hbuf, sizeof(hbuf),
pbuf, sizeof(pbuf), NI_NUMERICHOST | NI_NUMERICSERV)) {
rb_str_cat_cstr(str, ", ");
rb_str_cat_cstr(str, hbuf);
rb_str_cat_cstr(str, ", ");
rb_str_cat_cstr(str, pbuf);
}
if (last) rb_str_cat(str, &last, 1);
}
return str;
}
static VALUE
ip_addr(int argc, VALUE *argv, VALUE sock)
{
rb_io_t *fptr;
union_sockaddr addr;
socklen_t len = (socklen_t)sizeof addr;
int norevlookup;
GetOpenFile(sock, fptr);
if (argc < 1 || !rsock_revlookup_flag(argv[0], &norevlookup))
norevlookup = fptr->mode & FMODE_NOREVLOOKUP;
if (getsockname(fptr->fd, &addr.addr, &len) < 0)
rb_sys_fail("getsockname(2)");
return rsock_ipaddr(&addr.addr, len, norevlookup);
}
static VALUE
ip_peeraddr(int argc, VALUE *argv, VALUE sock)
{
rb_io_t *fptr;
union_sockaddr addr;
socklen_t len = (socklen_t)sizeof addr;
int norevlookup;
GetOpenFile(sock, fptr);
if (argc < 1 || !rsock_revlookup_flag(argv[0], &norevlookup))
norevlookup = fptr->mode & FMODE_NOREVLOOKUP;
if (getpeername(fptr->fd, &addr.addr, &len) < 0)
rb_sys_fail("getpeername(2)");
return rsock_ipaddr(&addr.addr, len, norevlookup);
}
static VALUE
ip_recvfrom(int argc, VALUE *argv, VALUE sock)
{
return rsock_s_recvfrom(sock, argc, argv, RECV_IP);
}
static VALUE
ip_s_getaddress(VALUE obj, VALUE host)
{
union_sockaddr addr;
struct rb_addrinfo *res = rsock_addrinfo(host, Qnil, AF_UNSPEC, SOCK_STREAM, 0);
socklen_t len = res->ai->ai_addrlen;
memcpy(&addr, res->ai->ai_addr, len);
rb_freeaddrinfo(res);
return rsock_make_ipaddr(&addr.addr, len);
}
void
rsock_init_ipsocket(void)
{
rb_cIPSocket = rb_define_class("IPSocket", rb_cBasicSocket);
rb_define_method(rb_cIPSocket, "inspect", ip_inspect, 0);
rb_define_method(rb_cIPSocket, "addr", ip_addr, -1);
rb_define_method(rb_cIPSocket, "peeraddr", ip_peeraddr, -1);
rb_define_method(rb_cIPSocket, "recvfrom", ip_recvfrom, -1);
rb_define_singleton_method(rb_cIPSocket, "getaddress", ip_s_getaddress, 1);
rb_undef_method(rb_cIPSocket, "getpeereid");
id_numeric = rb_intern_const("numeric");
id_hostname = rb_intern_const("hostname");
}
