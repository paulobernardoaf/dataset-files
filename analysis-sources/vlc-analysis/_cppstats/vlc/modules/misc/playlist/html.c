#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_input_item.h>
#include <vlc_playlist_export.h>
#include <vlc_strings.h>
#include <assert.h>
int Export_HTML( vlc_object_t *p_this );
static void DoExport( struct vlc_playlist_export *p_export )
{
size_t count = vlc_playlist_view_Count(p_export->playlist_view);
for( size_t i = 0; i < count; ++i )
{
vlc_playlist_item_t *item =
vlc_playlist_view_Get(p_export->playlist_view, i);
input_item_t *media = vlc_playlist_item_GetMedia(item);
char* psz_name = NULL;
char *psz_tmp = input_item_GetName(media);
if( psz_tmp )
psz_name = vlc_xml_encode( psz_tmp );
free( psz_tmp );
if( psz_name )
{
char* psz_artist = NULL;
psz_tmp = input_item_GetArtist(media);
if( psz_tmp )
psz_artist = vlc_xml_encode( psz_tmp );
free( psz_tmp );
vlc_tick_t i_duration = input_item_GetDuration(media);
int min = SEC_FROM_VLC_TICK( i_duration ) / 60;
int sec = SEC_FROM_VLC_TICK( i_duration ) - min * 60;
if( psz_artist && *psz_artist )
fprintf( p_export->file, " <li>%s - %s (%02d:%02d)</li>\n", psz_artist, psz_name, min, sec );
else
fprintf( p_export->file, " <li>%s (%2d:%2d)</li>\n", psz_name, min, sec);
free( psz_artist );
}
free( psz_name );
}
}
int Export_HTML( vlc_object_t *p_this )
{
struct vlc_playlist_export *p_export = (struct vlc_playlist_export *) p_this;
msg_Dbg( p_export, "saving using HTML format" );
fprintf( p_export->file, "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n"
"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\" \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n"
"<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\">\n"
"<head>\n"
" <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n"
" <meta name=\"Generator\" content=\"VLC media player\" />\n"
" <meta name=\"Author\" content=\"VLC, http://www.videolan.org/vlc/\" />\n"
" <title>VLC generated playlist</title>\n"
" <style type=\"text/css\">\n"
" body {\n"
" background-color: #E4F3FF;\n"
" font-family: sans-serif, Helvetica, Arial;\n"
" font-size: 13px;\n"
" }\n"
" h1 {\n"
" color: #2D58AE;\n"
" font-size: 25px;\n"
" }\n"
" hr {\n"
" color: #555555;\n"
" }\n"
" </style>\n"
"</head>\n\n"
"<body>\n"
" <h1>Playlist</h1>\n"
" <hr />\n"
" <ol>\n" );
DoExport(p_export);
fprintf( p_export->file, " </ol>\n"
" <hr />\n"
"</body>\n"
"</html>" );
return VLC_SUCCESS;
}
