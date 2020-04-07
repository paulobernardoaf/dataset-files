#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_access.h>
#include <vlc_network.h>
#include <vlc_block.h>
#include <vlc_interrupt.h>
#if defined(HAVE_POLL)
#include <poll.h>
#endif
#if defined(HAVE_SYS_UIO_H)
#include <sys/uio.h>
#endif
#define MRU 65507u
typedef struct {
int fd;
int timeout;
size_t length;
char *offset;
char buf[MRU];
} access_sys_t;
static int Control(stream_t *access, int query, va_list args)
{
switch (query) {
case STREAM_CAN_SEEK:
case STREAM_CAN_FASTSEEK:
case STREAM_CAN_PAUSE:
case STREAM_CAN_CONTROL_PACE:
*va_arg(args, bool *) = false;
break;
case STREAM_GET_PTS_DELAY:
*va_arg(args, vlc_tick_t *) =
VLC_TICK_FROM_MS(var_InheritInteger(access, "network-caching"));
break;
default:
return VLC_EGENERIC;
}
return VLC_SUCCESS;
}
static ssize_t Read(stream_t *access, void *buf, size_t len)
{
access_sys_t *sys = access->p_sys;
if (sys->length > 0) {
if (len > sys->length)
len = sys->length;
memcpy(buf, sys->offset, len);
sys->offset += len;
sys->length -= len;
return len;
}
struct pollfd ufd[1];
ufd[0].fd = sys->fd;
ufd[0].events = POLLIN;
switch (vlc_poll_i11e(ufd, 1, sys->timeout)) {
case 0:
msg_Err(access, "receive time-out");
return 0;
case -1:
return -1;
}
struct iovec iov[] = {
{ .iov_base = buf, .iov_len = len, },
{ .iov_base = sys->buf, .iov_len = MRU, },
};
struct msghdr msg = {
.msg_iov = iov,
.msg_iovlen = ARRAY_SIZE(iov),
};
ssize_t val = recvmsg(sys->fd, &msg, 0);
if (val <= 0) 
return -1;
if (unlikely((size_t)val > len)) {
sys->offset = sys->buf;
sys->length = val - len;
val = len;
}
return val;
}
static int Open( vlc_object_t *p_this )
{
stream_t *p_access = (stream_t*)p_this;
access_sys_t *sys;
if( p_access->b_preparsing )
return VLC_EGENERIC;
sys = vlc_obj_malloc( p_this, sizeof( *sys ) );
if( unlikely( sys == NULL ) )
return VLC_ENOMEM;
sys->length = 0;
p_access->p_sys = sys;
p_access->pf_read = Read;
p_access->pf_block = NULL;
p_access->pf_control = Control;
p_access->pf_seek = NULL;
char *psz_name = strdup( p_access->psz_location );
char *psz_parser;
const char *psz_server_addr, *psz_bind_addr = "";
int i_bind_port = 1234, i_server_port = 0;
if( unlikely(psz_name == NULL) )
return VLC_ENOMEM;
psz_parser = strchr( psz_name, '@' );
if( psz_parser != NULL )
{
*psz_parser++ = '\0';
psz_bind_addr = psz_parser;
if( psz_bind_addr[0] == '[' )
psz_parser = strchr( psz_parser, ']' );
if( psz_parser != NULL )
{
psz_parser = strchr( psz_parser, ':' );
if( psz_parser != NULL )
{
*psz_parser++ = '\0';
i_bind_port = atoi( psz_parser );
}
}
}
psz_server_addr = psz_name;
psz_parser = ( psz_server_addr[0] == '[' )
? strchr( psz_name, ']' ) 
: psz_name;
if( psz_parser != NULL )
{
psz_parser = strchr( psz_parser, ':' );
if( psz_parser != NULL )
{
*psz_parser++ = '\0';
i_server_port = atoi( psz_parser );
}
}
msg_Dbg( p_access, "opening server=%s:%d local=%s:%d",
psz_server_addr, i_server_port, psz_bind_addr, i_bind_port );
sys->fd = net_OpenDgram( p_access, psz_bind_addr, i_bind_port,
psz_server_addr, i_server_port, IPPROTO_UDP );
free( psz_name );
if( sys->fd == -1 )
{
msg_Err( p_access, "cannot open socket" );
return VLC_EGENERIC;
}
sys->timeout = var_InheritInteger( p_access, "udp-timeout");
if( sys->timeout > 0)
sys->timeout *= 1000;
return VLC_SUCCESS;
}
static void Close( vlc_object_t *p_this )
{
stream_t *p_access = (stream_t*)p_this;
access_sys_t *sys = p_access->p_sys;
net_Close( sys->fd );
}
#define TIMEOUT_TEXT N_("UDP Source timeout (sec)")
vlc_module_begin()
set_shortname(N_("UDP"))
set_description(N_("UDP input"))
set_category(CAT_INPUT)
set_subcategory(SUBCAT_INPUT_ACCESS)
add_obsolete_integer("server-port") 
add_obsolete_integer("udp-buffer") 
add_integer("udp-timeout", -1, TIMEOUT_TEXT, NULL, true)
set_capability("access", 0)
add_shortcut("udp", "udpstream", "udp4", "udp6")
set_callbacks(Open, Close)
vlc_module_end()
