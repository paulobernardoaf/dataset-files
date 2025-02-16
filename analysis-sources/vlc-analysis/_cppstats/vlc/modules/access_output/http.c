#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <stdint.h>
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_sout.h>
#include <vlc_block.h>
#include <vlc_httpd.h>
static int Open ( vlc_object_t * );
static void Close( vlc_object_t * );
#define SOUT_CFG_PREFIX "sout-http-"
#define USER_TEXT N_("Username")
#define USER_LONGTEXT N_("Username that will be " "requested to access the stream." )
#define PASS_TEXT N_("Password")
#define PASS_LONGTEXT N_("Password that will be " "requested to access the stream." )
#define MIME_TEXT N_("Mime")
#define MIME_LONGTEXT N_("MIME returned by the server (autodetected " "if not specified)." )
#define METACUBE_TEXT N_("Metacube")
#define METACUBE_LONGTEXT N_("Use the Metacube protocol. Needed for streaming " "to the Cubemap reflector.")
vlc_module_begin ()
set_description( N_("HTTP stream output") )
set_capability( "sout access", 0 )
set_shortname( "HTTP" )
add_shortcut( "http", "https", "mmsh" )
set_category( CAT_SOUT )
set_subcategory( SUBCAT_SOUT_ACO )
add_string( SOUT_CFG_PREFIX "user", "",
USER_TEXT, USER_LONGTEXT, true )
add_password(SOUT_CFG_PREFIX "pwd", "", PASS_TEXT, PASS_LONGTEXT)
add_string( SOUT_CFG_PREFIX "mime", "",
MIME_TEXT, MIME_LONGTEXT, true )
add_bool( SOUT_CFG_PREFIX "metacube", false,
METACUBE_TEXT, METACUBE_LONGTEXT, true )
set_callbacks( Open, Close )
vlc_module_end ()
static const char *const ppsz_sout_options[] = {
"user", "pwd", "mime", "metacube", NULL
};
static ssize_t Write( sout_access_out_t *, block_t * );
static int Control( sout_access_out_t *, int, va_list );
typedef struct
{
httpd_host_t *p_httpd_host;
httpd_stream_t *p_httpd_stream;
int i_header_allocated;
int i_header_size;
uint8_t *p_header;
bool b_header_complete;
bool b_metacube;
bool b_has_keyframes;
} sout_access_out_sys_t;
static const uint8_t METACUBE2_SYNC[8] = {'c', 'u', 'b', 'e', '!', 'm', 'a', 'p'};
#define METACUBE_FLAGS_HEADER 0x1
#define METACUBE_FLAGS_NOT_SUITABLE_FOR_STREAM_START 0x2
struct metacube2_block_header
{
char sync[8]; 
uint32_t size; 
uint16_t flags; 
uint16_t csum; 
};
#define METACUBE2_CRC_POLYNOMIAL 0x8FDB
#define METACUBE2_CRC_START 0x1234
static uint16_t metacube2_compute_crc(const struct metacube2_block_header *hdr)
{
static const int data_len = sizeof(hdr->size) + sizeof(hdr->flags);
const uint8_t *data = (uint8_t *)&hdr->size;
uint16_t crc = METACUBE2_CRC_START;
for (int i = 0; i < data_len; ++i) {
uint8_t c = data[i];
for (int j = 0; j < 8; j++) {
int bit = crc & 0x8000;
crc = (crc << 1) | ((c >> (7 - j)) & 0x01);
if (bit) {
crc ^= METACUBE2_CRC_POLYNOMIAL;
}
}
}
for (int i = 0; i < 16; i++) {
int bit = crc & 0x8000;
crc = crc << 1;
if (bit) {
crc ^= METACUBE2_CRC_POLYNOMIAL;
}
}
return crc;
}
static int Open( vlc_object_t *p_this )
{
sout_access_out_t *p_access = (sout_access_out_t*)p_this;
sout_access_out_sys_t *p_sys;
char *psz_user;
char *psz_pwd;
char *psz_mime;
if( !( p_sys = p_access->p_sys =
malloc( sizeof( sout_access_out_sys_t ) ) ) )
return VLC_ENOMEM ;
config_ChainParse( p_access, SOUT_CFG_PREFIX, ppsz_sout_options, p_access->p_cfg );
const char *path = p_access->psz_path;
path += strcspn( path, "/" );
if( path > p_access->psz_path )
{
const char *port = strrchr( p_access->psz_path, ':' );
if( port != NULL && strchr( port, ']' ) != NULL )
port = NULL; 
if( port != p_access->psz_path )
{
int len = (port ? port : path) - p_access->psz_path;
msg_Warn( p_access, "\"%.*s\" HTTP host might be ignored in "
"multiple-host configurations, use at your own risks.",
len, p_access->psz_path );
msg_Info( p_access, "Consider passing --http-host=IP on the "
"command line instead." );
char host[len + 1];
strncpy( host, p_access->psz_path, len );
host[len] = '\0';
var_Create( p_access, "http-host", VLC_VAR_STRING );
var_SetString( p_access, "http-host", host );
}
if( port != NULL )
{
port++;
int bind_port = atoi( port );
if( bind_port > 0 )
{
const char *var = strcasecmp( p_access->psz_access, "https" )
? "http-port" : "https-port";
var_Create( p_access, var, VLC_VAR_INTEGER );
var_SetInteger( p_access, var, bind_port );
}
}
}
if( !*path )
path = "/";
if( p_access->psz_access && !strcmp( p_access->psz_access, "https" ) )
p_sys->p_httpd_host = vlc_https_HostNew( VLC_OBJECT(p_access) );
else
p_sys->p_httpd_host = vlc_http_HostNew( VLC_OBJECT(p_access) );
if( p_sys->p_httpd_host == NULL )
{
msg_Err( p_access, "cannot start HTTP server" );
free( p_sys );
return VLC_EGENERIC;
}
psz_user = var_GetNonEmptyString( p_access, SOUT_CFG_PREFIX "user" );
psz_pwd = var_GetNonEmptyString( p_access, SOUT_CFG_PREFIX "pwd" );
if( p_access->psz_access && !strcmp( p_access->psz_access, "mmsh" ) )
{
psz_mime = strdup( "video/x-ms-asf-stream" );
}
else
{
psz_mime = var_GetNonEmptyString( p_access, SOUT_CFG_PREFIX "mime" );
}
p_sys->b_metacube = var_GetBool( p_access, SOUT_CFG_PREFIX "metacube" );
p_sys->b_has_keyframes = false;
p_sys->p_httpd_stream =
httpd_StreamNew( p_sys->p_httpd_host, path, psz_mime,
psz_user, psz_pwd );
free( psz_user );
free( psz_pwd );
free( psz_mime );
if( p_sys->p_httpd_stream == NULL )
{
msg_Err( p_access, "cannot add stream %s", path );
httpd_HostDelete( p_sys->p_httpd_host );
free( p_sys );
return VLC_EGENERIC;
}
if( p_sys->b_metacube )
{
const httpd_header headers[] = {
{ (char *)"Content-encoding", (char *)"metacube" }
};
int err = httpd_StreamSetHTTPHeaders( p_sys->p_httpd_stream, headers,
ARRAY_SIZE(headers) );
if( err != VLC_SUCCESS )
{
free( p_sys );
return err;
}
}
p_sys->i_header_allocated = 1024;
p_sys->i_header_size = 0;
p_sys->p_header = xmalloc( p_sys->i_header_allocated );
p_sys->b_header_complete = false;
p_access->pf_write = Write;
p_access->pf_control = Control;
return VLC_SUCCESS;
}
static void Close( vlc_object_t * p_this )
{
sout_access_out_t *p_access = (sout_access_out_t*)p_this;
sout_access_out_sys_t *p_sys = p_access->p_sys;
httpd_StreamDelete( p_sys->p_httpd_stream );
httpd_HostDelete( p_sys->p_httpd_host );
free( p_sys->p_header );
msg_Dbg( p_access, "Close" );
free( p_sys );
}
static int Control( sout_access_out_t *p_access, int i_query, va_list args )
{
(void)p_access;
switch( i_query )
{
case ACCESS_OUT_CONTROLS_PACE:
*va_arg( args, bool * ) = false;
break;
default:
return VLC_EGENERIC;
}
return VLC_SUCCESS;
}
static ssize_t Write( sout_access_out_t *p_access, block_t *p_buffer )
{
sout_access_out_sys_t *p_sys = p_access->p_sys;
int i_err = 0;
int i_len = 0;
while( p_buffer )
{
block_t *p_next;
if( p_buffer->i_flags & BLOCK_FLAG_HEADER )
{
if( p_sys->b_header_complete )
{
p_sys->i_header_size = 0;
p_sys->b_header_complete = false;
}
if( (int)(p_buffer->i_buffer + p_sys->i_header_size) >
p_sys->i_header_allocated )
{
p_sys->i_header_allocated =
p_buffer->i_buffer + p_sys->i_header_size + 1024;
p_sys->p_header = xrealloc( p_sys->p_header,
p_sys->i_header_allocated );
}
memcpy( &p_sys->p_header[p_sys->i_header_size],
p_buffer->p_buffer,
p_buffer->i_buffer );
p_sys->i_header_size += p_buffer->i_buffer;
}
else if( !p_sys->b_header_complete )
{
p_sys->b_header_complete = true;
if ( p_sys->b_metacube )
{
struct metacube2_block_header hdr;
memcpy( hdr.sync, METACUBE2_SYNC, sizeof( METACUBE2_SYNC ) );
hdr.size = hton32( p_sys->i_header_size );
hdr.flags = hton16( METACUBE_FLAGS_HEADER );
hdr.csum = hton16( metacube2_compute_crc( &hdr ) );
int i_header_size = p_sys->i_header_size + sizeof( hdr );
block_t *p_hdr_block = block_Alloc( i_header_size );
if( p_hdr_block == NULL ) {
block_ChainRelease( p_buffer );
return VLC_ENOMEM;
}
p_hdr_block->i_flags = 0;
memcpy( p_hdr_block->p_buffer, &hdr, sizeof( hdr ) );
memcpy( p_hdr_block->p_buffer + sizeof( hdr ), p_sys->p_header, p_sys->i_header_size );
httpd_StreamHeader( p_sys->p_httpd_stream, p_hdr_block->p_buffer, p_hdr_block->i_buffer );
httpd_StreamSend( p_sys->p_httpd_stream, p_hdr_block );
block_Release( p_hdr_block );
}
else
{
httpd_StreamHeader( p_sys->p_httpd_stream, p_sys->p_header,
p_sys->i_header_size );
}
}
i_len += p_buffer->i_buffer;
if( p_buffer->i_flags & BLOCK_FLAG_TYPE_I )
{
p_sys->b_has_keyframes = true;
}
p_next = p_buffer->p_next;
if( p_sys->b_metacube )
{
if( p_buffer->i_flags & BLOCK_FLAG_HEADER ) {
block_Release( p_buffer );
p_buffer = p_next;
continue;
}
struct metacube2_block_header hdr;
memcpy( hdr.sync, METACUBE2_SYNC, sizeof( METACUBE2_SYNC ) );
hdr.size = hton32( p_buffer->i_buffer );
hdr.flags = hton16( 0 );
if( p_buffer->i_flags & BLOCK_FLAG_HEADER )
hdr.flags |= hton16( METACUBE_FLAGS_HEADER );
if( p_sys->b_has_keyframes && !( p_buffer->i_flags & BLOCK_FLAG_TYPE_I ) )
hdr.flags |= hton16( METACUBE_FLAGS_NOT_SUITABLE_FOR_STREAM_START );
hdr.csum = hton16( metacube2_compute_crc( &hdr ) );
p_buffer = block_Realloc( p_buffer, sizeof( hdr ), p_buffer->i_buffer );
if( p_buffer == NULL ) {
block_ChainRelease( p_next );
return VLC_ENOMEM;
}
memcpy( p_buffer->p_buffer, &hdr, sizeof( hdr ) );
}
i_err = httpd_StreamSend( p_sys->p_httpd_stream, p_buffer );
block_Release( p_buffer );
p_buffer = p_next;
if( i_err < 0 )
{
break;
}
}
if( i_err < 0 )
{
block_ChainRelease( p_buffer );
}
return( i_err < 0 ? VLC_EGENERIC : i_len );
}
