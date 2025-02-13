#include <vlc_common.h>
#include <vlc_input_item.h>
#include <vlc_services_discovery.h>
#if defined(__cplusplus)
extern "C" {
#endif
typedef struct vlc_media_tree {
input_item_node_t root;
} vlc_media_tree_t;
struct vlc_media_tree_callbacks
{
void
(*on_children_reset)(vlc_media_tree_t *tree, input_item_node_t *node,
void *userdata);
void
(*on_children_added)(vlc_media_tree_t *tree, input_item_node_t *node,
input_item_node_t *const children[], size_t count,
void *userdata);
void
(*on_children_removed)(vlc_media_tree_t *tree, input_item_node_t *node,
input_item_node_t *const children[], size_t count,
void *userdata);
void
(*on_preparse_end)(vlc_media_tree_t *tree, input_item_node_t * node,
enum input_item_preparse_status status,
void *userdata);
};
typedef struct vlc_media_tree_listener_id vlc_media_tree_listener_id;
VLC_API vlc_media_tree_listener_id *
vlc_media_tree_AddListener(vlc_media_tree_t *tree,
const struct vlc_media_tree_callbacks *cbs,
void *userdata, bool notify_current_state);
VLC_API void
vlc_media_tree_RemoveListener(vlc_media_tree_t *tree,
vlc_media_tree_listener_id *listener);
VLC_API void
vlc_media_tree_Lock(vlc_media_tree_t *);
VLC_API void
vlc_media_tree_Unlock(vlc_media_tree_t *);
VLC_API bool
vlc_media_tree_Find(vlc_media_tree_t *tree, const input_item_t *media,
input_item_node_t **result,
input_item_node_t **result_parent);
VLC_API void
vlc_media_tree_Preparse(vlc_media_tree_t *tree, libvlc_int_t *libvlc,
input_item_t *media, void *id);
VLC_API void
vlc_media_tree_PreparseCancel(libvlc_int_t *libvlc, void* id);
typedef struct vlc_media_source_t
{
vlc_media_tree_t *tree;
const char *description;
} vlc_media_source_t;
VLC_API void
vlc_media_source_Hold(vlc_media_source_t *);
VLC_API void
vlc_media_source_Release(vlc_media_source_t *);
typedef struct vlc_media_source_provider_t vlc_media_source_provider_t;
VLC_API vlc_media_source_provider_t *
vlc_media_source_provider_Get(libvlc_int_t *);
VLC_API vlc_media_source_t *
vlc_media_source_provider_GetMediaSource(vlc_media_source_provider_t *,
const char *name);
struct vlc_media_source_meta
{
char *name;
char *longname;
enum services_discovery_category_e category;
};
typedef struct vlc_media_source_meta_list vlc_media_source_meta_list_t;
VLC_API vlc_media_source_meta_list_t *
vlc_media_source_provider_List(vlc_media_source_provider_t *,
enum services_discovery_category_e category);
VLC_API size_t
vlc_media_source_meta_list_Count(vlc_media_source_meta_list_t *);
VLC_API struct vlc_media_source_meta *
vlc_media_source_meta_list_Get(vlc_media_source_meta_list_t *, size_t index);
VLC_API void
vlc_media_source_meta_list_Delete(vlc_media_source_meta_list_t *);
#if defined(__cplusplus)
}
#endif
