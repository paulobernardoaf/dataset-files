











#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>

#include "nvim/assert.h"
#include "nvim/vim.h"
#include "nvim/ascii.h"
#include "nvim/getchar.h"
#include "nvim/buffer_defs.h"
#include "nvim/charset.h"
#include "nvim/cursor.h"
#include "nvim/edit.h"
#include "nvim/eval.h"
#include "nvim/ex_docmd.h"
#include "nvim/ex_getln.h"
#include "nvim/func_attr.h"
#include "nvim/main.h"
#include "nvim/mbyte.h"
#include "nvim/memline.h"
#include "nvim/memory.h"
#include "nvim/message.h"
#include "nvim/misc1.h"
#include "nvim/keymap.h"
#include "nvim/garray.h"
#include "nvim/move.h"
#include "nvim/normal.h"
#include "nvim/ops.h"
#include "nvim/option.h"
#include "nvim/regexp.h"
#include "nvim/screen.h"
#include "nvim/ex_session.h"
#include "nvim/state.h"
#include "nvim/strings.h"
#include "nvim/ui.h"
#include "nvim/undo.h"
#include "nvim/event/loop.h"
#include "nvim/os/input.h"
#include "nvim/os/os.h"
#include "nvim/os/fileio.h"
#include "nvim/api/private/handle.h"



static int curscript = 0;
FileDescriptor *scriptin[NSCRIPT] = { NULL };





















#define MINIMAL_SIZE 20 

static buffheader_T redobuff = { { NULL, { NUL } }, NULL, 0, 0 };
static buffheader_T old_redobuff = { { NULL, { NUL } }, NULL, 0, 0 };
static buffheader_T recordbuff = { { NULL, { NUL } }, NULL, 0, 0 };


static buffheader_T readbuf1 = { { NULL, { NUL } }, NULL, 0, 0 };


static buffheader_T readbuf2 = { { NULL, { NUL } }, NULL, 0, 0 };

static int typeahead_char = 0; 





static int block_redo = FALSE;






#define MAP_HASH(mode, c1) (((mode) & (NORMAL + VISUAL + SELECTMODE + OP_PENDING + TERM_FOCUS)) ? (c1) : ((c1) ^ 0x80))






static mapblock_T *(maphash[MAX_MAPHASH]);
static bool maphash_valid = false;




static mapblock_T *first_abbr = NULL; 

static int KeyNoremap = 0; 




















#define RM_YES 0 
#define RM_NONE 1 
#define RM_SCRIPT 2 
#define RM_ABBR 4 





#define TYPELEN_INIT (5 * (MAXMAPLEN + 3))
static char_u typebuf_init[TYPELEN_INIT]; 
static char_u noremapbuf_init[TYPELEN_INIT]; 

static size_t last_recorded_len = 0; 

#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "getchar.c.generated.h"
#endif




void free_buff(buffheader_T *buf)
{
buffblock_T *p, *np;

for (p = buf->bh_first.b_next; p != NULL; p = np) {
np = p->b_next;
xfree(p);
}
buf->bh_first.b_next = NULL;
}





static char_u *get_buffcont(buffheader_T *buffer,
int dozero 
)
{
size_t count = 0;
char_u *p = NULL;
char_u *p2;


for (const buffblock_T *bp = buffer->bh_first.b_next;
bp != NULL; bp = bp->b_next) {
count += STRLEN(bp->b_str);
}

if (count || dozero) {
p = xmalloc(count + 1);
p2 = p;
for (const buffblock_T *bp = buffer->bh_first.b_next;
bp != NULL; bp = bp->b_next) {
for (const char_u *str = bp->b_str; *str;) {
*p2++ = *str++;
}
}
*p2 = NUL;
}
return p;
}






char_u *get_recorded(void)
{
char_u *p;
size_t len;

p = get_buffcont(&recordbuff, TRUE);
free_buff(&recordbuff);





len = STRLEN(p);
if (len >= last_recorded_len) {
len -= last_recorded_len;
p[len] = NUL;
}





if (len > 0 && restart_edit != 0 && p[len - 1] == Ctrl_O)
p[len - 1] = NUL;

return p;
}





char_u *get_inserted(void)
{
return get_buffcont(&redobuff, FALSE);
}








static void add_buff(buffheader_T *const buf, const char *const s,
ptrdiff_t slen)
{
if (slen < 0) {
slen = (ptrdiff_t)strlen(s);
}
if (slen == 0) { 
return;
}

if (buf->bh_first.b_next == NULL) { 
buf->bh_space = 0;
buf->bh_curr = &(buf->bh_first);
} else if (buf->bh_curr == NULL) { 
IEMSG(_("E222: Add to read buffer"));
return;
} else if (buf->bh_index != 0) {
memmove(buf->bh_first.b_next->b_str,
buf->bh_first.b_next->b_str + buf->bh_index,
STRLEN(buf->bh_first.b_next->b_str + buf->bh_index) + 1);
}
buf->bh_index = 0;

size_t len;
if (buf->bh_space >= (size_t)slen) {
len = STRLEN(buf->bh_curr->b_str);
STRLCPY(buf->bh_curr->b_str + len, s, slen + 1);
buf->bh_space -= (size_t)slen;
} else {
if (slen < MINIMAL_SIZE) {
len = MINIMAL_SIZE;
} else {
len = (size_t)slen;
}
buffblock_T *p = xmalloc(sizeof(buffblock_T) + len);
buf->bh_space = len - (size_t)slen;
STRLCPY(p->b_str, s, slen + 1);

p->b_next = buf->bh_curr->b_next;
buf->bh_curr->b_next = p;
buf->bh_curr = p;
}
return;
}




static void add_num_buff(buffheader_T *buf, long n)
{
char number[32];
snprintf(number, sizeof(number), "%ld", n);
add_buff(buf, number, -1L);
}





static void add_char_buff(buffheader_T *buf, int c)
{
uint8_t bytes[MB_MAXBYTES + 1];

int len;
if (IS_SPECIAL(c)) {
len = 1;
} else {
len = utf_char2bytes(c, bytes);
}

for (int i = 0; i < len; i++) {
if (!IS_SPECIAL(c)) {
c = bytes[i];
}

char temp[4];
if (IS_SPECIAL(c) || c == K_SPECIAL || c == NUL) {

temp[0] = (char)K_SPECIAL;
temp[1] = (char)K_SECOND(c);
temp[2] = (char)K_THIRD(c);
temp[3] = NUL;
} else {
temp[0] = (char)c;
temp[1] = NUL;
}
add_buff(buf, temp, -1L);
}
}







static int read_readbuffers(int advance)
{
int c;

c = read_readbuf(&readbuf1, advance);
if (c == NUL)
c = read_readbuf(&readbuf2, advance);
return c;
}

static int read_readbuf(buffheader_T *buf, int advance)
{
char_u c;

if (buf->bh_first.b_next == NULL) { 
return NUL;
}

buffblock_T *const curr = buf->bh_first.b_next;
c = curr->b_str[buf->bh_index];

if (advance) {
if (curr->b_str[++buf->bh_index] == NUL) {
buf->bh_first.b_next = curr->b_next;
xfree(curr);
buf->bh_index = 0;
}
}
return c;
}




static void start_stuff(void)
{
if (readbuf1.bh_first.b_next != NULL) {
readbuf1.bh_curr = &(readbuf1.bh_first);
readbuf1.bh_space = 0;
}
if (readbuf2.bh_first.b_next != NULL) {
readbuf2.bh_curr = &(readbuf2.bh_first);
readbuf2.bh_space = 0;
}
}




int stuff_empty(void)
{
return (readbuf1.bh_first.b_next == NULL && readbuf2.bh_first.b_next == NULL);
}





int readbuf1_empty(void)
{
return (readbuf1.bh_first.b_next == NULL);
}




void typeahead_noflush(int c)
{
typeahead_char = c;
}






void flush_buffers(flush_buffers_T flush_typeahead)
{
init_typebuf();

start_stuff();
while (read_readbuffers(TRUE) != NUL) {
}

if (flush_typeahead == FLUSH_MINIMAL) {

typebuf.tb_off += typebuf.tb_maplen;
typebuf.tb_len -= typebuf.tb_maplen;
} else {

if (flush_typeahead == FLUSH_INPUT) {



while (inchar(typebuf.tb_buf, typebuf.tb_buflen - 1, 10L) != 0) {
}
}
typebuf.tb_off = MAXMAPLEN;
typebuf.tb_len = 0;


typebuf_was_filled = false;
}
typebuf.tb_maplen = 0;
typebuf.tb_silent = 0;
cmd_silent = false;
typebuf.tb_no_abbr_cnt = 0;
}





void ResetRedobuff(void)
{
if (!block_redo) {
free_buff(&old_redobuff);
old_redobuff = redobuff;
redobuff.bh_first.b_next = NULL;
}
}





void CancelRedo(void)
{
if (!block_redo) {
free_buff(&redobuff);
redobuff = old_redobuff;
old_redobuff.bh_first.b_next = NULL;
start_stuff();
while (read_readbuffers(TRUE) != NUL) {
}
}
}



void saveRedobuff(save_redo_T *save_redo)
{
save_redo->sr_redobuff = redobuff;
redobuff.bh_first.b_next = NULL;
save_redo->sr_old_redobuff = old_redobuff;
old_redobuff.bh_first.b_next = NULL;


char *const s = (char *)get_buffcont(&save_redo->sr_redobuff, false);
if (s != NULL) {
add_buff(&redobuff, s, -1L);
xfree(s);
}
}



void restoreRedobuff(save_redo_T *save_redo)
{
free_buff(&redobuff);
redobuff = save_redo->sr_redobuff;
free_buff(&old_redobuff);
old_redobuff = save_redo->sr_old_redobuff;
}





void AppendToRedobuff(const char *s)
{
if (!block_redo) {
add_buff(&redobuff, (const char *)s, -1L);
}
}






void AppendToRedobuffLit(const char_u *str, int len)
{
if (block_redo) {
return;
}

const char *s = (const char *)str;
while (len < 0 ? *s != NUL : s - (const char *)str < len) {


const char *start = s;
while (*s >= ' ' && *s < DEL && (len < 0 || s - (const char *)str < len)) {
s++;
}



if (*s == NUL && (s[-1] == '0' || s[-1] == '^')) {
s--;
}
if (s > start) {
add_buff(&redobuff, start, (long)(s - start));
}

if (*s == NUL || (len >= 0 && s - (const char *)str >= len)) {
break;
}



const int c = (has_mbyte
? mb_cptr2char_adv((const char_u **)&s)
: (uint8_t)(*s++));
if (c < ' ' || c == DEL || (*s == NUL && (c == '0' || c == '^'))) {
add_char_buff(&redobuff, Ctrl_V);
}


if (*s == NUL && c == '0') {
add_buff(&redobuff, "048", 3L);
} else {
add_char_buff(&redobuff, c);
}
}
}





void AppendCharToRedobuff(int c)
{
if (!block_redo)
add_char_buff(&redobuff, c);
}




void AppendNumberToRedobuff(long n)
{
if (!block_redo)
add_num_buff(&redobuff, n);
}





void stuffReadbuff(const char *s)
{
add_buff(&readbuf1, s, -1L);
}



void stuffRedoReadbuff(const char *s)
{
add_buff(&readbuf2, s, -1L);
}

void stuffReadbuffLen(const char *s, long len)
{
add_buff(&readbuf1, s, len);
}






void stuffReadbuffSpec(const char *s)
{
while (*s != NUL) {
if ((uint8_t)(*s) == K_SPECIAL && s[1] != NUL && s[2] != NUL) {

stuffReadbuffLen(s, 3);
s += 3;
} else {
int c = mb_ptr2char_adv((const char_u **)&s);
if (c == CAR || c == NL || c == ESC) {
c = ' ';
}
stuffcharReadbuff(c);
}
}
}





void stuffcharReadbuff(int c)
{
add_char_buff(&readbuf1, c);
}




void stuffnumReadbuff(long n)
{
add_num_buff(&readbuf1, n);
}







static int read_redo(bool init, bool old_redo)
{
static buffblock_T *bp;
static char_u *p;
int c;
int n;
char_u buf[MB_MAXBYTES + 1];
int i;

if (init) {
bp = old_redo ? old_redobuff.bh_first.b_next : redobuff.bh_first.b_next;
if (bp == NULL) {
return FAIL;
}
p = bp->b_str;
return OK;
}
if ((c = *p) == NUL) {
return c;
}



if (has_mbyte && (c != K_SPECIAL || p[1] == KS_SPECIAL))
n = MB_BYTE2LEN_CHECK(c);
else
n = 1;
for (i = 0;; ++i) {
if (c == K_SPECIAL) { 
c = TO_SPECIAL(p[1], p[2]);
p += 2;
}
if (*++p == NUL && bp->b_next != NULL) {
bp = bp->b_next;
p = bp->b_str;
}
buf[i] = (char_u)c;
if (i == n - 1) { 
if (n != 1) {
c = utf_ptr2char(buf);
}
break;
}
c = *p;
if (c == NUL) 
break;
}

return c;
}




static void copy_redo(bool old_redo)
{
int c;

while ((c = read_redo(false, old_redo)) != NUL) {
add_char_buff(&readbuf2, c);
}
}








int start_redo(long count, bool old_redo)
{
int c;


if (read_redo(true, old_redo) == FAIL) {
return FAIL;
}

c = read_redo(false, old_redo);


if (c == '"') {
add_buff(&readbuf2, "\"", 1L);
c = read_redo(false, old_redo);


if (c >= '1' && c < '9')
++c;
add_char_buff(&readbuf2, c);


if (c == '=') {
add_char_buff(&readbuf2, CAR);
cmd_silent = true;
}

c = read_redo(false, old_redo);
}

if (c == 'v') { 
VIsual = curwin->w_cursor;
VIsual_active = true;
VIsual_select = false;
VIsual_reselect = true;
redo_VIsual_busy = true;
c = read_redo(false, old_redo);
}


if (count) {
while (ascii_isdigit(c)) { 
c = read_redo(false, old_redo);
}
add_num_buff(&readbuf2, count);
}


add_char_buff(&readbuf2, c);
copy_redo(old_redo);
return OK;
}






int start_redo_ins(void)
{
int c;

if (read_redo(true, false) == FAIL) {
return FAIL;
}
start_stuff();


while ((c = read_redo(false, false)) != NUL) {
if (vim_strchr((char_u *)"AaIiRrOo", c) != NULL) {
if (c == 'O' || c == 'o') {
add_buff(&readbuf2, NL_STR, -1L);
}
break;
}
}


copy_redo(false);
block_redo = true;
return OK;
}

void stop_redo_ins(void)
{
block_redo = FALSE;
}






static void init_typebuf(void)
{
if (typebuf.tb_buf == NULL) {
typebuf.tb_buf = typebuf_init;
typebuf.tb_noremap = noremapbuf_init;
typebuf.tb_buflen = TYPELEN_INIT;
typebuf.tb_len = 0;
typebuf.tb_off = MAXMAPLEN + 4;
typebuf.tb_change_cnt = 1;
}
}




















int ins_typebuf(char_u *str, int noremap, int offset, int nottyped, bool silent)
{
char_u *s1, *s2;
int newlen;
int addlen;
int i;
int newoff;
int val;
int nrm;

init_typebuf();
if (++typebuf.tb_change_cnt == 0)
typebuf.tb_change_cnt = 1;

addlen = (int)STRLEN(str);

if (offset == 0 && addlen <= typebuf.tb_off) {

typebuf.tb_off -= addlen;
memmove(typebuf.tb_buf + typebuf.tb_off, str, (size_t)addlen);
} else if (typebuf.tb_len == 0
&& typebuf.tb_buflen >= addlen + 3 * (MAXMAPLEN + 4)) {


typebuf.tb_off = (typebuf.tb_buflen - addlen - 3 * (MAXMAPLEN + 4)) / 2;
memmove(typebuf.tb_buf + typebuf.tb_off, str, (size_t)addlen);
} else {




newoff = MAXMAPLEN + 4;
newlen = typebuf.tb_len + addlen + newoff + 4 * (MAXMAPLEN + 4);
if (newlen < 0) { 
EMSG(_(e_toocompl)); 
setcursor();
return FAIL;
}
s1 = xmalloc((size_t)newlen);
s2 = xmalloc((size_t)newlen);
typebuf.tb_buflen = newlen;


memmove(s1 + newoff, typebuf.tb_buf + typebuf.tb_off, (size_t)offset);

memmove(s1 + newoff + offset, str, (size_t)addlen);


int bytes = typebuf.tb_len - offset + 1;
assert(bytes > 0);
memmove(s1 + newoff + offset + addlen,
typebuf.tb_buf + typebuf.tb_off + offset, (size_t)bytes);
if (typebuf.tb_buf != typebuf_init) {
xfree(typebuf.tb_buf);
}
typebuf.tb_buf = s1;

memmove(s2 + newoff, typebuf.tb_noremap + typebuf.tb_off,
(size_t)offset);
memmove(s2 + newoff + offset + addlen,
typebuf.tb_noremap + typebuf.tb_off + offset,
(size_t)(typebuf.tb_len - offset));
if (typebuf.tb_noremap != noremapbuf_init)
xfree(typebuf.tb_noremap);
typebuf.tb_noremap = s2;

typebuf.tb_off = newoff;
}
typebuf.tb_len += addlen;


if (noremap == REMAP_SCRIPT)
val = RM_SCRIPT;
else if (noremap == REMAP_SKIP)
val = RM_ABBR;
else
val = RM_NONE;









if (noremap == REMAP_SKIP)
nrm = 1;
else if (noremap < 0)
nrm = addlen;
else
nrm = noremap;
for (i = 0; i < addlen; ++i)
typebuf.tb_noremap[typebuf.tb_off + i + offset] =
(char_u)((--nrm >= 0) ? val : RM_YES);




if (nottyped || typebuf.tb_maplen > offset)
typebuf.tb_maplen += addlen;
if (silent || typebuf.tb_silent > offset) {
typebuf.tb_silent += addlen;
cmd_silent = true;
}
if (typebuf.tb_no_abbr_cnt && offset == 0) 
typebuf.tb_no_abbr_cnt += addlen;

return OK;
}







void ins_char_typebuf(int c)
{
char_u buf[MB_MAXBYTES + 1];
if (IS_SPECIAL(c)) {
buf[0] = K_SPECIAL;
buf[1] = (char_u)K_SECOND(c);
buf[2] = (char_u)K_THIRD(c);
buf[3] = NUL;
} else {
buf[utf_char2bytes(c, buf)] = NUL;
}
(void)ins_typebuf(buf, KeyNoremap, 0, !KeyTyped, cmd_silent);
}










int 
typebuf_changed (
int tb_change_cnt 
)
{
return tb_change_cnt != 0 && (typebuf.tb_change_cnt != tb_change_cnt
|| typebuf_was_filled
);
}





int typebuf_typed(void)
{
return typebuf.tb_maplen == 0;
}




int typebuf_maplen(void)
{
return typebuf.tb_maplen;
}




void del_typebuf(int len, int offset)
{
int i;

if (len == 0)
return; 

typebuf.tb_len -= len;




if (offset == 0 && typebuf.tb_buflen - (typebuf.tb_off + len)
>= 3 * MAXMAPLEN + 3)
typebuf.tb_off += len;



else {
i = typebuf.tb_off + offset;



if (typebuf.tb_off > MAXMAPLEN) {
memmove(typebuf.tb_buf + MAXMAPLEN,
typebuf.tb_buf + typebuf.tb_off, (size_t)offset);
memmove(typebuf.tb_noremap + MAXMAPLEN,
typebuf.tb_noremap + typebuf.tb_off, (size_t)offset);
typebuf.tb_off = MAXMAPLEN;
}

int bytes = typebuf.tb_len - offset + 1;
assert(bytes > 0);
memmove(typebuf.tb_buf + typebuf.tb_off + offset,
typebuf.tb_buf + i + len, (size_t)bytes);

memmove(typebuf.tb_noremap + typebuf.tb_off + offset,
typebuf.tb_noremap + i + len,
(size_t)(typebuf.tb_len - offset));
}

if (typebuf.tb_maplen > offset) { 
if (typebuf.tb_maplen < offset + len)
typebuf.tb_maplen = offset;
else
typebuf.tb_maplen -= len;
}
if (typebuf.tb_silent > offset) { 
if (typebuf.tb_silent < offset + len)
typebuf.tb_silent = offset;
else
typebuf.tb_silent -= len;
}
if (typebuf.tb_no_abbr_cnt > offset) { 
if (typebuf.tb_no_abbr_cnt < offset + len)
typebuf.tb_no_abbr_cnt = offset;
else
typebuf.tb_no_abbr_cnt -= len;
}



typebuf_was_filled = false;
if (++typebuf.tb_change_cnt == 0) {
typebuf.tb_change_cnt = 1;
}
}





static void gotchars(const char_u *chars, size_t len)
FUNC_ATTR_NONNULL_ALL
{
const char_u *s = chars;
static char_u buf[4] = { 0 };
static size_t buflen = 0;
size_t todo = len;

while (todo--) {
buf[buflen++] = *s++;



if (buflen == 1 && buf[0] == K_SPECIAL) {
continue;
}
if (buflen == 2) {
continue;
}


for (size_t i = 0; i < buflen; i++) {
updatescript(buf[i]);
}

if (reg_recording != 0) {
buf[buflen] = NUL;
add_buff(&recordbuff, (char *)buf, (ptrdiff_t)buflen);

last_recorded_len += buflen;
}
buflen = 0;
}

may_sync_undo();


debug_did_msg = FALSE;



++maptick;
}









void may_sync_undo(void)
{
if ((!(State & (INSERT + CMDLINE)) || arrow_used)
&& scriptin[curscript] == NULL)
u_sync(FALSE);
}




void alloc_typebuf(void)
{
typebuf.tb_buf = xmalloc(TYPELEN_INIT);
typebuf.tb_noremap = xmalloc(TYPELEN_INIT);
typebuf.tb_buflen = TYPELEN_INIT;
typebuf.tb_off = MAXMAPLEN + 4; 
typebuf.tb_len = 0;
typebuf.tb_maplen = 0;
typebuf.tb_silent = 0;
typebuf.tb_no_abbr_cnt = 0;
if (++typebuf.tb_change_cnt == 0)
typebuf.tb_change_cnt = 1;
}




void free_typebuf(void)
{
if (typebuf.tb_buf == typebuf_init) {
internal_error("Free typebuf 1");
} else {
XFREE_CLEAR(typebuf.tb_buf);
}
if (typebuf.tb_noremap == noremapbuf_init) {
internal_error("Free typebuf 2");
} else {
XFREE_CLEAR(typebuf.tb_noremap);
}
}





static typebuf_T saved_typebuf[NSCRIPT];

void save_typebuf(void)
{
init_typebuf();
saved_typebuf[curscript] = typebuf;
alloc_typebuf();
}

static int old_char = -1; 
static int old_mod_mask; 
static int old_mouse_grid; 
static int old_mouse_row; 
static int old_mouse_col; 





void save_typeahead(tasave_T *tp)
{
tp->save_typebuf = typebuf;
alloc_typebuf();
tp->typebuf_valid = TRUE;
tp->old_char = old_char;
tp->old_mod_mask = old_mod_mask;
old_char = -1;

tp->save_readbuf1 = readbuf1;
readbuf1.bh_first.b_next = NULL;
tp->save_readbuf2 = readbuf2;
readbuf2.bh_first.b_next = NULL;
}





void restore_typeahead(tasave_T *tp)
{
if (tp->typebuf_valid) {
free_typebuf();
typebuf = tp->save_typebuf;
}

old_char = tp->old_char;
old_mod_mask = tp->old_mod_mask;

free_buff(&readbuf1);
readbuf1 = tp->save_readbuf1;
free_buff(&readbuf2);
readbuf2 = tp->save_readbuf2;
}




void 
openscript (
char_u *name,
int directly 
)
{
if (curscript + 1 == NSCRIPT) {
EMSG(_(e_nesting));
return;
}



if (check_secure()) {
return;
}

if (ignore_script) {

return;
}

if (scriptin[curscript] != NULL) 
++curscript;

expand_env(name, NameBuff, MAXPATHL);
int error;
if ((scriptin[curscript] = file_open_new(&error, (char *)NameBuff,
kFileReadOnly, 0)) == NULL) {
emsgf(_(e_notopen_2), name, os_strerror(error));
if (curscript) {
curscript--;
}
return;
}
save_typebuf();







if (directly) {
oparg_T oa;
int oldcurscript;
int save_State = State;
int save_restart_edit = restart_edit;
int save_insertmode = p_im;
int save_finish_op = finish_op;
int save_msg_scroll = msg_scroll;

State = NORMAL;
msg_scroll = FALSE; 
restart_edit = 0; 
p_im = FALSE; 
clear_oparg(&oa);
finish_op = FALSE;

oldcurscript = curscript;
do {
update_topline_cursor(); 
normal_cmd(&oa, false); 
vpeekc(); 
} while (scriptin[oldcurscript] != NULL);

State = save_State;
msg_scroll = save_msg_scroll;
restart_edit = save_restart_edit;
p_im = save_insertmode;
finish_op = save_finish_op;
}
}




static void closescript(void)
{
free_typebuf();
typebuf = saved_typebuf[curscript];

file_free(scriptin[curscript], false);
scriptin[curscript] = NULL;
if (curscript > 0)
--curscript;
}

#if defined(EXITFREE)
void close_all_scripts(void)
{
while (scriptin[0] != NULL)
closescript();
}

#endif




int using_script(void)
{
return scriptin[curscript] != NULL;
}



void before_blocking(void)
{
updatescript(0);
if (may_garbage_collect) {
garbage_collect(false);
}
}






static void updatescript(int c)
{
static int count = 0;

if (c && scriptout) {
putc(c, scriptout);
}
bool idle = (c == 0);
if (idle || (p_uc > 0 && ++count >= p_uc)) {
ml_sync_all(idle, true,
(!!p_fs || idle)); 
count = 0;
}
}










int vgetc(void)
{
int c, c2;
int n;
char_u buf[MB_MAXBYTES + 1];
int i;



if (may_garbage_collect && want_garbage_collect) {
garbage_collect(false);
}





if (old_char != -1) {
c = old_char;
old_char = -1;
mod_mask = old_mod_mask;
mouse_grid = old_mouse_grid;
mouse_row = old_mouse_row;
mouse_col = old_mouse_col;
} else {
mod_mask = 0x0;
last_recorded_len = 0;
for (;; ) { 
bool did_inc = false;
if (mod_mask) { 
no_mapping++;
did_inc = true; 
}
c = vgetorpeek(true);
if (did_inc) {
no_mapping--;
}


if (c == K_SPECIAL) {
no_mapping++;
c2 = vgetorpeek(true); 
c = vgetorpeek(true);
no_mapping--;
if (c2 == KS_MODIFIER) {
mod_mask = c;
continue;
}
c = TO_SPECIAL(c2, c);

}



switch (c) {
case K_KPLUS: c = '+'; break;
case K_KMINUS: c = '-'; break;
case K_KDIVIDE: c = '/'; break;
case K_KMULTIPLY: c = '*'; break;
case K_KENTER: c = CAR; break;
case K_KPOINT: c = '.'; break;
case K_KCOMMA: c = ','; break;
case K_KEQUAL: c = '='; break;
case K_K0: c = '0'; break;
case K_K1: c = '1'; break;
case K_K2: c = '2'; break;
case K_K3: c = '3'; break;
case K_K4: c = '4'; break;
case K_K5: c = '5'; break;
case K_K6: c = '6'; break;
case K_K7: c = '7'; break;
case K_K8: c = '8'; break;
case K_K9: c = '9'; break;

case K_XHOME:
case K_ZHOME:
if (mod_mask == MOD_MASK_SHIFT) {
c = K_S_HOME;
mod_mask = 0;
} else if (mod_mask == MOD_MASK_CTRL) {
c = K_C_HOME;
mod_mask = 0;
} else {
c = K_HOME;
}
break;
case K_XEND:
case K_ZEND:
if (mod_mask == MOD_MASK_SHIFT) {
c = K_S_END;
mod_mask = 0;
} else if (mod_mask == MOD_MASK_CTRL) {
c = K_C_END;
mod_mask = 0;
} else {
c = K_END;
}
break;

case K_XUP: c = K_UP; break;
case K_XDOWN: c = K_DOWN; break;
case K_XLEFT: c = K_LEFT; break;
case K_XRIGHT: c = K_RIGHT; break;
}




if ((n = MB_BYTE2LEN_CHECK(c)) > 1) {
no_mapping++;
buf[0] = (char_u)c;
for (i = 1; i < n; i++) {
buf[i] = (char_u)vgetorpeek(true);
if (buf[i] == K_SPECIAL
) {





c = vgetorpeek(true);
if (vgetorpeek(true) == (int)KE_CSI && c == KS_EXTRA) {
buf[i] = CSI;
}
}
}
no_mapping--;
c = utf_ptr2char(buf);
}

break;
}
}






may_garbage_collect = false;

return c;
}





int safe_vgetc(void)
{
int c;

c = vgetc();
if (c == NUL) {
c = get_keystroke(NULL);
}
return c;
}





int plain_vgetc(void)
{
int c;

do {
c = safe_vgetc();
} while (c == K_IGNORE || c == K_VER_SCROLLBAR || c == K_HOR_SCROLLBAR);
return c;
}







int vpeekc(void)
{
if (old_char != -1)
return old_char;
return vgetorpeek(false);
}






int vpeekc_any(void)
{
int c;

c = vpeekc();
if (c == NUL && typebuf.tb_len > 0)
c = ESC;
return c;
}





int char_avail(void)
{
int retval;

no_mapping++;
retval = vpeekc();
no_mapping--;
return retval != NUL;
}

void 
vungetc ( 
int c
)
{
old_char = c;
old_mod_mask = mod_mask;
old_mouse_grid = mouse_grid;
old_mouse_row = mouse_row;
old_mouse_col = mouse_col;
}























static int vgetorpeek(bool advance)
{
int c, c1;
int keylen;
char_u *s;
mapblock_T *mp;
mapblock_T *mp2;
mapblock_T *mp_match;
int mp_match_len = 0;
int timedout = FALSE; 

int mapdepth = 0; 
int mode_deleted = FALSE; 
int local_State;
int mlen;
int max_mlen;
int i;
int new_wcol, new_wrow;
int n;
int nolmaplen;
int old_wcol, old_wrow;
int wait_tb_len;












if (vgetc_busy > 0
&& ex_normal_busy == 0
)
return NUL;

local_State = get_real_state();

++vgetc_busy;

if (advance)
KeyStuffed = FALSE;

init_typebuf();
start_stuff();
if (advance && typebuf.tb_maplen == 0) {
reg_executing = 0;
}
do {



if (typeahead_char != 0) {
c = typeahead_char;
if (advance)
typeahead_char = 0;
} else {
c = read_readbuffers(advance);
}
if (c != NUL && !got_int) {
if (advance) {



KeyStuffed = true;
}
if (typebuf.tb_no_abbr_cnt == 0)
typebuf.tb_no_abbr_cnt = 1; 
} else {






for (;; ) {





if (typebuf.tb_maplen)
line_breakcheck();
else
os_breakcheck(); 
keylen = 0;
if (got_int) {

c = inchar(typebuf.tb_buf, typebuf.tb_buflen - 1, 0L);





if ((c || typebuf.tb_maplen)
&& (State & (INSERT + CMDLINE))) {
c = ESC;
} else {
c = Ctrl_C;
}
flush_buffers(FLUSH_INPUT); 

if (advance) {


*typebuf.tb_buf = (char_u)c;
gotchars(typebuf.tb_buf, 1);
}
cmd_silent = false;

break;
} else if (typebuf.tb_len > 0) {















mp = NULL;
max_mlen = 0;
c1 = typebuf.tb_buf[typebuf.tb_off];
if (no_mapping == 0 && maphash_valid
&& (no_zero_mapping == 0 || c1 != '0')
&& (typebuf.tb_maplen == 0
|| (p_remap
&& (typebuf.tb_noremap[typebuf.tb_off]
& (RM_NONE|RM_ABBR)) == 0))
&& !(p_paste && (State & (INSERT + CMDLINE)))
&& !(State == HITRETURN && (c1 == CAR || c1 == ' '))
&& State != ASKMORE
&& State != CONFIRM
&& !((ctrl_x_mode_not_default() && vim_is_ctrl_x_key(c1))
|| ((compl_cont_status & CONT_LOCAL)
&& (c1 == Ctrl_N || c1 == Ctrl_P)))
) {
if (c1 == K_SPECIAL) {
nolmaplen = 2;
} else {
LANGMAP_ADJUST(c1, (State & (CMDLINE | INSERT)) == 0
&& get_real_state() != SELECTMODE);
nolmaplen = 0;
}

mp = curbuf->b_maphash[MAP_HASH(local_State, c1)];
mp2 = maphash[MAP_HASH(local_State, c1)];
if (mp == NULL) {

mp = mp2;
mp2 = NULL;
}







mp_match = NULL;
mp_match_len = 0;
for (; mp != NULL;
mp->m_next == NULL ? (mp = mp2, mp2 = NULL) :
(mp = mp->m_next)) {





if (mp->m_keys[0] == c1
&& (mp->m_mode & local_State)
&& ((mp->m_mode & LANGMAP) == 0
|| typebuf.tb_maplen == 0)) {
int nomap = nolmaplen;
int c2;

for (mlen = 1; mlen < typebuf.tb_len; ++mlen) {
c2 = typebuf.tb_buf[typebuf.tb_off + mlen];
if (nomap > 0)
--nomap;
else if (c2 == K_SPECIAL)
nomap = 2;
else
LANGMAP_ADJUST(c2, TRUE);
if (mp->m_keys[mlen] != c2)
break;
}





char_u *p1 = mp->m_keys;
char_u *p2 = (char_u *)mb_unescape((const char **)&p1);

if (p2 != NULL && MB_BYTE2LEN(c1) > utfc_ptr2len(p2)) {
mlen = 0;
}




keylen = mp->m_keylen;
if (mlen == keylen
|| (mlen == typebuf.tb_len
&& typebuf.tb_len < keylen)) {





s = typebuf.tb_noremap + typebuf.tb_off;
if (*s == RM_SCRIPT
&& (mp->m_keys[0] != K_SPECIAL
|| mp->m_keys[1] != KS_EXTRA
|| mp->m_keys[2]
!= (int)KE_SNR))
continue;




for (n = mlen; --n >= 0; )
if (*s++ & (RM_NONE|RM_ABBR))
break;
if (n >= 0)
continue;

if (keylen > typebuf.tb_len) {
if (!timedout && !(mp_match != NULL
&& mp_match->m_nowait)) {

keylen = KEYLEN_PART_MAP;
break;
}
} else if (keylen > mp_match_len
|| (keylen == mp_match_len
&& mp_match != NULL
&& (mp_match->m_mode & LANGMAP) == 0
&& (mp->m_mode & LANGMAP) != 0)) {

mp_match = mp;
mp_match_len = keylen;
}
} else {

if (max_mlen < mlen) {
max_mlen = mlen;
}
}
}
}



if (keylen != KEYLEN_PART_MAP) {
mp = mp_match;
keylen = mp_match_len;
}
}

if (*p_pt != NUL && mp == NULL && (State & (INSERT|NORMAL))) {
bool match = typebuf_match_len(p_pt, &mlen);
if (match) {

if (mlen > typebuf.tb_maplen) {
gotchars(typebuf.tb_buf + typebuf.tb_off + typebuf.tb_maplen,
(size_t)(mlen - typebuf.tb_maplen));
}

del_typebuf(mlen, 0); 
set_option_value("paste", !p_paste, NULL, 0);
if (!(State & INSERT)) {
msg_col = 0;
msg_row = Rows - 1;
msg_clr_eos(); 
}
status_redraw_all();
redraw_statuslines();
showmode();
setcursor();
continue;
}

if (mlen == typebuf.tb_len)
keylen = KEYLEN_PART_KEY;
else if (max_mlen < mlen)


max_mlen = mlen + 1;
}

if ((mp == NULL || max_mlen >= mp_match_len)
&& keylen != KEYLEN_PART_MAP) {


keylen = 0;
(void)keylen; 


if (mp == NULL) {

c = typebuf.tb_buf[typebuf.tb_off] & 255;
if (advance) { 
cmd_silent = (typebuf.tb_silent > 0);
if (typebuf.tb_maplen > 0) {
KeyTyped = false;
} else {
KeyTyped = true;

gotchars(typebuf.tb_buf + typebuf.tb_off, 1);
}
KeyNoremap = typebuf.tb_noremap[typebuf.tb_off];
del_typebuf(1, 0);
}
break; 
} else {
keylen = mp_match_len;
}
}


if (keylen >= 0 && keylen <= typebuf.tb_len) {
int save_m_expr;
int save_m_noremap;
int save_m_silent;
char_u *save_m_keys;
char_u *save_m_str;



if (keylen > typebuf.tb_maplen && (mp->m_mode & LANGMAP) == 0) {
gotchars(typebuf.tb_buf + typebuf.tb_off + typebuf.tb_maplen,
(size_t)(keylen - typebuf.tb_maplen));
}

cmd_silent = (typebuf.tb_silent > 0);
del_typebuf(keylen, 0); 





if (++mapdepth >= p_mmd) {
EMSG(_("E223: recursive mapping"));
if (State & CMDLINE)
redrawcmdline();
else
setcursor();
flush_buffers(FLUSH_MINIMAL);
mapdepth = 0; 
c = -1;
break;
}






if (VIsual_active && VIsual_select
&& (mp->m_mode & VISUAL)) {
VIsual_select = FALSE;
(void)ins_typebuf(K_SELECT_STRING, REMAP_NONE,
0, TRUE, FALSE);
}





save_m_expr = mp->m_expr;
save_m_noremap = mp->m_noremap;
save_m_silent = mp->m_silent;
save_m_keys = NULL; 
save_m_str = NULL; 






if (mp->m_expr) {
int save_vgetc_busy = vgetc_busy;

vgetc_busy = 0;
save_m_keys = vim_strsave(mp->m_keys);
save_m_str = vim_strsave(mp->m_str);
s = eval_map_expr(save_m_str, NUL);
vgetc_busy = save_vgetc_busy;
} else
s = mp->m_str;









if (s == NULL)
i = FAIL;
else {
int noremap;



if (keylen > typebuf.tb_maplen && (mp->m_mode & LANGMAP) != 0) {
gotchars(s, STRLEN(s));
}

if (save_m_noremap != REMAP_YES)
noremap = save_m_noremap;
else if (
STRNCMP(s, save_m_keys != NULL
? save_m_keys : mp->m_keys,
(size_t)keylen)
!= 0)
noremap = REMAP_YES;
else
noremap = REMAP_SKIP;
i = ins_typebuf(s, noremap,
0, TRUE, cmd_silent || save_m_silent);
if (save_m_expr)
xfree(s);
}
xfree(save_m_keys);
xfree(save_m_str);
if (i == FAIL) {
c = -1;
break;
}
continue;
}
}












c = 0;
new_wcol = curwin->w_wcol;
new_wrow = curwin->w_wrow;
if (advance
&& typebuf.tb_len == 1
&& typebuf.tb_buf[typebuf.tb_off] == ESC
&& !no_mapping
&& ex_normal_busy == 0
&& typebuf.tb_maplen == 0
&& (State & INSERT)
&& (p_timeout
|| (keylen == KEYLEN_PART_KEY && p_ttimeout))
&& (c = inchar(typebuf.tb_buf + typebuf.tb_off + typebuf.tb_len,
3, 25L)) == 0) {
colnr_T col = 0, vcol;
char_u *ptr;

if (mode_displayed) {
unshowmode(TRUE);
mode_deleted = TRUE;
}
validate_cursor();
old_wcol = curwin->w_wcol;
old_wrow = curwin->w_wrow;


if (curwin->w_cursor.col != 0) {
if (curwin->w_wcol > 0) {
if (did_ai) {





col = vcol = curwin->w_wcol = 0;
ptr = get_cursor_line_ptr();
while (col < curwin->w_cursor.col) {
if (!ascii_iswhite(ptr[col]))
curwin->w_wcol = vcol;
vcol += lbr_chartabsize(ptr, ptr + col,
(colnr_T)vcol);
if (has_mbyte)
col += (*mb_ptr2len)(ptr + col);
else
++col;
}
curwin->w_wrow = curwin->w_cline_row
+ curwin->w_wcol / curwin->w_width_inner;
curwin->w_wcol %= curwin->w_width_inner;
curwin->w_wcol += curwin_col_off();
col = 0; 
} else {
--curwin->w_wcol;
col = curwin->w_cursor.col - 1;
}
} else if (curwin->w_p_wrap && curwin->w_wrow) {
curwin->w_wrow--;
curwin->w_wcol = curwin->w_width_inner - 1;
col = curwin->w_cursor.col - 1;
}
if (col > 0 && curwin->w_wcol > 0) {


ptr = get_cursor_line_ptr();
col -= utf_head_off(ptr, ptr + col);
if (utf_ptr2cells(ptr + col) > 1) {
curwin->w_wcol--;
}
}
}
setcursor();
ui_flush();
new_wcol = curwin->w_wcol;
new_wrow = curwin->w_wrow;
curwin->w_wcol = old_wcol;
curwin->w_wrow = old_wrow;
}
if (c < 0)
continue; 



for (n = 1; n <= c; n++) {
typebuf.tb_noremap[typebuf.tb_off + n] = RM_YES;
}
typebuf.tb_len += c;


if (typebuf.tb_len >= typebuf.tb_maplen + MAXMAPLEN) {
timedout = TRUE;
continue;
}

if (ex_normal_busy > 0) {
static int tc = 0;




if (typebuf.tb_len > 0) {
timedout = TRUE;
continue;
}






if (p_im && (State & INSERT))
c = Ctrl_L;
else if ((State & CMDLINE)
|| (cmdwin_type > 0 && tc == ESC)
)
c = Ctrl_C;
else
c = ESC;
tc = c;
break;
}











if (((State & INSERT) != 0 || p_lz) && (State & CMDLINE) == 0
&& advance && must_redraw != 0 && !need_wait_return) {
update_screen(0);
setcursor(); 
}






i = 0;
c1 = 0;
if (typebuf.tb_len > 0 && advance && !exmode_active) {
if (((State & (NORMAL | INSERT)) || State == LANGMAP)
&& State != HITRETURN) {

if (State & INSERT
&& ptr2cells(typebuf.tb_buf + typebuf.tb_off
+ typebuf.tb_len - 1) == 1) {
edit_putchar(typebuf.tb_buf[typebuf.tb_off
+ typebuf.tb_len - 1], FALSE);
setcursor(); 
c1 = 1;
}

old_wcol = curwin->w_wcol;
old_wrow = curwin->w_wrow;
curwin->w_wcol = new_wcol;
curwin->w_wrow = new_wrow;
push_showcmd();
if (typebuf.tb_len > SHOWCMD_COLS)
i = typebuf.tb_len - SHOWCMD_COLS;
while (i < typebuf.tb_len)
(void)add_to_showcmd(typebuf.tb_buf[typebuf.tb_off
+ i++]);
curwin->w_wcol = old_wcol;
curwin->w_wrow = old_wrow;
}


if ((State & CMDLINE) && cmdline_star == 0) {
char_u *p = typebuf.tb_buf + typebuf.tb_off + typebuf.tb_len - 1;
if (ptr2cells(p) == 1 && *p < 128) {
putcmdline((char)(*p), false);
c1 = 1;
}
}
}




if (typebuf.tb_len == 0) {


timedout = false;
}

long wait_time = 0;

if (advance) {
if (typebuf.tb_len == 0
|| !(p_timeout || (p_ttimeout && keylen == KEYLEN_PART_KEY))) {

wait_time = -1L;
} else if (keylen == KEYLEN_PART_KEY && p_ttm >= 0) {
wait_time = p_ttm;
} else {
wait_time = p_tm;
}
}

wait_tb_len = typebuf.tb_len;
c = inchar(typebuf.tb_buf + typebuf.tb_off + typebuf.tb_len,
typebuf.tb_buflen - typebuf.tb_off - typebuf.tb_len - 1,
wait_time);

if (i != 0)
pop_showcmd();
if (c1 == 1) {
if (State & INSERT)
edit_unputchar();
if (State & CMDLINE)
unputcmdline();
else
setcursor(); 
}

if (c < 0)
continue; 
if (c == NUL) { 
if (!advance)
break;
if (wait_tb_len > 0) { 
timedout = TRUE;
continue;
}
} else { 
while (typebuf.tb_buf[typebuf.tb_off
+ typebuf.tb_len] != NUL)
typebuf.tb_noremap[typebuf.tb_off
+ typebuf.tb_len++] = RM_YES;
}
} 
} 


} while (c < 0 || (advance && c == NUL));






if (advance && p_smd && msg_silent == 0 && (State & INSERT)) {
if (c == ESC && !mode_deleted && !no_mapping && mode_displayed) {
if (typebuf.tb_len && !KeyTyped)
redraw_cmdline = TRUE; 
else
unshowmode(FALSE);
} else if (c != ESC && mode_deleted) {
if (typebuf.tb_len && !KeyTyped)
redraw_cmdline = TRUE; 
else
showmode();
}
}

if (timedout && c == ESC) {
char_u nop_buf[3];



nop_buf[0] = K_SPECIAL;
nop_buf[1] = KS_EXTRA;
nop_buf[2] = KE_NOP;
gotchars(nop_buf, 3);
}

--vgetc_busy;

return c;
}
























int inchar(
char_u *buf,
int maxlen,
long wait_time 
)
{
int len = 0; 
int retesc = false; 
const int tb_change_cnt = typebuf.tb_change_cnt;

if (wait_time == -1L || wait_time > 100L) {

ui_flush();
}






if (State != HITRETURN) {
did_outofmem_msg = FALSE; 
did_swapwrite_msg = FALSE; 
}



ptrdiff_t read_size = -1;
while (scriptin[curscript] != NULL && read_size <= 0 && !ignore_script) {
char script_char;
if (got_int
|| (read_size = file_read(scriptin[curscript], &script_char, 1)) != 1) {



closescript();



if (got_int) {
retesc = true;
} else {
return -1;
}
} else {
buf[0] = (char_u)script_char;
len = 1;
}
}

if (read_size <= 0) { 






if (got_int) {
#define DUM_LEN MAXMAPLEN * 3 + 3
char_u dum[DUM_LEN + 1];

for (;; ) {
len = os_inchar(dum, DUM_LEN, 0L, 0, NULL);
if (len == 0 || (len == 1 && dum[0] == 3)) {
break;
}
}
return retesc;
}



ui_flush();



len = os_inchar(buf, maxlen / 3, (int)wait_time, tb_change_cnt, NULL);
}



if (typebuf_changed(tb_change_cnt)) {
return 0;
}




if (len > 0 && ++typebuf.tb_change_cnt == 0) {
typebuf.tb_change_cnt = 1;
}

return fix_input_buffer(buf, len);
}




int fix_input_buffer(char_u *buf, int len)
FUNC_ATTR_NONNULL_ALL
{
if (!using_script()) {


buf[len] = NUL;
return len;
}


int i;
char_u *p = buf;





for (i = len; --i >= 0; ++p) {
if (p[0] == NUL
|| (p[0] == K_SPECIAL
&& (i < 2 || p[1] != KS_EXTRA))) {
memmove(p + 3, p + 1, (size_t)i);
p[2] = (char_u)K_THIRD(p[0]);
p[1] = (char_u)K_SECOND(p[0]);
p[0] = K_SPECIAL;
p += 2;
len += 2;
}
}
*p = NUL; 
return len;
}























void set_maparg_lhs_rhs(const char_u *orig_lhs, const size_t orig_lhs_len,
const char_u *orig_rhs, const size_t orig_rhs_len,
int cpo_flags, MapArguments *mapargs)
{
char_u *lhs_buf = NULL;
char_u *rhs_buf = NULL;








char_u *replaced = replace_termcodes(orig_lhs, orig_lhs_len, &lhs_buf,
true, true, true, cpo_flags);
mapargs->lhs_len = STRLEN(replaced);
xstrlcpy((char *)mapargs->lhs, (char *)replaced, sizeof(mapargs->lhs));

mapargs->orig_rhs_len = orig_rhs_len;
mapargs->orig_rhs = xcalloc(mapargs->orig_rhs_len + 1, sizeof(char_u));
xstrlcpy((char *)mapargs->orig_rhs, (char *)orig_rhs,
mapargs->orig_rhs_len + 1);

if (STRICMP(orig_rhs, "<nop>") == 0) { 
mapargs->rhs = xcalloc(1, sizeof(char_u)); 
mapargs->rhs_len = 0;
mapargs->rhs_is_noop = true;
} else {
replaced = replace_termcodes(orig_rhs, orig_rhs_len, &rhs_buf,
false, true, true, cpo_flags);
mapargs->rhs_len = STRLEN(replaced);
mapargs->rhs_is_noop = false;
mapargs->rhs = xcalloc(mapargs->rhs_len + 1, sizeof(char_u));
xstrlcpy((char *)mapargs->rhs, (char *)replaced, mapargs->rhs_len + 1);
}

xfree(lhs_buf);
xfree(rhs_buf);
}




















int str_to_mapargs(const char_u *strargs, bool is_unmap, MapArguments *mapargs)
{
const char_u *to_parse = strargs;
to_parse = skipwhite(to_parse);
MapArguments parsed_args; 
memset(&parsed_args, 0, sizeof(parsed_args));



while (true) {
if (STRNCMP(to_parse, "<buffer>", 8) == 0) {
to_parse = skipwhite(to_parse + 8);
parsed_args.buffer = true;
continue;
}

if (STRNCMP(to_parse, "<nowait>", 8) == 0) {
to_parse = skipwhite(to_parse + 8);
parsed_args.nowait = true;
continue;
}

if (STRNCMP(to_parse, "<silent>", 8) == 0) {
to_parse = skipwhite(to_parse + 8);
parsed_args.silent = true;
continue;
}


if (STRNCMP(to_parse, "<special>", 9) == 0) {
to_parse = skipwhite(to_parse + 9);
continue;
}

if (STRNCMP(to_parse, "<script>", 8) == 0) {
to_parse = skipwhite(to_parse + 8);
parsed_args.script = true;
continue;
}

if (STRNCMP(to_parse, "<expr>", 6) == 0) {
to_parse = skipwhite(to_parse + 6);
parsed_args.expr = true;
continue;
}

if (STRNCMP(to_parse, "<unique>", 8) == 0) {
to_parse = skipwhite(to_parse + 8);
parsed_args.unique = true;
continue;
}
break;
}











const char_u *lhs_end = to_parse;
bool do_backslash = (vim_strchr(p_cpo, CPO_BSLASH) == NULL);
while (*lhs_end && (is_unmap || !ascii_iswhite(*lhs_end))) {
if ((lhs_end[0] == Ctrl_V || (do_backslash && lhs_end[0] == '\\'))
&& lhs_end[1] != NUL) {
lhs_end++; 
}
lhs_end++;
}



const char_u *rhs_start = skipwhite(lhs_end);



size_t orig_lhs_len = (size_t)(lhs_end - to_parse);
char_u *lhs_to_replace = xcalloc(orig_lhs_len + 1, sizeof(char_u));
xstrlcpy((char *)lhs_to_replace, (char *)to_parse, orig_lhs_len + 1);

size_t orig_rhs_len = STRLEN(rhs_start);
set_maparg_lhs_rhs(lhs_to_replace, orig_lhs_len,
rhs_start, orig_rhs_len,
CPO_TO_CPO_FLAGS, &parsed_args);

xfree(lhs_to_replace);

*mapargs = parsed_args;

if (parsed_args.lhs_len > MAXMAPLEN) {
return 1;
}
return 0;
}











int buf_do_map(int maptype, MapArguments *args, int mode, bool is_abbrev,
buf_T *buf)
{
mapblock_T *mp, **mpp;
char_u *p;
int n;
int len = 0; 
int did_it = false;
int did_local = false;
int round;
int retval = 0;
int hash;
int new_hash;
mapblock_T **abbr_table;
mapblock_T **map_table;
int noremap;

map_table = maphash;
abbr_table = &first_abbr;


if (maptype == 2) {
noremap = REMAP_NONE;
} else {
noremap = REMAP_YES;
}

if (args->buffer) {


map_table = buf->b_maphash;
abbr_table = &buf->b_first_abbr;
}
if (args->script) {
noremap = REMAP_SCRIPT;
}

validate_maphash();

bool has_lhs = (args->lhs[0] != NUL);
bool has_rhs = (args->rhs[0] != NUL) || args->rhs_is_noop;


if (maptype == 1 && !has_lhs) {
retval = 1;
goto theend;
}

char_u *lhs = (char_u *)&args->lhs;
char_u *rhs = (char_u *)args->rhs;
char_u *orig_rhs = args->orig_rhs;


if (has_lhs) {
len = (int)args->lhs_len;
if (len > MAXMAPLEN) {
retval = 1;
goto theend;
}

if (is_abbrev && maptype != 1) {






if (has_mbyte) {
int first, last;
int same = -1;

first = vim_iswordp(lhs);
last = first;
p = lhs + (*mb_ptr2len)(lhs);
n = 1;
while (p < lhs + len) {
n++; 
last = vim_iswordp(p); 
if (same == -1 && last != first) {
same = n - 1; 
}
p += (*mb_ptr2len)(p);
}
if (last && n > 2 && same >= 0 && same < n - 1) {
retval = 1;
goto theend;
}
} else if (vim_iswordc(lhs[len - 1])) { 
for (n = 0; n < len - 2; n++) {
if (vim_iswordc(lhs[n]) != vim_iswordc(lhs[len - 2])) {
retval = 1;
goto theend;
}
} 
}

for (n = 0; n < len; n++) {
if (ascii_iswhite(lhs[n])) {
retval = 1;
goto theend;
}
} 
}
}

if (has_lhs && has_rhs && is_abbrev) { 
no_abbr = false; 
}

if (!has_lhs || (maptype != 1 && !has_rhs)) {
msg_start();
}


if (map_table == buf->b_maphash && has_lhs && has_rhs && maptype != 1) {

for (hash = 0; hash < 256 && !got_int; hash++) {
if (is_abbrev) {
if (hash != 0) { 
break;
}
mp = first_abbr;
} else {
mp = maphash[hash];
}
for (; mp != NULL && !got_int; mp = mp->m_next) {

if ((mp->m_mode & mode) != 0
&& mp->m_keylen == len
&& args->unique
&& STRNCMP(mp->m_keys, lhs, (size_t)len) == 0) {
if (is_abbrev) {
EMSG2(_("E224: global abbreviation already exists for %s"),
mp->m_keys);
} else {
EMSG2(_("E225: global mapping already exists for %s"), mp->m_keys);
}
retval = 5;
goto theend;
}
}
}
}


if (map_table != buf->b_maphash && !has_rhs && maptype != 1) {

for (hash = 0; hash < 256 && !got_int; hash++) {
if (is_abbrev) {
if (hash != 0) { 
break;
}
mp = buf->b_first_abbr;
} else {
mp = buf->b_maphash[hash];
}
for (; mp != NULL && !got_int; mp = mp->m_next) {

if ((mp->m_mode & mode) != 0) {
if (!has_lhs) { 
showmap(mp, true);
did_local = true;
} else {
n = mp->m_keylen;
if (STRNCMP(mp->m_keys, lhs, (size_t)(n < len ? n : len)) == 0) {
showmap(mp, true);
did_local = true;
}
}
}
}
}
}







for (round = 0; (round == 0 || maptype == 1) && round <= 1
&& !did_it && !got_int; round++) {

for (hash = 0; hash < 256 && !got_int; hash++) {
if (is_abbrev) {
if (hash > 0) { 
break;
}
mpp = abbr_table;
} else {
mpp = &(map_table[hash]);
}
for (mp = *mpp; mp != NULL && !got_int; mp = *mpp) {
if (!(mp->m_mode & mode)) { 
mpp = &(mp->m_next);
continue;
}
if (!has_lhs) { 
showmap(mp, map_table != maphash);
did_it = true;
} else { 
if (round) { 
n = (int)STRLEN(mp->m_str);
p = mp->m_str;
} else {
n = mp->m_keylen;
p = mp->m_keys;
}
if (STRNCMP(p, lhs, (size_t)(n < len ? n : len)) == 0) {
if (maptype == 1) { 




if (n != len && (!is_abbrev || round || n > len
|| *skipwhite(lhs + n) != NUL)) {
mpp = &(mp->m_next);
continue;
}


mp->m_mode &= ~mode;
did_it = true; 
} else if (!has_rhs) { 
showmap(mp, map_table != maphash);
did_it = true;
} else if (n != len) { 
mpp = &(mp->m_next);
continue;
} else if (args->unique) {
if (is_abbrev) {
EMSG2(_("E226: abbreviation already exists for %s"), p);
} else {
EMSG2(_("E227: mapping already exists for %s"), p);
}
retval = 5;
goto theend;
} else { 
mp->m_mode &= ~mode; 
if (mp->m_mode == 0 && !did_it) { 
xfree(mp->m_str);
mp->m_str = vim_strsave(rhs);
xfree(mp->m_orig_str);
mp->m_orig_str = vim_strsave(orig_rhs);
mp->m_noremap = noremap;
mp->m_nowait = args->nowait;
mp->m_silent = args->silent;
mp->m_mode = mode;
mp->m_expr = args->expr;
mp->m_script_ctx = current_sctx;
mp->m_script_ctx.sc_lnum += sourcing_lnum;
did_it = true;
}
}
if (mp->m_mode == 0) { 
mapblock_free(mpp);
continue; 
}


new_hash = MAP_HASH(mp->m_mode, mp->m_keys[0]);
if (!is_abbrev && new_hash != hash) {
*mpp = mp->m_next;
mp->m_next = map_table[new_hash];
map_table[new_hash] = mp;

continue; 
}
}
}
mpp = &(mp->m_next);
}
}
}

if (maptype == 1) { 
if (!did_it) {
retval = 2; 
} else if (*lhs == Ctrl_C) {

if (map_table == buf->b_maphash) {
buf->b_mapped_ctrl_c &= ~mode;
} else {
mapped_ctrl_c &= ~mode;
}
}
goto theend;
}

if (!has_lhs || !has_rhs) { 
if (!did_it && !did_local) {
if (is_abbrev) {
MSG(_("No abbreviation found"));
} else {
MSG(_("No mapping found"));
}
}
goto theend; 
}

if (did_it) { 
goto theend;
}


mp = xmalloc(sizeof(mapblock_T));


if (*lhs == Ctrl_C) {
if (map_table == buf->b_maphash) {
buf->b_mapped_ctrl_c |= mode;
} else {
mapped_ctrl_c |= mode;
}
}

mp->m_keys = vim_strsave(lhs);
mp->m_str = vim_strsave(rhs);
mp->m_orig_str = vim_strsave(orig_rhs);
mp->m_keylen = (int)STRLEN(mp->m_keys);
mp->m_noremap = noremap;
mp->m_nowait = args->nowait;
mp->m_silent = args->silent;
mp->m_mode = mode;
mp->m_expr = args->expr;
mp->m_script_ctx = current_sctx;
mp->m_script_ctx.sc_lnum += sourcing_lnum;


if (is_abbrev) {
mp->m_next = *abbr_table;
*abbr_table = mp;
} else {
n = MAP_HASH(mp->m_mode, mp->m_keys[0]);
mp->m_next = map_table[n];
map_table[n] = mp;
}

theend:
return retval;
}
















































int do_map(int maptype, char_u *arg, int mode, bool is_abbrev)
{
MapArguments parsed_args;
int result = str_to_mapargs(arg, maptype == 1, &parsed_args);
switch (result) {
case 0:
break;
case 1:

goto free_and_return;
default:
assert(false && "Unknown return code from str_to_mapargs!");
result = -1;
goto free_and_return;
} 

result = buf_do_map(maptype, &parsed_args, mode, is_abbrev, curbuf);

free_and_return:
xfree(parsed_args.rhs);
xfree(parsed_args.orig_rhs);
return result;
}





static void mapblock_free(mapblock_T **mpp)
{
mapblock_T *mp;

mp = *mpp;
xfree(mp->m_keys);
xfree(mp->m_str);
xfree(mp->m_orig_str);
*mpp = mp->m_next;
xfree(mp);
}




static void validate_maphash(void)
{
if (!maphash_valid) {
memset(maphash, 0, sizeof(maphash));
maphash_valid = TRUE;
}
}




int get_map_mode(char_u **cmdp, int forceit)
{
char_u *p;
int modec;
int mode;

p = *cmdp;
modec = *p++;
if (modec == 'i')
mode = INSERT; 
else if (modec == 'l')
mode = LANGMAP; 
else if (modec == 'c')
mode = CMDLINE; 
else if (modec == 'n' && *p != 'o') 
mode = NORMAL; 
else if (modec == 'v')
mode = VISUAL + SELECTMODE; 
else if (modec == 'x')
mode = VISUAL; 
else if (modec == 's')
mode = SELECTMODE; 
else if (modec == 'o')
mode = OP_PENDING; 
else if (modec == 't')
mode = TERM_FOCUS; 
else {
--p;
if (forceit)
mode = INSERT + CMDLINE; 
else
mode = VISUAL + SELECTMODE + NORMAL + OP_PENDING; 
}

*cmdp = p;
return mode;
}





void map_clear_mode(char_u *cmdp, char_u *arg, int forceit, int abbr)
{
int mode;
int local;

local = (STRCMP(arg, "<buffer>") == 0);
if (!local && *arg != NUL) {
EMSG(_(e_invarg));
return;
}

mode = get_map_mode(&cmdp, forceit);
map_clear_int(curbuf, mode,
local,
abbr);
}




void 
map_clear_int (
buf_T *buf, 
int mode, 
int local, 
int abbr 
)
{
mapblock_T *mp, **mpp;
int hash;
int new_hash;

validate_maphash();

for (hash = 0; hash < 256; ++hash) {
if (abbr) {
if (hash > 0) 
break;
if (local)
mpp = &buf->b_first_abbr;
else
mpp = &first_abbr;
} else {
if (local)
mpp = &buf->b_maphash[hash];
else
mpp = &maphash[hash];
}
while (*mpp != NULL) {
mp = *mpp;
if (mp->m_mode & mode) {
mp->m_mode &= ~mode;
if (mp->m_mode == 0) { 
mapblock_free(mpp);
continue;
}



new_hash = MAP_HASH(mp->m_mode, mp->m_keys[0]);
if (!abbr && new_hash != hash) {
*mpp = mp->m_next;
if (local) {
mp->m_next = buf->b_maphash[new_hash];
buf->b_maphash[new_hash] = mp;
} else {
mp->m_next = maphash[new_hash];
maphash[new_hash] = mp;
}
continue; 
}
}
mpp = &(mp->m_next);
}
}
}




char *map_mode_to_chars(int mode)
FUNC_ATTR_MALLOC FUNC_ATTR_NONNULL_RET
{
garray_T mapmode;

ga_init(&mapmode, 1, 7);

if ((mode & (INSERT + CMDLINE)) == INSERT + CMDLINE) {
ga_append(&mapmode, '!'); 
} else if (mode & INSERT) {
ga_append(&mapmode, 'i'); 
} else if (mode & LANGMAP) {
ga_append(&mapmode, 'l'); 
} else if (mode & CMDLINE) {
ga_append(&mapmode, 'c'); 
} else if ((mode & (NORMAL + VISUAL + SELECTMODE + OP_PENDING))
== NORMAL + VISUAL + SELECTMODE + OP_PENDING) {
ga_append(&mapmode, ' '); 
} else {
if (mode & NORMAL) {
ga_append(&mapmode, 'n'); 
}
if (mode & OP_PENDING) {
ga_append(&mapmode, 'o'); 
}
if (mode & TERM_FOCUS) {
ga_append(&mapmode, 't'); 
}
if ((mode & (VISUAL + SELECTMODE)) == VISUAL + SELECTMODE) {
ga_append(&mapmode, 'v'); 
} else {
if (mode & VISUAL) {
ga_append(&mapmode, 'x'); 
}
if (mode & SELECTMODE) {
ga_append(&mapmode, 's'); 
}
}
}

ga_append(&mapmode, NUL);
return (char *)mapmode.ga_data;
}

static void 
showmap (
mapblock_T *mp,
int local 
)
{
size_t len = 1;

if (message_filtered(mp->m_keys) && message_filtered(mp->m_str)) {
return;
}

if (msg_didout || msg_silent != 0) {
msg_putchar('\n');
if (got_int) 
return;
}

{
char *const mapchars = map_mode_to_chars(mp->m_mode);
msg_puts(mapchars);
len = strlen(mapchars);
xfree(mapchars);
}

while (++len <= 3)
msg_putchar(' ');


len = (size_t)msg_outtrans_special(mp->m_keys, true, 0);
do {
msg_putchar(' '); 
++len;
} while (len < 12);

if (mp->m_noremap == REMAP_NONE) {
msg_puts_attr("*", HL_ATTR(HLF_8));
} else if (mp->m_noremap == REMAP_SCRIPT) {
msg_puts_attr("&", HL_ATTR(HLF_8));
} else {
msg_putchar(' ');
}

if (local)
msg_putchar('@');
else
msg_putchar(' ');



if (*mp->m_str == NUL) {
msg_puts_attr("<Nop>", HL_ATTR(HLF_8));
} else {


char_u *s = vim_strsave(mp->m_str);
vim_unescape_csi(s);
msg_outtrans_special(s, false, 0);
xfree(s);
}
if (p_verbose > 0) {
last_set_msg(mp->m_script_ctx);
}
ui_flush(); 
}











bool map_to_exists(const char *const str, const char *const modechars,
const bool abbr)
FUNC_ATTR_NONNULL_ALL FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_PURE
{
int mode = 0;
int retval;

char_u *buf;
char_u *const rhs = replace_termcodes((const char_u *)str, strlen(str), &buf,
false, true, true,
CPO_TO_CPO_FLAGS);

#define MAPMODE(mode, modechars, chr, modeflags) do { if (strchr(modechars, chr) != NULL) { mode |= modeflags; } } while (0)





MAPMODE(mode, modechars, 'n', NORMAL);
MAPMODE(mode, modechars, 'v', VISUAL|SELECTMODE);
MAPMODE(mode, modechars, 'x', VISUAL);
MAPMODE(mode, modechars, 's', SELECTMODE);
MAPMODE(mode, modechars, 'o', OP_PENDING);
MAPMODE(mode, modechars, 'i', INSERT);
MAPMODE(mode, modechars, 'l', LANGMAP);
MAPMODE(mode, modechars, 'c', CMDLINE);
#undef MAPMODE

retval = map_to_exists_mode((const char *)rhs, mode, abbr);
xfree(buf);

return retval;
}











int map_to_exists_mode(const char *const rhs, const int mode, const bool abbr)
{
mapblock_T *mp;
int hash;
bool exp_buffer = false;

validate_maphash();


for (;;) {
for (hash = 0; hash < 256; hash++) {
if (abbr) {
if (hash > 0) { 
break;
}
if (exp_buffer) {
mp = curbuf->b_first_abbr;
} else {
mp = first_abbr;
}
} else if (exp_buffer) {
mp = curbuf->b_maphash[hash];
} else {
mp = maphash[hash];
}
for (; mp; mp = mp->m_next) {
if ((mp->m_mode & mode)
&& strstr((char *)mp->m_str, rhs) != NULL) {
return true;
}
}
}
if (exp_buffer) {
break;
}
exp_buffer = true;
}

return false;
}




static int expand_mapmodes = 0;
static int expand_isabbrev = 0;
static int expand_buffer = FALSE;





char_u *
set_context_in_map_cmd (
expand_T *xp,
char_u *cmd,
char_u *arg,
int forceit, 
int isabbrev, 
int isunmap, 
cmdidx_T cmdidx
)
{
if (forceit && cmdidx != CMD_map && cmdidx != CMD_unmap)
xp->xp_context = EXPAND_NOTHING;
else {
if (isunmap)
expand_mapmodes = get_map_mode(&cmd, forceit || isabbrev);
else {
expand_mapmodes = INSERT + CMDLINE;
if (!isabbrev)
expand_mapmodes += VISUAL + SELECTMODE + NORMAL + OP_PENDING;
}
expand_isabbrev = isabbrev;
xp->xp_context = EXPAND_MAPPINGS;
expand_buffer = FALSE;
for (;; ) {
if (STRNCMP(arg, "<buffer>", 8) == 0) {
expand_buffer = TRUE;
arg = skipwhite(arg + 8);
continue;
}
if (STRNCMP(arg, "<unique>", 8) == 0) {
arg = skipwhite(arg + 8);
continue;
}
if (STRNCMP(arg, "<nowait>", 8) == 0) {
arg = skipwhite(arg + 8);
continue;
}
if (STRNCMP(arg, "<silent>", 8) == 0) {
arg = skipwhite(arg + 8);
continue;
}
if (STRNCMP(arg, "<special>", 9) == 0) {
arg = skipwhite(arg + 9);
continue;
}
if (STRNCMP(arg, "<script>", 8) == 0) {
arg = skipwhite(arg + 8);
continue;
}
if (STRNCMP(arg, "<expr>", 6) == 0) {
arg = skipwhite(arg + 6);
continue;
}
break;
}
xp->xp_pattern = arg;
}

return NULL;
}




int ExpandMappings(regmatch_T *regmatch, int *num_file, char_u ***file)
{
mapblock_T *mp;
int hash;
int count;
int round;
char_u *p;
int i;

validate_maphash();

*num_file = 0; 
*file = NULL;





for (round = 1; round <= 2; ++round) {
count = 0;

for (i = 0; i < 7; i++) {
if (i == 0) {
p = (char_u *)"<silent>";
} else if (i == 1) {
p = (char_u *)"<unique>";
} else if (i == 2) {
p = (char_u *)"<script>";
} else if (i == 3) {
p = (char_u *)"<expr>";
} else if (i == 4 && !expand_buffer) {
p = (char_u *)"<buffer>";
} else if (i == 5) {
p = (char_u *)"<nowait>";
} else if (i == 6) {
p = (char_u *)"<special>";
} else {
continue;
}

if (vim_regexec(regmatch, p, (colnr_T)0)) {
if (round == 1)
++count;
else
(*file)[count++] = vim_strsave(p);
}
}

for (hash = 0; hash < 256; ++hash) {
if (expand_isabbrev) {
if (hash > 0) 
break; 
mp = first_abbr;
} else if (expand_buffer)
mp = curbuf->b_maphash[hash];
else
mp = maphash[hash];
for (; mp; mp = mp->m_next) {
if (mp->m_mode & expand_mapmodes) {
p = translate_mapping(mp->m_keys, CPO_TO_CPO_FLAGS);
if (p != NULL && vim_regexec(regmatch, p, (colnr_T)0)) {
if (round == 1)
++count;
else {
(*file)[count++] = p;
p = NULL;
}
}
xfree(p);
}
} 
} 

if (count == 0) 
break; 

if (round == 1) {
*file = (char_u **)xmalloc((size_t)count * sizeof(char_u *));
}
} 

if (count > 1) {
char_u **ptr1;
char_u **ptr2;
char_u **ptr3;


sort_strings(*file, count);


ptr1 = *file;
ptr2 = ptr1 + 1;
ptr3 = ptr1 + count;

while (ptr2 < ptr3) {
if (STRCMP(*ptr1, *ptr2))
*++ptr1 = *ptr2++;
else {
xfree(*ptr2++);
count--;
}
}
}

*num_file = count;
return count == 0 ? FAIL : OK;
}



















int check_abbr(int c, char_u *ptr, int col, int mincol)
{
int len;
int scol; 
int j;
char_u *s;
char_u tb[MB_MAXBYTES + 4];
mapblock_T *mp;
mapblock_T *mp2;
int clen = 0; 
int is_id = TRUE;
int vim_abbr;

if (typebuf.tb_no_abbr_cnt) 
return FALSE;


if ((KeyNoremap & (RM_NONE|RM_SCRIPT)) != 0 && c != Ctrl_RSB)
return FALSE;







if (col == 0) 
return FALSE;

if (has_mbyte) {
char_u *p;

p = mb_prevptr(ptr, ptr + col);
if (!vim_iswordp(p))
vim_abbr = TRUE; 
else {
vim_abbr = FALSE; 
if (p > ptr)
is_id = vim_iswordp(mb_prevptr(ptr, p));
}
clen = 1;
while (p > ptr + mincol) {
p = mb_prevptr(ptr, p);
if (ascii_isspace(*p) || (!vim_abbr && is_id != vim_iswordp(p))) {
p += (*mb_ptr2len)(p);
break;
}
++clen;
}
scol = (int)(p - ptr);
} else {
if (!vim_iswordc(ptr[col - 1]))
vim_abbr = TRUE; 
else {
vim_abbr = FALSE; 
if (col > 1)
is_id = vim_iswordc(ptr[col - 2]);
}
for (scol = col - 1; scol > 0 && !ascii_isspace(ptr[scol - 1])
&& (vim_abbr || is_id == vim_iswordc(ptr[scol - 1])); --scol)
;
}

if (scol < mincol)
scol = mincol;
if (scol < col) { 
ptr += scol;
len = col - scol;
mp = curbuf->b_first_abbr;
mp2 = first_abbr;
if (mp == NULL) {
mp = mp2;
mp2 = NULL;
}
for (; mp;
mp->m_next == NULL ? (mp = mp2, mp2 = NULL) :
(mp = mp->m_next)) {
int qlen = mp->m_keylen;
char_u *q = mp->m_keys;
int match;

if (strchr((const char *)mp->m_keys, K_SPECIAL) != NULL) {

q = vim_strsave(mp->m_keys);
vim_unescape_csi(q);
qlen = (int)STRLEN(q);
}

match = (mp->m_mode & State)
&& qlen == len
&& !STRNCMP(q, ptr, (size_t)len);
if (q != mp->m_keys) {
xfree(q);
}
if (match) {
break;
}
}
if (mp != NULL) {













j = 0;
if (c != Ctrl_RSB) {

if (IS_SPECIAL(c) || c == K_SPECIAL) {
tb[j++] = K_SPECIAL;
tb[j++] = (char_u)K_SECOND(c);
tb[j++] = (char_u)K_THIRD(c);
} else {
if (c < ABBR_OFF && (c < ' ' || c > '~')) {
tb[j++] = Ctrl_V; 
}

if (c >= ABBR_OFF) {
c -= ABBR_OFF;
}
j += utf_char2bytes(c, tb + j);
}
tb[j] = NUL;

(void)ins_typebuf(tb, 1, 0, TRUE, mp->m_silent);
}
if (mp->m_expr)
s = eval_map_expr(mp->m_str, c);
else
s = mp->m_str;
if (s != NULL) {

(void)ins_typebuf(s, mp->m_noremap, 0, TRUE, mp->m_silent);

typebuf.tb_no_abbr_cnt += (int)STRLEN(s) + j + 1;
if (mp->m_expr)
xfree(s);
}

tb[0] = Ctrl_H;
tb[1] = NUL;
if (has_mbyte)
len = clen; 
while (len-- > 0) 
(void)ins_typebuf(tb, 1, 0, TRUE, mp->m_silent);
return TRUE;
}
}
return FALSE;
}





static char_u *
eval_map_expr (
char_u *str,
int c 
)
{
char_u *res;
char_u *p;
char_u *expr;
char_u *save_cmd;
pos_T save_cursor;
int save_msg_col;
int save_msg_row;



expr = vim_strsave(str);
vim_unescape_csi(expr);

save_cmd = save_cmdline_alloc();



++textlock;
++ex_normal_lock;
set_vim_var_char(c); 
save_cursor = curwin->w_cursor;
save_msg_col = msg_col;
save_msg_row = msg_row;
p = eval_to_string(expr, NULL, FALSE);
--textlock;
--ex_normal_lock;
curwin->w_cursor = save_cursor;
msg_col = save_msg_col;
msg_row = save_msg_row;

restore_cmdline_alloc(save_cmd);
xfree(expr);

if (p == NULL)
return NULL;

res = vim_strsave_escape_csi(p);
xfree(p);

return res;
}





char_u *vim_strsave_escape_csi(char_u *p)
{



char_u *res = xmalloc(STRLEN(p) * 4 + 1);
char_u *d = res;
for (char_u *s = p; *s != NUL; ) {
if (s[0] == K_SPECIAL && s[1] != NUL && s[2] != NUL) {

*d++ = *s++;
*d++ = *s++;
*d++ = *s++;
} else {


d = add_char2buf(PTR2CHAR(s), d);
s += MB_CPTR2LEN(s);
}
}
*d = NUL;

return res;
}





void vim_unescape_csi(char_u *p)
{
char_u *s = p, *d = p;

while (*s != NUL) {
if (s[0] == K_SPECIAL && s[1] == KS_SPECIAL && s[2] == KE_FILLER) {
*d++ = K_SPECIAL;
s += 3;
} else if ((s[0] == K_SPECIAL || s[0] == CSI)
&& s[1] == KS_EXTRA && s[2] == (int)KE_CSI) {
*d++ = CSI;
s += 3;
} else
*d++ = *s++;
}
*d = NUL;
}





int
makemap(
FILE *fd,
buf_T *buf 
)
{
mapblock_T *mp;
char_u c1, c2, c3;
char_u *p;
char *cmd;
int abbr;
int hash;
bool did_cpo = false;

validate_maphash();



for (abbr = 0; abbr < 2; abbr++) {
for (hash = 0; hash < 256; hash++) {
if (abbr) {
if (hash > 0) { 
break;
}
if (buf != NULL) {
mp = buf->b_first_abbr;
} else {
mp = first_abbr;
}
} else {
if (buf != NULL) {
mp = buf->b_maphash[hash];
} else {
mp = maphash[hash];
}
}

for (; mp; mp = mp->m_next) {

if (mp->m_noremap == REMAP_SCRIPT) {
continue;
}



for (p = mp->m_str; *p != NUL; p++) {
if (p[0] == K_SPECIAL && p[1] == KS_EXTRA
&& p[2] == (int)KE_SNR) {
break;
}
}
if (*p != NUL) {
continue;
}




c1 = NUL;
c2 = NUL;
c3 = NUL;
if (abbr) {
cmd = "abbr";
} else {
cmd = "map";
}
switch (mp->m_mode) {
case NORMAL + VISUAL + SELECTMODE + OP_PENDING:
break;
case NORMAL:
c1 = 'n';
break;
case VISUAL:
c1 = 'x';
break;
case SELECTMODE:
c1 = 's';
break;
case OP_PENDING:
c1 = 'o';
break;
case NORMAL + VISUAL:
c1 = 'n';
c2 = 'x';
break;
case NORMAL + SELECTMODE:
c1 = 'n';
c2 = 's';
break;
case NORMAL + OP_PENDING:
c1 = 'n';
c2 = 'o';
break;
case VISUAL + SELECTMODE:
c1 = 'v';
break;
case VISUAL + OP_PENDING:
c1 = 'x';
c2 = 'o';
break;
case SELECTMODE + OP_PENDING:
c1 = 's';
c2 = 'o';
break;
case NORMAL + VISUAL + SELECTMODE:
c1 = 'n';
c2 = 'v';
break;
case NORMAL + VISUAL + OP_PENDING:
c1 = 'n';
c2 = 'x';
c3 = 'o';
break;
case NORMAL + SELECTMODE + OP_PENDING:
c1 = 'n';
c2 = 's';
c3 = 'o';
break;
case VISUAL + SELECTMODE + OP_PENDING:
c1 = 'v';
c2 = 'o';
break;
case CMDLINE + INSERT:
if (!abbr) {
cmd = "map!";
}
break;
case CMDLINE:
c1 = 'c';
break;
case INSERT:
c1 = 'i';
break;
case LANGMAP:
c1 = 'l';
break;
case TERM_FOCUS:
c1 = 't';
break;
default:
IEMSG(_("E228: makemap: Illegal mode"));
return FAIL;
}
do {



if (!did_cpo) {
if (*mp->m_str == NUL) { 
did_cpo = true;
} else {
const char specials[] = { (char)(uint8_t)K_SPECIAL, NL, NUL };
if (strpbrk((const char *)mp->m_str, specials) != NULL
|| strpbrk((const char *)mp->m_keys, specials) != NULL) {
did_cpo = true;
}
}
if (did_cpo) {
if (fprintf(fd, "let s:cpo_save=&cpo") < 0
|| put_eol(fd) < 0
|| fprintf(fd, "set cpo&vim") < 0
|| put_eol(fd) < 0) {
return FAIL;
}
}
}
if (c1 && putc(c1, fd) < 0) {
return FAIL;
}
if (mp->m_noremap != REMAP_YES && fprintf(fd, "nore") < 0) {
return FAIL;
}
if (fputs(cmd, fd) < 0) {
return FAIL;
}
if (buf != NULL && fputs(" <buffer>", fd) < 0) {
return FAIL;
}
if (mp->m_nowait && fputs(" <nowait>", fd) < 0) {
return FAIL;
}
if (mp->m_silent && fputs(" <silent>", fd) < 0) {
return FAIL;
}
if (mp->m_expr && fputs(" <expr>", fd) < 0) {
return FAIL;
}

if (putc(' ', fd) < 0
|| put_escstr(fd, mp->m_keys, 0) == FAIL
|| putc(' ', fd) < 0
|| put_escstr(fd, mp->m_str, 1) == FAIL
|| put_eol(fd) < 0) {
return FAIL;
}
c1 = c2;
c2 = c3;
c3 = NUL;
} while (c1 != NUL);
}
}
}
if (did_cpo) {
if (fprintf(fd, "let &cpo=s:cpo_save") < 0
|| put_eol(fd) < 0
|| fprintf(fd, "unlet s:cpo_save") < 0
|| put_eol(fd) < 0) {
return FAIL;
}
}
return OK;
}





int put_escstr(FILE *fd, char_u *strstart, int what)
{
char_u *str = strstart;
int c;
int modifiers;


if (*str == NUL && what == 1) {
if (fprintf(fd, "<Nop>") < 0)
return FAIL;
return OK;
}

for (; *str != NUL; str++) {


const char *p = mb_unescape((const char **)&str);
if (p != NULL) {
while (*p != NUL)
if (fputc(*p++, fd) < 0)
return FAIL;
--str;
continue;
}

c = *str;




if (c == K_SPECIAL && what != 2) {
modifiers = 0x0;
if (str[1] == KS_MODIFIER) {
modifiers = str[2];
str += 3;
c = *str;
}
if (c == K_SPECIAL) {
c = TO_SPECIAL(str[1], str[2]);
str += 2;
}
if (IS_SPECIAL(c) || modifiers) { 
if (fputs((char *)get_special_key_name(c, modifiers), fd) < 0)
return FAIL;
continue;
}
}





if (c == NL) {
if (what == 2) {
if (fprintf(fd, "\\\026\n") < 0)
return FAIL;
} else {
if (fprintf(fd, "<NL>") < 0)
return FAIL;
}
continue;
}












if (what == 2 && (ascii_iswhite(c) || c == '"' || c == '\\')) {
if (putc('\\', fd) < 0)
return FAIL;
} else if (c < ' ' || c > '~' || c == '|'
|| (what == 0 && c == ' ')
|| (what == 1 && str == strstart && c == ' ')
|| (what != 2 && c == '<')) {
if (putc(Ctrl_V, fd) < 0)
return FAIL;
}
if (putc(c, fd) < 0)
return FAIL;
}
return OK;
}






char_u *
check_map (
char_u *keys,
int mode,
int exact, 
int ign_mod, 
int abbr, 
mapblock_T **mp_ptr, 
int *local_ptr 
)
{
int hash;
int len, minlen;
mapblock_T *mp;
int local;

validate_maphash();

len = (int)STRLEN(keys);
for (local = 1; local >= 0; --local)

for (hash = 0; hash < 256; ++hash) {
if (abbr) {
if (hash > 0) 
break;
if (local)
mp = curbuf->b_first_abbr;
else
mp = first_abbr;
} else if (local)
mp = curbuf->b_maphash[hash];
else
mp = maphash[hash];
for (; mp != NULL; mp = mp->m_next) {


if ((mp->m_mode & mode) && (!exact || mp->m_keylen == len)) {
char_u *s = mp->m_keys;
int keylen = mp->m_keylen;
if (ign_mod && keylen >= 3
&& s[0] == K_SPECIAL && s[1] == KS_MODIFIER) {
s += 3;
keylen -= 3;
}
minlen = keylen < len ? keylen : len;
if (STRNCMP(s, keys, minlen) == 0) {
if (mp_ptr != NULL)
*mp_ptr = mp;
if (local_ptr != NULL)
*local_ptr = local;
return mp->m_str;
}
}
}
}

return NULL;
}






void add_map(char_u *map, int mode)
{
char_u *s;
char_u *cpo_save = p_cpo;

p_cpo = (char_u *)""; 
s = vim_strsave(map);
(void)do_map(0, s, mode, FALSE);
xfree(s);
p_cpo = cpo_save;
}












static char_u * translate_mapping (
char_u *str,
int cpo_flags 
)
{
garray_T ga;
ga_init(&ga, 1, 40);

bool cpo_bslash = !(cpo_flags&FLAG_CPO_BSLASH);

for (; *str; ++str) {
int c = *str;
if (c == K_SPECIAL && str[1] != NUL && str[2] != NUL) {
int modifiers = 0;
if (str[1] == KS_MODIFIER) {
str++;
modifiers = *++str;
c = *++str;
}

if (c == K_SPECIAL && str[1] != NUL && str[2] != NUL) {
c = TO_SPECIAL(str[1], str[2]);
if (c == K_ZERO) {

c = NUL;
}
str += 2;
}
if (IS_SPECIAL(c) || modifiers) { 
ga_concat(&ga, get_special_key_name(c, modifiers));
continue; 
}
}

if (c == ' ' || c == '\t' || c == Ctrl_J || c == Ctrl_V
|| (c == '\\' && !cpo_bslash)) {
ga_append(&ga, cpo_bslash ? Ctrl_V : '\\');
}

if (c) {
ga_append(&ga, (char)c);
}
}
ga_append(&ga, NUL);
return (char_u *)(ga.ga_data);
}

static bool typebuf_match_len(const uint8_t *str, int *mlen)
{
int i;
for (i = 0; i < typebuf.tb_len && str[i]; i++) {
if (str[i] != typebuf.tb_buf[typebuf.tb_off + i])
break;
}
*mlen = i;
return str[i] == NUL; 
}





mapblock_T *get_maphash(int index, buf_T *buf)
FUNC_ATTR_PURE
{
if (index >= MAX_MAPHASH) {
return NULL;
}

return (buf == NULL) ? maphash[index] : buf->b_maphash[index];
}


char_u * getcmdkeycmd(int promptc, void *cookie, int indent, bool do_concat)
{
garray_T line_ga;
int c1 = -1, c2;
int cmod = 0;
bool aborted = false;

ga_init(&line_ga, 1, 32);

no_mapping++;

got_int = false;
while (c1 != NUL && !aborted) {
ga_grow(&line_ga, 32);

if (vgetorpeek(false) == NUL) {


EMSG(e_cmdmap_err);
aborted = true;
break;
}


c1 = vgetorpeek(true);

if (c1 == K_SPECIAL) {
c1 = vgetorpeek(true); 
c2 = vgetorpeek(true);
if (c1 == KS_MODIFIER) {
cmod = c2;
continue;
}
c1 = TO_SPECIAL(c1, c2);
}


if (got_int) {
aborted = true;
} else if (c1 == '\r' || c1 == '\n') {
c1 = NUL; 
} else if (c1 == ESC) {
aborted = true;
} else if (c1 == K_COMMAND) {

EMSG(e_cmdmap_repeated);
aborted = true;
} else if (IS_SPECIAL(c1)) {
if (c1 == K_SNR) {
ga_append(&line_ga, (char)K_SPECIAL);
ga_append(&line_ga, (char)KS_EXTRA);
ga_append(&line_ga, (char)KE_SNR);
} else {
EMSG2(e_cmdmap_key, get_special_key_name(c1, cmod));
aborted = true;
}
} else {
ga_append(&line_ga, (char)c1);
}

cmod = 0;
}

no_mapping--;

if (aborted) {
ga_clear(&line_ga);
}

return (char_u *)line_ga.ga_data;
}
