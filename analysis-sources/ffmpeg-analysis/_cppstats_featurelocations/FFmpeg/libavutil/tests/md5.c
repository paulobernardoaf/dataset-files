

















#include <stdint.h>
#include <stdio.h>

#include "libavutil/md5.h"

static void print_md5(uint8_t *md5)
{
int i;
for (i = 0; i < 16; i++)
printf("%02x", md5[i]);
printf("\n");
}

int main(void)
{
uint8_t md5val[16];
int i;
volatile uint8_t in[1000]; 


for (i = 0; i < 1000; i++)
in[i] = i * i;
av_md5_sum(md5val, in, 1000);
print_md5(md5val);
av_md5_sum(md5val, in, 63);
print_md5(md5val);
av_md5_sum(md5val, in, 64);
print_md5(md5val);
av_md5_sum(md5val, in, 65);
print_md5(md5val);
for (i = 0; i < 1000; i++)
in[i] = i % 127;
av_md5_sum(md5val, in, 999);
print_md5(md5val);

return 0;
}
