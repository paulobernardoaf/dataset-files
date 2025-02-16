#include <vlc/libvlc.h>
#include <vlc/libvlc_media.h>
#include <vlc_common.h>
#include <vlc_input.h>
struct libvlc_media_t
{
libvlc_event_manager_t event_manager;
input_item_t *p_input_item;
int i_refcount;
libvlc_instance_t *p_libvlc_instance;
libvlc_state_t state;
VLC_FORWARD_DECLARE_OBJECT(libvlc_media_list_t*) p_subitems; 
void *p_user_data;
vlc_cond_t parsed_cond;
vlc_mutex_t parsed_lock;
vlc_mutex_t subitems_lock;
libvlc_media_parsed_status_t parsed_status;
bool is_parsed;
bool has_asked_preparse;
};
libvlc_media_t * libvlc_media_new_from_input_item(
libvlc_instance_t *, input_item_t * );
void libvlc_media_set_state( libvlc_media_t *, libvlc_state_t );
void libvlc_media_add_subtree(libvlc_media_t *, input_item_node_t *);
