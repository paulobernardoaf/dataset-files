#undef NDEBUG
#include <assert.h>
#define NO_VIM_MAIN
#include "main.c"
#include "charset.c"
static void
test_isword_funcs_utf8(void)
{
buf_T buf;
int c;
vim_memset(&buf, 0, sizeof(buf));
p_enc = (char_u *)"utf-8";
p_isi = (char_u *)"";
p_isp = (char_u *)"";
p_isf = (char_u *)"";
buf.b_p_isk = (char_u *)"@,48-57,_,128-167,224-235";
curbuf = &buf;
mb_init(); 
for (c = 0; c < 0x10000; ++c)
{
char_u p[4] = {0};
int c1;
int retc;
int retp;
utf_char2bytes(c, p);
c1 = utf_ptr2char(p);
if (c != c1)
{
fprintf(stderr, "Failed: ");
fprintf(stderr,
"[c = %#04x, p = {%#02x, %#02x, %#02x}] ",
c, p[0], p[1], p[2]);
fprintf(stderr, "c != utf_ptr2char(p) (=%#04x)\n", c1);
abort();
}
retc = vim_iswordc_buf(c, &buf);
retp = vim_iswordp_buf(p, &buf);
if (retc != retp)
{
fprintf(stderr, "Failed: ");
fprintf(stderr,
"[c = %#04x, p = {%#02x, %#02x, %#02x}] ",
c, p[0], p[1], p[2]);
fprintf(stderr, "vim_iswordc(c) (=%d) != vim_iswordp(p) (=%d)\n",
retc, retp);
abort();
}
}
}
int
main(void)
{
estack_init();
test_isword_funcs_utf8();
return 0;
}
