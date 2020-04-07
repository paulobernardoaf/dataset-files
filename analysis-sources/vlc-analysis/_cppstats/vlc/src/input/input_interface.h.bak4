





















#if !defined(LIBVLC_INPUT_INTERFACE_H)
#define LIBVLC_INPUT_INTERFACE_H 1

#include <vlc_input.h>
#include <libvlc.h>




void input_item_SetPreparsed( input_item_t *p_i, bool b_preparsed );
void input_item_SetArtNotFound( input_item_t *p_i, bool b_not_found );
void input_item_SetArtFetched( input_item_t *p_i, bool b_art_fetched );
void input_item_SetEpg( input_item_t *p_item, const vlc_epg_t *p_epg, bool );
void input_item_ChangeEPGSource( input_item_t *p_item, int i_source_id );
void input_item_SetEpgEvent( input_item_t *p_item, const vlc_epg_event_t *p_epg_evt );
void input_item_SetEpgTime( input_item_t *, int64_t );
void input_item_SetEpgOffline( input_item_t * );




void input_resource_TerminateSout( input_resource_t *p_resource );

#endif
