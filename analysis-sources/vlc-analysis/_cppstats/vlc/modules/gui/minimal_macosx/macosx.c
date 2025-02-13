#include <stdlib.h> 
#include <string.h>
#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_vout_window.h>
int OpenIntf ( vlc_object_t * );
void CloseIntf ( vlc_object_t * );
int WindowOpen ( vout_window_t * );
vlc_module_begin ()
set_shortname( "Minimal Macosx" )
add_shortcut( "minimal_macosx", "miosx" )
set_description( N_("Minimal Mac OS X interface") )
set_capability( "interface", 50 )
set_callbacks( OpenIntf, CloseIntf )
set_category( CAT_INTERFACE )
set_subcategory( SUBCAT_INTERFACE_MAIN )
add_submodule ()
set_description( "Minimal Mac OS X Video Output Provider" )
set_capability( "vout window", 50 )
set_callback( WindowOpen )
vlc_module_end ()
