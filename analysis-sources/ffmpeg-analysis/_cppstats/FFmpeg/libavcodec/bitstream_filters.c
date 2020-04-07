#include "config.h"
#include "libavutil/common.h"
#include "libavutil/log.h"
#include "avcodec.h"
#include "bsf.h"
extern const AVBitStreamFilter ff_aac_adtstoasc_bsf;
extern const AVBitStreamFilter ff_av1_frame_merge_bsf;
extern const AVBitStreamFilter ff_av1_frame_split_bsf;
extern const AVBitStreamFilter ff_av1_metadata_bsf;
extern const AVBitStreamFilter ff_chomp_bsf;
extern const AVBitStreamFilter ff_dump_extradata_bsf;
extern const AVBitStreamFilter ff_dca_core_bsf;
extern const AVBitStreamFilter ff_eac3_core_bsf;
extern const AVBitStreamFilter ff_extract_extradata_bsf;
extern const AVBitStreamFilter ff_filter_units_bsf;
extern const AVBitStreamFilter ff_h264_metadata_bsf;
extern const AVBitStreamFilter ff_h264_mp4toannexb_bsf;
extern const AVBitStreamFilter ff_h264_redundant_pps_bsf;
extern const AVBitStreamFilter ff_hapqa_extract_bsf;
extern const AVBitStreamFilter ff_hevc_metadata_bsf;
extern const AVBitStreamFilter ff_hevc_mp4toannexb_bsf;
extern const AVBitStreamFilter ff_imx_dump_header_bsf;
extern const AVBitStreamFilter ff_mjpeg2jpeg_bsf;
extern const AVBitStreamFilter ff_mjpega_dump_header_bsf;
extern const AVBitStreamFilter ff_mp3_header_decompress_bsf;
extern const AVBitStreamFilter ff_mpeg2_metadata_bsf;
extern const AVBitStreamFilter ff_mpeg4_unpack_bframes_bsf;
extern const AVBitStreamFilter ff_mov2textsub_bsf;
extern const AVBitStreamFilter ff_noise_bsf;
extern const AVBitStreamFilter ff_null_bsf;
extern const AVBitStreamFilter ff_prores_metadata_bsf;
extern const AVBitStreamFilter ff_remove_extradata_bsf;
extern const AVBitStreamFilter ff_text2movsub_bsf;
extern const AVBitStreamFilter ff_trace_headers_bsf;
extern const AVBitStreamFilter ff_truehd_core_bsf;
extern const AVBitStreamFilter ff_vp9_metadata_bsf;
extern const AVBitStreamFilter ff_vp9_raw_reorder_bsf;
extern const AVBitStreamFilter ff_vp9_superframe_bsf;
extern const AVBitStreamFilter ff_vp9_superframe_split_bsf;
#include "libavcodec/bsf_list.c"
const AVBitStreamFilter *av_bsf_iterate(void **opaque)
{
uintptr_t i = (uintptr_t)*opaque;
const AVBitStreamFilter *f = bitstream_filters[i];
if (f)
*opaque = (void*)(i + 1);
return f;
}
#if FF_API_NEXT
const AVBitStreamFilter *av_bsf_next(void **opaque) {
return av_bsf_iterate(opaque);
}
#endif
const AVBitStreamFilter *av_bsf_get_by_name(const char *name)
{
const AVBitStreamFilter *f = NULL;
void *i = 0;
if (!name)
return NULL;
while ((f = av_bsf_iterate(&i))) {
if (!strcmp(f->name, name))
return f;
}
return NULL;
}
const AVClass *ff_bsf_child_class_next(const AVClass *prev)
{
const AVBitStreamFilter *f = NULL;
void *i = 0;
while (prev && (f = av_bsf_iterate(&i))) {
if (f->priv_class == prev) {
break;
}
}
while ((f = av_bsf_iterate(&i))) {
if (f->priv_class)
return f->priv_class;
}
return NULL;
}
