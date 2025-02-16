#include <vlc_common.h>
void vlm_SendEventMediaAdded( vlm_t *, int64_t id, const char *psz_name );
void vlm_SendEventMediaRemoved( vlm_t *, int64_t id, const char *psz_name );
void vlm_SendEventMediaChanged( vlm_t *, int64_t id, const char *psz_name );
void vlm_SendEventMediaInstanceStarted( vlm_t *, int64_t id, const char *psz_name );
void vlm_SendEventMediaInstanceStopped( vlm_t *, int64_t id, const char *psz_name );
void vlm_SendEventMediaInstanceState( vlm_t *, int64_t id, const char *psz_name, const char *psz_instance_name, vlm_state_e state );
