#if !defined(MERGE_FFMPEG)
#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_plugin.h>
#include "avformat.h"
#include "../../codec/avcodec/avcommon.h"
vlc_module_begin ()
#endif 
add_shortcut( "ffmpeg", "avformat" )
set_category( CAT_INPUT )
set_subcategory( SUBCAT_INPUT_DEMUX )
set_description( N_("Avformat demuxer" ) )
set_shortname( N_("Avformat") )
set_capability( "demux", 2 )
set_callbacks( avformat_OpenDemux, avformat_CloseDemux )
set_section( N_("Demuxer"), NULL )
add_string( "avformat-format", NULL, FORMAT_TEXT, FORMAT_LONGTEXT, true )
add_obsolete_string("ffmpeg-format") 
add_string( "avformat-options", NULL, AV_OPTIONS_TEXT, AV_OPTIONS_LONGTEXT, true )
#if defined(ENABLE_SOUT)
add_submodule ()
add_shortcut( "ffmpeg", "avformat" )
set_description( N_("Avformat muxer" ) )
set_capability( "sout mux", 2 )
set_section( N_("Muxer"), NULL )
add_string( "sout-avformat-mux", NULL, MUX_TEXT, MUX_LONGTEXT, true )
add_obsolete_string("ffmpeg-mux") 
add_string( "sout-avformat-options", NULL, AV_OPTIONS_TEXT, AV_OPTIONS_LONGTEXT, true )
add_bool( "sout-avformat-reset-ts", false, AV_RESET_TS_TEXT, AV_RESET_TS_LONGTEXT, true )
set_callbacks( avformat_OpenMux, avformat_CloseMux )
#endif
#if !defined(MERGE_FFMPEG)
vlc_module_end ()
#endif
