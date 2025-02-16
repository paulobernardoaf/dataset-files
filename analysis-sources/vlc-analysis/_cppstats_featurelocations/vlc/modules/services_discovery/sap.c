


























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#define VLC_MODULE_LICENSE VLC_LICENSE_GPL_2_PLUS
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <assert.h>

#include <vlc_demux.h>
#include <vlc_services_discovery.h>

#include <vlc_network.h>
#include <vlc_charset.h>

#include <errno.h>
#include <unistd.h>
#if defined(HAVE_ARPA_INET_H)
#include <arpa/inet.h>
#endif
#if defined(HAVE_POLL)
#include <poll.h>
#endif

#if defined(HAVE_ZLIB_H)
#include <zlib.h>
#endif

#if defined(HAVE_NET_IF_H)
#include <net/if.h>
#endif





#define MAX_LINE_LENGTH 256


#define SAP_PORT 9875

#define SAP_V4_GLOBAL_ADDRESS "224.2.127.254"

#define SAP_V4_ORG_ADDRESS "239.195.255.255"

#define SAP_V4_LOCAL_ADDRESS "239.255.255.255"

#define SAP_V4_LINK_ADDRESS "224.0.0.255"
#define ADD_SESSION 1




#define SAP_ADDR_TEXT N_( "SAP multicast address" )
#define SAP_ADDR_LONGTEXT N_( "The SAP module normally chooses itself the " "right addresses to listen to. However, you " "can specify a specific address." )


#define SAP_TIMEOUT_TEXT N_( "SAP timeout (seconds)" )
#define SAP_TIMEOUT_LONGTEXT N_( "Delay after which SAP items get deleted if no new announcement " "is received." )


#define SAP_PARSE_TEXT N_( "Try to parse the announce" )
#define SAP_PARSE_LONGTEXT N_( "This enables actual parsing of the announces by the SAP module. " "Otherwise, all announcements are parsed by the \"live555\" " "(RTP/RTSP) module." )



#define SAP_STRICT_TEXT N_( "SAP Strict mode" )
#define SAP_STRICT_LONGTEXT N_( "When this is set, the SAP parser will discard some non-compliant " "announcements." )




static int Open ( vlc_object_t * );
static void Close( vlc_object_t * );
static int OpenDemux ( vlc_object_t * );
static void CloseDemux ( vlc_object_t * );

VLC_SD_PROBE_HELPER("sap", N_("Network streams (SAP)"), SD_CAT_LAN)

vlc_module_begin ()
set_shortname( N_("SAP"))
set_description( N_("Network streams (SAP)") )
set_category( CAT_PLAYLIST )
set_subcategory( SUBCAT_PLAYLIST_SD )

add_string( "sap-addr", NULL,
SAP_ADDR_TEXT, SAP_ADDR_LONGTEXT, true )
add_obsolete_bool( "sap-ipv4" ) 
add_obsolete_bool( "sap-ipv6" ) 
add_integer( "sap-timeout", 1800,
SAP_TIMEOUT_TEXT, SAP_TIMEOUT_LONGTEXT, true )
add_bool( "sap-parse", true,
SAP_PARSE_TEXT,SAP_PARSE_LONGTEXT, true )
add_bool( "sap-strict", false,
SAP_STRICT_TEXT,SAP_STRICT_LONGTEXT, true )
add_obsolete_bool( "sap-timeshift" ) 

set_capability( "services_discovery", 0 )
set_callbacks( Open, Close )

VLC_SD_PROBE_SUBMODULE

add_submodule ()
set_description( N_("SDP Descriptions parser") )
add_shortcut( "sdp" )
set_capability( "demux", 51 )
set_callbacks( OpenDemux, CloseDemux )
vlc_module_end ()






typedef struct sdp_t sdp_t;
typedef struct attribute_t attribute_t;
typedef struct sap_announce_t sap_announce_t;


struct sdp_media_t
{
struct sdp_t *parent;
char *fmt;
struct sockaddr_storage addr;
socklen_t addrlen;
unsigned n_addr;
int i_attributes;
attribute_t **pp_attributes;
};



struct sdp_t
{
const char *psz_sdp;


char username[64];
uint64_t session_id;
uint64_t session_version;
unsigned orig_ip_version;
char orig_host[1024];


char *psz_sessionname;


char *psz_sessioninfo;



char *psz_uri;
int i_media_type;
unsigned rtcp_port;


int i_attributes;
attribute_t **pp_attributes;


unsigned mediac;
struct sdp_media_t *mediav;
};

struct attribute_t
{
const char *value;
char name[];
};

struct sap_announce_t
{
vlc_tick_t i_last;
vlc_tick_t i_period;
uint8_t i_period_trust;

uint16_t i_hash;
uint32_t i_source[4];


sdp_t *p_sdp;

input_item_t * p_item;
};

typedef struct
{
vlc_thread_t thread;


int i_fd;
int *pi_fd;


int i_announces;
struct sap_announce_t **pp_announces;


bool b_strict;
bool b_parse;

vlc_tick_t i_timeout;
} services_discovery_sys_t;

typedef struct
{
sdp_t *p_sdp;
} demux_sys_t;







static int Demux( demux_t *p_demux );
static int Control( demux_t *, int, va_list );
static void *Run ( void *p_sd );


static int ParseConnection( vlc_object_t *p_obj, sdp_t *p_sdp );
static int ParseSAP( services_discovery_t *p_sd, const uint8_t *p_buffer, size_t i_read );
static sdp_t *ParseSDP (vlc_object_t *p_sd, const char *psz_sdp);
static sap_announce_t *CreateAnnounce( services_discovery_t *, uint32_t *, uint16_t, sdp_t * );
static int RemoveAnnounce( services_discovery_t *p_sd, sap_announce_t *p_announce );


static inline attribute_t *MakeAttribute (const char *str);
static const char *GetAttribute (attribute_t **tab, unsigned n, const char *name);
static inline void FreeAttribute (attribute_t *a);
static const char *FindAttribute (const sdp_t *sdp, unsigned media,
const char *name);

static bool IsSameSession( sdp_t *p_sdp1, sdp_t *p_sdp2 );
static int InitSocket( services_discovery_t *p_sd, const char *psz_address, int i_port );
static int Decompress( const unsigned char *psz_src, unsigned char **_dst, int i_len );
static void FreeSDP( sdp_t *p_sdp );

static bool IsWellKnownPayload (int type)
{
switch (type)
{ 
case 0: 
case 3:
case 8: 
case 10: 
case 11: 
case 12:
case 14: 
case 32: 
case 33: 
return true;
}
return false;
}




static int Open( vlc_object_t *p_this )
{
services_discovery_t *p_sd = ( services_discovery_t* )p_this;
services_discovery_sys_t *p_sys = (services_discovery_sys_t *)
malloc( sizeof( services_discovery_sys_t ) );
if( !p_sys )
return VLC_ENOMEM;

p_sys->i_timeout = vlc_tick_from_sec(var_CreateGetInteger( p_sd, "sap-timeout" ));

p_sd->p_sys = p_sys;
p_sd->description = _("Network streams (SAP)");

p_sys->pi_fd = NULL;
p_sys->i_fd = 0;

p_sys->b_strict = var_CreateGetBool( p_sd, "sap-strict");
p_sys->b_parse = var_CreateGetBool( p_sd, "sap-parse" );

p_sys->i_announces = 0;
p_sys->pp_announces = NULL;

if (vlc_clone (&p_sys->thread, Run, p_sd, VLC_THREAD_PRIORITY_LOW))
{
free (p_sys);
return VLC_EGENERIC;
}

return VLC_SUCCESS;
}




static int OpenDemux( vlc_object_t *p_this )
{
demux_t *p_demux = (demux_t *)p_this;
const uint8_t *p_peek;
char *psz_sdp = NULL;
sdp_t *p_sdp = NULL;
int errval = VLC_EGENERIC;
size_t i_len;

if( !var_CreateGetBool( p_demux, "sap-parse" ) )
{

return VLC_EGENERIC;
}

assert( p_demux->s ); 


if( vlc_stream_Peek( p_demux->s, &p_peek, 7 ) < 7 )
return VLC_EGENERIC;

if( memcmp( p_peek, "v=0\r\no=", 7 ) && memcmp( p_peek, "v=0\no=", 6 ) )
return VLC_EGENERIC;


for( i_len = 0, psz_sdp = NULL; i_len < 65536; )
{
const int i_read_max = 1024;
char *psz_sdp_new = realloc( psz_sdp, i_len + i_read_max + 1 );
size_t i_read;
if( psz_sdp_new == NULL )
{
errval = VLC_ENOMEM;
goto error;
}
psz_sdp = psz_sdp_new;

i_read = vlc_stream_Read( p_demux->s, &psz_sdp[i_len], i_read_max );
if( (int)i_read < 0 )
{
msg_Err( p_demux, "cannot read SDP" );
goto error;
}
i_len += i_read;

psz_sdp[i_len] = '\0';

if( (int)i_read < i_read_max )
break; 
}

p_sdp = ParseSDP( VLC_OBJECT(p_demux), psz_sdp );

if( !p_sdp )
{
msg_Warn( p_demux, "invalid SDP");
goto error;
}

if( ParseConnection( VLC_OBJECT( p_demux ), p_sdp ) )
{
p_sdp->psz_uri = NULL;
}
if (!IsWellKnownPayload (p_sdp->i_media_type))
goto error;
if( p_sdp->psz_uri == NULL ) goto error;

demux_sys_t *p_sys = malloc( sizeof(*p_sys) );
if( unlikely(p_sys == NULL) )
goto error;
p_sys->p_sdp = p_sdp;
p_demux->p_sys = p_sys;
p_demux->pf_control = Control;
p_demux->pf_demux = Demux;

FREENULL( psz_sdp );
return VLC_SUCCESS;

error:
FREENULL( psz_sdp );
if( p_sdp ) FreeSDP( p_sdp );
return errval;
}




static void Close( vlc_object_t *p_this )
{
services_discovery_t *p_sd = ( services_discovery_t* )p_this;
services_discovery_sys_t *p_sys = p_sd->p_sys;
int i;

vlc_cancel (p_sys->thread);
vlc_join (p_sys->thread, NULL);

for( i = p_sys->i_fd-1 ; i >= 0 ; i-- )
{
net_Close( p_sys->pi_fd[i] );
}
FREENULL( p_sys->pi_fd );

for( i = p_sys->i_announces - 1; i>= 0; i-- )
{
RemoveAnnounce( p_sd, p_sys->pp_announces[i] );
}
FREENULL( p_sys->pp_announces );

free( p_sys );
}




static void CloseDemux( vlc_object_t *p_this )
{
demux_t *p_demux = (demux_t *)p_this;
demux_sys_t *sys = p_demux->p_sys;

if( sys->p_sdp )
FreeSDP( sys->p_sdp );
free( sys );
}






#define MAX_SAP_BUFFER 5000

static void *Run( void *data )
{
services_discovery_t *p_sd = data;
services_discovery_sys_t *p_sys = p_sd->p_sys;
char *psz_addr;
int timeout = -1;
int canc = vlc_savecancel ();







InitSocket( p_sd, SAP_V4_GLOBAL_ADDRESS, SAP_PORT );
InitSocket( p_sd, SAP_V4_ORG_ADDRESS, SAP_PORT );
InitSocket( p_sd, SAP_V4_LOCAL_ADDRESS, SAP_PORT );
InitSocket( p_sd, SAP_V4_LINK_ADDRESS, SAP_PORT );

char psz_address[NI_MAXNUMERICHOST] = "ff02::2:7ffe%";
#if !defined(_WIN32)
struct if_nameindex *l = if_nameindex ();
if (l != NULL)
{
char *ptr = strchr (psz_address, '%') + 1;
for (unsigned i = 0; l[i].if_index; i++)
{
strcpy (ptr, l[i].if_name);
InitSocket (p_sd, psz_address, SAP_PORT);
}
if_freenameindex (l);
}
#else




SOCKET s = socket(PF_INET6, SOCK_DGRAM, IPPROTO_UDP);
if( s != INVALID_SOCKET )
{
INTERFACE_INFO ifaces[10]; 
DWORD len = sizeof(ifaces);

if( SOCKET_ERROR != WSAIoctl(s, SIO_GET_INTERFACE_LIST, NULL, 0, &ifaces, len, &len, NULL, NULL) )
{
unsigned ifcount = len/sizeof(INTERFACE_INFO);
char *ptr = strchr (psz_address, '%') + 1;
for(unsigned i = 1; i<=ifcount; ++i )
{

sprintf(ptr, "%d", i);
}
}
closesocket(s);
}
#endif
*strchr (psz_address, '%') = '\0';

static const char ipv6_scopes[] = "1456789ABCDE";
for (const char *c_scope = ipv6_scopes; *c_scope; c_scope++)
{
psz_address[3] = *c_scope;
InitSocket( p_sd, psz_address, SAP_PORT );
}

psz_addr = var_CreateGetString( p_sd, "sap-addr" );
if( psz_addr && *psz_addr )
InitSocket( p_sd, psz_addr, SAP_PORT );
free( psz_addr );

if( p_sys->i_fd == 0 )
{
msg_Err( p_sd, "unable to listen on any address" );
return NULL;
}


for (;;)
{
vlc_restorecancel (canc);
unsigned n = p_sys->i_fd;
struct pollfd ufd[n];

for (unsigned i = 0; i < n; i++)
{
ufd[i].fd = p_sys->pi_fd[i];
ufd[i].events = POLLIN;
ufd[i].revents = 0;
}

int val = poll (ufd, n, timeout);
canc = vlc_savecancel ();
if (val > 0)
{
for (unsigned i = 0; i < n; i++)
{
if (ufd[i].revents)
{
uint8_t p_buffer[MAX_SAP_BUFFER+1];
ssize_t i_read;

i_read = recv (ufd[i].fd, p_buffer, MAX_SAP_BUFFER, 0);
if (i_read < 0)
msg_Warn (p_sd, "receive error: %s",
vlc_strerror_c(errno));
if (i_read > 6)
{

p_buffer[i_read] = '\0';
ParseSAP (p_sd, p_buffer, i_read);
}
}
}
}

vlc_tick_t now = vlc_tick_now();



timeout = 1000 * 60 * 60;


for( int i = 0; i < p_sys->i_announces; i++ )
{
sap_announce_t * p_announce = p_sys->pp_announces[i];
vlc_tick_t i_last_period = now - p_announce->i_last;




if( ( p_announce->i_period_trust > 5 && i_last_period > 10 * p_announce->i_period ) ||
i_last_period > p_sys->i_timeout )
{
RemoveAnnounce( p_sd, p_announce );
}
else
{

if( p_announce->i_period_trust > 5 )
timeout = __MIN(MS_FROM_VLC_TICK(10 * p_announce->i_period - i_last_period), timeout);
timeout = __MIN(MS_FROM_VLC_TICK(p_sys->i_timeout - i_last_period), timeout);
}
}

if( !p_sys->i_announces )
timeout = -1; 
else if( timeout < 200 )
timeout = 200; 
}
vlc_assert_unreachable ();
}





static int Demux( demux_t *p_demux )
{
demux_sys_t *p_sys = p_demux->p_sys;
sdp_t *p_sdp = p_sys->p_sdp;
input_item_t *p_parent_input = p_demux->p_input_item;

if( !p_parent_input )
{
msg_Err( p_demux, "parent input could not be found" );
return VLC_EGENERIC;
}

input_item_SetURI( p_parent_input, p_sdp->psz_uri );
input_item_SetName( p_parent_input, p_sdp->psz_sessionname );
if( p_sdp->rtcp_port )
{
char *rtcp;
if( asprintf( &rtcp, ":rtcp-port=%u", p_sdp->rtcp_port ) != -1 )
{
input_item_AddOption( p_parent_input, rtcp, VLC_INPUT_OPTION_TRUSTED );
free( rtcp );
}
}

vlc_mutex_lock( &p_parent_input->lock );

p_parent_input->i_type = ITEM_TYPE_STREAM;
p_parent_input->b_net = true;

vlc_mutex_unlock( &p_parent_input->lock );
return VLC_SUCCESS;
}

static int Control( demux_t *p_demux, int i_query, va_list args )
{
VLC_UNUSED(p_demux); VLC_UNUSED(i_query); VLC_UNUSED(args);
return VLC_EGENERIC;
}






static int ParseSAP( services_discovery_t *p_sd, const uint8_t *buf,
size_t len )
{
services_discovery_sys_t *p_sys = p_sd->p_sys;
const char *psz_sdp;
const uint8_t *end = buf + len;
sdp_t *p_sdp;
uint32_t i_source[4];

assert (buf[len] == '\0');

if (len < 4)
return VLC_EGENERIC;

uint8_t flags = buf[0];
uint8_t auth_len = buf[1];


if ((flags >> 5) != 1)
return VLC_EGENERIC;

bool b_ipv6 = (flags & 0x10) != 0;
bool b_need_delete = (flags & 0x04) != 0;

if (flags & 0x02)
{
msg_Dbg( p_sd, "encrypted packet, unsupported" );
return VLC_EGENERIC;
}

bool b_compressed = (flags & 0x01) != 0;

uint16_t i_hash = U16_AT (buf + 2);

if( p_sys->b_strict && i_hash == 0 )
{
msg_Dbg( p_sd, "strict mode, discarding announce with null id hash");
return VLC_EGENERIC;
}

buf += 4;
if( b_ipv6 )
{
for( int i = 0; i < 4; i++,buf+=4)
i_source[i] = U32_AT(buf);
}
else
{
memset(i_source, 0, sizeof(i_source));
i_source[3] = U32_AT(buf);
buf+=4;
}

buf += auth_len;
if (buf > end)
return VLC_EGENERIC;

uint8_t *decomp = NULL;
if( b_compressed )
{
int newsize = Decompress (buf, &decomp, end - buf);
if (newsize < 0)
{
msg_Dbg( p_sd, "decompression of SAP packet failed" );
return VLC_EGENERIC;
}

decomp = xrealloc (decomp, newsize + 1);
decomp[newsize] = '\0';
psz_sdp = (const char *)decomp;
len = newsize;
}
else
{
psz_sdp = (const char *)buf;
len = end - buf;
}


assert( psz_sdp[len] == '\0');



if (strncmp (psz_sdp, "v=0", 3))
{
size_t clen = strlen (psz_sdp) + 1;

if (strcmp (psz_sdp, "application/sdp"))
{
msg_Dbg (p_sd, "unsupported content type: %s", psz_sdp);
goto error;
}


if (len <= clen)
goto error;

len -= clen;
psz_sdp += clen;
}


p_sdp = ParseSDP( VLC_OBJECT(p_sd), psz_sdp );

if( p_sdp == NULL )
goto error;

p_sdp->psz_sdp = psz_sdp;



if( ParseConnection( VLC_OBJECT(p_sd), p_sdp ) )
p_sdp->psz_uri = NULL;


if( !IsWellKnownPayload( p_sdp->i_media_type ) || !p_sys->b_parse )
{
free( p_sdp->psz_uri );
if (asprintf( &p_sdp->psz_uri, "sdp://%s", p_sdp->psz_sdp ) == -1)
p_sdp->psz_uri = NULL;
}

if( p_sdp->psz_uri == NULL )
{
FreeSDP( p_sdp );
goto error;
}

for( int i = 0 ; i < p_sys->i_announces ; i++ )
{
sap_announce_t * p_announce = p_sys->pp_announces[i];

if( ( !i_hash && IsSameSession( p_announce->p_sdp, p_sdp ) )
|| ( i_hash && p_announce->i_hash == i_hash
&& !memcmp(p_announce->i_source, i_source, sizeof(i_source)) ) )
{









if( !b_need_delete )
{


if( p_announce->i_period_trust <= 5 )
p_announce->i_period_trust++;


vlc_tick_t now = vlc_tick_now();
p_announce->i_period = ( p_announce->i_period * (p_announce->i_period_trust-1) + (now - p_announce->i_last) ) / p_announce->i_period_trust;
p_announce->i_last = now;
}
FreeSDP( p_sdp );
free (decomp);
return VLC_SUCCESS;
}
}

CreateAnnounce( p_sd, i_source, i_hash, p_sdp );

free (decomp);
return VLC_SUCCESS;
error:
free (decomp);
return VLC_EGENERIC;
}

sap_announce_t *CreateAnnounce( services_discovery_t *p_sd, uint32_t *i_source, uint16_t i_hash,
sdp_t *p_sdp )
{
input_item_t *p_input;
const char *psz_value;
sap_announce_t *p_sap = (sap_announce_t *)malloc(
sizeof(sap_announce_t ) );
services_discovery_sys_t *p_sys;
if( p_sap == NULL )
return NULL;

p_sys = p_sd->p_sys;

p_sap->i_last = vlc_tick_now();
p_sap->i_period = 0;
p_sap->i_period_trust = 0;
p_sap->i_hash = i_hash;
memcpy (p_sap->i_source, i_source, sizeof(p_sap->i_source));
p_sap->p_sdp = p_sdp;


p_input = input_item_NewStream( p_sap->p_sdp->psz_uri, p_sdp->psz_sessionname,
INPUT_DURATION_INDEFINITE );
if( unlikely(p_input == NULL) )
{
free( p_sap );
return NULL;
}
p_sap->p_item = p_input;

vlc_meta_t *p_meta = vlc_meta_New();
if( likely(p_meta != NULL) )
{
vlc_meta_Set( p_meta, vlc_meta_Description, p_sdp->psz_sessioninfo );
p_input->p_meta = p_meta;
}

if( p_sdp->rtcp_port )
{
char *rtcp;
if( asprintf( &rtcp, ":rtcp-port=%u", p_sdp->rtcp_port ) != -1 )
{
input_item_AddOption( p_input, rtcp, VLC_INPUT_OPTION_TRUSTED );
free( rtcp );
}
}

psz_value = GetAttribute( p_sap->p_sdp->pp_attributes, p_sap->p_sdp->i_attributes, "tool" );
if( psz_value != NULL )
{
input_item_AddInfo( p_input, _("Session"), _("Tool"), "%s", psz_value );
}
if( strcmp( p_sdp->username, "-" ) )
{
input_item_AddInfo( p_input, _("Session"), _("User"), "%s",
p_sdp->username );
}


psz_value = GetAttribute(p_sap->p_sdp->pp_attributes,
p_sap->p_sdp->i_attributes, "cat");
if (psz_value != NULL)
{


char *str = strdup(psz_value);
if (likely(str != NULL))
for (char *p = strchr(str, '.'); p != NULL; p = strchr(p, '.'))
*(p++) = '|';
services_discovery_AddItemCat(p_sd, p_input, str ? str : psz_value);
free(str);
}
else
{

psz_value = GetAttribute(p_sap->p_sdp->pp_attributes,
p_sap->p_sdp->i_attributes, "x-plgroup");
services_discovery_AddItemCat(p_sd, p_input, psz_value);
}

TAB_APPEND( p_sys->i_announces, p_sys->pp_announces, p_sap );

return p_sap;
}


static const char *FindAttribute (const sdp_t *sdp, unsigned media,
const char *name)
{

const char *attr = GetAttribute (sdp->mediav[media].pp_attributes,
sdp->mediav[media].i_attributes, name);
if (attr == NULL)
attr = GetAttribute (sdp->pp_attributes, sdp->i_attributes, name);
return attr;
}



static int ParseConnection( vlc_object_t *p_obj, sdp_t *p_sdp )
{
if (p_sdp->mediac == 0)
{
msg_Dbg (p_obj, "Ignoring SDP with no media");
return VLC_EGENERIC;
}

for (unsigned i = 1; i < p_sdp->mediac; i++)
{
if ((p_sdp->mediav[i].n_addr != p_sdp->mediav->n_addr)
|| (p_sdp->mediav[i].addrlen != p_sdp->mediav->addrlen)
|| memcmp (&p_sdp->mediav[i].addr, &p_sdp->mediav->addr,
p_sdp->mediav->addrlen))
{
msg_Dbg (p_obj, "Multiple media ports not supported -> live555");
return VLC_EGENERIC;
}
}

if (p_sdp->mediav->n_addr != 1)
{
msg_Dbg (p_obj, "Layered encoding not supported -> live555");
return VLC_EGENERIC;
}

char psz_uri[1026];
const char *host;
int port;

psz_uri[0] = '[';
if (vlc_getnameinfo ((struct sockaddr *)&(p_sdp->mediav->addr),
p_sdp->mediav->addrlen, psz_uri + 1,
sizeof (psz_uri) - 2, &port, NI_NUMERICHOST))
return VLC_EGENERIC;

if (strchr (psz_uri + 1, ':'))
{
host = psz_uri;
strcat (psz_uri, "]");
}
else
host = psz_uri + 1;


char *sdp_proto = strdup (p_sdp->mediav[0].fmt);
if (sdp_proto == NULL)
return VLC_ENOMEM;

char *subtype = strchr (sdp_proto, ' ');
if (subtype == NULL)
{
msg_Dbg (p_obj, "missing SDP media subtype: %s", sdp_proto);
free (sdp_proto);
return VLC_EGENERIC;
}
else
{
*subtype++ = '\0';


if (!strcasecmp (sdp_proto, "udp"))
p_sdp->i_media_type = 33;
else
p_sdp->i_media_type = atoi (subtype);
}



static const char proto_match[] =
"udp\0" "udp\0\0"
"RTP/AVP\0" "rtp\0\0"
"UDPLite/RTP/AVP\0" "udplite\0\0"
"DCCP/RTP/AVP\0" "dccp\0\1"
"TCP/RTP/AVP\0" "rtptcp\0\1"
"\0";

const char *vlc_proto = NULL;
uint8_t flags = 0;
for (const char *proto = proto_match; *proto;)
{
if (strcasecmp (proto, sdp_proto) == 0)
{
vlc_proto = proto + strlen (proto) + 1;
flags = vlc_proto[strlen (vlc_proto) + 1];
break;
}
proto += strlen (proto) + 1;
proto += strlen (proto) + 2;
}

free (sdp_proto);
if (vlc_proto == NULL)
{
msg_Dbg (p_obj, "unknown SDP media protocol: %s",
p_sdp->mediav[0].fmt);
return VLC_EGENERIC;
}

if (!strcmp (vlc_proto, "udp") || FindAttribute (p_sdp, 0, "rtcp-mux"))
p_sdp->rtcp_port = 0;
else
{
const char *rtcp = FindAttribute (p_sdp, 0, "rtcp");
if (rtcp)
p_sdp->rtcp_port = atoi (rtcp);
else
if (port & 1) 
p_sdp->rtcp_port = port++;
else 
p_sdp->rtcp_port = port + 1;
}

if (flags & 1)
{

const char *setup = FindAttribute (p_sdp, 0, "setup");
if (setup == NULL)
setup = "active"; 

if (strcmp (setup, "actpass") && strcmp (setup, "passive"))
{
msg_Dbg (p_obj, "unsupported COMEDIA mode: %s", setup);
return VLC_EGENERIC;
}

if (asprintf (&p_sdp->psz_uri, "%s://%s:%d", vlc_proto,
host, port) == -1)
return VLC_ENOMEM;
}
else
{

char psz_source[258] = "";
const char *sfilter = FindAttribute (p_sdp, 0, "source-filter");
if (sfilter != NULL)
{
char psz_source_ip[256];
unsigned ipv;

if (sscanf (sfilter, " incl IN IP%u %*s %255s ", &ipv,
psz_source_ip) == 2)
{


switch (ipv)
{
#if defined(AF_INET6)
case 6:
{
struct in6_addr addr;
if ((inet_pton (AF_INET6, psz_source_ip, &addr) > 0)
&& (inet_ntop (AF_INET6, &addr, psz_source + 1,
sizeof (psz_source) - 2) != NULL))
{
psz_source[0] = '[';
psz_source[strlen (psz_source)] = ']';
}
break;
}
#endif
case 4:
{
struct in_addr addr;
if ((inet_pton (AF_INET, psz_source_ip, &addr) > 0)
&& (inet_ntop (AF_INET, &addr, psz_source,
sizeof (psz_source)) == NULL))
*psz_source = '\0';
break;
}
}
}
}

if (asprintf (&p_sdp->psz_uri, "%s://%s@%s:%i", vlc_proto, psz_source,
host, port) == -1)
return VLC_ENOMEM;
}

return VLC_SUCCESS;
}


static int ParseSDPConnection (const char *str, struct sockaddr_storage *addr,
socklen_t *addrlen, unsigned *number)
{
char host[60];
unsigned fam, n1, n2;

int res = sscanf (str, "IN IP%u %59[^/]/%u/%u", &fam, host, &n1, &n2);
if (res < 2)
return -1;

switch (fam)
{
#if defined(AF_INET6)
case 6:
addr->ss_family = AF_INET6;
#if defined(HAVE_SA_LEN)
addr->ss_len =
#endif
*addrlen = sizeof (struct sockaddr_in6);

if (inet_pton (AF_INET6, host,
&((struct sockaddr_in6 *)addr)->sin6_addr) <= 0)
return -1;

*number = (res >= 3) ? n1 : 1;
break;
#endif

case 4:
addr->ss_family = AF_INET;
#if defined(HAVE_SA_LEN)
addr->ss_len =
#endif
*addrlen = sizeof (struct sockaddr_in);

if (inet_pton (AF_INET, host,
&((struct sockaddr_in *)addr)->sin_addr) <= 0)
return -1;

*number = (res >= 4) ? n2 : 1;
break;

default:
return -1;
}
return 0;
}

static void net_SetPort(struct sockaddr *addr, uint16_t port)
{
switch (addr->sa_family)
{
#if defined(AF_INET6)
case AF_INET6:
((struct sockaddr_in6 *)addr)->sin6_port = port;
break;
#endif
case AF_INET:
((struct sockaddr_in *)addr)->sin_port = port;
break;
}
}






static sdp_t *ParseSDP (vlc_object_t *p_obj, const char *psz_sdp)
{
if( psz_sdp == NULL )
return NULL;

sdp_t *p_sdp = calloc (1, sizeof (*p_sdp));
if (p_sdp == NULL)
return NULL;

char expect = 'V';
struct sockaddr_storage glob_addr;
memset (&glob_addr, 0, sizeof (glob_addr));
socklen_t glob_len = 0;
unsigned glob_count = 1;
int port = 0;


while (*psz_sdp)
{

size_t linelen = strcspn(psz_sdp, "\n");
if (psz_sdp[linelen] == '\0')
goto error;

char line[linelen + 1];
memcpy (line, psz_sdp, linelen);
line[linelen] = '\0';

psz_sdp += linelen + 1;


char *eol = strchr (line, '\r');
if (eol != NULL)
{
linelen = eol - line;
line[linelen] = '\0';
}


char cat = line[0], *data = line + 2;
if (!cat || (strchr ("vosiuepcbtrzkam", cat) == NULL))
{

msg_Dbg (p_obj, "unknown SDP line type: 0x%02x", (int)cat);
goto error;
}
if (line[1] != '=')
{
msg_Dbg (p_obj, "invalid SDP line: %s", line);
goto error;
}

assert (linelen >= 2);




switch (expect)
{

case 'V':
expect = 'O';
if (cat != 'v')
{
msg_Dbg (p_obj, "missing SDP version");
goto error;
}
if (strcmp (data, "0"))
{
msg_Dbg (p_obj, "unknown SDP version: %s", data);
goto error;
}
break;

case 'O':
expect = 'S';
if (cat != 'o')
{
msg_Dbg (p_obj, "missing SDP originator");
goto error;
}

if ((sscanf (data, "%63s %"SCNu64" %"SCNu64" IN IP%u %1023s",
p_sdp->username, &p_sdp->session_id,
&p_sdp->session_version, &p_sdp->orig_ip_version,
p_sdp->orig_host) != 5)
|| ((p_sdp->orig_ip_version != 4)
&& (p_sdp->orig_ip_version != 6)))
{
msg_Dbg (p_obj, "SDP origin not supported: %s", data);

goto error;
}
EnsureUTF8 (p_sdp->orig_host);
break;

case 'S':
expect = 'I';
if ((cat != 's') || !*data)
{

msg_Dbg (p_obj, "missing SDP session name");
goto error;
}
assert (p_sdp->psz_sessionname == NULL); 
p_sdp->psz_sessionname = strdup (data);
if (p_sdp->psz_sessionname == NULL)
goto error;
EnsureUTF8 (p_sdp->psz_sessionname);
break;

case 'I':
expect = 'U';

if (cat == 'i')
{
assert (p_sdp->psz_sessioninfo == NULL);
p_sdp->psz_sessioninfo = strdup (data);
if (p_sdp->psz_sessioninfo == NULL)
goto error;
EnsureUTF8 (p_sdp->psz_sessioninfo);
break;
}


case 'U':
expect = 'E';
if (cat == 'u')
break;

case 'E':
expect = 'E';
if (cat == 'e')
break;

case 'P':
expect = 'P';
if (cat == 'p')
break;

case 'C':
expect = 'B';
if (cat == 'c')
{
if (ParseSDPConnection (data, &glob_addr, &glob_len,
&glob_count))
{
msg_Dbg (p_obj, "SDP connection infos not supported: "
"%s", data);
goto error;
}
break;
}

case 'B':
assert (expect == 'B');
if (cat == 'b')
break;

case 'T':
expect = 'R';
if (cat != 't')
{
msg_Dbg (p_obj, "missing SDP time description");
goto error;
}
break;

case 'R':
if ((cat == 't') || (cat == 'r'))
break;

case 'Z':
expect = 'K';
if (cat == 'z')
break;

case 'K':
expect = 'A';
if (cat == 'k')
break;

case 'A':

if (cat == 'a')
{
attribute_t *p_attr = MakeAttribute (data);
TAB_APPEND( p_sdp->i_attributes, p_sdp->pp_attributes, p_attr );
break;
}



case 'm':
media:
{
expect = 'i';
if (cat != 'm')
{
msg_Dbg (p_obj, "missing SDP media description");
goto error;
}
struct sdp_media_t *m;
m = realloc (p_sdp->mediav, (p_sdp->mediac + 1) * sizeof (*m));
if (m == NULL)
goto error;

p_sdp->mediav = m;
m += p_sdp->mediac;
p_sdp->mediac++;

memset (m, 0, sizeof (*m));
memcpy (&m->addr, &glob_addr, m->addrlen = glob_len);
m->n_addr = glob_count;


data = strchr (data, ' ');
if (data == NULL)
{
msg_Dbg (p_obj, "missing SDP media port");
goto error;
}
port = atoi (++data);
if (port <= 0 || port >= 65536)
{
msg_Dbg (p_obj, "invalid transport port %d", port);
goto error;
}
net_SetPort ((struct sockaddr *)&m->addr, htons (port));

data = strchr (data, ' ');
if (data == NULL)
{
msg_Dbg (p_obj, "missing SDP media format");
goto error;
}
m->fmt = strdup (++data);
if (m->fmt == NULL)
goto error;

break;
}

case 'i':
expect = 'c';
if (cat == 'i')
break;

case 'c':
expect = 'b';
if (cat == 'c')
{
struct sdp_media_t *m = p_sdp->mediav + p_sdp->mediac - 1;
if (ParseSDPConnection (data, &m->addr, &m->addrlen,
&m->n_addr))
{
msg_Dbg (p_obj, "SDP connection infos not supported: "
"%s", data);
goto error;
}
net_SetPort ((struct sockaddr *)&m->addr, htons (port));
break;
}

case 'b':
expect = 'b';
if (cat == 'b')
break;

case 'k':
expect = 'a';
if (cat == 'k')
break;

case 'a':
assert (expect == 'a');
if (cat == 'a')
{
attribute_t *p_attr = MakeAttribute (data);
if (p_attr == NULL)
goto error;

TAB_APPEND (p_sdp->mediav[p_sdp->mediac - 1].i_attributes,
p_sdp->mediav[p_sdp->mediac - 1].pp_attributes, p_attr);
break;
}

if (cat == 'm')
goto media;

msg_Dbg (p_obj, "unexpected SDP line: 0x%02x", (int)cat);
goto error;

default:
msg_Err (p_obj, "*** BUG in SDP parser! ***");
goto error;
}
}

return p_sdp;

error:
FreeSDP (p_sdp);
return NULL;
}

static int InitSocket( services_discovery_t *p_sd, const char *psz_address,
int i_port )
{
int i_fd = net_ListenUDP1 ((vlc_object_t *)p_sd, psz_address, i_port);
if (i_fd == -1)
return VLC_EGENERIC;

shutdown( i_fd, SHUT_WR );
services_discovery_sys_t *p_sys = p_sd->p_sys;
TAB_APPEND(p_sys->i_fd, p_sys->pi_fd, i_fd);
return VLC_SUCCESS;
}

static int Decompress( const unsigned char *psz_src, unsigned char **_dst, int i_len )
{
#if defined(HAVE_ZLIB_H)
int i_result, i_dstsize, n = 0;
unsigned char *psz_dst = NULL;
z_stream d_stream;

memset (&d_stream, 0, sizeof (d_stream));

i_result = inflateInit(&d_stream);
if( i_result != Z_OK )
return( -1 );

d_stream.next_in = (Bytef *)psz_src;
d_stream.avail_in = i_len;

do
{
n++;
psz_dst = xrealloc( psz_dst, n * 1000 );
d_stream.next_out = (Bytef *)&psz_dst[(n - 1) * 1000];
d_stream.avail_out = 1000;

i_result = inflate(&d_stream, Z_NO_FLUSH);
if( ( i_result != Z_OK ) && ( i_result != Z_STREAM_END ) )
{
inflateEnd( &d_stream );
free( psz_dst );
return( -1 );
}
}
while( ( d_stream.avail_out == 0 ) && ( d_stream.avail_in != 0 ) &&
( i_result != Z_STREAM_END ) );

i_dstsize = d_stream.total_out;
inflateEnd( &d_stream );

*_dst = xrealloc( psz_dst, i_dstsize );

return i_dstsize;
#else
(void)psz_src;
(void)_dst;
(void)i_len;
return -1;
#endif
}


static void FreeSDP( sdp_t *p_sdp )
{
free( p_sdp->psz_sessionname );
free( p_sdp->psz_sessioninfo );
free( p_sdp->psz_uri );

for (unsigned j = 0; j < p_sdp->mediac; j++)
{
free (p_sdp->mediav[j].fmt);
for (int i = 0; i < p_sdp->mediav[j].i_attributes; i++)
FreeAttribute (p_sdp->mediav[j].pp_attributes[i]);
free (p_sdp->mediav[j].pp_attributes);
}
free (p_sdp->mediav);

for (int i = 0; i < p_sdp->i_attributes; i++)
FreeAttribute (p_sdp->pp_attributes[i]);

free (p_sdp->pp_attributes);
free (p_sdp);
}

static int RemoveAnnounce( services_discovery_t *p_sd,
sap_announce_t *p_announce )
{
if( p_announce->p_sdp )
{
FreeSDP( p_announce->p_sdp );
p_announce->p_sdp = NULL;
}

if( p_announce->p_item )
{
services_discovery_RemoveItem( p_sd, p_announce->p_item );
input_item_Release( p_announce->p_item );
p_announce->p_item = NULL;
}

services_discovery_sys_t *p_sys = p_sd->p_sys;
TAB_REMOVE(p_sys->i_announces, p_sys->pp_announces, p_announce);
free( p_announce );

return VLC_SUCCESS;
}




static bool IsSameSession( sdp_t *p_sdp1, sdp_t *p_sdp2 )
{







if (strcmp (p_sdp1->username, p_sdp2->username)
|| (p_sdp1->session_id != p_sdp2->session_id)
|| (p_sdp1->orig_ip_version != p_sdp2->orig_ip_version)
|| strcmp (p_sdp1->orig_host, p_sdp2->orig_host))
return false;

return true;
}

static inline attribute_t *MakeAttribute (const char *str)
{
attribute_t *a = malloc (sizeof (*a) + strlen (str) + 1);
if (a == NULL)
return NULL;

strcpy (a->name, str);
EnsureUTF8 (a->name);
char *value = strchr (a->name, ':');
if (value != NULL)
{
*value++ = '\0';
a->value = value;
}
else
a->value = "";
return a;
}


static const char *GetAttribute (attribute_t **tab, unsigned n,
const char *name)
{
for (unsigned i = 0; i < n; i++)
if (strcasecmp (tab[i]->name, name) == 0)
return tab[i]->value;
return NULL;
}


static inline void FreeAttribute (attribute_t *a)
{
free (a);
}
