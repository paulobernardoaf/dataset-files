



























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include "libvlc.h"
#include <vlc_charset.h>
#include <vlc_plugin.h>

#include "vlc_interface.h"
#include "configuration.h"




static bool IsEscapeNeeded( char c )
{
return c == '\'' || c == '"' || c == '\\';
}
static bool IsEscape( const char *psz )
{
if( !psz )
return false;
return psz[0] == '\\' && IsEscapeNeeded( psz[1] );
}
static bool IsSpace( char c )
{
return c == ' ' || c == '\t';
}

#define SKIPSPACE( p ) p += strspn( p, " \t" )
#define SKIPTRAILINGSPACE( p, e ) do { while( e > p && IsSpace( *(e-1) ) ) e--; } while(0)









static const char *ChainGetEnd( const char *psz_string )
{
const char *p = psz_string;
char c;

if( !psz_string )
return NULL;


SKIPSPACE( p );

for( ;; p++)
{
if( *p == '\0' || *p == ',' || *p == '}' )
return p;

if( *p == '{' || *p == '"' || *p == '\'' )
break;
}


if( *p == '{' )
c = '}';
else
c = *p;
p++;


for( ;; )
{
if( *p == '\0')
return p;

if( IsEscape( p ) )
p += 2;
else if( *p == c )
return ++p;
else if( *p == '{' && c == '}' )
p = ChainGetEnd( p );
else
p++;
}
}





static char *ChainGetValue( const char **ppsz_string )
{
const char *p = *ppsz_string;

char *psz_value = NULL;
const char *end;
bool b_keep_brackets = (*p == '{');

if( *p == '=' )
p++;

end = ChainGetEnd( p );
if( end <= p )
{
psz_value = NULL;
}
else
{



SKIPSPACE( p );
}

if( end <= p )
{
psz_value = NULL;
}
else
{
if( *p == '\'' || *p == '"' || ( !b_keep_brackets && *p == '{' ) )
{
p++;

if( *(end-1) != '\'' && *(end-1) == '"' )
SKIPTRAILINGSPACE( p, end );

if( end - 1 <= p )
psz_value = NULL;
else
psz_value = strndup( p, end -1 - p );
}
else
{
SKIPTRAILINGSPACE( p, end );
if( end <= p )
psz_value = NULL;
else
psz_value = strndup( p, end - p );
}
}


if( psz_value )
config_StringUnescape( psz_value );


*ppsz_string = end;
return psz_value;
}


const char *config_ChainParseOptions( config_chain_t **pp_cfg, const char *psz_opts )
{
config_chain_t **pp_next = pp_cfg;
bool first = true;
do
{
if (!first)
psz_opts++; 
SKIPSPACE( psz_opts );

first = false;


size_t len = strcspn( psz_opts, "=,{} \t" );
if( len == 0 )
continue; 


config_chain_t *p_cfg = malloc( sizeof(*p_cfg) );
if( !p_cfg )
break;
p_cfg->psz_name = strndup( psz_opts, len );
psz_opts += len;
p_cfg->psz_value = NULL;
p_cfg->p_next = NULL;

*pp_next = p_cfg;
pp_next = &p_cfg->p_next;


SKIPSPACE( psz_opts );
if( strchr( "={", *psz_opts ) )
{
p_cfg->psz_value = ChainGetValue( &psz_opts );
SKIPSPACE( psz_opts );
}
}
while( !memchr( "}", *psz_opts, 2 ) );

if( *psz_opts ) psz_opts++; ;
SKIPSPACE( psz_opts );

return psz_opts;
}

char *config_ChainCreate( char **ppsz_name, config_chain_t **pp_cfg,
const char *psz_chain )
{
size_t len;

*ppsz_name = NULL;
*pp_cfg = NULL;

if( !psz_chain )
return NULL;
SKIPSPACE( psz_chain );


len = strcspn( psz_chain, "{: \t" );
*ppsz_name = strndup( psz_chain, len );
psz_chain += len;


SKIPSPACE( psz_chain );
if( *psz_chain == '{' )
psz_chain = config_ChainParseOptions( pp_cfg, psz_chain );

if( *psz_chain == ':' )
return strdup( psz_chain + 1 );

return NULL;
}

void config_ChainDestroy( config_chain_t *p_cfg )
{
while( p_cfg != NULL )
{
config_chain_t *p_next;

p_next = p_cfg->p_next;

FREENULL( p_cfg->psz_name );
FREENULL( p_cfg->psz_value );
free( p_cfg );

p_cfg = p_next;
}
}

#undef config_ChainParse
void config_ChainParse( vlc_object_t *p_this, const char *psz_prefix,
const char *const *ppsz_options, const config_chain_t *cfg )
{
if( psz_prefix == NULL ) psz_prefix = "";
size_t plen = 1 + strlen( psz_prefix );


for( size_t i = 0; ppsz_options[i] != NULL; i++ )
{
const char *optname = ppsz_options[i];
if (optname[0] == '*')
optname++;

char name[plen + strlen( optname )];
snprintf( name, sizeof (name), "%s%s", psz_prefix, optname );
if( var_Create( p_this, name,
config_GetType( name ) | VLC_VAR_DOINHERIT ) )
return ;

module_config_t* p_conf = config_FindConfig( name );
if( p_conf )
{
switch( CONFIG_CLASS( p_conf->i_type ) )
{
case CONFIG_ITEM_INTEGER:
var_Change( p_this, name, VLC_VAR_SETMINMAX,
(vlc_value_t){ .i_int = p_conf->min.i },
(vlc_value_t){ .i_int = p_conf->max.i } );
break;
case CONFIG_ITEM_FLOAT:
var_Change( p_this, name, VLC_VAR_SETMINMAX,
(vlc_value_t){ .f_float = p_conf->min.f },
(vlc_value_t){ .f_float = p_conf->max.f } );
break;
}
}
}


for(; cfg; cfg = cfg->p_next )
{
vlc_value_t val;
bool b_yes = true;
bool b_once = false;
module_config_t *p_conf;
int i_type;
size_t i;

if( cfg->psz_name == NULL || *cfg->psz_name == '\0' )
continue;

for( i = 0; ppsz_options[i] != NULL; i++ )
{
if( !strcmp( ppsz_options[i], cfg->psz_name ) )
{
break;
}
if( ( !strncmp( cfg->psz_name, "no-", 3 ) &&
!strcmp( ppsz_options[i], cfg->psz_name + 3 ) ) ||
( !strncmp( cfg->psz_name, "no", 2 ) &&
!strcmp( ppsz_options[i], cfg->psz_name + 2 ) ) )
{
b_yes = false;
break;
}

if( *ppsz_options[i] == '*' &&
!strcmp( &ppsz_options[i][1], cfg->psz_name ) )
{
b_once = true;
break;
}

}

if( ppsz_options[i] == NULL )
{
msg_Warn( p_this, "option %s is unknown", cfg->psz_name );
continue;
}


char name[plen + strlen( ppsz_options[i] )];
const char *psz_name = name;
snprintf( name, sizeof (name), "%s%s", psz_prefix,
b_once ? (ppsz_options[i] + 1) : ppsz_options[i] );


p_conf = config_FindConfig( name );



if( p_conf )
{
if( p_conf->b_removed )
{
msg_Err( p_this, "Option %s is not supported anymore.",
name );



continue;
}
}



i_type = config_GetType( psz_name );
if( !i_type )
{
msg_Warn( p_this, "unknown option %s (value=%s)",
cfg->psz_name, cfg->psz_value );
continue;
}

if( i_type != VLC_VAR_BOOL && cfg->psz_value == NULL )
{
msg_Warn( p_this, "missing value for option %s", cfg->psz_name );
continue;
}
if( i_type != VLC_VAR_STRING && b_once )
{
msg_Warn( p_this, "*option_name need to be a string option" );
continue;
}

switch( i_type )
{
case VLC_VAR_BOOL:
val.b_bool = b_yes;
break;
case VLC_VAR_INTEGER:
val.i_int = strtol( cfg->psz_value ? cfg->psz_value : "0",
NULL, 0 );
break;
case VLC_VAR_FLOAT:
val.f_float = us_atof( cfg->psz_value ? cfg->psz_value : "0" );
break;
case VLC_VAR_STRING:
val.psz_string = cfg->psz_value;
break;
default:
msg_Warn( p_this, "unhandled config var type (%d)", i_type );
memset( &val, 0, sizeof( vlc_value_t ) );
break;
}
if( b_once )
{
vlc_value_t val2;

var_Get( p_this, psz_name, &val2 );
if( *val2.psz_string )
{
free( val2.psz_string );
msg_Dbg( p_this, "ignoring option %s (not first occurrence)", psz_name );
continue;
}
free( val2.psz_string );
}
var_Set( p_this, psz_name, val );
msg_Dbg( p_this, "set config option: %s to %s", psz_name,
cfg->psz_value ? cfg->psz_value : "(null)" );
}
}

config_chain_t *config_ChainDuplicate( const config_chain_t *p_src )
{
config_chain_t *p_dst = NULL;
config_chain_t **pp_last = &p_dst;

for( ; p_src != NULL; p_src = p_src->p_next )
{
config_chain_t *p = malloc( sizeof(*p) );
if( !p )
break;
p->p_next = NULL;
p->psz_name = p_src->psz_name ? strdup( p_src->psz_name ) : NULL;
p->psz_value = p_src->psz_value ? strdup( p_src->psz_value ) : NULL;

*pp_last = p;
pp_last = &p->p_next;
}
return p_dst;
}

char *config_StringUnescape( char *psz_string )
{
char *psz_src = psz_string;
char *psz_dst = psz_string;
if( !psz_src )
return NULL;

while( *psz_src )
{
if( IsEscape( psz_src ) )
psz_src++;
*psz_dst++ = *psz_src++;
}
*psz_dst = '\0';

return psz_string;
}

char *config_StringEscape( const char *str )
{
size_t length = 0;

if( str == NULL )
return NULL;

for( const char *p = str; *p; p++ )
length += IsEscapeNeeded( *p ) ? 2 : 1;

char *ret = malloc( length + 1 ), *dst = ret;

if( unlikely( !ret ) )
return NULL;

for( const char *p = str; *p; p++ )
{
if( IsEscapeNeeded( *p ) )
*dst++ = '\\';
*dst++ = *p;
}
*dst = '\0';;
return ret;
}
