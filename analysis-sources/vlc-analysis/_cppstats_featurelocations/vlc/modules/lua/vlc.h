






















#if !defined(VLC_LUA_H)
#define VLC_LUA_H




#include <vlc_common.h>
#include <vlc_input.h>
#include <vlc_playlist.h>
#include <vlc_meta.h>
#include <vlc_meta_fetcher.h>
#include <vlc_url.h>
#include <vlc_strings.h>
#include <vlc_stream.h>
#include <vlc_demux.h>

#define LUA_COMPAT_MODULE
#include <lua.h> 
#include <lauxlib.h> 
#include <lualib.h> 

#if LUA_VERSION_NUM >= 502
#define lua_equal(L,idx1,idx2) lua_compare(L,(idx1),(idx2),LUA_OPEQ)
#define lua_objlen(L,idx) lua_rawlen(L,idx)
#define lua_strlen(L,idx) lua_rawlen(L,idx)
#endif

#if LUA_VERSION_NUM >= 503
#undef luaL_register
#define luaL_register(L, n, l) luaL_setfuncs(L, (l), 0)
#define luaL_register_namespace(L, n, l) lua_getglobal( L, n ); if( lua_isnil( L, -1 ) ) { lua_pop( L, 1 ); lua_newtable( L ); } luaL_setfuncs( L, (l), 0 ); lua_pushvalue( L, -1 ); lua_setglobal( L, n );









#else
#define luaL_register_namespace(L, n, l) luaL_register( L, n, (l) );
#endif





int ReadMeta( demux_meta_t * );
int FetchMeta( meta_fetcher_t * );
int FindArt( meta_fetcher_t * );

int Import_LuaPlaylist( vlc_object_t * );
void Close_LuaPlaylist( vlc_object_t * );

#define TELNETPORT_DEFAULT 4212
int Open_LuaIntf( vlc_object_t * );
void Close_LuaIntf( vlc_object_t * );
int Open_LuaHTTP( vlc_object_t * );
int Open_LuaCLI( vlc_object_t * );
int Open_LuaTelnet( vlc_object_t * );


int Open_Extension( vlc_object_t * );
void Close_Extension( vlc_object_t * );

int Open_LuaSD( vlc_object_t * );
void Close_LuaSD( vlc_object_t * );


int vlclua_probe_sd( vlc_object_t *, const char *name );




static inline void lua_Dbg( vlc_object_t * p_this, const char * ppz_fmt, ... )
{
va_list ap;
va_start( ap, ppz_fmt );
msg_GenericVa( p_this, VLC_MSG_DBG, ppz_fmt, ap );
va_end( ap );
}

static inline bool lua_Disabled( vlc_object_t *p_this )
{
return !var_InheritBool( p_this, "lua" );
}
#define lua_Disabled( x ) lua_Disabled( VLC_OBJECT( x ) )




static inline bool luaL_checkboolean( lua_State *L, int narg )
{
luaL_checktype( L, narg, LUA_TBOOLEAN ); 
return lua_toboolean( L, narg );
}

static inline int luaL_optboolean( lua_State *L, int narg, int def )
{
return luaL_opt( L, luaL_checkboolean, narg, def );
}

static inline const char *luaL_nilorcheckstring( lua_State *L, int narg )
{
if( lua_isnil( L, narg ) )
return NULL;
return luaL_checkstring( L, narg );
}

static inline char *luaL_strdupornull( lua_State *L, int narg )
{
if( lua_isstring( L, narg ) )
return strdup( luaL_checkstring( L, narg ) );
return NULL;
}

void vlclua_set_this( lua_State *, vlc_object_t * );
#define vlclua_set_this(a, b) vlclua_set_this(a, VLC_OBJECT(b))
vlc_object_t * vlclua_get_this( lua_State * );

void vlclua_set_playlist_internal( lua_State *, vlc_playlist_t * );
vlc_playlist_t * vlclua_get_playlist_internal( lua_State * );




#define vlclua_error( L ) luaL_error( L, "VLC lua error in file %s line %d (function %s)", __FILE__, __LINE__, __func__ )
int vlclua_push_ret( lua_State *, int i_error );





typedef struct luabatch_context_t luabatch_context_t;
struct luabatch_context_t
{
input_item_t *p_item;
meta_fetcher_scope_t e_scope;
bool (*pf_validator)( const luabatch_context_t *, meta_fetcher_scope_t );
};

int vlclua_scripts_batch_execute( vlc_object_t *p_this, const char * luadirname,
int (*func)(vlc_object_t *, const char *, const luabatch_context_t *),
void * user_data );
int vlclua_dir_list( const char *luadirname, char ***pppsz_dir_list );
void vlclua_dir_list_free( char **ppsz_dir_list );
char *vlclua_find_file( const char *psz_luadirname, const char *psz_name );




int vlclua_dofile( vlc_object_t *p_this, lua_State *L, const char *url );




void vlclua_read_options( vlc_object_t *, lua_State *, int *, char *** );
#define vlclua_read_options( a, b, c, d ) vlclua_read_options( VLC_OBJECT( a ), b, c, d )
void vlclua_read_meta_data( vlc_object_t *, lua_State *, input_item_t * );
#define vlclua_read_meta_data( a, b, c ) vlclua_read_meta_data( VLC_OBJECT( a ), b, c )
void vlclua_read_custom_meta_data( vlc_object_t *, lua_State *,
input_item_t *);
#define vlclua_read_custom_meta_data( a, b, c ) vlclua_read_custom_meta_data( VLC_OBJECT( a ), b, c )

input_item_t *vlclua_read_input_item(vlc_object_t *, lua_State *);

int vlclua_add_modules_path( lua_State *, const char *psz_filename );

struct vlc_interrupt;




typedef struct
{
struct vlc_interrupt *interrupt;
int *fdv;
unsigned fdc;
} vlclua_dtable_t;

int vlclua_fd_init( lua_State *, vlclua_dtable_t * );
void vlclua_fd_interrupt( vlclua_dtable_t * );
void vlclua_fd_cleanup( vlclua_dtable_t * );
struct vlc_interrupt *vlclua_set_interrupt( lua_State *L );

#endif 

