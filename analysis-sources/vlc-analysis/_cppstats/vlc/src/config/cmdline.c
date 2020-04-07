#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include "../libvlc.h"
#include <vlc_actions.h>
#include <vlc_charset.h>
#include <vlc_modules.h>
#include <vlc_plugin.h>
#include "vlc_getopt.h"
#include "configuration.h"
#include "modules/modules.h"
#include <assert.h>
#undef config_LoadCmdLine
int config_LoadCmdLine( vlc_object_t *p_this, int i_argc,
const char *ppsz_argv[], int *pindex )
{
int i_cmd, i_index, i_opts, i_shortopts, flag, i_verbose = 0;
struct vlc_option *p_longopts;
const char **argv_copy = NULL;
#define b_ignore_errors (pindex == NULL)
const module_config_t *pp_shortopts[256];
char *psz_shortopts;
i_opts = 0;
for (const vlc_plugin_t *p = vlc_plugins; p != NULL; p = p->next)
i_opts += p->conf.count + 2 * p->conf.booleans;
p_longopts = vlc_alloc( i_opts + 1, sizeof(*p_longopts) );
if( p_longopts == NULL )
return -1;
psz_shortopts = malloc( 2 * i_opts + 1 );
if( psz_shortopts == NULL )
{
free( p_longopts );
return -1;
}
if( b_ignore_errors )
{
argv_copy = vlc_alloc( i_argc, sizeof(char *) );
if( argv_copy == NULL )
{
free( psz_shortopts );
free( p_longopts );
return -1;
}
memcpy( argv_copy, ppsz_argv, i_argc * sizeof(char *) );
ppsz_argv = argv_copy;
}
i_shortopts = 0;
for( i_index = 0; i_index < 256; i_index++ )
{
pp_shortopts[i_index] = NULL;
}
i_index = 0;
for (const vlc_plugin_t *p = vlc_plugins; p != NULL; p = p->next)
{
for (const module_config_t *p_item = p->conf.items,
*p_end = p_item + p->conf.size;
p_item < p_end;
p_item++)
{
if( !CONFIG_ITEM(p_item->i_type) )
continue;
p_longopts[i_index].name = strdup( p_item->psz_name );
if( p_longopts[i_index].name == NULL ) continue;
p_longopts[i_index].flag = &flag;
p_longopts[i_index].val = 0;
if( CONFIG_CLASS(p_item->i_type) != CONFIG_ITEM_BOOL )
p_longopts[i_index].has_arg = true;
else
{
char *psz_name;
p_longopts[i_index].has_arg = false;
i_index++;
if( asprintf( &psz_name, "no%s", p_item->psz_name ) == -1 )
continue;
p_longopts[i_index].name = psz_name;
p_longopts[i_index].has_arg = false;
p_longopts[i_index].flag = &flag;
p_longopts[i_index].val = 1;
i_index++;
if( asprintf( &psz_name, "no-%s", p_item->psz_name ) == -1 )
continue;
p_longopts[i_index].name = psz_name;
p_longopts[i_index].has_arg = false;
p_longopts[i_index].flag = &flag;
p_longopts[i_index].val = 1;
}
i_index++;
if( p_item->i_short )
{
pp_shortopts[(int)p_item->i_short] = p_item;
psz_shortopts[i_shortopts] = p_item->i_short;
i_shortopts++;
if( p_item->i_type != CONFIG_ITEM_BOOL
&& p_item->i_short != 'v' )
{
psz_shortopts[i_shortopts] = ':';
i_shortopts++;
}
}
}
}
memset( &p_longopts[i_index], 0, sizeof(*p_longopts) );
psz_shortopts[i_shortopts] = '\0';
int ret = -1;
vlc_getopt_t state;
state.ind = 0 ; 
while( ( i_cmd = vlc_getopt_long( i_argc, (char **)ppsz_argv,
psz_shortopts,
p_longopts, &i_index, &state ) ) != -1 )
{
if( i_cmd == 0 )
{
module_config_t *p_conf;
const char *psz_name = p_longopts[i_index].name;
if( flag ) psz_name += psz_name[2] == '-' ? 3 : 2;
p_conf = config_FindConfig( psz_name );
if( p_conf )
{
if( p_conf->b_removed )
{
fprintf(stderr,
"Warning: option --%s no longer exists.\n",
psz_name);
continue;
}
switch( CONFIG_CLASS(p_conf->i_type) )
{
case CONFIG_ITEM_STRING:
var_Create( p_this, psz_name, VLC_VAR_STRING );
var_SetString( p_this, psz_name, state.arg );
break;
case CONFIG_ITEM_INTEGER:
var_Create( p_this, psz_name, VLC_VAR_INTEGER );
var_Change( p_this, psz_name, VLC_VAR_SETMINMAX,
(vlc_value_t){ .i_int = p_conf->min.i },
(vlc_value_t){ .i_int = p_conf->max.i } );
var_SetInteger( p_this, psz_name,
strtoll(state.arg, NULL, 0));
break;
case CONFIG_ITEM_FLOAT:
var_Create( p_this, psz_name, VLC_VAR_FLOAT );
var_Change( p_this, psz_name, VLC_VAR_SETMINMAX,
(vlc_value_t){ .f_float = p_conf->min.f },
(vlc_value_t){ .f_float = p_conf->max.f } );
var_SetFloat( p_this, psz_name, us_atof(state.arg) );
break;
case CONFIG_ITEM_BOOL:
var_Create( p_this, psz_name, VLC_VAR_BOOL );
var_SetBool( p_this, psz_name, !flag );
break;
}
continue;
}
}
if( pp_shortopts[i_cmd] != NULL )
{
const char *name = pp_shortopts[i_cmd]->psz_name;
switch( CONFIG_CLASS(pp_shortopts[i_cmd]->i_type) )
{
case CONFIG_ITEM_STRING:
var_Create( p_this, name, VLC_VAR_STRING );
var_SetString( p_this, name, state.arg );
break;
case CONFIG_ITEM_INTEGER:
var_Create( p_this, name, VLC_VAR_INTEGER );
if( i_cmd == 'v' )
{
i_verbose++; 
var_SetInteger( p_this, name, i_verbose );
}
else
{
var_SetInteger( p_this, name,
strtoll(state.arg, NULL, 0) );
}
break;
case CONFIG_ITEM_BOOL:
var_Create( p_this, name, VLC_VAR_BOOL );
var_SetBool( p_this, name, true );
break;
}
continue;
}
if( !b_ignore_errors )
{
fputs( "vlc: unknown option"
" or missing mandatory argument ", stderr );
if( state.opt )
{
fprintf( stderr, "`-%c'\n", state.opt );
}
else
{
fprintf( stderr, "`%s'\n", ppsz_argv[state.ind-1] );
}
fputs( "Try `vlc --help' for more information.\n", stderr );
goto out;
}
}
ret = 0;
if( pindex != NULL )
*pindex = state.ind;
out:
for( i_index = 0; p_longopts[i_index].name; i_index++ )
free( (char *)p_longopts[i_index].name );
free( p_longopts );
free( psz_shortopts );
free( argv_copy );
return ret;
}
