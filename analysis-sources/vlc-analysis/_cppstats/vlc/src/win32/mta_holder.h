#include <vlc_common.h>
#include <assert.h>
#include <windows.h>
#include <objbase.h>
typedef struct vlc_mta_holder
{
vlc_thread_t thread;
int i_refcount;
vlc_sem_t ready_sem;
vlc_sem_t release_sem;
} vlc_mta_holder;
static inline void* MtaMainLoop( void* opaque )
{
vlc_mta_holder* p_mta = (vlc_mta_holder*)opaque;
CoInitializeEx( NULL, COINIT_MULTITHREADED );
vlc_sem_post( &p_mta->ready_sem );
vlc_sem_wait( &p_mta->release_sem );
CoUninitialize();
return NULL;
}
static inline bool vlc_mta_acquire( vlc_object_t *p_parent )
{
vlc_object_t *vlc = VLC_OBJECT(vlc_object_instance(p_parent));
vlc_global_lock( VLC_MTA_MUTEX );
vlc_mta_holder* p_mta = (vlc_mta_holder*)var_CreateGetAddress( vlc, "mta-holder" );
if ( p_mta == NULL )
{
p_mta = (vlc_mta_holder*)malloc( sizeof( *p_mta ) );
if ( unlikely( p_mta == NULL ) )
{
vlc_global_unlock( VLC_MTA_MUTEX );
return false;
}
vlc_sem_init( &p_mta->ready_sem, 0 );
vlc_sem_init( &p_mta->release_sem, 0 );
p_mta->i_refcount = 1;
if ( vlc_clone( &p_mta->thread, MtaMainLoop, p_mta, VLC_THREAD_PRIORITY_LOW ) )
{
free( p_mta );
p_mta = NULL;
vlc_global_unlock( VLC_MTA_MUTEX );
return false;
}
var_SetAddress( vlc, "mta-holder", p_mta );
vlc_sem_wait( &p_mta->ready_sem );
}
else
++p_mta->i_refcount;
vlc_global_unlock( VLC_MTA_MUTEX );
return true;
}
static inline void vlc_mta_release( vlc_object_t* p_parent )
{
vlc_object_t *vlc = VLC_OBJECT(vlc_object_instance(p_parent));
vlc_global_lock( VLC_MTA_MUTEX );
vlc_mta_holder *p_mta = (vlc_mta_holder*)var_InheritAddress( vlc, "mta-holder" );
assert( p_mta != NULL );
int i_refcount = --p_mta->i_refcount;
if ( i_refcount == 0 )
var_SetAddress( vlc, "mta-holder", NULL );
vlc_global_unlock( VLC_MTA_MUTEX );
if ( i_refcount == 0 )
{
vlc_sem_post( &p_mta->release_sem );
vlc_join( p_mta->thread, NULL );
free( p_mta );
}
}
