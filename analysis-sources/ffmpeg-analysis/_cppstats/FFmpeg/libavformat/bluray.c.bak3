





















#include <libbluray/bluray.h>

#include "libavutil/avstring.h"
#include "libavformat/avformat.h"
#include "libavformat/url.h"
#include "libavutil/opt.h"

#define BLURAY_PROTO_PREFIX "bluray:"
#define MIN_PLAYLIST_LENGTH 180 

typedef struct {
const AVClass *class;

BLURAY *bd;

int playlist;
int angle;
int chapter;

} BlurayContext;

#define OFFSET(x) offsetof(BlurayContext, x)
static const AVOption options[] = {
{"playlist", "", OFFSET(playlist), AV_OPT_TYPE_INT, { .i64=-1 }, -1, 99999, AV_OPT_FLAG_DECODING_PARAM },
{"angle", "", OFFSET(angle), AV_OPT_TYPE_INT, { .i64=0 }, 0, 0xfe, AV_OPT_FLAG_DECODING_PARAM },
{"chapter", "", OFFSET(chapter), AV_OPT_TYPE_INT, { .i64=1 }, 1, 0xfffe, AV_OPT_FLAG_DECODING_PARAM },

{NULL}
};

static const AVClass bluray_context_class = {
.class_name = "bluray",
.item_name = av_default_item_name,
.option = options,
.version = LIBAVUTIL_VERSION_INT,
};


static int check_disc_info(URLContext *h)
{
BlurayContext *bd = h->priv_data;
const BLURAY_DISC_INFO *disc_info;

disc_info = bd_get_disc_info(bd->bd);
if (!disc_info) {
av_log(h, AV_LOG_ERROR, "bd_get_disc_info() failed\n");
return -1;
}

if (!disc_info->bluray_detected) {
av_log(h, AV_LOG_ERROR, "BluRay disc not detected\n");
return -1;
}


if (disc_info->aacs_detected && !disc_info->aacs_handled) {
if (!disc_info->libaacs_detected) {
av_log(h, AV_LOG_ERROR,
"Media stream encrypted with AACS, install and configure libaacs\n");
} else {
av_log(h, AV_LOG_ERROR, "Your libaacs can't decrypt this media\n");
}
return -1;
}


if (disc_info->bdplus_detected && !disc_info->bdplus_handled) {






av_log(h, AV_LOG_ERROR, "Unable to decrypt BD+ encrypted media\n");

return -1;
}

return 0;
}

static int bluray_close(URLContext *h)
{
BlurayContext *bd = h->priv_data;
if (bd->bd) {
bd_close(bd->bd);
}

return 0;
}

static int bluray_open(URLContext *h, const char *path, int flags)
{
BlurayContext *bd = h->priv_data;
int num_title_idx;
const char *diskname = path;

av_strstart(path, BLURAY_PROTO_PREFIX, &diskname);

bd->bd = bd_open(diskname, NULL);
if (!bd->bd) {
av_log(h, AV_LOG_ERROR, "bd_open() failed\n");
return AVERROR(EIO);
}


if (check_disc_info(h) < 0) {
return AVERROR(EIO);
}










num_title_idx = bd_get_titles(bd->bd, TITLES_RELEVANT, MIN_PLAYLIST_LENGTH);
av_log(h, AV_LOG_INFO, "%d usable playlists:\n", num_title_idx);
if (num_title_idx < 1) {
return AVERROR(EIO);
}


if (bd->playlist < 0) {
uint64_t duration = 0;
int i;
for (i = 0; i < num_title_idx; i++) {
BLURAY_TITLE_INFO *info = bd_get_title_info(bd->bd, i, 0);

av_log(h, AV_LOG_INFO, "playlist %05d.mpls (%d:%02d:%02d)\n",
info->playlist,
((int)(info->duration / 90000) / 3600),
((int)(info->duration / 90000) % 3600) / 60,
((int)(info->duration / 90000) % 60));

if (info->duration > duration) {
bd->playlist = info->playlist;
duration = info->duration;
}

bd_free_title_info(info);
}
av_log(h, AV_LOG_INFO, "selected %05d.mpls\n", bd->playlist);
}


if (bd_select_playlist(bd->bd, bd->playlist) <= 0) {
av_log(h, AV_LOG_ERROR, "bd_select_playlist(%05d.mpls) failed\n", bd->playlist);
return AVERROR(EIO);
}


if (bd->angle >= 0) {
bd_select_angle(bd->bd, bd->angle);
}


if (bd->chapter > 1) {
bd_seek_chapter(bd->bd, bd->chapter - 1);
}

return 0;
}

static int bluray_read(URLContext *h, unsigned char *buf, int size)
{
BlurayContext *bd = h->priv_data;
int len;

if (!bd || !bd->bd) {
return AVERROR(EFAULT);
}

len = bd_read(bd->bd, buf, size);

return len == 0 ? AVERROR_EOF : len;
}

static int64_t bluray_seek(URLContext *h, int64_t pos, int whence)
{
BlurayContext *bd = h->priv_data;

if (!bd || !bd->bd) {
return AVERROR(EFAULT);
}

switch (whence) {
case SEEK_SET:
case SEEK_CUR:
case SEEK_END:
return bd_seek(bd->bd, pos);

case AVSEEK_SIZE:
return bd_get_title_size(bd->bd);
}

av_log(h, AV_LOG_ERROR, "Unsupported whence operation %d\n", whence);
return AVERROR(EINVAL);
}


const URLProtocol ff_bluray_protocol = {
.name = "bluray",
.url_close = bluray_close,
.url_open = bluray_open,
.url_read = bluray_read,
.url_seek = bluray_seek,
.priv_data_size = sizeof(BlurayContext),
.priv_data_class = &bluray_context_class,
};
