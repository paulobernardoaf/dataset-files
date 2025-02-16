
























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <math.h>
#include <errno.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#if !defined(MAP_ANONYMOUS)
#define MAP_ANONYMOUS MAP_ANON
#endif
#include <poll.h>

#include <vlc_common.h>
#include <vlc_demux.h>

#include "v4l2.h"

typedef struct
{
int fd;
vlc_thread_t thread;

struct buffer_t *bufv;
union
{
uint32_t bufc;
uint32_t blocksize;
};
uint32_t block_flags;

es_out_id_t *es;
vlc_v4l2_ctrl_t *controls;
vlc_tick_t start;

#if defined(ZVBI_COMPILED)
vlc_v4l2_vbi_t *vbi;
#endif
} demux_sys_t;

static void *UserPtrThread (void *);
static void *MmapThread (void *);
static void *ReadThread (void *);
static int DemuxControl( demux_t *, int, va_list );
static int InitVideo (demux_t *, int fd, uint32_t caps);

int DemuxOpen( vlc_object_t *obj )
{
demux_t *demux = (demux_t *)obj;
if (demux->out == NULL)
return VLC_EGENERIC;

demux_sys_t *sys = malloc (sizeof (*sys));
if (unlikely(sys == NULL))
return VLC_ENOMEM;
demux->p_sys = sys;
#if defined(ZVBI_COMPILED)
sys->vbi = NULL;
#endif

ParseMRL( obj, demux->psz_location );

char *path = var_InheritString (obj, CFG_PREFIX"dev");
if (unlikely(path == NULL))
goto error; 

uint32_t caps;
int fd = OpenDevice (obj, path, &caps);
free (path);
if (fd == -1)
goto error;
sys->fd = fd;

if (InitVideo (demux, fd, caps))
{
v4l2_close (fd);
goto error;
}

sys->controls = ControlsInit(vlc_object_parent(obj), fd);
sys->start = vlc_tick_now ();
demux->pf_demux = NULL;
demux->pf_control = DemuxControl;
return VLC_SUCCESS;
error:
free (sys);
return VLC_EGENERIC;
}

typedef struct
{
uint32_t v4l2;
vlc_fourcc_t vlc;
uint8_t bpp; 
uint32_t red;
uint32_t green;
uint32_t blue;
} vlc_v4l2_fmt_t;



static const vlc_v4l2_fmt_t v4l2_fmts[] =
{

{ V4L2_PIX_FMT_YUV420, VLC_CODEC_I420, 1, 0, 0, 0 },
{ V4L2_PIX_FMT_YVU420, VLC_CODEC_YV12, 1, 0, 0, 0 },
{ V4L2_PIX_FMT_YUV422P, VLC_CODEC_I422, 1, 0, 0, 0 },

{ V4L2_PIX_FMT_YUYV, VLC_CODEC_YUYV, 2, 0, 0, 0 },
{ V4L2_PIX_FMT_UYVY, VLC_CODEC_UYVY, 2, 0, 0, 0 },
{ V4L2_PIX_FMT_YVYU, VLC_CODEC_YVYU, 2, 0, 0, 0 },
{ V4L2_PIX_FMT_VYUY, VLC_CODEC_VYUY, 2, 0, 0, 0 },

{ V4L2_PIX_FMT_YUV411P, VLC_CODEC_I411, 1, 0, 0, 0 },

{ V4L2_PIX_FMT_YUV410, VLC_CODEC_I410, 1, 0, 0, 0 },






{ V4L2_PIX_FMT_NV12, VLC_CODEC_NV12, 1, 0, 0, 0 },
{ V4L2_PIX_FMT_NV21, VLC_CODEC_NV21, 1, 0, 0, 0 },







#if defined(WORDS_BIGENDIAN)
{ V4L2_PIX_FMT_RGB32, VLC_CODEC_RGB32, 4, 0xFF00, 0xFF0000, 0xFF000000 },
{ V4L2_PIX_FMT_BGR32, VLC_CODEC_RGB32, 4, 0xFF000000, 0xFF0000, 0xFF00 },
{ V4L2_PIX_FMT_RGB24, VLC_CODEC_RGB24, 3, 0xFF0000, 0x00FF00, 0x0000FF },
{ V4L2_PIX_FMT_BGR24, VLC_CODEC_RGB24, 3, 0x0000FF, 0x00FF00, 0xFF0000 },


{ V4L2_PIX_FMT_RGB565X, VLC_CODEC_RGB16, 2, 0x001F, 0x07E0, 0xF800 },

{ V4L2_PIX_FMT_RGB555X, VLC_CODEC_RGB15, 2, 0x001F, 0x03E0, 0x7C00 },

#else
{ V4L2_PIX_FMT_RGB32, VLC_CODEC_RGB32, 4, 0x0000FF, 0x00FF00, 0xFF0000 },
{ V4L2_PIX_FMT_BGR32, VLC_CODEC_RGB32, 4, 0xFF0000, 0x00FF00, 0x0000FF },
{ V4L2_PIX_FMT_RGB24, VLC_CODEC_RGB24, 3, 0x0000FF, 0x00FF00, 0xFF0000 },
{ V4L2_PIX_FMT_BGR24, VLC_CODEC_RGB24, 3, 0xFF0000, 0x00FF00, 0x0000FF },

{ V4L2_PIX_FMT_RGB565, VLC_CODEC_RGB16, 2, 0x001F, 0x07E0, 0xF800 },

{ V4L2_PIX_FMT_RGB555, VLC_CODEC_RGB15, 2, 0x001F, 0x03E0, 0x7C00 },


#endif


















{ V4L2_PIX_FMT_JPEG, VLC_CODEC_MJPG, 0, 0, 0, 0 },
{ V4L2_PIX_FMT_H264, VLC_CODEC_H264, 0, 0, 0, 0 },


{ V4L2_PIX_FMT_MPEG4, VLC_CODEC_MP4V, 0, 0, 0, 0 },
{ V4L2_PIX_FMT_XVID, VLC_CODEC_MP4V, 0, 0, 0, 0 },
{ V4L2_PIX_FMT_H263, VLC_CODEC_H263, 0, 0, 0, 0 },
{ V4L2_PIX_FMT_MPEG2, VLC_CODEC_MPGV, 0, 0, 0, 0 },
{ V4L2_PIX_FMT_MPEG1, VLC_CODEC_MPGV, 0, 0, 0, 0 },
{ V4L2_PIX_FMT_VC1_ANNEX_G, VLC_CODEC_VC1, 0, 0, 0, 0 },
{ V4L2_PIX_FMT_VC1_ANNEX_L, VLC_CODEC_VC1, 0, 0, 0, 0 },



{ V4L2_PIX_FMT_MJPEG, VLC_CODEC_MJPG, 0, 0, 0, 0 },



{ V4L2_PIX_FMT_Y16, VLC_CODEC_GREY_16L, 2, 0, 0, 0 },
{ V4L2_PIX_FMT_Y12, VLC_CODEC_GREY_12L, 2, 0, 0, 0 },
{ V4L2_PIX_FMT_Y10, VLC_CODEC_GREY_10L, 2, 0, 0, 0 },

{ V4L2_PIX_FMT_GREY, VLC_CODEC_GREY, 1, 0, 0, 0 },
};

static const vlc_v4l2_fmt_t *vlc_from_v4l2_fourcc (uint32_t fourcc)
{
for (size_t i = 0; i < sizeof (v4l2_fmts) / sizeof (v4l2_fmts[0]); i++)
if (v4l2_fmts[i].v4l2 == fourcc)
return v4l2_fmts + i;
return NULL;
}

static size_t vlc_v4l2_fmt_rank (const vlc_v4l2_fmt_t *fmt)
{
if (fmt == NULL)
return SIZE_MAX;

ptrdiff_t d = fmt - v4l2_fmts;
assert (d >= 0);
assert (d < (ptrdiff_t)(sizeof (v4l2_fmts) / sizeof (v4l2_fmts[0])));
return d;
}

static vlc_fourcc_t var_InheritFourCC (vlc_object_t *obj, const char *varname)
{
char *str = var_InheritString (obj, varname);
if (str == NULL)
return 0;

vlc_fourcc_t fourcc = vlc_fourcc_GetCodecFromString (VIDEO_ES, str);
if (fourcc == 0)
msg_Err (obj, "invalid codec %s", str);
free (str);
return fourcc;
}
#define var_InheritFourCC(o, v) var_InheritFourCC(VLC_OBJECT(o), v)

static void GetAR (int fd, unsigned *restrict num, unsigned *restrict den)
{
struct v4l2_cropcap cropcap = { .type = V4L2_BUF_TYPE_VIDEO_CAPTURE };


if (v4l2_ioctl (fd, VIDIOC_CROPCAP, &cropcap) < 0)
{
*num = *den = 1;
return;
}
*num = cropcap.pixelaspect.numerator;
*den = cropcap.pixelaspect.denominator;
}

static int InitVideo (demux_t *demux, int fd, uint32_t caps)
{
demux_sys_t *sys = demux->p_sys;
v4l2_std_id std;

if (!(caps & V4L2_CAP_VIDEO_CAPTURE))
{
msg_Err (demux, "not a video capture device");
return -1;
}

if (SetupInput (VLC_OBJECT(demux), fd, &std))
return -1;


const vlc_v4l2_fmt_t *selected = NULL;
vlc_fourcc_t reqfourcc = var_InheritFourCC (demux, CFG_PREFIX"chroma");
bool native = false;

for (struct v4l2_fmtdesc codec = { .type = V4L2_BUF_TYPE_VIDEO_CAPTURE };
v4l2_ioctl (fd, VIDIOC_ENUM_FMT, &codec) >= 0;
codec.index++)
{ 
const vlc_v4l2_fmt_t *dsc = vlc_from_v4l2_fourcc (codec.pixelformat);

msg_Dbg (demux, " %s %s format %4.4s (%4.4s): %s",
(codec.flags & V4L2_FMT_FLAG_EMULATED) ? "emulates" : "supports",
(codec.flags & V4L2_FMT_FLAG_COMPRESSED) ? "compressed" : "raw",
(char *)&codec.pixelformat,
(dsc != NULL) ? (const char *)&dsc->vlc : "N.A.",
codec.description);

if (dsc == NULL)
continue; 

if (dsc->vlc == reqfourcc)
{
msg_Dbg (demux, " matches the requested format");
selected = dsc;
break; 
}

if (codec.flags & V4L2_FMT_FLAG_EMULATED)
{
if (native)
continue; 
}
else
native = true;

if (vlc_v4l2_fmt_rank (dsc) > vlc_v4l2_fmt_rank (selected))
continue; 

selected = dsc;
}

if (selected == NULL)
{
msg_Err (demux, "cannot negotiate supported video format");
return -1;
}
msg_Dbg (demux, "selected format %4.4s (%4.4s)",
(const char *)&selected->v4l2, (const char *)&selected->vlc);


struct v4l2_format fmt;
struct v4l2_streamparm parm;
if (SetupFormat (demux, fd, selected->v4l2, &fmt, &parm))
return -1;


msg_Dbg (demux, "%d bytes maximum for complete image",
fmt.fmt.pix.sizeimage);

sys->block_flags = 0;
switch (fmt.fmt.pix.field)
{
case V4L2_FIELD_NONE:
msg_Dbg (demux, "Interlacing setting: progressive");
break;
case V4L2_FIELD_TOP:
msg_Dbg (demux, "Interlacing setting: top field only");
sys->block_flags = BLOCK_FLAG_TOP_FIELD_FIRST|BLOCK_FLAG_SINGLE_FIELD;
break;
case V4L2_FIELD_BOTTOM:
msg_Dbg (demux, "Interlacing setting: bottom field only");
sys->block_flags = BLOCK_FLAG_BOTTOM_FIELD_FIRST|BLOCK_FLAG_SINGLE_FIELD;
break;
case V4L2_FIELD_INTERLACED:
msg_Dbg (demux, "Interlacing setting: interleaved");



sys->block_flags = BLOCK_FLAG_TOP_FIELD_FIRST;
break;
case V4L2_FIELD_SEQ_TB:
msg_Dbg (demux, "Interlacing setting: sequential top bottom (TODO)");
break;
case V4L2_FIELD_SEQ_BT:
msg_Dbg (demux, "Interlacing setting: sequential bottom top (TODO)");
break;
case V4L2_FIELD_ALTERNATE:
msg_Dbg (demux, "Interlacing setting: alternate fields (TODO)");
fmt.fmt.pix.height *= 2;
break;
case V4L2_FIELD_INTERLACED_TB:
msg_Dbg (demux, "Interlacing setting: interleaved top bottom");
sys->block_flags = BLOCK_FLAG_TOP_FIELD_FIRST;
break;
case V4L2_FIELD_INTERLACED_BT:
msg_Dbg (demux, "Interlacing setting: interleaved bottom top");
sys->block_flags = BLOCK_FLAG_BOTTOM_FIELD_FIRST;
break;
default:
msg_Warn (demux, "Interlacing setting: unknown type (%d)",
fmt.fmt.pix.field);
break;
}


es_format_t es_fmt;

es_format_Init (&es_fmt, VIDEO_ES, selected->vlc);
es_fmt.video.i_chroma = selected->vlc;
es_fmt.video.i_rmask = selected->red;
es_fmt.video.i_gmask = selected->green;
es_fmt.video.i_bmask = selected->blue;
es_fmt.video.i_visible_width = fmt.fmt.pix.width;
if (fmt.fmt.pix.bytesperline != 0 && selected->bpp != 0)
es_fmt.video.i_width = fmt.fmt.pix.bytesperline / selected->bpp;
else
es_fmt.video.i_width = fmt.fmt.pix.width;
es_fmt.video.i_visible_height =
es_fmt.video.i_height = fmt.fmt.pix.height;
es_fmt.video.i_frame_rate = parm.parm.capture.timeperframe.denominator;
es_fmt.video.i_frame_rate_base = parm.parm.capture.timeperframe.numerator;
GetAR (fd, &es_fmt.video.i_sar_num, &es_fmt.video.i_sar_den);

msg_Dbg (demux, "color primaries: %u", fmt.fmt.pix.colorspace);
switch (fmt.fmt.pix.colorspace)
{
case V4L2_COLORSPACE_DEFAULT:
break;
case V4L2_COLORSPACE_SMPTE170M:
es_fmt.video.primaries = COLOR_PRIMARIES_BT601_525;
es_fmt.video.transfer = TRANSFER_FUNC_BT709;
es_fmt.video.space = COLOR_SPACE_BT601;
break;
case V4L2_COLORSPACE_SMPTE240M: 
break;
case V4L2_COLORSPACE_REC709:
es_fmt.video.primaries = COLOR_PRIMARIES_BT709;
es_fmt.video.transfer = TRANSFER_FUNC_BT709;
es_fmt.video.space = COLOR_SPACE_BT709;
break;
case V4L2_COLORSPACE_470_SYSTEM_M:
es_fmt.video.transfer = TRANSFER_FUNC_BT709;
es_fmt.video.space = COLOR_SPACE_BT601;
break;
case V4L2_COLORSPACE_470_SYSTEM_BG:
es_fmt.video.primaries = COLOR_PRIMARIES_BT601_625;
es_fmt.video.transfer = TRANSFER_FUNC_BT709;
es_fmt.video.space = COLOR_SPACE_BT601;
break;
case V4L2_COLORSPACE_JPEG:
es_fmt.video.primaries = COLOR_PRIMARIES_SRGB;
es_fmt.video.transfer = TRANSFER_FUNC_SRGB;
es_fmt.video.space = COLOR_SPACE_BT601;
es_fmt.video.color_range = COLOR_RANGE_FULL;
break;
case V4L2_COLORSPACE_SRGB:
es_fmt.video.primaries = COLOR_PRIMARIES_SRGB;
es_fmt.video.transfer = TRANSFER_FUNC_SRGB;
es_fmt.video.space = COLOR_SPACE_UNDEF; 
break;
case V4L2_COLORSPACE_ADOBERGB: 
es_fmt.video.space = COLOR_SPACE_BT601;
break;
case V4L2_COLORSPACE_BT2020:
es_fmt.video.primaries = COLOR_PRIMARIES_BT2020;
es_fmt.video.transfer = TRANSFER_FUNC_BT2020;
es_fmt.video.space = COLOR_SPACE_BT2020;
break;
case V4L2_COLORSPACE_RAW:
es_fmt.video.transfer = TRANSFER_FUNC_LINEAR;
break;
case V4L2_COLORSPACE_DCI_P3:
es_fmt.video.primaries = COLOR_PRIMARIES_DCI_P3;
es_fmt.video.transfer = TRANSFER_FUNC_UNDEF;
es_fmt.video.space = COLOR_SPACE_BT2020;
break;
default:
msg_Warn (demux, "unknown color space %u", fmt.fmt.pix.colorspace);
break;
}

msg_Dbg (demux, "transfer function: %u", fmt.fmt.pix.xfer_func);
switch (fmt.fmt.pix.xfer_func)
{
case V4L2_XFER_FUNC_DEFAULT:



break;
case V4L2_XFER_FUNC_709:
es_fmt.video.transfer = TRANSFER_FUNC_BT709;
break;
case V4L2_XFER_FUNC_SRGB:
es_fmt.video.transfer = TRANSFER_FUNC_SRGB;
break;
case V4L2_XFER_FUNC_ADOBERGB:
case V4L2_XFER_FUNC_SMPTE240M:
es_fmt.video.transfer = TRANSFER_FUNC_UNDEF;
break;
case V4L2_XFER_FUNC_NONE:
es_fmt.video.transfer = TRANSFER_FUNC_LINEAR;
break;
case V4L2_XFER_FUNC_DCI_P3:
case V4L2_XFER_FUNC_SMPTE2084:
es_fmt.video.transfer = TRANSFER_FUNC_UNDEF;
break;
default:
msg_Warn (demux, "unknown transfer function %u",
fmt.fmt.pix.xfer_func);
break;
}

msg_Dbg (demux, "YCbCr encoding: %u", fmt.fmt.pix.ycbcr_enc);
switch (fmt.fmt.pix.ycbcr_enc)
{
case V4L2_YCBCR_ENC_DEFAULT:

break;
case V4L2_YCBCR_ENC_601:
es_fmt.video.space = COLOR_SPACE_BT601;
break;
case V4L2_YCBCR_ENC_709:
es_fmt.video.space = COLOR_SPACE_BT709;
break;
case V4L2_YCBCR_ENC_XV601:
case V4L2_YCBCR_ENC_XV709:
case V4L2_YCBCR_ENC_SYCC:
break;
case V4L2_YCBCR_ENC_BT2020:
es_fmt.video.space = COLOR_SPACE_BT2020;
break;
case V4L2_YCBCR_ENC_BT2020_CONST_LUM:
case V4L2_YCBCR_ENC_SMPTE240M:
break;
default:
msg_Err (demux, "unknown YCbCr encoding: %u",
fmt.fmt.pix.ycbcr_enc);
break;
}

msg_Dbg (demux, "quantization: %u", fmt.fmt.pix.quantization);
switch (fmt.fmt.pix.quantization)
{
case V4L2_QUANTIZATION_DEFAULT:
break;
case V4L2_QUANTIZATION_FULL_RANGE:
es_fmt.video.color_range = COLOR_RANGE_FULL;
break;
case V4L2_QUANTIZATION_LIM_RANGE:
es_fmt.video.color_range = COLOR_RANGE_LIMITED;
break;
default:
msg_Err (demux, "unknown quantization: %u",
fmt.fmt.pix.quantization);
break;
}

msg_Dbg (demux, "added new video ES %4.4s %ux%u (%ux%u)",
(char *)&es_fmt.i_codec,
es_fmt.video.i_visible_width, es_fmt.video.i_visible_height,
es_fmt.video.i_width, es_fmt.video.i_height);
msg_Dbg (demux, " frame rate: %u/%u", es_fmt.video.i_frame_rate,
es_fmt.video.i_frame_rate_base);
msg_Dbg (demux, " aspect ratio: %u/%u", es_fmt.video.i_sar_num,
es_fmt.video.i_sar_den);
sys->es = es_out_Add (demux->out, &es_fmt);


void *(*entry) (void *);
if (caps & V4L2_CAP_STREAMING)
{
if (0 && StartUserPtr (VLC_OBJECT(demux), fd) == 0)
{



const long pagemask = sysconf (_SC_PAGE_SIZE) - 1;

sys->blocksize = (fmt.fmt.pix.sizeimage + pagemask) & ~pagemask;
sys->bufv = NULL;
entry = UserPtrThread;
msg_Dbg (demux, "streaming with %"PRIu32"-bytes user buffers",
sys->blocksize);
}
else 
{
sys->bufc = 4;
sys->bufv = StartMmap (VLC_OBJECT(demux), fd, &sys->bufc);
if (sys->bufv == NULL)
return -1;
entry = MmapThread;
msg_Dbg (demux, "streaming with %"PRIu32" memory-mapped buffers",
sys->bufc);
}
}
else if (caps & V4L2_CAP_READWRITE)
{
sys->blocksize = fmt.fmt.pix.sizeimage;
sys->bufv = NULL;
entry = ReadThread;
msg_Dbg (demux, "reading %"PRIu32" bytes at a time", sys->blocksize);
}
else
{
msg_Err (demux, "no supported capture method");
return -1;
}

#if defined(ZVBI_COMPILED)
if (std & V4L2_STD_NTSC_M)
{
char *vbi_path = var_InheritString (demux, CFG_PREFIX"vbidev");
if (vbi_path != NULL)
sys->vbi = OpenVBI (demux, vbi_path);
free(vbi_path);
}
#endif

if (vlc_clone (&sys->thread, entry, demux, VLC_THREAD_PRIORITY_INPUT))
{
#if defined(ZVBI_COMPILED)
if (sys->vbi != NULL)
CloseVBI (sys->vbi);
#endif
if (sys->bufv != NULL)
StopMmap (sys->fd, sys->bufv, sys->bufc);
return -1;
}
return 0;
}

void DemuxClose( vlc_object_t *obj )
{
demux_t *demux = (demux_t *)obj;
demux_sys_t *sys = demux->p_sys;

vlc_cancel (sys->thread);
vlc_join (sys->thread, NULL);
if (sys->bufv != NULL)
StopMmap (sys->fd, sys->bufv, sys->bufc);
ControlsDeinit(vlc_object_parent(obj), sys->controls);
v4l2_close (sys->fd);

#if defined(ZVBI_COMPILED)
if (sys->vbi != NULL)
CloseVBI (sys->vbi);
#endif

free( sys );
}


static block_t *UserPtrQueue (vlc_object_t *obj, int fd, size_t length)
{
void *ptr = mmap (NULL, length, PROT_READ | PROT_WRITE,
MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
if (ptr == MAP_FAILED)
{
msg_Err (obj, "cannot allocate %zu-bytes buffer: %s", length,
vlc_strerror_c(errno));
return NULL;
}

block_t *block = block_mmap_Alloc (ptr, length);
if (unlikely(block == NULL))
{
munmap (ptr, length);
return NULL;
}

struct v4l2_buffer buf = {
.type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
.memory = V4L2_MEMORY_USERPTR,
.m = {
.userptr = (uintptr_t)ptr,
},
.length = length,
};

if (v4l2_ioctl (fd, VIDIOC_QBUF, &buf) < 0)
{
msg_Err (obj, "cannot queue buffer: %s", vlc_strerror_c(errno));
block_Release (block);
return NULL;
}
return block;
}

static void *UserPtrThread (void *data)
{
demux_t *demux = data;
demux_sys_t *sys = demux->p_sys;
int fd = sys->fd;
struct pollfd ufd[2];
nfds_t numfds = 1;

ufd[0].fd = fd;
ufd[0].events = POLLIN;

int canc = vlc_savecancel ();
for (;;)
{
struct v4l2_buffer buf = {
.type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
.memory = V4L2_MEMORY_USERPTR,
};
block_t *block = UserPtrQueue (VLC_OBJECT(demux), fd, sys->blocksize);
if (block == NULL)
break;


vlc_restorecancel (canc);
block_cleanup_push (block);
while (poll (ufd, numfds, -1) == -1)
if (errno != EINTR)
msg_Err (demux, "poll error: %s", vlc_strerror_c(errno));
vlc_cleanup_pop ();
canc = vlc_savecancel ();

if (v4l2_ioctl (fd, VIDIOC_DQBUF, &buf) < 0)
{
msg_Err (demux, "cannot dequeue buffer: %s",
vlc_strerror_c(errno));
block_Release (block);
continue;
}

assert (block->p_buffer == (void *)buf.m.userptr);
block->i_buffer = buf.length;
block->i_pts = block->i_dts = GetBufferPTS (&buf);
block->i_flags |= sys->block_flags;
es_out_SetPCR(demux->out, block->i_pts);
es_out_Send (demux->out, sys->es, block);
}
vlc_restorecancel (canc); 
return NULL;
}

static void *MmapThread (void *data)
{
demux_t *demux = data;
demux_sys_t *sys = demux->p_sys;
int fd = sys->fd;
struct pollfd ufd[2];
nfds_t numfds = 1;

ufd[0].fd = fd;
ufd[0].events = POLLIN;

#if defined(ZVBI_COMPILED)
if (sys->vbi != NULL)
{
ufd[1].fd = GetFdVBI (sys->vbi);
ufd[1].events = POLLIN;
numfds++;
}
#endif

for (;;)
{

if (poll (ufd, numfds, -1) == -1)
{
if (errno != EINTR)
msg_Err (demux, "poll error: %s", vlc_strerror_c(errno));
continue;
}

if( ufd[0].revents )
{
int canc = vlc_savecancel ();
block_t *block = GrabVideo (VLC_OBJECT(demux), fd, sys->bufv);
if (block != NULL)
{
block->i_flags |= sys->block_flags;
es_out_SetPCR(demux->out, block->i_pts);
es_out_Send (demux->out, sys->es, block);
}
vlc_restorecancel (canc);
}
#if defined(ZVBI_COMPILED)
if (sys->vbi != NULL && ufd[1].revents)
GrabVBI (demux, sys->vbi);
#endif
}

vlc_assert_unreachable ();
}

static void *ReadThread (void *data)
{
demux_t *demux = data;
demux_sys_t *sys = demux->p_sys;
int fd = sys->fd;
struct pollfd ufd[2];
nfds_t numfds = 1;

ufd[0].fd = fd;
ufd[0].events = POLLIN;

#if defined(ZVBI_COMPILED)
if (sys->vbi != NULL)
{
ufd[1].fd = GetFdVBI (sys->vbi);
ufd[1].events = POLLIN;
numfds++;
}
#endif

for (;;)
{

if (poll (ufd, numfds, -1) == -1)
{
if (errno != EINTR)
msg_Err (demux, "poll error: %s", vlc_strerror_c(errno));
continue;
}

if( ufd[0].revents )
{
block_t *block = block_Alloc (sys->blocksize);
if (unlikely(block == NULL))
{
msg_Err (demux, "read error: %s", vlc_strerror_c(errno));
v4l2_read (fd, NULL, 0); 
continue;
}
block->i_pts = block->i_dts = vlc_tick_now ();
block->i_flags |= sys->block_flags;

int canc = vlc_savecancel ();
ssize_t val = v4l2_read (fd, block->p_buffer, block->i_buffer);
if (val != -1)
{
block->i_buffer = val;
es_out_SetPCR(demux->out, block->i_pts);
es_out_Send (demux->out, sys->es, block);
}
else
block_Release (block);
vlc_restorecancel (canc);
}
#if defined(ZVBI_COMPILED)
if (sys->vbi != NULL && ufd[1].revents)
GrabVBI (demux, sys->vbi);
#endif
}
vlc_assert_unreachable ();
}

static int DemuxControl( demux_t *demux, int query, va_list args )
{
demux_sys_t *sys = demux->p_sys;

switch( query )
{

case DEMUX_CAN_PAUSE:
case DEMUX_CAN_SEEK:
case DEMUX_CAN_CONTROL_PACE:
*va_arg( args, bool * ) = false;
return VLC_SUCCESS;

case DEMUX_GET_PTS_DELAY:
*va_arg(args,vlc_tick_t *) = VLC_TICK_FROM_MS(
var_InheritInteger( demux, "live-caching" ) );
return VLC_SUCCESS;

case DEMUX_GET_TIME:
*va_arg (args, vlc_tick_t *) = vlc_tick_now() - sys->start;
return VLC_SUCCESS;


default:
return VLC_EGENERIC;
}

return VLC_EGENERIC;
}
