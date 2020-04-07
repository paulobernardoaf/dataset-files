#include "vim.h"
#if defined(FEAT_CRYPT) || defined(PROTO)
typedef struct {
char *name; 
char *magic; 
int salt_len; 
int seed_len; 
#if defined(CRYPT_NOT_INPLACE)
int works_inplace; 
#endif
int whole_undofile; 
int (* self_test_fn)();
int (* init_fn)(cryptstate_T *state, char_u *key,
char_u *salt, int salt_len, char_u *seed, int seed_len);
void (*encode_fn)(cryptstate_T *state, char_u *from, size_t len,
char_u *to);
void (*decode_fn)(cryptstate_T *state, char_u *from, size_t len,
char_u *to);
long (*encode_buffer_fn)(cryptstate_T *state, char_u *from, size_t len,
char_u **newptr);
long (*decode_buffer_fn)(cryptstate_T *state, char_u *from, size_t len,
char_u **newptr);
void (*encode_inplace_fn)(cryptstate_T *state, char_u *p1, size_t len,
char_u *p2);
void (*decode_inplace_fn)(cryptstate_T *state, char_u *p1, size_t len,
char_u *p2);
} cryptmethod_T;
static cryptmethod_T cryptmethods[CRYPT_M_COUNT] = {
{
"zip",
"VimCrypt~01!",
0,
0,
#if defined(CRYPT_NOT_INPLACE)
TRUE,
#endif
FALSE,
NULL,
crypt_zip_init,
crypt_zip_encode, crypt_zip_decode,
NULL, NULL,
crypt_zip_encode, crypt_zip_decode,
},
{
"blowfish",
"VimCrypt~02!",
8,
8,
#if defined(CRYPT_NOT_INPLACE)
TRUE,
#endif
FALSE,
blowfish_self_test,
crypt_blowfish_init,
crypt_blowfish_encode, crypt_blowfish_decode,
NULL, NULL,
crypt_blowfish_encode, crypt_blowfish_decode,
},
{
"blowfish2",
"VimCrypt~03!",
8,
8,
#if defined(CRYPT_NOT_INPLACE)
TRUE,
#endif
TRUE,
blowfish_self_test,
crypt_blowfish_init,
crypt_blowfish_encode, crypt_blowfish_decode,
NULL, NULL,
crypt_blowfish_encode, crypt_blowfish_decode,
},
};
#define CRYPT_MAGIC_LEN 12 
static char crypt_magic_head[] = "VimCrypt~";
int
crypt_method_nr_from_name(char_u *name)
{
int i;
for (i = 0; i < CRYPT_M_COUNT; ++i)
if (STRCMP(name, cryptmethods[i].name) == 0)
return i;
return 0;
}
int
crypt_method_nr_from_magic(char *ptr, int len)
{
int i;
if (len < CRYPT_MAGIC_LEN)
return -1;
for (i = 0; i < CRYPT_M_COUNT; i++)
if (memcmp(ptr, cryptmethods[i].magic, CRYPT_MAGIC_LEN) == 0)
return i;
i = (int)STRLEN(crypt_magic_head);
if (len >= i && memcmp(ptr, crypt_magic_head, i) == 0)
emsg(_("E821: File is encrypted with unknown method"));
return -1;
}
#if defined(CRYPT_NOT_INPLACE)
int
crypt_works_inplace(cryptstate_T *state)
{
return cryptmethods[state->method_nr].works_inplace;
}
#endif
int
crypt_get_method_nr(buf_T *buf)
{
return crypt_method_nr_from_name(*buf->b_p_cm == NUL ? p_cm : buf->b_p_cm);
}
int
crypt_whole_undofile(int method_nr)
{
return cryptmethods[method_nr].whole_undofile;
}
int
crypt_get_header_len(int method_nr)
{
return CRYPT_MAGIC_LEN
+ cryptmethods[method_nr].salt_len
+ cryptmethods[method_nr].seed_len;
}
void
crypt_set_cm_option(buf_T *buf, int method_nr)
{
free_string_option(buf->b_p_cm);
buf->b_p_cm = vim_strsave((char_u *)cryptmethods[method_nr].name);
}
int
crypt_self_test(void)
{
int method_nr = crypt_get_method_nr(curbuf);
if (cryptmethods[method_nr].self_test_fn == NULL)
return OK;
return cryptmethods[method_nr].self_test_fn();
}
cryptstate_T *
crypt_create(
int method_nr,
char_u *key,
char_u *salt,
int salt_len,
char_u *seed,
int seed_len)
{
cryptstate_T *state = ALLOC_ONE(cryptstate_T);
if (state == NULL)
return state;
state->method_nr = method_nr;
if (cryptmethods[method_nr].init_fn(
state, key, salt, salt_len, seed, seed_len) == FAIL)
{
vim_free(state);
return NULL;
}
return state;
}
cryptstate_T *
crypt_create_from_header(
int method_nr,
char_u *key,
char_u *header)
{
char_u *salt = NULL;
char_u *seed = NULL;
int salt_len = cryptmethods[method_nr].salt_len;
int seed_len = cryptmethods[method_nr].seed_len;
if (salt_len > 0)
salt = header + CRYPT_MAGIC_LEN;
if (seed_len > 0)
seed = header + CRYPT_MAGIC_LEN + salt_len;
return crypt_create(method_nr, key, salt, salt_len, seed, seed_len);
}
cryptstate_T *
crypt_create_from_file(FILE *fp, char_u *key)
{
int method_nr;
int header_len;
char magic_buffer[CRYPT_MAGIC_LEN];
char_u *buffer;
cryptstate_T *state;
if (fread(magic_buffer, CRYPT_MAGIC_LEN, 1, fp) != 1)
return NULL;
method_nr = crypt_method_nr_from_magic(magic_buffer, CRYPT_MAGIC_LEN);
if (method_nr < 0)
return NULL;
header_len = crypt_get_header_len(method_nr);
if ((buffer = alloc(header_len)) == NULL)
return NULL;
mch_memmove(buffer, magic_buffer, CRYPT_MAGIC_LEN);
if (header_len > CRYPT_MAGIC_LEN
&& fread(buffer + CRYPT_MAGIC_LEN,
header_len - CRYPT_MAGIC_LEN, 1, fp) != 1)
{
vim_free(buffer);
return NULL;
}
state = crypt_create_from_header(method_nr, key, buffer);
vim_free(buffer);
return state;
}
cryptstate_T *
crypt_create_for_writing(
int method_nr,
char_u *key,
char_u **header,
int *header_len)
{
int len = crypt_get_header_len(method_nr);
char_u *salt = NULL;
char_u *seed = NULL;
int salt_len = cryptmethods[method_nr].salt_len;
int seed_len = cryptmethods[method_nr].seed_len;
cryptstate_T *state;
*header_len = len;
*header = alloc(len);
if (*header == NULL)
return NULL;
mch_memmove(*header, cryptmethods[method_nr].magic, CRYPT_MAGIC_LEN);
if (salt_len > 0 || seed_len > 0)
{
if (salt_len > 0)
salt = *header + CRYPT_MAGIC_LEN;
if (seed_len > 0)
seed = *header + CRYPT_MAGIC_LEN + salt_len;
sha2_seed(salt, salt_len, seed, seed_len);
}
state = crypt_create(method_nr, key, salt, salt_len, seed, seed_len);
if (state == NULL)
VIM_CLEAR(*header);
return state;
}
void
crypt_free_state(cryptstate_T *state)
{
vim_free(state->method_state);
vim_free(state);
}
#if defined(CRYPT_NOT_INPLACE)
long
crypt_encode_alloc(
cryptstate_T *state,
char_u *from,
size_t len,
char_u **newptr)
{
cryptmethod_T *method = &cryptmethods[state->method_nr];
if (method->encode_buffer_fn != NULL)
return method->encode_buffer_fn(state, from, len, newptr);
if (len == 0)
return (long)len;
*newptr = alloc(len);
if (*newptr == NULL)
return -1;
method->encode_fn(state, from, len, *newptr);
return (long)len;
}
long
crypt_decode_alloc(
cryptstate_T *state,
char_u *ptr,
long len,
char_u **newptr)
{
cryptmethod_T *method = &cryptmethods[state->method_nr];
if (method->decode_buffer_fn != NULL)
return method->decode_buffer_fn(state, ptr, len, newptr);
if (len == 0)
return len;
*newptr = alloc(len);
if (*newptr == NULL)
return -1;
method->decode_fn(state, ptr, len, *newptr);
return len;
}
#endif
void
crypt_encode(
cryptstate_T *state,
char_u *from,
size_t len,
char_u *to)
{
cryptmethods[state->method_nr].encode_fn(state, from, len, to);
}
#if 0 
void
crypt_decode(
cryptstate_T *state,
char_u *from,
size_t len,
char_u *to)
{
cryptmethods[state->method_nr].decode_fn(state, from, len, to);
}
#endif
void
crypt_encode_inplace(
cryptstate_T *state,
char_u *buf,
size_t len)
{
cryptmethods[state->method_nr].encode_inplace_fn(state, buf, len, buf);
}
void
crypt_decode_inplace(
cryptstate_T *state,
char_u *buf,
size_t len)
{
cryptmethods[state->method_nr].decode_inplace_fn(state, buf, len, buf);
}
void
crypt_free_key(char_u *key)
{
char_u *p;
if (key != NULL)
{
for (p = key; *p != NUL; ++p)
*p = 0;
vim_free(key);
}
}
void
crypt_check_method(int method)
{
if (method < CRYPT_M_BF2)
{
msg_scroll = TRUE;
msg(_("Warning: Using a weak encryption method; see :help 'cm'"));
}
}
void
crypt_check_current_method(void)
{
crypt_check_method(crypt_get_method_nr(curbuf));
}
char_u *
crypt_get_key(
int store,
int twice) 
{
char_u *p1, *p2 = NULL;
int round;
for (round = 0; ; ++round)
{
cmdline_star = TRUE;
cmdline_row = msg_row;
p1 = getcmdline_prompt(NUL, round == 0
? (char_u *)_("Enter encryption key: ")
: (char_u *)_("Enter same key again: "), 0, EXPAND_NOTHING,
NULL);
cmdline_star = FALSE;
if (p1 == NULL)
break;
if (round == twice)
{
if (p2 != NULL && STRCMP(p1, p2) != 0)
{
msg(_("Keys don't match!"));
crypt_free_key(p1);
crypt_free_key(p2);
p2 = NULL;
round = -1; 
continue;
}
if (store)
{
set_option_value((char_u *)"key", 0L, p1, OPT_LOCAL);
crypt_free_key(p1);
p1 = curbuf->b_p_key;
}
break;
}
p2 = p1;
}
if (msg_didout)
msg_putchar('\n');
need_wait_return = FALSE;
msg_didout = FALSE;
crypt_free_key(p2);
return p1;
}
void
crypt_append_msg(
buf_T *buf)
{
if (crypt_get_method_nr(buf) == 0)
STRCAT(IObuff, _("[crypted]"));
else
{
STRCAT(IObuff, "[");
STRCAT(IObuff, *buf->b_p_cm == NUL ? p_cm : buf->b_p_cm);
STRCAT(IObuff, "]");
}
}
#endif 
