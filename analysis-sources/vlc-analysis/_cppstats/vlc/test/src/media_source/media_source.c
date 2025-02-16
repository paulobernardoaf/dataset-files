#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include "../../libvlc/test.h"
#include "../lib/libvlc_internal.h"
#include <assert.h>
#include <vlc_common.h>
#include <vlc_media_source.h>
#include <vlc_vector.h>
#include <vlc/vlc.h>
static const char *libvlc_argv[] = {
"-v",
"--ignore-config",
"-Idummy",
"--no-media-library",
};
static void
test_list(void)
{
libvlc_instance_t *vlc = libvlc_new(ARRAY_SIZE(libvlc_argv), libvlc_argv);
assert(vlc);
vlc_media_source_provider_t *provider =
vlc_media_source_provider_Get(vlc->p_libvlc_int);
assert(provider);
vlc_media_source_meta_list_t *list =
vlc_media_source_provider_List(provider, 0);
assert(list);
size_t count = vlc_media_source_meta_list_Count(list);
assert(count);
for (size_t i = 0; i < count; ++i)
{
struct vlc_media_source_meta *meta =
vlc_media_source_meta_list_Get(list, i);
assert(meta);
assert(meta->name);
assert(meta->longname);
assert(meta->category); 
}
vlc_media_source_meta_list_Delete(list);
libvlc_release(vlc);
}
static void
test_list_filtered_by_category(void)
{
libvlc_instance_t *vlc = libvlc_new(ARRAY_SIZE(libvlc_argv), libvlc_argv);
assert(vlc);
vlc_media_source_provider_t *provider =
vlc_media_source_provider_Get(vlc->p_libvlc_int);
assert(provider);
vlc_media_source_meta_list_t *list =
vlc_media_source_provider_List(provider, SD_CAT_LAN);
assert(list);
size_t count = vlc_media_source_meta_list_Count(list);
assert(count);
for (size_t i = 0; i < count; ++i)
{
struct vlc_media_source_meta *meta =
vlc_media_source_meta_list_Get(list, i);
assert(meta);
assert(meta->name);
assert(meta->longname);
assert(meta->category == SD_CAT_LAN);
}
vlc_media_source_meta_list_Delete(list);
libvlc_release(vlc);
}
int main(void)
{
test_init();
test_list();
test_list_filtered_by_category();
return 0;
}
