#include <vlc_input.h>
#include <vlc_probe.h>
#if defined(__cplusplus)
extern "C" {
#endif
struct services_discovery_callbacks
{
void (*item_added)(struct services_discovery_t *sd, input_item_t *parent,
input_item_t *item, const char *category);
void (*item_removed)(struct services_discovery_t *sd, input_item_t *item);
};
struct services_discovery_owner_t
{
const struct services_discovery_callbacks *cbs;
void *sys; 
};
struct services_discovery_t
{
struct vlc_object_t obj;
module_t * p_module; 
char *psz_name; 
config_chain_t *p_cfg; 
const char *description; 
int ( *pf_control ) ( services_discovery_t *, int, va_list );
void *p_sys; 
struct services_discovery_owner_t owner; 
};
enum services_discovery_category_e
{
SD_CAT_DEVICES = 1, 
SD_CAT_LAN, 
SD_CAT_INTERNET, 
SD_CAT_MYCOMPUTER 
};
enum services_discovery_command_e
{
SD_CMD_SEARCH = 1, 
SD_CMD_DESCRIPTOR 
};
enum services_discovery_capability_e
{
SD_CAP_SEARCH = 1 
};
typedef struct
{
char *psz_short_desc; 
char *psz_icon_url; 
char *psz_url; 
int i_capabilities; 
} services_discovery_descriptor_t;
static inline int vlc_sd_control( services_discovery_t *p_sd, int i_control, va_list args )
{
if( p_sd->pf_control )
return p_sd->pf_control( p_sd, i_control, args );
else
return VLC_EGENERIC;
}
VLC_API char ** vlc_sd_GetNames( vlc_object_t *, char ***, int ** ) VLC_USED;
#define vlc_sd_GetNames(obj, pln, pcat ) vlc_sd_GetNames(VLC_OBJECT(obj), pln, pcat)
VLC_API services_discovery_t *vlc_sd_Create(vlc_object_t *parent,
const char *chain, const struct services_discovery_owner_t *owner)
VLC_USED;
#define vlc_sd_Create( obj, a, b ) vlc_sd_Create( VLC_OBJECT( obj ), a, b )
VLC_API void vlc_sd_Destroy( services_discovery_t * );
static inline void services_discovery_AddItem(services_discovery_t *sd,
input_item_t *item)
{
sd->owner.cbs->item_added(sd, NULL, item, NULL);
}
static inline void services_discovery_AddSubItem(services_discovery_t *sd,
input_item_t *parent,
input_item_t *item)
{
sd->owner.cbs->item_added(sd, parent, item, NULL);
}
VLC_DEPRECATED
static inline void services_discovery_AddItemCat(services_discovery_t *sd,
input_item_t *item,
const char *category)
{
sd->owner.cbs->item_added(sd, NULL, item, category);
}
static inline void services_discovery_RemoveItem(services_discovery_t *sd,
input_item_t *item)
{
sd->owner.cbs->item_removed(sd, item);
}
VLC_API int vlc_sd_probe_Add(vlc_probe_t *, const char *, const char *, int category);
#define VLC_SD_PROBE_SUBMODULE add_submodule() set_capability( "services probe", 100 ) set_callback( vlc_sd_probe_Open )
#define VLC_SD_PROBE_HELPER(name, longname, cat) static int vlc_sd_probe_Open (vlc_object_t *obj) { return vlc_sd_probe_Add ((struct vlc_probe_t *)obj, name, longname, cat); }
#if defined(__cplusplus)
}
#endif
