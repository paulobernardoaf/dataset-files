

















#include <stdlib.h>

#include "libavutil/common.h"
#include "libavutil/opt.h"

#include "bsf.h"
#include "cbs.h"


typedef struct FilterUnitsContext {
const AVClass *class;

CodedBitstreamContext *cbc;
CodedBitstreamFragment fragment;

const char *pass_types;
const char *remove_types;

enum {
NOOP,
PASS,
REMOVE,
} mode;
CodedBitstreamUnitType *type_list;
int nb_types;
} FilterUnitsContext;


static int filter_units_make_type_list(const char *list_string,
CodedBitstreamUnitType **type_list,
int *nb_types)
{
CodedBitstreamUnitType *list = NULL;
int pass, count;

for (pass = 1; pass <= 2; pass++) {
long value, range_start, range_end;
const char *str;
char *value_end;

count = 0;
for (str = list_string; *str;) {
value = strtol(str, &value_end, 0);
if (str == value_end)
goto invalid;
str = (const char *)value_end;
if (*str == '-') {
++str;
range_start = value;
range_end = strtol(str, &value_end, 0);
if (str == value_end)
goto invalid;

for (value = range_start; value < range_end; value++) {
if (pass == 2)
list[count] = value;
++count;
}
} else {
if (pass == 2)
list[count] = value;
++count;
}
if (*str == '|')
++str;
}
if (pass == 1) {
list = av_malloc_array(count, sizeof(*list));
if (!list)
return AVERROR(ENOMEM);
}
}

*type_list = list;
*nb_types = count;
return 0;

invalid:
av_freep(&list);
return AVERROR(EINVAL);
}

static int filter_units_filter(AVBSFContext *bsf, AVPacket *pkt)
{
FilterUnitsContext *ctx = bsf->priv_data;
CodedBitstreamFragment *frag = &ctx->fragment;
int err, i, j;

err = ff_bsf_get_packet_ref(bsf, pkt);
if (err < 0)
return err;

if (ctx->mode == NOOP)
return 0;

err = ff_cbs_read_packet(ctx->cbc, frag, pkt);
if (err < 0) {
av_log(bsf, AV_LOG_ERROR, "Failed to read packet.\n");
goto fail;
}

for (i = frag->nb_units - 1; i >= 0; i--) {
for (j = 0; j < ctx->nb_types; j++) {
if (frag->units[i].type == ctx->type_list[j])
break;
}
if (ctx->mode == REMOVE ? j < ctx->nb_types
: j >= ctx->nb_types)
ff_cbs_delete_unit(ctx->cbc, frag, i);
}

if (frag->nb_units == 0) {

err = AVERROR(EAGAIN);
goto fail;
}

err = ff_cbs_write_packet(ctx->cbc, pkt, frag);
if (err < 0) {
av_log(bsf, AV_LOG_ERROR, "Failed to write packet.\n");
goto fail;
}

fail:
if (err < 0)
av_packet_unref(pkt);
ff_cbs_fragment_reset(ctx->cbc, frag);

return err;
}

static int filter_units_init(AVBSFContext *bsf)
{
FilterUnitsContext *ctx = bsf->priv_data;
int err;

if (ctx->pass_types && ctx->remove_types) {
av_log(bsf, AV_LOG_ERROR, "Exactly one of pass_types or "
"remove_types is required.\n");
return AVERROR(EINVAL);
}

if (ctx->pass_types) {
ctx->mode = PASS;
err = filter_units_make_type_list(ctx->pass_types,
&ctx->type_list, &ctx->nb_types);
if (err < 0) {
av_log(bsf, AV_LOG_ERROR, "Failed to parse pass_types.\n");
return err;
}
} else if (ctx->remove_types) {
ctx->mode = REMOVE;
err = filter_units_make_type_list(ctx->remove_types,
&ctx->type_list, &ctx->nb_types);
if (err < 0) {
av_log(bsf, AV_LOG_ERROR, "Failed to parse remove_types.\n");
return err;
}
} else {
return 0;
}

err = ff_cbs_init(&ctx->cbc, bsf->par_in->codec_id, bsf);
if (err < 0)
return err;


ctx->cbc->decompose_unit_types = ctx->type_list;
ctx->cbc->nb_decompose_unit_types = 0;

if (bsf->par_in->extradata) {
CodedBitstreamFragment *frag = &ctx->fragment;

err = ff_cbs_read_extradata(ctx->cbc, frag, bsf->par_in);
if (err < 0) {
av_log(bsf, AV_LOG_ERROR, "Failed to read extradata.\n");
} else {
err = ff_cbs_write_extradata(ctx->cbc, bsf->par_out, frag);
if (err < 0)
av_log(bsf, AV_LOG_ERROR, "Failed to write extradata.\n");
}

ff_cbs_fragment_reset(ctx->cbc, frag);
}

return err;
}

static void filter_units_close(AVBSFContext *bsf)
{
FilterUnitsContext *ctx = bsf->priv_data;

av_freep(&ctx->type_list);

ff_cbs_fragment_free(ctx->cbc, &ctx->fragment);
ff_cbs_close(&ctx->cbc);
}

#define OFFSET(x) offsetof(FilterUnitsContext, x)
#define FLAGS (AV_OPT_FLAG_VIDEO_PARAM|AV_OPT_FLAG_BSF_PARAM)
static const AVOption filter_units_options[] = {
{ "pass_types", "List of unit types to pass through the filter.",
OFFSET(pass_types), AV_OPT_TYPE_STRING,
{ .str = NULL }, .flags = FLAGS },
{ "remove_types", "List of unit types to remove in the filter.",
OFFSET(remove_types), AV_OPT_TYPE_STRING,
{ .str = NULL }, .flags = FLAGS },

{ NULL }
};

static const AVClass filter_units_class = {
.class_name = "filter_units",
.item_name = av_default_item_name,
.option = filter_units_options,
.version = LIBAVUTIL_VERSION_INT,
};

const AVBitStreamFilter ff_filter_units_bsf = {
.name = "filter_units",
.priv_data_size = sizeof(FilterUnitsContext),
.priv_class = &filter_units_class,
.init = &filter_units_init,
.close = &filter_units_close,
.filter = &filter_units_filter,
.codec_ids = ff_cbs_all_codec_ids,
};
