#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_rand.h>
#include "../vlc.h"
#include "../libs.h"
static int vlclua_rand_number( lua_State *L )
{
long rand = vlc_lrand48();
lua_pushnumber( L, rand );
return 1;
}
static int vlclua_rand_bytes( lua_State *L )
{
lua_Integer i_size = luaL_checkinteger( L, 1 );
char* p_buff = malloc( i_size * sizeof( *p_buff ) );
if ( unlikely( p_buff == NULL ) )
return vlclua_error( L );
vlc_rand_bytes( p_buff, i_size );
lua_pushlstring( L, p_buff, i_size );
free( p_buff );
return 1;
}
static const luaL_Reg vlclua_rand_reg[] = {
{ "number", vlclua_rand_number },
{ "bytes", vlclua_rand_bytes },
{ NULL, NULL }
};
void luaopen_rand( lua_State *L )
{
lua_newtable( L );
luaL_register( L, NULL, vlclua_rand_reg );
lua_setfield( L, -2, "rand" );
}
