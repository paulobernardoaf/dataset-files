





















#if !defined(LUA_EXTENSION_H)
#define LUA_EXTENSION_H

#include <vlc_extensions.h>
#include <vlc_arrays.h>
#include <vlc_dialog.h>

#define WATCH_TIMER_PERIOD VLC_TICK_FROM_SEC(10) 


typedef enum
{
CMD_ACTIVATE = 1,
CMD_DEACTIVATE,
CMD_TRIGGERMENU, 
CMD_CLICK, 
CMD_CLOSE,
CMD_SET_INPUT, 
CMD_UPDATE_META, 
CMD_PLAYING_CHANGED 
} command_type_e;


typedef enum
{
LUA_END = 0,
LUA_NUM,
LUA_TEXT
} lua_datatype_e;

struct extension_sys_t
{

int i_capabilities;


lua_State *L;

vlclua_dtable_t dtable;


vlc_thread_t thread;
vlc_mutex_t command_lock;
vlc_mutex_t running_lock;
vlc_cond_t wait;



struct input_item_t *p_item;

extensions_manager_t *p_mgr; 

struct command_t
{
command_type_e i_command;
void *data[10]; 
struct command_t *next; 
} *command;


vlc_dialog_id *p_progress_id;
vlc_timer_t timer; 

bool b_exiting;

bool b_thread_running; 
bool b_activated; 
};


int Activate( extensions_manager_t *p_mgr, extension_t * );
int Deactivate( extensions_manager_t *p_mgr, extension_t * );
bool QueueDeactivateCommand( extension_t *p_ext );
void KillExtension( extensions_manager_t *p_mgr, extension_t *p_ext );
int PushCommand__( extension_t *ext, bool unique, command_type_e cmd, va_list options );
static inline int PushCommand( extension_t *ext, int cmd, ... )
{
va_list args;
va_start( args, cmd );
int i_ret = PushCommand__( ext, false, cmd, args );
va_end( args );
return i_ret;
}
static inline int PushCommandUnique( extension_t *ext, int cmd, ... )
{
va_list args;
va_start( args, cmd );
int i_ret = PushCommand__( ext, true, cmd, args );
va_end( args );
return i_ret;
}


void vlclua_extension_set( lua_State *L, extension_t * );
extension_t *vlclua_extension_get( lua_State *L );
int lua_ExtensionActivate( extensions_manager_t *, extension_t * );
int lua_ExtensionDeactivate( extensions_manager_t *, extension_t * );
int lua_ExecuteFunctionVa( extensions_manager_t *p_mgr, extension_t *p_ext,
const char *psz_function, va_list args );
int lua_ExecuteFunction( extensions_manager_t *p_mgr, extension_t *p_ext,
const char *psz_function, ... );
int lua_ExtensionWidgetClick( extensions_manager_t *p_mgr,
extension_t *p_ext,
extension_widget_t *p_widget );
int lua_ExtensionTriggerMenu( extensions_manager_t *p_mgr,
extension_t *p_ext, int id );


int lua_DialogFlush( lua_State *L );

#endif 
