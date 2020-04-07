#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <errno.h>
#include <ctype.h>
#include <assert.h>
#if defined(HAVE_ARPA_INET_H)
#include <arpa/inet.h>
#endif
#if defined(HAVE_SYS_SOCKET_H)
#include <sys/socket.h>
#endif
#include <vlc_common.h>
#include <vlc_demux.h>
#include <vlc_plugin.h>
#include <vlc_access.h>
#include <vlc_network.h>
#include <vlc_block.h>
#include <vlc_interrupt.h>
#include <vlc_url.h>
#if defined(HAVE_POLL)
#include <poll.h>
#endif
#if defined(HAVE_SYS_UIO_H)
#include <sys/uio.h>
#endif
#define BUFFER_TEXT N_("Receive buffer")
#define BUFFER_LONGTEXT N_("AMT receive buffer size (bytes)" )
#define TIMEOUT_TEXT N_("Native multicast timeout (sec)")
#define AMT_TIMEOUT_TEXT N_("AMT timeout (sec)")
#define AMT_RELAY_ADDRESS N_("AMT relay (IP address or FQDN)")
#define AMT_RELAY_ADDR_LONG N_("AMT relay anycast address, or specify the relay you want by address or fully qualified domain name")
#define AMT_DEFAULT_RELAY N_("amt-relay.m2icast.net")
#define MAC_LEN 6 
#define NONCE_LEN 4 
#define MSG_TYPE_LEN 1 
#define RELAY_QUERY_MSG_LEN 48 
#define RELAY_ADV_MSG_LEN 12 
#define IGMP_QUERY_LEN 24 
#define IGMP_REPORT_LEN 20
#define AMT_HDR_LEN 2 
#define IP_HDR_LEN 20 
#define IP_HDR_IGMP_LEN 24 
#define UDP_HDR_LEN 8 
#define AMT_REQUEST_MSG_LEN 9
#define AMT_DISCO_MSG_LEN 8
#define AMT_RELAY_DISCO 1 
#define AMT_RELAY_ADV 2 
#define AMT_REQUEST 3 
#define AMT_MEM_QUERY 4 
#define AMT_MEM_UPD 5 
#define AMT_MULT_DATA 6 
#define AMT_TEARDOWN 7 
#define AMT_IGMPV3_MEMBERSHIP_QUERY_TYPEID 0x11
#define AMT_IGMPV3_MEMBERSHIP_REPORT_TYPEID 0x22
#define AMT_IGMPV1_MEMBERSHIP_REPORT_TYPEID 0x12
#define AMT_IGMPV2_MEMBERSHIP_REPORT_TYPEID 0x16
#define AMT_IGMPV2_MEMBERSHIP_LEAVE_TYPEID 0x17
#define AMT_IGMP_INCLUDE 0x01
#define AMT_IGMP_EXCLUDE 0x02
#define AMT_IGMP_INCLUDE_CHANGE 0x03
#define AMT_IGMP_EXCLUDE_CHANGE 0x04
#define AMT_IGMP_ALLOW 0x05
#define AMT_IGMP_BLOCK 0x06
#define MCAST_ANYCAST "0.0.0.0"
#define MCAST_ALLHOSTS "224.0.0.22"
#define LOCAL_LOOPBACK "127.0.0.1"
#define AMT_PORT 2268
#define DEFAULT_MTU (1500u - (20 + 8))
typedef struct _amt_ip {
uint8_t ver_ihl;
uint8_t tos;
uint16_t tot_len;
uint16_t id;
uint16_t frag_off;
uint8_t ttl;
uint8_t protocol;
uint16_t check;
uint32_t srcAddr;
uint32_t destAddr;
} amt_ip_t;
typedef struct _amt_ip_alert {
uint8_t ver_ihl;
uint8_t tos;
uint16_t tot_len;
uint16_t id;
uint16_t frag_off;
uint8_t ttl;
uint8_t protocol;
uint16_t check;
uint32_t srcAddr;
uint32_t destAddr;
uint32_t options;
} amt_ip_alert_t;
typedef struct _amt_igmpv3_groupRecord {
uint8_t type;
uint8_t auxDatalen;
uint16_t nSrc;
uint32_t ssm;
uint32_t srcIP[1];
} amt_igmpv3_groupRecord_t;
typedef struct _amt_igmpv3_membership_report {
uint8_t type;
uint8_t resv;
uint16_t checksum;
uint16_t resv2;
uint16_t nGroupRecord;
amt_igmpv3_groupRecord_t grp[1];
} amt_igmpv3_membership_report_t;
typedef struct _amt_igmpv3_membership_query {
uint8_t type;
uint8_t max_resp_code; 
uint32_t checksum;
uint32_t ssmIP;
uint8_t s_qrv;
uint8_t qqic; 
uint16_t nSrc;
uint32_t srcIP[1];
} amt_igmpv3_membership_query_t;
typedef struct _amt_membership_update_msg {
amt_ip_alert_t ipHead;
amt_igmpv3_membership_report_t memReport;
} amt_membership_update_msg_t;
static int amt_sockets_init( stream_t *p_access );
static void amt_send_relay_discovery_msg( stream_t *p_access, char *relay_ip );
static void amt_send_relay_request( stream_t *p_access, char *relay_ip );
static int amt_joinSSM_group( stream_t *p_access );
static int amt_joinASM_group( stream_t *p_access );
static int amt_leaveASM_group( stream_t *p_access );
static int amt_leaveSSM_group( stream_t *p_access );
static bool amt_rcv_relay_adv( stream_t *p_access );
static bool amt_rcv_relay_mem_query( stream_t *p_access );
static void amt_send_mem_update( stream_t *p_access, char *relay_ip, bool leave );
static bool open_amt_tunnel( stream_t *p_access );
static void amt_update_timer_cb( void *data );
typedef struct _access_sys_t
{
char *relay;
char relayDisco[INET_ADDRSTRLEN];
vlc_timer_t updateTimer;
struct sockaddr_in mcastGroupAddr;
struct sockaddr_in mcastSrcAddr;
struct sockaddr_in relayDiscoAddr;
struct relay_mem_query_msg_t {
uint32_t ulRcvedNonce;
uint8_t type;
uint8_t uchaMAC[MAC_LEN];
uint8_t uchaIGMP[IGMP_QUERY_LEN];
} relay_mem_query_msg;
amt_igmpv3_membership_query_t relay_igmp_query;
size_t mtu;
uint32_t glob_ulNonce;
int fd;
int sAMT;
int sQuery;
int timeout;
bool tryAMT;
} access_sys_t;
static int Open (vlc_object_t *);
static void Close (vlc_object_t *);
static unsigned short get_checksum( unsigned short *buffer, int nLen );
static void make_report( amt_igmpv3_membership_report_t *mr );
static void make_ip_header( amt_ip_alert_t *p_ipHead );
vlc_module_begin ()
set_shortname( N_("AMT" ) )
set_description( N_("AMT input") )
set_category( CAT_INPUT )
set_subcategory( SUBCAT_INPUT_ACCESS )
add_integer( "amt-timeout", 5, AMT_TIMEOUT_TEXT, NULL, true )
add_integer( "amt-native-timeout", 5, TIMEOUT_TEXT, NULL, true )
add_string( "amt-relay", AMT_DEFAULT_RELAY, AMT_RELAY_ADDRESS, AMT_RELAY_ADDR_LONG, true )
set_capability( "access", 0 )
add_shortcut( "amt" )
set_callbacks( Open, Close )
vlc_module_end ()
static block_t *BlockAMT( stream_t *, bool * );
static int Control( stream_t *, int, va_list );
static int Open( vlc_object_t *p_this )
{
stream_t *p_access = (stream_t*) p_this;
access_sys_t *sys = NULL;
struct addrinfo hints, *serverinfo = NULL;
struct sockaddr_in *server_addr;
char *psz_name = NULL, *saveptr, *psz_strtok_r;
char mcastSrc_buf[INET_ADDRSTRLEN], mcastGroup_buf[INET_ADDRSTRLEN];
const char *mcastSrc, *mcastGroup;
int i_bind_port = 1234, i_server_port = 0, VLC_ret = VLC_SUCCESS, response;
vlc_url_t url = { 0 };
if( p_access->b_preparsing )
return VLC_EGENERIC;
ACCESS_SET_CALLBACKS( NULL, BlockAMT, Control, NULL );
if( !p_access->psz_location )
return VLC_EGENERIC;
sys = vlc_obj_calloc( p_this, 1, sizeof( *sys ) );
if( unlikely( sys == NULL ) )
return VLC_ENOMEM;
sys->mtu = 7 * 188;
p_access->p_sys = sys;
sys->fd = sys->sAMT = sys->sQuery = -1;
psz_name = strdup( p_access->psz_location );
if ( unlikely( psz_name == NULL ) )
{
VLC_ret = VLC_EGENERIC;
goto cleanup;
}
if( vlc_UrlParse( &url, p_access->psz_url ) != 0 )
{
msg_Err( p_access, "Invalid URL: %s", p_access->psz_url );
VLC_ret = VLC_EGENERIC;
goto cleanup;
}
if( url.i_port > 0 )
i_bind_port = url.i_port;
msg_Dbg( p_access, "Opening multicast: %s:%d local=%s:%d", url.psz_host, i_server_port, url.psz_path, i_bind_port );
memset( &hints, 0, sizeof( hints ));
hints.ai_family = AF_INET; 
hints.ai_socktype = SOCK_DGRAM;
response = vlc_getaddrinfo( url.psz_host, AMT_PORT, &hints, &serverinfo );
if( response != 0 )
{
msg_Err( p_access, "Could not find multicast group %s, reason: %s", url.psz_host, gai_strerror(response) );
VLC_ret = VLC_EGENERIC;
goto cleanup;
}
server_addr = (struct sockaddr_in *) serverinfo->ai_addr;
if( unlikely( inet_ntop(AF_INET, &(server_addr->sin_addr), mcastGroup_buf, INET_ADDRSTRLEN) == NULL ) )
{
int errConv = errno;
msg_Err(p_access, "Could not convert binary socket address to string: %s", gai_strerror(errConv));
goto cleanup;
}
mcastGroup = mcastGroup_buf;
sys->mcastGroupAddr = *server_addr;
freeaddrinfo( serverinfo );
serverinfo = NULL;
msg_Dbg( p_access, "Setting multicast group address to %s", mcastGroup);
psz_strtok_r = strtok_r( psz_name, "@", &saveptr );
if ( !psz_strtok_r )
{
msg_Err( p_access, "Could not parse location %s", psz_name);
VLC_ret = VLC_EGENERIC;
goto cleanup;
}
mcastSrc = psz_strtok_r;
if( strcmp( url.psz_host, mcastSrc ) == 0 )
{
mcastSrc = MCAST_ANYCAST;
sys->mcastSrcAddr.sin_addr.s_addr = 0;
msg_Dbg( p_access, "No multicast source address specified, trying ASM...");
}
response = vlc_getaddrinfo( mcastSrc, AMT_PORT, &hints, &serverinfo );
if( response != 0 )
{
msg_Err( p_access, "Could not find multicast source %s, reason: %s", mcastSrc, gai_strerror(response) );
VLC_ret = VLC_EGENERIC;
goto cleanup;
}
server_addr = (struct sockaddr_in *) serverinfo->ai_addr;
if( unlikely( inet_ntop(AF_INET, &(server_addr->sin_addr), mcastSrc_buf, INET_ADDRSTRLEN) == NULL ) )
{
int errConv = errno;
msg_Err(p_access, "Could not binary socket address to string: %s", gai_strerror(errConv));
goto cleanup;
}
mcastSrc = mcastSrc_buf;
sys->mcastSrcAddr = *server_addr;
msg_Dbg( p_access, "Setting multicast source address to %s", mcastSrc);
sys->relay = var_InheritString( p_access, "amt-relay" );
if( unlikely( sys->relay == NULL ) )
{
msg_Err( p_access, "No relay anycast or unicast address specified." );
VLC_ret = VLC_EGENERIC;
goto cleanup;
}
msg_Dbg( p_access, "Addresses: mcastGroup: %s mcastSrc: %s relay: %s", \
mcastGroup, mcastSrc, sys->relay);
sys->fd = net_OpenDgram( p_access, mcastGroup, i_bind_port,
mcastSrc, i_server_port, IPPROTO_UDP );
if( sys->fd == -1 )
{
VLC_ret = VLC_EGENERIC;
goto cleanup;
}
int ret = vlc_timer_create( &sys->updateTimer, amt_update_timer_cb, p_access );
if( ret != 0 )
{
VLC_ret = VLC_EGENERIC;
goto cleanup;
}
sys->timeout = var_InheritInteger( p_access, "amt-native-timeout");
if( sys->timeout > 0)
sys->timeout *= 1000;
sys->tryAMT = false;
cleanup: 
free( psz_name );
vlc_UrlClean( &url );
if( serverinfo )
freeaddrinfo( serverinfo );
if ( VLC_ret != VLC_SUCCESS )
{
free( sys->relay );
if( sys->fd != -1 )
net_Close( sys->fd );
}
return VLC_ret;
}
static void Close( vlc_object_t *p_this )
{
stream_t *p_access = (stream_t*)p_this;
access_sys_t *sys = p_access->p_sys;
vlc_timer_destroy( sys->updateTimer );
if ( sys->tryAMT )
{
if( sys->mcastSrcAddr.sin_addr.s_addr )
amt_leaveSSM_group( p_access );
else
amt_leaveASM_group( p_access );
amt_send_mem_update( p_access, sys->relayDisco, true );
}
free( sys->relay );
net_Close( sys->fd );
if( sys->sAMT != -1 )
net_Close( sys->sAMT );
if( sys->sQuery != -1 )
net_Close( sys->sQuery );
}
static int Control( stream_t *p_access, int i_query, va_list args )
{
switch( i_query )
{
case STREAM_CAN_SEEK:
case STREAM_CAN_FASTSEEK:
case STREAM_CAN_PAUSE:
case STREAM_CAN_CONTROL_PACE:
*va_arg( args, bool * ) = false;
break;
case STREAM_GET_PTS_DELAY:
*va_arg( args, vlc_tick_t * ) =
VLC_TICK_FROM_MS(var_InheritInteger( p_access, "network-caching" ));
break;
default:
return VLC_EGENERIC;
}
return VLC_SUCCESS;
}
static block_t *BlockAMT(stream_t *p_access, bool *restrict eof)
{
access_sys_t *sys = p_access->p_sys;
ssize_t len = 0, shift = 0, tunnel = IP_HDR_LEN + UDP_HDR_LEN + AMT_HDR_LEN;
block_t *pkt = block_Alloc( sys->mtu + tunnel );
if ( unlikely( pkt == NULL ) )
return NULL;
struct pollfd ufd[1];
if( sys->tryAMT )
ufd[0].fd = sys->sAMT; 
else
ufd[0].fd = sys->fd; 
ufd[0].events = POLLIN;
switch (vlc_poll_i11e(ufd, 1, sys->timeout))
{
case 0:
if( !sys->tryAMT )
{
msg_Err(p_access, "Native multicast receive time-out");
if( !open_amt_tunnel( p_access ) )
goto error;
break;
}
else
{
*eof = true;
}
case -1:
goto error;
}
if( sys->tryAMT )
{
len = recv( sys->sAMT, pkt->p_buffer, sys->mtu + tunnel, 0 );
if( len < 0 || *(pkt->p_buffer) != AMT_MULT_DATA )
goto error;
shift += tunnel;
if( len < tunnel )
{
msg_Err(p_access, "%zd bytes packet truncated (MTU was %zd)", len, sys->mtu);
pkt->i_flags |= BLOCK_FLAG_CORRUPTED;
}
else
{
len -= tunnel;
}
}
else
{
struct sockaddr temp;
socklen_t temp_size = sizeof( struct sockaddr );
len = recvfrom( sys->sAMT, (char *)pkt->p_buffer, sys->mtu + tunnel, 0, (struct sockaddr*)&temp, &temp_size );
}
pkt->p_buffer += shift;
pkt->i_buffer -= shift;
return pkt;
error:
block_Release( pkt );
return NULL;
}
static bool open_amt_tunnel( stream_t *p_access )
{
struct addrinfo hints, *serverinfo, *server;
access_sys_t *sys = p_access->p_sys;
memset( &hints, 0, sizeof( hints ));
hints.ai_family = AF_INET; 
hints.ai_socktype = SOCK_DGRAM;
msg_Dbg( p_access, "Attempting AMT to %s...", sys->relay);
sys->tryAMT = true;
int response = vlc_getaddrinfo( sys->relay, AMT_PORT, &hints, &serverinfo );
if( response != 0 )
{
msg_Err( p_access, "Could not find relay %s, reason: %s", sys->relay, gai_strerror(response) );
goto error;
}
for (server = serverinfo; server != NULL && !vlc_killed(); server = server->ai_next)
{
struct sockaddr_in *server_addr = (struct sockaddr_in *) server->ai_addr;
char relay_ip[INET_ADDRSTRLEN];
if( unlikely( inet_ntop(AF_INET, &(server_addr->sin_addr), relay_ip, INET_ADDRSTRLEN) == NULL ) )
{
int errConv = errno;
msg_Err(p_access, "Could not convert relay ip to binary representation: %s", gai_strerror(errConv));
goto error;
}
memcpy(sys->relayDisco, relay_ip, INET_ADDRSTRLEN);
if( unlikely( sys->relayDisco == NULL ) )
{
goto error;
}
msg_Dbg( p_access, "Trying AMT Server: %s", sys->relayDisco);
sys->relayDiscoAddr.sin_addr = server_addr->sin_addr;
if( amt_sockets_init( p_access ) != 0 )
continue; 
amt_send_relay_discovery_msg( p_access, relay_ip );
msg_Dbg( p_access, "Sent relay AMT discovery message to %s", relay_ip );
if( !amt_rcv_relay_adv( p_access ) )
{
msg_Err( p_access, "Error receiving AMT relay advertisement msg from %s, skipping", relay_ip );
goto error;
}
msg_Dbg( p_access, "Received AMT relay advertisement from %s", relay_ip );
amt_send_relay_request( p_access, relay_ip );
msg_Dbg( p_access, "Sent AMT relay request message to %s", relay_ip );
if( !amt_rcv_relay_mem_query( p_access ) )
{
msg_Err( p_access, "Could not receive AMT relay membership query from %s, reason: %s", relay_ip, vlc_strerror(errno));
goto error;
}
msg_Dbg( p_access, "Received AMT relay membership query from %s", relay_ip );
if( sys->mcastSrcAddr.sin_addr.s_addr )
{
if( amt_joinSSM_group( p_access ) != 0 )
{
msg_Err( p_access, "Error joining SSM %s", vlc_strerror(errno) );
goto error;
}
msg_Dbg( p_access, "Joined SSM" );
}
else {
if( amt_joinASM_group( p_access ) != 0 )
{
msg_Err( p_access, "Error joining ASM %s", vlc_strerror(errno) );
goto error;
}
msg_Dbg( p_access, "Joined ASM group" );
}
vlc_timer_disarm( sys->updateTimer );
amt_send_mem_update( p_access, sys->relayDisco, false );
bool eof=false;
block_t *pkt;
if ( !(pkt = BlockAMT( p_access, &eof )) )
{
msg_Err( p_access, "Unable to receive UDP packet from AMT relay %s for multicast group", relay_ip );
continue;
}
else
{
block_Release( pkt );
msg_Dbg( p_access, "Got UDP packet from multicast group via AMT relay %s, continuing...", relay_ip );
break; 
}
}
if (server == NULL)
{
msg_Err( p_access, "No AMT servers responding" );
goto error;
}
freeaddrinfo( serverinfo );
return true;
error:
vlc_timer_disarm( sys->updateTimer );
if( serverinfo )
freeaddrinfo( serverinfo );
return false;
}
static unsigned short get_checksum( unsigned short *buffer, int nLen )
{
int nleft = nLen;
int sum = 0;
unsigned short *w = buffer;
unsigned short answer = 0;
while (nleft > 1)
{
sum += *w++;
nleft -= 2;
}
if (nleft == 1)
{
*(unsigned char*)(&answer) = *(unsigned char*)w;
sum += answer;
}
sum = (sum >> 16) + (sum & 0xffff);
answer = ~sum;
return (answer);
}
static void make_report( amt_igmpv3_membership_report_t *mr )
{
mr->type = AMT_IGMPV3_MEMBERSHIP_REPORT_TYPEID;
mr->resv = 0;
mr->checksum = 0;
mr->resv2 = 0;
mr->nGroupRecord = htons(1);
}
static void make_ip_header( amt_ip_alert_t *p_ipHead )
{
p_ipHead->ver_ihl = 0x46;
p_ipHead->tos = 0xc0;
p_ipHead->tot_len = htons( IP_HDR_IGMP_LEN + IGMP_REPORT_LEN );
p_ipHead->id = 0x00;
p_ipHead->frag_off = 0x0000;
p_ipHead->ttl = 0x01;
p_ipHead->protocol = 0x02;
p_ipHead->check = 0;
p_ipHead->srcAddr = INADDR_ANY;
p_ipHead->options = 0x0000;
}
static int amt_sockets_init( stream_t *p_access )
{
struct sockaddr_in rcvAddr;
access_sys_t *sys = p_access->p_sys;
memset( &rcvAddr, 0, sizeof(struct sockaddr_in) );
int enable = 0, res = 0;
sys->relayDiscoAddr.sin_family = AF_INET;
sys->relayDiscoAddr.sin_port = htons( AMT_PORT );
sys->sAMT = vlc_socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP, true );
if( sys->sAMT == -1 )
{
msg_Err( p_access, "Failed to create UDP socket" );
goto error;
}
res = setsockopt(sys->sAMT, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
if(res < 0)
{
msg_Err( p_access, "Couldn't make socket reusable");
goto error;
}
rcvAddr.sin_family = AF_INET;
rcvAddr.sin_port = htons( 0 );
rcvAddr.sin_addr.s_addr = INADDR_ANY;
if( bind(sys->sAMT, (struct sockaddr *)&rcvAddr, sizeof(rcvAddr) ) != 0 )
{
msg_Err( p_access, "Failed to bind UDP socket error: %s", vlc_strerror(errno) );
goto error;
}
sys->sQuery = vlc_socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP, true );
if( sys->sQuery == -1 )
{
msg_Err( p_access, "Failed to create query socket" );
goto error;
}
struct sockaddr_in stLocalAddr =
{
.sin_family = AF_INET,
.sin_port = htons( 0 ),
.sin_addr.s_addr = INADDR_ANY,
};
if( bind(sys->sQuery, (struct sockaddr *)&stLocalAddr, sizeof(struct sockaddr) ) != 0 )
{
msg_Err( p_access, "Failed to bind query socket" );
goto error;
}
struct sockaddr_in stSvrAddr =
{
.sin_family = AF_INET,
.sin_port = htons( 9124 ),
};
res = inet_pton( AF_INET, LOCAL_LOOPBACK, &stSvrAddr.sin_addr );
if( res != 1 )
{
msg_Err( p_access, "Could not convert loopback address" );
goto error;
}
return 0;
error:
if( sys->sAMT != -1 )
{
net_Close( sys->sAMT );
sys->sAMT = -1;
}
if( sys->sQuery != -1 )
{
net_Close( sys->sQuery );
sys->sQuery = -1;
}
return -1;
}
static void amt_send_relay_discovery_msg( stream_t *p_access, char *relay_ip )
{
char chaSendBuffer[AMT_DISCO_MSG_LEN];
unsigned int ulNonce;
int nRet;
access_sys_t *sys = p_access->p_sys;
memset( chaSendBuffer, 0, sizeof(chaSendBuffer) );
ulNonce = 0;
nRet = 0;
chaSendBuffer[0] = AMT_RELAY_DISCO;
chaSendBuffer[1] = 0;
chaSendBuffer[2] = 0;
chaSendBuffer[3] = 0;
srand( (unsigned int)time(NULL) );
ulNonce = htonl( rand() );
memcpy( &chaSendBuffer[4], &ulNonce, sizeof(ulNonce) );
sys->glob_ulNonce = ulNonce;
nRet = sendto( sys->sAMT, chaSendBuffer, sizeof(chaSendBuffer), 0,\
(struct sockaddr *)&sys->relayDiscoAddr, sizeof(struct sockaddr) );
if( nRet < 0)
msg_Err( p_access, "Sendto failed to %s with error %d.", relay_ip, errno);
}
static void amt_send_relay_request( stream_t *p_access, char *relay_ip )
{
char chaSendBuffer[AMT_REQUEST_MSG_LEN];
uint32_t ulNonce;
int nRet;
access_sys_t *sys = p_access->p_sys;
memset( chaSendBuffer, 0, sizeof(chaSendBuffer) );
ulNonce = 0;
nRet = 0;
chaSendBuffer[0] = AMT_REQUEST;
chaSendBuffer[1] = 0;
chaSendBuffer[2] = 0;
chaSendBuffer[3] = 0;
ulNonce = sys->glob_ulNonce;
memcpy( &chaSendBuffer[4], &ulNonce, sizeof(uint32_t) );
nRet = send( sys->sAMT, chaSendBuffer, sizeof(chaSendBuffer), 0 );
if( nRet < 0 )
msg_Err(p_access, "Error sending relay request to %s error: %s", relay_ip, vlc_strerror(errno) );
}
static void amt_send_mem_update( stream_t *p_access, char *relay_ip, bool leave)
{
int sendBufSize = IP_HDR_IGMP_LEN + MAC_LEN + NONCE_LEN + AMT_HDR_LEN;
char pSendBuffer[ sendBufSize + IGMP_REPORT_LEN ];
uint32_t ulNonce = 0;
access_sys_t *sys = p_access->p_sys;
memset( pSendBuffer, 0, sizeof(pSendBuffer) );
pSendBuffer[0] = AMT_MEM_UPD;
memcpy( &pSendBuffer[2], sys->relay_mem_query_msg.uchaMAC, MAC_LEN );
ulNonce = sys->glob_ulNonce;
memcpy( &pSendBuffer[8], &ulNonce, NONCE_LEN );
amt_ip_alert_t p_ipHead;
memset( &p_ipHead, 0, IP_HDR_IGMP_LEN );
make_ip_header( &p_ipHead );
struct sockaddr_in temp;
int res = inet_pton( AF_INET, MCAST_ALLHOSTS, &(temp.sin_addr) );
if( res != 1 )
{
msg_Err(p_access, "Could not convert all hosts multicast address: %s", gai_strerror(errno) );
return;
}
p_ipHead.destAddr = temp.sin_addr.s_addr;
p_ipHead.check = get_checksum( (unsigned short*)&p_ipHead, IP_HDR_IGMP_LEN );
amt_igmpv3_groupRecord_t groupRcd;
groupRcd.auxDatalen = 0;
groupRcd.ssm = sys->mcastGroupAddr.sin_addr.s_addr;
if( sys->mcastSrcAddr.sin_addr.s_addr )
{
groupRcd.type = leave ? AMT_IGMP_BLOCK:AMT_IGMP_INCLUDE;
groupRcd.nSrc = htons(1);
groupRcd.srcIP[0] = sys->mcastSrcAddr.sin_addr.s_addr;
} else {
groupRcd.type = leave ? AMT_IGMP_INCLUDE_CHANGE:AMT_IGMP_EXCLUDE_CHANGE;
groupRcd.nSrc = htons(0);
}
amt_igmpv3_membership_report_t p_igmpMemRep;
make_report( &p_igmpMemRep );
memcpy(&p_igmpMemRep.grp[0], &groupRcd, (int)sizeof(groupRcd) );
p_igmpMemRep.checksum = get_checksum( (unsigned short*)&p_igmpMemRep, IGMP_REPORT_LEN );
amt_membership_update_msg_t memUpdateMsg;
memset(&memUpdateMsg, 0, sizeof(memUpdateMsg));
memcpy(&memUpdateMsg.ipHead, &p_ipHead, sizeof(p_ipHead) );
memcpy(&memUpdateMsg.memReport, &p_igmpMemRep, sizeof(p_igmpMemRep) );
memcpy( &pSendBuffer[12], &memUpdateMsg, sizeof(memUpdateMsg) );
send( sys->sAMT, pSendBuffer, sizeof(pSendBuffer), 0 );
msg_Dbg( p_access, "AMT relay membership report sent to %s", relay_ip );
}
static bool amt_rcv_relay_adv( stream_t *p_access )
{
char pkt[RELAY_ADV_MSG_LEN];
access_sys_t *sys = p_access->p_sys;
memset( pkt, 0, RELAY_ADV_MSG_LEN );
struct pollfd ufd[1];
ufd[0].fd = sys->sAMT;
ufd[0].events = POLLIN;
switch( vlc_poll_i11e(ufd, 1, sys->timeout) )
{
case 0:
msg_Err(p_access, "AMT relay advertisement receive time-out");
case -1:
return false;
}
struct sockaddr temp;
socklen_t temp_size = sizeof( struct sockaddr );
ssize_t len = recvfrom( sys->sAMT, pkt, RELAY_ADV_MSG_LEN, 0, &temp, &temp_size );
if (len < 0)
{
msg_Err(p_access, "Received message length less than zero");
return false;
}
struct {
uint32_t ulRcvNonce;
uint32_t ipAddr;
uint8_t type;
} relay_adv_msg;
memcpy( &relay_adv_msg.type, &pkt[0], MSG_TYPE_LEN );
if( relay_adv_msg.type != AMT_RELAY_ADV )
{
msg_Err( p_access, "Received message not an AMT relay advertisement, ignoring. ");
return false;
}
memcpy( &relay_adv_msg.ulRcvNonce, &pkt[NONCE_LEN], NONCE_LEN );
if( sys->glob_ulNonce != relay_adv_msg.ulRcvNonce )
{
msg_Err( p_access, "Discovery nonces differ! currNonce:%x rcvd%x", sys->glob_ulNonce, (uint32_t) ntohl(relay_adv_msg.ulRcvNonce) );
return false;
}
memcpy( &relay_adv_msg.ipAddr, &pkt[8], 4 );
struct sockaddr_in relayAddr =
{
.sin_family = AF_INET,
.sin_addr.s_addr = relay_adv_msg.ipAddr,
.sin_port = htons( AMT_PORT ),
};
int nRet = connect( sys->sAMT, (struct sockaddr *)&relayAddr, sizeof(relayAddr) );
if( nRet < 0 )
{
msg_Err( p_access, "Error connecting AMT UDP socket: %s", vlc_strerror(errno) );
return false;
}
return true;
}
static bool amt_rcv_relay_mem_query( stream_t *p_access )
{
char pkt[RELAY_QUERY_MSG_LEN];
memset( pkt, 0, RELAY_QUERY_MSG_LEN );
struct pollfd ufd[1];
access_sys_t *sys = p_access->p_sys;
ufd[0].fd = sys->sAMT;
ufd[0].events = POLLIN;
switch( vlc_poll_i11e(ufd, 1, sys->timeout) )
{
case 0:
msg_Err(p_access, "AMT relay membership query receive time-out");
case -1:
return false;
}
ssize_t len = recv( sys->sAMT, pkt, RELAY_QUERY_MSG_LEN, 0 );
if (len < 0 || len != RELAY_QUERY_MSG_LEN)
{
msg_Err(p_access, "length less than zero");
return false;
}
memcpy( &sys->relay_mem_query_msg.type, &pkt[0], MSG_TYPE_LEN );
memcpy( &sys->relay_mem_query_msg.uchaMAC[0], &pkt[AMT_HDR_LEN], MAC_LEN );
memcpy( &sys->relay_mem_query_msg.ulRcvedNonce, &pkt[AMT_HDR_LEN + MAC_LEN], NONCE_LEN );
if( sys->relay_mem_query_msg.ulRcvedNonce != sys->glob_ulNonce )
{
msg_Warn( p_access, "Nonces are different rcvd: %x glob: %x", sys->relay_mem_query_msg.ulRcvedNonce, sys->glob_ulNonce );
return false;
}
size_t shift = AMT_HDR_LEN + MAC_LEN + NONCE_LEN + IP_HDR_IGMP_LEN;
sys->relay_igmp_query.type = pkt[shift];
shift++; assert( shift < RELAY_QUERY_MSG_LEN);
sys->relay_igmp_query.max_resp_code = pkt[shift];
shift++; assert( shift < RELAY_QUERY_MSG_LEN);
memcpy( &sys->relay_igmp_query.checksum, &pkt[shift], 2 );
shift += 2; assert( shift < RELAY_QUERY_MSG_LEN);
memcpy( &sys->relay_igmp_query.ssmIP, &pkt[shift], 4 );
shift += 4; assert( shift < RELAY_QUERY_MSG_LEN);
sys->relay_igmp_query.s_qrv = pkt[shift];
shift++; assert( shift < RELAY_QUERY_MSG_LEN);
if( pkt[shift] == 0 )
sys->relay_igmp_query.qqic = 125;
else
sys->relay_igmp_query.qqic = pkt[shift];
shift++; assert( shift < RELAY_QUERY_MSG_LEN);
memcpy( &sys->relay_igmp_query.nSrc, &pkt[shift], 2 );
vlc_timer_schedule( sys->updateTimer, false,
VLC_TICK_FROM_SEC( sys->relay_igmp_query.qqic ), 0 );
return true;
}
static int amt_joinSSM_group( stream_t *p_access )
{
#if defined(IP_ADD_SOURCE_MEMBERSHIP)
struct ip_mreq_source imr;
access_sys_t *sys = p_access->p_sys;
imr.imr_multiaddr.s_addr = sys->mcastGroupAddr.sin_addr.s_addr;
imr.imr_sourceaddr.s_addr = sys->mcastSrcAddr.sin_addr.s_addr;
imr.imr_interface.s_addr = INADDR_ANY;
return setsockopt( sys->sAMT, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP, (char *)&imr, sizeof(imr) );
#else
errno = EINVAL;
return -1;
#endif
}
static int amt_joinASM_group( stream_t *p_access )
{
struct ip_mreq imr;
access_sys_t *sys = p_access->p_sys;
imr.imr_multiaddr.s_addr = sys->mcastGroupAddr.sin_addr.s_addr;
imr.imr_interface.s_addr = INADDR_ANY;
return setsockopt( sys->sAMT, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&imr, sizeof(imr) );
}
static int amt_leaveSSM_group( stream_t *p_access )
{
#if defined(IP_DROP_SOURCE_MEMBERSHIP)
struct ip_mreq_source imr;
access_sys_t *sys = p_access->p_sys;
imr.imr_multiaddr.s_addr = sys->mcastGroupAddr.sin_addr.s_addr;
imr.imr_sourceaddr.s_addr = sys->mcastSrcAddr.sin_addr.s_addr;
imr.imr_interface.s_addr = INADDR_ANY;
return setsockopt( sys->sAMT, IPPROTO_IP, IP_DROP_SOURCE_MEMBERSHIP, (char *)&imr, sizeof(imr) );
#else
errno = EINVAL;
return -1;
#endif
}
static int amt_leaveASM_group( stream_t *p_access )
{
struct ip_mreq imr;
access_sys_t *sys = p_access->p_sys;
imr.imr_multiaddr.s_addr = sys->mcastGroupAddr.sin_addr.s_addr;
imr.imr_interface.s_addr = INADDR_ANY;
return setsockopt( sys->sAMT, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char *)&imr, sizeof(imr) );
}
static void amt_update_timer_cb( void *data )
{
stream_t *p_access = (stream_t*) data;
access_sys_t *sys = p_access->p_sys;
amt_send_mem_update( p_access, sys->relayDisco, false );
vlc_timer_schedule( sys->updateTimer, false,
VLC_TICK_FROM_SEC( sys->relay_igmp_query.qqic ), 0 );
}
