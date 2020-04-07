#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <unistd.h>
#include <vlc_network.h>
#include <vlc_interrupt.h>
static int SocksNegotiate( vlc_object_t *p_obj,
int fd, int i_socks_version,
const char *psz_socks_user,
const char *psz_socks_passwd )
{
uint8_t buffer[128+2*256];
int i_len;
bool b_auth = false;
if( i_socks_version != 5 )
return VLC_SUCCESS;
buffer[0] = i_socks_version; 
if( psz_socks_user != NULL && psz_socks_passwd != NULL )
{
buffer[1] = 2; 
buffer[2] = 0x00; 
buffer[3] = 0x02; 
i_len = 4;
b_auth = true;
}
else
{
buffer[1] = 1; 
buffer[2] = 0x00; 
i_len = 3;
}
if( net_Write( p_obj, fd, buffer, i_len ) != i_len )
return VLC_EGENERIC;
if( net_Read( p_obj, fd, buffer, 2) != 2 )
return VLC_EGENERIC;
msg_Dbg( p_obj, "socks: v=%d method=%x", buffer[0], buffer[1] );
if( buffer[1] == 0x00 )
{
msg_Dbg( p_obj, "socks: no authentication required" );
}
else if( buffer[1] == 0x02 )
{
if( psz_socks_user == NULL || psz_socks_passwd == NULL )
{
msg_Err( p_obj, "socks: server mandates authentication but "
"a username and/or password was not supplied" );
return VLC_EGENERIC;
}
int const i_user = strlen( psz_socks_user );
int const i_pasw = strlen( psz_socks_passwd );
if( i_user > 255 || i_pasw > 255 )
{
msg_Err( p_obj, "socks: rejecting username and/or password due to "
"violation of RFC1929 (longer than 255 bytes)" );
return VLC_EGENERIC;
}
msg_Dbg( p_obj, "socks: username/password authentication" );
buffer[0] = i_socks_version; 
buffer[1] = i_user; 
memcpy( &buffer[2], psz_socks_user, i_user );
buffer[2+i_user] = i_pasw; 
memcpy( &buffer[2+i_user+1], psz_socks_passwd, i_pasw );
i_len = 3 + i_user + i_pasw;
if( net_Write( p_obj, fd, buffer, i_len ) != i_len )
return VLC_EGENERIC;
if( net_Read( p_obj, fd, buffer, 2 ) != 2 )
return VLC_EGENERIC;
msg_Dbg( p_obj, "socks: v=%d status=%x", buffer[0], buffer[1] );
if( buffer[1] != 0x00 )
{
msg_Err( p_obj, "socks: authentication rejected" );
return VLC_EGENERIC;
}
}
else
{
if( b_auth )
msg_Err( p_obj, "socks: unsupported authentication method %x",
buffer[0] );
else
msg_Err( p_obj, "socks: authentication needed" );
return VLC_EGENERIC;
}
return VLC_SUCCESS;
}
static int SocksHandshakeTCP( vlc_object_t *p_obj,
int fd,
int i_socks_version,
const char *psz_user, const char *psz_passwd,
const char *psz_host, int i_port )
{
uint8_t buffer[128+2*256];
if( i_socks_version != 4 && i_socks_version != 5 )
{
msg_Warn( p_obj, "invalid socks protocol version %d", i_socks_version );
i_socks_version = 5;
}
if( i_socks_version == 5 &&
SocksNegotiate( p_obj, fd, i_socks_version,
psz_user, psz_passwd ) )
return VLC_EGENERIC;
if( i_socks_version == 4 )
{
static const struct addrinfo hints = {
.ai_family = AF_INET,
.ai_socktype = SOCK_STREAM,
.ai_protocol = IPPROTO_TCP,
.ai_flags = AI_IDN,
};
struct addrinfo *res;
if (vlc_getaddrinfo_i11e(psz_host, 0, &hints, &res))
return VLC_EGENERIC;
buffer[0] = i_socks_version;
buffer[1] = 0x01; 
SetWBE( &buffer[2], i_port ); 
memcpy (&buffer[4], 
&((struct sockaddr_in *)(res->ai_addr))->sin_addr, 4);
freeaddrinfo (res);
buffer[8] = 0; 
if( net_Write( p_obj, fd, buffer, 9 ) != 9 )
return VLC_EGENERIC;
if( net_Read( p_obj, fd, buffer, 8 ) != 8 )
return VLC_EGENERIC;
msg_Dbg( p_obj, "socks: v=%d cd=%d",
buffer[0], buffer[1] );
if( buffer[1] != 90 )
return VLC_EGENERIC;
}
else if( i_socks_version == 5 )
{
int i_hlen = __MIN(strlen( psz_host ), 255);
int i_len;
buffer[0] = i_socks_version; 
buffer[1] = 0x01; 
buffer[2] = 0x00; 
buffer[3] = 3; 
buffer[4] = i_hlen;
memcpy( &buffer[5], psz_host, i_hlen );
SetWBE( &buffer[5+i_hlen], i_port );
i_len = 5 + i_hlen + 2;
if( net_Write( p_obj, fd, buffer, i_len ) != i_len )
return VLC_EGENERIC;
if( net_Read( p_obj, fd, buffer, 5 ) != 5 )
return VLC_EGENERIC;
msg_Dbg( p_obj, "socks: v=%d rep=%d atyp=%d",
buffer[0], buffer[1], buffer[3] );
if( buffer[1] != 0x00 )
{
msg_Err( p_obj, "socks: CONNECT request failed" );
return VLC_EGENERIC;
}
if( buffer[3] == 0x01 )
i_len = 4-1 + 2;
else if( buffer[3] == 0x03 )
i_len = buffer[4] + 2;
else if( buffer[3] == 0x04 )
i_len = 16-1+2;
else
return VLC_EGENERIC;
if( net_Read( p_obj, fd, buffer, i_len ) != i_len )
return VLC_EGENERIC;
}
return VLC_SUCCESS;
}
int (net_ConnectTCP)(vlc_object_t *obj, const char *host, int serv)
{
const char *realhost;
int realserv;
char *socks = var_InheritString(obj, "socks");
if (socks != NULL)
{
realhost = socks;
char *p = strchr(socks, ':');
if (p != NULL)
{
*(p++) = '\0';
realserv = atoi(p);
}
else
realserv = 1080;
msg_Dbg(obj, "net: connecting to %s port %d (SOCKS) "
"for %s port %d", realhost, realserv, host, serv);
}
else
{
msg_Dbg(obj, "net: connecting to %s port %d", host, serv);
realhost = host;
realserv = serv;
}
int fd = net_Connect(obj, realhost, realserv, SOCK_STREAM, IPPROTO_TCP);
if (socks != NULL && fd != -1)
{
char *user = var_InheritString(obj, "socks-user");
char *pwd = var_InheritString(obj, "socks-pwd");
if (SocksHandshakeTCP(obj, fd, 5, user, pwd, host, serv))
{
msg_Err(obj, "SOCKS handshake failed");
net_Close(fd);
fd = -1;
}
free(pwd);
free(user);
}
return fd;
}
