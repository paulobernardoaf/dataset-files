
























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_access.h>
#include <vlc_url.h>
#include <assert.h>

#include "playlist.h"




static int ReadDVD( stream_t *, input_item_node_t * );
static int ReadDVD_VR( stream_t *, input_item_node_t * );

static const char *StreamLocation( const stream_t *s )
{
return s->psz_filepath ? s->psz_filepath : s->psz_url;
}




int Import_IFO( vlc_object_t *p_this )
{
stream_t *p_stream = (stream_t *)p_this;

CHECK_FILE(p_stream);

if( !stream_HasExtension( p_stream, ".IFO" ) )
return VLC_EGENERIC;

const char *psz_location = StreamLocation( p_stream );
if( psz_location == NULL )
return VLC_EGENERIC;

size_t len = strlen( psz_location );
if( len < 12 )
return VLC_EGENERIC;

const char *psz_probe;
const char *psz_file = &psz_location[len - 12];




if( !strncasecmp( psz_file, "VIDEO_TS", 8 ) ||
!strncasecmp( psz_file, "VTS_", 4 ) )
{
psz_probe = "DVDVIDEO";
p_stream->pf_readdir = ReadDVD;
}

else if( !strncasecmp( psz_file, "VR_MANGR", 8 ) )
{
psz_probe = "DVD_RTR_";
p_stream->pf_readdir = ReadDVD_VR;
}
else
return VLC_EGENERIC;

const uint8_t *p_peek;
ssize_t i_peek = vlc_stream_Peek( p_stream->s, &p_peek, 8 );
if( i_peek < 8 || memcmp( p_peek, psz_probe, 8 ) )
return VLC_EGENERIC;

p_stream->pf_control = access_vaDirectoryControlHelper;

return VLC_SUCCESS;
}

static int ReadDVD( stream_t *p_stream, input_item_node_t *node )
{
const char *psz_location = StreamLocation(p_stream);

char *psz_url = strndup( psz_location, strlen( psz_location ) - 12 );
if( !psz_url )
return VLC_ENOMEM;

input_item_t *p_input = input_item_New( psz_url, psz_url );
if( p_input )
{
input_item_AddOption( p_input, "demux=dvd", VLC_INPUT_OPTION_TRUSTED );
input_item_node_AppendItem( node, p_input );
input_item_Release( p_input );
}

free( psz_url );

return VLC_SUCCESS;
}

static int ReadDVD_VR( stream_t *p_stream, input_item_node_t *node )
{
const char *psz_location = StreamLocation(p_stream);

size_t len = strlen( psz_location );
char *psz_url = strdup( psz_location );

if( unlikely( psz_url == NULL ) )
return VLC_EGENERIC;

strcpy( &psz_url[len - 12], "VR_MOVIE.VRO" );

input_item_t *p_input = input_item_New( psz_url, psz_url );
if( p_input )
{
input_item_node_AppendItem( node, p_input );
input_item_Release( p_input );
}

free( psz_url );

return VLC_SUCCESS;
}
