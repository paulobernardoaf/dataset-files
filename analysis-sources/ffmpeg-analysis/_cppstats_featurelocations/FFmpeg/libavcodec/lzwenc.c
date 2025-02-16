


























#include "avcodec.h"
#include "lzw.h"
#include "mathops.h"
#include "put_bits.h"

#define LZW_MAXBITS 12
#define LZW_SIZTABLE (1<<LZW_MAXBITS)
#define LZW_HASH_SIZE 16411
#define LZW_HASH_SHIFT 6

#define LZW_PREFIX_EMPTY -1
#define LZW_PREFIX_FREE -2


typedef struct Code{

int hash_prefix;
int code; 
uint8_t suffix; 
}Code;


typedef struct LZWEncodeState {
int clear_code; 
int end_code; 
Code tab[LZW_HASH_SIZE]; 
int tabsize; 
int bits; 
int bufsize; 
PutBitContext pb; 
int maxbits; 
int maxcode; 
int output_bytes; 
int last_code; 
enum FF_LZW_MODES mode; 
void (*put_bits)(PutBitContext *, int, unsigned); 
}LZWEncodeState;


const int ff_lzw_encode_state_size = sizeof(LZWEncodeState);







static inline int hash(int head, const int add)
{
head ^= (add << LZW_HASH_SHIFT);
if (head >= LZW_HASH_SIZE)
head -= LZW_HASH_SIZE;
av_assert2(head >= 0 && head < LZW_HASH_SIZE);
return head;
}







static inline int hashNext(int head, const int offset)
{
head -= offset;
if(head < 0)
head += LZW_HASH_SIZE;
return head;
}






static inline int hashOffset(const int head)
{
return head ? LZW_HASH_SIZE - head : 1;
}






static inline void writeCode(LZWEncodeState * s, int c)
{
av_assert2(0 <= c && c < 1 << s->bits);
s->put_bits(&s->pb, s->bits, c);
}









static inline int findCode(LZWEncodeState * s, uint8_t c, int hash_prefix)
{
int h = hash(FFMAX(hash_prefix, 0), c);
int hash_offset = hashOffset(h);

while (s->tab[h].hash_prefix != LZW_PREFIX_FREE) {
if ((s->tab[h].suffix == c)
&& (s->tab[h].hash_prefix == hash_prefix))
return h;
h = hashNext(h, hash_offset);
}

return h;
}








static inline void addCode(LZWEncodeState * s, uint8_t c, int hash_prefix, int hash_code)
{
s->tab[hash_code].code = s->tabsize;
s->tab[hash_code].suffix = c;
s->tab[hash_code].hash_prefix = hash_prefix;

s->tabsize++;

if (s->tabsize >= (1 << s->bits) + (s->mode == FF_LZW_GIF))
s->bits++;
}





static void clearTable(LZWEncodeState * s)
{
int i, h;

writeCode(s, s->clear_code);
s->bits = 9;
for (i = 0; i < LZW_HASH_SIZE; i++) {
s->tab[i].hash_prefix = LZW_PREFIX_FREE;
}
for (i = 0; i < 256; i++) {
h = hash(0, i);
s->tab[h].code = i;
s->tab[h].suffix = i;
s->tab[h].hash_prefix = LZW_PREFIX_EMPTY;
}
s->tabsize = 258;
}






static int writtenBytes(LZWEncodeState *s){
int ret = put_bits_count(&s->pb) >> 3;
ret -= s->output_bytes;
s->output_bytes += ret;
return ret;
}








void ff_lzw_encode_init(LZWEncodeState *s, uint8_t *outbuf, int outsize,
int maxbits, enum FF_LZW_MODES mode,
void (*lzw_put_bits)(PutBitContext *, int, unsigned))
{
s->clear_code = 256;
s->end_code = 257;
s->maxbits = maxbits;
init_put_bits(&s->pb, outbuf, outsize);
s->bufsize = outsize;
av_assert0(s->maxbits >= 9 && s->maxbits <= LZW_MAXBITS);
s->maxcode = 1 << s->maxbits;
s->output_bytes = 0;
s->last_code = LZW_PREFIX_EMPTY;
s->bits = 9;
s->mode = mode;
s->put_bits = lzw_put_bits;
}








int ff_lzw_encode(LZWEncodeState * s, const uint8_t * inbuf, int insize)
{
int i;

if(insize * 3 > (s->bufsize - s->output_bytes) * 2){
return -1;
}

if (s->last_code == LZW_PREFIX_EMPTY)
clearTable(s);

for (i = 0; i < insize; i++) {
uint8_t c = *inbuf++;
int code = findCode(s, c, s->last_code);
if (s->tab[code].hash_prefix == LZW_PREFIX_FREE) {
writeCode(s, s->last_code);
addCode(s, c, s->last_code, code);
code= hash(0, c);
}
s->last_code = s->tab[code].code;
if (s->tabsize >= s->maxcode - 1) {
clearTable(s);
}
}

return writtenBytes(s);
}






int ff_lzw_encode_flush(LZWEncodeState *s,
void (*lzw_flush_put_bits)(PutBitContext *))
{
if (s->last_code != -1)
writeCode(s, s->last_code);
writeCode(s, s->end_code);
if (s->mode == FF_LZW_GIF)
s->put_bits(&s->pb, 1, 0);

lzw_flush_put_bits(&s->pb);
s->last_code = -1;

return writtenBytes(s);
}
