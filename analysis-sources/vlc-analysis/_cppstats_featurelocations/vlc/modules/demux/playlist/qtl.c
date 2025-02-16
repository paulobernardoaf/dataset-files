
















































#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_access.h>

#include "playlist.h"
#include <vlc_xml.h>
#include <vlc_strings.h>

typedef enum { FULLSCREEN_NORMAL,
FULLSCREEN_DOUBLE,
FULLSCREEN_HALF,
FULLSCREEN_CURRENT,
FULLSCREEN_FULL } qtl_fullscreen_t;
const char* ppsz_fullscreen[] = { "normal", "double", "half", "current", "full" };
typedef enum { LOOP_TRUE,
LOOP_FALSE,
LOOP_PALINDROME } qtl_loop_t;
const char* ppsz_loop[] = { "true", "false", "palindrome" };

#define ROOT_NODE_MAX_DEPTH 2




static int ReadDir( stream_t *, input_item_node_t * );




int Import_QTL( vlc_object_t *p_this )
{
stream_t *p_demux = (stream_t *)p_this;

CHECK_FILE(p_demux);
if( !stream_HasExtension( p_demux, ".qtl" ) )
return VLC_EGENERIC;

p_demux->pf_readdir = ReadDir;
p_demux->pf_control = access_vaDirectoryControlHelper;
msg_Dbg( p_demux, "using QuickTime Media Link reader" );

return VLC_SUCCESS;
}

static int ReadDir( stream_t *p_demux, input_item_node_t *p_subitems )
{
xml_reader_t *p_xml_reader;
input_item_t *p_input;
int i_ret = -1;


bool b_autoplay = false;
bool b_controller = true;
qtl_fullscreen_t fullscreen = false;
char *psz_href = NULL;
bool b_kioskmode = false;
qtl_loop_t loop = LOOP_FALSE;
int i_movieid = -1;
char *psz_moviename = NULL;
bool b_playeveryframe = false;
char *psz_qtnext = NULL;
bool b_quitwhendone = false;
char *psz_src = NULL;
char *psz_mimetype = NULL;
int i_volume = 100;

p_xml_reader = xml_ReaderCreate( p_demux, p_demux->s );
if( !p_xml_reader )
goto error;

for( int i = 0;; ++i ) 
{
const char *node;
if( i == ROOT_NODE_MAX_DEPTH ||
xml_ReaderNextNode( p_xml_reader, &node ) != XML_READER_STARTELEM )
{
msg_Err( p_demux, "unable to locate root-node" );
goto error;
}

if( strcmp( node, "embed" ) == 0 )
break; 

msg_Dbg( p_demux, "invalid root node <%s>, trying next (%d / %d)",
node, i + 1, ROOT_NODE_MAX_DEPTH );
}

const char *attrname, *value;
while( (attrname = xml_ReaderNextAttr( p_xml_reader, &value )) != NULL )
{
if( !strcmp( attrname, "autoplay" ) )
b_autoplay = !strcmp( value, "true" );
else if( !strcmp( attrname, "controller" ) )
b_controller = !strcmp( attrname, "false" );
else if( !strcmp( attrname, "fullscreen" ) )
{
if( !strcmp( value, "double" ) )
fullscreen = FULLSCREEN_DOUBLE;
else if( !strcmp( value, "half" ) )
fullscreen = FULLSCREEN_HALF;
else if( !strcmp( value, "current" ) )
fullscreen = FULLSCREEN_CURRENT;
else if( !strcmp( value, "full" ) )
fullscreen = FULLSCREEN_FULL;
else
fullscreen = FULLSCREEN_NORMAL;
}
else if( !strcmp( attrname, "href" ) )
{
free( psz_href );
psz_href = strdup( value );
}
else if( !strcmp( attrname, "kioskmode" ) )
b_kioskmode = !strcmp( value, "true" );
else if( !strcmp( attrname, "loop" ) )
{
if( !strcmp( value, "true" ) )
loop = LOOP_TRUE;
else if( !strcmp( value, "palindrome" ) )
loop = LOOP_PALINDROME;
else
loop = LOOP_FALSE;
}
else if( !strcmp( attrname, "movieid" ) )
i_movieid = atoi( value );
else if( !strcmp( attrname, "moviename" ) )
{
free( psz_moviename );
psz_moviename = strdup( value );
}
else if( !strcmp( attrname, "playeveryframe" ) )
b_playeveryframe = !strcmp( value, "true" );
else if( !strcmp( attrname, "qtnext" ) )
{
free( psz_qtnext );
psz_qtnext = strdup( value );
}
else if( !strcmp( attrname, "quitwhendone" ) )
b_quitwhendone = !strcmp( value, "true" );
else if( !strcmp( attrname, "src" ) )
{
free( psz_src );
psz_src = strdup( value );
}
else if( !strcmp( attrname, "mimetype" ) )
{
free( psz_mimetype );
psz_mimetype = strdup( value );
}
else if( !strcmp( attrname, "volume" ) )
i_volume = atoi( value );
else
msg_Dbg( p_demux, "Attribute %s with value %s isn't valid",
attrname, value );
}

msg_Dbg( p_demux, "autoplay: %s (unused by VLC)",
b_autoplay ? "true": "false" );
msg_Dbg( p_demux, "controller: %s (unused by VLC)",
b_controller ? "true": "false" );
msg_Dbg( p_demux, "fullscreen: %s (unused by VLC)",
ppsz_fullscreen[fullscreen] );
msg_Dbg( p_demux, "href: %s", psz_href );
msg_Dbg( p_demux, "kioskmode: %s (unused by VLC)",
b_kioskmode ? "true":"false" );
msg_Dbg( p_demux, "loop: %s (unused by VLC)", ppsz_loop[loop] );
msg_Dbg( p_demux, "movieid: %d (unused by VLC)", i_movieid );
msg_Dbg( p_demux, "moviename: %s", psz_moviename );
msg_Dbg( p_demux, "playeverframe: %s (unused by VLC)",
b_playeveryframe ? "true":"false" );
msg_Dbg( p_demux, "qtnext: %s", psz_qtnext );
msg_Dbg( p_demux, "quitwhendone: %s (unused by VLC)",
b_quitwhendone ? "true":"false" );
msg_Dbg( p_demux, "src: %s", psz_src );
msg_Dbg( p_demux, "mimetype: %s", psz_mimetype );
msg_Dbg( p_demux, "volume: %d (unused by VLC)", i_volume );


if( !psz_src )
{
msg_Err( p_demux, "Mandatory attribute 'src' not found" );
}
else
{
p_input = input_item_New( psz_src, psz_moviename );
#define SADD_INFO( type, field ) if( field ) { input_item_AddInfo( p_input, "QuickTime Media Link", type, "%s", field ) ; }

SADD_INFO( "href", psz_href );
SADD_INFO( _("Mime"), psz_mimetype );
input_item_node_AppendItem( p_subitems, p_input );
input_item_Release( p_input );
if( psz_qtnext )
{
vlc_xml_decode( psz_qtnext );
p_input = input_item_New( psz_qtnext, NULL );
input_item_node_AppendItem( p_subitems, p_input );
input_item_Release( p_input );
}
}

i_ret = 0; 

error:
if( p_xml_reader )
xml_ReaderDelete( p_xml_reader );

free( psz_href );
free( psz_moviename );
free( psz_qtnext );
free( psz_src );
free( psz_mimetype );
return i_ret;
}
