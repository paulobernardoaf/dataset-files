









#include "rubysocket.h"












static VALUE
bsock_s_for_fd(VALUE klass, VALUE fd)
{
rb_io_t *fptr;
VALUE sock = rsock_init_sock(rb_obj_alloc(klass), NUM2INT(fd));

GetOpenFile(sock, fptr);

return sock;
}




























static VALUE
bsock_shutdown(int argc, VALUE *argv, VALUE sock)
{
VALUE howto;
int how;
rb_io_t *fptr;

rb_scan_args(argc, argv, "01", &howto);
if (howto == Qnil)
how = SHUT_RDWR;
else {
how = rsock_shutdown_how_arg(howto);
if (how != SHUT_WR && how != SHUT_RD && how != SHUT_RDWR) {
rb_raise(rb_eArgError, "`how' should be either :SHUT_RD, :SHUT_WR, :SHUT_RDWR");
}
}
GetOpenFile(sock, fptr);
if (shutdown(fptr->fd, how) == -1)
rb_sys_fail("shutdown(2)");

return INT2FIX(0);
}











static VALUE
bsock_close_read(VALUE sock)
{
rb_io_t *fptr;

GetOpenFile(sock, fptr);
shutdown(fptr->fd, 0);
if (!(fptr->mode & FMODE_WRITABLE)) {
return rb_io_close(sock);
}
fptr->mode &= ~FMODE_READABLE;

return Qnil;
}
















static VALUE
bsock_close_write(VALUE sock)
{
rb_io_t *fptr;

GetOpenFile(sock, fptr);
if (!(fptr->mode & FMODE_READABLE)) {
return rb_io_close(sock);
}
shutdown(fptr->fd, 1);
fptr->mode &= ~FMODE_WRITABLE;

return Qnil;
}


























































static VALUE
bsock_setsockopt(int argc, VALUE *argv, VALUE sock)
{
VALUE lev, optname, val;
int family, level, option;
rb_io_t *fptr;
int i;
char *v;
int vlen;

if (argc == 1) {
lev = rb_funcall(argv[0], rb_intern("level"), 0);
optname = rb_funcall(argv[0], rb_intern("optname"), 0);
val = rb_funcall(argv[0], rb_intern("data"), 0);
}
else {
rb_scan_args(argc, argv, "30", &lev, &optname, &val);
}

GetOpenFile(sock, fptr);
family = rsock_getfamily(fptr);
level = rsock_level_arg(family, lev);
option = rsock_optname_arg(family, level, optname);

switch (TYPE(val)) {
case T_FIXNUM:
i = FIX2INT(val);
goto numval;
case T_FALSE:
i = 0;
goto numval;
case T_TRUE:
i = 1;
numval:
v = (char*)&i; vlen = (int)sizeof(i);
break;
default:
StringValue(val);
v = RSTRING_PTR(val);
vlen = RSTRING_SOCKLEN(val);
break;
}

rb_io_check_closed(fptr);
if (setsockopt(fptr->fd, level, option, v, vlen) < 0)
rsock_sys_fail_path("setsockopt(2)", fptr->pathv);

return INT2FIX(0);
}

























































static VALUE
bsock_getsockopt(VALUE sock, VALUE lev, VALUE optname)
{
int level, option;
socklen_t len;
char *buf;
rb_io_t *fptr;
int family;

GetOpenFile(sock, fptr);
family = rsock_getfamily(fptr);
level = rsock_level_arg(family, lev);
option = rsock_optname_arg(family, level, optname);
len = 256;
#if defined(_AIX)
switch (option) {
case SO_DEBUG:
case SO_REUSEADDR:
case SO_KEEPALIVE:
case SO_DONTROUTE:
case SO_BROADCAST:
case SO_OOBINLINE:

len = sizeof(int);
}
#endif
buf = ALLOCA_N(char,len);

rb_io_check_closed(fptr);

if (getsockopt(fptr->fd, level, option, buf, &len) < 0)
rsock_sys_fail_path("getsockopt(2)", fptr->pathv);

return rsock_sockopt_new(family, level, option, rb_str_new(buf, len));
}














static VALUE
bsock_getsockname(VALUE sock)
{
union_sockaddr buf;
socklen_t len = (socklen_t)sizeof buf;
socklen_t len0 = len;
rb_io_t *fptr;

GetOpenFile(sock, fptr);
if (getsockname(fptr->fd, &buf.addr, &len) < 0)
rb_sys_fail("getsockname(2)");
if (len0 < len) len = len0;
return rb_str_new((char*)&buf, len);
}

















static VALUE
bsock_getpeername(VALUE sock)
{
union_sockaddr buf;
socklen_t len = (socklen_t)sizeof buf;
socklen_t len0 = len;
rb_io_t *fptr;

GetOpenFile(sock, fptr);
if (getpeername(fptr->fd, &buf.addr, &len) < 0)
rb_sys_fail("getpeername(2)");
if (len0 < len) len = len0;
return rb_str_new((char*)&buf, len);
}

#if defined(HAVE_GETPEEREID) || defined(SO_PEERCRED) || defined(HAVE_GETPEERUCRED)






















static VALUE
bsock_getpeereid(VALUE self)
{
#if defined(HAVE_GETPEEREID)
rb_io_t *fptr;
uid_t euid;
gid_t egid;
GetOpenFile(self, fptr);
if (getpeereid(fptr->fd, &euid, &egid) == -1)
rb_sys_fail("getpeereid(3)");
return rb_assoc_new(UIDT2NUM(euid), GIDT2NUM(egid));
#elif defined(SO_PEERCRED) 
rb_io_t *fptr;
struct ucred cred;
socklen_t len = sizeof(cred);
GetOpenFile(self, fptr);
if (getsockopt(fptr->fd, SOL_SOCKET, SO_PEERCRED, &cred, &len) == -1)
rb_sys_fail("getsockopt(SO_PEERCRED)");
return rb_assoc_new(UIDT2NUM(cred.uid), GIDT2NUM(cred.gid));
#elif defined(HAVE_GETPEERUCRED) 
rb_io_t *fptr;
ucred_t *uc = NULL;
VALUE ret;
GetOpenFile(self, fptr);
if (getpeerucred(fptr->fd, &uc) == -1)
rb_sys_fail("getpeerucred(3C)");
ret = rb_assoc_new(UIDT2NUM(ucred_geteuid(uc)), GIDT2NUM(ucred_getegid(uc)));
ucred_free(uc);
return ret;
#endif
}
#else
#define bsock_getpeereid rb_f_notimplement
#endif


















static VALUE
bsock_local_address(VALUE sock)
{
union_sockaddr buf;
socklen_t len = (socklen_t)sizeof buf;
socklen_t len0 = len;
rb_io_t *fptr;

GetOpenFile(sock, fptr);
if (getsockname(fptr->fd, &buf.addr, &len) < 0)
rb_sys_fail("getsockname(2)");
if (len0 < len) len = len0;
return rsock_fd_socket_addrinfo(fptr->fd, &buf.addr, len);
}




















static VALUE
bsock_remote_address(VALUE sock)
{
union_sockaddr buf;
socklen_t len = (socklen_t)sizeof buf;
socklen_t len0 = len;
rb_io_t *fptr;

GetOpenFile(sock, fptr);
if (getpeername(fptr->fd, &buf.addr, &len) < 0)
rb_sys_fail("getpeername(2)");
if (len0 < len) len = len0;
return rsock_fd_socket_addrinfo(fptr->fd, &buf.addr, len);
}


















VALUE
rsock_bsock_send(int argc, VALUE *argv, VALUE sock)
{
struct rsock_send_arg arg;
VALUE flags, to;
rb_io_t *fptr;
ssize_t n;
rb_blocking_function_t *func;
const char *funcname;

rb_scan_args(argc, argv, "21", &arg.mesg, &flags, &to);

StringValue(arg.mesg);
if (!NIL_P(to)) {
SockAddrStringValue(to);
to = rb_str_new4(to);
arg.to = (struct sockaddr *)RSTRING_PTR(to);
arg.tolen = RSTRING_SOCKLEN(to);
func = rsock_sendto_blocking;
funcname = "sendto(2)";
}
else {
func = rsock_send_blocking;
funcname = "send(2)";
}
GetOpenFile(sock, fptr);
arg.fd = fptr->fd;
arg.flags = NUM2INT(flags);
while (rsock_maybe_fd_writable(arg.fd),
(n = (ssize_t)BLOCKING_REGION_FD(func, &arg)) < 0) {
if (rb_io_wait_writable(arg.fd)) {
continue;
}
rb_sys_fail(funcname);
}
return SSIZET2NUM(n);
}


















static VALUE
bsock_do_not_reverse_lookup(VALUE sock)
{
rb_io_t *fptr;

GetOpenFile(sock, fptr);
return (fptr->mode & FMODE_NOREVLOOKUP) ? Qtrue : Qfalse;
}















static VALUE
bsock_do_not_reverse_lookup_set(VALUE sock, VALUE state)
{
rb_io_t *fptr;

GetOpenFile(sock, fptr);
if (RTEST(state)) {
fptr->mode |= FMODE_NOREVLOOKUP;
}
else {
fptr->mode &= ~FMODE_NOREVLOOKUP;
}
return sock;
}






















static VALUE
bsock_recv(int argc, VALUE *argv, VALUE sock)
{
return rsock_s_recvfrom(sock, argc, argv, RECV_RECV);
}


static VALUE
bsock_recv_nonblock(VALUE sock, VALUE len, VALUE flg, VALUE str, VALUE ex)
{
return rsock_s_recvfrom_nonblock(sock, len, flg, str, ex, RECV_RECV);
}









static VALUE
bsock_do_not_rev_lookup(VALUE _)
{
return rsock_do_not_reverse_lookup?Qtrue:Qfalse;
}

















static VALUE
bsock_do_not_rev_lookup_set(VALUE self, VALUE val)
{
rsock_do_not_reverse_lookup = RTEST(val);
return val;
}

void
rsock_init_basicsocket(void)
{





rb_cBasicSocket = rb_define_class("BasicSocket", rb_cIO);
rb_undef_method(rb_cBasicSocket, "initialize");

rb_define_singleton_method(rb_cBasicSocket, "do_not_reverse_lookup",
bsock_do_not_rev_lookup, 0);
rb_define_singleton_method(rb_cBasicSocket, "do_not_reverse_lookup=",
bsock_do_not_rev_lookup_set, 1);
rb_define_singleton_method(rb_cBasicSocket, "for_fd", bsock_s_for_fd, 1);

rb_define_method(rb_cBasicSocket, "close_read", bsock_close_read, 0);
rb_define_method(rb_cBasicSocket, "close_write", bsock_close_write, 0);
rb_define_method(rb_cBasicSocket, "shutdown", bsock_shutdown, -1);
rb_define_method(rb_cBasicSocket, "setsockopt", bsock_setsockopt, -1);
rb_define_method(rb_cBasicSocket, "getsockopt", bsock_getsockopt, 2);
rb_define_method(rb_cBasicSocket, "getsockname", bsock_getsockname, 0);
rb_define_method(rb_cBasicSocket, "getpeername", bsock_getpeername, 0);
rb_define_method(rb_cBasicSocket, "getpeereid", bsock_getpeereid, 0);
rb_define_method(rb_cBasicSocket, "local_address", bsock_local_address, 0);
rb_define_method(rb_cBasicSocket, "remote_address", bsock_remote_address, 0);
rb_define_method(rb_cBasicSocket, "send", rsock_bsock_send, -1);
rb_define_method(rb_cBasicSocket, "recv", bsock_recv, -1);

rb_define_method(rb_cBasicSocket, "do_not_reverse_lookup", bsock_do_not_reverse_lookup, 0);
rb_define_method(rb_cBasicSocket, "do_not_reverse_lookup=", bsock_do_not_reverse_lookup_set, 1);


rb_define_private_method(rb_cBasicSocket,
"__recv_nonblock", bsock_recv_nonblock, 4);

#if MSG_DONTWAIT_RELIABLE
rb_define_private_method(rb_cBasicSocket,
"__read_nonblock", rsock_read_nonblock, 3);
rb_define_private_method(rb_cBasicSocket,
"__write_nonblock", rsock_write_nonblock, 2);
#endif


rb_define_private_method(rb_cBasicSocket, "__sendmsg",
rsock_bsock_sendmsg, 4);
rb_define_private_method(rb_cBasicSocket, "__sendmsg_nonblock",
rsock_bsock_sendmsg_nonblock, 5);
rb_define_private_method(rb_cBasicSocket, "__recvmsg",
rsock_bsock_recvmsg, 4);
rb_define_private_method(rb_cBasicSocket, "__recvmsg_nonblock",
rsock_bsock_recvmsg_nonblock, 5);

}
