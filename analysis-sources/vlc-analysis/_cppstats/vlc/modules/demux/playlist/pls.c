#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_access.h>
#include <vlc_charset.h>
#include "playlist.h"
static int ReadDir( stream_t *, input_item_node_t * );
int Import_PLS( vlc_object_t *p_this )
{
stream_t *p_demux = (stream_t *)p_this;
const uint8_t *p_peek;
CHECK_FILE(p_demux);
if( vlc_stream_Peek( p_demux->s, &p_peek, 10 ) < 10 ) {
msg_Dbg( p_demux, "not enough data" );
return VLC_EGENERIC;
}
if( strncasecmp( (const char *)p_peek, "[playlist]", 10 )
&& !stream_HasExtension( p_demux, ".pls" ) )
return VLC_EGENERIC;
msg_Dbg( p_demux, "found valid PLS playlist file");
p_demux->pf_readdir = ReadDir;
p_demux->pf_control = access_vaDirectoryControlHelper;
return VLC_SUCCESS;
}
static int ReadDir( stream_t *p_demux, input_item_node_t *p_subitems )
{
char *psz_name = NULL;
char *psz_line;
char *psz_mrl = NULL;
char *psz_mrl_orig = NULL;
char *psz_key;
char *psz_value;
int i_item = -1;
input_item_t *p_input;
bool ascii = true;
bool unicode = true;
while( ( psz_line = vlc_stream_ReadLine( p_demux->s ) ) )
{
if (ascii && !IsASCII(psz_line))
{
unicode = IsUTF8(psz_line);
ascii = false;
}
if (!unicode)
{
char *latin = FromLatin1(psz_line);
free(psz_line);
psz_line = latin;
}
if( !strncasecmp( psz_line, "[playlist]", sizeof("[playlist]")-1 ) )
{
free( psz_line );
continue;
}
psz_key = psz_line;
psz_value = strchr( psz_line, '=' );
if( psz_value )
{
*psz_value='\0';
psz_value++;
}
else
{
free( psz_line );
continue;
}
if( !strcasecmp( psz_key, "version" ) )
{
msg_Dbg( p_demux, "pls file version: %s", psz_value );
free( psz_line );
continue;
}
if( !strcasecmp( psz_key, "numberofentries" ) )
{
msg_Dbg( p_demux, "pls should have %d entries", atoi(psz_value) );
free( psz_line);
continue;
}
int i_new_item;
if( sscanf( psz_key, "%*[^0-9]%d", &i_new_item ) != 1 )
{
msg_Warn( p_demux, "couldn't find number of items" );
free( psz_line );
continue;
}
if( i_item == -1 )
i_item = i_new_item;
else if( i_item != i_new_item )
{
if( psz_mrl )
{
p_input = input_item_New( psz_mrl, psz_name );
input_item_node_AppendItem( p_subitems, p_input );
input_item_Release( p_input );
free( psz_mrl_orig );
psz_mrl_orig = psz_mrl = NULL;
}
else
{
msg_Warn( p_demux, "no file= part found for item %d", i_item );
}
free( psz_name );
psz_name = NULL;
i_item = i_new_item;
}
if( !strncasecmp( psz_key, "file", sizeof("file") -1 ) )
{
free( psz_mrl_orig );
psz_mrl_orig =
psz_mrl = ProcessMRL( psz_value, p_demux->psz_url );
}
else if( !strncasecmp( psz_key, "title", sizeof("title") -1 ) )
{
free( psz_name );
psz_name = strdup( psz_value );
}
else if( !strncasecmp( psz_key, "length", sizeof("length") -1 ) )
;
else
{
msg_Warn( p_demux, "unknown key found in pls file: %s", psz_key );
}
free( psz_line );
}
if( psz_mrl )
{
p_input = input_item_New( psz_mrl, psz_name );
input_item_node_AppendItem( p_subitems, p_input );
input_item_Release( p_input );
free( psz_mrl_orig );
}
else
{
msg_Warn( p_demux, "no file= part found for item %d", i_item );
}
free( psz_name );
psz_name = NULL;
return VLC_SUCCESS;
}
