

















#include <stdint.h>
#include <stdio.h>

#include "libavutil/crc.h"

int main(void)
{
uint8_t buf[1999];
int i;
static const unsigned p[7][3] = {
{ AV_CRC_32_IEEE_LE, 0xEDB88320, 0x3D5CDD04 },
{ AV_CRC_32_IEEE , 0x04C11DB7, 0xC0F5BAE0 },
{ AV_CRC_24_IEEE , 0x864CFB , 0xB704CE },
{ AV_CRC_16_ANSI_LE, 0xA001 , 0xBFD8 },
{ AV_CRC_16_ANSI , 0x8005 , 0x1FBB },
{ AV_CRC_8_ATM , 0x07 , 0xE3 },
{ AV_CRC_8_EBU , 0x1D , 0xD6 },
};
const AVCRC *ctx;

for (i = 0; i < sizeof(buf); i++)
buf[i] = i + i * i;

for (i = 0; i < 7; i++) {
ctx = av_crc_get_table(p[i][0]);
printf("crc %08X = %X\n", p[i][1], av_crc(ctx, 0, buf, sizeof(buf)));
}
return 0;
}
