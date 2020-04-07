#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif
#include <vlc_common.h>
#include <vlc_services_discovery.h>
#include <vlc_plugin.h>
#define IOCTL_CDROMDISK2 0x82
#define CDROMDISK2_DRIVELETTERS 0x60
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
HFILE hcd2;
ULONG ulAction;
ULONG ulParamLen;
ULONG ulData;
ULONG ulDataLen;
ULONG rc;
sd->description = _("Discs");
if (DosOpen ((PSZ)"CD-ROM2$", (PHFILE)&hcd2, &ulAction, 0, FILE_NORMAL,
OPEN_ACTION_OPEN_IF_EXISTS | OPEN_ACTION_FAIL_IF_NEW,
OPEN_ACCESS_READONLY | OPEN_SHARE_DENYNONE, NULL))
return VLC_EGENERIC;
rc = DosDevIOCtl (hcd2, IOCTL_CDROMDISK2, CDROMDISK2_DRIVELETTERS,
NULL, 0, &ulParamLen, &ulData, sizeof(ulData), &ulDataLen);
if (!rc)
{
char mrl[] = "file:///A:/", name[] = "A:";
int count = LOUSHORT(ulData);
int drive = HIUSHORT(ulData);
input_item_t *item;
char letter;
for (; count; --count, ++drive)
{
letter = 'A' + drive;
mrl[8] = name[0] = letter;
item = input_item_NewDisc (mrl, name, INPUT_DURATION_INDEFINITE);
msg_Dbg (sd, "adding %s (%s)", mrl, name);
if (item == NULL)
break;
services_discovery_AddItem (sd, item);
}
}
DosClose (hcd2);
return rc ? VLC_EGENERIC : VLC_SUCCESS;
}
