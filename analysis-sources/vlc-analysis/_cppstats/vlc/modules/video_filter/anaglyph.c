#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_filter.h>
#include <vlc_picture.h>
#include "filter_picture.h"
static int Create(vlc_object_t *);
static void Destroy(vlc_object_t *);
static picture_t *Filter(filter_t *, picture_t *);
static void combine_side_by_side_yuv420(picture_t *, picture_t *, int, int);
#define SCHEME_TEXT N_("Color scheme")
#define SCHEME_LONGTEXT N_("Define the glasses' color scheme")
#define FILTER_PREFIX "anaglyph-"
enum scheme_e
{
red_green = 1,
red_blue,
red_cyan,
trioscopic,
magenta_cyan,
};
static const char *const ppsz_scheme_values[] = {
"red-green",
"red-blue",
"red-cyan",
"trioscopic",
"magenta-cyan",
};
static const char *const ppsz_scheme_descriptions[] = {
"pure red (left) pure green (right)",
"pure red (left) pure blue (right)",
"pure red (left) pure cyan (right)",
"pure green (left) pure magenta (right)",
"magenta (left) cyan (right)",
};
vlc_module_begin()
set_description(N_("Convert 3D picture to anaglyph image video filter"));
set_shortname(N_("Anaglyph"))
set_category(CAT_VIDEO)
set_subcategory(SUBCAT_VIDEO_VFILTER)
set_capability("video filter", 0)
add_string(FILTER_PREFIX "scheme", "red-cyan", SCHEME_TEXT, SCHEME_LONGTEXT, false)
change_string_list(ppsz_scheme_values, ppsz_scheme_descriptions)
set_callbacks(Create, Destroy)
vlc_module_end()
static const char *const ppsz_filter_options[] = {
"scheme", NULL
};
typedef struct
{
int left, right;
} filter_sys_t;
static int Create(vlc_object_t *p_this)
{
filter_t *p_filter = (filter_t *)p_this;
switch (p_filter->fmt_in.video.i_chroma)
{
case VLC_CODEC_I420:
case VLC_CODEC_J420:
case VLC_CODEC_YV12:
break;
default:
msg_Err(p_filter, "Unsupported input chroma (%4.4s)",
(char*)&(p_filter->fmt_in.video.i_chroma));
return VLC_EGENERIC;
}
p_filter->p_sys = malloc(sizeof(filter_sys_t));
if (!p_filter->p_sys)
return VLC_ENOMEM;
filter_sys_t *p_sys = p_filter->p_sys;
config_ChainParse(p_filter, FILTER_PREFIX, ppsz_filter_options,
p_filter->p_cfg);
char *psz_scheme = var_CreateGetStringCommand(p_filter,
FILTER_PREFIX "scheme");
enum scheme_e scheme = red_cyan;
if (psz_scheme)
{
if (!strcmp(psz_scheme, "red-green"))
scheme = red_green;
else if (!strcmp(psz_scheme, "red-blue"))
scheme = red_blue;
else if (!strcmp(psz_scheme, "red-cyan"))
scheme = red_cyan;
else if (!strcmp(psz_scheme, "trioscopic"))
scheme = trioscopic;
else if (!strcmp(psz_scheme, "magenta-cyan"))
scheme = magenta_cyan;
else
msg_Err(p_filter, "Unknown anaglyph color scheme '%s'", psz_scheme);
}
free(psz_scheme);
switch (scheme)
{
case red_green:
p_sys->left = 0xff0000;
p_sys->right = 0x00ff00;
break;
case red_blue:
p_sys->left = 0xff0000;
p_sys->right = 0x0000ff;
break;
case red_cyan:
p_sys->left = 0xff0000;
p_sys->right = 0x00ffff;
break;
case trioscopic:
p_sys->left = 0x00ff00;
p_sys->right = 0xff00ff;
break;
case magenta_cyan:
p_sys->left = 0xff00ff;
p_sys->right = 0x00ffff;
break;
}
p_filter->pf_video_filter = Filter;
return VLC_SUCCESS;
}
static void Destroy(vlc_object_t *p_this)
{
filter_t *p_filter = (filter_t *)p_this;
filter_sys_t *p_sys = p_filter->p_sys;
free(p_sys);
}
static picture_t *Filter(filter_t *p_filter, picture_t *p_pic)
{
filter_sys_t *p_sys = p_filter->p_sys;
if (!p_pic)
return NULL;
picture_t *p_outpic = filter_NewPicture(p_filter);
if (!p_outpic)
{
picture_Release(p_pic);
return NULL;
}
switch (p_pic->format.i_chroma)
{
case VLC_CODEC_I420:
case VLC_CODEC_J420:
case VLC_CODEC_YV12:
combine_side_by_side_yuv420(p_pic, p_outpic,
p_sys->left, p_sys->right);
break;
default:
msg_Warn(p_filter, "Unsupported input chroma (%4.4s)",
(char*)&(p_pic->format.i_chroma));
picture_Release(p_pic);
return NULL;
}
return CopyInfoAndRelease(p_outpic, p_pic);
}
static void combine_side_by_side_yuv420(picture_t *p_inpic, picture_t *p_outpic,
int left, int right)
{
uint8_t *y1inl = p_inpic->p[Y_PLANE].p_pixels;
uint8_t *y2inl;
uint8_t *uinl = p_inpic->p[U_PLANE].p_pixels;
uint8_t *vinl = p_inpic->p[V_PLANE].p_pixels;
uint8_t *y1out = p_outpic->p[Y_PLANE].p_pixels;
uint8_t *y2out;
uint8_t *uout = p_outpic->p[U_PLANE].p_pixels;
uint8_t *vout = p_outpic->p[V_PLANE].p_pixels;
const int in_pitch = p_inpic->p[Y_PLANE].i_pitch;
const int out_pitch = p_outpic->p[Y_PLANE].i_pitch;
const int visible_pitch = p_inpic->p[Y_PLANE].i_visible_pitch;
const int visible_lines = p_inpic->p[Y_PLANE].i_visible_lines;
const int uv_visible_pitch = p_inpic->p[U_PLANE].i_visible_pitch;
const uint8_t *yend = y1inl + visible_lines * in_pitch;
while (y1inl < yend)
{
uint8_t *y1inr = y1inl + visible_pitch/2;
uint8_t *y2inr;
uint8_t *uinr = uinl + uv_visible_pitch/2;
uint8_t *vinr = vinl + uv_visible_pitch/2;
const uint8_t *y1end = y1inr;
y2inl = y1inl + in_pitch;
y2inr = y1inr + in_pitch;
y2out = y1out + out_pitch;
while (y1inl < y1end)
{
int rl, gl, bl, rr, gr, br, r, g, b;
int rshift = !!((0xff0000&left) && (0xff0000&right));
int gshift = !!((0x00ff00&left) && (0x00ff00&right));
int bshift = !!((0x0000ff&left) && (0x0000ff&right));
yuv_to_rgb(&rl, &gl, &bl, *y1inl, *uinl, *vinl);
yuv_to_rgb(&rr, &gr, &br, *y1inr, *uinr, *vinr);
r = ((!!(0xff0000&left))*rl + (!!(0xff0000&right))*rr)>>rshift;
g = ((!!(0x00ff00&left))*gl + (!!(0x00ff00&right))*gr)>>gshift;
b = ((!!(0x0000ff&left))*bl + (!!(0x0000ff&right))*br)>>bshift;
rgb_to_yuv(y1out, uout++, vout++, r, g, b);
y1out[1] = *y1out;
y1out+=2;
y1inl++;
y1inr++;
yuv_to_rgb(&rl, &gl, &bl, *y1inl, *uinl, *vinl);
yuv_to_rgb(&rr, &gr, &br, *y1inr, *uinr, *vinr);
r = ((!!(0xff0000&left))*rl + (!!(0xff0000&right))*rr)>>rshift;
g = ((!!(0x00ff00&left))*gl + (!!(0x00ff00&right))*gr)>>gshift;
b = ((!!(0x0000ff&left))*bl + (!!(0x0000ff&right))*br)>>bshift;
rgb_to_yuv(y1out, uout++, vout++, r, g, b);
y1out[1] = *y1out;
y1out+=2;
y1inl++;
y1inr++;
yuv_to_rgb(&rl, &gl, &bl, *y2inl, *uinl, *vinl);
yuv_to_rgb(&rr, &gr, &br, *y2inr, *uinr, *vinr);
r = ((!!(0xff0000&left))*rl + (!!(0xff0000&right))*rr)>>rshift;
g = ((!!(0x00ff00&left))*gl + (!!(0x00ff00&right))*gr)>>gshift;
b = ((!!(0x0000ff&left))*bl + (!!(0x0000ff&right))*br)>>bshift;
rgb_to_yuv(y2out, uout, vout, r, g, b);
y2out[1] = *y2out;
y2out+=2;
y2inl++;
y2inr++;
yuv_to_rgb(&rl, &gl, &bl, *y2inl, *uinl, *vinl);
yuv_to_rgb(&rr, &gr, &br, *y2inr, *uinr, *vinr);
r = ((!!(0xff0000&left))*rl + (!!(0xff0000&right))*rr)>>rshift;
g = ((!!(0x00ff00&left))*gl + (!!(0x00ff00&right))*gr)>>gshift;
b = ((!!(0x0000ff&left))*bl + (!!(0x0000ff&right))*br)>>bshift;
rgb_to_yuv(y2out, uout, vout, r, g, b);
y2out[1] = *y2out;
y2out+=2;
y2inl++;
y2inr++;
uinl++;
vinl++;
uinr++;
vinr++;
}
y1inl = y1inr + 2*in_pitch - visible_pitch;
y1out += 2*out_pitch - visible_pitch;
uinl = uinr + p_inpic->p[U_PLANE].i_pitch - uv_visible_pitch;
vinl = vinr + p_inpic->p[V_PLANE].i_pitch - uv_visible_pitch;
uout += p_outpic->p[U_PLANE].i_pitch - uv_visible_pitch;
vout += p_outpic->p[V_PLANE].i_pitch - uv_visible_pitch;
}
}
