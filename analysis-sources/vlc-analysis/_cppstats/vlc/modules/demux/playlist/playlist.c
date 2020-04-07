#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_demux.h>
#include <vlc_url.h>
#if defined( _WIN32 ) || defined( __OS2__ )
#include <ctype.h> 
#endif
#include <assert.h>
#include "playlist.h"
#define SHOW_ADULT_TEXT N_( "Show shoutcast adult content" )
#define SHOW_ADULT_LONGTEXT N_( "Show NC17 rated video streams when " "using shoutcast video playlists." )
#define SKIP_ADS_TEXT N_( "Skip ads" )
#define SKIP_ADS_LONGTEXT N_( "Use playlist options usually used to prevent " "ads skipping to detect ads and prevent adding them to the playlist." )
vlc_module_begin ()
add_shortcut( "playlist" )
set_category( CAT_INPUT )
set_subcategory( SUBCAT_INPUT_DEMUX )
add_obsolete_integer( "parent-item" ) 
add_bool( "playlist-skip-ads", true,
SKIP_ADS_TEXT, SKIP_ADS_LONGTEXT, false )
set_shortname( N_("Playlist") )
set_description( N_("Playlist") )
add_submodule ()
set_description( N_("M3U playlist import") )
add_shortcut( "m3u", "m3u8" )
set_capability( "stream_filter", 310 )
set_callback( Import_M3U )
add_submodule ()
set_description( N_("RAM playlist import") )
set_capability( "stream_filter", 310 )
set_callback( Import_RAM )
add_submodule ()
set_description( N_("PLS playlist import") )
set_capability( "stream_filter", 310 )
set_callback( Import_PLS )
add_submodule ()
set_description( N_("B4S playlist import") )
add_shortcut( "shout-b4s" )
set_capability( "stream_filter", 310 )
set_callback( Import_B4S )
add_submodule ()
set_description( N_("DVB playlist import") )
add_shortcut( "dvb" )
set_capability( "stream_filter", 310 )
set_callback( Import_DVB )
add_submodule ()
set_description( N_("Podcast parser") )
add_shortcut( "podcast" )
set_capability( "stream_filter", 310 )
set_callback( Import_podcast )
add_submodule ()
set_description( N_("XSPF playlist import") )
set_capability( "stream_filter", 310 )
set_callbacks( Import_xspf, Close_xspf )
add_submodule ()
set_description( N_("ASX playlist import") )
set_capability( "stream_filter", 310 )
set_callback( Import_ASX )
add_submodule ()
set_description( N_("Kasenna MediaBase parser") )
add_shortcut( "sgimb" )
set_capability( "stream_filter", 310 )
set_callbacks( Import_SGIMB, Close_SGIMB )
add_submodule ()
set_description( N_("QuickTime Media Link importer") )
add_shortcut( "qtl" )
set_capability( "stream_filter", 310 )
set_callback( Import_QTL )
add_submodule ()
set_description( N_("Dummy IFO demux") )
set_capability( "stream_filter", 312 )
set_callback( Import_IFO )
add_submodule ()
set_description( N_("Dummy BDMV demux") )
set_capability( "stream_filter", 312 )
set_callback( Import_BDMV )
add_submodule ()
set_description( N_("iTunes Music Library importer") )
add_shortcut( "itml" )
set_capability( "stream_filter", 310 )
set_callback( Import_iTML )
add_submodule()
set_description(N_("Windows Media Server metafile import") )
set_capability("stream_filter", 310)
set_callback(Import_WMS)
add_submodule ()
set_description( N_("WPL playlist import") )
add_shortcut( "wpl" )
set_capability( "stream_filter", 310 )
set_callbacks( Import_WPL, Close_WPL )
vlc_module_end ()
char *ProcessMRL(const char *str, const char *base)
{
char const* orig = str;
if (str == NULL)
return NULL;
#if (DIR_SEP_CHAR == '\\')
if (strncmp(str, "\\\\", 2) == 0
|| (isalpha((unsigned char)str[0]) && str[1] == ':'))
return vlc_path2uri(str, NULL);
#endif
char *rel = vlc_uri_fixup(str);
if (rel != NULL)
str = rel;
char *abs = vlc_uri_resolve(base, str);
free(rel);
if (abs == NULL)
{
char const* scheme_end = strstr( orig, "://" );
char const* valid_chars = "abcdefghijklmnopqrstuvwxyz"
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"0123456789" "+-./";
if (scheme_end &&
strspn (orig, valid_chars) == (size_t)(scheme_end - orig))
{
abs = strdup (orig);
}
}
return abs;
}
