












#include "vim.h"










static yankreg_T y_regs[NUM_REGISTERS];

static yankreg_T *y_current; 
static int y_append; 
static yankreg_T *y_previous = NULL; 

static int stuff_yank(int, char_u *);
static void put_reedit_in_typebuf(int silent);
static int put_in_typebuf(char_u *s, int esc, int colon,
int silent);
static int yank_copy_line(struct block_def *bd, long y_idx);
#if defined(FEAT_CLIPBOARD)
static void copy_yank_reg(yankreg_T *reg);
#endif
static void dis_msg(char_u *p, int skip_esc);

yankreg_T *
get_y_regs(void)
{
return y_regs;
}

yankreg_T *
get_y_register(int reg)
{
return &y_regs[reg];
}

yankreg_T *
get_y_current(void)
{
return y_current;
}

yankreg_T *
get_y_previous(void)
{
return y_previous;
}

void
set_y_current(yankreg_T *yreg)
{
y_current = yreg;
}

void
set_y_previous(yankreg_T *yreg)
{
y_previous = yreg;
}

#if defined(FEAT_EVAL) || defined(PROTO)



static char_u *expr_line = NULL;





int
get_expr_register(void)
{
char_u *new_line;

new_line = getcmdline('=', 0L, 0, TRUE);
if (new_line == NULL)
return NUL;
if (*new_line == NUL) 
vim_free(new_line);
else
set_expr_line(new_line);
return '=';
}





void
set_expr_line(char_u *new_line)
{
vim_free(expr_line);
expr_line = new_line;
}





char_u *
get_expr_line(void)
{
char_u *expr_copy;
char_u *rv;
static int nested = 0;

if (expr_line == NULL)
return NULL;



expr_copy = vim_strsave(expr_line);
if (expr_copy == NULL)
return NULL;



if (nested >= 10)
return expr_copy;

++nested;
rv = eval_to_string(expr_copy, NULL, TRUE);
--nested;
vim_free(expr_copy);
return rv;
}




static char_u *
get_expr_line_src(void)
{
if (expr_line == NULL)
return NULL;
return vim_strsave(expr_line);
}
#endif 





int
valid_yank_reg(
int regname,
int writing) 
{
if ( (regname > 0 && ASCII_ISALNUM(regname))
|| (!writing && vim_strchr((char_u *)
#if defined(FEAT_EVAL)
"/.%:="
#else
"/.%:"
#endif
, regname) != NULL)
|| regname == '#'
|| regname == '"'
|| regname == '-'
|| regname == '_'
#if defined(FEAT_CLIPBOARD)
|| regname == '*'
|| regname == '+'
#endif
#if defined(FEAT_DND)
|| (!writing && regname == '~')
#endif
)
return TRUE;
return FALSE;
}












int
get_yank_register(int regname, int writing)
{
int i;
int ret = FALSE;

y_append = FALSE;
if ((regname == 0 || regname == '"') && !writing && y_previous != NULL)
{
y_current = y_previous;
return ret;
}
i = regname;
if (VIM_ISDIGIT(i))
i -= '0';
else if (ASCII_ISLOWER(i))
i = CharOrdLow(i) + 10;
else if (ASCII_ISUPPER(i))
{
i = CharOrdUp(i) + 10;
y_append = TRUE;
}
else if (regname == '-')
i = DELETION_REGISTER;
#if defined(FEAT_CLIPBOARD)

else if (clip_star.available && regname == '*')
{
i = STAR_REGISTER;
ret = TRUE;
}

else if (clip_plus.available && regname == '+')
{
i = PLUS_REGISTER;
ret = TRUE;
}
#endif
#if defined(FEAT_DND)
else if (!writing && regname == '~')
i = TILDE_REGISTER;
#endif
else 
i = 0;
y_current = &(y_regs[i]);
if (writing) 
y_previous = y_current;
return ret;
}





void *
get_register(
int name,
int copy) 
{
yankreg_T *reg;
int i;

#if defined(FEAT_CLIPBOARD)


if (name == '*' && clip_star.available)
{
if (clip_isautosel_star())
clip_update_selection(&clip_star);
may_get_selection(name);
}
if (name == '+' && clip_plus.available)
{
if (clip_isautosel_plus())
clip_update_selection(&clip_plus);
may_get_selection(name);
}
#endif

get_yank_register(name, 0);
reg = ALLOC_ONE(yankreg_T);
if (reg != NULL)
{
*reg = *y_current;
if (copy)
{

if (reg->y_size == 0)
reg->y_array = NULL;
else
reg->y_array = ALLOC_MULT(char_u *, reg->y_size);
if (reg->y_array != NULL)
{
for (i = 0; i < reg->y_size; ++i)
reg->y_array[i] = vim_strsave(y_current->y_array[i]);
}
}
else
y_current->y_array = NULL;
}
return (void *)reg;
}




void
put_register(int name, void *reg)
{
get_yank_register(name, 0);
free_yank_all();
*y_current = *(yankreg_T *)reg;
vim_free(reg);

#if defined(FEAT_CLIPBOARD)

may_set_selection();
#endif
}

#if (defined(FEAT_CLIPBOARD) && defined(FEAT_X11) && defined(USE_SYSTEM)) || defined(PROTO)

void
free_register(void *reg)
{
yankreg_T tmp;

tmp = *y_current;
*y_current = *(yankreg_T *)reg;
free_yank_all();
vim_free(reg);
*y_current = tmp;
}
#endif




int
yank_register_mline(int regname)
{
if (regname != 0 && !valid_yank_reg(regname, FALSE))
return FALSE;
if (regname == '_') 
return FALSE;
get_yank_register(regname, FALSE);
return (y_current->y_type == MLINE);
}






int
do_record(int c)
{
char_u *p;
static int regname;
yankreg_T *old_y_previous, *old_y_current;
int retval;

if (reg_recording == 0) 
{

if (c < 0 || (!ASCII_ISALNUM(c) && c != '"'))
retval = FAIL;
else
{
reg_recording = c;
showmode();
regname = c;
retval = OK;
}
}
else 
{



reg_recording = 0;
msg("");
p = get_recorded();
if (p == NULL)
retval = FAIL;
else
{

vim_unescape_csi(p);



old_y_previous = y_previous;
old_y_current = y_current;

retval = stuff_yank(regname, p);

y_previous = old_y_previous;
y_current = old_y_current;
}
}
return retval;
}







static int
stuff_yank(int regname, char_u *p)
{
char_u *lp;
char_u **pp;


if (regname != 0 && !valid_yank_reg(regname, TRUE))
{
vim_free(p);
return FAIL;
}
if (regname == '_') 
{
vim_free(p);
return OK;
}
get_yank_register(regname, TRUE);
if (y_append && y_current->y_array != NULL)
{
pp = &(y_current->y_array[y_current->y_size - 1]);
lp = alloc(STRLEN(*pp) + STRLEN(p) + 1);
if (lp == NULL)
{
vim_free(p);
return FAIL;
}
STRCPY(lp, *pp);
STRCAT(lp, p);
vim_free(p);
vim_free(*pp);
*pp = lp;
}
else
{
free_yank_all();
if ((y_current->y_array = ALLOC_ONE(char_u *)) == NULL)
{
vim_free(p);
return FAIL;
}
y_current->y_array[0] = p;
y_current->y_size = 1;
y_current->y_type = MCHAR; 
#if defined(FEAT_VIMINFO)
y_current->y_time_set = vim_time();
#endif
}
return OK;
}

static int execreg_lastc = NUL;

int
get_execreg_lastc(void)
{
return execreg_lastc;
}

void
set_execreg_lastc(int lastc)
{
execreg_lastc = lastc;
}






int
do_execreg(
int regname,
int colon, 
int addcr, 
int silent) 
{
long i;
char_u *p;
int retval = OK;
int remap;


if (regname == '@')
{
if (execreg_lastc == NUL)
{
emsg(_("E748: No previously used register"));
return FAIL;
}
regname = execreg_lastc;
}

if (regname == '%' || regname == '#' || !valid_yank_reg(regname, FALSE))
{
emsg_invreg(regname);
return FAIL;
}
execreg_lastc = regname;

#if defined(FEAT_CLIPBOARD)
regname = may_get_selection(regname);
#endif


if (regname == '_')
return OK;


if (regname == ':')
{
if (last_cmdline == NULL)
{
emsg(_(e_nolastcmd));
return FAIL;
}

VIM_CLEAR(new_last_cmdline);

p = vim_strsave_escaped_ext(last_cmdline,
(char_u *)"\001\002\003\004\005\006\007"
"\010\011\012\013\014\015\016\017"
"\020\021\022\023\024\025\026\027"
"\030\031\032\033\034\035\036\037",
Ctrl_V, FALSE);
if (p != NULL)
{


if (VIsual_active && STRNCMP(p, "'<,'>", 5) == 0)
retval = put_in_typebuf(p + 5, TRUE, TRUE, silent);
else
retval = put_in_typebuf(p, TRUE, TRUE, silent);
}
vim_free(p);
}
#if defined(FEAT_EVAL)
else if (regname == '=')
{
p = get_expr_line();
if (p == NULL)
return FAIL;
retval = put_in_typebuf(p, TRUE, colon, silent);
vim_free(p);
}
#endif
else if (regname == '.') 
{
p = get_last_insert_save();
if (p == NULL)
{
emsg(_(e_noinstext));
return FAIL;
}
retval = put_in_typebuf(p, FALSE, colon, silent);
vim_free(p);
}
else
{
get_yank_register(regname, FALSE);
if (y_current->y_array == NULL)
return FAIL;


remap = colon ? REMAP_NONE : REMAP_YES;


put_reedit_in_typebuf(silent);
for (i = y_current->y_size; --i >= 0; )
{
char_u *escaped;


if (y_current->y_type == MLINE || i < y_current->y_size - 1
|| addcr)
{
if (ins_typebuf((char_u *)"\n", remap, 0, TRUE, silent) == FAIL)
return FAIL;
}
escaped = vim_strsave_escape_csi(y_current->y_array[i]);
if (escaped == NULL)
return FAIL;
retval = ins_typebuf(escaped, remap, 0, TRUE, silent);
vim_free(escaped);
if (retval == FAIL)
return FAIL;
if (colon && ins_typebuf((char_u *)":", remap, 0, TRUE, silent)
== FAIL)
return FAIL;
}
reg_executing = regname == 0 ? '"' : regname; 
}
return retval;
}





static void
put_reedit_in_typebuf(int silent)
{
char_u buf[3];

if (restart_edit != NUL)
{
if (restart_edit == 'V')
{
buf[0] = 'g';
buf[1] = 'R';
buf[2] = NUL;
}
else
{
buf[0] = restart_edit == 'I' ? 'i' : restart_edit;
buf[1] = NUL;
}
if (ins_typebuf(buf, REMAP_NONE, 0, TRUE, silent) == OK)
restart_edit = NUL;
}
}







static int
put_in_typebuf(
char_u *s,
int esc,
int colon, 
int silent)
{
int retval = OK;

put_reedit_in_typebuf(silent);
if (colon)
retval = ins_typebuf((char_u *)"\n", REMAP_NONE, 0, TRUE, silent);
if (retval == OK)
{
char_u *p;

if (esc)
p = vim_strsave_escape_csi(s);
else
p = s;
if (p == NULL)
retval = FAIL;
else
retval = ins_typebuf(p, esc ? REMAP_NONE : REMAP_YES,
0, TRUE, silent);
if (esc)
vim_free(p);
}
if (colon && retval == OK)
retval = ins_typebuf((char_u *)":", REMAP_NONE, 0, TRUE, silent);
return retval;
}







int
insert_reg(
int regname,
int literally_arg) 
{
long i;
int retval = OK;
char_u *arg;
int allocated;
int literally = literally_arg;




ui_breakcheck();
if (got_int)
return FAIL;


if (regname != NUL && !valid_yank_reg(regname, FALSE))
return FAIL;

#if defined(FEAT_CLIPBOARD)
regname = may_get_selection(regname);
#endif

if (regname == '.') 
retval = stuff_inserted(NUL, 1L, TRUE);
else if (get_spec_reg(regname, &arg, &allocated, TRUE))
{
if (arg == NULL)
return FAIL;
stuffescaped(arg, literally);
if (allocated)
vim_free(arg);
}
else 
{
if (get_yank_register(regname, FALSE))
literally = TRUE;
if (y_current->y_array == NULL)
retval = FAIL;
else
{
for (i = 0; i < y_current->y_size; ++i)
{
stuffescaped(y_current->y_array[i], literally);


if (y_current->y_type == MLINE || i < y_current->y_size - 1)
stuffcharReadbuff('\n');
}
}
}

return retval;
}





int
get_spec_reg(
int regname,
char_u **argp,
int *allocated, 
int errmsg) 
{
int cnt;

*argp = NULL;
*allocated = FALSE;
switch (regname)
{
case '%': 
if (errmsg)
check_fname(); 
*argp = curbuf->b_fname;
return TRUE;

case '#': 
*argp = getaltfname(errmsg); 
return TRUE;

#if defined(FEAT_EVAL)
case '=': 
*argp = get_expr_line();
*allocated = TRUE;
return TRUE;
#endif

case ':': 
if (last_cmdline == NULL && errmsg)
emsg(_(e_nolastcmd));
*argp = last_cmdline;
return TRUE;

case '/': 
if (last_search_pat() == NULL && errmsg)
emsg(_(e_noprevre));
*argp = last_search_pat();
return TRUE;

case '.': 
*argp = get_last_insert_save();
*allocated = TRUE;
if (*argp == NULL && errmsg)
emsg(_(e_noinstext));
return TRUE;

#if defined(FEAT_SEARCHPATH)
case Ctrl_F: 
case Ctrl_P: 
if (!errmsg)
return FALSE;
*argp = file_name_at_cursor(FNAME_MESS | FNAME_HYP
| (regname == Ctrl_P ? FNAME_EXP : 0), 1L, NULL);
*allocated = TRUE;
return TRUE;
#endif

case Ctrl_W: 
case Ctrl_A: 
if (!errmsg)
return FALSE;
cnt = find_ident_under_cursor(argp, regname == Ctrl_W
? (FIND_IDENT|FIND_STRING) : FIND_STRING);
*argp = cnt ? vim_strnsave(*argp, cnt) : NULL;
*allocated = TRUE;
return TRUE;

case Ctrl_L: 
if (!errmsg)
return FALSE;

*argp = ml_get_buf(curwin->w_buffer,
curwin->w_cursor.lnum, FALSE);
return TRUE;

case '_': 
*argp = (char_u *)"";
return TRUE;
}

return FALSE;
}










int
cmdline_paste_reg(
int regname,
int literally_arg, 
int remcr) 
{
long i;
int literally = literally_arg;

if (get_yank_register(regname, FALSE))
literally = TRUE;
if (y_current->y_array == NULL)
return FAIL;

for (i = 0; i < y_current->y_size; ++i)
{
cmdline_paste_str(y_current->y_array[i], literally);



if ((y_current->y_type == MLINE || i < y_current->y_size - 1) && !remcr)
cmdline_paste_str((char_u *)"\r", literally);



ui_breakcheck();
if (got_int)
return FAIL;
}
return OK;
}




void
shift_delete_registers()
{
int n;

y_current = &y_regs[9];
free_yank_all(); 
for (n = 9; n > 1; --n)
y_regs[n] = y_regs[n - 1];
y_current = &y_regs[1];
if (!y_append)
y_previous = y_current;
y_regs[1].y_array = NULL; 
}

#if defined(FEAT_EVAL)
void
yank_do_autocmd(oparg_T *oap, yankreg_T *reg)
{
static int recursive = FALSE;
dict_T *v_event;
list_T *list;
int n;
char_u buf[NUMBUFLEN + 2];
long reglen = 0;

if (recursive)
return;

v_event = get_vim_var_dict(VV_EVENT);

list = list_alloc();
if (list == NULL)
return;
for (n = 0; n < reg->y_size; n++)
list_append_string(list, reg->y_array[n], -1);
list->lv_lock = VAR_FIXED;
dict_add_list(v_event, "regcontents", list);

buf[0] = (char_u)oap->regname;
buf[1] = NUL;
dict_add_string(v_event, "regname", buf);

buf[0] = get_op_char(oap->op_type);
buf[1] = get_extra_op_char(oap->op_type);
buf[2] = NUL;
dict_add_string(v_event, "operator", buf);

buf[0] = NUL;
buf[1] = NUL;
switch (get_reg_type(oap->regname, &reglen))
{
case MLINE: buf[0] = 'V'; break;
case MCHAR: buf[0] = 'v'; break;
case MBLOCK:
vim_snprintf((char *)buf, sizeof(buf), "%c%ld", Ctrl_V,
reglen + 1);
break;
}
dict_add_string(v_event, "regtype", buf);


dict_set_items_ro(v_event);

recursive = TRUE;
textlock++;
apply_autocmds(EVENT_TEXTYANKPOST, NULL, NULL, FALSE, curbuf);
textlock--;
recursive = FALSE;


dict_free_contents(v_event);
hash_init(&v_event->dv_hashtab);
}
#endif




void
init_yank(void)
{
int i;

for (i = 0; i < NUM_REGISTERS; ++i)
y_regs[i].y_array = NULL;
}

#if defined(EXITFREE) || defined(PROTO)
void
clear_registers(void)
{
int i;

for (i = 0; i < NUM_REGISTERS; ++i)
{
y_current = &y_regs[i];
if (y_current->y_array != NULL)
free_yank_all();
}
}
#endif





static void
free_yank(long n)
{
if (y_current->y_array != NULL)
{
long i;

for (i = n; --i >= 0; )
{
#if defined(AMIGA)
if ((i & 1023) == 1023) 
{


++no_wait_return;
smsg(_("freeing %ld lines"), i + 1);
--no_wait_return;
msg_didout = FALSE;
msg_col = 0;
}
#endif
vim_free(y_current->y_array[i]);
}
VIM_CLEAR(y_current->y_array);
#if defined(AMIGA)
if (n >= 1000)
msg("");
#endif
}
}

void
free_yank_all(void)
{
free_yank(y_current->y_size);
}









int
op_yank(oparg_T *oap, int deleting, int mess)
{
long y_idx; 
yankreg_T *curr; 
yankreg_T newreg; 
char_u **new_ptr;
linenr_T lnum; 
long j;
int yanktype = oap->motion_type;
long yanklines = oap->line_count;
linenr_T yankendlnum = oap->end.lnum;
char_u *p;
char_u *pnew;
struct block_def bd;
#if defined(FEAT_CLIPBOARD) && defined(FEAT_X11)
int did_star = FALSE;
#endif


if (oap->regname != 0 && !valid_yank_reg(oap->regname, TRUE))
{
beep_flush();
return FAIL;
}
if (oap->regname == '_') 
return OK;

#if defined(FEAT_CLIPBOARD)
if (!clip_star.available && oap->regname == '*')
oap->regname = 0;
else if (!clip_plus.available && oap->regname == '+')
oap->regname = 0;
#endif

if (!deleting) 
get_yank_register(oap->regname, TRUE);

curr = y_current;

if (y_append && y_current->y_array != NULL)
y_current = &newreg;
else
free_yank_all(); 



if ( oap->motion_type == MCHAR
&& oap->start.col == 0
&& !oap->inclusive
&& (!oap->is_VIsual || *p_sel == 'o')
&& !oap->block_mode
&& oap->end.col == 0
&& yanklines > 1)
{
yanktype = MLINE;
--yankendlnum;
--yanklines;
}

y_current->y_size = yanklines;
y_current->y_type = yanktype; 
y_current->y_width = 0;
y_current->y_array = lalloc_clear(sizeof(char_u *) * yanklines, TRUE);
if (y_current->y_array == NULL)
{
y_current = curr;
return FAIL;
}
#if defined(FEAT_VIMINFO)
y_current->y_time_set = vim_time();
#endif

y_idx = 0;
lnum = oap->start.lnum;

if (oap->block_mode)
{

y_current->y_type = MBLOCK; 
y_current->y_width = oap->end_vcol - oap->start_vcol;

if (curwin->w_curswant == MAXCOL && y_current->y_width > 0)
y_current->y_width--;
}

for ( ; lnum <= yankendlnum; lnum++, y_idx++)
{
switch (y_current->y_type)
{
case MBLOCK:
block_prep(oap, &bd, lnum, FALSE);
if (yank_copy_line(&bd, y_idx) == FAIL)
goto fail;
break;

case MLINE:
if ((y_current->y_array[y_idx] =
vim_strsave(ml_get(lnum))) == NULL)
goto fail;
break;

case MCHAR:
{
colnr_T startcol = 0, endcol = MAXCOL;
int is_oneChar = FALSE;
colnr_T cs, ce;

p = ml_get(lnum);
bd.startspaces = 0;
bd.endspaces = 0;

if (lnum == oap->start.lnum)
{
startcol = oap->start.col;
if (virtual_op)
{
getvcol(curwin, &oap->start, &cs, NULL, &ce);
if (ce != cs && oap->start.coladd > 0)
{


bd.startspaces = (ce - cs + 1)
- oap->start.coladd;
startcol++;
}
}
}

if (lnum == oap->end.lnum)
{
endcol = oap->end.col;
if (virtual_op)
{
getvcol(curwin, &oap->end, &cs, NULL, &ce);
if (p[endcol] == NUL || (cs + oap->end.coladd < ce



&& (*mb_head_off)(p, p + endcol) == 0))
{
if (oap->start.lnum == oap->end.lnum
&& oap->start.col == oap->end.col)
{

is_oneChar = TRUE;
bd.startspaces = oap->end.coladd
- oap->start.coladd + oap->inclusive;
endcol = startcol;
}
else
{
bd.endspaces = oap->end.coladd
+ oap->inclusive;
endcol -= oap->inclusive;
}
}
}
}
if (endcol == MAXCOL)
endcol = (colnr_T)STRLEN(p);
if (startcol > endcol || is_oneChar)
bd.textlen = 0;
else
bd.textlen = endcol - startcol + oap->inclusive;
bd.textstart = p + startcol;
if (yank_copy_line(&bd, y_idx) == FAIL)
goto fail;
break;
}

}
}

if (curr != y_current) 
{
new_ptr = ALLOC_MULT(char_u *, curr->y_size + y_current->y_size);
if (new_ptr == NULL)
goto fail;
for (j = 0; j < curr->y_size; ++j)
new_ptr[j] = curr->y_array[j];
vim_free(curr->y_array);
curr->y_array = new_ptr;
#if defined(FEAT_VIMINFO)
curr->y_time_set = vim_time();
#endif

if (yanktype == MLINE) 
curr->y_type = MLINE;



if (curr->y_type == MCHAR && vim_strchr(p_cpo, CPO_REGAPPEND) == NULL)
{
pnew = alloc(STRLEN(curr->y_array[curr->y_size - 1])
+ STRLEN(y_current->y_array[0]) + 1);
if (pnew == NULL)
{
y_idx = y_current->y_size - 1;
goto fail;
}
STRCPY(pnew, curr->y_array[--j]);
STRCAT(pnew, y_current->y_array[0]);
vim_free(curr->y_array[j]);
vim_free(y_current->y_array[0]);
curr->y_array[j++] = pnew;
y_idx = 1;
}
else
y_idx = 0;
while (y_idx < y_current->y_size)
curr->y_array[j++] = y_current->y_array[y_idx++];
curr->y_size = j;
vim_free(y_current->y_array);
y_current = curr;
}
if (curwin->w_p_rnu)
redraw_later(SOME_VALID); 
if (mess) 
{
if (yanktype == MCHAR
&& !oap->block_mode
&& yanklines == 1)
yanklines = 0;

if (yanklines > p_report)
{
char namebuf[100];

if (oap->regname == NUL)
*namebuf = NUL;
else
vim_snprintf(namebuf, sizeof(namebuf),
_(" into \"%c"), oap->regname);


update_topline_redraw();
if (oap->block_mode)
{
smsg(NGETTEXT("block of %ld line yanked%s",
"block of %ld lines yanked%s", yanklines),
yanklines, namebuf);
}
else
{
smsg(NGETTEXT("%ld line yanked%s",
"%ld lines yanked%s", yanklines),
yanklines, namebuf);
}
}
}

if (!cmdmod.lockmarks)
{

curbuf->b_op_start = oap->start;
curbuf->b_op_end = oap->end;
if (yanktype == MLINE && !oap->block_mode)
{
curbuf->b_op_start.col = 0;
curbuf->b_op_end.col = MAXCOL;
}
}

#if defined(FEAT_CLIPBOARD)



if (clip_star.available
&& (curr == &(y_regs[STAR_REGISTER])
|| (!deleting && oap->regname == 0
&& ((clip_unnamed | clip_unnamed_saved) & CLIP_UNNAMED))))
{
if (curr != &(y_regs[STAR_REGISTER]))

copy_yank_reg(&(y_regs[STAR_REGISTER]));

clip_own_selection(&clip_star);
clip_gen_set_selection(&clip_star);
#if defined(FEAT_X11)
did_star = TRUE;
#endif
}

#if defined(FEAT_X11)


if (clip_plus.available
&& (curr == &(y_regs[PLUS_REGISTER])
|| (!deleting && oap->regname == 0
&& ((clip_unnamed | clip_unnamed_saved) &
CLIP_UNNAMED_PLUS))))
{
if (curr != &(y_regs[PLUS_REGISTER]))

copy_yank_reg(&(y_regs[PLUS_REGISTER]));

clip_own_selection(&clip_plus);
clip_gen_set_selection(&clip_plus);
if (!clip_isautosel_star() && !clip_isautosel_plus()
&& !did_star && curr == &(y_regs[PLUS_REGISTER]))
{
copy_yank_reg(&(y_regs[STAR_REGISTER]));
clip_own_selection(&clip_star);
clip_gen_set_selection(&clip_star);
}
}
#endif
#endif

#if defined(FEAT_EVAL)
if (!deleting && has_textyankpost())
yank_do_autocmd(oap, y_current);
#endif

return OK;

fail: 
free_yank(y_idx + 1);
y_current = curr;
return FAIL;
}

static int
yank_copy_line(struct block_def *bd, long y_idx)
{
char_u *pnew;

if ((pnew = alloc(bd->startspaces + bd->endspaces + bd->textlen + 1))
== NULL)
return FAIL;
y_current->y_array[y_idx] = pnew;
vim_memset(pnew, ' ', (size_t)bd->startspaces);
pnew += bd->startspaces;
mch_memmove(pnew, bd->textstart, (size_t)bd->textlen);
pnew += bd->textlen;
vim_memset(pnew, ' ', (size_t)bd->endspaces);
pnew += bd->endspaces;
*pnew = NUL;
return OK;
}

#if defined(FEAT_CLIPBOARD)



static void
copy_yank_reg(yankreg_T *reg)
{
yankreg_T *curr = y_current;
long j;

y_current = reg;
free_yank_all();
*y_current = *curr;
y_current->y_array = lalloc_clear(
sizeof(char_u *) * y_current->y_size, TRUE);
if (y_current->y_array == NULL)
y_current->y_size = 0;
else
for (j = 0; j < y_current->y_size; ++j)
if ((y_current->y_array[j] = vim_strsave(curr->y_array[j])) == NULL)
{
free_yank(j);
y_current->y_size = 0;
break;
}
y_current = curr;
}
#endif








void
do_put(
int regname,
int dir, 
long count,
int flags)
{
char_u *ptr;
char_u *newp, *oldp;
int yanklen;
int totlen = 0; 
linenr_T lnum;
colnr_T col;
long i; 
int y_type;
long y_size;
int oldlen;
long y_width = 0;
colnr_T vcol;
int delcount;
int incr = 0;
long j;
struct block_def bd;
char_u **y_array = NULL;
long nr_lines = 0;
pos_T new_cursor;
int indent;
int orig_indent = 0; 
int indent_diff = 0; 
int first_indent = TRUE;
int lendiff = 0;
pos_T old_pos;
char_u *insert_string = NULL;
int allocated = FALSE;
long cnt;
pos_T orig_start = curbuf->b_op_start;
pos_T orig_end = curbuf->b_op_end;

#if defined(FEAT_CLIPBOARD)

adjust_clip_reg(&regname);
(void)may_get_selection(regname);
#endif

if (flags & PUT_FIXINDENT)
orig_indent = get_indent();

curbuf->b_op_start = curwin->w_cursor; 
curbuf->b_op_end = curwin->w_cursor; 



if (regname == '.')
{
if (VIsual_active)
stuffcharReadbuff(VIsual_mode);
(void)stuff_inserted((dir == FORWARD ? (count == -1 ? 'o' : 'a') :
(count == -1 ? 'O' : 'i')), count, FALSE);


if ((flags & PUT_CURSEND) && gchar_cursor() != NUL)
stuffcharReadbuff('l');
return;
}



if (get_spec_reg(regname, &insert_string, &allocated, TRUE))
{
if (insert_string == NULL)
return;
}



if (u_save(curwin->w_cursor.lnum, curwin->w_cursor.lnum + 1) == FAIL)
goto end;

if (insert_string != NULL)
{
y_type = MCHAR;
#if defined(FEAT_EVAL)
if (regname == '=')
{



for (;;)
{
y_size = 0;
ptr = insert_string;
while (ptr != NULL)
{
if (y_array != NULL)
y_array[y_size] = ptr;
++y_size;
ptr = vim_strchr(ptr, '\n');
if (ptr != NULL)
{
if (y_array != NULL)
*ptr = NUL;
++ptr;

if (*ptr == NUL)
{
y_type = MLINE;
break;
}
}
}
if (y_array != NULL)
break;
y_array = ALLOC_MULT(char_u *, y_size);
if (y_array == NULL)
goto end;
}
}
else
#endif
{
y_size = 1; 
y_array = &insert_string;
}
}
else
{
get_yank_register(regname, FALSE);

y_type = y_current->y_type;
y_width = y_current->y_width;
y_size = y_current->y_size;
y_array = y_current->y_array;
}

if (y_type == MLINE)
{
if (flags & PUT_LINE_SPLIT)
{
char_u *p;



if (u_save_cursor() == FAIL)
goto end;
p = ml_get_cursor();
if (dir == FORWARD && *p != NUL)
MB_PTR_ADV(p);
ptr = vim_strsave(p);
if (ptr == NULL)
goto end;
ml_append(curwin->w_cursor.lnum, ptr, (colnr_T)0, FALSE);
vim_free(ptr);

oldp = ml_get_curline();
p = oldp + curwin->w_cursor.col;
if (dir == FORWARD && *p != NUL)
MB_PTR_ADV(p);
ptr = vim_strnsave(oldp, p - oldp);
if (ptr == NULL)
goto end;
ml_replace(curwin->w_cursor.lnum, ptr, FALSE);
++nr_lines;
dir = FORWARD;
}
if (flags & PUT_LINE_FORWARD)
{

curwin->w_cursor = curbuf->b_visual.vi_end;
dir = FORWARD;
}
curbuf->b_op_start = curwin->w_cursor; 
curbuf->b_op_end = curwin->w_cursor; 
}

if (flags & PUT_LINE) 
y_type = MLINE;

if (y_size == 0 || y_array == NULL)
{
semsg(_("E353: Nothing in register %s"),
regname == 0 ? (char_u *)"\"" : transchar(regname));
goto end;
}

if (y_type == MBLOCK)
{
lnum = curwin->w_cursor.lnum + y_size + 1;
if (lnum > curbuf->b_ml.ml_line_count)
lnum = curbuf->b_ml.ml_line_count + 1;
if (u_save(curwin->w_cursor.lnum - 1, lnum) == FAIL)
goto end;
}
else if (y_type == MLINE)
{
lnum = curwin->w_cursor.lnum;
#if defined(FEAT_FOLDING)


if (dir == BACKWARD)
(void)hasFolding(lnum, &lnum, NULL);
else
(void)hasFolding(lnum, NULL, &lnum);
#endif
if (dir == FORWARD)
++lnum;


if ((BUFEMPTY() ? u_save(0, 2) : u_save(lnum - 1, lnum)) == FAIL)
goto end;
#if defined(FEAT_FOLDING)
if (dir == FORWARD)
curwin->w_cursor.lnum = lnum - 1;
else
curwin->w_cursor.lnum = lnum;
curbuf->b_op_start = curwin->w_cursor; 
#endif
}
else if (u_save_cursor() == FAIL)
goto end;

yanklen = (int)STRLEN(y_array[0]);

if (ve_flags == VE_ALL && y_type == MCHAR)
{
if (gchar_cursor() == TAB)
{
int viscol = getviscol();
int ts = curbuf->b_p_ts;



if (dir == FORWARD ?
#if defined(FEAT_VARTABS)
tabstop_padding(viscol, ts, curbuf->b_p_vts_array) != 1
#else
ts - (viscol % ts) != 1
#endif
: curwin->w_cursor.coladd > 0)
coladvance_force(viscol);
else
curwin->w_cursor.coladd = 0;
}
else if (curwin->w_cursor.coladd > 0 || gchar_cursor() == NUL)
coladvance_force(getviscol() + (dir == FORWARD));
}

lnum = curwin->w_cursor.lnum;
col = curwin->w_cursor.col;


if (y_type == MBLOCK)
{
int c = gchar_cursor();
colnr_T endcol2 = 0;

if (dir == FORWARD && c != NUL)
{
if (ve_flags == VE_ALL)
getvcol(curwin, &curwin->w_cursor, &col, NULL, &endcol2);
else
getvcol(curwin, &curwin->w_cursor, NULL, NULL, &col);

if (has_mbyte)

curwin->w_cursor.col += (*mb_ptr2len)(ml_get_cursor());
else
if (c != TAB || ve_flags != VE_ALL)
++curwin->w_cursor.col;
++col;
}
else
getvcol(curwin, &curwin->w_cursor, &col, NULL, &endcol2);

col += curwin->w_cursor.coladd;
if (ve_flags == VE_ALL
&& (curwin->w_cursor.coladd > 0
|| endcol2 == curwin->w_cursor.col))
{
if (dir == FORWARD && c == NUL)
++col;
if (dir != FORWARD && c != NUL)
++curwin->w_cursor.col;
if (c == TAB)
{
if (dir == BACKWARD && curwin->w_cursor.col)
curwin->w_cursor.col--;
if (dir == FORWARD && col - 1 == endcol2)
curwin->w_cursor.col++;
}
}
curwin->w_cursor.coladd = 0;
bd.textcol = 0;
for (i = 0; i < y_size; ++i)
{
int spaces;
char shortline;

bd.startspaces = 0;
bd.endspaces = 0;
vcol = 0;
delcount = 0;


if (curwin->w_cursor.lnum > curbuf->b_ml.ml_line_count)
{
if (ml_append(curbuf->b_ml.ml_line_count, (char_u *)"",
(colnr_T)1, FALSE) == FAIL)
break;
++nr_lines;
}

oldp = ml_get_curline();
oldlen = (int)STRLEN(oldp);
for (ptr = oldp; vcol < col && *ptr; )
{

incr = lbr_chartabsize_adv(oldp, &ptr, (colnr_T)vcol);
vcol += incr;
}
bd.textcol = (colnr_T)(ptr - oldp);

shortline = (vcol < col) || (vcol == col && !*ptr) ;

if (vcol < col) 
bd.startspaces = col - vcol;
else if (vcol > col)
{
bd.endspaces = vcol - col;
bd.startspaces = incr - bd.endspaces;
--bd.textcol;
delcount = 1;
if (has_mbyte)
bd.textcol -= (*mb_head_off)(oldp, oldp + bd.textcol);
if (oldp[bd.textcol] != TAB)
{



delcount = 0;
bd.endspaces = 0;
}
}

yanklen = (int)STRLEN(y_array[i]);


spaces = y_width + 1;
for (j = 0; j < yanklen; j++)
spaces -= lbr_chartabsize(NULL, &y_array[i][j], 0);
if (spaces < 0)
spaces = 0;


totlen = count * (yanklen + spaces) + bd.startspaces + bd.endspaces;
newp = alloc(totlen + oldlen + 1);
if (newp == NULL)
break;

ptr = newp;
mch_memmove(ptr, oldp, (size_t)bd.textcol);
ptr += bd.textcol;

vim_memset(ptr, ' ', (size_t)bd.startspaces);
ptr += bd.startspaces;

for (j = 0; j < count; ++j)
{
mch_memmove(ptr, y_array[i], (size_t)yanklen);
ptr += yanklen;


if ((j < count - 1 || !shortline) && spaces)
{
vim_memset(ptr, ' ', (size_t)spaces);
ptr += spaces;
}
}

vim_memset(ptr, ' ', (size_t)bd.endspaces);
ptr += bd.endspaces;

mch_memmove(ptr, oldp + bd.textcol + delcount,
(size_t)(oldlen - bd.textcol - delcount + 1));
ml_replace(curwin->w_cursor.lnum, newp, FALSE);

++curwin->w_cursor.lnum;
if (i == 0)
curwin->w_cursor.col += bd.startspaces;
}

changed_lines(lnum, 0, curwin->w_cursor.lnum, nr_lines);


curbuf->b_op_start = curwin->w_cursor;
curbuf->b_op_start.lnum = lnum;


curbuf->b_op_end.lnum = curwin->w_cursor.lnum - 1;
curbuf->b_op_end.col = bd.textcol + totlen - 1;
curbuf->b_op_end.coladd = 0;
if (flags & PUT_CURSEND)
{
colnr_T len;

curwin->w_cursor = curbuf->b_op_end;
curwin->w_cursor.col++;


len = (colnr_T)STRLEN(ml_get_curline());
if (curwin->w_cursor.col > len)
curwin->w_cursor.col = len;
}
else
curwin->w_cursor.lnum = lnum;
}
else
{

if (y_type == MCHAR)
{


if (dir == FORWARD && gchar_cursor() != NUL)
{
if (has_mbyte)
{
int bytelen = (*mb_ptr2len)(ml_get_cursor());


col += bytelen;
if (yanklen)
{
curwin->w_cursor.col += bytelen;
curbuf->b_op_end.col += bytelen;
}
}
else
{
++col;
if (yanklen)
{
++curwin->w_cursor.col;
++curbuf->b_op_end.col;
}
}
}
curbuf->b_op_start = curwin->w_cursor;
}

else if (dir == BACKWARD)
--lnum;
new_cursor = curwin->w_cursor;


if (y_type == MCHAR && y_size == 1)
{
linenr_T end_lnum = 0; 

if (VIsual_active)
{
end_lnum = curbuf->b_visual.vi_end.lnum;
if (end_lnum < curbuf->b_visual.vi_start.lnum)
end_lnum = curbuf->b_visual.vi_start.lnum;
}

do {
totlen = count * yanklen;
if (totlen > 0)
{
oldp = ml_get(lnum);
if (VIsual_active && col > (int)STRLEN(oldp))
{
lnum++;
continue;
}
newp = alloc(STRLEN(oldp) + totlen + 1);
if (newp == NULL)
goto end; 
mch_memmove(newp, oldp, (size_t)col);
ptr = newp + col;
for (i = 0; i < count; ++i)
{
mch_memmove(ptr, y_array[0], (size_t)yanklen);
ptr += yanklen;
}
STRMOVE(ptr, oldp + col);
ml_replace(lnum, newp, FALSE);

if (lnum == curwin->w_cursor.lnum)
{

changed_cline_bef_curs();
curwin->w_cursor.col += (colnr_T)(totlen - 1);
}
}
if (VIsual_active)
lnum++;
} while (VIsual_active && lnum <= end_lnum);

if (VIsual_active) 
lnum--;

curbuf->b_op_end = curwin->w_cursor;

if (totlen && (restart_edit != 0 || (flags & PUT_CURSEND)))
++curwin->w_cursor.col;
changed_bytes(lnum, col);
}
else
{


for (cnt = 1; cnt <= count; ++cnt)
{
i = 0;
if (y_type == MCHAR)
{



lnum = new_cursor.lnum;
ptr = ml_get(lnum) + col;
totlen = (int)STRLEN(y_array[y_size - 1]);
newp = alloc(STRLEN(ptr) + totlen + 1);
if (newp == NULL)
goto error;
STRCPY(newp, y_array[y_size - 1]);
STRCAT(newp, ptr);

ml_append(lnum, newp, (colnr_T)0, FALSE);
vim_free(newp);

oldp = ml_get(lnum);
newp = alloc(col + yanklen + 1);
if (newp == NULL)
goto error;

mch_memmove(newp, oldp, (size_t)col);

mch_memmove(newp + col, y_array[0], (size_t)(yanklen + 1));
ml_replace(lnum, newp, FALSE);

curwin->w_cursor.lnum = lnum;
i = 1;
}

for (; i < y_size; ++i)
{
if ((y_type != MCHAR || i < y_size - 1)
&& ml_append(lnum, y_array[i], (colnr_T)0, FALSE)
== FAIL)
goto error;
lnum++;
++nr_lines;
if (flags & PUT_FIXINDENT)
{
old_pos = curwin->w_cursor;
curwin->w_cursor.lnum = lnum;
ptr = ml_get(lnum);
if (cnt == count && i == y_size - 1)
lendiff = (int)STRLEN(ptr);
#if defined(FEAT_SMARTINDENT) || defined(FEAT_CINDENT)
if (*ptr == '#' && preprocs_left())
indent = 0; 
else
#endif
if (*ptr == NUL)
indent = 0; 
else if (first_indent)
{
indent_diff = orig_indent - get_indent();
indent = orig_indent;
first_indent = FALSE;
}
else if ((indent = get_indent() + indent_diff) < 0)
indent = 0;
(void)set_indent(indent, 0);
curwin->w_cursor = old_pos;

if (cnt == count && i == y_size - 1)
lendiff -= (int)STRLEN(ml_get(lnum));
}
}
}

error:

if (y_type == MLINE)
{
curbuf->b_op_start.col = 0;
if (dir == FORWARD)
curbuf->b_op_start.lnum++;
}


if (curbuf->b_op_start.lnum + (y_type == MCHAR) - 1 + nr_lines
< curbuf->b_ml.ml_line_count
#if defined(FEAT_DIFF)
|| curwin->w_p_diff
#endif
)
mark_adjust(curbuf->b_op_start.lnum + (y_type == MCHAR),
(linenr_T)MAXLNUM, nr_lines, 0L);


if (y_type == MCHAR)
changed_lines(curwin->w_cursor.lnum, col,
curwin->w_cursor.lnum + 1, nr_lines);
else
changed_lines(curbuf->b_op_start.lnum, 0,
curbuf->b_op_start.lnum, nr_lines);


curbuf->b_op_end.lnum = lnum;

col = (colnr_T)STRLEN(y_array[y_size - 1]) - lendiff;
if (col > 1)
curbuf->b_op_end.col = col - 1;
else
curbuf->b_op_end.col = 0;

if (flags & PUT_CURSLINE)
{

curwin->w_cursor.lnum = lnum;
beginline(BL_WHITE | BL_FIX);
}
else if (flags & PUT_CURSEND)
{

if (y_type == MLINE)
{
if (lnum >= curbuf->b_ml.ml_line_count)
curwin->w_cursor.lnum = curbuf->b_ml.ml_line_count;
else
curwin->w_cursor.lnum = lnum + 1;
curwin->w_cursor.col = 0;
}
else
{
curwin->w_cursor.lnum = lnum;
curwin->w_cursor.col = col;
}
}
else if (y_type == MLINE)
{

curwin->w_cursor.col = 0;
if (dir == FORWARD)
++curwin->w_cursor.lnum;
beginline(BL_WHITE | BL_FIX);
}
else 
curwin->w_cursor = new_cursor;
}
}

msgmore(nr_lines);
curwin->w_set_curswant = TRUE;

end:
if (cmdmod.lockmarks)
{
curbuf->b_op_start = orig_start;
curbuf->b_op_end = orig_end;
}
if (allocated)
vim_free(insert_string);
if (regname == '=')
vim_free(y_array);

VIsual_active = FALSE;


adjust_cursor_eol();
}




int
get_register_name(int num)
{
if (num == -1)
return '"';
else if (num < 10)
return num + '0';
else if (num == DELETION_REGISTER)
return '-';
#if defined(FEAT_CLIPBOARD)
else if (num == STAR_REGISTER)
return '*';
else if (num == PLUS_REGISTER)
return '+';
#endif
else
{
#if defined(EBCDIC)
int i;


i = 'a' + (num - 10);
if (i > 'i')
i += 7;
if (i > 'r')
i += 8;
return i;
#else
return num + 'a' - 10;
#endif
}
}




void
ex_display(exarg_T *eap)
{
int i, n;
long j;
char_u *p;
yankreg_T *yb;
int name;
int attr;
char_u *arg = eap->arg;
int clen;
int type;

if (arg != NULL && *arg == NUL)
arg = NULL;
attr = HL_ATTR(HLF_8);


msg_puts_title(_("\nType Name Content"));
for (i = -1; i < NUM_REGISTERS && !got_int; ++i)
{
name = get_register_name(i);
switch (get_reg_type(name, NULL))
{
case MLINE: type = 'l'; break;
case MCHAR: type = 'c'; break;
default: type = 'b'; break;
}
if (arg != NULL && vim_strchr(arg, name) == NULL
#if defined(ONE_CLIPBOARD)

&& (name != '*' || vim_strchr(arg, '+') == NULL)
#endif
)
continue; 

#if defined(FEAT_CLIPBOARD)



adjust_clip_reg(&name);
(void)may_get_selection(name);
#endif

if (i == -1)
{
if (y_previous != NULL)
yb = y_previous;
else
yb = &(y_regs[0]);
}
else
yb = &(y_regs[i]);

#if defined(FEAT_EVAL)
if (name == MB_TOLOWER(redir_reg)
|| (redir_reg == '"' && yb == y_previous))
continue; 

#endif

if (yb->y_array != NULL)
{
int do_show = FALSE;

for (j = 0; !do_show && j < yb->y_size; ++j)
do_show = !message_filtered(yb->y_array[j]);

if (do_show || yb->y_size == 0)
{
msg_putchar('\n');
msg_puts(" ");
msg_putchar(type);
msg_puts(" ");
msg_putchar('"');
msg_putchar(name);
msg_puts(" ");

n = (int)Columns - 11;
for (j = 0; j < yb->y_size && n > 1; ++j)
{
if (j)
{
msg_puts_attr("^J", attr);
n -= 2;
}
for (p = yb->y_array[j]; *p && (n -= ptr2cells(p)) >= 0;
++p)
{
clen = (*mb_ptr2len)(p);
msg_outtrans_len(p, clen);
p += clen - 1;
}
}
if (n > 1 && yb->y_type == MLINE)
msg_puts_attr("^J", attr);
out_flush(); 
}
ui_breakcheck();
}
}


if ((p = get_last_insert()) != NULL
&& (arg == NULL || vim_strchr(arg, '.') != NULL) && !got_int
&& !message_filtered(p))
{
msg_puts("\n c \". ");
dis_msg(p, TRUE);
}


if (last_cmdline != NULL && (arg == NULL || vim_strchr(arg, ':') != NULL)
&& !got_int && !message_filtered(last_cmdline))
{
msg_puts("\n c \": ");
dis_msg(last_cmdline, FALSE);
}


if (curbuf->b_fname != NULL
&& (arg == NULL || vim_strchr(arg, '%') != NULL) && !got_int
&& !message_filtered(curbuf->b_fname))
{
msg_puts("\n c \"% ");
dis_msg(curbuf->b_fname, FALSE);
}


if ((arg == NULL || vim_strchr(arg, '%') != NULL) && !got_int)
{
char_u *fname;
linenr_T dummy;

if (buflist_name_nr(0, &fname, &dummy) != FAIL
&& !message_filtered(fname))
{
msg_puts("\n c \"#");
dis_msg(fname, FALSE);
}
}


if (last_search_pat() != NULL
&& (arg == NULL || vim_strchr(arg, '/') != NULL) && !got_int
&& !message_filtered(last_search_pat()))
{
msg_puts("\n c \"/ ");
dis_msg(last_search_pat(), FALSE);
}

#if defined(FEAT_EVAL)

if (expr_line != NULL && (arg == NULL || vim_strchr(arg, '=') != NULL)
&& !got_int && !message_filtered(expr_line))
{
msg_puts("\n c \"= ");
dis_msg(expr_line, FALSE);
}
#endif
}





static void
dis_msg(
char_u *p,
int skip_esc) 
{
int n;
int l;

n = (int)Columns - 6;
while (*p != NUL
&& !(*p == ESC && skip_esc && *(p + 1) == NUL)
&& (n -= ptr2cells(p)) >= 0)
{
if (has_mbyte && (l = (*mb_ptr2len)(p)) > 1)
{
msg_outtrans_len(p, l);
p += l;
}
else
msg_outtrans_len(p++, 1);
}
ui_breakcheck();
}

#if defined(FEAT_DND) || defined(PROTO)



void
dnd_yank_drag_data(char_u *str, long len)
{
yankreg_T *curr;

curr = y_current;
y_current = &y_regs[TILDE_REGISTER];
free_yank_all();
str_to_reg(y_current, MCHAR, str, len, 0L, FALSE);
y_current = curr;
}
#endif







char_u
get_reg_type(int regname, long *reglen)
{
switch (regname)
{
case '%': 
case '#': 
case '=': 
case ':': 
case '/': 
case '.': 
#if defined(FEAT_SEARCHPATH)
case Ctrl_F: 
case Ctrl_P: 
#endif
case Ctrl_W: 
case Ctrl_A: 
case '_': 
return MCHAR;
}

#if defined(FEAT_CLIPBOARD)
regname = may_get_selection(regname);
#endif

if (regname != NUL && !valid_yank_reg(regname, FALSE))
return MAUTO;

get_yank_register(regname, FALSE);

if (y_current->y_array != NULL)
{
if (reglen != NULL && y_current->y_type == MBLOCK)
*reglen = y_current->y_width;
return y_current->y_type;
}
return MAUTO;
}

#if defined(FEAT_EVAL) || defined(PROTO)




static char_u *
getreg_wrap_one_line(char_u *s, int flags)
{
if (flags & GREG_LIST)
{
list_T *list = list_alloc();

if (list != NULL)
{
if (list_append_string(list, NULL, -1) == FAIL)
{
list_free(list);
return NULL;
}
list->lv_first->li_tv.vval.v_string = s;
}
return (char_u *)list;
}
return s;
}











char_u *
get_reg_contents(int regname, int flags)
{
long i;
char_u *retval;
int allocated;
long len;


if (regname == '=')
{
if (flags & GREG_NO_EXPR)
return NULL;
if (flags & GREG_EXPR_SRC)
return getreg_wrap_one_line(get_expr_line_src(), flags);
return getreg_wrap_one_line(get_expr_line(), flags);
}

if (regname == '@') 
regname = '"';


if (regname != NUL && !valid_yank_reg(regname, FALSE))
return NULL;

#if defined(FEAT_CLIPBOARD)
regname = may_get_selection(regname);
#endif

if (get_spec_reg(regname, &retval, &allocated, FALSE))
{
if (retval == NULL)
return NULL;
if (allocated)
return getreg_wrap_one_line(retval, flags);
return getreg_wrap_one_line(vim_strsave(retval), flags);
}

get_yank_register(regname, FALSE);
if (y_current->y_array == NULL)
return NULL;

if (flags & GREG_LIST)
{
list_T *list = list_alloc();
int error = FALSE;

if (list == NULL)
return NULL;
for (i = 0; i < y_current->y_size; ++i)
if (list_append_string(list, y_current->y_array[i], -1) == FAIL)
error = TRUE;
if (error)
{
list_free(list);
return NULL;
}
return (char_u *)list;
}


len = 0;
for (i = 0; i < y_current->y_size; ++i)
{
len += (long)STRLEN(y_current->y_array[i]);


if (y_current->y_type == MLINE || i < y_current->y_size - 1)
++len;
}

retval = alloc(len + 1);


if (retval != NULL)
{
len = 0;
for (i = 0; i < y_current->y_size; ++i)
{
STRCPY(retval + len, y_current->y_array[i]);
len += (long)STRLEN(retval + len);



if (y_current->y_type == MLINE || i < y_current->y_size - 1)
retval[len++] = '\n';
}
retval[len] = NUL;
}

return retval;
}

static int
init_write_reg(
int name,
yankreg_T **old_y_previous,
yankreg_T **old_y_current,
int must_append,
int *yank_type UNUSED)
{
if (!valid_yank_reg(name, TRUE)) 
{
emsg_invreg(name);
return FAIL;
}


*old_y_previous = y_previous;
*old_y_current = y_current;

get_yank_register(name, TRUE);
if (!y_append && !must_append)
free_yank_all();
return OK;
}

static void
finish_write_reg(
int name,
yankreg_T *old_y_previous,
yankreg_T *old_y_current)
{
#if defined(FEAT_CLIPBOARD)

may_set_selection();
#endif


if (name != '"')
y_previous = old_y_previous;
y_current = old_y_current;
}










void
write_reg_contents(
int name,
char_u *str,
int maxlen,
int must_append)
{
write_reg_contents_ex(name, str, maxlen, must_append, MAUTO, 0L);
}

void
write_reg_contents_lst(
int name,
char_u **strings,
int maxlen UNUSED,
int must_append,
int yank_type,
long block_len)
{
yankreg_T *old_y_previous, *old_y_current;

if (name == '/' || name == '=')
{
char_u *s;

if (strings[0] == NULL)
s = (char_u *)"";
else if (strings[1] != NULL)
{
emsg(_("E883: search pattern and expression register may not "
"contain two or more lines"));
return;
}
else
s = strings[0];
write_reg_contents_ex(name, s, -1, must_append, yank_type, block_len);
return;
}

if (name == '_') 
return;

if (init_write_reg(name, &old_y_previous, &old_y_current, must_append,
&yank_type) == FAIL)
return;

str_to_reg(y_current, yank_type, (char_u *) strings, -1, block_len, TRUE);

finish_write_reg(name, old_y_previous, old_y_current);
}

void
write_reg_contents_ex(
int name,
char_u *str,
int maxlen,
int must_append,
int yank_type,
long block_len)
{
yankreg_T *old_y_previous, *old_y_current;
long len;

if (maxlen >= 0)
len = maxlen;
else
len = (long)STRLEN(str);


if (name == '/')
{
set_last_search_pat(str, RE_SEARCH, TRUE, TRUE);
return;
}

if (name == '#')
{
buf_T *buf;

if (VIM_ISDIGIT(*str))
{
int num = atoi((char *)str);

buf = buflist_findnr(num);
if (buf == NULL)
semsg(_(e_nobufnr), (long)num);
}
else
buf = buflist_findnr(buflist_findpat(str, str + STRLEN(str),
TRUE, FALSE, FALSE));
if (buf == NULL)
return;
curwin->w_alt_fnum = buf->b_fnum;
return;
}

if (name == '=')
{
char_u *p, *s;

p = vim_strnsave(str, (int)len);
if (p == NULL)
return;
if (must_append && expr_line != NULL)
{
s = concat_str(expr_line, p);
vim_free(p);
p = s;
}
set_expr_line(p);
return;
}

if (name == '_') 
return;

if (init_write_reg(name, &old_y_previous, &old_y_current, must_append,
&yank_type) == FAIL)
return;

str_to_reg(y_current, yank_type, str, len, block_len, FALSE);

finish_write_reg(name, old_y_previous, old_y_current);
}
#endif 

#if defined(FEAT_CLIPBOARD) || defined(FEAT_EVAL)




void
str_to_reg(
yankreg_T *y_ptr, 
int yank_type, 
char_u *str, 
long len, 
long blocklen, 
int str_list) 
{
int type; 
int lnum;
long start;
long i;
int extra;
int newlines; 
int extraline = 0; 
int append = FALSE; 
char_u *s;
char_u **ss;
char_u **pp;
long maxlen;

if (y_ptr->y_array == NULL) 
y_ptr->y_size = 0;

if (yank_type == MAUTO)
type = ((str_list || (len > 0 && (str[len - 1] == NL
|| str[len - 1] == CAR)))
? MLINE : MCHAR);
else
type = yank_type;


newlines = 0;
if (str_list)
{
for (ss = (char_u **) str; *ss != NULL; ++ss)
++newlines;
}
else
{
for (i = 0; i < len; i++)
if (str[i] == '\n')
++newlines;
if (type == MCHAR || len == 0 || str[len - 1] != '\n')
{
extraline = 1;
++newlines; 
}
if (y_ptr->y_size > 0 && y_ptr->y_type == MCHAR)
{
append = TRUE;
--newlines; 
}
}


if (y_ptr->y_size + newlines == 0)
{
VIM_CLEAR(y_ptr->y_array);
return;
}



pp = lalloc_clear((y_ptr->y_size + newlines) * sizeof(char_u *), TRUE);
if (pp == NULL) 
return;
for (lnum = 0; lnum < y_ptr->y_size; ++lnum)
pp[lnum] = y_ptr->y_array[lnum];
vim_free(y_ptr->y_array);
y_ptr->y_array = pp;
maxlen = 0;


if (str_list)
{
for (ss = (char_u **) str; *ss != NULL; ++ss, ++lnum)
{
i = (long)STRLEN(*ss);
pp[lnum] = vim_strnsave(*ss, i);
if (i > maxlen)
maxlen = i;
}
}
else
{
for (start = 0; start < len + extraline; start += i + 1)
{
for (i = start; i < len; ++i) 
if (str[i] == '\n')
break;
i -= start; 
if (i > maxlen)
maxlen = i;
if (append)
{
--lnum;
extra = (int)STRLEN(y_ptr->y_array[lnum]);
}
else
extra = 0;
s = alloc(i + extra + 1);
if (s == NULL)
break;
if (extra)
mch_memmove(s, y_ptr->y_array[lnum], (size_t)extra);
if (append)
vim_free(y_ptr->y_array[lnum]);
if (i)
mch_memmove(s + extra, str + start, (size_t)i);
extra += i;
s[extra] = NUL;
y_ptr->y_array[lnum++] = s;
while (--extra >= 0)
{
if (*s == NUL)
*s = '\n'; 
++s;
}
append = FALSE; 
}
}
y_ptr->y_type = type;
y_ptr->y_size = lnum;
if (type == MBLOCK)
y_ptr->y_width = (blocklen < 0 ? maxlen - 1 : blocklen);
else
y_ptr->y_width = 0;
#if defined(FEAT_VIMINFO)
y_ptr->y_time_set = vim_time();
#endif
}
#endif 
