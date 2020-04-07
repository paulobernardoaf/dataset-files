#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_video_splitter.h>
#define COUNT_TEXT N_("Number of clones")
#define COUNT_LONGTEXT N_("Number of video windows in which to ""clone the video.")
#define VOUTLIST_TEXT N_("Video output modules")
#define VOUTLIST_LONGTEXT N_("You can use specific video output modules " "for the clones. Use a comma-separated list of modules." )
#define CLONE_HELP N_("Duplicate your video to multiple windows " "and/or video output modules")
#define CFG_PREFIX "clone-"
static int Open ( vlc_object_t * );
static void Close( vlc_object_t * );
vlc_module_begin ()
set_description( N_("Clone video filter") )
set_capability( "video splitter", 0 )
set_shortname( N_("Clone" ))
set_help(CLONE_HELP)
set_category( CAT_VIDEO )
set_subcategory( SUBCAT_VIDEO_SPLITTER )
add_integer( CFG_PREFIX "count", 2, COUNT_TEXT, COUNT_LONGTEXT, false )
add_module_list(CFG_PREFIX "vout-list", "vout display", NULL,
VOUTLIST_TEXT, VOUTLIST_LONGTEXT)
add_shortcut( "clone" )
set_callbacks( Open, Close )
vlc_module_end ()
static const char *const ppsz_filter_options[] = {
"count", "vout-list", NULL
};
#define VOUTSEPARATOR ':'
static int Filter( video_splitter_t *, picture_t *pp_dst[], picture_t * );
static int Open( vlc_object_t *p_this )
{
video_splitter_t *p_splitter = (video_splitter_t*)p_this;
config_ChainParse( p_splitter, CFG_PREFIX, ppsz_filter_options,
p_splitter->p_cfg );
char *psz_clonelist = var_CreateGetNonEmptyString( p_splitter,
CFG_PREFIX "vout-list" );
if( psz_clonelist )
{
p_splitter->i_output = 1;
for( int i = 0; psz_clonelist[i]; i++ )
{
if( psz_clonelist[i] == VOUTSEPARATOR )
p_splitter->i_output++;
}
p_splitter->p_output = calloc( p_splitter->i_output,
sizeof(*p_splitter->p_output) );
if( !p_splitter->p_output )
{
free( psz_clonelist );
return VLC_EGENERIC;
}
char *psz_tmp = psz_clonelist;
for( int i = 0; psz_tmp && *psz_tmp; i++ )
{
char *psz_new = strchr( psz_tmp, VOUTSEPARATOR );
if( psz_new )
*psz_new++ = '\0';
p_splitter->p_output[i].psz_module = strdup( psz_tmp );
psz_tmp = psz_new;
}
free( psz_clonelist );
}
else
{
p_splitter->i_output = var_CreateGetInteger( p_splitter, CFG_PREFIX "count" );
if( p_splitter->i_output <= 0 )
p_splitter->i_output = 1;
p_splitter->p_output = calloc( p_splitter->i_output,
sizeof(*p_splitter->p_output) );
if( !p_splitter->p_output )
return VLC_EGENERIC;
for( int i = 0; i < p_splitter->i_output; i++ )
p_splitter->p_output[i].psz_module = NULL;
}
for( int i = 0; i < p_splitter->i_output; i++ )
{
video_splitter_output_t *p_cfg = &p_splitter->p_output[i];
video_format_Copy( &p_cfg->fmt, &p_splitter->fmt );
}
p_splitter->pf_filter = Filter;
p_splitter->mouse = NULL;
msg_Dbg( p_splitter, "spawning %i clone(s)", p_splitter->i_output );
return VLC_SUCCESS;
}
static void Close( vlc_object_t *p_this )
{
video_splitter_t *p_splitter = (video_splitter_t*)p_this;
for( int i = 0; i < p_splitter->i_output; i++ )
{
video_splitter_output_t *p_cfg = &p_splitter->p_output[i];
free( p_cfg->psz_module );
video_format_Clean( &p_cfg->fmt );
}
free( p_splitter->p_output );
}
static int Filter( video_splitter_t *p_splitter,
picture_t *pp_dst[], picture_t *p_src )
{
if( video_splitter_NewPicture( p_splitter, pp_dst ) )
{
picture_Release( p_src );
return VLC_EGENERIC;
}
for( int i = 0; i < p_splitter->i_output; i++ )
picture_Copy( pp_dst[i], p_src );
picture_Release( p_src );
return VLC_SUCCESS;
}
