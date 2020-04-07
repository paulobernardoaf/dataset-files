#include <stdio.h>
int main(void)
{
static const char b64[] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
unsigned i_bits = 0;
int i_shift = 0;
int out_len = 0;
int in;
#define putb64() do { putchar(b64[(i_bits << 6 >> i_shift) & 0x3f]); out_len++; i_shift -= 6; } while (0)
while ((in = getchar()) != EOF) {
i_bits = (i_bits << 8) + in;
i_shift += 8;
while (i_shift > 6)
putb64();
}
while (i_shift > 0)
putb64();
while (out_len++ & 3)
putchar('=');
putchar('\n');
return 0;
}
