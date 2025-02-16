



























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>

#include <assert.h>

#include <vlc_events.h>
#include <vlc_arrays.h>









typedef struct vlc_event_listener_t
{
void * p_user_data;
vlc_event_callback_t pf_callback;
} vlc_event_listener_t;





#undef vlc_event_manager_init






void vlc_event_manager_init( vlc_event_manager_t * p_em, void * p_obj )
{
p_em->p_obj = p_obj;


vlc_mutex_init_recursive( &p_em->lock );

for( size_t i = 0; i < ARRAY_SIZE(p_em->events); i++ )
ARRAY_INIT( p_em->events[i].listeners );
}




void vlc_event_manager_fini( vlc_event_manager_t * p_em )
{
struct vlc_event_listener_t * listener;

for( size_t i = 0; i < ARRAY_SIZE(p_em->events); i++ )
{
struct vlc_event_listeners_group_t *slot = p_em->events + i;

ARRAY_FOREACH( listener, slot->listeners )
free( listener );

ARRAY_RESET( slot->listeners );
}
}




void vlc_event_send( vlc_event_manager_t * p_em,
vlc_event_t * p_event )
{
vlc_event_listeners_group_t *slot = &p_em->events[p_event->type];
vlc_event_listener_t * listener;


p_event->p_obj = p_em->p_obj;

vlc_mutex_lock( &p_em->lock ) ;

ARRAY_FOREACH( listener, slot->listeners )
listener->pf_callback( p_event, listener->p_user_data );

vlc_mutex_unlock( &p_em->lock );
}

#undef vlc_event_attach



int vlc_event_attach( vlc_event_manager_t * p_em,
vlc_event_type_t event_type,
vlc_event_callback_t pf_callback,
void *p_user_data )
{
vlc_event_listener_t * listener;
vlc_event_listeners_group_t *slot = &p_em->events[event_type];

listener = malloc(sizeof(vlc_event_listener_t));
if( !listener )
return VLC_ENOMEM;

listener->p_user_data = p_user_data;
listener->pf_callback = pf_callback;

vlc_mutex_lock( &p_em->lock );
ARRAY_APPEND( slot->listeners, listener );
vlc_mutex_unlock( &p_em->lock );
return VLC_SUCCESS;
}





void vlc_event_detach( vlc_event_manager_t *p_em,
vlc_event_type_t event_type,
vlc_event_callback_t pf_callback,
void *p_user_data )
{
vlc_event_listeners_group_t *slot = &p_em->events[event_type];

vlc_mutex_lock( &p_em->lock );

for (int i = 0; i < slot->listeners.i_size; ++i)
{
struct vlc_event_listener_t *listener = slot->listeners.p_elems[i];
if( listener->pf_callback == pf_callback &&
listener->p_user_data == p_user_data )
{

ARRAY_REMOVE( slot->listeners, i );
vlc_mutex_unlock( &p_em->lock );
free( listener );
return;
}
}

vlc_assert_unreachable();
}
