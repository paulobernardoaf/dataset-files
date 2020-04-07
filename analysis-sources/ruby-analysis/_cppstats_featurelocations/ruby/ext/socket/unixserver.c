









#include "rubysocket.h"

#if defined(HAVE_SYS_UN_H)












static VALUE
unix_svr_init(VALUE sock, VALUE path)
{
return rsock_init_unixsock(sock, path, 1);
}


















static VALUE
unix_accept(VALUE sock)
{
rb_io_t *fptr;
struct sockaddr_un from;
socklen_t fromlen;

GetOpenFile(sock, fptr);
fromlen = (socklen_t)sizeof(struct sockaddr_un);
return rsock_s_accept(rb_cUNIXSocket, fptr->fd,
(struct sockaddr*)&from, &fromlen);
}


static VALUE
unix_accept_nonblock(VALUE sock, VALUE ex)
{
rb_io_t *fptr;
struct sockaddr_un from;
socklen_t fromlen;

GetOpenFile(sock, fptr);
fromlen = (socklen_t)sizeof(from);
return rsock_s_accept_nonblock(rb_cUNIXSocket, ex, fptr,
(struct sockaddr *)&from, &fromlen);
}



















static VALUE
unix_sysaccept(VALUE sock)
{
rb_io_t *fptr;
struct sockaddr_un from;
socklen_t fromlen;

GetOpenFile(sock, fptr);
fromlen = (socklen_t)sizeof(struct sockaddr_un);
return rsock_s_accept(0, fptr->fd, (struct sockaddr*)&from, &fromlen);
}

#endif

void
rsock_init_unixserver(void)
{
#if defined(HAVE_SYS_UN_H)






rb_cUNIXServer = rb_define_class("UNIXServer", rb_cUNIXSocket);
rb_define_method(rb_cUNIXServer, "initialize", unix_svr_init, 1);
rb_define_method(rb_cUNIXServer, "accept", unix_accept, 0);

rb_define_private_method(rb_cUNIXServer,
"__accept_nonblock", unix_accept_nonblock, 1);

rb_define_method(rb_cUNIXServer, "sysaccept", unix_sysaccept, 0);
rb_define_method(rb_cUNIXServer, "listen", rsock_sock_listen, 1); 
#endif
}
