#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif
#include <vlc_common.h>
#include <vlc_services_discovery.h>
#include <vlc_plugin.h>
static int Open (vlc_object_t *);
VLC_SD_PROBE_HELPER("disc", N_("Discs"), SD_CAT_DEVICES)
vlc_module_begin ()
add_submodule ()
set_shortname (N_("Discs"))
set_description (N_("Discs"))
set_category (CAT_PLAYLIST)
set_subcategory (SUBCAT_PLAYLIST_SD)
set_capability ("services_discovery", 0)
set_callback(Open)
add_shortcut ("disc")
VLC_SD_PROBE_SUBMODULE
vlc_module_end ()
static int Open (vlc_object_t *obj)
{
services_discovery_t *sd = (services_discovery_t *)obj;
sd->description = _("Discs");
LONG drives = GetLogicalDrives ();
char mrl[12] = "file:///A:/", name[3] = "A:";
WCHAR path[4] = TEXT("A:\\");
for (char d = 0; d < 26; d++)
{
input_item_t *item;
WCHAR letter = 'A' + d;
if (!(drives & (1 << d)))
continue;
path[0] = letter;
if (GetDriveType (path) != DRIVE_CDROM)
continue;
mrl[8] = name[0] = letter;
item = input_item_NewDisc (mrl, name, INPUT_DURATION_INDEFINITE);
msg_Dbg (sd, "adding %s (%s)", mrl, name);
if (item == NULL)
break;
services_discovery_AddItem(sd, item);
}
return VLC_SUCCESS;
}
