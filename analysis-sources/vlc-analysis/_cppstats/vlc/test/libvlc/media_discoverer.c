#include "test.h"
#include <string.h>
static void
ml_item_event(const struct libvlc_event_t *p_ev, const char *psz_event)
{
char *psz_mrl = libvlc_media_get_mrl(p_ev->u.media_list_item_added.item);
assert(psz_mrl);
test_log("item %s(%d): '%s'\n", psz_event,
p_ev->u.media_list_item_added.index, psz_mrl);
free(psz_mrl);
}
static void
ml_item_added(const struct libvlc_event_t *p_ev, void *p_data)
{
(void) p_data;
ml_item_event(p_ev, "added");
}
static void
ml_item_deleted(const struct libvlc_event_t *p_ev, void *p_data)
{
(void) p_data;
ml_item_event(p_ev, "deleted");
}
static void
test_discoverer(libvlc_instance_t *p_vlc, const char *psz_name, bool b_wait)
{
test_log("creating and starting discoverer %s\n", psz_name);
libvlc_media_discoverer_t *p_md =
libvlc_media_discoverer_new(p_vlc, psz_name);
assert(p_md != NULL);
libvlc_media_list_t *p_ml = libvlc_media_discoverer_media_list(p_md);
assert(p_ml != NULL);
libvlc_event_manager_t *p_evm = libvlc_media_list_event_manager(p_ml);
assert(p_evm);
int i_ret;
i_ret = libvlc_event_attach(p_evm, libvlc_MediaListItemAdded,
ml_item_added, NULL);
assert(i_ret == 0);
i_ret = libvlc_event_attach(p_evm, libvlc_MediaListItemDeleted,
ml_item_deleted, NULL);
assert(i_ret == 0);
if (libvlc_media_discoverer_start(p_md) == -1)
{
test_log("warn: could not start md (not critical)\n");
}
else
{
assert(libvlc_media_discoverer_is_running(p_md));
if (b_wait)
{
test_log("Press any keys to stop\n");
getchar();
}
libvlc_media_discoverer_stop(p_md);
}
libvlc_event_detach(p_evm, libvlc_MediaListItemAdded,
ml_item_added, NULL);
libvlc_event_detach(p_evm, libvlc_MediaListItemDeleted,
ml_item_deleted, NULL);
libvlc_media_list_release(p_ml);
libvlc_media_discoverer_release(p_md);
}
int
main(int i_argc, char *ppsz_argv[])
{
test_init();
char *psz_test_name = i_argc > 1 ? ppsz_argv[1] : NULL;
libvlc_instance_t *p_vlc = libvlc_new(test_defaults_nargs,
test_defaults_args);
assert(p_vlc != NULL);
if (psz_test_name != NULL)
{
alarm(0);
test_discoverer(p_vlc, psz_test_name, true);
libvlc_release(p_vlc);
return 0;
}
for(libvlc_media_discoverer_category_t i_cat = libvlc_media_discoverer_devices;
i_cat <= libvlc_media_discoverer_localdirs; i_cat ++)
{
test_log("== getting list of media_discoverer for %d category ==\n", i_cat);
libvlc_media_discoverer_description_t **pp_services;
ssize_t i_count =
libvlc_media_discoverer_list_get(p_vlc, i_cat, &pp_services);
if (i_count <= 0)
{
test_log("warn: no discoverers (not critical)\n");
continue;
}
assert(pp_services != NULL);
for (unsigned int i = 0; i < i_count; ++i)
{
libvlc_media_discoverer_description_t *p_service = pp_services[i];
assert(i_cat == p_service->i_cat);
test_log("= discoverer: name: '%s', longname: '%s' =\n",
p_service->psz_name, p_service->psz_longname);
#if 0
if (!strncasecmp(p_service->psz_name, "podcast", 7)
|| i_cat == libvlc_media_discoverer_lan)
{
continue;
}
test_discoverer(p_vlc, p_service->psz_name, false);
#endif
}
libvlc_media_discoverer_list_release(pp_services, i_count);
}
libvlc_release(p_vlc);
return 0;
}
