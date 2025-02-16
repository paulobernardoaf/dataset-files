









#include "rubysocket.h"
















static VALUE
udp_init(int argc, VALUE *argv, VALUE sock)
{
VALUE arg;
int family = AF_INET;
int fd;

if (rb_scan_args(argc, argv, "01", &arg) == 1) {
family = rsock_family_arg(arg);
}
fd = rsock_socket(family, SOCK_DGRAM, 0);
if (fd < 0) {
rb_sys_fail("socket(2) - udp");
}

return rsock_init_sock(sock, fd);
}

struct udp_arg
{
struct rb_addrinfo *res;
rb_io_t *fptr;
};

static VALUE
udp_connect_internal(VALUE v)
{
struct udp_arg *arg = (void *)v;
rb_io_t *fptr;
int fd;
struct addrinfo *res;

rb_io_check_closed(fptr = arg->fptr);
fd = fptr->fd;
for (res = arg->res->ai; res; res = res->ai_next) {
if (rsock_connect(fd, res->ai_addr, res->ai_addrlen, 0) >= 0) {
return Qtrue;
}
}
return Qfalse;
}

















static VALUE
udp_connect(VALUE sock, VALUE host, VALUE port)
{
struct udp_arg arg;
VALUE ret;

GetOpenFile(sock, arg.fptr);
arg.res = rsock_addrinfo(host, port, rsock_fd_family(arg.fptr->fd), SOCK_DGRAM, 0);
ret = rb_ensure(udp_connect_internal, (VALUE)&arg,
rsock_freeaddrinfo, (VALUE)arg.res);
if (!ret) rsock_sys_fail_host_port("connect(2)", host, port);
return INT2FIX(0);
}

static VALUE
udp_bind_internal(VALUE v)
{
struct udp_arg *arg = (void *)v;
rb_io_t *fptr;
int fd;
struct addrinfo *res;

rb_io_check_closed(fptr = arg->fptr);
fd = fptr->fd;
for (res = arg->res->ai; res; res = res->ai_next) {
if (bind(fd, res->ai_addr, res->ai_addrlen) < 0) {
continue;
}
return Qtrue;
}
return Qfalse;
}













static VALUE
udp_bind(VALUE sock, VALUE host, VALUE port)
{
struct udp_arg arg;
VALUE ret;

GetOpenFile(sock, arg.fptr);
arg.res = rsock_addrinfo(host, port, rsock_fd_family(arg.fptr->fd), SOCK_DGRAM, 0);
ret = rb_ensure(udp_bind_internal, (VALUE)&arg,
rsock_freeaddrinfo, (VALUE)arg.res);
if (!ret) rsock_sys_fail_host_port("bind(2)", host, port);
return INT2FIX(0);
}

struct udp_send_arg {
struct rb_addrinfo *res;
rb_io_t *fptr;
struct rsock_send_arg sarg;
};

static VALUE
udp_send_internal(VALUE v)
{
struct udp_send_arg *arg = (void *)v;
rb_io_t *fptr;
int n;
struct addrinfo *res;

rb_io_check_closed(fptr = arg->fptr);
for (res = arg->res->ai; res; res = res->ai_next) {
retry:
arg->sarg.fd = fptr->fd;
arg->sarg.to = res->ai_addr;
arg->sarg.tolen = res->ai_addrlen;
rsock_maybe_fd_writable(arg->sarg.fd);
n = (int)BLOCKING_REGION_FD(rsock_sendto_blocking, &arg->sarg);
if (n >= 0) {
return INT2FIX(n);
}
if (rb_io_wait_writable(fptr->fd)) {
goto retry;
}
}
return Qfalse;
}























static VALUE
udp_send(int argc, VALUE *argv, VALUE sock)
{
VALUE flags, host, port;
struct udp_send_arg arg;
VALUE ret;

if (argc == 2 || argc == 3) {
return rsock_bsock_send(argc, argv, sock);
}
rb_scan_args(argc, argv, "4", &arg.sarg.mesg, &flags, &host, &port);

StringValue(arg.sarg.mesg);
GetOpenFile(sock, arg.fptr);
arg.sarg.fd = arg.fptr->fd;
arg.sarg.flags = NUM2INT(flags);
arg.res = rsock_addrinfo(host, port, rsock_fd_family(arg.fptr->fd), SOCK_DGRAM, 0);
ret = rb_ensure(udp_send_internal, (VALUE)&arg,
rsock_freeaddrinfo, (VALUE)arg.res);
if (!ret) rsock_sys_fail_host_port("sendto(2)", host, port);
return ret;
}


static VALUE
udp_recvfrom_nonblock(VALUE sock, VALUE len, VALUE flg, VALUE str, VALUE ex)
{
return rsock_s_recvfrom_nonblock(sock, len, flg, str, ex, RECV_IP);
}

void
rsock_init_udpsocket(void)
{






rb_cUDPSocket = rb_define_class("UDPSocket", rb_cIPSocket);
rb_define_method(rb_cUDPSocket, "initialize", udp_init, -1);
rb_define_method(rb_cUDPSocket, "connect", udp_connect, 2);
rb_define_method(rb_cUDPSocket, "bind", udp_bind, 2);
rb_define_method(rb_cUDPSocket, "send", udp_send, -1);


rb_define_private_method(rb_cUDPSocket,
"__recvfrom_nonblock", udp_recvfrom_nonblock, 4);
}
