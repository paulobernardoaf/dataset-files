
























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <assert.h>
#include <errno.h>
#include <poll.h>

#include <vlc_common.h>
#include <vlc_access.h>
#include <vlc_interrupt.h>

#include "v4l2.h"

typedef struct
{
int fd;
uint32_t block_flags;
union
{
uint32_t bufc;
uint32_t blocksize;
};
struct buffer_t *bufv;
vlc_v4l2_ctrl_t *controls;
} access_sys_t;

static block_t *MMapBlock (stream_t *, bool *);
static block_t *ReadBlock (stream_t *, bool *);
static int AccessControl( stream_t *, int, va_list );
static int InitVideo(stream_t *, int, uint32_t);

int AccessOpen( vlc_object_t *obj )
{
stream_t *access = (stream_t *)obj;

if( access->b_preparsing )
return VLC_EGENERIC;

access_sys_t *sys = calloc (1, sizeof (*sys));
if( unlikely(sys == NULL) )
return VLC_ENOMEM;
access->p_sys = sys;

ParseMRL( obj, access->psz_location );

char *path = var_InheritString (obj, CFG_PREFIX"dev");
if (unlikely(path == NULL))
goto error; 

uint32_t caps;
int fd = OpenDevice (obj, path, &caps);
free (path);
if (fd == -1)
goto error;
sys->fd = fd;

if (InitVideo (access, fd, caps))
{
v4l2_close (fd);
goto error;
}

sys->controls = ControlsInit(vlc_object_parent(obj), fd);
access->pf_seek = NULL;
access->pf_control = AccessControl;
return VLC_SUCCESS;
error:
free (sys);
return VLC_EGENERIC;
}

int InitVideo (stream_t *access, int fd, uint32_t caps)
{
access_sys_t *sys = access->p_sys;

if (!(caps & V4L2_CAP_VIDEO_CAPTURE))
{
msg_Err (access, "not a video capture device");
return -1;
}

v4l2_std_id std;
if (SetupInput (VLC_OBJECT(access), fd, &std))
return -1;




uint32_t pixfmt = 0;
char *fmtstr = var_InheritString (access, CFG_PREFIX"chroma");
if (fmtstr != NULL && strlen (fmtstr) <= 4)
{
memcpy (&pixfmt, fmtstr, strlen (fmtstr));
free (fmtstr);
}
else

{
struct v4l2_format fmt = { .type = V4L2_BUF_TYPE_VIDEO_CAPTURE };
if (v4l2_ioctl (fd, VIDIOC_G_FMT, &fmt) < 0)
{
msg_Err (access, "cannot get default format: %s",
vlc_strerror_c(errno));
return -1;
}
pixfmt = fmt.fmt.pix.pixelformat;
}
msg_Dbg (access, "selected format %4.4s", (const char *)&pixfmt);

struct v4l2_format fmt;
struct v4l2_streamparm parm;
if (SetupFormat (access, fd, pixfmt, &fmt, &parm))
return -1;

msg_Dbg (access, "%"PRIu32" bytes for complete image", fmt.fmt.pix.sizeimage);

switch (fmt.fmt.pix.field)
{
case V4L2_FIELD_INTERLACED:
msg_Dbg (access, "Interlacing setting: interleaved");



sys->block_flags = BLOCK_FLAG_TOP_FIELD_FIRST;
break;
case V4L2_FIELD_INTERLACED_TB:
msg_Dbg (access, "Interlacing setting: interleaved top bottom" );
sys->block_flags = BLOCK_FLAG_TOP_FIELD_FIRST;
break;
case V4L2_FIELD_INTERLACED_BT:
msg_Dbg (access, "Interlacing setting: interleaved bottom top" );
sys->block_flags = BLOCK_FLAG_BOTTOM_FIELD_FIRST;
break;
default:
break;
}


if (caps & V4L2_CAP_STREAMING)
{
sys->bufc = 4;
sys->bufv = StartMmap (VLC_OBJECT(access), fd, &sys->bufc);
if (sys->bufv == NULL)
return -1;
access->pf_block = MMapBlock;
}
else if (caps & V4L2_CAP_READWRITE)
{
sys->blocksize = fmt.fmt.pix.sizeimage;
sys->bufv = NULL;
access->pf_block = ReadBlock;
}
else
{
msg_Err (access, "no supported capture method");
return -1;
}

return 0;
}

void AccessClose( vlc_object_t *obj )
{
stream_t *access = (stream_t *)obj;
access_sys_t *sys = access->p_sys;

if (sys->bufv != NULL)
StopMmap (sys->fd, sys->bufv, sys->bufc);
ControlsDeinit(vlc_object_parent(obj), sys->controls);
v4l2_close (sys->fd);
free( sys );
}


static int AccessPoll (stream_t *access)
{
access_sys_t *sys = access->p_sys;
struct pollfd ufd;

ufd.fd = sys->fd;
ufd.events = POLLIN;

return vlc_poll_i11e (&ufd, 1, -1);
}


static block_t *MMapBlock (stream_t *access, bool *restrict eof)
{
access_sys_t *sys = access->p_sys;

if (AccessPoll (access))
return NULL;

block_t *block = GrabVideo (VLC_OBJECT(access), sys->fd, sys->bufv);
if( block != NULL )
{
block->i_pts = block->i_dts = vlc_tick_now();
block->i_flags |= sys->block_flags;
}
(void) eof;
return block;
}

static block_t *ReadBlock (stream_t *access, bool *restrict eof)
{
access_sys_t *sys = access->p_sys;

if (AccessPoll (access))
return NULL;

block_t *block = block_Alloc (sys->blocksize);
if (unlikely(block == NULL))
return NULL;

ssize_t val = v4l2_read (sys->fd, block->p_buffer, block->i_buffer);
if (val < 0)
{
block_Release (block);
msg_Err (access, "cannot read buffer: %s", vlc_strerror_c(errno));
*eof = true;
return NULL;
}

block->i_buffer = val;
return block;
}

static int AccessControl( stream_t *access, int query, va_list args )
{
switch( query )
{
case STREAM_CAN_SEEK:
case STREAM_CAN_FASTSEEK:
case STREAM_CAN_PAUSE:
case STREAM_CAN_CONTROL_PACE:
*va_arg( args, bool* ) = false;
break;

case STREAM_GET_PTS_DELAY:
*va_arg(args,vlc_tick_t *) = VLC_TICK_FROM_MS(
var_InheritInteger( access, "live-caching" ) );
break;

case STREAM_SET_PAUSE_STATE:

break;

default:
return VLC_EGENERIC;

}
return VLC_SUCCESS;
}
