#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <assert.h>
#include <errno.h>
#include <vlc/libvlc.h>
#include "libvlc_internal.h"
#include <vlc_common.h>
typedef struct libvlc_event_listener_t
{
libvlc_event_type_t event_type;
void * p_user_data;
libvlc_callback_t pf_callback;
} libvlc_event_listener_t;
void libvlc_event_manager_init(libvlc_event_manager_t *em, void *obj)
{
em->p_obj = obj;
vlc_array_init(&em->listeners);
vlc_mutex_init_recursive(&em->lock);
}
void libvlc_event_manager_destroy(libvlc_event_manager_t *em)
{
for (size_t i = 0; i < vlc_array_count(&em->listeners); i++)
free(vlc_array_item_at_index(&em->listeners, i));
vlc_array_clear(&em->listeners);
}
void libvlc_event_send( libvlc_event_manager_t * p_em,
libvlc_event_t * p_event )
{
p_event->p_obj = p_em->p_obj;
vlc_mutex_lock(&p_em->lock);
for (size_t i = 0; i < vlc_array_count(&p_em->listeners); i++)
{
libvlc_event_listener_t *listener;
listener = vlc_array_item_at_index(&p_em->listeners, i);
if (listener->event_type == p_event->type)
listener->pf_callback(p_event, listener->p_user_data);
}
vlc_mutex_unlock(&p_em->lock);
}
int libvlc_event_attach(libvlc_event_manager_t *em, libvlc_event_type_t type,
libvlc_callback_t callback, void *opaque)
{
libvlc_event_listener_t *listener = malloc(sizeof (*listener));
if (unlikely(listener == NULL))
return ENOMEM;
listener->event_type = type;
listener->p_user_data = opaque;
listener->pf_callback = callback;
int i_ret;
vlc_mutex_lock(&em->lock);
if(vlc_array_append(&em->listeners, listener) != 0)
{
i_ret = VLC_EGENERIC;
free(listener);
}
else
i_ret = VLC_SUCCESS;
vlc_mutex_unlock(&em->lock);
return i_ret;
}
void libvlc_event_detach(libvlc_event_manager_t *em, libvlc_event_type_t type,
libvlc_callback_t callback, void *opaque)
{
vlc_mutex_lock(&em->lock);
for (size_t i = 0; i < vlc_array_count(&em->listeners); i++)
{
libvlc_event_listener_t *listener;
listener = vlc_array_item_at_index(&em->listeners, i);
if (listener->event_type == type
&& listener->pf_callback == callback
&& listener->p_user_data == opaque)
{ 
vlc_array_remove(&em->listeners, i);
vlc_mutex_unlock(&em->lock);
free(listener);
return;
}
}
abort();
}
