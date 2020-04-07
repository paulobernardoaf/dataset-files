









#include "rubysocket.h"

#if defined(SOCKS)













static VALUE
socks_init(VALUE sock, VALUE host, VALUE port)
{
static int init = 0;

if (init == 0) {
SOCKSinit("ruby");
init = 1;
}

return rsock_init_inetsock(sock, host, port, Qnil, Qnil, INET_SOCKS);
}

#if defined(SOCKS5)




static VALUE
socks_s_close(VALUE sock)
{
rb_io_t *fptr;

GetOpenFile(sock, fptr);
shutdown(fptr->fd, 2);
return rb_io_close(sock);
}
#endif
#endif

void
rsock_init_sockssocket(void)
{
#if defined(SOCKS)







rb_cSOCKSSocket = rb_define_class("SOCKSSocket", rb_cTCPSocket);
rb_define_method(rb_cSOCKSSocket, "initialize", socks_init, 2);
#if defined(SOCKS5)
rb_define_method(rb_cSOCKSSocket, "close", socks_s_close, 0);
#endif
#endif
}
