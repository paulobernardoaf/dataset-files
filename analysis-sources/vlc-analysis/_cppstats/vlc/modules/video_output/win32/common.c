#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_vout_display.h>
#include <windows.h>
#include <assert.h>
#define COBJMACROS
#include <shobjidl.h>
#include "events.h"
#include "common.h"
#include "../../video_chroma/copy.h"
void CommonInit(vout_display_t *vd, display_win32_area_t *area, const vout_display_cfg_t *vdcfg)
{
area->place_changed = false;
area->vdcfg = *vdcfg;
area->texture_source = vd->source;
var_Create(vd, "disable-screensaver", VLC_VAR_BOOL | VLC_VAR_DOINHERIT);
}
#if !VLC_WINSTORE_APP
static void CommonChangeThumbnailClip(vlc_object_t *, vout_display_sys_win32_t *, bool show);
int CommonWindowInit(vlc_object_t *obj, display_win32_area_t *area,
vout_display_sys_win32_t *sys, bool projection_gestures)
{
if (unlikely(area->vdcfg.window == NULL))
return VLC_EGENERIC;
#if !defined(NDEBUG) && defined(HAVE_DXGIDEBUG_H)
sys->dxgidebug_dll = LoadLibrary(TEXT("DXGIDEBUG.DLL"));
#endif
sys->hvideownd = NULL;
sys->hparent = NULL;
sys->event = EventThreadCreate(obj, area->vdcfg.window);
if (!sys->event)
return VLC_EGENERIC;
event_cfg_t cfg;
memset(&cfg, 0, sizeof(cfg));
cfg.width = area->vdcfg.display.width;
cfg.height = area->vdcfg.display.height;
cfg.is_projected = projection_gestures;
event_hwnd_t hwnd;
if (EventThreadStart(sys->event, &hwnd, &cfg))
return VLC_EGENERIC;
sys->hparent = hwnd.hparent;
sys->hvideownd = hwnd.hvideownd;
CommonPlacePicture(obj, area, sys);
return VLC_SUCCESS;
}
#endif 
void CommonPlacePicture(vlc_object_t *obj, display_win32_area_t *area, vout_display_sys_win32_t *sys)
{
vout_display_cfg_t place_cfg = area->vdcfg;
#if (defined(MODULE_NAME_IS_glwin32))
if (place_cfg.align.vertical == VLC_VIDEO_ALIGN_TOP)
place_cfg.align.vertical = VLC_VIDEO_ALIGN_BOTTOM;
else if (place_cfg.align.vertical == VLC_VIDEO_ALIGN_BOTTOM)
place_cfg.align.vertical = VLC_VIDEO_ALIGN_TOP;
#endif
vout_display_place_t before_place = area->place;
vout_display_PlacePicture(&area->place, &area->texture_source, &place_cfg);
if (!vout_display_PlaceEquals(&before_place, &area->place))
{
area->place_changed |= true;
#if !defined(NDEBUG)
msg_Dbg(obj, "UpdateRects source offset: %i,%i visible: %ix%i decoded: %ix%i",
area->texture_source.i_x_offset, area->texture_source.i_y_offset,
area->texture_source.i_visible_width, area->texture_source.i_visible_height,
area->texture_source.i_width, area->texture_source.i_height);
msg_Dbg(obj, "UpdateRects image_dst coords: %i,%i %ix%i",
area->place.x, area->place.y, area->place.width, area->place.height);
#endif
#if !VLC_WINSTORE_APP
if (sys->event != NULL)
{
CommonChangeThumbnailClip(obj, sys, true);
}
#endif
}
}
#if !VLC_WINSTORE_APP
void CommonWindowClean(vlc_object_t *obj, vout_display_sys_win32_t *sys)
{
if (sys->event) {
CommonChangeThumbnailClip(obj, sys, false);
EventThreadStop(sys->event);
EventThreadDestroy(sys->event);
}
}
static void CommonChangeThumbnailClip(vlc_object_t *obj, vout_display_sys_win32_t *sys, bool show)
{
OSVERSIONINFO winVer;
winVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
if (!GetVersionEx(&winVer) || winVer.dwMajorVersion <= 5)
return;
if( FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)) )
vlc_assert_unreachable();
void *ptr;
if (S_OK == CoCreateInstance(&CLSID_TaskbarList,
NULL, CLSCTX_INPROC_SERVER,
&IID_ITaskbarList3,
&ptr)) {
ITaskbarList3 *taskbl = ptr;
taskbl->lpVtbl->HrInit(taskbl);
HWND hroot = GetAncestor(sys->hvideownd, GA_ROOT);
RECT video;
if (show) {
GetWindowRect(sys->hparent, &video);
POINT client = {video.left, video.top};
if (ScreenToClient(hroot, &client))
{
unsigned int width = RECTWidth(video);
unsigned int height = RECTHeight(video);
video.left = client.x;
video.top = client.y;
video.right = video.left + width;
video.bottom = video.top + height;
}
}
HRESULT hr;
hr = taskbl->lpVtbl->SetThumbnailClip(taskbl, hroot,
show ? &video : NULL);
if ( hr != S_OK )
msg_Err(obj, "SetThumbNailClip failed: 0x%lX", hr);
taskbl->lpVtbl->Release(taskbl);
}
CoUninitialize();
}
#endif 
int CommonControl(vlc_object_t *obj, display_win32_area_t *area, vout_display_sys_win32_t *sys, int query, va_list args)
{
switch (query) {
case VOUT_DISPLAY_CHANGE_DISPLAY_FILLED: 
case VOUT_DISPLAY_CHANGE_ZOOM: 
case VOUT_DISPLAY_CHANGE_SOURCE_ASPECT:
case VOUT_DISPLAY_CHANGE_SOURCE_CROP: {
area->vdcfg = *va_arg(args, const vout_display_cfg_t *);
CommonPlacePicture(obj, area, sys);
return VLC_SUCCESS;
}
case VOUT_DISPLAY_CHANGE_DISPLAY_SIZE: 
{ 
area->vdcfg = *va_arg(args, const vout_display_cfg_t *);
#if !VLC_WINSTORE_APP
if (sys->event != NULL)
{
RECT clientRect;
GetClientRect(sys->hparent, &clientRect);
area->vdcfg.display.width = RECTWidth(clientRect);
area->vdcfg.display.height = RECTHeight(clientRect);
SetWindowPos(sys->hvideownd, 0, 0, 0,
area->vdcfg.display.width,
area->vdcfg.display.height, SWP_NOZORDER|SWP_NOMOVE|SWP_NOACTIVATE);
}
#endif 
CommonPlacePicture(obj, area, sys);
return VLC_SUCCESS;
}
case VOUT_DISPLAY_RESET_PICTURES:
vlc_assert_unreachable();
default:
return VLC_EGENERIC;
}
}
