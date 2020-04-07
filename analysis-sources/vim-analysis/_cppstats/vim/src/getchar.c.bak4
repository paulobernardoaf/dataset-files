

















#include "vim.h"





















#define MINIMAL_SIZE 20 

static buffheader_T redobuff = {{NULL, {NUL}}, NULL, 0, 0};
static buffheader_T old_redobuff = {{NULL, {NUL}}, NULL, 0, 0};
static buffheader_T recordbuff = {{NULL, {NUL}}, NULL, 0, 0};

static int typeahead_char = 0; 





static int block_redo = FALSE;

static int KeyNoremap = 0; 




















#define RM_YES 0 
#define RM_NONE 1 
#define RM_SCRIPT 2 
#define RM_ABBR 4 




#define TYPELEN_INIT (5 * (MAXMAPLEN + 3))
static char_u typebuf_init[TYPELEN_INIT]; 
static char_u noremapbuf_init[TYPELEN_INIT]; 

static int last_recorded_len = 0; 

static int read_readbuf(buffheader_T *buf, int advance);
static void init_typebuf(void);
static void may_sync_undo(void);
static void free_typebuf(void);
static void closescript(void);
static void updatescript(int c);
static int vgetorpeek(int);
static int inchar(char_u *buf, int maxlen, long wait_time);




void
free_buff(buffheader_T *buf)
{
buffblock_T *p, *np;

for (p = buf->bh_first.b_next; p != NULL; p = np)
{
np = p->b_next;
vim_free(p);
}
buf->bh_first.b_next = NULL;
}





static char_u *
get_buffcont(
buffheader_T *buffer,
int dozero) 
{
long_u count = 0;
char_u *p = NULL;
char_u *p2;
char_u *str;
buffblock_T *bp;


for (bp = buffer->bh_first.b_next; bp != NULL; bp = bp->b_next)
count += (long_u)STRLEN(bp->b_str);

if ((count || dozero) && (p = alloc(count + 1)) != NULL)
{
p2 = p;
for (bp = buffer->bh_first.b_next; bp != NULL; bp = bp->b_next)
for (str = bp->b_str; *str; )
*p2++ = *str++;
*p2 = NUL;
}
return (p);
}






char_u *
get_recorded(void)
{
char_u *p;
size_t len;

p = get_buffcont(&recordbuff, TRUE);
free_buff(&recordbuff);





len = STRLEN(p);
if ((int)len >= last_recorded_len)
{
len -= last_recorded_len;
p[len] = NUL;
}





if (len > 0 && restart_edit != 0 && p[len - 1] == Ctrl_O)
p[len - 1] = NUL;

return (p);
}





char_u *
get_inserted(void)
{
return get_buffcont(&redobuff, FALSE);
}





static void
add_buff(
buffheader_T *buf,
char_u *s,
long slen) 
{
buffblock_T *p;
long_u len;

if (slen < 0)
slen = (long)STRLEN(s);
if (slen == 0) 
return;

if (buf->bh_first.b_next == NULL) 
{
buf->bh_space = 0;
buf->bh_curr = &(buf->bh_first);
}
else if (buf->bh_curr == NULL) 
{
iemsg(_("E222: Add to read buffer"));
return;
}
else if (buf->bh_index != 0)
mch_memmove(buf->bh_first.b_next->b_str,
buf->bh_first.b_next->b_str + buf->bh_index,
STRLEN(buf->bh_first.b_next->b_str + buf->bh_index) + 1);
buf->bh_index = 0;

if (buf->bh_space >= (int)slen)
{
len = (long_u)STRLEN(buf->bh_curr->b_str);
vim_strncpy(buf->bh_curr->b_str + len, s, (size_t)slen);
buf->bh_space -= slen;
}
else
{
if (slen < MINIMAL_SIZE)
len = MINIMAL_SIZE;
else
len = slen;
p = alloc(offsetof(buffblock_T, b_str) + len + 1);
if (p == NULL)
return; 
buf->bh_space = (int)(len - slen);
vim_strncpy(p->b_str, s, (size_t)slen);

p->b_next = buf->bh_curr->b_next;
buf->bh_curr->b_next = p;
buf->bh_curr = p;
}
return;
}




static void
add_num_buff(buffheader_T *buf, long n)
{
char_u number[32];

sprintf((char *)number, "%ld", n);
add_buff(buf, number, -1L);
}





static void
add_char_buff(buffheader_T *buf, int c)
{
char_u bytes[MB_MAXBYTES + 1];
int len;
int i;
char_u temp[4];

if (IS_SPECIAL(c))
len = 1;
else
len = (*mb_char2bytes)(c, bytes);
for (i = 0; i < len; ++i)
{
if (!IS_SPECIAL(c))
c = bytes[i];

if (IS_SPECIAL(c) || c == K_SPECIAL || c == NUL)
{

temp[0] = K_SPECIAL;
temp[1] = K_SECOND(c);
temp[2] = K_THIRD(c);
temp[3] = NUL;
}
#if defined(FEAT_GUI)
else if (c == CSI)
{

temp[0] = CSI;
temp[1] = KS_EXTRA;
temp[2] = (int)KE_CSI;
temp[3] = NUL;
}
#endif
else
{
temp[0] = c;
temp[1] = NUL;
}
add_buff(buf, temp, -1L);
}
}


static buffheader_T readbuf1 = {{NULL, {NUL}}, NULL, 0, 0};


static buffheader_T readbuf2 = {{NULL, {NUL}}, NULL, 0, 0};







static int
read_readbuffers(int advance)
{
int c;

c = read_readbuf(&readbuf1, advance);
if (c == NUL)
c = read_readbuf(&readbuf2, advance);
return c;
}

static int
read_readbuf(buffheader_T *buf, int advance)
{
char_u c;
buffblock_T *curr;

if (buf->bh_first.b_next == NULL) 
return NUL;

curr = buf->bh_first.b_next;
c = curr->b_str[buf->bh_index];

if (advance)
{
if (curr->b_str[++buf->bh_index] == NUL)
{
buf->bh_first.b_next = curr->b_next;
vim_free(curr);
buf->bh_index = 0;
}
}
return c;
}




static void
start_stuff(void)
{
if (readbuf1.bh_first.b_next != NULL)
{
readbuf1.bh_curr = &(readbuf1.bh_first);
readbuf1.bh_space = 0;
}
if (readbuf2.bh_first.b_next != NULL)
{
readbuf2.bh_curr = &(readbuf2.bh_first);
readbuf2.bh_space = 0;
}
}




int
stuff_empty(void)
{
return (readbuf1.bh_first.b_next == NULL
&& readbuf2.bh_first.b_next == NULL);
}

#if defined(FEAT_EVAL) || defined(PROTO)




int
readbuf1_empty(void)
{
return (readbuf1.bh_first.b_next == NULL);
}
#endif




void
typeahead_noflush(int c)
{
typeahead_char = c;
}






void
flush_buffers(flush_buffers_T flush_typeahead)
{
init_typebuf();

start_stuff();
while (read_readbuffers(TRUE) != NUL)
;

if (flush_typeahead == FLUSH_MINIMAL)
{

typebuf.tb_off += typebuf.tb_maplen;
typebuf.tb_len -= typebuf.tb_maplen;
#if defined(FEAT_CLIENTSERVER) || defined(FEAT_EVAL)
if (typebuf.tb_len == 0)
typebuf_was_filled = FALSE;
#endif
}
else
{

if (flush_typeahead == FLUSH_INPUT)



while (inchar(typebuf.tb_buf, typebuf.tb_buflen - 1, 10L) != 0)
;
typebuf.tb_off = MAXMAPLEN;
typebuf.tb_len = 0;
#if defined(FEAT_CLIENTSERVER) || defined(FEAT_EVAL)


typebuf_was_filled = FALSE;
#endif
}
typebuf.tb_maplen = 0;
typebuf.tb_silent = 0;
cmd_silent = FALSE;
typebuf.tb_no_abbr_cnt = 0;
}





void
ResetRedobuff(void)
{
if (!block_redo)
{
free_buff(&old_redobuff);
old_redobuff = redobuff;
redobuff.bh_first.b_next = NULL;
}
}





void
CancelRedo(void)
{
if (!block_redo)
{
free_buff(&redobuff);
redobuff = old_redobuff;
old_redobuff.bh_first.b_next = NULL;
start_stuff();
while (read_readbuffers(TRUE) != NUL)
;
}
}





void
saveRedobuff(save_redo_T *save_redo)
{
char_u *s;

save_redo->sr_redobuff = redobuff;
redobuff.bh_first.b_next = NULL;
save_redo->sr_old_redobuff = old_redobuff;
old_redobuff.bh_first.b_next = NULL;


s = get_buffcont(&save_redo->sr_redobuff, FALSE);
if (s != NULL)
{
add_buff(&redobuff, s, -1L);
vim_free(s);
}
}





void
restoreRedobuff(save_redo_T *save_redo)
{
free_buff(&redobuff);
redobuff = save_redo->sr_redobuff;
free_buff(&old_redobuff);
old_redobuff = save_redo->sr_old_redobuff;
}





void
AppendToRedobuff(char_u *s)
{
if (!block_redo)
add_buff(&redobuff, s, -1L);
}





void
AppendToRedobuffLit(
char_u *str,
int len) 
{
char_u *s = str;
int c;
char_u *start;

if (block_redo)
return;

while (len < 0 ? *s != NUL : s - str < len)
{


start = s;
while (*s >= ' '
#if !defined(EBCDIC)
&& *s < DEL 
#endif
&& (len < 0 || s - str < len))
++s;



if (*s == NUL && (s[-1] == '0' || s[-1] == '^'))
--s;
if (s > start)
add_buff(&redobuff, start, (long)(s - start));

if (*s == NUL || (len >= 0 && s - str >= len))
break;


if (has_mbyte)

c = mb_cptr2char_adv(&s);
else
c = *s++;
if (c < ' ' || c == DEL || (*s == NUL && (c == '0' || c == '^')))
add_char_buff(&redobuff, Ctrl_V);


if (*s == NUL && c == '0')
#if defined(EBCDIC)
add_buff(&redobuff, (char_u *)"xf0", 3L);
#else
add_buff(&redobuff, (char_u *)"048", 3L);
#endif
else
add_char_buff(&redobuff, c);
}
}





void
AppendCharToRedobuff(int c)
{
if (!block_redo)
add_char_buff(&redobuff, c);
}




void
AppendNumberToRedobuff(long n)
{
if (!block_redo)
add_num_buff(&redobuff, n);
}





void
stuffReadbuff(char_u *s)
{
add_buff(&readbuf1, s, -1L);
}





void
stuffRedoReadbuff(char_u *s)
{
add_buff(&readbuf2, s, -1L);
}

void
stuffReadbuffLen(char_u *s, long len)
{
add_buff(&readbuf1, s, len);
}

#if defined(FEAT_EVAL) || defined(PROTO)





void
stuffReadbuffSpec(char_u *s)
{
int c;

while (*s != NUL)
{
if (*s == K_SPECIAL && s[1] != NUL && s[2] != NUL)
{

stuffReadbuffLen(s, 3L);
s += 3;
}
else
{
c = mb_ptr2char_adv(&s);
if (c == CAR || c == NL || c == ESC)
c = ' ';
stuffcharReadbuff(c);
}
}
}
#endif





void
stuffcharReadbuff(int c)
{
add_char_buff(&readbuf1, c);
}




void
stuffnumReadbuff(long n)
{
add_num_buff(&readbuf1, n);
}









static int
read_redo(int init, int old_redo)
{
static buffblock_T *bp;
static char_u *p;
int c;
int n;
char_u buf[MB_MAXBYTES + 1];
int i;

if (init)
{
if (old_redo)
bp = old_redobuff.bh_first.b_next;
else
bp = redobuff.bh_first.b_next;
if (bp == NULL)
return FAIL;
p = bp->b_str;
return OK;
}
if ((c = *p) != NUL)
{



if (has_mbyte && (c != K_SPECIAL || p[1] == KS_SPECIAL))
n = MB_BYTE2LEN_CHECK(c);
else
n = 1;
for (i = 0; ; ++i)
{
if (c == K_SPECIAL) 
{
c = TO_SPECIAL(p[1], p[2]);
p += 2;
}
#if defined(FEAT_GUI)
if (c == CSI) 
p += 2;
#endif
if (*++p == NUL && bp->b_next != NULL)
{
bp = bp->b_next;
p = bp->b_str;
}
buf[i] = c;
if (i == n - 1) 
{
if (n != 1)
c = (*mb_ptr2char)(buf);
break;
}
c = *p;
if (c == NUL) 
break;
}
}

return c;
}






static void
copy_redo(int old_redo)
{
int c;

while ((c = read_redo(FALSE, old_redo)) != NUL)
add_char_buff(&readbuf2, c);
}










int
start_redo(long count, int old_redo)
{
int c;


if (read_redo(TRUE, old_redo) == FAIL)
return FAIL;

c = read_redo(FALSE, old_redo);


if (c == '"')
{
add_buff(&readbuf2, (char_u *)"\"", 1L);
c = read_redo(FALSE, old_redo);


if (c >= '1' && c < '9')
++c;
add_char_buff(&readbuf2, c);


if (c == '=')
{
add_char_buff(&readbuf2, CAR);
cmd_silent = TRUE;
}

c = read_redo(FALSE, old_redo);
}

if (c == 'v') 
{
VIsual = curwin->w_cursor;
VIsual_active = TRUE;
VIsual_select = FALSE;
VIsual_reselect = TRUE;
redo_VIsual_busy = TRUE;
c = read_redo(FALSE, old_redo);
}


if (count)
{
while (VIM_ISDIGIT(c)) 
c = read_redo(FALSE, old_redo);
add_num_buff(&readbuf2, count);
}


add_char_buff(&readbuf2, c);
copy_redo(old_redo);
return OK;
}






int
start_redo_ins(void)
{
int c;

if (read_redo(TRUE, FALSE) == FAIL)
return FAIL;
start_stuff();


while ((c = read_redo(FALSE, FALSE)) != NUL)
{
if (vim_strchr((char_u *)"AaIiRrOo", c) != NULL)
{
if (c == 'O' || c == 'o')
add_buff(&readbuf2, NL_STR, -1L);
break;
}
}


copy_redo(FALSE);
block_redo = TRUE;
return OK;
}

void
stop_redo_ins(void)
{
block_redo = FALSE;
}






static void
init_typebuf(void)
{
if (typebuf.tb_buf == NULL)
{
typebuf.tb_buf = typebuf_init;
typebuf.tb_noremap = noremapbuf_init;
typebuf.tb_buflen = TYPELEN_INIT;
typebuf.tb_len = 0;
typebuf.tb_off = MAXMAPLEN + 4;
typebuf.tb_change_cnt = 1;
}
}




int
noremap_keys(void)
{
return KeyNoremap & (RM_NONE|RM_SCRIPT);
}




















int
ins_typebuf(
char_u *str,
int noremap,
int offset,
int nottyped,
int silent)
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
state_no_longer_safe("ins_typebuf()");

addlen = (int)STRLEN(str);

if (offset == 0 && addlen <= typebuf.tb_off)
{



typebuf.tb_off -= addlen;
mch_memmove(typebuf.tb_buf + typebuf.tb_off, str, (size_t)addlen);
}
else if (typebuf.tb_len == 0 && typebuf.tb_buflen
>= addlen + 3 * (MAXMAPLEN + 4))
{




typebuf.tb_off = (typebuf.tb_buflen - addlen - 3 * (MAXMAPLEN + 4)) / 2;
mch_memmove(typebuf.tb_buf + typebuf.tb_off, str, (size_t)addlen);
}
else
{






newoff = MAXMAPLEN + 4;
newlen = typebuf.tb_len + addlen + newoff + 4 * (MAXMAPLEN + 4);
if (newlen < 0) 
{
emsg(_(e_toocompl)); 
setcursor();
return FAIL;
}
s1 = alloc(newlen);
if (s1 == NULL) 
return FAIL;
s2 = alloc(newlen);
if (s2 == NULL) 
{
vim_free(s1);
return FAIL;
}
typebuf.tb_buflen = newlen;


mch_memmove(s1 + newoff, typebuf.tb_buf + typebuf.tb_off,
(size_t)offset);

mch_memmove(s1 + newoff + offset, str, (size_t)addlen);


mch_memmove(s1 + newoff + offset + addlen,
typebuf.tb_buf + typebuf.tb_off + offset,
(size_t)(typebuf.tb_len - offset + 1));
if (typebuf.tb_buf != typebuf_init)
vim_free(typebuf.tb_buf);
typebuf.tb_buf = s1;

mch_memmove(s2 + newoff, typebuf.tb_noremap + typebuf.tb_off,
(size_t)offset);
mch_memmove(s2 + newoff + offset + addlen,
typebuf.tb_noremap + typebuf.tb_off + offset,
(size_t)(typebuf.tb_len - offset));
if (typebuf.tb_noremap != noremapbuf_init)
vim_free(typebuf.tb_noremap);
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
(--nrm >= 0) ? val : RM_YES;




if (nottyped || typebuf.tb_maplen > offset)
typebuf.tb_maplen += addlen;
if (silent || typebuf.tb_silent > offset)
{
typebuf.tb_silent += addlen;
cmd_silent = TRUE;
}
if (typebuf.tb_no_abbr_cnt && offset == 0) 
typebuf.tb_no_abbr_cnt += addlen;

return OK;
}







void
ins_char_typebuf(int c)
{
char_u buf[MB_MAXBYTES + 1];
if (IS_SPECIAL(c))
{
buf[0] = K_SPECIAL;
buf[1] = K_SECOND(c);
buf[2] = K_THIRD(c);
buf[3] = NUL;
}
else
buf[(*mb_char2bytes)(c, buf)] = NUL;
(void)ins_typebuf(buf, KeyNoremap, 0, !KeyTyped, cmd_silent);
}










int
typebuf_changed(
int tb_change_cnt) 
{
return (tb_change_cnt != 0 && (typebuf.tb_change_cnt != tb_change_cnt
#if defined(FEAT_CLIENTSERVER) || defined(FEAT_EVAL)
|| typebuf_was_filled
#endif
));
}





int
typebuf_typed(void)
{
return typebuf.tb_maplen == 0;
}




int
typebuf_maplen(void)
{
return typebuf.tb_maplen;
}




void
del_typebuf(int len, int offset)
{
int i;

if (len == 0)
return; 

typebuf.tb_len -= len;




if (offset == 0 && typebuf.tb_buflen - (typebuf.tb_off + len)
>= 3 * MAXMAPLEN + 3)
typebuf.tb_off += len;



else
{
i = typebuf.tb_off + offset;



if (typebuf.tb_off > MAXMAPLEN)
{
mch_memmove(typebuf.tb_buf + MAXMAPLEN,
typebuf.tb_buf + typebuf.tb_off, (size_t)offset);
mch_memmove(typebuf.tb_noremap + MAXMAPLEN,
typebuf.tb_noremap + typebuf.tb_off, (size_t)offset);
typebuf.tb_off = MAXMAPLEN;
}

mch_memmove(typebuf.tb_buf + typebuf.tb_off + offset,
typebuf.tb_buf + i + len,
(size_t)(typebuf.tb_len - offset + 1));

mch_memmove(typebuf.tb_noremap + typebuf.tb_off + offset,
typebuf.tb_noremap + i + len,
(size_t)(typebuf.tb_len - offset));
}

if (typebuf.tb_maplen > offset) 
{
if (typebuf.tb_maplen < offset + len)
typebuf.tb_maplen = offset;
else
typebuf.tb_maplen -= len;
}
if (typebuf.tb_silent > offset) 
{
if (typebuf.tb_silent < offset + len)
typebuf.tb_silent = offset;
else
typebuf.tb_silent -= len;
}
if (typebuf.tb_no_abbr_cnt > offset) 
{
if (typebuf.tb_no_abbr_cnt < offset + len)
typebuf.tb_no_abbr_cnt = offset;
else
typebuf.tb_no_abbr_cnt -= len;
}

#if defined(FEAT_CLIENTSERVER) || defined(FEAT_EVAL)


typebuf_was_filled = FALSE;
#endif
if (++typebuf.tb_change_cnt == 0)
typebuf.tb_change_cnt = 1;
}





static void
gotchars(char_u *chars, int len)
{
char_u *s = chars;
int i;
static char_u buf[4];
static int buflen = 0;
int todo = len;

while (todo--)
{
buf[buflen++] = *s++;



if (buflen == 1 && buf[0] == K_SPECIAL)
continue;
if (buflen == 2)
continue;
if (buflen == 3 && buf[1] == KS_EXTRA
&& (buf[2] == KE_FOCUSGAINED || buf[2] == KE_FOCUSLOST))
{


buflen = 0;
continue;
}


for (i = 0; i < buflen; ++i)
updatescript(buf[i]);

if (reg_recording != 0)
{
buf[buflen] = NUL;
add_buff(&recordbuff, buf, (long)buflen);

last_recorded_len += buflen;
}
buflen = 0;
}
may_sync_undo();

#if defined(FEAT_EVAL)

debug_did_msg = FALSE;
#endif



++maptick;
}









static void
may_sync_undo(void)
{
if ((!(State & (INSERT + CMDLINE)) || arrow_used)
&& scriptin[curscript] == NULL)
u_sync(FALSE);
}





static int
alloc_typebuf(void)
{
typebuf.tb_buf = alloc(TYPELEN_INIT);
typebuf.tb_noremap = alloc(TYPELEN_INIT);
if (typebuf.tb_buf == NULL || typebuf.tb_noremap == NULL)
{
free_typebuf();
return FAIL;
}
typebuf.tb_buflen = TYPELEN_INIT;
typebuf.tb_off = MAXMAPLEN + 4; 
typebuf.tb_len = 0;
typebuf.tb_maplen = 0;
typebuf.tb_silent = 0;
typebuf.tb_no_abbr_cnt = 0;
if (++typebuf.tb_change_cnt == 0)
typebuf.tb_change_cnt = 1;
#if defined(FEAT_CLIENTSERVER) || defined(FEAT_EVAL)
typebuf_was_filled = FALSE;
#endif
return OK;
}




static void
free_typebuf(void)
{
if (typebuf.tb_buf == typebuf_init)
internal_error("Free typebuf 1");
else
VIM_CLEAR(typebuf.tb_buf);
if (typebuf.tb_noremap == noremapbuf_init)
internal_error("Free typebuf 2");
else
VIM_CLEAR(typebuf.tb_noremap);
}





static typebuf_T saved_typebuf[NSCRIPT];

int
save_typebuf(void)
{
init_typebuf();
saved_typebuf[curscript] = typebuf;

if (alloc_typebuf() == FAIL)
{
closescript();
return FAIL;
}
return OK;
}

static int old_char = -1; 
static int old_mod_mask; 
static int old_mouse_row; 
static int old_mouse_col; 




void
save_typeahead(tasave_T *tp)
{
tp->save_typebuf = typebuf;
tp->typebuf_valid = (alloc_typebuf() == OK);
if (!tp->typebuf_valid)
typebuf = tp->save_typebuf;

tp->old_char = old_char;
tp->old_mod_mask = old_mod_mask;
old_char = -1;

tp->save_readbuf1 = readbuf1;
readbuf1.bh_first.b_next = NULL;
tp->save_readbuf2 = readbuf2;
readbuf2.bh_first.b_next = NULL;
#if defined(USE_INPUT_BUF)
tp->save_inputbuf = get_input_buf();
#endif
}





void
restore_typeahead(tasave_T *tp)
{
if (tp->typebuf_valid)
{
free_typebuf();
typebuf = tp->save_typebuf;
}

old_char = tp->old_char;
old_mod_mask = tp->old_mod_mask;

free_buff(&readbuf1);
readbuf1 = tp->save_readbuf1;
free_buff(&readbuf2);
readbuf2 = tp->save_readbuf2;
#if defined(USE_INPUT_BUF)
set_input_buf(tp->save_inputbuf);
#endif
}




void
openscript(
char_u *name,
int directly) 
{
if (curscript + 1 == NSCRIPT)
{
emsg(_(e_nesting));
return;
}



if (check_secure())
return;

#if defined(FEAT_EVAL)
if (ignore_script)

return;
#endif

if (scriptin[curscript] != NULL) 
++curscript;

expand_env(name, NameBuff, MAXPATHL);
if ((scriptin[curscript] = mch_fopen((char *)NameBuff, READBIN)) == NULL)
{
semsg(_(e_notopen), name);
if (curscript)
--curscript;
return;
}
if (save_typebuf() == FAIL)
return;







if (directly)
{
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
do
{
update_topline_cursor(); 
normal_cmd(&oa, FALSE); 
vpeekc(); 
}
while (scriptin[oldcurscript] != NULL);

State = save_State;
msg_scroll = save_msg_scroll;
restart_edit = save_restart_edit;
p_im = save_insertmode;
finish_op = save_finish_op;
}
}




static void
closescript(void)
{
free_typebuf();
typebuf = saved_typebuf[curscript];

fclose(scriptin[curscript]);
scriptin[curscript] = NULL;
if (curscript > 0)
--curscript;
}

#if defined(EXITFREE) || defined(PROTO)
void
close_all_scripts(void)
{
while (scriptin[0] != NULL)
closescript();
}
#endif




int
using_script(void)
{
return scriptin[curscript] != NULL;
}





void
before_blocking(void)
{
updatescript(0);
#if defined(FEAT_EVAL)
if (may_garbage_collect)
garbage_collect(FALSE);
#endif
}








static void
updatescript(int c)
{
static int count = 0;

if (c && scriptout)
putc(c, scriptout);
if (c == 0 || (p_uc > 0 && ++count >= p_uc))
{
ml_sync_all(c == 0, TRUE);
count = 0;
}
}





int
merge_modifyOtherKeys(int c_arg)
{
int c = c_arg;

if (mod_mask & MOD_MASK_CTRL)
{
if ((c >= '`' && c <= 0x7f) || (c >= '@' && c <= '_'))
{
c &= 0x1f;
mod_mask &= ~MOD_MASK_CTRL;
}
else if (c == '6')
{

c = 0x1e;
mod_mask &= ~MOD_MASK_CTRL;
}
}
if ((mod_mask & (MOD_MASK_META | MOD_MASK_ALT))
&& c >= 0 && c <= 127)
{
c += 0x80;
mod_mask &= ~(MOD_MASK_META|MOD_MASK_ALT);
}
return c;
}










int
vgetc(void)
{
int c, c2;
int n;
char_u buf[MB_MAXBYTES + 1];
int i;

#if defined(FEAT_EVAL)


if (may_garbage_collect && want_garbage_collect)
garbage_collect(FALSE);
#endif





if (old_char != -1)
{
c = old_char;
old_char = -1;
mod_mask = old_mod_mask;
mouse_row = old_mouse_row;
mouse_col = old_mouse_col;
}
else
{
mod_mask = 0x0;
last_recorded_len = 0;
for (;;) 
{
int did_inc = FALSE;

if (mod_mask
#if defined(FEAT_XIM) && defined(FEAT_GUI_GTK)
|| im_is_preediting()
#endif
#if defined(FEAT_PROP_POPUP)
|| popup_no_mapping()
#endif
)
{

++no_mapping;
++allow_keys;
did_inc = TRUE; 
}
c = vgetorpeek(TRUE);
if (did_inc)
{
--no_mapping;
--allow_keys;
}


if (c == K_SPECIAL
#if defined(FEAT_GUI)
|| (c == CSI)
#endif
)
{
int save_allow_keys = allow_keys;

++no_mapping;
allow_keys = 0; 
c2 = vgetorpeek(TRUE); 
c = vgetorpeek(TRUE);
--no_mapping;
allow_keys = save_allow_keys;
if (c2 == KS_MODIFIER)
{
mod_mask = c;
continue;
}
c = TO_SPECIAL(c2, c);

#if defined(FEAT_GUI_MSWIN) && defined(FEAT_MENU) && defined(FEAT_TEAROFF)


if (
#if defined(VIMDLL)
gui.in_use &&
#endif
c == K_TEAROFF)
{
char_u name[200];
int i;


for (i = 0; (c = vgetorpeek(TRUE)) != '\r'; )
{
name[i] = c;
if (i < 199)
++i;
}
name[i] = NUL;
gui_make_tearoff(name);
continue;
}
#endif
#if defined(FEAT_GUI) && defined(FEAT_GUI_GTK) && defined(FEAT_MENU)



if (c == K_F10 && gui.menubar != NULL)
{
gtk_menu_shell_select_first(
GTK_MENU_SHELL(gui.menubar), FALSE);
continue;
}
#endif
#if defined(FEAT_GUI)



if (c == K_FOCUSGAINED || c == K_FOCUSLOST)
{
ui_focus_change(c == K_FOCUSGAINED);
c = K_IGNORE;
}



if (c == K_CSI)
c = CSI;
#endif
}


switch (c)
{
case K_KPLUS: c = '+'; break;
case K_KMINUS: c = '-'; break;
case K_KDIVIDE: c = '/'; break;
case K_KMULTIPLY: c = '*'; break;
case K_KENTER: c = CAR; break;
case K_KPOINT:
#if defined(MSWIN)


c = MapVirtualKey(VK_DECIMAL, 2); break;
#else
c = '.'; break;
#endif
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
case K_ZHOME: if (mod_mask == MOD_MASK_SHIFT)
{
c = K_S_HOME;
mod_mask = 0;
}
else if (mod_mask == MOD_MASK_CTRL)
{
c = K_C_HOME;
mod_mask = 0;
}
else
c = K_HOME;
break;
case K_XEND:
case K_ZEND: if (mod_mask == MOD_MASK_SHIFT)
{
c = K_S_END;
mod_mask = 0;
}
else if (mod_mask == MOD_MASK_CTRL)
{
c = K_C_END;
mod_mask = 0;
}
else
c = K_END;
break;

case K_XUP: c = K_UP; break;
case K_XDOWN: c = K_DOWN; break;
case K_XLEFT: c = K_LEFT; break;
case K_XRIGHT: c = K_RIGHT; break;
}




if (has_mbyte && (n = MB_BYTE2LEN_CHECK(c)) > 1)
{
++no_mapping;
buf[0] = c;
for (i = 1; i < n; ++i)
{
buf[i] = vgetorpeek(TRUE);
if (buf[i] == K_SPECIAL
#if defined(FEAT_GUI)
|| (buf[i] == CSI)
#endif
)
{






c = vgetorpeek(TRUE);
if (vgetorpeek(TRUE) == (int)KE_CSI && c == KS_EXTRA)
buf[i] = CSI;
}
}
--no_mapping;
c = (*mb_ptr2char)(buf);
}

if (!no_reduce_keys)


c = merge_modifyOtherKeys(c);

break;
}
}

#if defined(FEAT_EVAL)





may_garbage_collect = FALSE;
#endif

#if defined(FEAT_BEVAL_TERM)
if (c != K_MOUSEMOVE && c != K_IGNORE && c != K_CURSORHOLD)
{

bevalexpr_due_set = FALSE;
ui_remove_balloon();
}
#endif
#if defined(FEAT_PROP_POPUP)
if (popup_do_filter(c))
{
if (c == Ctrl_C)
got_int = FALSE; 
c = K_IGNORE;
}
#endif



if (c != K_IGNORE)
state_no_longer_safe("key typed");

return c;
}





int
safe_vgetc(void)
{
int c;

c = vgetc();
if (c == NUL)
c = get_keystroke();
return c;
}





int
plain_vgetc(void)
{
int c;

do
c = safe_vgetc();
while (c == K_IGNORE || c == K_VER_SCROLLBAR || c == K_HOR_SCROLLBAR);

if (c == K_PS)


c = bracketed_paste(PASTE_ONE_CHAR, FALSE, NULL);

return c;
}







int
vpeekc(void)
{
if (old_char != -1)
return old_char;
return vgetorpeek(FALSE);
}

#if defined(FEAT_TERMRESPONSE) || defined(FEAT_TERMINAL) || defined(PROTO)




int
vpeekc_nomap(void)
{
int c;

++no_mapping;
++allow_keys;
c = vpeekc();
--no_mapping;
--allow_keys;
return c;
}
#endif






int
vpeekc_any(void)
{
int c;

c = vpeekc();
if (c == NUL && typebuf.tb_len > 0)
c = ESC;
return c;
}





int
char_avail(void)
{
int retval;

#if defined(FEAT_EVAL)


if (disable_char_avail_for_testing)
return FALSE;
#endif
++no_mapping;
retval = vpeekc();
--no_mapping;
return (retval != NUL);
}

#if defined(FEAT_EVAL) || defined(PROTO)



void
f_getchar(typval_T *argvars, typval_T *rettv)
{
varnumber_T n;
int error = FALSE;

#if defined(MESSAGE_QUEUE)



parse_queued_messages();
#endif


windgoto(msg_row, msg_col);

++no_mapping;
++allow_keys;
for (;;)
{
if (argvars[0].v_type == VAR_UNKNOWN)

n = plain_vgetc();
else if (tv_get_number_chk(&argvars[0], &error) == 1)

n = vpeekc_any();
else if (error || vpeekc_any() == NUL)

n = 0;
else

n = plain_vgetc();

if (n == K_IGNORE)
continue;
break;
}
--no_mapping;
--allow_keys;

set_vim_var_nr(VV_MOUSE_WIN, 0);
set_vim_var_nr(VV_MOUSE_WINID, 0);
set_vim_var_nr(VV_MOUSE_LNUM, 0);
set_vim_var_nr(VV_MOUSE_COL, 0);

rettv->vval.v_number = n;
if (IS_SPECIAL(n) || mod_mask != 0)
{
char_u temp[10]; 
int i = 0;


if (mod_mask != 0)
{
temp[i++] = K_SPECIAL;
temp[i++] = KS_MODIFIER;
temp[i++] = mod_mask;
}
if (IS_SPECIAL(n))
{
temp[i++] = K_SPECIAL;
temp[i++] = K_SECOND(n);
temp[i++] = K_THIRD(n);
}
else if (has_mbyte)
i += (*mb_char2bytes)(n, temp + i);
else
temp[i++] = n;
temp[i++] = NUL;
rettv->v_type = VAR_STRING;
rettv->vval.v_string = vim_strsave(temp);

if (is_mouse_key(n))
{
int row = mouse_row;
int col = mouse_col;
win_T *win;
linenr_T lnum;
win_T *wp;
int winnr = 1;

if (row >= 0 && col >= 0)
{


win = mouse_find_win(&row, &col, FIND_POPUP);
if (win == NULL)
return;
(void)mouse_comp_pos(win, &row, &col, &lnum, NULL);
#if defined(FEAT_PROP_POPUP)
if (WIN_IS_POPUP(win))
winnr = 0;
else
#endif
for (wp = firstwin; wp != win && wp != NULL;
wp = wp->w_next)
++winnr;
set_vim_var_nr(VV_MOUSE_WIN, winnr);
set_vim_var_nr(VV_MOUSE_WINID, win->w_id);
set_vim_var_nr(VV_MOUSE_LNUM, lnum);
set_vim_var_nr(VV_MOUSE_COL, col + 1);
}
}
}
}




void
f_getcharmod(typval_T *argvars UNUSED, typval_T *rettv)
{
rettv->vval.v_number = mod_mask;
}
#endif 

#if defined(MESSAGE_QUEUE) || defined(PROTO)
#define MAX_REPEAT_PARSE 8







void
parse_queued_messages(void)
{
int old_curwin_id;
int old_curbuf_fnum;
int i;
int save_may_garbage_collect = may_garbage_collect;
static int entered = 0;
int was_safe = get_was_safe_state();



if (updating_screen)
return;



if (curbuf == NULL || curwin == NULL)
return;

old_curbuf_fnum = curbuf->b_fnum;
old_curwin_id = curwin->w_id;

++entered;





may_garbage_collect = FALSE;


for (i = 0; i < MAX_REPEAT_PARSE; ++i)
{

#if defined(MSWIN) && defined(FEAT_JOB_CHANNEL)
channel_handle_events(FALSE);
#endif

#if defined(FEAT_NETBEANS_INTG)

netbeans_parse_messages();
#endif
#if defined(FEAT_JOB_CHANNEL)

channel_write_any_lines();


channel_parse_messages();
#endif
#if defined(FEAT_CLIENTSERVER) && defined(FEAT_X11)

server_parse_messages();
#endif
#if defined(FEAT_JOB_CHANNEL)


if (job_check_ended())
continue;
#endif
#if defined(FEAT_TERMINAL)
free_unused_terminals();
#endif
#if defined(FEAT_SOUND_CANBERRA)
if (has_sound_callback_in_queue())
invoke_sound_callback();
#endif
break;
}



if (entered == 1 && was_safe)
may_trigger_safestateagain();

may_garbage_collect = save_may_garbage_collect;



if (curwin->w_id != old_curwin_id || curbuf->b_fnum != old_curbuf_fnum)
ins_char_typebuf(K_IGNORE);

--entered;
}
#endif


typedef enum {
map_result_fail, 
map_result_get, 
map_result_retry, 
map_result_nomatch 
} map_result_T;





static int
at_ctrl_x_key(void)
{
char_u *p = typebuf.tb_buf + typebuf.tb_off;
int c = *p;

if (typebuf.tb_len > 3
&& c == K_SPECIAL
&& p[1] == KS_MODIFIER
&& (p[2] & MOD_MASK_CTRL))
c = p[3] & 0x1f;
return vim_is_ctrl_x_key(c);
}








static int
handle_mapping(
int *keylenp,
int *timedout,
int *mapdepth)
{
mapblock_T *mp = NULL;
mapblock_T *mp2;
mapblock_T *mp_match;
int mp_match_len = 0;
int max_mlen = 0;
int tb_c1;
int mlen;
#if defined(FEAT_LANGMAP)
int nolmaplen;
#endif
int keylen = *keylenp;
int i;
int local_State = get_real_state();














tb_c1 = typebuf.tb_buf[typebuf.tb_off];
if (no_mapping == 0 && is_maphash_valid()
&& (no_zero_mapping == 0 || tb_c1 != '0')
&& (typebuf.tb_maplen == 0
|| (p_remap
&& (typebuf.tb_noremap[typebuf.tb_off]
& (RM_NONE|RM_ABBR)) == 0))
&& !(p_paste && (State & (INSERT + CMDLINE)))
&& !(State == HITRETURN && (tb_c1 == CAR || tb_c1 == ' '))
&& State != ASKMORE
&& State != CONFIRM
&& !((ctrl_x_mode_not_default() && at_ctrl_x_key())
|| ((compl_cont_status & CONT_LOCAL)
&& (tb_c1 == Ctrl_N || tb_c1 == Ctrl_P))))
{
#if defined(FEAT_LANGMAP)
if (tb_c1 == K_SPECIAL)
nolmaplen = 2;
else
{
LANGMAP_ADJUST(tb_c1, (State & (CMDLINE | INSERT)) == 0
&& get_real_state() != SELECTMODE);
nolmaplen = 0;
}
#endif

mp = get_buf_maphash_list(local_State, tb_c1);
mp2 = get_maphash_list(local_State, tb_c1);
if (mp == NULL)
{

mp = mp2;
mp2 = NULL;
}








mp_match = NULL;
mp_match_len = 0;
for ( ; mp != NULL;
mp->m_next == NULL ? (mp = mp2, mp2 = NULL) : (mp = mp->m_next))
{



if (mp->m_keys[0] == tb_c1
&& (mp->m_mode & local_State)
&& !(mp->m_simplified && seenModifyOtherKeys)
&& ((mp->m_mode & LANGMAP) == 0 || typebuf.tb_maplen == 0))
{
#if defined(FEAT_LANGMAP)
int nomap = nolmaplen;
int c2;
#endif

for (mlen = 1; mlen < typebuf.tb_len; ++mlen)
{
#if defined(FEAT_LANGMAP)
c2 = typebuf.tb_buf[typebuf.tb_off + mlen];
if (nomap > 0)
--nomap;
else if (c2 == K_SPECIAL)
nomap = 2;
else
LANGMAP_ADJUST(c2, TRUE);
if (mp->m_keys[mlen] != c2)
#else
if (mp->m_keys[mlen] !=
typebuf.tb_buf[typebuf.tb_off + mlen])
#endif
break;
}




{
char_u *p1 = mp->m_keys;
char_u *p2 = mb_unescape(&p1);

if (has_mbyte && p2 != NULL
&& MB_BYTE2LEN(tb_c1) > mb_ptr2len(p2))
mlen = 0;
}




keylen = mp->m_keylen;
if (mlen == keylen || (mlen == typebuf.tb_len
&& typebuf.tb_len < keylen))
{
char_u *s;
int n;



s = typebuf.tb_noremap + typebuf.tb_off;
if (*s == RM_SCRIPT
&& (mp->m_keys[0] != K_SPECIAL
|| mp->m_keys[1] != KS_EXTRA
|| mp->m_keys[2] != (int)KE_SNR))
continue;



for (n = mlen; --n >= 0; )
if (*s++ & (RM_NONE|RM_ABBR))
break;
if (n >= 0)
continue;

if (keylen > typebuf.tb_len)
{
if (!*timedout && !(mp_match != NULL
&& mp_match->m_nowait))
{

keylen = KEYLEN_PART_MAP;
break;
}
}
else if (keylen > mp_match_len)
{

mp_match = mp;
mp_match_len = keylen;
}
}
else


if (max_mlen < mlen)
max_mlen = mlen;
}
}


if (keylen != KEYLEN_PART_MAP)
{
mp = mp_match;
keylen = mp_match_len;
}
}




if (*p_pt != NUL && mp == NULL && (State & (INSERT|NORMAL)))
{
for (mlen = 0; mlen < typebuf.tb_len && p_pt[mlen]; ++mlen)
if (p_pt[mlen] != typebuf.tb_buf[typebuf.tb_off + mlen])
break;
if (p_pt[mlen] == NUL) 
{

if (mlen > typebuf.tb_maplen)
gotchars(typebuf.tb_buf + typebuf.tb_off + typebuf.tb_maplen,
mlen - typebuf.tb_maplen);

del_typebuf(mlen, 0); 
set_option_value((char_u *)"paste", (long)!p_paste, NULL, 0);
if (!(State & INSERT))
{
msg_col = 0;
msg_row = Rows - 1;
msg_clr_eos(); 
}
status_redraw_all();
redraw_statuslines();
showmode();
setcursor();
*keylenp = keylen;
return map_result_retry;
}

if (mlen == typebuf.tb_len)
keylen = KEYLEN_PART_KEY;
else if (max_mlen < mlen)

max_mlen = mlen + 1;
}

if ((mp == NULL || max_mlen >= mp_match_len) && keylen != KEYLEN_PART_MAP)
{
int save_keylen = keylen;










if ((no_mapping == 0 || allow_keys != 0)
&& (typebuf.tb_maplen == 0
|| (p_remap && typebuf.tb_noremap[
typebuf.tb_off] == RM_YES))
&& !*timedout)
{
keylen = check_termcode(max_mlen + 1,
NULL, 0, NULL);



if (keylen == 0 && save_keylen == KEYLEN_PART_KEY)
keylen = KEYLEN_PART_KEY;





if (keylen < 0 && typebuf.tb_len == typebuf.tb_maplen)
keylen = 0;
}
else
keylen = 0;
if (keylen == 0) 
{
#if defined(AMIGA)

if (typebuf.tb_maplen == 0 && (typebuf.tb_buf[
typebuf.tb_off] & 0xff) == CSI)
{
char_u *s;

for (s = typebuf.tb_buf + typebuf.tb_off + 1;
s < typebuf.tb_buf + typebuf.tb_off + typebuf.tb_len
&& (VIM_ISDIGIT(*s) || *s == ';' || *s == ' ');
++s)
;
if (*s == 'r' || *s == '|') 
{
del_typebuf(
(int)(s + 1 - (typebuf.tb_buf + typebuf.tb_off)), 0);

shell_resized();
*keylenp = keylen;
return map_result_retry;
}
if (*s == NUL) 
keylen = KEYLEN_PART_KEY;
}
if (keylen >= 0)
#endif




if (mp == NULL)
{
*keylenp = keylen;
return map_result_get; 
}
}

if (keylen > 0) 
{
#if defined(FEAT_GUI) && defined(FEAT_MENU)
if (typebuf.tb_len >= 2
&& typebuf.tb_buf[typebuf.tb_off] == K_SPECIAL
&& typebuf.tb_buf[typebuf.tb_off + 1] == KS_MENU)
{
int idx;




may_sync_undo();
del_typebuf(3, 0);
idx = get_menu_index(current_menu, local_State);
if (idx != MENU_INDEX_INVALID)
{



if (VIsual_active && VIsual_select
&& (current_menu->modes & VISUAL))
{
VIsual_select = FALSE;
(void)ins_typebuf(K_SELECT_STRING,
REMAP_NONE, 0, TRUE, FALSE);
}
ins_typebuf(current_menu->strings[idx],
current_menu->noremap[idx],
0, TRUE, current_menu->silent[idx]);
}
}
#endif 
*keylenp = keylen;
return map_result_retry; 
}



if (mp == NULL || keylen < 0)
keylen = KEYLEN_PART_KEY;
else
keylen = mp_match_len;
}




if (keylen >= 0 && keylen <= typebuf.tb_len)
{
char_u *map_str;

#if defined(FEAT_EVAL)
int save_m_expr;
int save_m_noremap;
int save_m_silent;
char_u *save_m_keys;
char_u *save_m_str;
#else
#define save_m_noremap mp->m_noremap
#define save_m_silent mp->m_silent
#endif


if (keylen > typebuf.tb_maplen)
gotchars(typebuf.tb_buf + typebuf.tb_off + typebuf.tb_maplen,
keylen - typebuf.tb_maplen);

cmd_silent = (typebuf.tb_silent > 0);
del_typebuf(keylen, 0); 





if (++*mapdepth >= p_mmd)
{
emsg(_("E223: recursive mapping"));
if (State & CMDLINE)
redrawcmdline();
else
setcursor();
flush_buffers(FLUSH_MINIMAL);
*mapdepth = 0; 
*keylenp = keylen;
return map_result_fail;
}





if (VIsual_active && VIsual_select && (mp->m_mode & VISUAL))
{
VIsual_select = FALSE;
(void)ins_typebuf(K_SELECT_STRING, REMAP_NONE, 0, TRUE, FALSE);
}

#if defined(FEAT_EVAL)



save_m_expr = mp->m_expr;
save_m_noremap = mp->m_noremap;
save_m_silent = mp->m_silent;
save_m_keys = NULL; 
save_m_str = NULL; 





if (mp->m_expr)
{
int save_vgetc_busy = vgetc_busy;
int save_may_garbage_collect = may_garbage_collect;
int was_screen_col = screen_cur_col;
int was_screen_row = screen_cur_row;

vgetc_busy = 0;
may_garbage_collect = FALSE;

save_m_keys = vim_strsave(mp->m_keys);
save_m_str = vim_strsave(mp->m_str);
map_str = eval_map_expr(save_m_str, NUL);



windgoto(was_screen_row, was_screen_col);
out_flush();

vgetc_busy = save_vgetc_busy;
may_garbage_collect = save_may_garbage_collect;
}
else
#endif
map_str = mp->m_str;







if (map_str == NULL)
i = FAIL;
else
{
int noremap;

if (save_m_noremap != REMAP_YES)
noremap = save_m_noremap;
else if (
#if defined(FEAT_EVAL)
STRNCMP(map_str, save_m_keys != NULL ? save_m_keys : mp->m_keys,
(size_t)keylen)
#else
STRNCMP(map_str, mp->m_keys, (size_t)keylen)
#endif
!= 0)
noremap = REMAP_YES;
else
noremap = REMAP_SKIP;
i = ins_typebuf(map_str, noremap,
0, TRUE, cmd_silent || save_m_silent);
#if defined(FEAT_EVAL)
if (save_m_expr)
vim_free(map_str);
#endif
}
#if defined(FEAT_EVAL)
vim_free(save_m_keys);
vim_free(save_m_str);
#endif
*keylenp = keylen;
if (i == FAIL)
return map_result_fail;
return map_result_retry;
}

*keylenp = keylen;
return map_result_nomatch;
}




void
vungetc(int c)
{
old_char = c;
old_mod_mask = mod_mask;
old_mouse_row = mouse_row;
old_mouse_col = mouse_col;
}

























static int
vgetorpeek(int advance)
{
int c, c1;
int timedout = FALSE; 

int mapdepth = 0; 
int mode_deleted = FALSE; 
#if defined(FEAT_CMDL_INFO)
int new_wcol, new_wrow;
#endif
#if defined(FEAT_GUI)
int shape_changed = FALSE; 
#endif
int n;
int old_wcol, old_wrow;
int wait_tb_len;












if (vgetc_busy > 0 && ex_normal_busy == 0)
return NUL;

++vgetc_busy;

if (advance)
KeyStuffed = FALSE;

init_typebuf();
start_stuff();
if (advance && typebuf.tb_maplen == 0)
reg_executing = 0;
do
{



if (typeahead_char != 0)
{
c = typeahead_char;
if (advance)
typeahead_char = 0;
}
else
c = read_readbuffers(advance);
if (c != NUL && !got_int)
{
if (advance)
{



KeyStuffed = TRUE;
}
if (typebuf.tb_no_abbr_cnt == 0)
typebuf.tb_no_abbr_cnt = 1; 
}
else
{






for (;;)
{
long wait_time;
int keylen = 0;
#if defined(FEAT_CMDL_INFO)
int showcmd_idx;
#endif





if (typebuf.tb_maplen)
line_breakcheck();
else
ui_breakcheck(); 
if (got_int)
{

c = inchar(typebuf.tb_buf, typebuf.tb_buflen - 1, 0L);








if ((c || typebuf.tb_maplen)
&& (State & (INSERT + CMDLINE)))
c = ESC;
else
c = Ctrl_C;
flush_buffers(FLUSH_INPUT); 

if (advance)
{


*typebuf.tb_buf = c;
gotchars(typebuf.tb_buf, 1);
}
cmd_silent = FALSE;

break;
}
else if (typebuf.tb_len > 0)
{



map_result_T result = handle_mapping(
&keylen, &timedout, &mapdepth);

if (result == map_result_retry)

continue;
if (result == map_result_fail)
{

c = -1;
break;
}
if (result == map_result_get)
{



c = typebuf.tb_buf[typebuf.tb_off];
if (advance) 
{
cmd_silent = (typebuf.tb_silent > 0);
if (typebuf.tb_maplen > 0)
KeyTyped = FALSE;
else
{
KeyTyped = TRUE;

gotchars(typebuf.tb_buf
+ typebuf.tb_off, 1);
}
KeyNoremap = typebuf.tb_noremap[
typebuf.tb_off];
del_typebuf(1, 0);
}
break;
}


}












c = 0;
#if defined(FEAT_CMDL_INFO)
new_wcol = curwin->w_wcol;
new_wrow = curwin->w_wrow;
#endif
if ( advance
&& typebuf.tb_len == 1
&& typebuf.tb_buf[typebuf.tb_off] == ESC
&& !no_mapping
&& ex_normal_busy == 0
&& typebuf.tb_maplen == 0
&& (State & INSERT)
&& (p_timeout
|| (keylen == KEYLEN_PART_KEY && p_ttimeout))
&& (c = inchar(typebuf.tb_buf + typebuf.tb_off
+ typebuf.tb_len, 3, 25L)) == 0)
{
colnr_T col = 0, vcol;
char_u *ptr;

if (mode_displayed)
{
unshowmode(TRUE);
mode_deleted = TRUE;
}
#if defined(FEAT_GUI)

if (gui.in_use && State != NORMAL && !cmd_silent)
{
int save_State;

save_State = State;
State = NORMAL;
gui_update_cursor(TRUE, FALSE);
State = save_State;
shape_changed = TRUE;
}
#endif
validate_cursor();
old_wcol = curwin->w_wcol;
old_wrow = curwin->w_wrow;


if (curwin->w_cursor.col != 0)
{
if (curwin->w_wcol > 0)
{
if (did_ai)
{





col = vcol = curwin->w_wcol = 0;
ptr = ml_get_curline();
while (col < curwin->w_cursor.col)
{
if (!VIM_ISWHITE(ptr[col]))
curwin->w_wcol = vcol;
vcol += lbr_chartabsize(ptr, ptr + col,
(colnr_T)vcol);
if (has_mbyte)
col += (*mb_ptr2len)(ptr + col);
else
++col;
}
curwin->w_wrow = curwin->w_cline_row
+ curwin->w_wcol / curwin->w_width;
curwin->w_wcol %= curwin->w_width;
curwin->w_wcol += curwin_col_off();
col = 0; 
}
else
{
--curwin->w_wcol;
col = curwin->w_cursor.col - 1;
}
}
else if (curwin->w_p_wrap && curwin->w_wrow)
{
--curwin->w_wrow;
curwin->w_wcol = curwin->w_width - 1;
col = curwin->w_cursor.col - 1;
}
if (has_mbyte && col > 0 && curwin->w_wcol > 0)
{


ptr = ml_get_curline();
col -= (*mb_head_off)(ptr, ptr + col);
if ((*mb_ptr2cells)(ptr + col) > 1)
--curwin->w_wcol;
}
}
setcursor();
out_flush();
#if defined(FEAT_CMDL_INFO)
new_wcol = curwin->w_wcol;
new_wrow = curwin->w_wrow;
#endif
curwin->w_wcol = old_wcol;
curwin->w_wrow = old_wrow;
}
if (c < 0)
continue; 



for (n = 1; n <= c; ++n)
typebuf.tb_noremap[typebuf.tb_off + n] = RM_YES;
typebuf.tb_len += c;


if (typebuf.tb_len >= typebuf.tb_maplen + MAXMAPLEN)
{
timedout = TRUE;
continue;
}

if (ex_normal_busy > 0)
{
#if defined(FEAT_CMDWIN)
static int tc = 0;
#endif




if (typebuf.tb_len > 0)
{
timedout = TRUE;
continue;
}






if (p_im && (State & INSERT))
c = Ctrl_L;
#if defined(FEAT_TERMINAL)
else if (terminal_is_active())
c = K_CANCEL;
#endif
else if ((State & CMDLINE)
#if defined(FEAT_CMDWIN)
|| (cmdwin_type > 0 && tc == ESC)
#endif
)
c = Ctrl_C;
else
c = ESC;
#if defined(FEAT_CMDWIN)
tc = c;
#endif

if (pending_exmode_active)
exmode_active = EXMODE_NORMAL;

break;
}











if (((State & INSERT) != 0 || p_lz) && (State & CMDLINE) == 0
&& advance && must_redraw != 0 && !need_wait_return)
{
update_screen(0);
setcursor(); 
}






#if defined(FEAT_CMDL_INFO)
showcmd_idx = 0;
#endif
c1 = 0;
if (typebuf.tb_len > 0 && advance && !exmode_active)
{
if (((State & (NORMAL | INSERT)) || State == LANGMAP)
&& State != HITRETURN)
{

if (State & INSERT
&& ptr2cells(typebuf.tb_buf + typebuf.tb_off
+ typebuf.tb_len - 1) == 1)
{
edit_putchar(typebuf.tb_buf[typebuf.tb_off
+ typebuf.tb_len - 1], FALSE);
setcursor(); 
c1 = 1;
}
#if defined(FEAT_CMDL_INFO)

old_wcol = curwin->w_wcol;
old_wrow = curwin->w_wrow;
curwin->w_wcol = new_wcol;
curwin->w_wrow = new_wrow;
push_showcmd();
if (typebuf.tb_len > SHOWCMD_COLS)
showcmd_idx = typebuf.tb_len - SHOWCMD_COLS;
while (showcmd_idx < typebuf.tb_len)
(void)add_to_showcmd(
typebuf.tb_buf[typebuf.tb_off + showcmd_idx++]);
curwin->w_wcol = old_wcol;
curwin->w_wrow = old_wrow;
#endif
}


if ((State & CMDLINE)
#if defined(FEAT_CRYPT) || defined(FEAT_EVAL)
&& cmdline_star == 0
#endif
&& ptr2cells(typebuf.tb_buf + typebuf.tb_off
+ typebuf.tb_len - 1) == 1)
{
putcmdline(typebuf.tb_buf[typebuf.tb_off
+ typebuf.tb_len - 1], FALSE);
c1 = 1;
}
}




if (typebuf.tb_len == 0)


timedout = FALSE;

if (advance)
{
if (typebuf.tb_len == 0
|| !(p_timeout
|| (p_ttimeout && keylen == KEYLEN_PART_KEY)))

wait_time = -1L;
else if (keylen == KEYLEN_PART_KEY && p_ttm >= 0)
wait_time = p_ttm;
else
wait_time = p_tm;
}
else
wait_time = 0;

wait_tb_len = typebuf.tb_len;
c = inchar(typebuf.tb_buf + typebuf.tb_off + typebuf.tb_len,
typebuf.tb_buflen - typebuf.tb_off - typebuf.tb_len - 1,
wait_time);

#if defined(FEAT_CMDL_INFO)
if (showcmd_idx != 0)
pop_showcmd();
#endif
if (c1 == 1)
{
if (State & INSERT)
edit_unputchar();
if (State & CMDLINE)
unputcmdline();
else
setcursor(); 
}

if (c < 0)
continue; 
if (c == NUL) 
{
if (!advance)
break;
if (wait_tb_len > 0) 
{
timedout = TRUE;
continue;
}
}
else
{ 
while (typebuf.tb_buf[typebuf.tb_off
+ typebuf.tb_len] != NUL)
typebuf.tb_noremap[typebuf.tb_off
+ typebuf.tb_len++] = RM_YES;
#if defined(HAVE_INPUT_METHOD)


vgetc_im_active = im_get_status();
#endif
}
} 
} 


} while ((c < 0 && c != K_CANCEL) || (advance && c == NUL));






if (advance && p_smd && msg_silent == 0 && (State & INSERT))
{
if (c == ESC && !mode_deleted && !no_mapping && mode_displayed)
{
if (typebuf.tb_len && !KeyTyped)
redraw_cmdline = TRUE; 
else
unshowmode(FALSE);
}
else if (c != ESC && mode_deleted)
{
if (typebuf.tb_len && !KeyTyped)
redraw_cmdline = TRUE; 
else
showmode();
}
}
#if defined(FEAT_GUI)

if (gui.in_use && shape_changed)
gui_update_cursor(TRUE, FALSE);
#endif
if (timedout && c == ESC)
{
char_u nop_buf[3];



nop_buf[0] = K_SPECIAL;
nop_buf[1] = KS_EXTRA;
nop_buf[2] = KE_NOP;
gotchars(nop_buf, 3);
}

--vgetc_busy;

return c;
}
























static int
inchar(
char_u *buf,
int maxlen,
long wait_time) 
{
int len = 0; 
int retesc = FALSE; 
int script_char;
int tb_change_cnt = typebuf.tb_change_cnt;

if (wait_time == -1L || wait_time > 100L) 
{
cursor_on();
out_flush_cursor(FALSE, FALSE);
#if defined(FEAT_GUI) && defined(FEAT_MOUSESHAPE)
if (gui.in_use && postponed_mouseshape)
update_mouseshape(-1);
#endif
}






if (State != HITRETURN)
{
did_outofmem_msg = FALSE; 
did_swapwrite_msg = FALSE; 
}
undo_off = FALSE; 





script_char = -1;
while (scriptin[curscript] != NULL && script_char < 0
#if defined(FEAT_EVAL)
&& !ignore_script
#endif
)
{
#if defined(MESSAGE_QUEUE)
parse_queued_messages();
#endif

if (got_int || (script_char = getc(scriptin[curscript])) < 0)
{



closescript();





if (got_int)
retesc = TRUE;
else
return -1;
}
else
{
buf[0] = script_char;
len = 1;
}
}

if (script_char < 0) 
{








if (got_int)
{
#define DUM_LEN MAXMAPLEN * 3 + 3
char_u dum[DUM_LEN + 1];

for (;;)
{
len = ui_inchar(dum, DUM_LEN, 0L, 0);
if (len == 0 || (len == 1 && dum[0] == 3))
break;
}
return retesc;
}





if (wait_time == -1L || wait_time > 10L)
out_flush();





len = ui_inchar(buf, maxlen / 3, wait_time, tb_change_cnt);
}



if (typebuf_changed(tb_change_cnt))
return 0;




if (len > 0 && ++typebuf.tb_change_cnt == 0)
typebuf.tb_change_cnt = 1;

return fix_input_buffer(buf, len);
}






int
fix_input_buffer(char_u *buf, int len)
{
int i;
char_u *p = buf;








for (i = len; --i >= 0; ++p)
{
#if defined(FEAT_GUI)


if (gui.in_use && p[0] == CSI && i >= 2)
{
p += 2;
i -= 2;
}
#if !defined(MSWIN)

else if (!gui.in_use && p[0] == CSI)
{
mch_memmove(p + 3, p + 1, (size_t)i);
*p++ = K_SPECIAL;
*p++ = KS_EXTRA;
*p = (int)KE_CSI;
len += 2;
}
#endif
else
#endif
if (p[0] == NUL || (p[0] == K_SPECIAL

&& (i < 2 || p[1] != KS_EXTRA || p[2] != (int)KE_CURSORHOLD)
#if defined(MSWIN) && (!defined(FEAT_GUI) || defined(VIMDLL))

&& (
#if defined(VIMDLL)
gui.in_use ||
#endif
(i < 2 || p[1] != KS_MODIFIER))
#endif
))
{
mch_memmove(p + 3, p + 1, (size_t)i);
p[2] = K_THIRD(p[0]);
p[1] = K_SECOND(p[0]);
p[0] = K_SPECIAL;
p += 2;
len += 2;
}
}
*p = NUL; 
return len;
}

#if defined(USE_INPUT_BUF) || defined(PROTO)






int
input_available(void)
{
return (!vim_is_input_buf_empty()
#if defined(FEAT_CLIENTSERVER) || defined(FEAT_EVAL)
|| typebuf_was_filled
#endif
);
}
#endif
