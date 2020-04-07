#include "libavutil/attributes.h"
#include "libavutil/crc.h"
#include "libavutil/fifo.h"
#include "bytestream.h"
#include "parser.h"
#include "flac.h"
#define FLAC_MAX_SEQUENTIAL_HEADERS 4
#define FLAC_MIN_HEADERS 10
#define FLAC_AVG_FRAME_SIZE 8192
#define FLAC_HEADER_BASE_SCORE 10
#define FLAC_HEADER_CHANGED_PENALTY 7
#define FLAC_HEADER_CRC_FAIL_PENALTY 50
#define FLAC_HEADER_NOT_PENALIZED_YET 100000
#define FLAC_HEADER_NOT_SCORED_YET -100000
#define MAX_FRAME_HEADER_SIZE 16
typedef struct FLACHeaderMarker {
int offset; 
int link_penalty[FLAC_MAX_SEQUENTIAL_HEADERS]; 
int max_score; 
FLACFrameInfo fi; 
struct FLACHeaderMarker *next; 
struct FLACHeaderMarker *best_child; 
} FLACHeaderMarker;
typedef struct FLACParseContext {
AVCodecParserContext *pc; 
AVCodecContext *avctx; 
FLACHeaderMarker *headers; 
FLACHeaderMarker *best_header; 
int nb_headers_found; 
int nb_headers_buffered; 
int best_header_valid; 
AVFifoBuffer *fifo_buf; 
int end_padded; 
uint8_t *wrap_buf; 
int wrap_buf_allocated_size; 
FLACFrameInfo last_fi; 
int last_fi_valid; 
} FLACParseContext;
static int frame_header_is_valid(AVCodecContext *avctx, const uint8_t *buf,
FLACFrameInfo *fi)
{
GetBitContext gb;
init_get_bits(&gb, buf, MAX_FRAME_HEADER_SIZE * 8);
return !ff_flac_decode_frame_header(avctx, &gb, fi, 127);
}
static uint8_t *flac_fifo_read_wrap(FLACParseContext *fpc, int offset, int len,
uint8_t **wrap_buf, int *allocated_size)
{
AVFifoBuffer *f = fpc->fifo_buf;
uint8_t *start = f->rptr + offset;
uint8_t *tmp_buf;
if (start >= f->end)
start -= f->end - f->buffer;
if (f->end - start >= len)
return start;
tmp_buf = av_fast_realloc(*wrap_buf, allocated_size, len);
if (!tmp_buf) {
av_log(fpc->avctx, AV_LOG_ERROR,
"couldn't reallocate wrap buffer of size %d", len);
return NULL;
}
*wrap_buf = tmp_buf;
do {
int seg_len = FFMIN(f->end - start, len);
memcpy(tmp_buf, start, seg_len);
tmp_buf = (uint8_t*)tmp_buf + seg_len;
start += seg_len - (f->end - f->buffer);
len -= seg_len;
} while (len > 0);
return *wrap_buf;
}
static uint8_t *flac_fifo_read(FLACParseContext *fpc, int offset, int *len)
{
AVFifoBuffer *f = fpc->fifo_buf;
uint8_t *start = f->rptr + offset;
if (start >= f->end)
start -= f->end - f->buffer;
*len = FFMIN(*len, f->end - start);
return start;
}
static int find_headers_search_validate(FLACParseContext *fpc, int offset)
{
FLACFrameInfo fi;
uint8_t *header_buf;
int size = 0;
header_buf = flac_fifo_read_wrap(fpc, offset,
MAX_FRAME_HEADER_SIZE,
&fpc->wrap_buf,
&fpc->wrap_buf_allocated_size);
if (frame_header_is_valid(fpc->avctx, header_buf, &fi)) {
FLACHeaderMarker **end_handle = &fpc->headers;
int i;
size = 0;
while (*end_handle) {
end_handle = &(*end_handle)->next;
size++;
}
*end_handle = av_mallocz(sizeof(**end_handle));
if (!*end_handle) {
av_log(fpc->avctx, AV_LOG_ERROR,
"couldn't allocate FLACHeaderMarker\n");
return AVERROR(ENOMEM);
}
(*end_handle)->fi = fi;
(*end_handle)->offset = offset;
for (i = 0; i < FLAC_MAX_SEQUENTIAL_HEADERS; i++)
(*end_handle)->link_penalty[i] = FLAC_HEADER_NOT_PENALIZED_YET;
fpc->nb_headers_found++;
size++;
}
return size;
}
static int find_headers_search(FLACParseContext *fpc, uint8_t *buf,
int buf_size, int search_start)
{
int size = 0, mod_offset = (buf_size - 1) % 4, i, j;
uint32_t x;
for (i = 0; i < mod_offset; i++) {
if ((AV_RB16(buf + i) & 0xFFFE) == 0xFFF8)
size = find_headers_search_validate(fpc, search_start + i);
}
for (; i < buf_size - 1; i += 4) {
x = AV_RN32(buf + i);
if (((x & ~(x + 0x01010101)) & 0x80808080)) {
for (j = 0; j < 4; j++) {
if ((AV_RB16(buf + i + j) & 0xFFFE) == 0xFFF8)
size = find_headers_search_validate(fpc, search_start + i + j);
}
}
}
return size;
}
static int find_new_headers(FLACParseContext *fpc, int search_start)
{
FLACHeaderMarker *end;
int search_end, size = 0, read_len, temp;
uint8_t *buf;
fpc->nb_headers_found = 0;
search_end = av_fifo_size(fpc->fifo_buf) - (MAX_FRAME_HEADER_SIZE - 1);
read_len = search_end - search_start + 1;
buf = flac_fifo_read(fpc, search_start, &read_len);
size = find_headers_search(fpc, buf, read_len, search_start);
search_start += read_len - 1;
if (search_start != search_end) {
uint8_t wrap[2];
wrap[0] = buf[read_len - 1];
read_len = search_end - (search_start + 1) + 1;
buf = flac_fifo_read(fpc, search_start + 1, &read_len);
wrap[1] = buf[0];
if ((AV_RB16(wrap) & 0xFFFE) == 0xFFF8) {
temp = find_headers_search_validate(fpc, search_start);
size = FFMAX(size, temp);
}
search_start++;
temp = find_headers_search(fpc, buf, read_len, search_start);
size = FFMAX(size, temp);
search_start += read_len - 1;
}
if (!size && fpc->headers)
for (end = fpc->headers; end; end = end->next)
size++;
return size;
}
static int check_header_fi_mismatch(FLACParseContext *fpc,
FLACFrameInfo *header_fi,
FLACFrameInfo *child_fi,
int log_level_offset)
{
int deduction = 0;
if (child_fi->samplerate != header_fi->samplerate) {
deduction += FLAC_HEADER_CHANGED_PENALTY;
av_log(fpc->avctx, AV_LOG_WARNING + log_level_offset,
"sample rate change detected in adjacent frames\n");
}
if (child_fi->bps != header_fi->bps) {
deduction += FLAC_HEADER_CHANGED_PENALTY;
av_log(fpc->avctx, AV_LOG_WARNING + log_level_offset,
"bits per sample change detected in adjacent frames\n");
}
if (child_fi->is_var_size != header_fi->is_var_size) {
deduction += FLAC_HEADER_BASE_SCORE;
av_log(fpc->avctx, AV_LOG_WARNING + log_level_offset,
"blocking strategy change detected in adjacent frames\n");
}
if (child_fi->channels != header_fi->channels) {
deduction += FLAC_HEADER_CHANGED_PENALTY;
av_log(fpc->avctx, AV_LOG_WARNING + log_level_offset,
"number of channels change detected in adjacent frames\n");
}
return deduction;
}
static int check_header_mismatch(FLACParseContext *fpc,
FLACHeaderMarker *header,
FLACHeaderMarker *child,
int log_level_offset)
{
FLACFrameInfo *header_fi = &header->fi, *child_fi = &child->fi;
int deduction, deduction_expected = 0, i;
deduction = check_header_fi_mismatch(fpc, header_fi, child_fi,
log_level_offset);
if ((child_fi->frame_or_sample_num - header_fi->frame_or_sample_num
!= header_fi->blocksize) &&
(child_fi->frame_or_sample_num
!= header_fi->frame_or_sample_num + 1)) {
FLACHeaderMarker *curr;
int64_t expected_frame_num, expected_sample_num;
expected_frame_num = expected_sample_num = header_fi->frame_or_sample_num;
curr = header;
while (curr != child) {
for (i = 0; i < FLAC_MAX_SEQUENTIAL_HEADERS; i++) {
if (curr->link_penalty[i] < FLAC_HEADER_CRC_FAIL_PENALTY) {
expected_frame_num++;
expected_sample_num += curr->fi.blocksize;
break;
}
}
curr = curr->next;
}
if (expected_frame_num == child_fi->frame_or_sample_num ||
expected_sample_num == child_fi->frame_or_sample_num)
deduction_expected = deduction ? 0 : 1;
deduction += FLAC_HEADER_CHANGED_PENALTY;
av_log(fpc->avctx, AV_LOG_WARNING + log_level_offset,
"sample/frame number mismatch in adjacent frames\n");
}
if (deduction && !deduction_expected) {
FLACHeaderMarker *curr;
int read_len;
uint8_t *buf;
uint32_t crc = 1;
int inverted_test = 0;
curr = header->next;
for (i = 0; i < FLAC_MAX_SEQUENTIAL_HEADERS && curr != child; i++)
curr = curr->next;
if (header->link_penalty[i] < FLAC_HEADER_CRC_FAIL_PENALTY ||
header->link_penalty[i] == FLAC_HEADER_NOT_PENALIZED_YET) {
FLACHeaderMarker *start, *end;
start = header;
end = child;
if (i > 0 &&
header->link_penalty[i - 1] >= FLAC_HEADER_CRC_FAIL_PENALTY) {
while (start->next != child)
start = start->next;
inverted_test = 1;
} else if (i > 0 &&
header->next->link_penalty[i-1] >=
FLAC_HEADER_CRC_FAIL_PENALTY ) {
end = header->next;
inverted_test = 1;
}
read_len = end->offset - start->offset;
buf = flac_fifo_read(fpc, start->offset, &read_len);
crc = av_crc(av_crc_get_table(AV_CRC_16_ANSI), 0, buf, read_len);
read_len = (end->offset - start->offset) - read_len;
if (read_len) {
buf = flac_fifo_read(fpc, end->offset - read_len, &read_len);
crc = av_crc(av_crc_get_table(AV_CRC_16_ANSI), crc, buf, read_len);
}
}
if (!crc ^ !inverted_test) {
deduction += FLAC_HEADER_CRC_FAIL_PENALTY;
av_log(fpc->avctx, AV_LOG_WARNING + log_level_offset,
"crc check failed from offset %i (frame %"PRId64") to %i (frame %"PRId64")\n",
header->offset, header_fi->frame_or_sample_num,
child->offset, child_fi->frame_or_sample_num);
}
}
return deduction;
}
static int score_header(FLACParseContext *fpc, FLACHeaderMarker *header)
{
FLACHeaderMarker *child;
int dist = 0;
int child_score;
int base_score = FLAC_HEADER_BASE_SCORE;
if (header->max_score != FLAC_HEADER_NOT_SCORED_YET)
return header->max_score;
if (fpc->last_fi_valid) {
base_score -= check_header_fi_mismatch(fpc, &fpc->last_fi, &header->fi,
AV_LOG_DEBUG);
}
header->max_score = base_score;
child = header->next;
for (dist = 0; dist < FLAC_MAX_SEQUENTIAL_HEADERS && child; dist++) {
if (header->link_penalty[dist] == FLAC_HEADER_NOT_PENALIZED_YET) {
header->link_penalty[dist] = check_header_mismatch(fpc, header,
child, AV_LOG_DEBUG);
}
child_score = score_header(fpc, child) - header->link_penalty[dist];
if (FLAC_HEADER_BASE_SCORE + child_score > header->max_score) {
header->best_child = child;
header->max_score = base_score + child_score;
}
child = child->next;
}
return header->max_score;
}
static void score_sequences(FLACParseContext *fpc)
{
FLACHeaderMarker *curr;
int best_score = 0;
for (curr = fpc->headers; curr; curr = curr->next)
curr->max_score = FLAC_HEADER_NOT_SCORED_YET;
for (curr = fpc->headers; curr; curr = curr->next) {
if (score_header(fpc, curr) > best_score) {
fpc->best_header = curr;
best_score = curr->max_score;
}
}
}
static int get_best_header(FLACParseContext *fpc, const uint8_t **poutbuf,
int *poutbuf_size)
{
FLACHeaderMarker *header = fpc->best_header;
FLACHeaderMarker *child = header->best_child;
if (!child) {
*poutbuf_size = av_fifo_size(fpc->fifo_buf) - header->offset;
} else {
*poutbuf_size = child->offset - header->offset;
check_header_mismatch(fpc, header, child, 0);
}
if (header->fi.channels != fpc->avctx->channels ||
!fpc->avctx->channel_layout) {
fpc->avctx->channels = header->fi.channels;
ff_flac_set_channel_layout(fpc->avctx);
}
fpc->avctx->sample_rate = header->fi.samplerate;
fpc->pc->duration = header->fi.blocksize;
*poutbuf = flac_fifo_read_wrap(fpc, header->offset, *poutbuf_size,
&fpc->wrap_buf,
&fpc->wrap_buf_allocated_size);
if (fpc->pc->flags & PARSER_FLAG_USE_CODEC_TS) {
if (header->fi.is_var_size)
fpc->pc->pts = header->fi.frame_or_sample_num;
else if (header->best_child)
fpc->pc->pts = header->fi.frame_or_sample_num * header->fi.blocksize;
}
fpc->best_header_valid = 0;
fpc->last_fi_valid = 1;
fpc->last_fi = header->fi;
if (child)
return child->offset - av_fifo_size(fpc->fifo_buf);
return 0;
}
static int flac_parse(AVCodecParserContext *s, AVCodecContext *avctx,
const uint8_t **poutbuf, int *poutbuf_size,
const uint8_t *buf, int buf_size)
{
FLACParseContext *fpc = s->priv_data;
FLACHeaderMarker *curr;
int nb_headers;
const uint8_t *read_end = buf;
const uint8_t *read_start = buf;
if (s->flags & PARSER_FLAG_COMPLETE_FRAMES) {
FLACFrameInfo fi;
if (frame_header_is_valid(avctx, buf, &fi)) {
s->duration = fi.blocksize;
if (!avctx->sample_rate)
avctx->sample_rate = fi.samplerate;
if (fpc->pc->flags & PARSER_FLAG_USE_CODEC_TS) {
fpc->pc->pts = fi.frame_or_sample_num;
if (!fi.is_var_size)
fpc->pc->pts *= fi.blocksize;
}
}
*poutbuf = buf;
*poutbuf_size = buf_size;
return buf_size;
}
fpc->avctx = avctx;
if (fpc->best_header_valid)
return get_best_header(fpc, poutbuf, poutbuf_size);
if (fpc->best_header && fpc->best_header->best_child) {
FLACHeaderMarker *temp;
FLACHeaderMarker *best_child = fpc->best_header->best_child;
for (curr = fpc->headers; curr != best_child; curr = temp) {
if (curr != fpc->best_header) {
av_log(avctx, AV_LOG_DEBUG,
"dropping low score %i frame header from offset %i to %i\n",
curr->max_score, curr->offset, curr->next->offset);
}
temp = curr->next;
av_free(curr);
fpc->nb_headers_buffered--;
}
av_fifo_drain(fpc->fifo_buf, best_child->offset);
for (curr = best_child->next; curr; curr = curr->next)
curr->offset -= best_child->offset;
best_child->offset = 0;
fpc->headers = best_child;
if (fpc->nb_headers_buffered >= FLAC_MIN_HEADERS) {
fpc->best_header = best_child;
return get_best_header(fpc, poutbuf, poutbuf_size);
}
fpc->best_header = NULL;
} else if (fpc->best_header) {
FLACHeaderMarker *temp;
for (curr = fpc->headers; curr != fpc->best_header; curr = temp) {
temp = curr->next;
av_free(curr);
fpc->nb_headers_buffered--;
}
fpc->headers = fpc->best_header->next;
av_freep(&fpc->best_header);
fpc->nb_headers_buffered--;
}
while ((buf_size && read_end < buf + buf_size &&
fpc->nb_headers_buffered < FLAC_MIN_HEADERS)
|| (!buf_size && !fpc->end_padded)) {
int start_offset;
if (!buf_size) {
fpc->end_padded = 1;
read_end = read_start + MAX_FRAME_HEADER_SIZE;
} else {
int nb_desired = FLAC_MIN_HEADERS - fpc->nb_headers_buffered + 1;
read_end = read_end + FFMIN(buf + buf_size - read_end,
nb_desired * FLAC_AVG_FRAME_SIZE);
}
if (!av_fifo_space(fpc->fifo_buf) &&
av_fifo_size(fpc->fifo_buf) / FLAC_AVG_FRAME_SIZE >
fpc->nb_headers_buffered * 20) {
goto handle_error;
}
if ( av_fifo_space(fpc->fifo_buf) < read_end - read_start
&& av_fifo_realloc2(fpc->fifo_buf, (read_end - read_start) + 2*av_fifo_size(fpc->fifo_buf)) < 0) {
av_log(avctx, AV_LOG_ERROR,
"couldn't reallocate buffer of size %"PTRDIFF_SPECIFIER"\n",
(read_end - read_start) + av_fifo_size(fpc->fifo_buf));
goto handle_error;
}
if (buf_size) {
av_fifo_generic_write(fpc->fifo_buf, (void*) read_start,
read_end - read_start, NULL);
} else {
int8_t pad[MAX_FRAME_HEADER_SIZE] = { 0 };
av_fifo_generic_write(fpc->fifo_buf, pad, sizeof(pad), NULL);
}
start_offset = av_fifo_size(fpc->fifo_buf) -
((read_end - read_start) + (MAX_FRAME_HEADER_SIZE - 1));
start_offset = FFMAX(0, start_offset);
nb_headers = find_new_headers(fpc, start_offset);
if (nb_headers < 0) {
av_log(avctx, AV_LOG_ERROR,
"find_new_headers couldn't allocate FLAC header\n");
goto handle_error;
}
fpc->nb_headers_buffered = nb_headers;
if (!fpc->end_padded && fpc->nb_headers_buffered < FLAC_MIN_HEADERS) {
if (read_end < buf + buf_size) {
read_start = read_end;
continue;
} else {
goto handle_error;
}
}
if (fpc->end_padded || fpc->nb_headers_found)
score_sequences(fpc);
if (fpc->end_padded) {
int warp = fpc->fifo_buf->wptr - fpc->fifo_buf->buffer < MAX_FRAME_HEADER_SIZE;
fpc->fifo_buf->wptr -= MAX_FRAME_HEADER_SIZE;
fpc->fifo_buf->wndx -= MAX_FRAME_HEADER_SIZE;
if (warp) {
fpc->fifo_buf->wptr += fpc->fifo_buf->end -
fpc->fifo_buf->buffer;
}
read_start = read_end = NULL;
}
}
for (curr = fpc->headers; curr; curr = curr->next) {
if (!fpc->best_header || curr->max_score > fpc->best_header->max_score) {
fpc->best_header = curr;
}
}
if (fpc->best_header && fpc->best_header->max_score <= 0) {
if (!buf_size || read_end != buf || fpc->nb_headers_buffered < FLAC_MIN_HEADERS)
fpc->best_header = NULL;
}
if (fpc->best_header) {
fpc->best_header_valid = 1;
if (fpc->best_header->offset > 0) {
av_log(avctx, AV_LOG_DEBUG, "Junk frame till offset %i\n",
fpc->best_header->offset);
s->duration = 0;
*poutbuf_size = fpc->best_header->offset;
*poutbuf = flac_fifo_read_wrap(fpc, 0, *poutbuf_size,
&fpc->wrap_buf,
&fpc->wrap_buf_allocated_size);
return buf_size ? (read_end - buf) : (fpc->best_header->offset -
av_fifo_size(fpc->fifo_buf));
}
if (!buf_size)
return get_best_header(fpc, poutbuf, poutbuf_size);
}
handle_error:
*poutbuf = NULL;
*poutbuf_size = 0;
return buf_size ? read_end - buf : 0;
}
static av_cold int flac_parse_init(AVCodecParserContext *c)
{
FLACParseContext *fpc = c->priv_data;
fpc->pc = c;
fpc->fifo_buf = av_fifo_alloc_array(FLAC_MIN_HEADERS + 3, FLAC_AVG_FRAME_SIZE);
if (!fpc->fifo_buf) {
av_log(fpc->avctx, AV_LOG_ERROR,
"couldn't allocate fifo_buf\n");
return AVERROR(ENOMEM);
}
return 0;
}
static void flac_parse_close(AVCodecParserContext *c)
{
FLACParseContext *fpc = c->priv_data;
FLACHeaderMarker *curr = fpc->headers, *temp;
while (curr) {
temp = curr->next;
av_free(curr);
curr = temp;
}
fpc->headers = NULL;
av_fifo_freep(&fpc->fifo_buf);
av_freep(&fpc->wrap_buf);
}
AVCodecParser ff_flac_parser = {
.codec_ids = { AV_CODEC_ID_FLAC },
.priv_data_size = sizeof(FLACParseContext),
.parser_init = flac_parse_init,
.parser_parse = flac_parse,
.parser_close = flac_parse_close,
};
