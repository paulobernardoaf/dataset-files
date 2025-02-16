




















#include "libavutil/avassert.h"
#include "libavutil/imgutils.h"

#include "parser.h" 
#include "pnm.h"

typedef struct PNMParseContext {
ParseContext pc;
int remaining_bytes;
int ascii_scan;
}PNMParseContext;

static int pnm_parse(AVCodecParserContext *s, AVCodecContext *avctx,
const uint8_t **poutbuf, int *poutbuf_size,
const uint8_t *buf, int buf_size)
{
PNMParseContext *pnmpc = s->priv_data;
ParseContext *pc = &pnmpc->pc;
PNMContext pnmctx;
int next = END_NOT_FOUND;
int skip = 0;

if (pc->overread > 0) {
memmove(pc->buffer + pc->index, pc->buffer + pc->overread_index, pc->overread);
pc->index += pc->overread;
pc->overread_index += pc->overread;
pc->overread = 0;
}

if (pnmpc->remaining_bytes) {
int inc = FFMIN(pnmpc->remaining_bytes, buf_size);
skip += inc;
pnmpc->remaining_bytes -= inc;

if (!pnmpc->remaining_bytes)
next = skip;
goto end;
}

retry:
if (pc->index) {
pnmctx.bytestream_start =
pnmctx.bytestream = pc->buffer;
pnmctx.bytestream_end = pc->buffer + pc->index;
} else {
pnmctx.bytestream_start =
pnmctx.bytestream = (uint8_t *) buf + skip; 
pnmctx.bytestream_end = (uint8_t *) buf + buf_size - skip;
}
if (ff_pnm_decode_header(avctx, &pnmctx) < 0) {
if (pnmctx.bytestream < pnmctx.bytestream_end) {
if (pc->index) {
pc->index = 0;
pnmpc->ascii_scan = 0;
} else {
unsigned step = FFMAX(1, pnmctx.bytestream - pnmctx.bytestream_start);

skip += step;
}
goto retry;
}
} else if (pnmctx.type < 4) {
uint8_t *bs = pnmctx.bytestream;
const uint8_t *end = pnmctx.bytestream_end;
uint8_t *sync = bs;

if (pc->index) {
av_assert0(pnmpc->ascii_scan <= end - bs);
bs += pnmpc->ascii_scan;
}

while (bs < end) {
int c;
sync = bs;
c = *bs++;
if (c == '#') {
uint8_t *match = memchr(bs, '\n', end-bs);
if (match)
bs = match + 1;
else
break;
} else if (c == 'P') {
next = bs - pnmctx.bytestream_start + skip - 1;
pnmpc->ascii_scan = 0;
break;
}
}
if (next == END_NOT_FOUND)
pnmpc->ascii_scan = sync - pnmctx.bytestream + skip;
} else {
next = pnmctx.bytestream - pnmctx.bytestream_start + skip
+ av_image_get_buffer_size(avctx->pix_fmt, avctx->width, avctx->height, 1);
}
if (next != END_NOT_FOUND && pnmctx.bytestream_start != buf + skip)
next -= pc->index;
if (next > buf_size) {
pnmpc->remaining_bytes = next - buf_size;
next = END_NOT_FOUND;
}
end:
if (ff_combine_frame(pc, next, &buf, &buf_size) < 0) {
*poutbuf = NULL;
*poutbuf_size = 0;
return buf_size;
}
*poutbuf = buf;
*poutbuf_size = buf_size;
return next;
}

AVCodecParser ff_pnm_parser = {
.codec_ids = { AV_CODEC_ID_PGM, AV_CODEC_ID_PGMYUV, AV_CODEC_ID_PPM,
AV_CODEC_ID_PBM, AV_CODEC_ID_PAM },
.priv_data_size = sizeof(PNMParseContext),
.parser_parse = pnm_parse,
.parser_close = ff_parse_close,
};
