#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_input_item.h>
#include <vlc_playlist_export.h>
#include <vlc_strings.h>
#include <vlc_url.h>
#include <assert.h>
int xspf_export_playlist( vlc_object_t *p_this );
static char *input_xml( input_item_t *p_item, char *(*func)(input_item_t *) )
{
char *tmp = func( p_item );
if( tmp == NULL )
return NULL;
char *ret = vlc_xml_encode( tmp );
free( tmp );
return ret;
}
static void xspf_export_item( input_item_t *p_input, FILE *p_file, uint64_t id)
{
char *psz;
vlc_tick_t i_duration;
fputs( "\t\t<track>\n", p_file );
char *psz_uri = input_xml( p_input, input_item_GetURI );
if( psz_uri && *psz_uri )
fprintf( p_file, "\t\t\t<location>%s</location>\n", psz_uri );
psz = input_xml( p_input, input_item_GetTitle );
if( psz && strcmp( psz_uri, psz ) )
fprintf( p_file, "\t\t\t<title>%s</title>\n", psz );
free( psz );
free( psz_uri );
if( p_input->p_meta == NULL )
{
goto xspfexportitem_end;
}
psz = input_xml( p_input, input_item_GetArtist );
if( psz && *psz )
fprintf( p_file, "\t\t\t<creator>%s</creator>\n", psz );
free( psz );
psz = input_xml( p_input, input_item_GetAlbum );
if( psz && *psz )
fprintf( p_file, "\t\t\t<album>%s</album>\n", psz );
free( psz );
psz = input_xml( p_input, input_item_GetTrackNum );
if( psz )
{
int i_tracknum = atoi( psz );
free( psz );
if( i_tracknum > 0 )
fprintf( p_file, "\t\t\t<trackNum>%i</trackNum>\n", i_tracknum );
}
psz = input_xml( p_input, input_item_GetDescription );
if( psz && *psz )
fprintf( p_file, "\t\t\t<annotation>%s</annotation>\n", psz );
free( psz );
psz = input_xml( p_input, input_item_GetURL );
if( psz && *psz )
fprintf( p_file, "\t\t\t<info>%s</info>\n", psz );
free( psz );
psz = input_xml( p_input, input_item_GetArtURL );
if( psz && *psz )
fprintf( p_file, "\t\t\t<image>%s</image>\n", psz );
free( psz );
xspfexportitem_end:
i_duration = input_item_GetDuration( p_input );
if( i_duration > 0 )
fprintf( p_file, "\t\t\t<duration>%"PRIu64"</duration>\n",
MS_FROM_VLC_TICK(i_duration) );
fputs( "\t\t\t<extension application=\""
"http://www.videolan.org/vlc/playlist/0\">\n", p_file );
fprintf( p_file, "\t\t\t\t<vlc:id>%"PRIu64"</vlc:id>\n", id );
for( int i = 0; i < p_input->i_options; i++ )
{
char* psz_src = p_input->ppsz_options[i];
char* psz_ret = NULL;
if ( psz_src[0] == ':' )
psz_src++;
psz_ret = vlc_xml_encode( psz_src );
if ( psz_ret == NULL )
continue;
fprintf( p_file, "\t\t\t\t<vlc:option>%s</vlc:option>\n", psz_ret );
free( psz_ret );
}
fputs( "\t\t\t</extension>\n", p_file );
fputs( "\t\t</track>\n", p_file );
}
int xspf_export_playlist( vlc_object_t *p_this )
{
struct vlc_playlist_export *p_export = (struct vlc_playlist_export *) p_this;
fprintf( p_export->file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" );
fprintf( p_export->file,
"<playlist xmlns=\"http://xspf.org/ns/0/\" " \
"xmlns:vlc=\"http://www.videolan.org/vlc/playlist/ns/0/\" " \
"version=\"1\">\n" );
fprintf( p_export->file, "\t<trackList>\n" );
size_t count = vlc_playlist_view_Count(p_export->playlist_view);
for( size_t i = 0; i < count; ++i )
{
vlc_playlist_item_t *item =
vlc_playlist_view_Get(p_export->playlist_view, i);
input_item_t *media = vlc_playlist_item_GetMedia(item);
xspf_export_item(media, p_export->file, i);
}
fprintf( p_export->file, "\t</trackList>\n" );
fprintf( p_export->file, "</playlist>\n" );
return VLC_SUCCESS;
}
