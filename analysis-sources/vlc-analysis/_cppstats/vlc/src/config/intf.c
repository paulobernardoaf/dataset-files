#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <assert.h>
void config_AddIntf( const char *psz_intf )
{
assert( psz_intf );
char *psz_config, *psz_parser;
size_t i_len = strlen( psz_intf );
psz_config = psz_parser = config_GetPsz( "control" );
while( psz_parser )
{
if( !strncmp( psz_intf, psz_parser, i_len ) )
{
free( psz_config );
return;
}
psz_parser = strchr( psz_parser, ':' );
if( psz_parser ) psz_parser++; 
}
free( psz_config );
psz_config = psz_parser = config_GetPsz( "extraintf" );
while( psz_parser )
{
if( !strncmp( psz_intf, psz_parser, i_len ) )
{
free( psz_config );
return;
}
psz_parser = strchr( psz_parser, ':' );
if( psz_parser ) psz_parser++; 
}
if( psz_config && strlen( psz_config ) > 0 )
{
char *psz_newconfig;
if( asprintf( &psz_newconfig, "%s:%s", psz_config, psz_intf ) != -1 )
{
config_PutPsz( "extraintf", psz_newconfig );
free( psz_newconfig );
}
}
else
config_PutPsz( "extraintf", psz_intf );
free( psz_config );
}
void config_RemoveIntf( const char *psz_intf )
{
assert( psz_intf );
char *psz_config, *psz_parser;
size_t i_len = strlen( psz_intf );
psz_config = psz_parser = config_GetPsz( "extraintf" );
while( psz_parser )
{
if( !strncmp( psz_intf, psz_parser, i_len ) )
{
char *psz_newconfig;
char *psz_end = psz_parser + i_len;
if( *psz_end == ':' ) psz_end++;
*psz_parser = '\0';
if( asprintf( &psz_newconfig, "%s%s", psz_config, psz_end ) != -1 )
{
config_PutPsz( "extraintf", psz_newconfig );
free( psz_newconfig );
}
break;
}
psz_parser = strchr( psz_parser, ':' );
if( psz_parser ) psz_parser++; 
}
free( psz_config );
psz_config = psz_parser = config_GetPsz( "control" );
while( psz_parser )
{
if( !strncmp( psz_intf, psz_parser, i_len ) )
{
char *psz_newconfig;
char *psz_end = psz_parser + i_len;
if( *psz_end == ':' ) psz_end++;
*psz_parser = '\0';
if( asprintf( &psz_newconfig, "%s%s", psz_config, psz_end ) != -1 )
{
config_PutPsz( "control", psz_newconfig );
free( psz_newconfig );
}
break;
}
psz_parser = strchr( psz_parser, ':' );
if( psz_parser ) psz_parser++; 
}
free( psz_config );
}
bool config_ExistIntf( const char *psz_intf )
{
assert( psz_intf );
char *psz_config, *psz_parser;
size_t i_len = strlen( psz_intf );
psz_config = psz_parser = config_GetPsz( "extraintf" );
while( psz_parser )
{
if( !strncmp( psz_parser, psz_intf, i_len ) )
{
free( psz_config );
return true;
}
psz_parser = strchr( psz_parser, ':' );
if( psz_parser ) psz_parser++; 
}
free( psz_config );
psz_config = psz_parser = config_GetPsz( "control" );
while( psz_parser )
{
if( !strncmp( psz_parser, psz_intf, i_len ) )
{
free( psz_config );
return true;
}
psz_parser = strchr( psz_parser, ':' );
if( psz_parser ) psz_parser++; 
}
free( psz_config );
return false;
}
