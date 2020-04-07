#include "libavutil/attributes.h"
#include "parser.h"
#include "vc1.h"
#include "get_bits.h"
#include "internal.h"
#define UNESCAPED_THRESHOLD 37
#define UNESCAPED_LIMIT 144
typedef enum {
NO_MATCH,
ONE_ZERO,
TWO_ZEROS,
ONE
} VC1ParseSearchState;
typedef struct VC1ParseContext {
ParseContext pc;
VC1Context v;
uint8_t prev_start_code;
size_t bytes_to_skip;
uint8_t unesc_buffer[UNESCAPED_LIMIT];
size_t unesc_index;
VC1ParseSearchState search_state;
} VC1ParseContext;
static void vc1_extract_header(AVCodecParserContext *s, AVCodecContext *avctx,
const uint8_t *buf, int buf_size)
{
VC1ParseContext *vpc = s->priv_data;
GetBitContext gb;
int ret;
vpc->v.s.avctx = avctx;
vpc->v.parse_only = 1;
init_get_bits8(&gb, buf, buf_size);
switch (vpc->prev_start_code) {
case VC1_CODE_SEQHDR & 0xFF:
ff_vc1_decode_sequence_header(avctx, &vpc->v, &gb);
break;
case VC1_CODE_ENTRYPOINT & 0xFF:
ff_vc1_decode_entry_point(avctx, &vpc->v, &gb);
break;
case VC1_CODE_FRAME & 0xFF:
if(vpc->v.profile < PROFILE_ADVANCED)
ret = ff_vc1_parse_frame_header (&vpc->v, &gb);
else
ret = ff_vc1_parse_frame_header_adv(&vpc->v, &gb);
if (ret < 0)
break;
if (vpc->v.s.pict_type == AV_PICTURE_TYPE_BI)
s->pict_type = AV_PICTURE_TYPE_B;
else
s->pict_type = vpc->v.s.pict_type;
if (avctx->ticks_per_frame > 1){
s->repeat_pict = 1;
if (vpc->v.rff){
s->repeat_pict = 2;
}else if (vpc->v.rptfrm){
s->repeat_pict = vpc->v.rptfrm * 2 + 1;
}
}else{
s->repeat_pict = 0;
}
if (vpc->v.broadcast && vpc->v.interlace && !vpc->v.psf)
s->field_order = vpc->v.tff ? AV_FIELD_TT : AV_FIELD_BB;
else
s->field_order = AV_FIELD_PROGRESSIVE;
break;
}
if (avctx->framerate.num)
avctx->time_base = av_inv_q(av_mul_q(avctx->framerate, (AVRational){avctx->ticks_per_frame, 1}));
s->format = vpc->v.chromaformat == 1 ? AV_PIX_FMT_YUV420P
: AV_PIX_FMT_NONE;
if (avctx->width && avctx->height) {
s->width = avctx->width;
s->height = avctx->height;
s->coded_width = FFALIGN(avctx->coded_width, 16);
s->coded_height = FFALIGN(avctx->coded_height, 16);
}
}
static int vc1_parse(AVCodecParserContext *s,
AVCodecContext *avctx,
const uint8_t **poutbuf, int *poutbuf_size,
const uint8_t *buf, int buf_size)
{
VC1ParseContext *vpc = s->priv_data;
int pic_found = vpc->pc.frame_start_found;
uint8_t *unesc_buffer = vpc->unesc_buffer;
size_t unesc_index = vpc->unesc_index;
VC1ParseSearchState search_state = vpc->search_state;
int start_code_found = 0;
int next = END_NOT_FOUND;
int i = vpc->bytes_to_skip;
if (pic_found && buf_size == 0) {
memset(unesc_buffer + unesc_index, 0, UNESCAPED_THRESHOLD - unesc_index);
vc1_extract_header(s, avctx, unesc_buffer, unesc_index);
next = 0;
}
while (i < buf_size) {
uint8_t b;
start_code_found = 0;
while (i < buf_size && unesc_index < UNESCAPED_THRESHOLD) {
b = buf[i++];
unesc_buffer[unesc_index++] = b;
if (search_state <= ONE_ZERO)
search_state = b ? NO_MATCH : search_state + 1;
else if (search_state == TWO_ZEROS) {
if (b == 1)
search_state = ONE;
else if (b > 1) {
if (b == 3)
unesc_index--; 
search_state = NO_MATCH;
}
}
else { 
search_state = NO_MATCH;
start_code_found = 1;
break;
}
}
if ((s->flags & PARSER_FLAG_COMPLETE_FRAMES) &&
unesc_index >= UNESCAPED_THRESHOLD &&
vpc->prev_start_code == (VC1_CODE_FRAME & 0xFF))
{
vc1_extract_header(s, avctx, unesc_buffer, unesc_index);
break;
}
if (unesc_index >= UNESCAPED_THRESHOLD && !start_code_found) {
while (i < buf_size) {
if (search_state == NO_MATCH) {
i += vpc->v.vc1dsp.startcode_find_candidate(buf + i, buf_size - i);
if (i < buf_size) {
search_state = ONE_ZERO;
}
i++;
} else {
b = buf[i++];
if (search_state == ONE_ZERO)
search_state = b ? NO_MATCH : TWO_ZEROS;
else if (search_state == TWO_ZEROS) {
if (b >= 1)
search_state = b == 1 ? ONE : NO_MATCH;
}
else { 
search_state = NO_MATCH;
start_code_found = 1;
break;
}
}
}
}
if (start_code_found) {
vc1_extract_header(s, avctx, unesc_buffer, unesc_index);
vpc->prev_start_code = b;
unesc_index = 0;
if (!(s->flags & PARSER_FLAG_COMPLETE_FRAMES)) {
if (!pic_found && (b == (VC1_CODE_FRAME & 0xFF) || b == (VC1_CODE_FIELD & 0xFF))) {
pic_found = 1;
}
else if (pic_found && b != (VC1_CODE_FIELD & 0xFF) && b != (VC1_CODE_SLICE & 0xFF)) {
next = i - 4;
pic_found = b == (VC1_CODE_FRAME & 0xFF);
break;
}
}
}
}
vpc->pc.frame_start_found = pic_found;
vpc->unesc_index = unesc_index;
vpc->search_state = search_state;
if (s->flags & PARSER_FLAG_COMPLETE_FRAMES) {
next = buf_size;
} else {
if (ff_combine_frame(&vpc->pc, next, &buf, &buf_size) < 0) {
vpc->bytes_to_skip = 0;
*poutbuf = NULL;
*poutbuf_size = 0;
return buf_size;
}
}
vpc->bytes_to_skip = 4;
if (next < 0 && next != END_NOT_FOUND)
vpc->bytes_to_skip += next;
*poutbuf = buf;
*poutbuf_size = buf_size;
return next;
}
static int vc1_split(AVCodecContext *avctx,
const uint8_t *buf, int buf_size)
{
uint32_t state = -1;
int charged = 0;
const uint8_t *ptr = buf, *end = buf + buf_size;
while (ptr < end) {
ptr = avpriv_find_start_code(ptr, end, &state);
if (state == VC1_CODE_SEQHDR || state == VC1_CODE_ENTRYPOINT) {
charged = 1;
} else if (charged && IS_MARKER(state))
return ptr - 4 - buf;
}
return 0;
}
static av_cold int vc1_parse_init(AVCodecParserContext *s)
{
VC1ParseContext *vpc = s->priv_data;
vpc->v.s.slice_context_count = 1;
vpc->v.first_pic_header_flag = 1;
vpc->prev_start_code = 0;
vpc->bytes_to_skip = 0;
vpc->unesc_index = 0;
vpc->search_state = NO_MATCH;
return ff_vc1_init_common(&vpc->v);
}
AVCodecParser ff_vc1_parser = {
.codec_ids = { AV_CODEC_ID_VC1 },
.priv_data_size = sizeof(VC1ParseContext),
.parser_init = vc1_parse_init,
.parser_parse = vc1_parse,
.parser_close = ff_parse_close,
.split = vc1_split,
};
