




























#include <string.h>

#include "libavutil/intreadwrite.h"
#include "libavutil/mem.h"

#include "parser.h"

#define DIRAC_PARSE_INFO_PREFIX 0x42424344





typedef struct DiracParseContext {
int state;
int is_synced;
int sync_offset;
int header_bytes_needed;
int overread_index;
int buffer_size;
int index;
uint8_t *buffer;
int dirac_unit_size;
uint8_t *dirac_unit;
} DiracParseContext;

static int find_frame_end(DiracParseContext *pc,
const uint8_t *buf, int buf_size)
{
uint32_t state = pc->state;
int i = 0;

if (!pc->is_synced) {
for (i = 0; i < buf_size; i++) {
state = (state << 8) | buf[i];
if (state == DIRAC_PARSE_INFO_PREFIX) {
state = -1;
pc->is_synced = 1;
pc->header_bytes_needed = 9;
pc->sync_offset = i;
break;
}
}
}

if (pc->is_synced) {
pc->sync_offset = 0;
for (; i < buf_size; i++) {
if (state == DIRAC_PARSE_INFO_PREFIX) {
if ((buf_size - i) >= pc->header_bytes_needed) {
pc->state = -1;
return i + pc->header_bytes_needed;
} else {
pc->header_bytes_needed = 9 - (buf_size - i);
break;
}
} else
state = (state << 8) | buf[i];
}
}
pc->state = state;
return -1;
}

typedef struct DiracParseUnit {
int next_pu_offset;
int prev_pu_offset;
uint8_t pu_type;
} DiracParseUnit;

static int unpack_parse_unit(DiracParseUnit *pu, DiracParseContext *pc,
int offset)
{
int i;
int8_t *start;
static const uint8_t valid_pu_types[] = {
0x00, 0x10, 0x20, 0x30, 0x08, 0x48, 0xC8, 0xE8, 0x0A, 0x0C, 0x0D, 0x0E,
0x4C, 0x09, 0xCC, 0x88, 0xCB
};

if (offset < 0 || pc->index - 13 < offset)
return 0;

start = pc->buffer + offset;
pu->pu_type = start[4];

pu->next_pu_offset = AV_RB32(start + 5);
pu->prev_pu_offset = AV_RB32(start + 9);


for (i = 0; i < 17; i++)
if (valid_pu_types[i] == pu->pu_type)
break;
if (i == 17)
return 0;

if (pu->pu_type == 0x10 && pu->next_pu_offset == 0x00)
pu->next_pu_offset = 13; 


if ((pu->next_pu_offset && pu->next_pu_offset < 13) ||
(pu->prev_pu_offset && pu->prev_pu_offset < 13))
return 0;

return 1;
}

static int dirac_combine_frame(AVCodecParserContext *s, AVCodecContext *avctx,
int next, const uint8_t **buf, int *buf_size)
{
int parse_timing_info = (s->pts == AV_NOPTS_VALUE &&
s->dts == AV_NOPTS_VALUE);
DiracParseContext *pc = s->priv_data;

if (pc->overread_index) {
memmove(pc->buffer, pc->buffer + pc->overread_index,
pc->index - pc->overread_index);
pc->index -= pc->overread_index;
pc->overread_index = 0;
if (*buf_size == 0 && pc->buffer[4] == 0x10) {
*buf = pc->buffer;
*buf_size = pc->index;
return 0;
}
}

if (next == -1) {

void *new_buffer =
av_fast_realloc(pc->buffer, &pc->buffer_size,
pc->index + (*buf_size - pc->sync_offset));
if (!new_buffer)
return AVERROR(ENOMEM);
pc->buffer = new_buffer;
memcpy(pc->buffer + pc->index, (*buf + pc->sync_offset),
*buf_size - pc->sync_offset);
pc->index += *buf_size - pc->sync_offset;
return -1;
} else {

DiracParseUnit pu1, pu;
void *new_buffer = av_fast_realloc(pc->buffer, &pc->buffer_size,
pc->index + next);
if (!new_buffer)
return AVERROR(ENOMEM);
pc->buffer = new_buffer;
memcpy(pc->buffer + pc->index, *buf, next);
pc->index += next;







if (!unpack_parse_unit(&pu1, pc, pc->index - 13) ||
!unpack_parse_unit(&pu, pc, pc->index - 13 - pu1.prev_pu_offset) ||
pu.next_pu_offset != pu1.prev_pu_offset ||
pc->index < pc->dirac_unit_size + 13LL + pu1.prev_pu_offset
) {
pc->index -= 9;
*buf_size = next - 9;
pc->header_bytes_needed = 9;
return -1;
}





pc->dirac_unit = pc->buffer + pc->index - 13 -
pu1.prev_pu_offset - pc->dirac_unit_size;

pc->dirac_unit_size += pu.next_pu_offset;

if ((pu.pu_type & 0x08) != 0x08) {
pc->header_bytes_needed = 9;
*buf_size = next;
return -1;
}


if (parse_timing_info && pu1.prev_pu_offset >= 13) {
uint8_t *cur_pu = pc->buffer +
pc->index - 13 - pu1.prev_pu_offset;
int64_t pts = AV_RB32(cur_pu + 13);
if (s->last_pts == 0 && s->last_dts == 0)
s->dts = pts - 1;
else
s->dts = s->last_dts + 1;
s->pts = pts;
if (!avctx->has_b_frames && (cur_pu[4] & 0x03))
avctx->has_b_frames = 1;
}
if (avctx->has_b_frames && s->pts == s->dts)
s->pict_type = AV_PICTURE_TYPE_B;


*buf = pc->dirac_unit;
*buf_size = pc->dirac_unit_size;

pc->dirac_unit_size = 0;
pc->overread_index = pc->index - 13;
pc->header_bytes_needed = 9;
}
return next;
}

static int dirac_parse(AVCodecParserContext *s, AVCodecContext *avctx,
const uint8_t **poutbuf, int *poutbuf_size,
const uint8_t *buf, int buf_size)
{
DiracParseContext *pc = s->priv_data;
int next;

*poutbuf = NULL;
*poutbuf_size = 0;

if (s->flags & PARSER_FLAG_COMPLETE_FRAMES) {
next = buf_size;
*poutbuf = buf;
*poutbuf_size = buf_size;

} else {
next = find_frame_end(pc, buf, buf_size);
if (!pc->is_synced && next == -1)

return buf_size;

if (dirac_combine_frame(s, avctx, next, &buf, &buf_size) < 0)
return buf_size;
}

*poutbuf = buf;
*poutbuf_size = buf_size;
return next;
}

static void dirac_parse_close(AVCodecParserContext *s)
{
DiracParseContext *pc = s->priv_data;

if (pc->buffer_size > 0)
av_freep(&pc->buffer);
}

AVCodecParser ff_dirac_parser = {
.codec_ids = { AV_CODEC_ID_DIRAC },
.priv_data_size = sizeof(DiracParseContext),
.parser_parse = dirac_parse,
.parser_close = dirac_parse_close,
};
