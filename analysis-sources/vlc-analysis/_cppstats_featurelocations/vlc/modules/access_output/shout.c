









































#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_sout.h>
#include <vlc_block.h>
#include <vlc_url.h>

#include <shout/shout.h>




static int Open ( vlc_object_t * );
static void Close( vlc_object_t * );

#define SOUT_CFG_PREFIX "sout-shout-"

#define NAME_TEXT N_("Stream name")
#define NAME_LONGTEXT N_("Name to give to this stream/channel on the " "shoutcast/icecast server." )


#define DESCRIPTION_TEXT N_("Stream description")
#define DESCRIPTION_LONGTEXT N_("Description of the stream content or " "information about your channel." )


#define MP3_TEXT N_("Stream MP3")
#define MP3_LONGTEXT N_("You normally have to feed the shoutcast module " "with Ogg streams. It is also possible to stream " "MP3 instead, so you can forward MP3 streams to " "the shoutcast/icecast server." )








#define GENRE_TEXT N_("Genre description")
#define GENRE_LONGTEXT N_("Genre of the content." )

#define URL_TEXT N_("URL description")
#define URL_LONGTEXT N_("URL with information about the stream or your channel." )





#define BITRATE_TEXT N_("Bitrate")
#define BITRATE_LONGTEXT N_("Bitrate information of the transcoded stream." )

#define SAMPLERATE_TEXT N_("Samplerate")
#define SAMPLERATE_LONGTEXT N_("Samplerate information of the transcoded stream." )

#define CHANNELS_TEXT N_("Number of channels")
#define CHANNELS_LONGTEXT N_("Number of channels information of the transcoded stream." )

#define QUALITY_TEXT N_("Ogg Vorbis Quality")
#define QUALITY_LONGTEXT N_("Ogg Vorbis Quality information of the transcoded stream." )

#define PUBLIC_TEXT N_("Stream public")
#define PUBLIC_LONGTEXT N_("Make the server publicly available on the 'Yellow Pages' " "(directory listing of streams) on the icecast/shoutcast " "website. Requires the bitrate information specified for " "shoutcast. Requires Ogg streaming for icecast." )




vlc_module_begin ()
set_description( N_("IceCAST output") )
set_shortname( "Shoutcast" )
set_capability( "sout access", 0 )
set_category( CAT_SOUT )
set_subcategory( SUBCAT_SOUT_ACO )
add_shortcut( "shout" )
add_string( SOUT_CFG_PREFIX "name", "VLC media player - Live stream",
NAME_TEXT, NAME_LONGTEXT, false )
add_string( SOUT_CFG_PREFIX "description", "Live stream from VLC media player",
DESCRIPTION_TEXT, DESCRIPTION_LONGTEXT, false )
add_bool( SOUT_CFG_PREFIX "mp3", false,
MP3_TEXT, MP3_LONGTEXT, true )
add_string( SOUT_CFG_PREFIX "genre", "Alternative",
GENRE_TEXT, GENRE_LONGTEXT, false )
add_string( SOUT_CFG_PREFIX "url", "http://www.videolan.org/vlc",
URL_TEXT, URL_LONGTEXT, false )
add_string( SOUT_CFG_PREFIX "bitrate", "",
BITRATE_TEXT, BITRATE_LONGTEXT, false )
add_string( SOUT_CFG_PREFIX "samplerate", "",
SAMPLERATE_TEXT, SAMPLERATE_LONGTEXT, false )
add_string( SOUT_CFG_PREFIX "channels", "",
CHANNELS_TEXT, CHANNELS_LONGTEXT, false )
add_string( SOUT_CFG_PREFIX "quality", "",
QUALITY_TEXT, QUALITY_LONGTEXT, false )
add_bool( SOUT_CFG_PREFIX "public", false,
PUBLIC_TEXT, PUBLIC_LONGTEXT, true )
set_callbacks( Open, Close )
vlc_module_end ()




static const char *const ppsz_sout_options[] = {
"name", "description", "mp3", "genre", "url", "bitrate", "samplerate",
"channels", "quality", "public", NULL
};





static ssize_t Write( sout_access_out_t *, block_t * );
static int Control( sout_access_out_t *, int, va_list );

typedef struct
{
shout_t *p_shout;
} sout_access_out_sys_t;




static int Open( vlc_object_t *p_this )
{
sout_access_out_t *p_access = (sout_access_out_t*)p_this;
sout_access_out_sys_t *p_sys;
shout_t *p_shout;
long i_ret;
char *psz_val;

char *psz_name;
char *psz_description;
char *psz_genre;
char *psz_url;
vlc_url_t url;

config_ChainParse( p_access, SOUT_CFG_PREFIX, ppsz_sout_options, p_access->p_cfg );

vlc_UrlParse( &url , p_access->psz_path );
if( url.i_port <= 0 )
url.i_port = 8000;

if( url.psz_host == NULL )
{ 
vlc_UrlClean( &url );
if( unlikely(asprintf( &psz_url, "//%s", p_access->psz_path ) == -1) )
return VLC_ENOMEM;
vlc_UrlParse( &url, psz_url );
free( psz_url );
}

p_sys = p_access->p_sys = malloc( sizeof( sout_access_out_sys_t ) );
if( !p_sys )
{
vlc_UrlClean( &url );
return VLC_ENOMEM;
}

psz_name = var_GetNonEmptyString( p_access, SOUT_CFG_PREFIX "name" );
psz_description = var_GetNonEmptyString( p_access, SOUT_CFG_PREFIX "description" );
psz_genre = var_GetNonEmptyString( p_access, SOUT_CFG_PREFIX "genre" );
psz_url = var_GetNonEmptyString( p_access, SOUT_CFG_PREFIX "url" );

p_shout = p_sys->p_shout = shout_new();
if( !p_shout
|| shout_set_host( p_shout, url.psz_host ) != SHOUTERR_SUCCESS
|| shout_set_protocol( p_shout, SHOUT_PROTOCOL_ICY ) != SHOUTERR_SUCCESS
|| shout_set_port( p_shout, url.i_port ) != SHOUTERR_SUCCESS
|| shout_set_password( p_shout, url.psz_password ) != SHOUTERR_SUCCESS
|| shout_set_mount( p_shout, url.psz_path ) != SHOUTERR_SUCCESS
|| shout_set_user( p_shout, url.psz_username ) != SHOUTERR_SUCCESS
|| shout_set_agent( p_shout, "VLC media player " VERSION ) != SHOUTERR_SUCCESS
|| shout_set_name( p_shout, psz_name ) != SHOUTERR_SUCCESS
|| shout_set_description( p_shout, psz_description ) != SHOUTERR_SUCCESS
|| shout_set_genre( p_shout, psz_genre ) != SHOUTERR_SUCCESS
|| shout_set_url( p_shout, psz_url ) != SHOUTERR_SUCCESS

)
{
msg_Err( p_access, "failed to initialize shout streaming to %s:%i/%s",
url.psz_host, url.i_port, url.psz_path );

free( psz_name );
free( psz_description );
free( psz_genre );
free( psz_url );
goto error;
}

free( psz_name );
free( psz_description );
free( psz_genre );
free( psz_url );

i_ret = shout_set_format( p_shout, var_GetBool( p_access, SOUT_CFG_PREFIX "mp3" ) ?
SHOUT_FORMAT_MP3 : SHOUT_FORMAT_OGG );

if( i_ret != SHOUTERR_SUCCESS )
{
msg_Err( p_access, "failed to set the shoutcast streaming format" );
goto error;
}



psz_val = var_GetNonEmptyString( p_access, SOUT_CFG_PREFIX "bitrate" );
if( psz_val )
{
i_ret = shout_set_audio_info( p_shout, SHOUT_AI_BITRATE, psz_val );
free( psz_val );
if( i_ret != SHOUTERR_SUCCESS )
{
msg_Err( p_access, "failed to set the information about the bitrate" );
goto error;
}
}
else
{


msg_Warn( p_access, "no bitrate information specified (required for listing " \
"the server as public on the shoutcast website)" );
}





psz_val = var_GetNonEmptyString( p_access, SOUT_CFG_PREFIX "samplerate" );
if( psz_val )
{
i_ret = shout_set_audio_info( p_shout, SHOUT_AI_SAMPLERATE, psz_val );
free( psz_val );
if( i_ret != SHOUTERR_SUCCESS )
{
msg_Err( p_access, "failed to set the information about the samplerate" );
goto error;
}
}

psz_val = var_GetNonEmptyString( p_access, SOUT_CFG_PREFIX "channels" );
if( psz_val )
{
i_ret = shout_set_audio_info( p_shout, SHOUT_AI_CHANNELS, psz_val );
free( psz_val );
if( i_ret != SHOUTERR_SUCCESS )
{
msg_Err( p_access, "failed to set the information about the number of channels" );
goto error;
}
}

psz_val = var_GetNonEmptyString( p_access, SOUT_CFG_PREFIX "quality" );
if( psz_val )
{
i_ret = shout_set_audio_info( p_shout, SHOUT_AI_QUALITY, psz_val );
free( psz_val );
if( i_ret != SHOUTERR_SUCCESS )
{
msg_Err( p_access, "failed to set the information about Ogg Vorbis quality" );
goto error;
}
}

if( var_GetBool( p_access, SOUT_CFG_PREFIX "public" ) )
{
i_ret = shout_set_public( p_shout, 1 );
if( i_ret != SHOUTERR_SUCCESS )
{
msg_Err( p_access, "failed to set the server status setting to public" );
goto error;
}
}


i_ret = shout_get_connected( p_shout );
while ( i_ret != SHOUTERR_CONNECTED )
{

shout_close( p_shout );



i_ret = shout_set_protocol( p_shout, SHOUT_PROTOCOL_ICY );
if( i_ret != SHOUTERR_SUCCESS )
{
msg_Err( p_access, "failed to set the protocol to 'icy'" );
goto error;
}
i_ret = shout_open( p_shout );
if( i_ret == SHOUTERR_SUCCESS )
{
i_ret = SHOUTERR_CONNECTED;
msg_Dbg( p_access, "connected using 'icy' (shoutcast) protocol" );
}
else
{
msg_Warn( p_access, "failed to connect using 'icy' (shoutcast) protocol" );


shout_close( p_shout );


i_ret = shout_set_protocol( p_shout, SHOUT_PROTOCOL_HTTP );
if( i_ret != SHOUTERR_SUCCESS )
{
msg_Err( p_access, "failed to set the protocol to 'http'" );
goto error;
}
i_ret = shout_open( p_shout );
if( i_ret == SHOUTERR_SUCCESS )
{
i_ret = SHOUTERR_CONNECTED;
msg_Dbg( p_access, "connected using 'http' (icecast 2.x) protocol" );
}
else
msg_Warn( p_access, "failed to connect using 'http' (icecast 2.x) protocol " );
}








if ( i_ret != SHOUTERR_CONNECTED )
{
msg_Warn( p_access, "unable to establish connection, retrying..." );
vlc_tick_sleep( VLC_TICK_FROM_SEC(30) );
}
}

if( i_ret != SHOUTERR_CONNECTED )
{
msg_Err( p_access, "failed to open shout stream to %s:%i/%s: %s",
url.psz_host, url.i_port, url.psz_path, shout_get_error(p_shout) );
goto error;
}

p_access->pf_write = Write;
p_access->pf_control = Control;

msg_Dbg( p_access, "shout access output opened (%s@%s:%i/%s)",
url.psz_username, url.psz_host, url.i_port, url.psz_path );

vlc_UrlClean( &url );
return VLC_SUCCESS;

error:
if( p_sys->p_shout )
shout_free( p_sys->p_shout );
vlc_UrlClean( &url );
free( p_sys );
return VLC_EGENERIC;
}




static void Close( vlc_object_t * p_this )
{
sout_access_out_t *p_access = (sout_access_out_t*)p_this;
sout_access_out_sys_t *p_sys = p_access->p_sys;

if( p_sys->p_shout )
{
shout_close( p_sys->p_shout );
shout_free( p_sys->p_shout );
shout_shutdown();
}
free( p_sys );
msg_Dbg( p_access, "shout access output closed" );
}

static int Control( sout_access_out_t *p_access, int i_query, va_list args )
{
switch( i_query )
{
case ACCESS_OUT_CONTROLS_PACE:
{
bool *pb = va_arg( args, bool * );
*pb = strcmp( p_access->psz_access, "stream" );
break;
}

default:
return VLC_EGENERIC;
}
return VLC_SUCCESS;
}




static ssize_t Write( sout_access_out_t *p_access, block_t *p_buffer )
{
sout_access_out_sys_t *p_sys = p_access->p_sys;
size_t i_write = 0;

shout_sync( p_sys->p_shout );
while( p_buffer )
{
block_t *p_next = p_buffer->p_next;

if( shout_send( p_sys->p_shout, p_buffer->p_buffer, p_buffer->i_buffer )
== SHOUTERR_SUCCESS )
{
i_write += p_buffer->i_buffer;
}
else
{
msg_Err( p_access, "cannot write to stream: %s",
shout_get_error( p_sys->p_shout ) );





shout_close( p_sys->p_shout );
msg_Warn( p_access, "server unavailable? trying to reconnect..." );

if( shout_open( p_sys->p_shout ) == SHOUTERR_SUCCESS )
{
shout_sync( p_sys->p_shout );
msg_Warn( p_access, "reconnected to server" );
}
else
{
msg_Err( p_access, "failed to reconnect to server" );
block_ChainRelease( p_buffer );
return VLC_EGENERIC;
}

}
block_Release( p_buffer );



p_buffer = p_next;
}

return i_write;
}
