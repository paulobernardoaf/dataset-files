












#undef NDEBUG
#include <assert.h>


#define NO_VIM_MAIN
#include "main.c"



#include "message.c"

#if !defined(MIN)
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif











char *fmt_012p = "%012p";
char *fmt_5S = "%5S";
char *fmt_06b = "%06b";




static void
test_trunc_string(void)
{
char_u *buf; 
char_u *s;


buf = alloc(40);
STRCPY(buf, "text");
trunc_string(buf, buf, 20, 40);
assert(STRCMP(buf, "text") == 0);
vim_free(buf);

buf = alloc(40);
STRCPY(buf, "a short text");
trunc_string(buf, buf, 20, 40);
assert(STRCMP(buf, "a short text") == 0);
vim_free(buf);

buf = alloc(40);
STRCPY(buf, "a text tha just fits");
trunc_string(buf, buf, 20, 40);
assert(STRCMP(buf, "a text tha just fits") == 0);
vim_free(buf);

buf = alloc(40);
STRCPY(buf, "a text that nott fits");
trunc_string(buf, buf, 20, 40);
assert(STRCMP(buf, "a text t...nott fits") == 0);
vim_free(buf);


buf = alloc(40);
s = vim_strsave((char_u *)"text");
trunc_string(s, buf, 20, 40);
assert(STRCMP(buf, "text") == 0);
vim_free(buf);
vim_free(s);

buf = alloc(40);
s = vim_strsave((char_u *)"a text that fits");
trunc_string(s, buf, 34, 40);
assert(STRCMP(buf, "a text that fits") == 0);
vim_free(buf);
vim_free(s);

buf = alloc(40);
s = vim_strsave((char_u *)"a short text");
trunc_string(s, buf, 20, 40);
assert(STRCMP(buf, "a short text") == 0);
vim_free(buf);
vim_free(s);

buf = alloc(40);
s = vim_strsave((char_u *)"a text tha just fits");
trunc_string(s, buf, 20, 40);
assert(STRCMP(buf, "a text tha just fits") == 0);
vim_free(buf);
vim_free(s);

buf = alloc(40);
s = vim_strsave((char_u *)"a text that nott fits");
trunc_string(s, buf, 20, 40);
assert(STRCMP(buf, "a text t...nott fits") == 0);
vim_free(buf);
vim_free(s);
}











static void
test_vim_snprintf(void)
{
int n;
size_t bsize;
int bsize_int;
char *ptr = (char *)0x87654321;



for (bsize = 0; bsize < 15; ++bsize)
{
bsize_int = (int)bsize - 1;





char *buf = malloc(bsize);

n = vim_snprintf(buf, bsize, "%d", 1234567);
assert(n == 7);
assert(bsize == 0 || STRNCMP(buf, "1234567", bsize_int) == 0);
assert(bsize == 0 || buf[MIN(n, bsize_int)] == '\0');

n = vim_snprintf(buf, bsize, "%ld", 1234567L);
assert(n == 7);
assert(bsize == 0 || STRNCMP(buf, "1234567", bsize_int) == 0);
assert(bsize == 0 || buf[MIN(n, bsize_int)] == '\0');

n = vim_snprintf(buf, bsize, "%9ld", 1234567L);
assert(n == 9);
assert(bsize == 0 || STRNCMP(buf, " 1234567", bsize_int) == 0);
assert(bsize == 0 || buf[MIN(n, bsize_int)] == '\0');

n = vim_snprintf(buf, bsize, "%-9ld", 1234567L);
assert(n == 9);
assert(bsize == 0 || STRNCMP(buf, "1234567 ", bsize_int) == 0);
assert(bsize == 0 || buf[MIN(n, bsize_int)] == '\0');

n = vim_snprintf(buf, bsize, "%x", 0xdeadbeef);
assert(n == 8);
assert(bsize == 0 || STRNCMP(buf, "deadbeef", bsize_int) == 0);
assert(bsize == 0 || buf[MIN(n, bsize_int)] == '\0');

n = vim_snprintf(buf, bsize, fmt_06b, (uvarnumber_T)12);
assert(n == 6);
assert(bsize == 0 || STRNCMP(buf, "001100", bsize_int) == 0);
assert(bsize == 0 || buf[MIN(n, bsize_int)] == '\0');

#if defined(FEAT_FLOAT)
n = vim_snprintf(buf, bsize, "%f", 1.234);
assert(n == 8);
assert(bsize == 0 || STRNCMP(buf, "1.234000", bsize_int) == 0);
assert(bsize == 0 || buf[MIN(n, bsize_int)] == '\0');

n = vim_snprintf(buf, bsize, "%e", 1.234);
assert(n == 12);
assert(bsize == 0 || STRNCMP(buf, "1.234000e+00", bsize_int) == 0);
assert(bsize == 0 || buf[MIN(n, bsize_int)] == '\0');

n = vim_snprintf(buf, bsize, "%f", 0.0/0.0);
assert(n == 3);
assert(bsize == 0 || STRNCMP(buf, "nan", bsize_int) == 0);
assert(bsize == 0 || buf[MIN(n, bsize_int)] == '\0');

n = vim_snprintf(buf, bsize, "%f", 1.0/0.0);
assert(n == 3);
assert(bsize == 0 || STRNCMP(buf, "inf", bsize_int) == 0);
assert(bsize == 0 || buf[MIN(n, bsize_int)] == '\0');

n = vim_snprintf(buf, bsize, "%f", -1.0/0.0);
assert(n == 4);
assert(bsize == 0 || STRNCMP(buf, "-inf", bsize_int) == 0);
assert(bsize == 0 || buf[MIN(n, bsize_int)] == '\0');

n = vim_snprintf(buf, bsize, "%f", -0.0);
assert(n == 9);
assert(bsize == 0 || STRNCMP(buf, "-0.000000", bsize_int) == 0);
assert(bsize == 0 || buf[MIN(n, bsize_int)] == '\0');
#endif

n = vim_snprintf(buf, bsize, "%s", "漢語");
assert(n == 6);
assert(bsize == 0 || STRNCMP(buf, "漢語", bsize_int) == 0);
assert(bsize == 0 || buf[MIN(n, bsize_int)] == '\0');

n = vim_snprintf(buf, bsize, "%8s", "漢語");
assert(n == 8);
assert(bsize == 0 || STRNCMP(buf, " 漢語", bsize_int) == 0);
assert(bsize == 0 || buf[MIN(n, bsize_int)] == '\0');

n = vim_snprintf(buf, bsize, "%-8s", "漢語");
assert(n == 8);
assert(bsize == 0 || STRNCMP(buf, "漢語 ", bsize_int) == 0);
assert(bsize == 0 || buf[MIN(n, bsize_int)] == '\0');

n = vim_snprintf(buf, bsize, "%.3s", "漢語");
assert(n == 3);
assert(bsize == 0 || STRNCMP(buf, "漢", bsize_int) == 0);
assert(bsize == 0 || buf[MIN(n, bsize_int)] == '\0');

n = vim_snprintf(buf, bsize, fmt_5S, "foo");
assert(n == 5);
assert(bsize == 0 || STRNCMP(buf, " foo", bsize_int) == 0);
assert(bsize == 0 || buf[MIN(n, bsize_int)] == '\0');

n = vim_snprintf(buf, bsize, "%%%%%%");
assert(n == 3);
assert(bsize == 0 || STRNCMP(buf, "%%%", bsize_int) == 0);
assert(bsize == 0 || buf[MIN(n, bsize_int)] == '\0');

n = vim_snprintf(buf, bsize, "%c%c", 1, 2);
assert(n == 2);
assert(bsize == 0 || STRNCMP(buf, "\x01\x02", bsize_int) == 0);
assert(bsize == 0 || buf[MIN(n, bsize_int)] == '\0');




#if defined(SUN_SYSTEM)
#define PREFIX_LEN 0
#define PREFIX_STR1 ""
#define PREFIX_STR2 "00"
#else
#define PREFIX_LEN 2
#define PREFIX_STR1 "0x"
#define PREFIX_STR2 "0x"
#endif
n = vim_snprintf(buf, bsize, "%p", ptr);
assert(n == 8 + PREFIX_LEN);
assert(bsize == 0
|| STRNCMP(buf, PREFIX_STR1 "87654321", bsize_int) == 0);
assert(bsize == 0 || buf[MIN(n, bsize_int)] == '\0');

n = vim_snprintf(buf, bsize, fmt_012p, ptr);
assert(n == 12);
assert(bsize == 0
|| STRNCMP(buf, PREFIX_STR2 "0087654321", bsize_int) == 0);
assert(bsize == 0 || buf[MIN(n, bsize_int)] == '\0');

free(buf);
}
}

int
main(int argc, char **argv)
{
vim_memset(&params, 0, sizeof(params));
params.argc = argc;
params.argv = argv;
common_init(&params);

set_option_value((char_u *)"encoding", 0, (char_u *)"utf-8", 0);
init_chartab();
test_trunc_string();
test_vim_snprintf();

set_option_value((char_u *)"encoding", 0, (char_u *)"latin1", 0);
init_chartab();
test_trunc_string();
test_vim_snprintf();

return 0;
}
