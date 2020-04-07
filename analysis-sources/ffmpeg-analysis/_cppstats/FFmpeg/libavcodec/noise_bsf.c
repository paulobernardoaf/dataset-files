#include <stdlib.h>
#include <string.h>
#include "avcodec.h"
#include "bsf.h"
#include "libavutil/log.h"
#include "libavutil/mem.h"
#include "libavutil/opt.h"
typedef struct NoiseContext {
const AVClass *class;
int amount;
int dropamount;
unsigned int state;
} NoiseContext;
static int noise(AVBSFContext *ctx, AVPacket *pkt)
{
NoiseContext *s = ctx->priv_data;
int amount = s->amount > 0 ? s->amount : (s->state % 10001 + 1);
int i, ret;
if (amount <= 0)
return AVERROR(EINVAL);
ret = ff_bsf_get_packet_ref(ctx, pkt);
if (ret < 0)
return ret;
if (s->dropamount > 0 && s->state % s->dropamount == 0) {
s->state++;
av_packet_unref(pkt);
return AVERROR(EAGAIN);
}
ret = av_packet_make_writable(pkt);
if (ret < 0) {
av_packet_unref(pkt);
return ret;
}
for (i = 0; i < pkt->size; i++) {
s->state += pkt->data[i] + 1;
if (s->state % amount == 0)
pkt->data[i] = s->state;
}
return 0;
}
#define OFFSET(x) offsetof(NoiseContext, x)
#define FLAGS (AV_OPT_FLAG_VIDEO_PARAM|AV_OPT_FLAG_AUDIO_PARAM|AV_OPT_FLAG_BSF_PARAM)
static const AVOption options[] = {
{ "amount", NULL, OFFSET(amount), AV_OPT_TYPE_INT, { .i64 = 0 }, 0, INT_MAX, FLAGS },
{ "dropamount", NULL, OFFSET(dropamount), AV_OPT_TYPE_INT, { .i64 = 0 }, 0, INT_MAX, FLAGS },
{ NULL },
};
static const AVClass noise_class = {
.class_name = "noise",
.item_name = av_default_item_name,
.option = options,
.version = LIBAVUTIL_VERSION_INT,
};
const AVBitStreamFilter ff_noise_bsf = {
.name = "noise",
.priv_data_size = sizeof(NoiseContext),
.priv_class = &noise_class,
.filter = noise,
};
