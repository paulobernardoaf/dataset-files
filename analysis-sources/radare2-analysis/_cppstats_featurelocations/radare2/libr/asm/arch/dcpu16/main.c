

#include <stdio.h>
#include "dcpu16.h"

int main(int argc, char **argv) {
if (argc>1) {
char out[32];
ut8 buf[64] = {
0x01, 0x7c, 0x30, 0x00, 0xe1, 0x7d, 0x00, 0x10, 0x20, 0x00, 0x03, 
0x78, 0x00, 0x10, 0x0d, 0xc0, 0xc1, 0x7d, 0x1a, 0x00, 0x61, 0xa8, 
0x01, 0x7c, 0x00, 0x20, 0x61, 0x21, 0x00, 0x20, 0x63, 0x84, 0x6d, 
0x80, 0xc1, 0x7d, 0x0d, 0x00, 0x31, 0x90, 0x10, 0x7c, 0x18, 0x00, 
0xc1, 0x7d, 0x1a, 0x00, 0x37, 0x90, 0xc1, 0x61, 0xc1, 0x7d, 0x1a,
0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, };
int len = sizeof (buf)-1;
ut16 *p = (ut16*)buf;

do {
int ret = dcpu16_disasm (out, p, len, NULL);
len -= ret;
p += (ret/2);
printf (" %d %s\n", ret, out);
} while (len>0);
} else {
unsigned char out[32];
char *line = "set a, 0x30";
int i, ret = dcpu16_assemble (out, line);
printf ("len=%d\n", ret);
for (i=0;i<ret; i++)
printf ("%02x", out[i]);
printf ("\n");
}

return 0;
}
