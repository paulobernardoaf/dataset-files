






















#include "srt.h"
#include "utf8.h"
#include "vtt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

srt_t* srt_new()
{
return vtt_new();
}

void srt_free(srt_t* srt)
{
vtt_free(srt);
}

vtt_t* srt_parse(const utf8_char_t* data, size_t size)
{
return _vtt_parse(data, size, 1);
}

void srt_dump(srt_t* srt)
{
int i;
vtt_block_t* block;

for (block = srt->cue_head, i = 1; block; block = block->next, ++i) {
int hh1, hh2, mm1, mm2, ss1, ss2, ms1, ms2;
vtt_crack_time(block->timestamp, &hh1, &mm1, &ss1, &ms1);
vtt_crack_time(block->timestamp + block->duration, &hh2, &mm2, &ss2, &ms2);

printf("%02d\r\n%d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\r\n%s\r\n", i,
hh1, mm1, ss1, ms1, hh2, mm2, ss2, ms2, vtt_block_data(block));
}
}
