






















#if !defined(_LIBVLC_MEDIA_LIST_INTERNAL_H)
#define _LIBVLC_MEDIA_LIST_INTERNAL_H 1

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc/vlc.h>
#include <vlc/libvlc_media.h>

#include <vlc_common.h>

struct libvlc_media_list_t
{
libvlc_event_manager_t event_manager;
libvlc_instance_t * p_libvlc_instance;
int i_refcount;
vlc_mutex_t object_lock;
vlc_mutex_t refcount_lock;
libvlc_media_t * p_md; 

libvlc_media_t * p_internal_md; 
vlc_array_t items;



bool b_read_only;
};


void libvlc_media_list_internal_add_media(
libvlc_media_list_t * p_mlist,
libvlc_media_t * p_md );

void libvlc_media_list_internal_insert_media(
libvlc_media_list_t * p_mlist,
libvlc_media_t * p_md, int index );

int libvlc_media_list_internal_remove_index(
libvlc_media_list_t * p_mlist, int index );

void libvlc_media_list_internal_end_reached(
libvlc_media_list_t * p_mlist );

#endif
