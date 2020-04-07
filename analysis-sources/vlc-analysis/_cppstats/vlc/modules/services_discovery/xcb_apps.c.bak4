





















#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif
#include <stdarg.h>
#include <xcb/xcb.h>
typedef xcb_atom_t Atom;
#include <X11/Xatom.h> 
#include <vlc_common.h>
#include <vlc_services_discovery.h>
#include <vlc_dialog.h>
#include <vlc_charset.h>
#include <vlc_plugin.h>
#if defined(HAVE_SEARCH_H)
#include <search.h>
#endif
#if defined(HAVE_POLL)
#include <poll.h>
#endif
#if defined(_WIN32)
#include <winsock2.h>
#endif

static int Open (vlc_object_t *);
static void Close (vlc_object_t *);
static int vlc_sd_probe_Open (vlc_object_t *);




vlc_module_begin ()
set_shortname (N_("Screen capture"))
set_description (N_("Screen capture"))
set_category (CAT_PLAYLIST)
set_subcategory (SUBCAT_PLAYLIST_SD)
set_capability ("services_discovery", 0)
set_callbacks (Open, Close)

add_shortcut ("apps", "screen")

VLC_SD_PROBE_SUBMODULE
vlc_module_end ()

typedef struct
{
xcb_connection_t *conn;
vlc_thread_t thread;
xcb_atom_t net_client_list;
xcb_atom_t net_wm_name;
xcb_window_t root_window;
void *apps;
input_item_t *apps_root;
} services_discovery_sys_t;

static void *Run (void *);
static void UpdateApps (services_discovery_t *);
static void DelApp (void *);
static void AddDesktop(services_discovery_t *);

static int vlc_sd_probe_Open (vlc_object_t *obj)
{
vlc_probe_t *probe = (vlc_probe_t *)obj;

char *display = var_InheritString (obj, "x11-display");
xcb_connection_t *conn = xcb_connect (display, NULL);
free (display);
if (xcb_connection_has_error (conn))
return VLC_PROBE_CONTINUE;
xcb_disconnect (conn);
return vlc_sd_probe_Add (probe, "xcb_apps",
N_("Screen capture"), SD_CAT_DEVICES);
}




static int Open (vlc_object_t *obj)
{
services_discovery_t *sd = (services_discovery_t *)obj;
services_discovery_sys_t *p_sys = malloc (sizeof (*p_sys));

if (p_sys == NULL)
return VLC_ENOMEM;
sd->p_sys = p_sys;
sd->description = _("Screen capture");


char *display = var_InheritString (obj, "x11-display");
int snum;
xcb_connection_t *conn = xcb_connect (display, &snum);
free (display);
if (xcb_connection_has_error (conn))
{
free (p_sys);
return VLC_EGENERIC;
}
p_sys->conn = conn;


const xcb_setup_t *setup = xcb_get_setup (conn);
const xcb_screen_t *scr = NULL;
for (xcb_screen_iterator_t i = xcb_setup_roots_iterator (setup);
i.rem > 0; xcb_screen_next (&i))
{
if (snum == 0)
{
scr = i.data;
break;
}
snum--;
}
if (scr == NULL)
{
msg_Err (obj, "bad X11 screen number");
goto error;
}


AddDesktop (sd);

p_sys->root_window = scr->root;
xcb_change_window_attributes (conn, scr->root, XCB_CW_EVENT_MASK,
&(uint32_t) { XCB_EVENT_MASK_PROPERTY_CHANGE });



xcb_intern_atom_reply_t *r;
xcb_intern_atom_cookie_t ncl, nwn;

ncl = xcb_intern_atom (conn, 1, strlen ("_NET_CLIENT_LIST"),
"_NET_CLIENT_LIST");
nwn = xcb_intern_atom (conn, 0, strlen ("_NET_WM_NAME"), "_NET_WM_NAME");

r = xcb_intern_atom_reply (conn, ncl, NULL);
if (r == NULL || r->atom == 0)
{
vlc_dialog_display_error (sd, _("Screen capture"),
_("Your window manager does not provide a list of applications."));
msg_Err (sd, "client list not supported (_NET_CLIENT_LIST absent)");
}
p_sys->net_client_list = r ? r->atom : 0;
free (r);
r = xcb_intern_atom_reply (conn, nwn, NULL);
if (r != NULL)
{
p_sys->net_wm_name = r->atom;
free (r);
}

p_sys->apps = NULL;
p_sys->apps_root = input_item_NewExt(INPUT_ITEM_URI_NOP, _("Applications"),
INPUT_DURATION_INDEFINITE,
ITEM_TYPE_NODE, ITEM_LOCAL);
if (likely(p_sys->apps_root != NULL))
services_discovery_AddItem(sd, p_sys->apps_root);

UpdateApps (sd);

if (vlc_clone (&p_sys->thread, Run, sd, VLC_THREAD_PRIORITY_LOW))
goto error;
return VLC_SUCCESS;

error:
xcb_disconnect (p_sys->conn);
tdestroy (p_sys->apps, DelApp);
if (p_sys->apps_root != NULL)
input_item_Release(p_sys->apps_root);
free (p_sys);
return VLC_EGENERIC;
}





static void Close (vlc_object_t *obj)
{
services_discovery_t *sd = (services_discovery_t *)obj;
services_discovery_sys_t *p_sys = sd->p_sys;

vlc_cancel (p_sys->thread);
vlc_join (p_sys->thread, NULL);
xcb_disconnect (p_sys->conn);
tdestroy (p_sys->apps, DelApp);
if (p_sys->apps_root != NULL)
input_item_Release(p_sys->apps_root);
free (p_sys);
}

static void *Run (void *data)
{
services_discovery_t *sd = data;
services_discovery_sys_t *p_sys = sd->p_sys;
xcb_connection_t *conn = p_sys->conn;
int fd = xcb_get_file_descriptor (conn);
if (fd == -1)
return NULL;

while (!xcb_connection_has_error (conn))
{
xcb_generic_event_t *ev;
struct pollfd ufd = { .fd = fd, .events = POLLIN, };

poll (&ufd, 1, -1);

int canc = vlc_savecancel ();
while ((ev = xcb_poll_for_event (conn)) != NULL)
{
if ((ev->response_type & 0x7F) == XCB_PROPERTY_NOTIFY)
{
const xcb_property_notify_event_t *pn =
(xcb_property_notify_event_t *)ev;
if (pn->atom == p_sys->net_client_list)
UpdateApps (sd);
}
free (ev);
}
vlc_restorecancel (canc);
}
return NULL;
}


struct app
{
xcb_window_t xid; 
input_item_t *item;
services_discovery_t *owner;
};

static struct app *AddApp (services_discovery_t *sd, xcb_window_t xid)
{
services_discovery_sys_t *p_sys = sd->p_sys;
char *mrl, *name;

if (asprintf (&mrl, "window://0x%"PRIx8, xid) == -1)
return NULL;

xcb_get_property_reply_t *r =
xcb_get_property_reply (p_sys->conn,
xcb_get_property (p_sys->conn, 0, xid, p_sys->net_wm_name, 0,
0, 1023 ), NULL);
if (r != NULL)
{
name = strndup (xcb_get_property_value (r),
xcb_get_property_value_length (r));
if (name != NULL)
EnsureUTF8 (name); 
free (r);
}

else
name = NULL;

input_item_t *item = input_item_NewCard (mrl, name ? name : mrl); 
free (mrl);
free (name);
if (item == NULL)
return NULL;

struct app *app = malloc (sizeof (*app));
if (app == NULL)
{
input_item_Release (item);
return NULL;
}
app->xid = xid;
app->item = item;
app->owner = sd;
services_discovery_AddSubItem(sd, p_sys->apps_root, item);
return app;
}

static void DelApp (void *data)
{
struct app *app = data;

services_discovery_RemoveItem (app->owner, app->item);
input_item_Release (app->item);
free (app);
}

static int cmpapp (const void *a, const void *b)
{
xcb_window_t wa = *(xcb_window_t *)a;
xcb_window_t wb = *(xcb_window_t *)b;

if (wa > wb)
return 1;
if (wa < wb)
return -1;
return 0;
} 

static void UpdateApps (services_discovery_t *sd)
{
services_discovery_sys_t *p_sys = sd->p_sys;
xcb_connection_t *conn = p_sys->conn;

xcb_get_property_reply_t *r =
xcb_get_property_reply (conn,
xcb_get_property (conn, false, p_sys->root_window,
p_sys->net_client_list, XA_WINDOW, 0, 1024),
NULL);
if (r == NULL)
return; 

xcb_window_t *ent = xcb_get_property_value (r);
int n = xcb_get_property_value_length (r) / 4;
void *newnodes = NULL, *oldnodes = p_sys->apps;

for (int i = 0; i < n; i++)
{
xcb_window_t id = *(ent++);
struct app *app;

void **pa = tfind (&id, &oldnodes, cmpapp);
if (pa != NULL) 
{
app = *pa;
tdelete (app, &oldnodes, cmpapp);
}
else 
{
app = AddApp (sd, id);
if (app == NULL)
continue;
}

pa = tsearch (app, &newnodes, cmpapp);
if (pa == NULL || *pa != app )
DelApp (app);
}
free (r);


tdestroy (oldnodes, DelApp);
p_sys->apps = newnodes;
}


static void AddDesktop(services_discovery_t *sd)
{
input_item_t *item;

item = input_item_NewCard ("screen://", _("Desktop"));
if (item == NULL)
return;

services_discovery_AddItem(sd, item);
input_item_Release (item);
}
