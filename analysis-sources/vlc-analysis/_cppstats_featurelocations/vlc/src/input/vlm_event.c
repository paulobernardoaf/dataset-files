
























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_vlm.h>
#include "vlm_internal.h"
#include "vlm_event.h"
#include <assert.h>


static void Trigger( vlm_t *, int i_type, int64_t id, const char *psz_name );
static void TriggerInstanceState( vlm_t *, int i_type, int64_t id, const char *psz_name, const char *psz_instance_name, vlm_state_e input_state );




void vlm_SendEventMediaAdded( vlm_t *p_vlm, int64_t id, const char *psz_name )
{
Trigger( p_vlm, VLM_EVENT_MEDIA_ADDED, id, psz_name );
}
void vlm_SendEventMediaRemoved( vlm_t *p_vlm, int64_t id, const char *psz_name )
{
Trigger( p_vlm, VLM_EVENT_MEDIA_REMOVED, id, psz_name );
}
void vlm_SendEventMediaChanged( vlm_t *p_vlm, int64_t id, const char *psz_name )
{
Trigger( p_vlm, VLM_EVENT_MEDIA_CHANGED, id, psz_name );
}

void vlm_SendEventMediaInstanceStarted( vlm_t *p_vlm, int64_t id, const char *psz_name )
{
Trigger( p_vlm, VLM_EVENT_MEDIA_INSTANCE_STARTED, id, psz_name );
}
void vlm_SendEventMediaInstanceStopped( vlm_t *p_vlm, int64_t id, const char *psz_name )
{
Trigger( p_vlm, VLM_EVENT_MEDIA_INSTANCE_STOPPED, id, psz_name );
}

void vlm_SendEventMediaInstanceState( vlm_t *p_vlm, int64_t id, const char *psz_name, const char *psz_instance_name, vlm_state_e state )
{
TriggerInstanceState( p_vlm, VLM_EVENT_MEDIA_INSTANCE_STATE, id, psz_name, psz_instance_name, state );
}




static void Trigger( vlm_t *p_vlm, int i_type, int64_t id, const char *psz_name )
{
vlm_event_t event;

event.i_type = i_type;
event.id = id;
event.psz_name = psz_name;
event.input_state = 0;
event.psz_instance_name = NULL;
var_SetAddress( p_vlm, "intf-event", &event );
}

static void TriggerInstanceState( vlm_t *p_vlm, int i_type, int64_t id, const char *psz_name, const char *psz_instance_name, vlm_state_e input_state )
{
vlm_event_t event;

event.i_type = i_type;
event.id = id;
event.psz_name = psz_name;
event.input_state = input_state;
event.psz_instance_name = psz_instance_name;
var_SetAddress( p_vlm, "intf-event", &event );
}
