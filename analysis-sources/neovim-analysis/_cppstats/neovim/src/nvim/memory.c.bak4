




#include <assert.h>
#include <inttypes.h>
#include <string.h>
#include <stdbool.h>

#include "nvim/vim.h"
#include "nvim/context.h"
#include "nvim/eval.h"
#include "nvim/highlight.h"
#include "nvim/memfile.h"
#include "nvim/memory.h"
#include "nvim/message.h"
#include "nvim/misc1.h"
#include "nvim/ui.h"
#include "nvim/sign.h"
#include "nvim/api/vim.h"

#if defined(UNIT_TESTING)
#define malloc(size) mem_malloc(size)
#define calloc(count, size) mem_calloc(count, size)
#define realloc(ptr, size) mem_realloc(ptr, size)
#define free(ptr) mem_free(ptr)
MemMalloc mem_malloc = &malloc;
MemFree mem_free = &free;
MemCalloc mem_calloc = &calloc;
MemRealloc mem_realloc = &realloc;
#endif

#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "memory.c.generated.h"
#endif

#if defined(EXITFREE)
bool entered_free_all_mem = false;
#endif



void try_to_free_memory(void)
{
static bool trying_to_free = false;

if (trying_to_free)
return;
trying_to_free = true;


clear_sb_text(true);

mf_release_all();

trying_to_free = false;
}









void *try_malloc(size_t size) FUNC_ATTR_MALLOC FUNC_ATTR_ALLOC_SIZE(1)
{
size_t allocated_size = size ? size : 1;
void *ret = malloc(allocated_size);
if (!ret) {
try_to_free_memory();
ret = malloc(allocated_size);
}
return ret;
}







void *verbose_try_malloc(size_t size) FUNC_ATTR_MALLOC FUNC_ATTR_ALLOC_SIZE(1)
{
void *ret = try_malloc(size);
if (!ret) {
do_outofmem_msg(size);
}
return ret;
}









void *xmalloc(size_t size)
FUNC_ATTR_MALLOC FUNC_ATTR_ALLOC_SIZE(1) FUNC_ATTR_NONNULL_RET
{
void *ret = try_malloc(size);
if (!ret) {
mch_errmsg(e_outofmem);
mch_errmsg("\n");
preserve_exit();
}
return ret;
}




void xfree(void *ptr)
{
free(ptr);
}







void *xcalloc(size_t count, size_t size)
FUNC_ATTR_MALLOC FUNC_ATTR_ALLOC_SIZE_PROD(1, 2) FUNC_ATTR_NONNULL_RET
{
size_t allocated_count = count && size ? count : 1;
size_t allocated_size = count && size ? size : 1;
void *ret = calloc(allocated_count, allocated_size);
if (!ret) {
try_to_free_memory();
ret = calloc(allocated_count, allocated_size);
if (!ret) {
mch_errmsg(e_outofmem);
mch_errmsg("\n");
preserve_exit();
}
}
return ret;
}






void *xrealloc(void *ptr, size_t size)
FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_ALLOC_SIZE(2) FUNC_ATTR_NONNULL_RET
{
size_t allocated_size = size ? size : 1;
void *ret = realloc(ptr, allocated_size);
if (!ret) {
try_to_free_memory();
ret = realloc(ptr, allocated_size);
if (!ret) {
mch_errmsg(e_outofmem);
mch_errmsg("\n");
preserve_exit();
}
}
return ret;
}






void *xmallocz(size_t size)
FUNC_ATTR_MALLOC FUNC_ATTR_NONNULL_RET FUNC_ATTR_WARN_UNUSED_RESULT
{
size_t total_size = size + 1;
if (total_size < size) {
mch_errmsg(_("Vim: Data too large to fit into virtual memory space\n"));
preserve_exit();
}

void *ret = xmalloc(total_size);
((char*)ret)[size] = 0;

return ret;
}









void *xmemdupz(const void *data, size_t len)
FUNC_ATTR_MALLOC FUNC_ATTR_NONNULL_RET FUNC_ATTR_WARN_UNUSED_RESULT
FUNC_ATTR_NONNULL_ALL
{
return memcpy(xmallocz(len), data, len);
}








char *xstrchrnul(const char *str, char c)
FUNC_ATTR_NONNULL_RET FUNC_ATTR_NONNULL_ALL FUNC_ATTR_PURE
{
char *p = strchr(str, c);
return p ? p : (char *)(str + strlen(str));
}









void *xmemscan(const void *addr, char c, size_t size)
FUNC_ATTR_NONNULL_RET FUNC_ATTR_NONNULL_ALL FUNC_ATTR_PURE
{
char *p = memchr(addr, c, size);
return p ? p : (char *)addr + size;
}








void strchrsub(char *str, char c, char x)
FUNC_ATTR_NONNULL_ALL
{
assert(c != '\0');
while ((str = strchr(str, c))) {
*str++ = x;
}
}







void memchrsub(void *data, char c, char x, size_t len)
FUNC_ATTR_NONNULL_ALL
{
char *p = data, *end = (char *)data + len;
while ((p = memchr(p, c, (size_t)(end - p)))) {
*p++ = x;
}
}








size_t strcnt(const char *str, char c)
FUNC_ATTR_NONNULL_ALL FUNC_ATTR_PURE
{
assert(c != 0);
size_t cnt = 0;
while ((str = strchr(str, c))) {
cnt++;
str++; 
}
return cnt;
}







size_t memcnt(const void *data, char c, size_t len)
FUNC_ATTR_NONNULL_ALL FUNC_ATTR_PURE
{
size_t cnt = 0;
const char *ptr = data, *end = ptr + len;
while ((ptr = memchr(ptr, c, (size_t)(end - ptr))) != NULL) {
cnt++;
ptr++; 
}
return cnt;
}















char *xstpcpy(char *restrict dst, const char *restrict src)
FUNC_ATTR_NONNULL_RET FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ALL
{
const size_t len = strlen(src);
return (char *)memcpy(dst, src, len + 1) + len;
}

















char *xstpncpy(char *restrict dst, const char *restrict src, size_t maxlen)
FUNC_ATTR_NONNULL_RET FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ALL
{
const char *p = memchr(src, '\0', maxlen);
if (p) {
size_t srclen = (size_t)(p - src);
memcpy(dst, src, srclen);
memset(dst + srclen, 0, maxlen - srclen);
return dst + srclen;
} else {
memcpy(dst, src, maxlen);
return dst + maxlen;
}
}













size_t xstrlcpy(char *restrict dst, const char *restrict src, size_t dsize)
FUNC_ATTR_NONNULL_ALL
{
size_t slen = strlen(src);

if (dsize) {
size_t len = MIN(slen, dsize - 1);
memcpy(dst, src, len);
dst[len] = '\0';
}

return slen; 
}















size_t xstrlcat(char *const dst, const char *const src, const size_t dsize)
FUNC_ATTR_NONNULL_ALL
{
assert(dsize > 0);
const size_t dlen = strlen(dst);
assert(dlen < dsize);
const size_t slen = strlen(src);

if (slen > dsize - dlen - 1) {
memmove(dst + dlen, src, dsize - dlen - 1);
dst[dsize - 1] = '\0';
} else {
memmove(dst + dlen, src, slen + 1);
}

return slen + dlen; 
}






char *xstrdup(const char *str)
FUNC_ATTR_MALLOC FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_RET
FUNC_ATTR_NONNULL_ALL
{
return xmemdupz(str, strlen(str));
}




char *xstrdupnul(const char *const str)
FUNC_ATTR_MALLOC FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_RET
{
if (str == NULL) {
return xmallocz(0);
} else {
return xstrdup(str);
}
}









void *xmemrchr(const void *src, uint8_t c, size_t len)
FUNC_ATTR_NONNULL_ALL FUNC_ATTR_PURE
{
while (len--) {
if (((uint8_t *)src)[len] == c) {
return (uint8_t *) src + len;
}
}
return NULL;
}






char *xstrndup(const char *str, size_t len)
FUNC_ATTR_MALLOC FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_RET
FUNC_ATTR_NONNULL_ALL
{
char *p = memchr(str, '\0', len);
return xmemdupz(str, p ? (size_t)(p - str) : len);
}







void *xmemdup(const void *data, size_t len)
FUNC_ATTR_MALLOC FUNC_ATTR_ALLOC_SIZE(2) FUNC_ATTR_NONNULL_RET
FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ALL
{
return memcpy(xmalloc(len), data, len);
}


bool strequal(const char *a, const char *b)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
return (a == NULL && b == NULL) || (a && b && strcmp(a, b) == 0);
}


bool striequal(const char *a, const char *b)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
return (a == NULL && b == NULL) || (a && b && STRICMP(a, b) == 0);
}





void do_outofmem_msg(size_t size)
{
if (!did_outofmem_msg) {

emsg_silent = 0;



did_outofmem_msg = true;

EMSGU(_("E342: Out of memory! (allocating %" PRIu64 " bytes)"), size);
}
}


void time_to_bytes(time_t time_, uint8_t buf[8])
{


for (size_t i = 7, bufi = 0; bufi < 8; i--, bufi++) {
buf[bufi] = (uint8_t)((uint64_t)time_ >> (i * 8));
}
}

#if defined(EXITFREE)

#include "nvim/file_search.h"
#include "nvim/buffer.h"
#include "nvim/charset.h"
#include "nvim/diff.h"
#include "nvim/edit.h"
#include "nvim/ex_cmds.h"
#include "nvim/ex_docmd.h"
#include "nvim/ex_getln.h"
#include "nvim/fileio.h"
#include "nvim/fold.h"
#include "nvim/getchar.h"
#include "nvim/mark.h"
#include "nvim/mbyte.h"
#include "nvim/memline.h"
#include "nvim/move.h"
#include "nvim/option.h"
#include "nvim/ops.h"
#include "nvim/os_unix.h"
#include "nvim/path.h"
#include "nvim/quickfix.h"
#include "nvim/regexp.h"
#include "nvim/screen.h"
#include "nvim/search.h"
#include "nvim/spell.h"
#include "nvim/syntax.h"
#include "nvim/tag.h"
#include "nvim/window.h"
#include "nvim/os/os.h"
#include "nvim/eval/typval.h"








void free_all_mem(void)
{
buf_T *buf, *nextbuf;



if (entered_free_all_mem) {
return;
}
entered_free_all_mem = true;


block_autocmds();


p_ea = false;
if (first_tabpage->tp_next != NULL)
do_cmdline_cmd("tabonly!");

if (!ONE_WINDOW) {


curwin = firstwin;
do_cmdline_cmd("only!");
}


spell_free_all();


ex_comclear(NULL);


do_cmdline_cmd("aunmenu *");
do_cmdline_cmd("menutranslate clear");


do_cmdline_cmd("lmapclear");
do_cmdline_cmd("xmapclear");
do_cmdline_cmd("mapclear");
do_cmdline_cmd("mapclear!");
do_cmdline_cmd("abclear");
do_cmdline_cmd("breakdel *");
do_cmdline_cmd("profdel *");
do_cmdline_cmd("set keymap=");

free_titles();
free_findfile();


free_all_autocmds();
free_all_marks();
alist_clear(&global_alist);
free_homedir();
free_users();
free_search_patterns();
free_old_sub();
free_last_insert();
free_prev_shellcmd();
free_regexp_stuff();
free_tag_stuff();
free_cd_dir();
free_signs();
set_expr_line(NULL);
diff_clear(curtab);
clear_sb_text(true); 


xfree(last_cmdline);
xfree(new_last_cmdline);
set_keep_msg(NULL, 0);


p_hi = 0;
init_history();

qf_free_all(NULL);

FOR_ALL_TAB_WINDOWS(tab, win) {
qf_free_all(win);
}


close_all_scripts();


win_free_all();


free_all_options();

free_arshape_buf();


clear_registers();
ResetRedobuff();
ResetRedobuff();



free_highlight();

reset_last_sourcing();

free_tabpage(first_tabpage);
first_tabpage = NULL;


for (;; )
if (delete_first_msg() == FAIL)
break;

eval_clear();
api_vim_free_all_mem();
ctx_free_all();





p_acd = false;
for (buf = firstbuf; buf != NULL; ) {
bufref_T bufref;
set_bufref(&bufref, buf);
nextbuf = buf->b_next;
close_buffer(NULL, buf, DOBUF_WIPE, false);

buf = bufref_valid(&bufref) ? nextbuf : firstbuf;
}


screen_free_all_mem();

clear_hl_tables(false);
list_free_log();

check_quickfix_busy();
}

#endif

