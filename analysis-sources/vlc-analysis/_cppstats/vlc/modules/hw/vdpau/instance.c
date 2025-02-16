#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <pthread.h>
#include <X11/Xlib.h>
#include <vlc_common.h>
#include "vlc_vdpau.h"
#pragma GCC visibility push(default)
typedef struct vdp_instance
{
Display *display;
vdp_t *vdp;
VdpDevice device;
int num; 
uintptr_t refs; 
struct vdp_instance *next;
char name[]; 
} vdp_instance_t;
static vdp_instance_t *vdp_instance_create(const char *name, int num)
{
size_t namelen = strlen(name) + 1;
vdp_instance_t *vi = malloc(sizeof (*vi) + namelen);
if (unlikely(vi == NULL))
return NULL;
vi->display = XOpenDisplay(name);
if (vi->display == NULL)
{
free(vi);
return NULL;
}
if (num >= 0)
vi->num = num;
else
vi->num = XDefaultScreen(vi->display);
VdpStatus err = vdp_create_x11(vi->display, vi->num,
&vi->vdp, &vi->device);
if (err != VDP_STATUS_OK)
{
XCloseDisplay(vi->display);
free(vi);
return NULL;
}
vi->next = NULL;
memcpy(vi->name, name, namelen);
vi->refs = 1;
return vi;
}
static void vdp_instance_destroy(vdp_instance_t *vi)
{
vdp_device_destroy(vi->vdp, vi->device);
vdp_destroy_x11(vi->vdp);
XCloseDisplay(vi->display);
free(vi);
}
static int vicmp(const char *name, int num, const vdp_instance_t *vi)
{
int val = strcmp(name, vi->name);
if (val)
return val;
if (num < 0)
num = XDefaultScreen(vi->display);
return num - vi->num;
}
static vdp_instance_t *list = NULL;
static vdp_instance_t *vdp_instance_lookup(const char *name, int num)
{
vdp_instance_t *vi = NULL;
for (vi = list; vi != NULL; vi = vi->next)
{
int val = vicmp(name, num, vi);
if (val == 0)
{
assert(vi->refs < UINTPTR_MAX);
vi->refs++;
break;
}
}
return vi;
}
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
VdpStatus vdp_get_x11(const char *display_name, int snum,
vdp_t **restrict vdpp, VdpDevice *restrict devicep)
{
vdp_instance_t *vi, *vi2;
if (display_name == NULL)
{
display_name = getenv("DISPLAY");
if (display_name == NULL)
return VDP_STATUS_ERROR;
}
pthread_mutex_lock(&lock);
vi = vdp_instance_lookup(display_name, snum);
pthread_mutex_unlock(&lock);
if (vi != NULL)
goto found;
vi = vdp_instance_create(display_name, snum);
if (vi == NULL)
return VDP_STATUS_ERROR;
pthread_mutex_lock(&lock);
vi2 = vdp_instance_lookup(display_name, snum);
if (unlikely(vi2 != NULL))
{ 
pthread_mutex_unlock(&lock);
vdp_instance_destroy(vi);
vi = vi2;
}
else
{
vi->next = list;
list = vi;
pthread_mutex_unlock(&lock);
}
found:
*vdpp = vi->vdp;
*devicep = vi->device;
return VDP_STATUS_OK;
}
vdp_t *vdp_hold_x11(vdp_t *vdp, VdpDevice *restrict devp)
{
vdp_instance_t *vi, **pp = &list;
pthread_mutex_lock(&lock);
for (;;)
{
vi = *pp;
assert(vi != NULL);
if (vi->vdp == vdp)
break;
pp = &vi->next;
}
assert(vi->refs < UINTPTR_MAX);
vi->refs++;
pthread_mutex_unlock(&lock);
if (devp != NULL)
*devp = vi->device;
return vdp;
}
void vdp_release_x11(vdp_t *vdp)
{
vdp_instance_t *vi, **pp = &list;
pthread_mutex_lock(&lock);
for (;;)
{
vi = *pp;
assert(vi != NULL);
if (vi->vdp == vdp)
break;
pp = &vi->next;
}
assert(vi->refs > 0);
vi->refs--;
if (vi->refs > 0)
vi = NULL; 
else
*pp = vi->next; 
pthread_mutex_unlock(&lock);
if (vi != NULL)
vdp_instance_destroy(vi);
}
