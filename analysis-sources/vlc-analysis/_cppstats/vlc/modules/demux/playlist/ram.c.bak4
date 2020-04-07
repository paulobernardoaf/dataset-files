











































#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ctype.h>

#include <vlc_common.h>
#include <vlc_access.h>
#include <vlc_url.h>
#include <vlc_charset.h>

#include "playlist.h"




static int ReadDir( stream_t *, input_item_node_t * );
static void ParseClipInfo( const char * psz_clipinfo, char **ppsz_artist, char **ppsz_title,
char **ppsz_album, char **ppsz_genre, char **ppsz_year,
char **ppsz_cdnum, char **ppsz_comments );






int Import_RAM( vlc_object_t *p_this )
{
stream_t *p_demux = (stream_t *)p_this;
const uint8_t *p_peek;

CHECK_FILE(p_demux);
if( !stream_HasExtension( p_demux, ".ram" )
&& !stream_HasExtension( p_demux, ".rm" ) )
return VLC_EGENERIC;


if( vlc_stream_Peek( p_demux->s, &p_peek, 4 ) < 4 )
return VLC_EGENERIC;
if( !memcmp( p_peek, ".ra", 3 ) || !memcmp( p_peek, ".RMF", 4 ) )
{
return VLC_EGENERIC;
}

msg_Dbg( p_demux, "found valid RAM playlist" );
p_demux->pf_readdir = ReadDir;
p_demux->pf_control = access_vaDirectoryControlHelper;

return VLC_SUCCESS;
}






static const char *SkipBlanks( const char *s, size_t i_strlen )
{
while( i_strlen > 0 ) {
switch( *s )
{
case ' ':
case '\t':
case '\r':
case '\n':
--i_strlen;
++s;
break;
default:
i_strlen = 0;
}
}
return s;
}







static int ParseTime( const char *s, size_t i_strlen)
{

int result = 0;
int val;
const char *end = s + i_strlen;

s = SkipBlanks(s, i_strlen);

val = 0;
while( (s < end) && isdigit((unsigned char)*s) )
{
int newval = val*10 + (*s - '0');
if( newval < val )
{

val = 0;
break;
}
val = newval;
++s;
}
result = val;
s = SkipBlanks(s, end-s);
if( *s == ':' )
{
++s;
s = SkipBlanks(s, end-s);
result = result * 60;
val = 0;
while( (s < end) && isdigit((unsigned char)*s) )
{
int newval = val*10 + (*s - '0');
if( newval < val )
{

val = 0;
break;
}
val = newval;
++s;
}
result += val;
s = SkipBlanks(s, end-s);
if( *s == ':' )
{
++s;
s = SkipBlanks(s, end-s);
result = result * 60;
val = 0;
while( (s < end) && isdigit((unsigned char)*s) )
{
int newval = val*10 + (*s - '0');
if( newval < val )
{

val = 0;
break;
}
val = newval;
++s;
}
result += val;

}
}
return result;
}

static int ReadDir( stream_t *p_demux, input_item_node_t *p_subitems )
{
const char *psz_prefix = p_demux->psz_url;
if( unlikely(psz_prefix == NULL) )
return VLC_SUCCESS;

char *psz_line;
char *psz_artist = NULL, *psz_album = NULL, *psz_genre = NULL, *psz_year = NULL;
char *psz_author = NULL, *psz_title = NULL, *psz_copyright = NULL, *psz_cdnum = NULL, *psz_comments = NULL;
const char **ppsz_options = NULL;
int i_options = 0, i_start = 0, i_stop = 0;
bool b_cleanup = false;
input_item_t *p_input;

psz_line = vlc_stream_ReadLine( p_demux->s );
while( psz_line )
{
char *psz_parse = psz_line;


while( *psz_parse == ' ' || *psz_parse == '\t' ||
*psz_parse == '\n' || *psz_parse == '\r' ) psz_parse++;

if( *psz_parse == '#' )
{

}
else if( *psz_parse )
{
char *psz_mrl, *psz_option_next, *psz_option;
char *psz_param, *psz_value;


psz_mrl = ProcessMRL( psz_parse, psz_prefix );

b_cleanup = true;
if ( !psz_mrl ) goto error;


psz_option = strchr( psz_mrl, '?' ); 
if( psz_option )
{


*psz_option = '\0';
psz_option++;
psz_option_next = psz_option;
while( 1 ) 
{

psz_option = psz_option_next;
psz_option_next = strchr( psz_option, '&' );
if( psz_option_next )
{
*psz_option_next = '\0';
psz_option_next++;
}
else
psz_option_next = strchr( psz_option, '\0' );

if( psz_option_next == psz_option )
break;


psz_param = psz_option;
psz_value = strchr( psz_option, '=' );
if( psz_value == NULL )
break;
*psz_value = '\0';
psz_value++;



if( !strcmp( psz_param, "clipinfo" ) )
{
ParseClipInfo( psz_value, &psz_artist, &psz_title,
&psz_album, &psz_genre, &psz_year,
&psz_cdnum, &psz_comments ); 
}
else if( !strcmp( psz_param, "author" ) )
{
psz_author = vlc_uri_decode_duplicate(psz_value);
EnsureUTF8( psz_author );
}
else if( !strcmp( psz_param, "start" )
&& strncmp( psz_mrl, "rtsp", 4 ) )
{
i_start = ParseTime( psz_value, strlen( psz_value ) );
char *temp;
if( i_start )
{
if( asprintf( &temp, ":start-time=%d", i_start ) != -1 )
TAB_APPEND( i_options, ppsz_options, temp );
}
}
else if( !strcmp( psz_param, "end" ) )
{
i_stop = ParseTime( psz_value, strlen( psz_value ) );
char *temp;
if( i_stop )
{
if( asprintf( &temp, ":stop-time=%d", i_stop ) != -1 )
TAB_APPEND( i_options, ppsz_options, temp );
}
}
else if( !strcmp( psz_param, "title" ) )
{
free( psz_title );
psz_title = vlc_uri_decode_duplicate(psz_value);
EnsureUTF8( psz_title );
}
else if( !strcmp( psz_param, "copyright" ) )
{
psz_copyright = vlc_uri_decode_duplicate(psz_value);
EnsureUTF8( psz_copyright );
}
else
{ 

}
}
}


p_input = input_item_New( psz_mrl, psz_title );
if( !p_input )
{
free( psz_mrl );
goto error;
}
if( ppsz_options )
input_item_AddOptions( p_input, i_options, ppsz_options, 0 );

if( !EMPTY_STR( psz_artist ) ) input_item_SetArtist( p_input, psz_artist );
if( !EMPTY_STR( psz_author ) ) input_item_SetPublisher( p_input, psz_author );
if( !EMPTY_STR( psz_title ) ) input_item_SetTitle( p_input, psz_title );
if( !EMPTY_STR( psz_copyright ) ) input_item_SetCopyright( p_input, psz_copyright );
if( !EMPTY_STR( psz_album ) ) input_item_SetAlbum( p_input, psz_album );
if( !EMPTY_STR( psz_genre ) ) input_item_SetGenre( p_input, psz_genre );
if( !EMPTY_STR( psz_year ) ) input_item_SetDate( p_input, psz_year );
if( !EMPTY_STR( psz_cdnum ) ) input_item_SetTrackNum( p_input, psz_cdnum );
if( !EMPTY_STR( psz_comments ) ) input_item_SetDescription( p_input, psz_comments );

input_item_node_AppendItem( p_subitems, p_input );
input_item_Release( p_input );
free( psz_mrl );
}

error:

free( psz_line );
psz_line = vlc_stream_ReadLine( p_demux->s );
if( !psz_line ) b_cleanup = true;

if( b_cleanup )
{

while( i_options-- ) free( (char*)ppsz_options[i_options] );
FREENULL( ppsz_options );
FREENULL( psz_artist );
FREENULL( psz_title );
FREENULL( psz_author );
FREENULL( psz_copyright );
FREENULL( psz_album );
FREENULL( psz_genre );
FREENULL( psz_year );
FREENULL( psz_cdnum );
FREENULL( psz_comments );
i_options = 0;
i_start = 0;
i_stop = 0;
b_cleanup = false;
}
}
return VLC_SUCCESS;
}












static void ParseClipInfo( const char *psz_clipinfo, char **ppsz_artist, char **ppsz_title,
char **ppsz_album, char **ppsz_genre, char **ppsz_year,
char **ppsz_cdnum, char **ppsz_comments )
{
char *psz_option_next, *psz_option_start, *psz_param, *psz_value, *psz_suboption;
char *psz_temp_clipinfo = strdup( psz_clipinfo );
psz_option_start = strchr( psz_temp_clipinfo, '"' );
if( !psz_option_start )
{
free( psz_temp_clipinfo );
return;
}

psz_option_start++;
psz_option_next = psz_option_start;
while( 1 ) 
{

psz_option_start = psz_option_next;
psz_option_next = strchr( psz_option_start, '|' );
if( psz_option_next )
*psz_option_next = '\0';
else
psz_option_next = strchr( psz_option_start, '"' );
if( psz_option_next )
*psz_option_next = '\0';
else
psz_option_next = strchr( psz_option_start, '\0' );
if( psz_option_next == psz_option_start )
break;

psz_suboption = strdup( psz_option_start );
if( !psz_suboption )
break;


psz_param = psz_suboption;
if( strchr( psz_suboption, '=' ) )
{
psz_value = strchr( psz_suboption, '=' ) + 1;
*( strchr( psz_suboption, '=' ) ) = '\0';
}
else
{
free( psz_suboption );
break;
}

if( !strcmp( psz_param, "artist name" ) )
*ppsz_artist = vlc_uri_decode_duplicate( psz_value );
else if( !strcmp( psz_param, "title" ) )
*ppsz_title = vlc_uri_decode_duplicate( psz_value );
else if( !strcmp( psz_param, "album name" ) )
*ppsz_album = vlc_uri_decode_duplicate( psz_value );
else if( !strcmp( psz_param, "genre" ) )
*ppsz_genre = vlc_uri_decode_duplicate( psz_value );
else if( !strcmp( psz_param, "year" ) )
*ppsz_year = vlc_uri_decode_duplicate( psz_value );
else if( !strcmp( psz_param, "cdnum" ) )
*ppsz_cdnum = vlc_uri_decode_duplicate( psz_value );
else if( !strcmp( psz_param, "comments" ) )
*ppsz_comments = vlc_uri_decode_duplicate( psz_value );

free( psz_suboption );
psz_option_next++;
}

free( psz_temp_clipinfo );
}
