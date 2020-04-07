#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include "fs.h"
#include <vlc_plugin.h>
vlc_module_begin ()
set_description( N_("File input") )
set_shortname( N_("File") )
set_category( CAT_INPUT )
set_subcategory( SUBCAT_INPUT_ACCESS )
add_obsolete_string( "file-cat" )
set_capability( "access", 50 )
add_shortcut( "file", "fd", "stream" )
set_callbacks( FileOpen, FileClose )
add_submodule()
set_section( N_("Directory" ), NULL )
set_capability( "access", 55 )
#if !defined(HAVE_FDOPENDIR)
add_shortcut( "file", "directory", "dir" )
#else
add_shortcut( "directory", "dir" )
#endif
set_callbacks( DirOpen, DirClose )
add_bool("list-special-files", false, N_("List special files"),
N_("Include devices and pipes when listing directories"), true)
add_obsolete_string("directory-sort") 
vlc_module_end ()
