







#include "nvim/vim.h"
#include "nvim/sign.h"
#include "nvim/ascii.h"
#include "nvim/buffer.h"
#include "nvim/charset.h"
#include "nvim/cursor.h"
#include "nvim/ex_docmd.h"
#include "nvim/edit.h"
#include "nvim/fold.h"
#include "nvim/move.h"
#include "nvim/screen.h"
#include "nvim/syntax.h"


typedef struct sign sign_T;

struct sign
{
sign_T *sn_next; 
int sn_typenr; 
char_u *sn_name; 
char_u *sn_icon; 
char_u *sn_text; 
int sn_line_hl; 
int sn_text_hl; 
int sn_num_hl; 
};

static sign_T *first_sign = NULL;
static int next_sign_typenr = 1;

static void sign_list_defined(sign_T *sp);
static void sign_undefine(sign_T *sp, sign_T *sp_prev);

static char *cmds[] = {
"define",
#define SIGNCMD_DEFINE 0
"undefine",
#define SIGNCMD_UNDEFINE 1
"list",
#define SIGNCMD_LIST 2
"place",
#define SIGNCMD_PLACE 3
"unplace",
#define SIGNCMD_UNPLACE 4
"jump",
#define SIGNCMD_JUMP 5
NULL
#define SIGNCMD_LAST 6
};


static hashtab_T sg_table; 
static int next_sign_id = 1; 


void init_signs(void)
{
hash_init(&sg_table); 
}




static signgroup_T * sign_group_ref(const char_u *groupname)
{
hash_T hash;
hashitem_T *hi;
signgroup_T *group;

hash = hash_hash(groupname);
hi = hash_lookup(&sg_table, (char *)groupname, STRLEN(groupname), hash);
if (HASHITEM_EMPTY(hi)) {

group = xmalloc((unsigned)(sizeof(signgroup_T) + STRLEN(groupname)));

STRCPY(group->sg_name, groupname);
group->refcount = 1;
group->next_sign_id = 1;
hash_add_item(&sg_table, hi, group->sg_name, hash);
} else {

group = HI2SG(hi);
group->refcount++;
}

return group;
}



static void sign_group_unref(char_u *groupname)
{
hashitem_T *hi;
signgroup_T *group;

hi = hash_find(&sg_table, groupname);
if (!HASHITEM_EMPTY(hi)) {
group = HI2SG(hi);
group->refcount--;
if (group->refcount == 0) {

hash_remove(&sg_table, hi);
xfree(group);
}
}
}




int sign_in_group(signlist_T *sign, const char_u *group)
{
return ((group != NULL && STRCMP(group, "*") == 0)
|| (group == NULL && sign->group == NULL)
|| (group != NULL && sign->group != NULL
&& STRCMP(group, sign->group->sg_name) == 0));
}


int sign_group_get_next_signid(buf_T *buf, const char_u *groupname)
{
int id = 1;
signgroup_T *group = NULL;
signlist_T *sign;
hashitem_T *hi;
int found = false;

if (groupname != NULL) {
hi = hash_find(&sg_table, groupname);
if (HASHITEM_EMPTY(hi)) {
return id;
}
group = HI2SG(hi);
}


while (!found) {
if (group == NULL) {
id = next_sign_id++; 
} else {
id = group->next_sign_id++;
}


found = true;
FOR_ALL_SIGNS_IN_BUF(buf, sign) {
if (id == sign->id && sign_in_group(sign, groupname)) {
found = false; 
break;
}
}
}

return id;
}



static void insert_sign(
buf_T *buf, 
signlist_T *prev, 
signlist_T *next, 
int id, 
const char_u *group, 
int prio, 
linenr_T lnum, 
int typenr 
)
{
signlist_T *newsign = xmalloc(sizeof(signlist_T));
newsign->id = id;
newsign->lnum = lnum;
newsign->typenr = typenr;
if (group != NULL) {
newsign->group = sign_group_ref(group);
} else {
newsign->group = NULL;
}
newsign->priority = prio;
newsign->next = next;
newsign->prev = prev;
if (next != NULL) {
next->prev = newsign;
}
buf->b_signcols_max = -1;

if (prev == NULL) {


if (buf->b_signlist == NULL) {
redraw_buf_later(buf, NOT_VALID);
changed_line_abv_curs();
}


buf->b_signlist = newsign;
} else {
prev->next = newsign;
}
}


static void insert_sign_by_lnum_prio(
buf_T *buf, 
signlist_T *prev, 
int id, 
const char_u *group, 
int prio, 
linenr_T lnum, 
int typenr 
)
{
signlist_T *sign;



while (prev != NULL && prev->lnum == lnum
&& (prev->priority < prio
|| (prev->priority == prio && prev->id <= id))) {
prev = prev->prev;
}
if (prev == NULL) {
sign = buf->b_signlist;
} else {
sign = prev->next;
}

insert_sign(buf, prev, sign, id, group, prio, lnum, typenr);
}


char_u * sign_typenr2name(int typenr)
{
sign_T *sp;

for (sp = first_sign; sp != NULL; sp = sp->sn_next) {
if (sp->sn_typenr == typenr) {
return sp->sn_name;
}
}
return (char_u *)_("[Deleted]");
}


dict_T * sign_get_info(signlist_T *sign)
{
dict_T *d = tv_dict_alloc();
tv_dict_add_nr(d, S_LEN("id"), sign->id);
tv_dict_add_str(d, S_LEN("group"), ((sign->group == NULL)
? (char *)""
: (char *)sign->group->sg_name));
tv_dict_add_nr(d, S_LEN("lnum"), sign->lnum);
tv_dict_add_str(d, S_LEN("name"), (char *)sign_typenr2name(sign->typenr));
tv_dict_add_nr(d, S_LEN("priority"), sign->priority);

return d;
}




static void sign_sort_by_prio_on_line(buf_T *buf, signlist_T *sign)
FUNC_ATTR_NONNULL_ALL
{


if ((sign->prev == NULL
|| sign->prev->lnum != sign->lnum
|| sign->prev->priority > sign->priority)
&& (sign->next == NULL
|| sign->next->lnum != sign->lnum
|| sign->next->priority < sign->priority)) {
return;
}





signlist_T *p = sign;
while (p->prev != NULL
&& p->prev->lnum == sign->lnum
&& p->prev->priority <= sign->priority) {
p = p->prev;
}
if (p == sign) {


p = sign->next;
while (p->next != NULL
&& p->next->lnum == sign->lnum
&& p->next->priority > sign->priority) {
p = p->next;
}
}


if (buf->b_signlist == sign) {
buf->b_signlist = sign->next;
}
if (sign->prev != NULL) {
sign->prev->next = sign->next;
}
if (sign->next != NULL) {
sign->next->prev = sign->prev;
}
sign->prev = NULL;
sign->next = NULL;


if (p->priority <= sign->priority) {

sign->prev = p->prev;
sign->next = p;
p->prev = sign;
if (sign->prev != NULL) {
sign->prev->next = sign;
}
if (buf->b_signlist == p) {
buf->b_signlist = sign;
}
} else {

sign->prev = p;
sign->next = p->next;
p->next = sign;
if (sign->next != NULL) {
sign->next->prev = sign;
}
}
}



void buf_addsign(
buf_T *buf, 
int id, 
const char_u *groupname, 
int prio, 
linenr_T lnum, 
int typenr 
)
{
signlist_T *sign; 
signlist_T *prev; 

prev = NULL;
FOR_ALL_SIGNS_IN_BUF(buf, sign) {
if (lnum == sign->lnum && id == sign->id
&& sign_in_group(sign, groupname)) {

sign->typenr = typenr;
sign->priority = prio;
sign_sort_by_prio_on_line(buf, sign);
return;
} else if (lnum < sign->lnum) {
insert_sign_by_lnum_prio(buf, prev, id, groupname, prio, lnum, typenr);
return;
}
prev = sign;
}

insert_sign_by_lnum_prio(buf, prev, id, groupname, prio, lnum, typenr);
}



linenr_T buf_change_sign_type(
buf_T *buf, 
int markId, 
const char_u *group, 
int typenr 
)
{
signlist_T *sign; 

FOR_ALL_SIGNS_IN_BUF(buf, sign) {
if (sign->id == markId && sign_in_group(sign, group)) {
sign->typenr = typenr;
return sign->lnum;
}
}

return (linenr_T)0;
}














int buf_getsigntype(buf_T *buf, linenr_T lnum, SignType type,
int idx, int max_signs)
{
signlist_T *sign; 
signlist_T *matches[9];
int nr_matches = 0;

FOR_ALL_SIGNS_IN_BUF(buf, sign) {
if (sign->lnum == lnum
&& (type == SIGN_ANY
|| (type == SIGN_TEXT
&& sign_get_text(sign->typenr) != NULL)
|| (type == SIGN_LINEHL
&& sign_get_attr(sign->typenr, SIGN_LINEHL) != 0)
|| (type == SIGN_NUMHL
&& sign_get_attr(sign->typenr, SIGN_NUMHL) != 0))) {
matches[nr_matches] = sign;
nr_matches++;


if (nr_matches == ARRAY_SIZE(matches) || nr_matches >= max_signs) {
break;
}
}
}

if (nr_matches > 0) {
if (idx >= nr_matches) {
return 0;
}

return matches[nr_matches - idx -1]->typenr;
}

return 0;
}









linenr_T buf_delsign(
buf_T *buf, 
linenr_T atlnum, 
int id, 
char_u *group 
)
{
signlist_T **lastp; 
signlist_T *sign; 
signlist_T *next; 
linenr_T lnum; 

buf->b_signcols_max = -1;
lastp = &buf->b_signlist;
lnum = 0;
for (sign = buf->b_signlist; sign != NULL; sign = next) {
next = sign->next;
if ((id == 0 || sign->id == id)
&& (atlnum == 0 || sign->lnum == atlnum)
&& sign_in_group(sign, group)) {
*lastp = next;
if (next != NULL) {
next->prev = sign->prev;
}
lnum = sign->lnum;
if (sign->group != NULL) {
sign_group_unref(sign->group->sg_name);
}
xfree(sign);
redraw_buf_line_later(buf, lnum);




if (group == NULL
|| (*group != '*' && id != 0)
|| (*group == '*' && atlnum != 0)) {
break;
}
} else {
lastp = &sign->next;
}
}



if (buf->b_signlist == NULL) {
redraw_buf_later(buf, NOT_VALID);
changed_line_abv_curs();
}

return lnum;
}





int buf_findsign(
buf_T *buf, 
int id, 
char_u *group 
)
{
signlist_T *sign; 

FOR_ALL_SIGNS_IN_BUF(buf, sign) {
if (sign->id == id && sign_in_group(sign, group)) {
return (int)sign->lnum;
}
}

return 0;
}



static signlist_T * buf_getsign_at_line(
buf_T *buf, 
linenr_T lnum, 
char_u *groupname 
)
{
signlist_T *sign; 

FOR_ALL_SIGNS_IN_BUF(buf, sign) {
if (sign->lnum == lnum && sign_in_group(sign, groupname)) {
return sign;
}
}

return NULL;
}


int buf_findsign_id(
buf_T *buf, 
linenr_T lnum, 
char_u *groupname 
)
{
signlist_T *sign; 

sign = buf_getsign_at_line(buf, lnum, groupname);
if (sign != NULL) {
return sign->id;
}

return 0;
}


void buf_delete_signs(buf_T *buf, char_u *group)
{
signlist_T *sign;
signlist_T **lastp; 
signlist_T *next;



if (buf->b_signlist != NULL && curwin != NULL) {
changed_line_abv_curs();
}

lastp = &buf->b_signlist;
for (sign = buf->b_signlist; sign != NULL; sign = next) {
next = sign->next;
if (sign_in_group(sign, group)) {
*lastp = next;
if (next != NULL) {
next->prev = sign->prev;
}
if (sign->group != NULL) {
sign_group_unref(sign->group->sg_name);
}
xfree(sign);
} else {
lastp = &sign->next;
}
}
buf->b_signcols_max = -1;
}


void sign_list_placed(buf_T *rbuf, char_u *sign_group)
{
buf_T *buf;
signlist_T *sign;
char lbuf[MSG_BUF_LEN];
char group[MSG_BUF_LEN];

MSG_PUTS_TITLE(_("\n--- Signs ---"));
msg_putchar('\n');
if (rbuf == NULL) {
buf = firstbuf;
} else {
buf = rbuf;
}
while (buf != NULL && !got_int) {
if (buf->b_signlist != NULL) {
vim_snprintf(lbuf, MSG_BUF_LEN, _("Signs for %s:"), buf->b_fname);
MSG_PUTS_ATTR(lbuf, HL_ATTR(HLF_D));
msg_putchar('\n');
}
FOR_ALL_SIGNS_IN_BUF(buf, sign) {
if (got_int) {
break;
}
if (!sign_in_group(sign, sign_group)) {
continue;
}
if (sign->group != NULL) {
vim_snprintf(group, MSG_BUF_LEN, _(" group=%s"),
sign->group->sg_name);
} else {
group[0] = '\0';
}
vim_snprintf(lbuf, MSG_BUF_LEN,
_(" line=%ld id=%d%s name=%s priority=%d"),
(long)sign->lnum, sign->id, group,
sign_typenr2name(sign->typenr), sign->priority);
MSG_PUTS(lbuf);
msg_putchar('\n');
}
if (rbuf != NULL) {
break;
}
buf = buf->b_next;
}
}


void sign_mark_adjust(
linenr_T line1,
linenr_T line2,
long amount,
long amount_after
)
{
signlist_T *sign; 
linenr_T new_lnum; 

curbuf->b_signcols_max = -1;

FOR_ALL_SIGNS_IN_BUF(curbuf, sign) {
new_lnum = sign->lnum;
if (sign->lnum >= line1 && sign->lnum <= line2) {
if (amount != MAXLNUM) {
new_lnum += amount;
}
} else if (sign->lnum > line2) {
new_lnum += amount_after;
}



if (sign->lnum >= line1 && new_lnum <= curbuf->b_ml.ml_line_count) {
sign->lnum = new_lnum;
}
}
}



static int sign_cmd_idx(
char_u *begin_cmd, 
char_u *end_cmd 
)
{
int idx;
char_u save = *end_cmd;

*end_cmd = (char_u)NUL;
for (idx = 0; ; idx++) {
if (cmds[idx] == NULL || STRCMP(begin_cmd, cmds[idx]) == 0) {
break;
}
}
*end_cmd = save;
return idx;
}


static sign_T * sign_find(const char_u *name, sign_T **sp_prev)
{
sign_T *sp;

if (sp_prev != NULL) {
*sp_prev = NULL;
}
for (sp = first_sign; sp != NULL; sp = sp->sn_next) {
if (STRCMP(sp->sn_name, name) == 0) {
break;
}
if (sp_prev != NULL) {
*sp_prev = sp;
}
}

return sp;
}


static sign_T * alloc_new_sign(char_u *name)
{
sign_T *sp;
sign_T *lp;
int start = next_sign_typenr;


sp = xcalloc(1, sizeof(sign_T));




for (lp = first_sign; lp != NULL; ) {
if (lp->sn_typenr == next_sign_typenr) {
next_sign_typenr++;
if (next_sign_typenr == MAX_TYPENR) {
next_sign_typenr = 1;
}
if (next_sign_typenr == start) {
xfree(sp);
EMSG(_("E612: Too many signs defined"));
return NULL;
}
lp = first_sign; 
continue;
}
lp = lp->sn_next;
}

sp->sn_typenr = next_sign_typenr;
if (++next_sign_typenr == MAX_TYPENR) {
next_sign_typenr = 1; 
}

sp->sn_name = vim_strsave(name);

return sp;
}


static void sign_define_init_icon(sign_T *sp, char_u *icon)
{
xfree(sp->sn_icon);
sp->sn_icon = vim_strsave(icon);
backslash_halve(sp->sn_icon);
}


static int sign_define_init_text(sign_T *sp, char_u *text)
{
char_u *s;
char_u *endp;
int cells;
size_t len;

endp = text + (int)STRLEN(text);
for (s = text; s + 1 < endp; s++) {
if (*s == '\\') {


STRMOVE(s, s + 1);
endp--;
}
}

cells = 0;
for (s = text; s < endp; s += (*mb_ptr2len)(s)) {
if (!vim_isprintc(utf_ptr2char(s))) {
break;
}
cells += utf_ptr2cells(s);
}

if (s != endp || cells < 1 || cells > 2) {
EMSG2(_("E239: Invalid sign text: %s"), text);
return FAIL;
}

xfree(sp->sn_text);


len = (size_t)(endp - text + ((cells == 1) ? 1 : 0));
sp->sn_text = vim_strnsave(text, len);

if (cells == 1) {
STRCPY(sp->sn_text + len - 1, " ");
}

return OK;
}


int sign_define_by_name(
char_u *name,
char_u *icon,
char_u *linehl,
char_u *text,
char_u *texthl,
char_u *numhl
)
{
sign_T *sp_prev;
sign_T *sp;

sp = sign_find(name, &sp_prev);
if (sp == NULL) {
sp = alloc_new_sign(name);
if (sp == NULL) {
return FAIL;
}


if (sp_prev == NULL) {
first_sign = sp;
} else {
sp_prev->sn_next = sp;
}
} else {


FOR_ALL_WINDOWS_IN_TAB(wp, curtab) {
if (wp->w_buffer->b_signlist != NULL) {
redraw_buf_later(wp->w_buffer, NOT_VALID);
}
}
}


if (icon != NULL) {
sign_define_init_icon(sp, icon);
}

if (text != NULL && (sign_define_init_text(sp, text) == FAIL)) {
return FAIL;
}

if (linehl != NULL) {
sp->sn_line_hl = syn_check_group(linehl, (int)STRLEN(linehl));
}

if (texthl != NULL) {
sp->sn_text_hl = syn_check_group(texthl, (int)STRLEN(texthl));
}

if (numhl != NULL) {
sp->sn_num_hl = syn_check_group(numhl, (int)STRLEN(numhl));
}

return OK;
}


int sign_undefine_by_name(const char_u *name)
{
sign_T *sp_prev;
sign_T *sp;

sp = sign_find(name, &sp_prev);
if (sp == NULL) {
EMSG2(_("E155: Unknown sign: %s"), name);
return FAIL;
}
sign_undefine(sp, sp_prev);

return OK;
}


static void sign_list_by_name(char_u *name)
{
sign_T *sp;

sp = sign_find(name, NULL);
if (sp != NULL) {
sign_list_defined(sp);
} else {
EMSG2(_("E155: Unknown sign: %s"), name);
}
}



int sign_place(
int *sign_id,
const char_u *sign_group,
const char_u *sign_name,
buf_T *buf,
linenr_T lnum,
int prio
)
{
sign_T *sp;


if (sign_group != NULL && (*sign_group == '*' || *sign_group == '\0')) {
return FAIL;
}

for (sp = first_sign; sp != NULL; sp = sp->sn_next) {
if (STRCMP(sp->sn_name, sign_name) == 0) {
break;
}
}
if (sp == NULL) {
EMSG2(_("E155: Unknown sign: %s"), sign_name);
return FAIL;
}
if (*sign_id == 0) {
*sign_id = sign_group_get_next_signid(buf, sign_group);
}

if (lnum > 0) {


buf_addsign(buf, *sign_id, sign_group, prio, lnum, sp->sn_typenr);
} else {

lnum = buf_change_sign_type(buf, *sign_id, sign_group, sp->sn_typenr);
}
if (lnum > 0) {
redraw_buf_line_later(buf, lnum);
} else {
EMSG2(_("E885: Not possible to change sign %s"), sign_name);
return FAIL;
}

return OK;
}


int sign_unplace(int sign_id, char_u *sign_group, buf_T *buf, linenr_T atlnum)
{
if (buf->b_signlist == NULL) { 
return OK;
}
if (sign_id == 0) {

redraw_buf_later(buf, NOT_VALID);
buf_delete_signs(buf, sign_group);
} else {
linenr_T lnum;


lnum = buf_delsign(buf, atlnum, sign_id, sign_group);
if (lnum == 0) {
return FAIL;
}
redraw_buf_line_later(buf, lnum);
}

return OK;
}


static void sign_unplace_at_cursor(char_u *groupname)
{
int id = -1;

id = buf_findsign_id(curwin->w_buffer, curwin->w_cursor.lnum, groupname);
if (id > 0) {
sign_unplace(id, groupname, curwin->w_buffer, curwin->w_cursor.lnum);
} else {
EMSG(_("E159: Missing sign number"));
}
}


linenr_T sign_jump(int sign_id, char_u *sign_group, buf_T *buf)
{
linenr_T lnum;

if ((lnum = buf_findsign(buf, sign_id, sign_group)) <= 0) {
EMSGN(_("E157: Invalid sign ID: %" PRId64), sign_id);
return -1;
}


if (buf_jump_open_win(buf) != NULL) { 
curwin->w_cursor.lnum = lnum;
check_cursor_lnum();
beginline(BL_WHITE);
} else { 
if (buf->b_fname == NULL) {
EMSG(_("E934: Cannot jump to a buffer that does not have a name"));
return -1;
}
size_t cmdlen = STRLEN(buf->b_fname) + 24;
char *cmd = xmallocz(cmdlen);
snprintf(cmd, cmdlen, "e +%" PRId64 " %s",
(int64_t)lnum, buf->b_fname);
do_cmdline_cmd(cmd);
xfree(cmd);
}

foldOpenCursor();

return lnum;
}


static void sign_define_cmd(char_u *sign_name, char_u *cmdline)
{
char_u *arg;
char_u *p = cmdline;
char_u *icon = NULL;
char_u *text = NULL;
char_u *linehl = NULL;
char_u *texthl = NULL;
char_u *numhl = NULL;
int failed = false;


for (;;) {
arg = skipwhite(p);
if (*arg == NUL) {
break;
}
p = skiptowhite_esc(arg);
if (STRNCMP(arg, "icon=", 5) == 0) {
arg += 5;
icon = vim_strnsave(arg, (size_t)(p - arg));
} else if (STRNCMP(arg, "text=", 5) == 0) {
arg += 5;
text = vim_strnsave(arg, (size_t)(p - arg));
} else if (STRNCMP(arg, "linehl=", 7) == 0) {
arg += 7;
linehl = vim_strnsave(arg, (size_t)(p - arg));
} else if (STRNCMP(arg, "texthl=", 7) == 0) {
arg += 7;
texthl = vim_strnsave(arg, (size_t)(p - arg));
} else if (STRNCMP(arg, "numhl=", 6) == 0) {
arg += 6;
numhl = vim_strnsave(arg, (size_t)(p - arg));
} else {
EMSG2(_(e_invarg2), arg);
failed = true;
break;
}
}

if (!failed) {
sign_define_by_name(sign_name, icon, linehl, text, texthl, numhl);
}

xfree(icon);
xfree(text);
xfree(linehl);
xfree(texthl);
xfree(numhl);
}


static void sign_place_cmd(
buf_T *buf,
linenr_T lnum,
char_u *sign_name,
int id,
char_u *group,
int prio
)
{
if (id <= 0) {










if (lnum >= 0 || sign_name != NULL
|| (group != NULL && *group == '\0')) {
EMSG(_(e_invarg));
} else {
sign_list_placed(buf, group);
}
} else {

if (sign_name == NULL || buf == NULL
|| (group != NULL && *group == '\0')) {
EMSG(_(e_invarg));
return;
}

sign_place(&id, group, sign_name, buf, lnum, prio);
}
}


static void sign_unplace_cmd(
buf_T *buf,
linenr_T lnum,
char_u *sign_name,
int id,
char_u *group
)
{
if (lnum >= 0 || sign_name != NULL || (group != NULL && *group == '\0')) {
EMSG(_(e_invarg));
return;
}

if (id == -2) {
if (buf != NULL) {






sign_unplace(0, group, buf, 0);
} else {



FOR_ALL_BUFFERS(cbuf) {
if (cbuf->b_signlist != NULL) {
buf_delete_signs(cbuf, group);
}
}
}
} else {
if (buf != NULL) {






sign_unplace(id, group, buf, 0);
} else {
if (id == -1) {


sign_unplace_at_cursor(group);
} else {



FOR_ALL_BUFFERS(cbuf) {
sign_unplace(id, group, cbuf, 0);
}
}
}
}
}






static void sign_jump_cmd(
buf_T *buf,
linenr_T lnum,
char_u *sign_name,
int id,
char_u *group
)
{
if (sign_name == NULL && group == NULL && id == -1) {
EMSG(_(e_argreq));
return;
}

if (buf == NULL || (group != NULL && *group == '\0')
|| lnum >= 0 || sign_name != NULL) {


EMSG(_(e_invarg));
return;
}

(void)sign_jump(id, group, buf);
}





static int parse_sign_cmd_args(
int cmd,
char_u *arg,
char_u **sign_name,
int *signid,
char_u **group,
int *prio,
buf_T **buf,
linenr_T *lnum
)
{
char_u *arg1;
char_u *name;
char_u *filename = NULL;
int lnum_arg = false;


arg1 = arg;
if (ascii_isdigit(*arg)) {
*signid = getdigits_int(&arg, true, 0);
if (!ascii_iswhite(*arg) && *arg != NUL) {
*signid = -1;
arg = arg1;
} else {
arg = skipwhite(arg);
}
}

while (*arg != NUL) {
if (STRNCMP(arg, "line=", 5) == 0) {
arg += 5;
*lnum = atoi((char *)arg);
arg = skiptowhite(arg);
lnum_arg = true;
} else if (STRNCMP(arg, "*", 1) == 0 && cmd == SIGNCMD_UNPLACE) {
if (*signid != -1) {
EMSG(_(e_invarg));
return FAIL;
}
*signid = -2;
arg = skiptowhite(arg + 1);
} else if (STRNCMP(arg, "name=", 5) == 0) {
arg += 5;
name = arg;
arg = skiptowhite(arg);
if (*arg != NUL) {
*arg++ = NUL;
}
while (name[0] == '0' && name[1] != NUL) {
name++;
}
*sign_name = name;
} else if (STRNCMP(arg, "group=", 6) == 0) {
arg += 6;
*group = arg;
arg = skiptowhite(arg);
if (*arg != NUL) {
*arg++ = NUL;
}
} else if (STRNCMP(arg, "priority=", 9) == 0) {
arg += 9;
*prio = atoi((char *)arg);
arg = skiptowhite(arg);
} else if (STRNCMP(arg, "file=", 5) == 0) {
arg += 5;
filename = arg;
*buf = buflist_findname_exp(arg);
break;
} else if (STRNCMP(arg, "buffer=", 7) == 0) {
arg += 7;
filename = arg;
*buf = buflist_findnr(getdigits_int(&arg, true, 0));
if (*skipwhite(arg) != NUL) {
EMSG(_(e_trailing));
}
break;
} else {
EMSG(_(e_invarg));
return FAIL;
}
arg = skipwhite(arg);
}

if (filename != NULL && *buf == NULL) {
EMSG2(_("E158: Invalid buffer name: %s"), filename);
return FAIL;
}



if (filename == NULL && ((cmd == SIGNCMD_PLACE && lnum_arg)
|| cmd == SIGNCMD_JUMP)) {
*buf = curwin->w_buffer;
}
return OK;
}


void ex_sign(exarg_T *eap)
{
char_u *arg = eap->arg;
char_u *p;
int idx;
sign_T *sp;


p = skiptowhite(arg);
idx = sign_cmd_idx(arg, p);
if (idx == SIGNCMD_LAST) {
EMSG2(_("E160: Unknown sign command: %s"), arg);
return;
}
arg = skipwhite(p);

if (idx <= SIGNCMD_LIST) {

if (idx == SIGNCMD_LIST && *arg == NUL) {

for (sp = first_sign; sp != NULL && !got_int; sp = sp->sn_next) {
sign_list_defined(sp);
}
} else if (*arg == NUL) {
EMSG(_("E156: Missing sign name"));
} else {
char_u *name;



p = skiptowhite(arg);
if (*p != NUL) {
*p++ = NUL;
}
while (arg[0] == '0' && arg[1] != NUL) {
arg++;
}
name = vim_strsave(arg);

if (idx == SIGNCMD_DEFINE) {
sign_define_cmd(name, p);
} else if (idx == SIGNCMD_LIST) {

sign_list_by_name(name);
} else {

sign_undefine_by_name(name);
}

xfree(name);
return;
}
} else {
int id = -1;
linenr_T lnum = -1;
char_u *sign_name = NULL;
char_u *group = NULL;
int prio = SIGN_DEF_PRIO;
buf_T *buf = NULL;


if (parse_sign_cmd_args(idx, arg, &sign_name, &id, &group, &prio,
&buf, &lnum) == FAIL) {
return;
}

if (idx == SIGNCMD_PLACE) {
sign_place_cmd(buf, lnum, sign_name, id, group, prio);
} else if (idx == SIGNCMD_UNPLACE) {
sign_unplace_cmd(buf, lnum, sign_name, id, group);
} else if (idx == SIGNCMD_JUMP) {
sign_jump_cmd(buf, lnum, sign_name, id, group);
}
}
}


static void sign_getinfo(sign_T *sp, dict_T *retdict)
{
const char *p;

tv_dict_add_str(retdict, S_LEN("name"), (char *)sp->sn_name);
if (sp->sn_icon != NULL) {
tv_dict_add_str(retdict, S_LEN("icon"), (char *)sp->sn_icon);
}
if (sp->sn_text != NULL) {
tv_dict_add_str(retdict, S_LEN("text"), (char *)sp->sn_text);
}
if (sp->sn_line_hl > 0) {
p = get_highlight_name_ext(NULL, sp->sn_line_hl - 1, false);
if (p == NULL) {
p = "NONE";
}
tv_dict_add_str(retdict, S_LEN("linehl"), (char *)p);
}
if (sp->sn_text_hl > 0) {
p = get_highlight_name_ext(NULL, sp->sn_text_hl - 1, false);
if (p == NULL) {
p = "NONE";
}
tv_dict_add_str(retdict, S_LEN("texthl"), (char *)p);
}
if (sp->sn_num_hl > 0) {
p = get_highlight_name_ext(NULL, sp->sn_num_hl - 1, false);
if (p == NULL) {
p = "NONE";
}
tv_dict_add_str(retdict, S_LEN("numhl"), (char *)p);
}
}



void sign_getlist(const char_u *name, list_T *retlist)
{
sign_T *sp = first_sign;
dict_T *dict;

if (name != NULL) {
sp = sign_find(name, NULL);
if (sp == NULL) {
return;
}
}

for (; sp != NULL && !got_int; sp = sp->sn_next) {
dict = tv_dict_alloc();
tv_list_append_dict(retlist, dict);
sign_getinfo(sp, dict);

if (name != NULL) { 
break;
}
}
}


list_T *get_buffer_signs(buf_T *buf)
FUNC_ATTR_NONNULL_RET FUNC_ATTR_NONNULL_ALL FUNC_ATTR_WARN_UNUSED_RESULT
{
signlist_T *sign;
dict_T *d;
list_T *const l = tv_list_alloc(kListLenMayKnow);

FOR_ALL_SIGNS_IN_BUF(buf, sign) {
d = sign_get_info(sign);
tv_list_append_dict(l, d);
}
return l;
}


static void sign_get_placed_in_buf(
buf_T *buf,
linenr_T lnum,
int sign_id,
const char_u *sign_group,
list_T *retlist)
{
dict_T *d;
list_T *l;
signlist_T *sign;

d = tv_dict_alloc();
tv_list_append_dict(retlist, d);

tv_dict_add_nr(d, S_LEN("bufnr"), (long)buf->b_fnum);

l = tv_list_alloc(kListLenMayKnow);
tv_dict_add_list(d, S_LEN("signs"), l);

FOR_ALL_SIGNS_IN_BUF(buf, sign) {
if (!sign_in_group(sign, sign_group)) {
continue;
}
if ((lnum == 0 && sign_id == 0)
|| (sign_id == 0 && lnum == sign->lnum)
|| (lnum == 0 && sign_id == sign->id)
|| (lnum == sign->lnum && sign_id == sign->id)) {
tv_list_append_dict(l, sign_get_info(sign));
}
}
}




void sign_get_placed(
buf_T *buf,
linenr_T lnum,
int sign_id,
const char_u *sign_group,
list_T *retlist
)
{
if (buf != NULL) {
sign_get_placed_in_buf(buf, lnum, sign_id, sign_group, retlist);
} else {
FOR_ALL_BUFFERS(cbuf) {
if (cbuf->b_signlist != NULL) {
sign_get_placed_in_buf(cbuf, 0, sign_id, sign_group, retlist);
}
}
}
}


static void sign_list_defined(sign_T *sp)
{
smsg("sign %s", sp->sn_name);
if (sp->sn_icon != NULL) {
msg_puts(" icon=");
msg_outtrans(sp->sn_icon);
msg_puts(_(" (not supported)"));
}
if (sp->sn_text != NULL) {
msg_puts(" text=");
msg_outtrans(sp->sn_text);
}
if (sp->sn_line_hl > 0) {
msg_puts(" linehl=");
const char *const p = get_highlight_name_ext(NULL,
sp->sn_line_hl - 1, false);
if (p == NULL) {
msg_puts("NONE");
} else {
msg_puts(p);
}
}
if (sp->sn_text_hl > 0) {
msg_puts(" texthl=");
const char *const p = get_highlight_name_ext(NULL,
sp->sn_text_hl - 1, false);
if (p == NULL) {
msg_puts("NONE");
} else {
msg_puts(p);
}
}
if (sp->sn_num_hl > 0) {
msg_puts(" numhl=");
const char *const p = get_highlight_name_ext(NULL,
sp->sn_num_hl - 1, false);
if (p == NULL) {
msg_puts("NONE");
} else {
msg_puts(p);
}
}
}


static void sign_undefine(sign_T *sp, sign_T *sp_prev)
{
xfree(sp->sn_name);
xfree(sp->sn_icon);
xfree(sp->sn_text);
if (sp_prev == NULL) {
first_sign = sp->sn_next;
} else {
sp_prev->sn_next = sp->sn_next;
}
xfree(sp);
}


int sign_get_attr(int typenr, SignType type)
{
sign_T *sp;
int sign_hl = 0;

for (sp = first_sign; sp != NULL; sp = sp->sn_next) {
if (sp->sn_typenr == typenr) {
switch (type) {
case SIGN_TEXT:
sign_hl = sp->sn_text_hl;
break;
case SIGN_LINEHL:
sign_hl = sp->sn_line_hl;
break;
case SIGN_NUMHL:
sign_hl = sp->sn_num_hl;
break;
default:
abort();
}
if (sign_hl > 0) {
return syn_id2attr(sign_hl);
}
break;
}
}
return 0;
}



char_u * sign_get_text(int typenr)
{
sign_T *sp;

for (sp = first_sign; sp != NULL; sp = sp->sn_next) {
if (sp->sn_typenr == typenr) {
return sp->sn_text;
}
}
return NULL;
}


void free_signs(void)
{
while (first_sign != NULL) {
sign_undefine(first_sign, NULL);
}
}

static enum
{
EXP_SUBCMD, 
EXP_DEFINE, 
EXP_PLACE, 
EXP_LIST, 
EXP_UNPLACE, 
EXP_SIGN_NAMES, 
EXP_SIGN_GROUPS, 
} expand_what;


static char_u *get_nth_sign_name(int idx)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{

int current_idx = 0;
for (sign_T *sp = first_sign; sp != NULL; sp = sp->sn_next) {
if (current_idx++ == idx) {
return sp->sn_name;
}
}
return NULL;
}


static char_u *get_nth_sign_group_name(int idx)
{

int current_idx = 0;
int todo = (int)sg_table.ht_used;
for (hashitem_T *hi = sg_table.ht_array; todo > 0; hi++) {
if (!HASHITEM_EMPTY(hi)) {
todo--;
if (current_idx++ == idx) {
signgroup_T *const group = HI2SG(hi);
return group->sg_name;
}
}
}
return NULL;
}



char_u * get_sign_name(expand_T *xp, int idx)
{
switch (expand_what) {
case EXP_SUBCMD:
return (char_u *)cmds[idx];
case EXP_DEFINE: {
char *define_arg[] = { "icon=", "linehl=", "text=", "texthl=", "numhl=",
NULL };
return (char_u *)define_arg[idx];
}
case EXP_PLACE: {
char *place_arg[] = { "line=", "name=", "group=", "priority=", "file=",
"buffer=", NULL };
return (char_u *)place_arg[idx];
}
case EXP_LIST: {
char *list_arg[] = { "group=", "file=", "buffer=", NULL };
return (char_u *)list_arg[idx];
}
case EXP_UNPLACE: {
char *unplace_arg[] = { "group=", "file=", "buffer=", NULL };
return (char_u *)unplace_arg[idx];
}
case EXP_SIGN_NAMES:
return get_nth_sign_name(idx);
case EXP_SIGN_GROUPS:
return get_nth_sign_group_name(idx);
default:
return NULL;
}
}


void set_context_in_sign_cmd(expand_T *xp, char_u *arg)
{
char_u *end_subcmd;
char_u *last;
int cmd_idx;
char_u *begin_subcmd_args;


xp->xp_context = EXPAND_SIGN;
expand_what = EXP_SUBCMD;
xp->xp_pattern = arg;

end_subcmd = skiptowhite(arg);
if (*end_subcmd == NUL) {


return;
}

cmd_idx = sign_cmd_idx(arg, end_subcmd);




begin_subcmd_args = skipwhite(end_subcmd);








char_u *p = begin_subcmd_args;
do {
p = skipwhite(p);
last = p;
p = skiptowhite(p);
} while (*p != NUL);

p = vim_strchr(last, '=');




if (p == NULL) {

xp->xp_pattern = last;
switch (cmd_idx) {
case SIGNCMD_DEFINE:
expand_what = EXP_DEFINE;
break;
case SIGNCMD_PLACE:

if (ascii_isdigit(*begin_subcmd_args)) {

expand_what = EXP_PLACE;
} else {

expand_what = EXP_LIST;
}
break;
case SIGNCMD_LIST:
case SIGNCMD_UNDEFINE:


expand_what = EXP_SIGN_NAMES;
break;
case SIGNCMD_JUMP:
case SIGNCMD_UNPLACE:
expand_what = EXP_UNPLACE;
break;
default:
xp->xp_context = EXPAND_NOTHING;
}
} else {

xp->xp_pattern = p + 1;
switch (cmd_idx) {
case SIGNCMD_DEFINE:
if (STRNCMP(last, "texthl", 6) == 0
|| STRNCMP(last, "linehl", 6) == 0
|| STRNCMP(last, "numhl", 5) == 0) {
xp->xp_context = EXPAND_HIGHLIGHT;
} else if (STRNCMP(last, "icon", 4) == 0) {
xp->xp_context = EXPAND_FILES;
} else {
xp->xp_context = EXPAND_NOTHING;
}
break;
case SIGNCMD_PLACE:
if (STRNCMP(last, "name", 4) == 0) {
expand_what = EXP_SIGN_NAMES;
} else if (STRNCMP(last, "group", 5) == 0) {
expand_what = EXP_SIGN_GROUPS;
} else if (STRNCMP(last, "file", 4) == 0) {
xp->xp_context = EXPAND_BUFFERS;
} else {
xp->xp_context = EXPAND_NOTHING;
}
break;
case SIGNCMD_UNPLACE:
case SIGNCMD_JUMP:
if (STRNCMP(last, "group", 5) == 0) {
expand_what = EXP_SIGN_GROUPS;
} else if (STRNCMP(last, "file", 4) == 0) {
xp->xp_context = EXPAND_BUFFERS;
} else {
xp->xp_context = EXPAND_NOTHING;
}
break;
default:
xp->xp_context = EXPAND_NOTHING;
}
}
}

