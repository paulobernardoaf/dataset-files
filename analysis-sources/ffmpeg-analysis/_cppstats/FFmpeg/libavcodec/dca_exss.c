#include "dcadec.h"
static void parse_xll_parameters(DCAExssParser *s, DCAExssAsset *asset)
{
asset->xll_size = get_bits(&s->gb, s->exss_size_nbits) + 1;
if (asset->xll_sync_present = get_bits1(&s->gb)) {
int xll_delay_nbits;
skip_bits(&s->gb, 4);
xll_delay_nbits = get_bits(&s->gb, 5) + 1;
asset->xll_delay_nframes = get_bits_long(&s->gb, xll_delay_nbits);
asset->xll_sync_offset = get_bits(&s->gb, s->exss_size_nbits);
} else {
asset->xll_delay_nframes = 0;
asset->xll_sync_offset = 0;
}
}
static void parse_lbr_parameters(DCAExssParser *s, DCAExssAsset *asset)
{
asset->lbr_size = get_bits(&s->gb, 14) + 1;
if (get_bits1(&s->gb))
skip_bits(&s->gb, 2);
}
static int parse_descriptor(DCAExssParser *s, DCAExssAsset *asset)
{
int i, j, drc_present, descr_size, descr_pos = get_bits_count(&s->gb);
descr_size = get_bits(&s->gb, 9) + 1;
asset->asset_index = get_bits(&s->gb, 3);
if (s->static_fields_present) {
if (get_bits1(&s->gb))
skip_bits(&s->gb, 4);
if (get_bits1(&s->gb))
skip_bits(&s->gb, 24);
if (get_bits1(&s->gb)) {
int text_size = get_bits(&s->gb, 10) + 1;
if (get_bits_left(&s->gb) < text_size * 8)
return AVERROR_INVALIDDATA;
skip_bits_long(&s->gb, text_size * 8);
}
asset->pcm_bit_res = get_bits(&s->gb, 5) + 1;
asset->max_sample_rate = ff_dca_sampling_freqs[get_bits(&s->gb, 4)];
asset->nchannels_total = get_bits(&s->gb, 8) + 1;
if (asset->one_to_one_map_ch_to_spkr = get_bits1(&s->gb)) {
int spkr_mask_nbits = 0;
int spkr_remap_nsets;
int nspeakers[8];
asset->embedded_stereo = asset->nchannels_total > 2 && get_bits1(&s->gb);
asset->embedded_6ch = asset->nchannels_total > 6 && get_bits1(&s->gb);
if (asset->spkr_mask_enabled = get_bits1(&s->gb)) {
spkr_mask_nbits = (get_bits(&s->gb, 2) + 1) << 2;
asset->spkr_mask = get_bits(&s->gb, spkr_mask_nbits);
}
if ((spkr_remap_nsets = get_bits(&s->gb, 3)) && !spkr_mask_nbits) {
if (s->avctx)
av_log(s->avctx, AV_LOG_ERROR, "Speaker mask disabled yet there are remapping sets\n");
return AVERROR_INVALIDDATA;
}
for (i = 0; i < spkr_remap_nsets; i++)
nspeakers[i] = ff_dca_count_chs_for_mask(get_bits(&s->gb, spkr_mask_nbits));
for (i = 0; i < spkr_remap_nsets; i++) {
int nch_for_remaps = get_bits(&s->gb, 5) + 1;
for (j = 0; j < nspeakers[i]; j++) {
int remap_ch_mask = get_bits_long(&s->gb, nch_for_remaps);
skip_bits_long(&s->gb, av_popcount(remap_ch_mask) * 5);
}
}
} else {
asset->embedded_stereo = 0;
asset->embedded_6ch = 0;
asset->spkr_mask_enabled = 0;
asset->spkr_mask = 0;
asset->representation_type = get_bits(&s->gb, 3);
}
}
drc_present = get_bits1(&s->gb);
if (drc_present)
skip_bits(&s->gb, 8);
if (get_bits1(&s->gb))
skip_bits(&s->gb, 5);
if (drc_present && asset->embedded_stereo)
skip_bits(&s->gb, 8);
if (s->mix_metadata_enabled && get_bits1(&s->gb)) {
int nchannels_dmix;
skip_bits1(&s->gb);
skip_bits(&s->gb, 6);
if (get_bits(&s->gb, 2) == 3)
skip_bits(&s->gb, 8);
else
skip_bits(&s->gb, 3);
if (get_bits1(&s->gb))
for (i = 0; i < s->nmixoutconfigs; i++)
skip_bits_long(&s->gb, 6 * s->nmixoutchs[i]);
else
skip_bits_long(&s->gb, 6 * s->nmixoutconfigs);
nchannels_dmix = asset->nchannels_total;
if (asset->embedded_6ch)
nchannels_dmix += 6;
if (asset->embedded_stereo)
nchannels_dmix += 2;
for (i = 0; i < s->nmixoutconfigs; i++) {
if (!s->nmixoutchs[i]) {
if (s->avctx)
av_log(s->avctx, AV_LOG_ERROR, "Invalid speaker layout mask for mixing configuration\n");
return AVERROR_INVALIDDATA;
}
for (j = 0; j < nchannels_dmix; j++) {
int mix_map_mask = get_bits(&s->gb, s->nmixoutchs[i]);
skip_bits_long(&s->gb, av_popcount(mix_map_mask) * 6);
}
}
}
asset->coding_mode = get_bits(&s->gb, 2);
switch (asset->coding_mode) {
case 0: 
asset->extension_mask = get_bits(&s->gb, 12);
if (asset->extension_mask & DCA_EXSS_CORE) {
asset->core_size = get_bits(&s->gb, 14) + 1;
if (get_bits1(&s->gb))
skip_bits(&s->gb, 2);
}
if (asset->extension_mask & DCA_EXSS_XBR)
asset->xbr_size = get_bits(&s->gb, 14) + 1;
if (asset->extension_mask & DCA_EXSS_XXCH)
asset->xxch_size = get_bits(&s->gb, 14) + 1;
if (asset->extension_mask & DCA_EXSS_X96)
asset->x96_size = get_bits(&s->gb, 12) + 1;
if (asset->extension_mask & DCA_EXSS_LBR)
parse_lbr_parameters(s, asset);
if (asset->extension_mask & DCA_EXSS_XLL)
parse_xll_parameters(s, asset);
if (asset->extension_mask & DCA_EXSS_RSV1)
skip_bits(&s->gb, 16);
if (asset->extension_mask & DCA_EXSS_RSV2)
skip_bits(&s->gb, 16);
break;
case 1: 
asset->extension_mask = DCA_EXSS_XLL;
parse_xll_parameters(s, asset);
break;
case 2: 
asset->extension_mask = DCA_EXSS_LBR;
parse_lbr_parameters(s, asset);
break;
case 3: 
asset->extension_mask = 0;
skip_bits(&s->gb, 14);
skip_bits(&s->gb, 8);
if (get_bits1(&s->gb))
skip_bits(&s->gb, 3);
break;
}
if (asset->extension_mask & DCA_EXSS_XLL)
asset->hd_stream_id = get_bits(&s->gb, 3);
if (ff_dca_seek_bits(&s->gb, descr_pos + descr_size * 8)) {
if (s->avctx)
av_log(s->avctx, AV_LOG_ERROR, "Read past end of EXSS asset descriptor\n");
return AVERROR_INVALIDDATA;
}
return 0;
}
static int set_exss_offsets(DCAExssAsset *asset)
{
int offs = asset->asset_offset;
int size = asset->asset_size;
if (asset->extension_mask & DCA_EXSS_CORE) {
asset->core_offset = offs;
if (asset->core_size > size)
return AVERROR_INVALIDDATA;
offs += asset->core_size;
size -= asset->core_size;
}
if (asset->extension_mask & DCA_EXSS_XBR) {
asset->xbr_offset = offs;
if (asset->xbr_size > size)
return AVERROR_INVALIDDATA;
offs += asset->xbr_size;
size -= asset->xbr_size;
}
if (asset->extension_mask & DCA_EXSS_XXCH) {
asset->xxch_offset = offs;
if (asset->xxch_size > size)
return AVERROR_INVALIDDATA;
offs += asset->xxch_size;
size -= asset->xxch_size;
}
if (asset->extension_mask & DCA_EXSS_X96) {
asset->x96_offset = offs;
if (asset->x96_size > size)
return AVERROR_INVALIDDATA;
offs += asset->x96_size;
size -= asset->x96_size;
}
if (asset->extension_mask & DCA_EXSS_LBR) {
asset->lbr_offset = offs;
if (asset->lbr_size > size)
return AVERROR_INVALIDDATA;
offs += asset->lbr_size;
size -= asset->lbr_size;
}
if (asset->extension_mask & DCA_EXSS_XLL) {
asset->xll_offset = offs;
if (asset->xll_size > size)
return AVERROR_INVALIDDATA;
offs += asset->xll_size;
size -= asset->xll_size;
}
return 0;
}
int ff_dca_exss_parse(DCAExssParser *s, const uint8_t *data, int size)
{
int i, ret, offset, wide_hdr, header_size;
if ((ret = init_get_bits8(&s->gb, data, size)) < 0)
return ret;
skip_bits_long(&s->gb, 32);
skip_bits(&s->gb, 8);
s->exss_index = get_bits(&s->gb, 2);
wide_hdr = get_bits1(&s->gb);
header_size = get_bits(&s->gb, 8 + 4 * wide_hdr) + 1;
if (s->avctx && ff_dca_check_crc(s->avctx, &s->gb, 32 + 8, header_size * 8)) {
av_log(s->avctx, AV_LOG_ERROR, "Invalid EXSS header checksum\n");
return AVERROR_INVALIDDATA;
}
s->exss_size_nbits = 16 + 4 * wide_hdr;
s->exss_size = get_bits(&s->gb, s->exss_size_nbits) + 1;
if (s->exss_size > size) {
if (s->avctx)
av_log(s->avctx, AV_LOG_ERROR, "Packet too short for EXSS frame\n");
return AVERROR_INVALIDDATA;
}
if (s->static_fields_present = get_bits1(&s->gb)) {
int active_exss_mask[8];
skip_bits(&s->gb, 2);
skip_bits(&s->gb, 3);
if (get_bits1(&s->gb))
skip_bits_long(&s->gb, 36);
s->npresents = get_bits(&s->gb, 3) + 1;
if (s->npresents > 1) {
if (s->avctx)
avpriv_request_sample(s->avctx, "%d audio presentations", s->npresents);
return AVERROR_PATCHWELCOME;
}
s->nassets = get_bits(&s->gb, 3) + 1;
if (s->nassets > 1) {
if (s->avctx)
avpriv_request_sample(s->avctx, "%d audio assets", s->nassets);
return AVERROR_PATCHWELCOME;
}
for (i = 0; i < s->npresents; i++)
active_exss_mask[i] = get_bits(&s->gb, s->exss_index + 1);
for (i = 0; i < s->npresents; i++)
skip_bits_long(&s->gb, av_popcount(active_exss_mask[i]) * 8);
if (s->mix_metadata_enabled = get_bits1(&s->gb)) {
int spkr_mask_nbits;
skip_bits(&s->gb, 2);
spkr_mask_nbits = (get_bits(&s->gb, 2) + 1) << 2;
s->nmixoutconfigs = get_bits(&s->gb, 2) + 1;
for (i = 0; i < s->nmixoutconfigs; i++)
s->nmixoutchs[i] = ff_dca_count_chs_for_mask(get_bits(&s->gb, spkr_mask_nbits));
}
} else {
s->npresents = 1;
s->nassets = 1;
}
offset = header_size;
for (i = 0; i < s->nassets; i++) {
s->assets[i].asset_offset = offset;
s->assets[i].asset_size = get_bits(&s->gb, s->exss_size_nbits) + 1;
offset += s->assets[i].asset_size;
if (offset > s->exss_size) {
if (s->avctx)
av_log(s->avctx, AV_LOG_ERROR, "EXSS asset out of bounds\n");
return AVERROR_INVALIDDATA;
}
}
for (i = 0; i < s->nassets; i++) {
if ((ret = parse_descriptor(s, &s->assets[i])) < 0)
return ret;
if ((ret = set_exss_offsets(&s->assets[i])) < 0) {
if (s->avctx)
av_log(s->avctx, AV_LOG_ERROR, "Invalid extension size in EXSS asset descriptor\n");
return ret;
}
}
if (ff_dca_seek_bits(&s->gb, header_size * 8)) {
if (s->avctx)
av_log(s->avctx, AV_LOG_ERROR, "Read past end of EXSS header\n");
return AVERROR_INVALIDDATA;
}
return 0;
}
