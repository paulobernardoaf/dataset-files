#include "rubysocket.h"
static VALUE
tcp_svr_init(int argc, VALUE *argv, VALUE sock)
{
VALUE hostname, port;
rb_scan_args(argc, argv, "011", &hostname, &port);
return rsock_init_inetsock(sock, hostname, port, Qnil, Qnil, INET_SERVER);
}
static VALUE
tcp_accept(VALUE sock)
{
rb_io_t *fptr;
union_sockaddr from;
socklen_t fromlen;
GetOpenFile(sock, fptr);
fromlen = (socklen_t)sizeof(from);
return rsock_s_accept(rb_cTCPSocket, fptr->fd, &from.addr, &fromlen);
}
static VALUE
tcp_accept_nonblock(VALUE sock, VALUE ex)
{
rb_io_t *fptr;
union_sockaddr from;
socklen_t len = (socklen_t)sizeof(from);
GetOpenFile(sock, fptr);
return rsock_s_accept_nonblock(rb_cTCPSocket, ex, fptr, &from.addr, &len);
}
static VALUE
tcp_sysaccept(VALUE sock)
{
rb_io_t *fptr;
union_sockaddr from;
socklen_t fromlen;
GetOpenFile(sock, fptr);
fromlen = (socklen_t)sizeof(from);
return rsock_s_accept(0, fptr->fd, &from.addr, &fromlen);
}
void
rsock_init_tcpserver(void)
{
rb_cTCPServer = rb_define_class("TCPServer", rb_cTCPSocket);
rb_define_method(rb_cTCPServer, "accept", tcp_accept, 0);
rb_define_private_method(rb_cTCPServer,
"__accept_nonblock", tcp_accept_nonblock, 1);
rb_define_method(rb_cTCPServer, "sysaccept", tcp_sysaccept, 0);
rb_define_method(rb_cTCPServer, "initialize", tcp_svr_init, -1);
rb_define_method(rb_cTCPServer, "listen", rsock_sock_listen, 1); 
}
