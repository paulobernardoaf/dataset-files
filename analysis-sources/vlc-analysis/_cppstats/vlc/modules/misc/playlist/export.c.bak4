
























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#define VLC_MODULE_LICENSE VLC_LICENSE_GPL_2_PLUS
#include <vlc_common.h>
#include <vlc_plugin.h>




int Export_M3U ( vlc_object_t *p_intf );
int Export_M3U8 ( vlc_object_t *p_intf );
int Export_HTML ( vlc_object_t *p_intf );
int xspf_export_playlist( vlc_object_t *p_intf );




vlc_module_begin ()

set_category( CAT_PLAYLIST )
set_subcategory( SUBCAT_PLAYLIST_EXPORT )
add_submodule ()
set_description( N_("M3U playlist export") )
add_shortcut( "export-m3u" )
set_capability( "playlist export" , 0 )
set_callback( Export_M3U )

add_submodule ()
set_description( N_("M3U8 playlist export") )
add_shortcut( "export-m3u8" )
set_capability( "playlist export" , 0 )
set_callback( Export_M3U8 )

add_submodule ()
set_description( N_("XSPF playlist export") )
add_shortcut( "export-xspf" )
set_capability( "playlist export" , 0 )
set_callback( xspf_export_playlist )

add_submodule ()
set_description( N_("HTML playlist export") )
add_shortcut( "export-html" )
set_capability( "playlist export", 0 )
set_callback( Export_HTML )

vlc_module_end ()
