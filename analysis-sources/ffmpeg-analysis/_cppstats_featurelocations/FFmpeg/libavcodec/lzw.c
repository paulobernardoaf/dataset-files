




























#include "avcodec.h"
#include "bytestream.h"
#include "lzw.h"
#include "libavutil/mem.h"

#define LZW_MAXBITS 12
#define LZW_SIZTABLE (1<<LZW_MAXBITS)

static const uint16_t mask[17] =
{
0x0000, 0x0001, 0x0003, 0x0007,
0x000F, 0x001F, 0x003F, 0x007F,
0x00FF, 0x01FF, 0x03FF, 0x07FF,
0x0FFF, 0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF
};

struct LZWState {
GetByteContext gb;
int bbits;
unsigned int bbuf;

int mode; 
int cursize; 
int curmask;
int codesize;
int clear_code;
int end_code;
int newcodes; 
int top_slot; 
int extra_slot;
int slot; 
int fc, oc;
uint8_t *sp;
uint8_t stack[LZW_SIZTABLE];
uint8_t suffix[LZW_SIZTABLE];
uint16_t prefix[LZW_SIZTABLE];
int bs; 
};


static int lzw_get_code(struct LZWState * s)
{
int c;

if (s->bbits < s->cursize && bytestream2_get_bytes_left(&s->gb) <= 0)
return s->end_code;

if(s->mode == FF_LZW_GIF) {
while (s->bbits < s->cursize) {
if (!s->bs) {
s->bs = bytestream2_get_byte(&s->gb);
}
s->bbuf |= bytestream2_get_byte(&s->gb) << s->bbits;
s->bbits += 8;
s->bs--;
}
c = s->bbuf;
s->bbuf >>= s->cursize;
} else { 
while (s->bbits < s->cursize) {
s->bbuf = (s->bbuf << 8) | bytestream2_get_byte(&s->gb);
s->bbits += 8;
}
c = s->bbuf >> (s->bbits - s->cursize);
}
s->bbits -= s->cursize;
return c & s->curmask;
}

int ff_lzw_decode_tail(LZWState *p)
{
struct LZWState *s = (struct LZWState *)p;

if(s->mode == FF_LZW_GIF) {
while (s->bs > 0 && bytestream2_get_bytes_left(&s->gb)) {
bytestream2_skip(&s->gb, s->bs);
s->bs = bytestream2_get_byte(&s->gb);
}
}else
bytestream2_skip(&s->gb, bytestream2_get_bytes_left(&s->gb));
return bytestream2_tell(&s->gb);
}

av_cold void ff_lzw_decode_open(LZWState **p)
{
*p = av_mallocz(sizeof(struct LZWState));
}

av_cold void ff_lzw_decode_close(LZWState **p)
{
av_freep(p);
}









int ff_lzw_decode_init(LZWState *p, int csize, const uint8_t *buf, int buf_size, int mode)
{
struct LZWState *s = (struct LZWState *)p;

if(csize < 1 || csize >= LZW_MAXBITS)
return -1;

bytestream2_init(&s->gb, buf, buf_size);
s->bbuf = 0;
s->bbits = 0;
s->bs = 0;


s->codesize = csize;
s->cursize = s->codesize + 1;
s->curmask = mask[s->cursize];
s->top_slot = 1 << s->cursize;
s->clear_code = 1 << s->codesize;
s->end_code = s->clear_code + 1;
s->slot = s->newcodes = s->clear_code + 2;
s->oc = s->fc = -1;
s->sp = s->stack;

s->mode = mode;
s->extra_slot = s->mode == FF_LZW_TIFF;
return 0;
}











int ff_lzw_decode(LZWState *p, uint8_t *buf, int len){
int l, c, code, oc, fc;
uint8_t *sp;
struct LZWState *s = (struct LZWState *)p;

if (s->end_code < 0)
return 0;

l = len;
sp = s->sp;
oc = s->oc;
fc = s->fc;

for (;;) {
while (sp > s->stack) {
*buf++ = *(--sp);
if ((--l) == 0)
goto the_end;
}
c = lzw_get_code(s);
if (c == s->end_code) {
break;
} else if (c == s->clear_code) {
s->cursize = s->codesize + 1;
s->curmask = mask[s->cursize];
s->slot = s->newcodes;
s->top_slot = 1 << s->cursize;
fc= oc= -1;
} else {
code = c;
if (code == s->slot && fc>=0) {
*sp++ = fc;
code = oc;
}else if(code >= s->slot)
break;
while (code >= s->newcodes) {
*sp++ = s->suffix[code];
code = s->prefix[code];
}
*sp++ = code;
if (s->slot < s->top_slot && oc>=0) {
s->suffix[s->slot] = code;
s->prefix[s->slot++] = oc;
}
fc = code;
oc = c;
if (s->slot >= s->top_slot - s->extra_slot) {
if (s->cursize < LZW_MAXBITS) {
s->top_slot <<= 1;
s->curmask = mask[++s->cursize];
}
}
}
}
s->end_code = -1;
the_end:
s->sp = sp;
s->oc = oc;
s->fc = fc;
return len - l;
}
