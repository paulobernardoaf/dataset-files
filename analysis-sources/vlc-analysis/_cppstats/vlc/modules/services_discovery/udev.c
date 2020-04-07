#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif
#include <errno.h>
#include <search.h>
#include <poll.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libudev.h>
#include <vlc_common.h>
#include <vlc_services_discovery.h>
#include <vlc_plugin.h>
#if defined(HAVE_ALSA)
#include <vlc_modules.h>
#endif
#include <vlc_fs.h>
#include <vlc_url.h>
static int OpenV4L (vlc_object_t *);
#if defined(HAVE_ALSA)
static int OpenALSA (vlc_object_t *);
#endif
static int OpenDisc (vlc_object_t *);
static void Close (vlc_object_t *);
static int vlc_sd_probe_Open (vlc_object_t *);
vlc_module_begin ()
set_shortname (N_("Video capture"))
set_description (N_("Video capture (Video4Linux)"))
set_category (CAT_PLAYLIST)
set_subcategory (SUBCAT_PLAYLIST_SD)
set_capability ("services_discovery", 0)
set_callbacks (OpenV4L, Close)
add_shortcut ("v4l", "video")
#if defined(HAVE_ALSA)
add_submodule ()
set_shortname (N_("Audio capture"))
set_description (N_("Audio capture (ALSA)"))
set_category (CAT_PLAYLIST)
set_subcategory (SUBCAT_PLAYLIST_SD)
set_capability ("services_discovery", 0)
set_callbacks (OpenALSA, Close)
add_shortcut ("alsa", "audio")
#endif
add_submodule ()
set_shortname (N_("Discs"))
set_description (N_("Discs"))
set_category (CAT_PLAYLIST)
set_subcategory (SUBCAT_PLAYLIST_SD)
set_capability ("services_discovery", 0)
set_callbacks (OpenDisc, Close)
add_shortcut ("disc")
VLC_SD_PROBE_SUBMODULE
vlc_module_end ()
static int vlc_sd_probe_Open (vlc_object_t *obj)
{
vlc_probe_t *probe = (vlc_probe_t *)obj;
struct udev *udev = udev_new ();
if (udev == NULL)
return VLC_PROBE_CONTINUE;
struct udev_monitor *mon = udev_monitor_new_from_netlink (udev, "udev");
if (mon != NULL)
{
vlc_sd_probe_Add (probe, "v4l", N_("Video capture"), SD_CAT_DEVICES);
#if defined(HAVE_ALSA)
if (!module_exists ("pulselist"))
vlc_sd_probe_Add (probe, "alsa", N_("Audio capture"),
SD_CAT_DEVICES);
#endif
vlc_sd_probe_Add (probe, "disc", N_("Discs"), SD_CAT_DEVICES);
udev_monitor_unref (mon);
}
udev_unref (udev);
return VLC_PROBE_CONTINUE;
}
struct device
{
dev_t devnum; 
input_item_t *item;
services_discovery_t *sd;
};
struct subsys
{
const char *name;
const char *description;
char * (*get_mrl) (struct udev_device *dev);
char * (*get_name) (struct udev_device *dev);
int item_type;
};
typedef struct
{
const struct subsys *subsys;
struct udev_monitor *monitor;
vlc_thread_t thread;
void *root;
} services_discovery_sys_t;
static int cmpdev (const void *a, const void *b)
{
const dev_t *da = a, *db = b;
if (*da > *db)
return +1;
if (*da < *db)
return -1;
return 0;
}
static void DestroyDevice (void *data)
{
struct device *d = data;
if (d->sd)
services_discovery_RemoveItem (d->sd, d->item);
input_item_Release (d->item);
free (d);
}
static char *decode_property (struct udev_device *, const char *);
static int AddDevice (services_discovery_t *sd, struct udev_device *dev)
{
services_discovery_sys_t *p_sys = sd->p_sys;
char *mrl = p_sys->subsys->get_mrl (dev);
if (mrl == NULL)
return 0; 
char *name = p_sys->subsys->get_name (dev);
input_item_t *item = input_item_NewExt (mrl, name ? name : mrl,
INPUT_DURATION_INDEFINITE,
p_sys->subsys->item_type, ITEM_LOCAL);
msg_Dbg (sd, "adding %s (%s)", mrl, name);
free (name);
free (mrl);
if (item == NULL)
return -1;
struct device *d = malloc (sizeof (*d));
if (d == NULL)
{
input_item_Release (item);
return -1;
}
d->devnum = udev_device_get_devnum (dev);
d->item = item;
d->sd = NULL;
void **dp = tsearch (d, &p_sys->root, cmpdev);
if (dp == NULL) 
{
DestroyDevice (d);
return -1;
}
if (*dp != d) 
{
DestroyDevice (*dp);
*dp = d;
}
services_discovery_AddItem(sd, item);
d->sd = sd;
return 0;
}
static void RemoveDevice (services_discovery_t *sd, struct udev_device *dev)
{
services_discovery_sys_t *p_sys = sd->p_sys;
dev_t num = udev_device_get_devnum (dev);
void **dp = tfind (&(dev_t){ num }, &p_sys->root, cmpdev);
if (dp == NULL)
return;
struct device *d = *dp;
tdelete (d, &p_sys->root, cmpdev);
DestroyDevice (d);
}
static void *Run (void *);
static int Open (vlc_object_t *obj, const struct subsys *subsys)
{
services_discovery_t *sd = (services_discovery_t *)obj;
services_discovery_sys_t *p_sys = malloc (sizeof (*p_sys));
if (p_sys == NULL)
return VLC_ENOMEM;
sd->description = vlc_gettext(subsys->description);
sd->p_sys = p_sys;
p_sys->subsys = subsys;
p_sys->root = NULL;
struct udev_monitor *mon = NULL;
struct udev *udev = udev_new ();
if (udev == NULL)
goto error;
mon = udev_monitor_new_from_netlink (udev, "udev");
if (mon == NULL
|| udev_monitor_filter_add_match_subsystem_devtype (mon, subsys->name,
NULL))
goto error;
p_sys->monitor = mon;
struct udev_enumerate *devenum = udev_enumerate_new (udev);
if (devenum == NULL)
goto error;
if (udev_enumerate_add_match_subsystem (devenum, subsys->name))
{
udev_enumerate_unref (devenum);
goto error;
}
udev_monitor_enable_receiving (mon);
udev_enumerate_scan_devices (devenum);
struct udev_list_entry *devlist = udev_enumerate_get_list_entry (devenum);
struct udev_list_entry *deventry;
udev_list_entry_foreach (deventry, devlist)
{
const char *path = udev_list_entry_get_name (deventry);
struct udev_device *dev = udev_device_new_from_syspath (udev, path);
AddDevice (sd, dev);
udev_device_unref (dev);
}
udev_enumerate_unref (devenum);
if (vlc_clone (&p_sys->thread, Run, sd, VLC_THREAD_PRIORITY_LOW))
{ 
udev_monitor_unref (mon);
udev_unref (udev);
p_sys->monitor = NULL;
}
return VLC_SUCCESS;
error:
if (mon != NULL)
udev_monitor_unref (mon);
if (udev != NULL)
udev_unref (udev);
free (p_sys);
return VLC_EGENERIC;
}
static void Close (vlc_object_t *obj)
{
services_discovery_t *sd = (services_discovery_t *)obj;
services_discovery_sys_t *p_sys = sd->p_sys;
if (p_sys->monitor != NULL)
{
struct udev *udev = udev_monitor_get_udev (p_sys->monitor);
vlc_cancel (p_sys->thread);
vlc_join (p_sys->thread, NULL);
udev_monitor_unref (p_sys->monitor);
udev_unref (udev);
}
tdestroy (p_sys->root, DestroyDevice);
free (p_sys);
}
static void *Run (void *data)
{
services_discovery_t *sd = data;
services_discovery_sys_t *p_sys = sd->p_sys;
struct udev_monitor *mon = p_sys->monitor;
int fd = udev_monitor_get_fd (mon);
struct pollfd ufd = { .fd = fd, .events = POLLIN, };
for (;;)
{
while (poll (&ufd, 1, -1) == -1)
if (errno != EINTR)
break;
int canc = vlc_savecancel ();
struct udev_device *dev = udev_monitor_receive_device (mon);
if (dev == NULL)
continue;
const char *action = udev_device_get_action (dev);
if (!strcmp (action, "add"))
AddDevice (sd, dev);
else if (!strcmp (action, "remove"))
RemoveDevice (sd, dev);
else if (!strcmp (action, "change"))
{
RemoveDevice (sd, dev);
AddDevice (sd, dev);
}
udev_device_unref (dev);
vlc_restorecancel (canc);
}
return NULL;
}
static int hex (char c)
{
if (c >= '0' && c <= '9')
return c - '0';
if (c >= 'A' && c <= 'F')
return c + 10 - 'A';
if (c >= 'a' && c <= 'f')
return c + 10 - 'a';
return -1;
}
static char *decode (const char *enc)
{
char *ret = enc ? strdup (enc) : NULL;
if (ret == NULL)
return NULL;
char *out = ret;
for (const char *in = ret; *in; out++)
{
int h1, h2;
if ((in[0] == '\\') && (in[1] == 'x')
&& ((h1 = hex (in[2])) != -1)
&& ((h2 = hex (in[3])) != -1))
{
*out = (h1 << 4) | h2;
in += 4;
}
else
{
*out = *in;
in++;
}
}
*out = 0;
return ret;
}
static char *decode_property (struct udev_device *dev, const char *name)
{
return decode (udev_device_get_property_value (dev, name));
}
static bool v4l_is_legacy (struct udev_device *dev)
{
const char *version;
version = udev_device_get_property_value (dev, "ID_V4L_VERSION");
return (version != NULL) && !strcmp (version, "1");
}
static bool v4l_can_capture (struct udev_device *dev)
{
const char *caps;
caps = udev_device_get_property_value (dev, "ID_V4L_CAPABILITIES");
return (caps != NULL) && (strstr (caps, ":capture:") != NULL);
}
static char *v4l_get_mrl (struct udev_device *dev)
{
if (v4l_is_legacy (dev) || !v4l_can_capture (dev))
return NULL;
const char *node = udev_device_get_devnode (dev);
char *mrl;
if (asprintf (&mrl, "v4l2://%s", node) == -1)
mrl = NULL;
return mrl;
}
static char *v4l_get_name (struct udev_device *dev)
{
const char *prd = udev_device_get_property_value (dev, "ID_V4L_PRODUCT");
return prd ? strdup (prd) : NULL;
}
int OpenV4L (vlc_object_t *obj)
{
static const struct subsys subsys = {
"video4linux", N_("Video capture"),
v4l_get_mrl, v4l_get_name, ITEM_TYPE_CARD,
};
return Open (obj, &subsys);
}
#if defined(HAVE_ALSA)
#include <alsa/asoundlib.h>
static int alsa_get_device (struct udev_device *dev, unsigned *restrict pcard,
unsigned *restrict pdevice)
{
const char *node = udev_device_get_devpath (dev);
char type;
node = strrchr (node, '/');
if (node == NULL)
return -1;
if (sscanf (node, "/pcmC%uD%u%c", pcard, pdevice, &type) < 3)
return -1;
if (type != 'c')
return -1;
return 0;
}
static char *alsa_get_mrl (struct udev_device *dev)
{
char *mrl;
unsigned card, device;
if (alsa_get_device (dev, &card, &device))
return NULL;
if (asprintf (&mrl, "alsa://plughw:%u,%u", card, device) == -1)
mrl = NULL;
return mrl;
}
static char *alsa_get_name (struct udev_device *dev)
{
char *name = NULL;
unsigned card, device;
if (alsa_get_device (dev, &card, &device))
return NULL;
char card_name[4 + 3 * sizeof (int)];
snprintf (card_name, sizeof (card_name), "hw:%u", card);
snd_ctl_t *ctl;
if (snd_ctl_open (&ctl, card_name, 0))
return NULL;
snd_pcm_info_t *pcm_info;
snd_pcm_info_alloca (&pcm_info);
snd_pcm_info_set_device (pcm_info, device);
snd_pcm_info_set_subdevice (pcm_info, 0);
snd_pcm_info_set_stream (pcm_info, SND_PCM_STREAM_CAPTURE);
if (snd_ctl_pcm_info (ctl, pcm_info))
goto out;
name = strdup (snd_pcm_info_get_name (pcm_info));
out:
snd_ctl_close (ctl);
return name;
}
int OpenALSA (vlc_object_t *obj)
{
static const struct subsys subsys = {
"sound", N_("Audio capture"),
alsa_get_mrl, alsa_get_name, ITEM_TYPE_CARD,
};
return Open (obj, &subsys);
}
#endif 
static char *disc_get_mrl (struct udev_device *dev)
{
const char *node = udev_device_get_devnode (dev);
const char *val;
val = udev_device_get_property_value (dev, "ID_CDROM");
if (val == NULL)
return NULL; 
val = udev_device_get_property_value (dev, "ID_CDROM_MEDIA_STATE");
if (val == NULL)
{ 
int fd = vlc_open (node, O_RDONLY);
if (fd != -1)
vlc_close (fd);
return NULL;
}
if (!strcmp (val, "blank"))
return NULL; 
const char *scheme = NULL;
val = udev_device_get_property_value (dev,
"ID_CDROM_MEDIA_TRACK_COUNT_AUDIO");
if (val && atoi (val))
scheme = "cdda"; 
val = udev_device_get_property_value (dev, "ID_CDROM_MEDIA_DVD");
if (val && atoi (val))
scheme = "dvd";
val = udev_device_get_property_value (dev, "ID_CDROM_MEDIA_BD");
if (val && atoi (val))
scheme = "bluray";
#if defined(LOL)
val = udev_device_get_property_value (dev, "ID_CDROM_MEDIA_HDDVD");
if (val && atoi (val))
scheme = "hddvd";
#endif
if (scheme == NULL)
return NULL;
return vlc_path2uri (node, scheme);
}
static char *disc_get_name (struct udev_device *dev)
{
char *name = NULL;
struct udev_list_entry *list, *entry;
list = udev_device_get_properties_list_entry (dev);
if (unlikely(list == NULL))
return NULL;
const char *cat = NULL;
udev_list_entry_foreach (entry, list)
{
const char *propname = udev_list_entry_get_name(entry);
if (strncmp(propname, "ID_CDROM_MEDIA_", 15))
continue;
if (!atoi (udev_list_entry_get_value (entry)))
continue;
propname += 15;
if (!strncmp(propname, "CD", 2))
cat = N_("CD");
else if (!strncmp(propname, "DVD", 3))
cat = N_("DVD");
else if (!strncmp(propname, "BD", 2))
cat = N_("Blu-ray");
else if (!strncmp(propname, "HDDVD", 5))
cat = N_("HD DVD");
if (cat != NULL)
break;
}
if (cat == NULL)
cat = N_("Unknown type");
char *label = decode_property (dev, "ID_FS_LABEL_ENC");
if (label)
if (asprintf(&name, "%s (%s)", label, vlc_gettext(cat)) < 0)
name = NULL;
free(label);
return name;
}
int OpenDisc (vlc_object_t *obj)
{
static const struct subsys subsys = {
"block", N_("Discs"), disc_get_mrl, disc_get_name, ITEM_TYPE_DISC,
};
return Open (obj, &subsys);
}
