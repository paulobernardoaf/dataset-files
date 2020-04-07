#include "libavutil/common.h"
#include "libavutil/frame.h"
#include "libavutil/lfg.h"
#include "libavutil/xga_font_data.h"
#include "avcodec.h"
#include "cga_data.h"
#include "internal.h"
#define ATTR_BOLD 0x01 
#define ATTR_FAINT 0x02 
#define ATTR_ITALICS 0x04 
#define ATTR_UNDERLINE 0x08 
#define ATTR_BLINK 0x10 
#define ATTR_REVERSE 0x40 
#define ATTR_CONCEALED 0x80 
#define DEFAULT_FG_COLOR 7 
#define DEFAULT_BG_COLOR 0
#define DEFAULT_SCREEN_MODE 3 
#define FONT_WIDTH 8 
static const uint8_t ansi_to_cga[16] = {
0, 4, 2, 6, 1, 5, 3, 7, 8, 12, 10, 14, 9, 13, 11, 15
};
typedef struct AnsiContext {
AVFrame *frame;
int x; 
int y; 
int sx; 
int sy; 
const uint8_t* font; 
int font_height; 
int attributes; 
int fg; 
int bg; 
int first_frame;
enum {
STATE_NORMAL = 0,
STATE_ESCAPE,
STATE_CODE,
STATE_MUSIC_PREAMBLE
} state;
#define MAX_NB_ARGS 4
int args[MAX_NB_ARGS];
int nb_args; 
} AnsiContext;
static av_cold int decode_init(AVCodecContext *avctx)
{
AnsiContext *s = avctx->priv_data;
avctx->pix_fmt = AV_PIX_FMT_PAL8;
s->font = avpriv_vga16_font;
s->font_height = 16;
s->fg = DEFAULT_FG_COLOR;
s->bg = DEFAULT_BG_COLOR;
if (!avctx->width || !avctx->height) {
int ret = ff_set_dimensions(avctx, 80 << 3, 25 << 4);
if (ret < 0)
return ret;
} else if (avctx->width % FONT_WIDTH || avctx->height % s->font_height) {
av_log(avctx, AV_LOG_ERROR, "Invalid dimensions %d %d\n", avctx->width, avctx->height);
return AVERROR(EINVAL);
}
s->frame = av_frame_alloc();
if (!s->frame)
return AVERROR(ENOMEM);
return 0;
}
static void set_palette(uint32_t *pal)
{
int r, g, b;
memcpy(pal, ff_cga_palette, 16 * 4);
pal += 16;
#define COLOR(x) ((x) * 40 + 55)
for (r = 0; r < 6; r++)
for (g = 0; g < 6; g++)
for (b = 0; b < 6; b++)
*pal++ = 0xFF000000 | (COLOR(r) << 16) | (COLOR(g) << 8) | COLOR(b);
#define GRAY(x) ((x) * 10 + 8)
for (g = 0; g < 24; g++)
*pal++ = 0xFF000000 | (GRAY(g) << 16) | (GRAY(g) << 8) | GRAY(g);
}
static void hscroll(AVCodecContext *avctx)
{
AnsiContext *s = avctx->priv_data;
int i;
if (s->y <= avctx->height - 2*s->font_height) {
s->y += s->font_height;
return;
}
i = 0;
for (; i < avctx->height - s->font_height; i++)
memcpy(s->frame->data[0] + i * s->frame->linesize[0],
s->frame->data[0] + (i + s->font_height) * s->frame->linesize[0],
avctx->width);
for (; i < avctx->height; i++)
memset(s->frame->data[0] + i * s->frame->linesize[0],
DEFAULT_BG_COLOR, avctx->width);
}
static void erase_line(AVCodecContext * avctx, int xoffset, int xlength)
{
AnsiContext *s = avctx->priv_data;
int i;
for (i = 0; i < s->font_height; i++)
memset(s->frame->data[0] + (s->y + i)*s->frame->linesize[0] + xoffset,
DEFAULT_BG_COLOR, xlength);
}
static void erase_screen(AVCodecContext *avctx)
{
AnsiContext *s = avctx->priv_data;
int i;
for (i = 0; i < avctx->height; i++)
memset(s->frame->data[0] + i * s->frame->linesize[0], DEFAULT_BG_COLOR, avctx->width);
s->x = s->y = 0;
}
static void draw_char(AVCodecContext *avctx, int c)
{
AnsiContext *s = avctx->priv_data;
int fg = s->fg;
int bg = s->bg;
if ((s->attributes & ATTR_BOLD))
fg += 8;
if ((s->attributes & ATTR_BLINK))
bg += 8;
if ((s->attributes & ATTR_REVERSE))
FFSWAP(int, fg, bg);
if ((s->attributes & ATTR_CONCEALED))
fg = bg;
ff_draw_pc_font(s->frame->data[0] + s->y * s->frame->linesize[0] + s->x,
s->frame->linesize[0], s->font, s->font_height, c, fg, bg);
s->x += FONT_WIDTH;
if (s->x > avctx->width - FONT_WIDTH) {
s->x = 0;
hscroll(avctx);
}
}
static int execute_code(AVCodecContext * avctx, int c)
{
AnsiContext *s = avctx->priv_data;
int ret, i;
int width = avctx->width;
int height = avctx->height;
switch(c) {
case 'A': 
s->y = FFMAX(s->y - (s->nb_args > 0 ? s->args[0]*s->font_height : s->font_height), 0);
break;
case 'B': 
s->y = FFMIN(s->y + (s->nb_args > 0 ? s->args[0]*s->font_height : s->font_height), avctx->height - s->font_height);
break;
case 'C': 
s->x = FFMIN(s->x + (s->nb_args > 0 ? s->args[0]*FONT_WIDTH : FONT_WIDTH), avctx->width - FONT_WIDTH);
break;
case 'D': 
s->x = FFMAX(s->x - (s->nb_args > 0 ? s->args[0]*FONT_WIDTH : FONT_WIDTH), 0);
break;
case 'H': 
case 'f': 
s->y = s->nb_args > 0 ? av_clip((s->args[0] - 1)*s->font_height, 0, avctx->height - s->font_height) : 0;
s->x = s->nb_args > 1 ? av_clip((s->args[1] - 1)*FONT_WIDTH, 0, avctx->width - FONT_WIDTH) : 0;
break;
case 'h': 
case 'l': 
if (s->nb_args < 2)
s->args[0] = DEFAULT_SCREEN_MODE;
switch(s->args[0]) {
case 0: case 1: case 4: case 5: case 13: case 19: 
s->font = avpriv_cga_font;
s->font_height = 8;
width = 40<<3;
height = 25<<3;
break;
case 2: case 3: 
s->font = avpriv_vga16_font;
s->font_height = 16;
width = 80<<3;
height = 25<<4;
break;
case 6: case 14: 
s->font = avpriv_cga_font;
s->font_height = 8;
width = 80<<3;
height = 25<<3;
break;
case 7: 
break;
case 15: case 16: 
s->font = avpriv_cga_font;
s->font_height = 8;
width = 80<<3;
height = 43<<3;
break;
case 17: case 18: 
s->font = avpriv_cga_font;
s->font_height = 8;
width = 80<<3;
height = 60<<4;
break;
default:
avpriv_request_sample(avctx, "Unsupported screen mode");
}
s->x = av_clip(s->x, 0, width - FONT_WIDTH);
s->y = av_clip(s->y, 0, height - s->font_height);
if (width != avctx->width || height != avctx->height) {
av_frame_unref(s->frame);
ret = ff_set_dimensions(avctx, width, height);
if (ret < 0)
return ret;
if ((ret = ff_get_buffer(avctx, s->frame,
AV_GET_BUFFER_FLAG_REF)) < 0)
return ret;
s->frame->pict_type = AV_PICTURE_TYPE_I;
s->frame->palette_has_changed = 1;
set_palette((uint32_t *)s->frame->data[1]);
erase_screen(avctx);
} else if (c == 'l') {
erase_screen(avctx);
}
break;
case 'J': 
switch (s->args[0]) {
case 0:
erase_line(avctx, s->x, avctx->width - s->x);
if (s->y < avctx->height - s->font_height)
memset(s->frame->data[0] + (s->y + s->font_height)*s->frame->linesize[0],
DEFAULT_BG_COLOR, (avctx->height - s->y - s->font_height)*s->frame->linesize[0]);
break;
case 1:
erase_line(avctx, 0, s->x);
if (s->y > 0)
memset(s->frame->data[0], DEFAULT_BG_COLOR, s->y * s->frame->linesize[0]);
break;
case 2:
erase_screen(avctx);
}
break;
case 'K': 
switch(s->args[0]) {
case 0:
erase_line(avctx, s->x, avctx->width - s->x);
break;
case 1:
erase_line(avctx, 0, s->x);
break;
case 2:
erase_line(avctx, 0, avctx->width);
}
break;
case 'm': 
if (s->nb_args == 0) {
s->nb_args = 1;
s->args[0] = 0;
}
for (i = 0; i < FFMIN(s->nb_args, MAX_NB_ARGS); i++) {
int m = s->args[i];
if (m == 0) {
s->attributes = 0;
s->fg = DEFAULT_FG_COLOR;
s->bg = DEFAULT_BG_COLOR;
} else if (m == 1 || m == 2 || m == 3 || m == 4 || m == 5 || m == 7 || m == 8) {
s->attributes |= 1 << (m - 1);
} else if (m >= 30 && m <= 37) {
s->fg = ansi_to_cga[m - 30];
} else if (m == 38 && i + 2 < FFMIN(s->nb_args, MAX_NB_ARGS) && s->args[i + 1] == 5 && s->args[i + 2] < 256) {
int index = s->args[i + 2];
s->fg = index < 16 ? ansi_to_cga[index] : index;
i += 2;
} else if (m == 39) {
s->fg = ansi_to_cga[DEFAULT_FG_COLOR];
} else if (m >= 40 && m <= 47) {
s->bg = ansi_to_cga[m - 40];
} else if (m == 48 && i + 2 < FFMIN(s->nb_args, MAX_NB_ARGS) && s->args[i + 1] == 5 && s->args[i + 2] < 256) {
int index = s->args[i + 2];
s->bg = index < 16 ? ansi_to_cga[index] : index;
i += 2;
} else if (m == 49) {
s->fg = ansi_to_cga[DEFAULT_BG_COLOR];
} else {
avpriv_request_sample(avctx, "Unsupported rendition parameter");
}
}
break;
case 'n': 
case 'R': 
break;
case 's': 
s->sx = s->x;
s->sy = s->y;
break;
case 'u': 
s->x = av_clip(s->sx, 0, avctx->width - FONT_WIDTH);
s->y = av_clip(s->sy, 0, avctx->height - s->font_height);
break;
default:
avpriv_request_sample(avctx, "Unknown escape code");
break;
}
s->x = av_clip(s->x, 0, avctx->width - FONT_WIDTH);
s->y = av_clip(s->y, 0, avctx->height - s->font_height);
return 0;
}
static int decode_frame(AVCodecContext *avctx,
void *data, int *got_frame,
AVPacket *avpkt)
{
AnsiContext *s = avctx->priv_data;
uint8_t *buf = avpkt->data;
int buf_size = avpkt->size;
const uint8_t *buf_end = buf+buf_size;
int ret, i, count;
if ((ret = ff_reget_buffer(avctx, s->frame, 0)) < 0)
return ret;
if (!avctx->frame_number) {
for (i=0; i<avctx->height; i++)
memset(s->frame->data[0]+ i*s->frame->linesize[0], 0, avctx->width);
memset(s->frame->data[1], 0, AVPALETTE_SIZE);
}
s->frame->pict_type = AV_PICTURE_TYPE_I;
s->frame->palette_has_changed = 1;
set_palette((uint32_t *)s->frame->data[1]);
if (!s->first_frame) {
erase_screen(avctx);
s->first_frame = 1;
}
while(buf < buf_end) {
switch(s->state) {
case STATE_NORMAL:
switch (buf[0]) {
case 0x00: 
case 0x07: 
case 0x1A: 
break;
case 0x08: 
s->x = FFMAX(s->x - 1, 0);
break;
case 0x09: 
i = s->x / FONT_WIDTH;
count = ((i + 8) & ~7) - i;
for (i = 0; i < count; i++)
draw_char(avctx, ' ');
break;
case 0x0A: 
hscroll(avctx);
case 0x0D: 
s->x = 0;
break;
case 0x0C: 
erase_screen(avctx);
break;
case 0x1B: 
s->state = STATE_ESCAPE;
break;
default:
draw_char(avctx, buf[0]);
}
break;
case STATE_ESCAPE:
if (buf[0] == '[') {
s->state = STATE_CODE;
s->nb_args = 0;
s->args[0] = -1;
} else {
s->state = STATE_NORMAL;
draw_char(avctx, 0x1B);
continue;
}
break;
case STATE_CODE:
switch(buf[0]) {
case '0': case '1': case '2': case '3': case '4':
case '5': case '6': case '7': case '8': case '9':
if (s->nb_args < MAX_NB_ARGS && s->args[s->nb_args] < 6553)
s->args[s->nb_args] = FFMAX(s->args[s->nb_args], 0) * 10 + buf[0] - '0';
break;
case ';':
s->nb_args++;
if (s->nb_args < MAX_NB_ARGS)
s->args[s->nb_args] = 0;
break;
case 'M':
s->state = STATE_MUSIC_PREAMBLE;
break;
case '=': case '?':
break;
default:
if (s->nb_args > MAX_NB_ARGS)
av_log(avctx, AV_LOG_WARNING, "args overflow (%i)\n", s->nb_args);
if (s->nb_args < MAX_NB_ARGS && s->args[s->nb_args] >= 0)
s->nb_args++;
if ((ret = execute_code(avctx, buf[0])) < 0)
return ret;
s->state = STATE_NORMAL;
}
break;
case STATE_MUSIC_PREAMBLE:
if (buf[0] == 0x0E || buf[0] == 0x1B)
s->state = STATE_NORMAL;
break;
}
buf++;
}
*got_frame = 1;
if ((ret = av_frame_ref(data, s->frame)) < 0)
return ret;
return buf_size;
}
static av_cold int decode_close(AVCodecContext *avctx)
{
AnsiContext *s = avctx->priv_data;
av_frame_free(&s->frame);
return 0;
}
AVCodec ff_ansi_decoder = {
.name = "ansi",
.long_name = NULL_IF_CONFIG_SMALL("ASCII/ANSI art"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_ANSI,
.priv_data_size = sizeof(AnsiContext),
.init = decode_init,
.close = decode_close,
.decode = decode_frame,
.capabilities = AV_CODEC_CAP_DR1,
.caps_internal = FF_CODEC_CAP_INIT_THREADSAFE,
};
