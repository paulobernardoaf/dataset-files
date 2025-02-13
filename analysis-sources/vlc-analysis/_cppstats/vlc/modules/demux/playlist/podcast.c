#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_access.h>
#include "playlist.h"
#include <vlc_xml.h>
#include <vlc_strings.h>
static int ReadDir( stream_t *, input_item_node_t * );
static vlc_tick_t strTimeToMTime( const char *psz );
int Import_podcast( vlc_object_t *p_this )
{
stream_t *p_demux = (stream_t *)p_this;
CHECK_FILE(p_demux);
if( stream_IsMimeType( p_demux->s, "text/xml" )
|| stream_IsMimeType( p_demux->s, "application/xml" ) )
{
const uint8_t *p_peek;
ssize_t i_peek = vlc_stream_Peek( p_demux->s, &p_peek, 2048 );
if( unlikely( i_peek <= 0 ) )
return VLC_EGENERIC;
stream_t *p_probestream =
vlc_stream_MemoryNew( p_demux, (uint8_t *)p_peek, i_peek, true );
if( unlikely( !p_probestream ) )
return VLC_EGENERIC;
xml_reader_t *p_xml_reader = xml_ReaderCreate( p_demux, p_probestream );
if( !p_xml_reader )
{
vlc_stream_Delete( p_probestream );
return VLC_EGENERIC;
}
const char *node;
int ret;
if( ( ret = xml_ReaderNextNode( p_xml_reader, &node ) ) != XML_READER_STARTELEM
|| strcmp( node, "rss" ) )
{
vlc_stream_Delete( p_probestream );
xml_ReaderDelete( p_xml_reader );
return VLC_EGENERIC;
}
xml_ReaderDelete( p_xml_reader );
vlc_stream_Delete( p_probestream );
}
else if( !stream_IsMimeType( p_demux->s, "application/rss+xml" ) )
return VLC_EGENERIC;
p_demux->pf_readdir = ReadDir;
p_demux->pf_control = access_vaDirectoryControlHelper;
msg_Dbg( p_demux, "using podcast reader" );
return VLC_SUCCESS;
}
static int ReadDir( stream_t *p_demux, input_item_node_t *p_subitems )
{
bool b_item = false;
bool b_image = false;
xml_reader_t *p_xml_reader;
char *psz_elname = NULL;
char *psz_item_mrl = NULL;
char *psz_item_size = NULL;
char *psz_item_type = NULL;
char *psz_item_name = NULL;
char *psz_item_date = NULL;
char *psz_item_author = NULL;
char *psz_item_category = NULL;
char *psz_item_duration = NULL;
char *psz_item_keywords = NULL;
char *psz_item_subtitle = NULL;
char *psz_item_summary = NULL;
char *psz_art_url = NULL;
const char *node;
int i_type;
input_item_t *p_input;
input_item_t *p_current_input = GetCurrentItem(p_demux);
p_xml_reader = xml_ReaderCreate( p_demux, p_demux->s );
if( !p_xml_reader )
goto error;
if( xml_ReaderNextNode( p_xml_reader, &node ) != XML_READER_STARTELEM )
{
msg_Err( p_demux, "invalid file (no root node)" );
goto error;
}
if( strcmp( node, "rss" ) )
{
msg_Err( p_demux, "invalid root node <%s>", node );
goto error;
}
while( (i_type = xml_ReaderNextNode( p_xml_reader, &node )) > 0 )
{
switch( i_type )
{
case XML_READER_STARTELEM:
{
free( psz_elname );
psz_elname = strdup( node );
if( unlikely(!psz_elname) )
goto error;
if( !strcmp( node, "item" ) )
b_item = true;
else if( !strcmp( node, "image" ) )
b_image = true;
const char *attr, *value;
while( (attr = xml_ReaderNextAttr( p_xml_reader, &value )) )
{
if( !strcmp( node, "enclosure" ) )
{
char **p = NULL;
if( !strcmp( attr, "url" ) )
p = &psz_item_mrl;
else if( !strcmp( attr, "length" ) )
p = &psz_item_size;
else if( !strcmp( attr, "type" ) )
p = &psz_item_type;
if( p != NULL )
{
free( *p );
*p = strdup( value );
}
else
msg_Dbg( p_demux,"unhandled attribute %s in <%s>",
attr, node );
}
else
msg_Dbg( p_demux,"unhandled attribute %s in <%s>",
attr, node );
}
break;
}
case XML_READER_TEXT:
{
if(!psz_elname) break;
if( b_item )
{
char **p;
if( !strcmp( psz_elname, "title" ) )
p = &psz_item_name;
else if( !strcmp( psz_elname, "itunes:author" ) ||
!strcmp( psz_elname, "author" ) )
p = &psz_item_author;
else if( !strcmp( psz_elname, "itunes:summary" ) ||
!strcmp( psz_elname, "description" ) )
p = &psz_item_summary;
else if( !strcmp( psz_elname, "pubDate" ) )
p = &psz_item_date;
else if( !strcmp( psz_elname, "itunes:category" ) )
p = &psz_item_category;
else if( !strcmp( psz_elname, "itunes:duration" ) )
p = &psz_item_duration;
else if( !strcmp( psz_elname, "itunes:keywords" ) )
p = &psz_item_keywords;
else if( !strcmp( psz_elname, "itunes:subtitle" ) )
p = &psz_item_subtitle;
else
break;
free( *p );
*p = strdup( node );
}
else if( !b_image )
{
if( !strcmp( psz_elname, "title" ) )
input_item_SetName( p_current_input, node );
#define ADD_GINFO( info, name ) else if( !strcmp( psz_elname, name ) ) input_item_AddInfo( p_current_input, _("Podcast Info"), info, "%s", node );
ADD_GINFO( _("Podcast Link"), "link" )
ADD_GINFO( _("Podcast Copyright"), "copyright" )
ADD_GINFO( _("Podcast Category"), "itunes:category" )
ADD_GINFO( _("Podcast Keywords"), "itunes:keywords" )
ADD_GINFO( _("Podcast Subtitle"), "itunes:subtitle" )
#undef ADD_GINFO
else if( !strcmp( psz_elname, "itunes:summary" ) ||
!strcmp( psz_elname, "description" ) )
{ 
input_item_AddInfo( p_current_input,
_( "Podcast Info" ), _( "Podcast Summary" ),
"%s", node );
}
}
else
{
if( !strcmp( psz_elname, "url" ) && *node )
{
free( psz_art_url );
psz_art_url = strdup( node );
}
else
msg_Dbg( p_demux, "unhandled text in element <%s>",
psz_elname );
}
break;
}
case XML_READER_ENDELEM:
{
FREENULL( psz_elname );
if( !strcmp( node, "item" ) )
{
if( psz_item_mrl == NULL )
{
if (psz_item_name)
msg_Warn( p_demux, "invalid XML item, skipping %s",
psz_item_name );
else
msg_Warn( p_demux, "invalid XML item, skipped" );
FREENULL( psz_item_name );
FREENULL( psz_item_size );
FREENULL( psz_item_type );
FREENULL( psz_item_date );
FREENULL( psz_item_author );
FREENULL( psz_item_category );
FREENULL( psz_item_duration );
FREENULL( psz_item_keywords );
FREENULL( psz_item_subtitle );
FREENULL( psz_item_summary );
FREENULL( psz_art_url );
FREENULL( psz_elname );
continue;
}
vlc_xml_decode( psz_item_mrl );
if( psz_item_name )
vlc_xml_decode( psz_item_name );
p_input = input_item_New( psz_item_mrl, psz_item_name );
FREENULL( psz_item_mrl );
FREENULL( psz_item_name );
if( p_input == NULL )
break; 
if( psz_item_duration )
p_input->i_duration = strTimeToMTime( psz_item_duration );
#define ADD_INFO( info, field ) if( field ) { input_item_AddInfo( p_input, _( "Podcast Info" ), (info), "%s", (field) ); FREENULL( field ); }
ADD_INFO( _("Podcast Publication Date"), psz_item_date );
ADD_INFO( _("Podcast Author"), psz_item_author );
ADD_INFO( _("Podcast Subcategory"), psz_item_category );
ADD_INFO( _("Podcast Duration"), psz_item_duration );
ADD_INFO( _("Podcast Keywords"), psz_item_keywords );
ADD_INFO( _("Podcast Subtitle"), psz_item_subtitle );
ADD_INFO( _("Podcast Summary"), psz_item_summary );
ADD_INFO( _("Podcast Type"), psz_item_type );
#undef ADD_INFO
if( psz_art_url )
{
vlc_xml_decode( psz_art_url );
input_item_SetArtURL( p_input, psz_art_url );
}
if( psz_item_size )
{
input_item_AddInfo( p_input,
_( "Podcast Info" ),
_( "Podcast Size" ),
_("%s bytes"),
psz_item_size );
FREENULL( psz_item_size );
}
input_item_node_AppendItem( p_subitems, p_input );
input_item_Release( p_input );
b_item = false;
}
else if( !strcmp( node, "image" ) )
{
b_image = false;
}
break;
}
}
}
if( i_type < 0 )
{
msg_Warn( p_demux, "error while parsing data" );
}
free( psz_art_url );
free( psz_elname );
xml_ReaderDelete( p_xml_reader );
return VLC_SUCCESS;
error:
free( psz_item_name );
free( psz_item_mrl );
free( psz_item_size );
free( psz_item_type );
free( psz_item_date );
free( psz_item_author );
free( psz_item_category );
free( psz_item_duration );
free( psz_item_keywords );
free( psz_item_subtitle );
free( psz_item_summary );
free( psz_art_url );
free( psz_elname );
if( p_xml_reader )
xml_ReaderDelete( p_xml_reader );
return VLC_EGENERIC;
}
static vlc_tick_t strTimeToMTime( const char *psz )
{
int h, m, s;
switch( sscanf( psz, "%u:%u:%u", &h, &m, &s ) )
{
case 3:
return vlc_tick_from_sec( ( h*60 + m )*60 + s );
case 2:
return vlc_tick_from_sec( h*60 + m );
default:
return INPUT_DURATION_UNSET;
}
}
