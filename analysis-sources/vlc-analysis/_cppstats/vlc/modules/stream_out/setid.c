#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <stdlib.h>
#include <string.h>
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_sout.h>
#define ID_TEXT N_("Elementary Stream ID")
#define ID_LONGTEXT N_( "Specify an identifier integer for this elementary stream" )
#define NEWID_TEXT N_("New ES ID")
#define NEWID_LONGTEXT N_( "Specify an new identifier integer for this elementary stream" )
#define LANG_TEXT N_("Language")
#define LANG_LONGTEXT N_( "Specify an ISO-639 code (three characters) for this elementary stream" )
static int OpenId ( vlc_object_t * );
static int OpenLang ( vlc_object_t * );
static void Close ( vlc_object_t * );
#define SOUT_CFG_PREFIX_ID "sout-setid-"
#define SOUT_CFG_PREFIX_LANG "sout-setlang-"
vlc_module_begin()
set_shortname( N_("Set ID"))
set_section( N_("Set ES id"), NULL )
set_description( N_("Change the id of an elementary stream"))
set_capability( "sout filter", 50 )
add_shortcut( "setid" )
set_category( CAT_SOUT )
set_subcategory( SUBCAT_SOUT_STREAM )
set_callbacks( OpenId, Close )
add_integer( SOUT_CFG_PREFIX_ID "id", 0, ID_TEXT, ID_LONGTEXT, false )
add_integer( SOUT_CFG_PREFIX_ID "new-id", 0, NEWID_TEXT, NEWID_LONGTEXT,
false )
add_submodule ()
set_section( N_("Set ES Lang"), NULL )
set_shortname( N_("Set Lang"))
set_description( N_("Change the language of an elementary stream"))
set_capability( "sout filter", 50 )
add_shortcut( "setlang" );
set_callbacks( OpenLang, Close )
add_integer( SOUT_CFG_PREFIX_LANG "id", 0, ID_TEXT, ID_LONGTEXT, false )
add_string( SOUT_CFG_PREFIX_LANG "lang", "eng", LANG_TEXT, LANG_LONGTEXT,
false );
vlc_module_end()
static const char *ppsz_sout_options_id[] = {
"id", "new-id", NULL
};
static const char *ppsz_sout_options_lang[] = {
"id", "lang", NULL
};
static void *AddId ( sout_stream_t *, const es_format_t * );
static void *AddLang( sout_stream_t *, const es_format_t * );
static void Del ( sout_stream_t *, void * );
static int Send ( sout_stream_t *, void *, block_t * );
typedef struct
{
int i_id;
int i_new_id;
char *psz_language;
} sout_stream_sys_t;
static int OpenCommon( vlc_object_t *p_this )
{
sout_stream_t *p_stream = (sout_stream_t*)p_this;
sout_stream_sys_t *p_sys;
p_sys = malloc( sizeof( sout_stream_sys_t ) );
if( unlikely( !p_sys ) )
return VLC_ENOMEM;
p_stream->pf_del = Del;
p_stream->pf_send = Send;
p_stream->p_sys = p_sys;
return VLC_SUCCESS;
}
static int OpenId( vlc_object_t *p_this )
{
int i_ret = OpenCommon( p_this );
if( i_ret != VLC_SUCCESS )
return i_ret;
sout_stream_t *p_stream = (sout_stream_t*)p_this;
config_ChainParse( p_stream, SOUT_CFG_PREFIX_ID, ppsz_sout_options_id,
p_stream->p_cfg );
sout_stream_sys_t *p_sys = p_stream->p_sys;
p_sys->i_id = var_GetInteger( p_stream, SOUT_CFG_PREFIX_ID "id" );
p_sys->i_new_id = var_GetInteger( p_stream, SOUT_CFG_PREFIX_ID "new-id" );
p_sys->psz_language = NULL;
p_stream->pf_add = AddId;
return VLC_SUCCESS;
}
static int OpenLang( vlc_object_t *p_this )
{
int i_ret = OpenCommon( p_this );
if( i_ret != VLC_SUCCESS )
return i_ret;
sout_stream_t *p_stream = (sout_stream_t*)p_this;
config_ChainParse( p_stream, SOUT_CFG_PREFIX_LANG, ppsz_sout_options_lang,
p_stream->p_cfg );
sout_stream_sys_t *p_sys = p_stream->p_sys;
p_sys->i_id = var_GetInteger( p_stream, SOUT_CFG_PREFIX_LANG "id" );
p_sys->i_new_id = -1;
p_sys->psz_language = var_GetString( p_stream, SOUT_CFG_PREFIX_LANG "lang" );
p_stream->pf_add = AddLang;
return VLC_SUCCESS;
}
static void Close( vlc_object_t * p_this )
{
sout_stream_t *p_stream = (sout_stream_t*)p_this;
sout_stream_sys_t *p_sys = (sout_stream_sys_t *)p_stream->p_sys;
free( p_sys->psz_language );
free( p_sys );
}
static void *AddId( sout_stream_t *p_stream, const es_format_t *p_fmt )
{
sout_stream_sys_t *p_sys = (sout_stream_sys_t *)p_stream->p_sys;
es_format_t fmt;
if( p_fmt->i_id == p_sys->i_id )
{
msg_Dbg( p_stream, "turning ID %d to %d", p_sys->i_id,
p_sys->i_new_id );
fmt = *p_fmt;
fmt.i_id = p_sys->i_new_id;
p_fmt = &fmt;
}
return sout_StreamIdAdd( p_stream->p_next, p_fmt );
}
static void *AddLang( sout_stream_t *p_stream, const es_format_t *p_fmt )
{
sout_stream_sys_t *p_sys = (sout_stream_sys_t *)p_stream->p_sys;
es_format_t fmt;
if ( p_fmt->i_id == p_sys->i_id )
{
msg_Dbg( p_stream, "turning language %s of ID %d to %s",
p_fmt->psz_language ? p_fmt->psz_language : "unk",
p_sys->i_id, p_sys->psz_language );
fmt = *p_fmt;
fmt.psz_language = p_sys->psz_language;
p_fmt = &fmt;
}
return sout_StreamIdAdd( p_stream->p_next, p_fmt );
}
static void Del( sout_stream_t *p_stream, void *id )
{
sout_StreamIdDel( p_stream->p_next, id );
}
static int Send( sout_stream_t *p_stream, void *id, block_t *p_buffer )
{
return sout_StreamIdSend( p_stream->p_next, id, p_buffer );
}
