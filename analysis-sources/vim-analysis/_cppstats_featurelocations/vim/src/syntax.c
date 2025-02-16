












#include "vim.h"

#if defined(FEAT_SYN_HL) || defined(PROTO)

#define SYN_NAMELEN 50 


#define SPO_MS_OFF 0 
#define SPO_ME_OFF 1 
#define SPO_HS_OFF 2 
#define SPO_HE_OFF 3 
#define SPO_RS_OFF 4 
#define SPO_RE_OFF 5 
#define SPO_LC_OFF 6 
#define SPO_COUNT 7

static char *(spo_name_tab[SPO_COUNT]) =
{"ms=", "me=", "hs=", "he=", "rs=", "re=", "lc="};














typedef struct syn_pattern
{
char sp_type; 
char sp_syncing; 
short sp_syn_match_id; 
short sp_off_flags; 
int sp_offsets[SPO_COUNT]; 
int sp_flags; 
#if defined(FEAT_CONCEAL)
int sp_cchar; 
#endif
int sp_ic; 
int sp_sync_idx; 
int sp_line_id; 
int sp_startcol; 
short *sp_cont_list; 
short *sp_next_list; 
struct sp_syn sp_syn; 
char_u *sp_pattern; 
regprog_T *sp_prog; 
#if defined(FEAT_PROFILE)
syn_time_T sp_time;
#endif
} synpat_T;






#define SPTYPE_MATCH 1 
#define SPTYPE_START 2 
#define SPTYPE_END 3 
#define SPTYPE_SKIP 4 


#define SYN_ITEMS(buf) ((synpat_T *)((buf)->b_syn_patterns.ga_data))

#define NONE_IDX -2 




#define SF_CCOMMENT 0x01 
#define SF_MATCH 0x02 

#define SYN_STATE_P(ssp) ((bufstate_T *)((ssp)->ga_data))

#define MAXKEYWLEN 80 




static int current_attr = 0; 
#if defined(FEAT_EVAL)
static int current_id = 0; 
static int current_trans_id = 0; 
#endif
#if defined(FEAT_CONCEAL)
static int current_flags = 0;
static int current_seqnr = 0;
static int current_sub_char = 0;
#endif

typedef struct syn_cluster_S
{
char_u *scl_name; 
char_u *scl_name_u; 
short *scl_list; 
} syn_cluster_T;




#define CLUSTER_REPLACE 1 
#define CLUSTER_ADD 2 
#define CLUSTER_SUBTRACT 3 

#define SYN_CLSTR(buf) ((syn_cluster_T *)((buf)->b_syn_clusters.ga_data))









#define SYNID_ALLBUT MAX_HL_ID 
#define SYNID_TOP 21000 
#define SYNID_CONTAINED 22000 
#define SYNID_CLUSTER 23000 

#define MAX_SYN_INC_TAG 999 
#define MAX_CLUSTER_ID (32767 - SYNID_CLUSTER)






static char_u **syn_cmdlinep;






static int current_syn_inc_tag = 0;
static int running_syn_inc_tag = 0;








static keyentry_T dumkey;
#define KE2HIKEY(kp) ((kp)->keyword)
#define HIKEY2KE(p) ((keyentry_T *)((p) - (dumkey.keyword - (char_u *)&dumkey)))
#define HI2KE(hi) HIKEY2KE((hi)->hi_key)






static int keepend_level = -1;

static char msg_no_items[] = N_("No Syntax items defined for this buffer");






typedef struct state_item
{
int si_idx; 

int si_id; 
int si_trans_id; 
int si_m_lnum; 
int si_m_startcol; 
lpos_T si_m_endpos; 
lpos_T si_h_startpos; 
lpos_T si_h_endpos; 
lpos_T si_eoe_pos; 
int si_end_idx; 
int si_ends; 
int si_attr; 
long si_flags; 

#if defined(FEAT_CONCEAL)
int si_seqnr; 
int si_cchar; 
#endif
short *si_cont_list; 
short *si_next_list; 
reg_extmatch_T *si_extmatch; 

} stateitem_T;

#define KEYWORD_IDX -1 
#define ID_LIST_ALL (short *)-1 


#if defined(FEAT_CONCEAL)
static int next_seqnr = 1; 
#endif





typedef struct
{
int flags; 
int keyword; 
int *sync_idx; 

char has_cont_list; 
short *cont_list; 
short *cont_in_list; 
short *next_list; 
} syn_opt_arg_T;








static int next_match_col; 
static lpos_T next_match_m_endpos; 
static lpos_T next_match_h_startpos; 
static lpos_T next_match_h_endpos; 
static int next_match_idx; 
static long next_match_flags; 
static lpos_T next_match_eos_pos; 
static lpos_T next_match_eoe_pos; 
static int next_match_end_idx; 
static reg_extmatch_T *next_match_extmatch = NULL;





#define INVALID_STATE(ssp) ((ssp)->ga_itemsize == 0)
#define VALID_STATE(ssp) ((ssp)->ga_itemsize != 0)





static win_T *syn_win; 
static buf_T *syn_buf; 
static synblock_T *syn_block; 
#if defined(FEAT_RELTIME)
static proftime_T *syn_tm; 
#endif
static linenr_T current_lnum = 0; 
static colnr_T current_col = 0; 
static int current_state_stored = 0; 

static int current_finished = 0; 
static garray_T current_state 
= {0, 0, 0, 0, NULL};
static short *current_next_list = NULL; 
static int current_next_flags = 0; 
static int current_line_id = 0; 

#define CUR_STATE(idx) ((stateitem_T *)(current_state.ga_data))[idx]

static void syn_sync(win_T *wp, linenr_T lnum, synstate_T *last_valid);
static int syn_match_linecont(linenr_T lnum);
static void syn_start_line(void);
static void syn_update_ends(int startofline);
static void syn_stack_alloc(void);
static int syn_stack_cleanup(void);
static void syn_stack_free_entry(synblock_T *block, synstate_T *p);
static synstate_T *syn_stack_find_entry(linenr_T lnum);
static synstate_T *store_current_state(void);
static void load_current_state(synstate_T *from);
static void invalidate_current_state(void);
static int syn_stack_equal(synstate_T *sp);
static void validate_current_state(void);
static int syn_finish_line(int syncing);
static int syn_current_attr(int syncing, int displaying, int *can_spell, int keep_state);
static int did_match_already(int idx, garray_T *gap);
static stateitem_T *push_next_match(stateitem_T *cur_si);
static void check_state_ends(void);
static void update_si_attr(int idx);
static void check_keepend(void);
static void update_si_end(stateitem_T *sip, int startcol, int force);
static short *copy_id_list(short *list);
static int in_id_list(stateitem_T *item, short *cont_list, struct sp_syn *ssp, int contained);
static int push_current_state(int idx);
static void pop_current_state(void);
#if defined(FEAT_PROFILE)
static void syn_clear_time(syn_time_T *tt);
static void syntime_clear(void);
static void syntime_report(void);
static int syn_time_on = FALSE;
#define IF_SYN_TIME(p) (p)
#else
#define IF_SYN_TIME(p) NULL
typedef int syn_time_T;
#endif

static void syn_stack_apply_changes_block(synblock_T *block, buf_T *buf);
static void find_endpos(int idx, lpos_T *startpos, lpos_T *m_endpos, lpos_T *hl_endpos, long *flagsp, lpos_T *end_endpos, int *end_idx, reg_extmatch_T *start_ext);

static void limit_pos(lpos_T *pos, lpos_T *limit);
static void limit_pos_zero(lpos_T *pos, lpos_T *limit);
static void syn_add_end_off(lpos_T *result, regmmatch_T *regmatch, synpat_T *spp, int idx, int extra);
static void syn_add_start_off(lpos_T *result, regmmatch_T *regmatch, synpat_T *spp, int idx, int extra);
static char_u *syn_getcurline(void);
static int syn_regexec(regmmatch_T *rmp, linenr_T lnum, colnr_T col, syn_time_T *st);
static int check_keyword_id(char_u *line, int startcol, int *endcol, long *flags, short **next_list, stateitem_T *cur_si, int *ccharp);
static void syn_remove_pattern(synblock_T *block, int idx);
static void syn_clear_pattern(synblock_T *block, int i);
static void syn_clear_cluster(synblock_T *block, int i);
static void syn_clear_one(int id, int syncing);
static void syn_cmd_onoff(exarg_T *eap, char *name);
static void syn_lines_msg(void);
static void syn_match_msg(void);
static void syn_list_one(int id, int syncing, int link_only);
static void syn_list_cluster(int id);
static void put_id_list(char_u *name, short *list, int attr);
static void put_pattern(char *s, int c, synpat_T *spp, int attr);
static int syn_list_keywords(int id, hashtab_T *ht, int did_header, int attr);
static void syn_clear_keyword(int id, hashtab_T *ht);
static void clear_keywtab(hashtab_T *ht);
static int syn_scl_namen2id(char_u *linep, int len);
static int syn_check_cluster(char_u *pp, int len);
static int syn_add_cluster(char_u *name);
static void init_syn_patterns(void);
static char_u *get_syn_pattern(char_u *arg, synpat_T *ci);
static int get_id_list(char_u **arg, int keylen, short **list, int skip);
static void syn_combine_list(short **clstr1, short **clstr2, int list_op);

#if defined(FEAT_RELTIME) || defined(PROTO)




void
syn_set_timeout(proftime_T *tm)
{
syn_tm = tm;
}
#endif








void
syntax_start(win_T *wp, linenr_T lnum)
{
synstate_T *p;
synstate_T *last_valid = NULL;
synstate_T *last_min_valid = NULL;
synstate_T *sp, *prev = NULL;
linenr_T parsed_lnum;
linenr_T first_stored;
int dist;
static varnumber_T changedtick = 0; 

#if defined(FEAT_CONCEAL)
current_sub_char = NUL;
#endif






if (syn_block != wp->w_s
|| syn_buf != wp->w_buffer
|| changedtick != CHANGEDTICK(syn_buf))
{
invalidate_current_state();
syn_buf = wp->w_buffer;
syn_block = wp->w_s;
}
changedtick = CHANGEDTICK(syn_buf);
syn_win = wp;




syn_stack_alloc();
if (syn_block->b_sst_array == NULL)
return; 
syn_block->b_sst_lasttick = display_tick;




if (VALID_STATE(&current_state)
&& current_lnum < lnum
&& current_lnum < syn_buf->b_ml.ml_line_count)
{
(void)syn_finish_line(FALSE);
if (!current_state_stored)
{
++current_lnum;
(void)store_current_state();
}






if (current_lnum != lnum)
invalidate_current_state();
}
else
invalidate_current_state();





if (INVALID_STATE(&current_state) && syn_block->b_sst_array != NULL)
{

for (p = syn_block->b_sst_first; p != NULL; p = p->sst_next)
{
if (p->sst_lnum > lnum)
break;
if (p->sst_lnum <= lnum && p->sst_change_lnum == 0)
{
last_valid = p;
if (p->sst_lnum >= lnum - syn_block->b_syn_sync_minlines)
last_min_valid = p;
}
}
if (last_min_valid != NULL)
load_current_state(last_min_valid);
}





if (INVALID_STATE(&current_state))
{
syn_sync(wp, lnum, last_valid);
if (current_lnum == 1)

first_stored = 1;
else


first_stored = current_lnum + syn_block->b_syn_sync_minlines;
}
else
first_stored = current_lnum;





if (syn_block->b_sst_len <= Rows)
dist = 999999;
else
dist = syn_buf->b_ml.ml_line_count / (syn_block->b_sst_len - Rows) + 1;
while (current_lnum < lnum)
{
syn_start_line();
(void)syn_finish_line(FALSE);
++current_lnum;



if (current_lnum >= first_stored)
{



if (prev == NULL)
prev = syn_stack_find_entry(current_lnum - 1);
if (prev == NULL)
sp = syn_block->b_sst_first;
else
sp = prev;
while (sp != NULL && sp->sst_lnum < current_lnum)
sp = sp->sst_next;
if (sp != NULL
&& sp->sst_lnum == current_lnum
&& syn_stack_equal(sp))
{
parsed_lnum = current_lnum;
prev = sp;
while (sp != NULL && sp->sst_change_lnum <= parsed_lnum)
{
if (sp->sst_lnum <= lnum)

prev = sp;
else if (sp->sst_change_lnum == 0)

break;
sp->sst_change_lnum = 0;
sp = sp->sst_next;
}
load_current_state(prev);
}



else if (prev == NULL
|| current_lnum == lnum
|| current_lnum >= prev->sst_lnum + dist)
prev = store_current_state();
}



line_breakcheck();
if (got_int)
{
current_lnum = lnum;
break;
}
}

syn_start_line();
}





static void
clear_syn_state(synstate_T *p)
{
int i;
garray_T *gap;

if (p->sst_stacksize > SST_FIX_STATES)
{
gap = &(p->sst_union.sst_ga);
for (i = 0; i < gap->ga_len; i++)
unref_extmatch(SYN_STATE_P(gap)[i].bs_extmatch);
ga_clear(gap);
}
else
{
for (i = 0; i < p->sst_stacksize; i++)
unref_extmatch(p->sst_union.sst_stack[i].bs_extmatch);
}
}




static void
clear_current_state(void)
{
int i;
stateitem_T *sip;

sip = (stateitem_T *)(current_state.ga_data);
for (i = 0; i < current_state.ga_len; i++)
unref_extmatch(sip[i].si_extmatch);
ga_clear(&current_state);
}










static void
syn_sync(
win_T *wp,
linenr_T start_lnum,
synstate_T *last_valid)
{
buf_T *curbuf_save;
win_T *curwin_save;
pos_T cursor_save;
int idx;
linenr_T lnum;
linenr_T end_lnum;
linenr_T break_lnum;
int had_sync_point;
stateitem_T *cur_si;
synpat_T *spp;
char_u *line;
int found_flags = 0;
int found_match_idx = 0;
linenr_T found_current_lnum = 0;
int found_current_col= 0;
lpos_T found_m_endpos;
colnr_T prev_current_col;




invalidate_current_state();









if (syn_block->b_syn_sync_minlines > start_lnum)
start_lnum = 1;
else
{
if (syn_block->b_syn_sync_minlines == 1)
lnum = 1;
else if (syn_block->b_syn_sync_minlines < 10)
lnum = syn_block->b_syn_sync_minlines * 2;
else
lnum = syn_block->b_syn_sync_minlines * 3 / 2;
if (syn_block->b_syn_sync_maxlines != 0
&& lnum > syn_block->b_syn_sync_maxlines)
lnum = syn_block->b_syn_sync_maxlines;
if (lnum >= start_lnum)
start_lnum = 1;
else
start_lnum -= lnum;
}
current_lnum = start_lnum;




if (syn_block->b_syn_sync_flags & SF_CCOMMENT)
{


curwin_save = curwin;
curwin = wp;
curbuf_save = curbuf;
curbuf = syn_buf;




for ( ; start_lnum > 1; --start_lnum)
{
line = ml_get(start_lnum - 1);
if (*line == NUL || *(line + STRLEN(line) - 1) != '\\')
break;
}
current_lnum = start_lnum;


cursor_save = wp->w_cursor;
wp->w_cursor.lnum = start_lnum;
wp->w_cursor.col = 0;






if (find_start_comment((int)syn_block->b_syn_sync_maxlines) != NULL)
{
for (idx = syn_block->b_syn_patterns.ga_len; --idx >= 0; )
if (SYN_ITEMS(syn_block)[idx].sp_syn.id
== syn_block->b_syn_sync_id
&& SYN_ITEMS(syn_block)[idx].sp_type == SPTYPE_START)
{
validate_current_state();
if (push_current_state(idx) == OK)
update_si_attr(current_state.ga_len - 1);
break;
}
}


wp->w_cursor = cursor_save;
curwin = curwin_save;
curbuf = curbuf_save;
}




else if (syn_block->b_syn_sync_flags & SF_MATCH)
{
if (syn_block->b_syn_sync_maxlines != 0
&& start_lnum > syn_block->b_syn_sync_maxlines)
break_lnum = start_lnum - syn_block->b_syn_sync_maxlines;
else
break_lnum = 0;

found_m_endpos.lnum = 0;
found_m_endpos.col = 0;
end_lnum = start_lnum;
lnum = start_lnum;
while (--lnum > break_lnum)
{

line_breakcheck();
if (got_int)
{
invalidate_current_state();
current_lnum = start_lnum;
break;
}


if (last_valid != NULL && lnum == last_valid->sst_lnum)
{
load_current_state(last_valid);
break;
}




if (lnum > 1 && syn_match_linecont(lnum - 1))
continue;




validate_current_state();

for (current_lnum = lnum; current_lnum < end_lnum; ++current_lnum)
{
syn_start_line();
for (;;)
{
had_sync_point = syn_finish_line(TRUE);




if (had_sync_point && current_state.ga_len)
{
cur_si = &CUR_STATE(current_state.ga_len - 1);
if (cur_si->si_m_endpos.lnum > start_lnum)
{

current_lnum = end_lnum;
break;
}
if (cur_si->si_idx < 0)
{

found_flags = 0;
found_match_idx = KEYWORD_IDX;
}
else
{
spp = &(SYN_ITEMS(syn_block)[cur_si->si_idx]);
found_flags = spp->sp_flags;
found_match_idx = spp->sp_sync_idx;
}
found_current_lnum = current_lnum;
found_current_col = current_col;
found_m_endpos = cur_si->si_m_endpos;




if (found_m_endpos.lnum > current_lnum)
{
current_lnum = found_m_endpos.lnum;
current_col = found_m_endpos.col;
if (current_lnum >= end_lnum)
break;
}
else if (found_m_endpos.col > current_col)
current_col = found_m_endpos.col;
else
++current_col;




prev_current_col = current_col;
if (syn_getcurline()[current_col] != NUL)
++current_col;
check_state_ends();
current_col = prev_current_col;
}
else
break;
}
}




if (found_flags)
{





clear_current_state();
if (found_match_idx >= 0
&& push_current_state(found_match_idx) == OK)
update_si_attr(current_state.ga_len - 1);







if (found_flags & HL_SYNC_HERE)
{
if (current_state.ga_len)
{
cur_si = &CUR_STATE(current_state.ga_len - 1);
cur_si->si_h_startpos.lnum = found_current_lnum;
cur_si->si_h_startpos.col = found_current_col;
update_si_end(cur_si, (int)current_col, TRUE);
check_keepend();
}
current_col = found_m_endpos.col;
current_lnum = found_m_endpos.lnum;
(void)syn_finish_line(FALSE);
++current_lnum;
}
else
current_lnum = start_lnum;

break;
}

end_lnum = lnum;
invalidate_current_state();
}


if (lnum <= break_lnum)
{
invalidate_current_state();
current_lnum = break_lnum + 1;
}
}

validate_current_state();
}

static void
save_chartab(char_u *chartab)
{
if (syn_block->b_syn_isk != empty_option)
{
mch_memmove(chartab, syn_buf->b_chartab, (size_t)32);
mch_memmove(syn_buf->b_chartab, syn_win->w_s->b_syn_chartab,
(size_t)32);
}
}

static void
restore_chartab(char_u *chartab)
{
if (syn_win->w_s->b_syn_isk != empty_option)
mch_memmove(syn_buf->b_chartab, chartab, (size_t)32);
}




static int
syn_match_linecont(linenr_T lnum)
{
regmmatch_T regmatch;
int r;
char_u buf_chartab[32]; 

if (syn_block->b_syn_linecont_prog != NULL)
{

save_chartab(buf_chartab);
regmatch.rmm_ic = syn_block->b_syn_linecont_ic;
regmatch.regprog = syn_block->b_syn_linecont_prog;
r = syn_regexec(&regmatch, lnum, (colnr_T)0,
IF_SYN_TIME(&syn_block->b_syn_linecont_time));
syn_block->b_syn_linecont_prog = regmatch.regprog;
restore_chartab(buf_chartab);
return r;
}
return FALSE;
}




static void
syn_start_line(void)
{
current_finished = FALSE;
current_col = 0;





if (current_state.ga_len > 0)
{
syn_update_ends(TRUE);
check_state_ends();
}

next_match_idx = -1;
++current_line_id;
#if defined(FEAT_CONCEAL)
next_seqnr = 1;
#endif
}






static void
syn_update_ends(int startofline)
{
stateitem_T *cur_si;
int i;
int seen_keepend;

if (startofline)
{


for (i = 0; i < current_state.ga_len; ++i)
{
cur_si = &CUR_STATE(i);
if (cur_si->si_idx >= 0
&& (SYN_ITEMS(syn_block)[cur_si->si_idx]).sp_type
== SPTYPE_MATCH
&& cur_si->si_m_endpos.lnum < current_lnum)
{
cur_si->si_flags |= HL_MATCHCONT;
cur_si->si_m_endpos.lnum = 0;
cur_si->si_m_endpos.col = 0;
cur_si->si_h_endpos = cur_si->si_m_endpos;
cur_si->si_ends = TRUE;
}
}
}










i = current_state.ga_len - 1;
if (keepend_level >= 0)
for ( ; i > keepend_level; --i)
if (CUR_STATE(i).si_flags & HL_EXTEND)
break;

seen_keepend = FALSE;
for ( ; i < current_state.ga_len; ++i)
{
cur_si = &CUR_STATE(i);
if ((cur_si->si_flags & HL_KEEPEND)
|| (seen_keepend && !startofline)
|| (i == current_state.ga_len - 1 && startofline))
{
cur_si->si_h_startpos.col = 0; 
cur_si->si_h_startpos.lnum = current_lnum;

if (!(cur_si->si_flags & HL_MATCHCONT))
update_si_end(cur_si, (int)current_col, !startofline);

if (!startofline && (cur_si->si_flags & HL_KEEPEND))
seen_keepend = TRUE;
}
}
check_keepend();
}




































static void
syn_stack_free_block(synblock_T *block)
{
synstate_T *p;

if (block->b_sst_array != NULL)
{
for (p = block->b_sst_first; p != NULL; p = p->sst_next)
clear_syn_state(p);
VIM_CLEAR(block->b_sst_array);
block->b_sst_first = NULL;
block->b_sst_len = 0;
}
}




void
syn_stack_free_all(synblock_T *block)
{
#if defined(FEAT_FOLDING)
win_T *wp;
#endif

syn_stack_free_block(block);

#if defined(FEAT_FOLDING)

FOR_ALL_WINDOWS(wp)
{
if (wp->w_s == block && foldmethodIsSyntax(wp))
foldUpdateAll(wp);
}
#endif
}







static void
syn_stack_alloc(void)
{
long len;
synstate_T *to, *from;
synstate_T *sstp;

len = syn_buf->b_ml.ml_line_count / SST_DIST + Rows * 2;
if (len < SST_MIN_ENTRIES)
len = SST_MIN_ENTRIES;
else if (len > SST_MAX_ENTRIES)
len = SST_MAX_ENTRIES;
if (syn_block->b_sst_len > len * 2 || syn_block->b_sst_len < len)
{

len = syn_buf->b_ml.ml_line_count;
len = (len + len / 2) / SST_DIST + Rows * 2;
if (len < SST_MIN_ENTRIES)
len = SST_MIN_ENTRIES;
else if (len > SST_MAX_ENTRIES)
len = SST_MAX_ENTRIES;

if (syn_block->b_sst_array != NULL)
{


while (syn_block->b_sst_len - syn_block->b_sst_freecount + 2 > len
&& syn_stack_cleanup())
;
if (len < syn_block->b_sst_len - syn_block->b_sst_freecount + 2)
len = syn_block->b_sst_len - syn_block->b_sst_freecount + 2;
}

sstp = ALLOC_CLEAR_MULT(synstate_T, len);
if (sstp == NULL) 
return;

to = sstp - 1;
if (syn_block->b_sst_array != NULL)
{

for (from = syn_block->b_sst_first; from != NULL;
from = from->sst_next)
{
++to;
*to = *from;
to->sst_next = to + 1;
}
}
if (to != sstp - 1)
{
to->sst_next = NULL;
syn_block->b_sst_first = sstp;
syn_block->b_sst_freecount = len - (int)(to - sstp) - 1;
}
else
{
syn_block->b_sst_first = NULL;
syn_block->b_sst_freecount = len;
}


syn_block->b_sst_firstfree = to + 1;
while (++to < sstp + len)
to->sst_next = to + 1;
(sstp + len - 1)->sst_next = NULL;

vim_free(syn_block->b_sst_array);
syn_block->b_sst_array = sstp;
syn_block->b_sst_len = len;
}
}







void
syn_stack_apply_changes(buf_T *buf)
{
win_T *wp;

syn_stack_apply_changes_block(&buf->b_s, buf);

FOR_ALL_WINDOWS(wp)
{
if ((wp->w_buffer == buf) && (wp->w_s != &buf->b_s))
syn_stack_apply_changes_block(wp->w_s, buf);
}
}

static void
syn_stack_apply_changes_block(synblock_T *block, buf_T *buf)
{
synstate_T *p, *prev, *np;
linenr_T n;

prev = NULL;
for (p = block->b_sst_first; p != NULL; )
{
if (p->sst_lnum + block->b_syn_sync_linebreaks > buf->b_mod_top)
{
n = p->sst_lnum + buf->b_mod_xlines;
if (n <= buf->b_mod_bot)
{

np = p->sst_next;
if (prev == NULL)
block->b_sst_first = np;
else
prev->sst_next = np;
syn_stack_free_entry(block, p);
p = np;
continue;
}



if (p->sst_change_lnum != 0 && p->sst_change_lnum > buf->b_mod_top)
{
if (p->sst_change_lnum + buf->b_mod_xlines > buf->b_mod_top)
p->sst_change_lnum += buf->b_mod_xlines;
else
p->sst_change_lnum = buf->b_mod_top;
}
if (p->sst_change_lnum == 0
|| p->sst_change_lnum < buf->b_mod_bot)
p->sst_change_lnum = buf->b_mod_bot;

p->sst_lnum = n;
}
prev = p;
p = p->sst_next;
}
}





static int
syn_stack_cleanup(void)
{
synstate_T *p, *prev;
disptick_T tick;
int above;
int dist;
int retval = FALSE;

if (syn_block->b_sst_first == NULL)
return retval;


if (syn_block->b_sst_len <= Rows)
dist = 999999;
else
dist = syn_buf->b_ml.ml_line_count / (syn_block->b_sst_len - Rows) + 1;






tick = syn_block->b_sst_lasttick;
above = FALSE;
prev = syn_block->b_sst_first;
for (p = prev->sst_next; p != NULL; prev = p, p = p->sst_next)
{
if (prev->sst_lnum + dist > p->sst_lnum)
{
if (p->sst_tick > syn_block->b_sst_lasttick)
{
if (!above || p->sst_tick < tick)
tick = p->sst_tick;
above = TRUE;
}
else if (!above && p->sst_tick < tick)
tick = p->sst_tick;
}
}





prev = syn_block->b_sst_first;
for (p = prev->sst_next; p != NULL; prev = p, p = p->sst_next)
{
if (p->sst_tick == tick && prev->sst_lnum + dist > p->sst_lnum)
{

prev->sst_next = p->sst_next;
syn_stack_free_entry(syn_block, p);
p = prev;
retval = TRUE;
}
}
return retval;
}





static void
syn_stack_free_entry(synblock_T *block, synstate_T *p)
{
clear_syn_state(p);
p->sst_next = block->b_sst_firstfree;
block->b_sst_firstfree = p;
++block->b_sst_freecount;
}





static synstate_T *
syn_stack_find_entry(linenr_T lnum)
{
synstate_T *p, *prev;

prev = NULL;
for (p = syn_block->b_sst_first; p != NULL; prev = p, p = p->sst_next)
{
if (p->sst_lnum == lnum)
return p;
if (p->sst_lnum > lnum)
break;
}
return prev;
}





static synstate_T *
store_current_state(void)
{
int i;
synstate_T *p;
bufstate_T *bp;
stateitem_T *cur_si;
synstate_T *sp = syn_stack_find_entry(current_lnum);





for (i = current_state.ga_len - 1; i >= 0; --i)
{
cur_si = &CUR_STATE(i);
if (cur_si->si_h_startpos.lnum >= current_lnum
|| cur_si->si_m_endpos.lnum >= current_lnum
|| cur_si->si_h_endpos.lnum >= current_lnum
|| (cur_si->si_end_idx
&& cur_si->si_eoe_pos.lnum >= current_lnum))
break;
}
if (i >= 0)
{
if (sp != NULL)
{

if (syn_block->b_sst_first == sp)

syn_block->b_sst_first = sp->sst_next;
else
{

for (p = syn_block->b_sst_first; p != NULL; p = p->sst_next)
if (p->sst_next == sp)
break;
if (p != NULL) 
p->sst_next = sp->sst_next;
}
syn_stack_free_entry(syn_block, sp);
sp = NULL;
}
}
else if (sp == NULL || sp->sst_lnum != current_lnum)
{




if (syn_block->b_sst_freecount == 0)
{
(void)syn_stack_cleanup();

sp = syn_stack_find_entry(current_lnum);
}

if (syn_block->b_sst_freecount == 0)
sp = NULL;
else
{


p = syn_block->b_sst_firstfree;
syn_block->b_sst_firstfree = p->sst_next;
--syn_block->b_sst_freecount;
if (sp == NULL)
{

p->sst_next = syn_block->b_sst_first;
syn_block->b_sst_first = p;
}
else
{

p->sst_next = sp->sst_next;
sp->sst_next = p;
}
sp = p;
sp->sst_stacksize = 0;
sp->sst_lnum = current_lnum;
}
}
if (sp != NULL)
{

clear_syn_state(sp);
sp->sst_stacksize = current_state.ga_len;
if (current_state.ga_len > SST_FIX_STATES)
{


ga_init2(&sp->sst_union.sst_ga, (int)sizeof(bufstate_T), 1);
if (ga_grow(&sp->sst_union.sst_ga, current_state.ga_len) == FAIL)
sp->sst_stacksize = 0;
else
sp->sst_union.sst_ga.ga_len = current_state.ga_len;
bp = SYN_STATE_P(&(sp->sst_union.sst_ga));
}
else
bp = sp->sst_union.sst_stack;
for (i = 0; i < sp->sst_stacksize; ++i)
{
bp[i].bs_idx = CUR_STATE(i).si_idx;
bp[i].bs_flags = CUR_STATE(i).si_flags;
#if defined(FEAT_CONCEAL)
bp[i].bs_seqnr = CUR_STATE(i).si_seqnr;
bp[i].bs_cchar = CUR_STATE(i).si_cchar;
#endif
bp[i].bs_extmatch = ref_extmatch(CUR_STATE(i).si_extmatch);
}
sp->sst_next_flags = current_next_flags;
sp->sst_next_list = current_next_list;
sp->sst_tick = display_tick;
sp->sst_change_lnum = 0;
}
current_state_stored = TRUE;
return sp;
}




static void
load_current_state(synstate_T *from)
{
int i;
bufstate_T *bp;

clear_current_state();
validate_current_state();
keepend_level = -1;
if (from->sst_stacksize
&& ga_grow(&current_state, from->sst_stacksize) != FAIL)
{
if (from->sst_stacksize > SST_FIX_STATES)
bp = SYN_STATE_P(&(from->sst_union.sst_ga));
else
bp = from->sst_union.sst_stack;
for (i = 0; i < from->sst_stacksize; ++i)
{
CUR_STATE(i).si_idx = bp[i].bs_idx;
CUR_STATE(i).si_flags = bp[i].bs_flags;
#if defined(FEAT_CONCEAL)
CUR_STATE(i).si_seqnr = bp[i].bs_seqnr;
CUR_STATE(i).si_cchar = bp[i].bs_cchar;
#endif
CUR_STATE(i).si_extmatch = ref_extmatch(bp[i].bs_extmatch);
if (keepend_level < 0 && (CUR_STATE(i).si_flags & HL_KEEPEND))
keepend_level = i;
CUR_STATE(i).si_ends = FALSE;
CUR_STATE(i).si_m_lnum = 0;
if (CUR_STATE(i).si_idx >= 0)
CUR_STATE(i).si_next_list =
(SYN_ITEMS(syn_block)[CUR_STATE(i).si_idx]).sp_next_list;
else
CUR_STATE(i).si_next_list = NULL;
update_si_attr(i);
}
current_state.ga_len = from->sst_stacksize;
}
current_next_list = from->sst_next_list;
current_next_flags = from->sst_next_flags;
current_lnum = from->sst_lnum;
}





static int
syn_stack_equal(synstate_T *sp)
{
int i, j;
bufstate_T *bp;
reg_extmatch_T *six, *bsx;


if (sp->sst_stacksize == current_state.ga_len
&& sp->sst_next_list == current_next_list)
{

if (sp->sst_stacksize > SST_FIX_STATES)
bp = SYN_STATE_P(&(sp->sst_union.sst_ga));
else
bp = sp->sst_union.sst_stack;

for (i = current_state.ga_len; --i >= 0; )
{

if (bp[i].bs_idx != CUR_STATE(i).si_idx)
break;
if (bp[i].bs_extmatch != CUR_STATE(i).si_extmatch)
{



bsx = bp[i].bs_extmatch;
six = CUR_STATE(i).si_extmatch;


if (bsx == NULL || six == NULL)
break;
for (j = 0; j < NSUBEXP; ++j)
{


if (bsx->matches[j] != six->matches[j])
{



if (bsx->matches[j] == NULL
|| six->matches[j] == NULL)
break;
if ((SYN_ITEMS(syn_block)[CUR_STATE(i).si_idx]).sp_ic
? MB_STRICMP(bsx->matches[j],
six->matches[j]) != 0
: STRCMP(bsx->matches[j], six->matches[j]) != 0)
break;
}
}
if (j != NSUBEXP)
break;
}
}
if (i < 0)
return TRUE;
}
return FALSE;
}











void
syntax_end_parsing(linenr_T lnum)
{
synstate_T *sp;

sp = syn_stack_find_entry(lnum);
if (sp != NULL && sp->sst_lnum < lnum)
sp = sp->sst_next;

if (sp != NULL && sp->sst_change_lnum != 0)
sp->sst_change_lnum = lnum;
}





static void
invalidate_current_state(void)
{
clear_current_state();
current_state.ga_itemsize = 0; 
current_next_list = NULL;
keepend_level = -1;
}

static void
validate_current_state(void)
{
current_state.ga_itemsize = sizeof(stateitem_T);
current_state.ga_growsize = 3;
}






int
syntax_check_changed(linenr_T lnum)
{
int retval = TRUE;
synstate_T *sp;








if (VALID_STATE(&current_state) && lnum == current_lnum + 1)
{
sp = syn_stack_find_entry(lnum);
if (sp != NULL && sp->sst_lnum == lnum)
{




(void)syn_finish_line(FALSE);





if (syn_stack_equal(sp))
retval = FALSE;




++current_lnum;
(void)store_current_state();
}
}

return retval;
}







static int
syn_finish_line(
int syncing) 
{
stateitem_T *cur_si;
colnr_T prev_current_col;

while (!current_finished)
{
(void)syn_current_attr(syncing, FALSE, NULL, FALSE);



if (syncing && current_state.ga_len)
{



cur_si = &CUR_STATE(current_state.ga_len - 1);
if (cur_si->si_idx >= 0
&& (SYN_ITEMS(syn_block)[cur_si->si_idx].sp_flags
& (HL_SYNC_HERE|HL_SYNC_THERE)))
return TRUE;




prev_current_col = current_col;
if (syn_getcurline()[current_col] != NUL)
++current_col;
check_state_ends();
current_col = prev_current_col;
}
++current_col;
}
return FALSE;
}










int
get_syntax_attr(
colnr_T col,
int *can_spell,
int keep_state) 
{
int attr = 0;

if (can_spell != NULL)


*can_spell = syn_block->b_syn_spell == SYNSPL_DEFAULT
? (syn_block->b_spell_cluster_id == 0)
: (syn_block->b_syn_spell == SYNSPL_TOP);


if (syn_block->b_sst_array == NULL)
return 0;


if (syn_buf->b_p_smc > 0 && col >= (colnr_T)syn_buf->b_p_smc)
{
clear_current_state();
#if defined(FEAT_EVAL)
current_id = 0;
current_trans_id = 0;
#endif
#if defined(FEAT_CONCEAL)
current_flags = 0;
current_seqnr = 0;
#endif
return 0;
}


if (INVALID_STATE(&current_state))
validate_current_state();




while (current_col <= col)
{
attr = syn_current_attr(FALSE, TRUE, can_spell,
current_col == col ? keep_state : FALSE);
++current_col;
}

return attr;
}




static int
syn_current_attr(
int syncing, 
int displaying, 
int *can_spell, 
int keep_state) 
{
int syn_id;
lpos_T endpos; 
lpos_T hl_startpos; 
lpos_T hl_endpos;
lpos_T eos_pos; 
lpos_T eoe_pos; 
int end_idx; 
int idx;
synpat_T *spp;
stateitem_T *cur_si, *sip = NULL;
int startcol;
int endcol;
long flags;
int cchar;
short *next_list;
int found_match; 
static int try_next_column = FALSE; 
int do_keywords;
regmmatch_T regmatch;
lpos_T pos;
int lc_col;
reg_extmatch_T *cur_extmatch = NULL;
char_u buf_chartab[32]; 
char_u *line; 



int keep_next_list;
int zero_width_next_list = FALSE;
garray_T zero_width_next_ga;





line = syn_getcurline();
if (line[current_col] == NUL && current_col != 0)
{



if (next_match_idx >= 0 && next_match_col >= (int)current_col
&& next_match_col != MAXCOL)
(void)push_next_match(NULL);

current_finished = TRUE;
current_state_stored = FALSE;
return 0;
}


if (line[current_col] == NUL || line[current_col + 1] == NUL)
{
current_finished = TRUE;
current_state_stored = FALSE;
}






if (try_next_column)
{
next_match_idx = -1;
try_next_column = FALSE;
}


do_keywords = !syncing
&& (syn_block->b_keywtab.ht_used > 0
|| syn_block->b_keywtab_ic.ht_used > 0);



ga_init2(&zero_width_next_ga, (int)sizeof(int), 10);


save_chartab(buf_chartab);






do
{
found_match = FALSE;
keep_next_list = FALSE;
syn_id = 0;









if (current_state.ga_len)
cur_si = &CUR_STATE(current_state.ga_len - 1);
else
cur_si = NULL;

if (syn_block->b_syn_containedin || cur_si == NULL
|| cur_si->si_cont_list != NULL)
{




if (do_keywords)
{
line = syn_getcurline();
if (vim_iswordp_buf(line + current_col, syn_buf)
&& (current_col == 0
|| !vim_iswordp_buf(line + current_col - 1
- (has_mbyte
? (*mb_head_off)(line, line + current_col - 1)
: 0) , syn_buf)))
{
syn_id = check_keyword_id(line, (int)current_col,
&endcol, &flags, &next_list, cur_si,
&cchar);
if (syn_id != 0)
{
if (push_current_state(KEYWORD_IDX) == OK)
{
cur_si = &CUR_STATE(current_state.ga_len - 1);
cur_si->si_m_startcol = current_col;
cur_si->si_h_startpos.lnum = current_lnum;
cur_si->si_h_startpos.col = 0; 
cur_si->si_m_endpos.lnum = current_lnum;
cur_si->si_m_endpos.col = endcol;
cur_si->si_h_endpos.lnum = current_lnum;
cur_si->si_h_endpos.col = endcol;
cur_si->si_ends = TRUE;
cur_si->si_end_idx = 0;
cur_si->si_flags = flags;
#if defined(FEAT_CONCEAL)
cur_si->si_seqnr = next_seqnr++;
cur_si->si_cchar = cchar;
if (current_state.ga_len > 1)
cur_si->si_flags |=
CUR_STATE(current_state.ga_len - 2).si_flags
& HL_CONCEAL;
#endif
cur_si->si_id = syn_id;
cur_si->si_trans_id = syn_id;
if (flags & HL_TRANSP)
{
if (current_state.ga_len < 2)
{
cur_si->si_attr = 0;
cur_si->si_trans_id = 0;
}
else
{
cur_si->si_attr = CUR_STATE(
current_state.ga_len - 2).si_attr;
cur_si->si_trans_id = CUR_STATE(
current_state.ga_len - 2).si_trans_id;
}
}
else
cur_si->si_attr = syn_id2attr(syn_id);
cur_si->si_cont_list = NULL;
cur_si->si_next_list = next_list;
check_keepend();
}
else
vim_free(next_list);
}
}
}




if (syn_id == 0 && syn_block->b_syn_patterns.ga_len)
{




if (next_match_idx < 0 || next_match_col < (int)current_col)
{






next_match_idx = 0; 
next_match_col = MAXCOL;
for (idx = syn_block->b_syn_patterns.ga_len; --idx >= 0; )
{
spp = &(SYN_ITEMS(syn_block)[idx]);
if ( spp->sp_syncing == syncing
&& (displaying || !(spp->sp_flags & HL_DISPLAY))
&& (spp->sp_type == SPTYPE_MATCH
|| spp->sp_type == SPTYPE_START)
&& (current_next_list != NULL
? in_id_list(NULL, current_next_list,
&spp->sp_syn, 0)
: (cur_si == NULL
? !(spp->sp_flags & HL_CONTAINED)
: in_id_list(cur_si,
cur_si->si_cont_list, &spp->sp_syn,
spp->sp_flags & HL_CONTAINED))))
{
int r;




if (spp->sp_line_id == current_line_id
&& spp->sp_startcol >= next_match_col)
continue;
spp->sp_line_id = current_line_id;

lc_col = current_col - spp->sp_offsets[SPO_LC_OFF];
if (lc_col < 0)
lc_col = 0;

regmatch.rmm_ic = spp->sp_ic;
regmatch.regprog = spp->sp_prog;
r = syn_regexec(&regmatch,
current_lnum,
(colnr_T)lc_col,
IF_SYN_TIME(&spp->sp_time));
spp->sp_prog = regmatch.regprog;
if (!r)
{

spp->sp_startcol = MAXCOL;
continue;
}




syn_add_start_off(&pos, &regmatch,
spp, SPO_MS_OFF, -1);
if (pos.lnum > current_lnum)
{


spp->sp_startcol = MAXCOL;
continue;
}
startcol = pos.col;



spp->sp_startcol = startcol;





if (startcol >= next_match_col)
continue;






if (did_match_already(idx, &zero_width_next_ga))
{
try_next_column = TRUE;
continue;
}

endpos.lnum = regmatch.endpos[0].lnum;
endpos.col = regmatch.endpos[0].col;


syn_add_start_off(&hl_startpos, &regmatch,
spp, SPO_HS_OFF, -1);



syn_add_end_off(&eos_pos, &regmatch,
spp, SPO_RS_OFF, 0);





unref_extmatch(cur_extmatch);
cur_extmatch = re_extmatch_out;
re_extmatch_out = NULL;

flags = 0;
eoe_pos.lnum = 0; 
eoe_pos.col = 0;
end_idx = 0;
hl_endpos.lnum = 0;







if (spp->sp_type == SPTYPE_START
&& (spp->sp_flags & HL_ONELINE))
{
lpos_T startpos;

startpos = endpos;
find_endpos(idx, &startpos, &endpos, &hl_endpos,
&flags, &eoe_pos, &end_idx, cur_extmatch);
if (endpos.lnum == 0)
continue; 
}






else if (spp->sp_type == SPTYPE_MATCH)
{
syn_add_end_off(&hl_endpos, &regmatch, spp,
SPO_HE_OFF, 0);
syn_add_end_off(&endpos, &regmatch, spp,
SPO_ME_OFF, 0);
if (endpos.lnum == current_lnum
&& (int)endpos.col + syncing < startcol)
{




if (regmatch.startpos[0].col
== regmatch.endpos[0].col)
try_next_column = TRUE;
continue;
}
}






if (hl_startpos.lnum == current_lnum
&& (int)hl_startpos.col < startcol)
hl_startpos.col = startcol;
limit_pos_zero(&hl_endpos, &endpos);

next_match_idx = idx;
next_match_col = startcol;
next_match_m_endpos = endpos;
next_match_h_endpos = hl_endpos;
next_match_h_startpos = hl_startpos;
next_match_flags = flags;
next_match_eos_pos = eos_pos;
next_match_eoe_pos = eoe_pos;
next_match_end_idx = end_idx;
unref_extmatch(next_match_extmatch);
next_match_extmatch = cur_extmatch;
cur_extmatch = NULL;
}
}
}




if (next_match_idx >= 0 && next_match_col == (int)current_col)
{
synpat_T *lspp;



lspp = &(SYN_ITEMS(syn_block)[next_match_idx]);
if (next_match_m_endpos.lnum == current_lnum
&& next_match_m_endpos.col == current_col
&& lspp->sp_next_list != NULL)
{
current_next_list = lspp->sp_next_list;
current_next_flags = lspp->sp_flags;
keep_next_list = TRUE;
zero_width_next_list = TRUE;




if (ga_grow(&zero_width_next_ga, 1) == OK)
{
((int *)(zero_width_next_ga.ga_data))
[zero_width_next_ga.ga_len++] = next_match_idx;
}
next_match_idx = -1;
}
else
cur_si = push_next_match(cur_si);
found_match = TRUE;
}
}
}




if (current_next_list != NULL && !keep_next_list)
{





if (!found_match)
{
line = syn_getcurline();
if (((current_next_flags & HL_SKIPWHITE)
&& VIM_ISWHITE(line[current_col]))
|| ((current_next_flags & HL_SKIPEMPTY)
&& *line == NUL))
break;
}









current_next_list = NULL;
next_match_idx = -1;
if (!zero_width_next_list)
found_match = TRUE;
}

} while (found_match);

restore_chartab(buf_chartab);





current_attr = 0;
#if defined(FEAT_EVAL)
current_id = 0;
current_trans_id = 0;
#endif
#if defined(FEAT_CONCEAL)
current_flags = 0;
current_seqnr = 0;
#endif
if (cur_si != NULL)
{
#if !defined(FEAT_EVAL)
int current_trans_id = 0;
#endif
for (idx = current_state.ga_len - 1; idx >= 0; --idx)
{
sip = &CUR_STATE(idx);
if ((current_lnum > sip->si_h_startpos.lnum
|| (current_lnum == sip->si_h_startpos.lnum
&& current_col >= sip->si_h_startpos.col))
&& (sip->si_h_endpos.lnum == 0
|| current_lnum < sip->si_h_endpos.lnum
|| (current_lnum == sip->si_h_endpos.lnum
&& current_col < sip->si_h_endpos.col)))
{
current_attr = sip->si_attr;
#if defined(FEAT_EVAL)
current_id = sip->si_id;
#endif
current_trans_id = sip->si_trans_id;
#if defined(FEAT_CONCEAL)
current_flags = sip->si_flags;
current_seqnr = sip->si_seqnr;
current_sub_char = sip->si_cchar;
#endif
break;
}
}

if (can_spell != NULL)
{
struct sp_syn sps;





if (syn_block->b_spell_cluster_id == 0)
{


if (syn_block->b_nospell_cluster_id == 0
|| current_trans_id == 0)
*can_spell = (syn_block->b_syn_spell != SYNSPL_NOTOP);
else
{
sps.inc_tag = 0;
sps.id = syn_block->b_nospell_cluster_id;
sps.cont_in_list = NULL;
*can_spell = !in_id_list(sip, sip->si_cont_list, &sps, 0);
}
}
else
{




if (current_trans_id == 0)
*can_spell = (syn_block->b_syn_spell == SYNSPL_TOP);
else
{
sps.inc_tag = 0;
sps.id = syn_block->b_spell_cluster_id;
sps.cont_in_list = NULL;
*can_spell = in_id_list(sip, sip->si_cont_list, &sps, 0);

if (syn_block->b_nospell_cluster_id != 0)
{
sps.id = syn_block->b_nospell_cluster_id;
if (in_id_list(sip, sip->si_cont_list, &sps, 0))
*can_spell = FALSE;
}
}
}
}










if (!syncing && !keep_state)
{
check_state_ends();
if (current_state.ga_len > 0
&& syn_getcurline()[current_col] != NUL)
{
++current_col;
check_state_ends();
--current_col;
}
}
}
else if (can_spell != NULL)


*can_spell = syn_block->b_syn_spell == SYNSPL_DEFAULT
? (syn_block->b_spell_cluster_id == 0)
: (syn_block->b_syn_spell == SYNSPL_TOP);


if (current_next_list != NULL
&& (line = syn_getcurline())[current_col] != NUL
&& line[current_col + 1] == NUL
&& !(current_next_flags & (HL_SKIPNL | HL_SKIPEMPTY)))
current_next_list = NULL;

if (zero_width_next_ga.ga_len > 0)
ga_clear(&zero_width_next_ga);


unref_extmatch(re_extmatch_out);
re_extmatch_out = NULL;
unref_extmatch(cur_extmatch);

return current_attr;
}





static int
did_match_already(int idx, garray_T *gap)
{
int i;

for (i = current_state.ga_len; --i >= 0; )
if (CUR_STATE(i).si_m_startcol == (int)current_col
&& CUR_STATE(i).si_m_lnum == (int)current_lnum
&& CUR_STATE(i).si_idx == idx)
return TRUE;



for (i = gap->ga_len; --i >= 0; )
if (((int *)(gap->ga_data))[i] == idx)
return TRUE;

return FALSE;
}




static stateitem_T *
push_next_match(stateitem_T *cur_si)
{
synpat_T *spp;
#if defined(FEAT_CONCEAL)
int save_flags;
#endif

spp = &(SYN_ITEMS(syn_block)[next_match_idx]);




if (push_current_state(next_match_idx) == OK)
{




cur_si = &CUR_STATE(current_state.ga_len - 1);
cur_si->si_h_startpos = next_match_h_startpos;
cur_si->si_m_startcol = current_col;
cur_si->si_m_lnum = current_lnum;
cur_si->si_flags = spp->sp_flags;
#if defined(FEAT_CONCEAL)
cur_si->si_seqnr = next_seqnr++;
cur_si->si_cchar = spp->sp_cchar;
if (current_state.ga_len > 1)
cur_si->si_flags |=
CUR_STATE(current_state.ga_len - 2).si_flags & HL_CONCEAL;
#endif
cur_si->si_next_list = spp->sp_next_list;
cur_si->si_extmatch = ref_extmatch(next_match_extmatch);
if (spp->sp_type == SPTYPE_START && !(spp->sp_flags & HL_ONELINE))
{

update_si_end(cur_si, (int)(next_match_m_endpos.col), TRUE);
check_keepend();
}
else
{
cur_si->si_m_endpos = next_match_m_endpos;
cur_si->si_h_endpos = next_match_h_endpos;
cur_si->si_ends = TRUE;
cur_si->si_flags |= next_match_flags;
cur_si->si_eoe_pos = next_match_eoe_pos;
cur_si->si_end_idx = next_match_end_idx;
}
if (keepend_level < 0 && (cur_si->si_flags & HL_KEEPEND))
keepend_level = current_state.ga_len - 1;
check_keepend();
update_si_attr(current_state.ga_len - 1);

#if defined(FEAT_CONCEAL)
save_flags = cur_si->si_flags & (HL_CONCEAL | HL_CONCEALENDS);
#endif




if ( spp->sp_type == SPTYPE_START
&& spp->sp_syn_match_id != 0
&& push_current_state(next_match_idx) == OK)
{
cur_si = &CUR_STATE(current_state.ga_len - 1);
cur_si->si_h_startpos = next_match_h_startpos;
cur_si->si_m_startcol = current_col;
cur_si->si_m_lnum = current_lnum;
cur_si->si_m_endpos = next_match_eos_pos;
cur_si->si_h_endpos = next_match_eos_pos;
cur_si->si_ends = TRUE;
cur_si->si_end_idx = 0;
cur_si->si_flags = HL_MATCH;
#if defined(FEAT_CONCEAL)
cur_si->si_seqnr = next_seqnr++;
cur_si->si_flags |= save_flags;
if (cur_si->si_flags & HL_CONCEALENDS)
cur_si->si_flags |= HL_CONCEAL;
#endif
cur_si->si_next_list = NULL;
check_keepend();
update_si_attr(current_state.ga_len - 1);
}
}

next_match_idx = -1; 

return cur_si;
}




static void
check_state_ends(void)
{
stateitem_T *cur_si;
int had_extend;

cur_si = &CUR_STATE(current_state.ga_len - 1);
for (;;)
{
if (cur_si->si_ends
&& (cur_si->si_m_endpos.lnum < current_lnum
|| (cur_si->si_m_endpos.lnum == current_lnum
&& cur_si->si_m_endpos.col <= current_col)))
{






if (cur_si->si_end_idx
&& (cur_si->si_eoe_pos.lnum > current_lnum
|| (cur_si->si_eoe_pos.lnum == current_lnum
&& cur_si->si_eoe_pos.col > current_col)))
{
cur_si->si_idx = cur_si->si_end_idx;
cur_si->si_end_idx = 0;
cur_si->si_m_endpos = cur_si->si_eoe_pos;
cur_si->si_h_endpos = cur_si->si_eoe_pos;
cur_si->si_flags |= HL_MATCH;
#if defined(FEAT_CONCEAL)
cur_si->si_seqnr = next_seqnr++;
if (cur_si->si_flags & HL_CONCEALENDS)
cur_si->si_flags |= HL_CONCEAL;
#endif
update_si_attr(current_state.ga_len - 1);


current_next_list = NULL;


next_match_idx = 0;
next_match_col = MAXCOL;
break;
}
else
{


current_next_list = cur_si->si_next_list;
current_next_flags = cur_si->si_flags;
if (!(current_next_flags & (HL_SKIPNL | HL_SKIPEMPTY))
&& syn_getcurline()[current_col] == NUL)
current_next_list = NULL;



had_extend = (cur_si->si_flags & HL_EXTEND);

pop_current_state();

if (current_state.ga_len == 0)
break;

if (had_extend && keepend_level >= 0)
{
syn_update_ends(FALSE);
if (current_state.ga_len == 0)
break;
}

cur_si = &CUR_STATE(current_state.ga_len - 1);










if (cur_si->si_idx >= 0
&& SYN_ITEMS(syn_block)[cur_si->si_idx].sp_type
== SPTYPE_START
&& !(cur_si->si_flags & (HL_MATCH | HL_KEEPEND)))
{
update_si_end(cur_si, (int)current_col, TRUE);
check_keepend();
if ((current_next_flags & HL_HAS_EOL)
&& keepend_level < 0
&& syn_getcurline()[current_col] == NUL)
break;
}
}
}
else
break;
}
}





static void
update_si_attr(int idx)
{
stateitem_T *sip = &CUR_STATE(idx);
synpat_T *spp;


if (sip->si_idx < 0)
return;

spp = &(SYN_ITEMS(syn_block)[sip->si_idx]);
if (sip->si_flags & HL_MATCH)
sip->si_id = spp->sp_syn_match_id;
else
sip->si_id = spp->sp_syn.id;
sip->si_attr = syn_id2attr(sip->si_id);
sip->si_trans_id = sip->si_id;
if (sip->si_flags & HL_MATCH)
sip->si_cont_list = NULL;
else
sip->si_cont_list = spp->sp_cont_list;






if ((spp->sp_flags & HL_TRANSP) && !(sip->si_flags & HL_MATCH))
{
if (idx == 0)
{
sip->si_attr = 0;
sip->si_trans_id = 0;
if (sip->si_cont_list == NULL)
sip->si_cont_list = ID_LIST_ALL;
}
else
{
sip->si_attr = CUR_STATE(idx - 1).si_attr;
sip->si_trans_id = CUR_STATE(idx - 1).si_trans_id;
sip->si_h_startpos = CUR_STATE(idx - 1).si_h_startpos;
sip->si_h_endpos = CUR_STATE(idx - 1).si_h_endpos;
if (sip->si_cont_list == NULL)
{
sip->si_flags |= HL_TRANS_CONT;
sip->si_cont_list = CUR_STATE(idx - 1).si_cont_list;
}
}
}
}





static void
check_keepend(void)
{
int i;
lpos_T maxpos;
lpos_T maxpos_h;
stateitem_T *sip;





if (keepend_level < 0)
return;






for (i = current_state.ga_len - 1; i > keepend_level; --i)
if (CUR_STATE(i).si_flags & HL_EXTEND)
break;

maxpos.lnum = 0;
maxpos.col = 0;
maxpos_h.lnum = 0;
maxpos_h.col = 0;
for ( ; i < current_state.ga_len; ++i)
{
sip = &CUR_STATE(i);
if (maxpos.lnum != 0)
{
limit_pos_zero(&sip->si_m_endpos, &maxpos);
limit_pos_zero(&sip->si_h_endpos, &maxpos_h);
limit_pos_zero(&sip->si_eoe_pos, &maxpos);
sip->si_ends = TRUE;
}
if (sip->si_ends && (sip->si_flags & HL_KEEPEND))
{
if (maxpos.lnum == 0
|| maxpos.lnum > sip->si_m_endpos.lnum
|| (maxpos.lnum == sip->si_m_endpos.lnum
&& maxpos.col > sip->si_m_endpos.col))
maxpos = sip->si_m_endpos;
if (maxpos_h.lnum == 0
|| maxpos_h.lnum > sip->si_h_endpos.lnum
|| (maxpos_h.lnum == sip->si_h_endpos.lnum
&& maxpos_h.col > sip->si_h_endpos.col))
maxpos_h = sip->si_h_endpos;
}
}
}







static void
update_si_end(
stateitem_T *sip,
int startcol, 
int force) 
{
lpos_T startpos;
lpos_T endpos;
lpos_T hl_endpos;
lpos_T end_endpos;
int end_idx;


if (sip->si_idx < 0)
return;




if (!force && sip->si_m_endpos.lnum >= current_lnum)
return;





end_idx = 0;
startpos.lnum = current_lnum;
startpos.col = startcol;
find_endpos(sip->si_idx, &startpos, &endpos, &hl_endpos,
&(sip->si_flags), &end_endpos, &end_idx, sip->si_extmatch);

if (endpos.lnum == 0)
{

if (SYN_ITEMS(syn_block)[sip->si_idx].sp_flags & HL_ONELINE)
{

sip->si_ends = TRUE;
sip->si_m_endpos.lnum = current_lnum;
sip->si_m_endpos.col = (colnr_T)STRLEN(syn_getcurline());
}
else
{

sip->si_ends = FALSE;
sip->si_m_endpos.lnum = 0;
}
sip->si_h_endpos = sip->si_m_endpos;
}
else
{

sip->si_m_endpos = endpos;
sip->si_h_endpos = hl_endpos;
sip->si_eoe_pos = end_endpos;
sip->si_ends = TRUE;
sip->si_end_idx = end_idx;
}
}






static int
push_current_state(int idx)
{
if (ga_grow(&current_state, 1) == FAIL)
return FAIL;
vim_memset(&CUR_STATE(current_state.ga_len), 0, sizeof(stateitem_T));
CUR_STATE(current_state.ga_len).si_idx = idx;
++current_state.ga_len;
return OK;
}




static void
pop_current_state(void)
{
if (current_state.ga_len)
{
unref_extmatch(CUR_STATE(current_state.ga_len - 1).si_extmatch);
--current_state.ga_len;
}

next_match_idx = -1;


if (keepend_level >= current_state.ga_len)
keepend_level = -1;
}










static void
find_endpos(
int idx, 
lpos_T *startpos, 
lpos_T *m_endpos, 
lpos_T *hl_endpos, 
long *flagsp, 
lpos_T *end_endpos, 
int *end_idx, 
reg_extmatch_T *start_ext) 
{
colnr_T matchcol;
synpat_T *spp, *spp_skip;
int start_idx;
int best_idx;
regmmatch_T regmatch;
regmmatch_T best_regmatch; 
lpos_T pos;
char_u *line;
int had_match = FALSE;
char_u buf_chartab[32]; 


if (idx < 0)
return;






spp = &(SYN_ITEMS(syn_block)[idx]);
if (spp->sp_type != SPTYPE_START)
{
*hl_endpos = *startpos;
return;
}




for (;;)
{
spp = &(SYN_ITEMS(syn_block)[idx]);
if (spp->sp_type != SPTYPE_START)
break;
++idx;
}




if (spp->sp_type == SPTYPE_SKIP)
{
spp_skip = spp;
++idx;
}
else
spp_skip = NULL;


unref_extmatch(re_extmatch_in);
re_extmatch_in = ref_extmatch(start_ext);

matchcol = startpos->col; 
start_idx = idx; 
best_regmatch.startpos[0].col = 0; 


save_chartab(buf_chartab);

for (;;)
{



best_idx = -1;
for (idx = start_idx; idx < syn_block->b_syn_patterns.ga_len; ++idx)
{
int lc_col = matchcol;
int r;

spp = &(SYN_ITEMS(syn_block)[idx]);
if (spp->sp_type != SPTYPE_END) 
break;
lc_col -= spp->sp_offsets[SPO_LC_OFF];
if (lc_col < 0)
lc_col = 0;

regmatch.rmm_ic = spp->sp_ic;
regmatch.regprog = spp->sp_prog;
r = syn_regexec(&regmatch, startpos->lnum, lc_col,
IF_SYN_TIME(&spp->sp_time));
spp->sp_prog = regmatch.regprog;
if (r)
{
if (best_idx == -1 || regmatch.startpos[0].col
< best_regmatch.startpos[0].col)
{
best_idx = idx;
best_regmatch.startpos[0] = regmatch.startpos[0];
best_regmatch.endpos[0] = regmatch.endpos[0];
}
}
}





if (best_idx == -1)
break;





if (spp_skip != NULL)
{
int lc_col = matchcol - spp_skip->sp_offsets[SPO_LC_OFF];
int r;

if (lc_col < 0)
lc_col = 0;
regmatch.rmm_ic = spp_skip->sp_ic;
regmatch.regprog = spp_skip->sp_prog;
r = syn_regexec(&regmatch, startpos->lnum, lc_col,
IF_SYN_TIME(&spp_skip->sp_time));
spp_skip->sp_prog = regmatch.regprog;
if (r && regmatch.startpos[0].col
<= best_regmatch.startpos[0].col)
{
int line_len;


syn_add_end_off(&pos, &regmatch, spp_skip, SPO_ME_OFF, 1);



if (pos.lnum > startpos->lnum)
break;

line = ml_get_buf(syn_buf, startpos->lnum, FALSE);
line_len = (int)STRLEN(line);


if (pos.col <= matchcol)
++matchcol;
else if (pos.col <= regmatch.endpos[0].col)
matchcol = pos.col;
else

for (matchcol = regmatch.endpos[0].col;
matchcol < line_len && matchcol < pos.col;
++matchcol)
;


if (matchcol >= line_len)
break;

continue; 
}
}





spp = &(SYN_ITEMS(syn_block)[best_idx]);
syn_add_end_off(m_endpos, &best_regmatch, spp, SPO_ME_OFF, 1);

if (m_endpos->lnum == startpos->lnum && m_endpos->col < startpos->col)
m_endpos->col = startpos->col;

syn_add_end_off(end_endpos, &best_regmatch, spp, SPO_HE_OFF, 1);

if (end_endpos->lnum == startpos->lnum
&& end_endpos->col < startpos->col)
end_endpos->col = startpos->col;

limit_pos(end_endpos, m_endpos);




if (spp->sp_syn_match_id != spp->sp_syn.id && spp->sp_syn_match_id != 0)
{
*end_idx = best_idx;
if (spp->sp_off_flags & (1 << (SPO_RE_OFF + SPO_COUNT)))
{
hl_endpos->lnum = best_regmatch.endpos[0].lnum;
hl_endpos->col = best_regmatch.endpos[0].col;
}
else
{
hl_endpos->lnum = best_regmatch.startpos[0].lnum;
hl_endpos->col = best_regmatch.startpos[0].col;
}
hl_endpos->col += spp->sp_offsets[SPO_RE_OFF];


if (hl_endpos->lnum == startpos->lnum
&& hl_endpos->col < startpos->col)
hl_endpos->col = startpos->col;
limit_pos(hl_endpos, m_endpos);



*m_endpos = *hl_endpos;
}
else
{
*end_idx = 0;
*hl_endpos = *end_endpos;
}

*flagsp = spp->sp_flags;

had_match = TRUE;
break;
}


if (!had_match)
m_endpos->lnum = 0;

restore_chartab(buf_chartab);


unref_extmatch(re_extmatch_in);
re_extmatch_in = NULL;
}




static void
limit_pos(lpos_T *pos, lpos_T *limit)
{
if (pos->lnum > limit->lnum)
*pos = *limit;
else if (pos->lnum == limit->lnum && pos->col > limit->col)
pos->col = limit->col;
}




static void
limit_pos_zero(
lpos_T *pos,
lpos_T *limit)
{
if (pos->lnum == 0)
*pos = *limit;
else
limit_pos(pos, limit);
}




static void
syn_add_end_off(
lpos_T *result, 
regmmatch_T *regmatch, 
synpat_T *spp, 
int idx, 
int extra) 
{
int col;
int off;
char_u *base;
char_u *p;

if (spp->sp_off_flags & (1 << idx))
{
result->lnum = regmatch->startpos[0].lnum;
col = regmatch->startpos[0].col;
off = spp->sp_offsets[idx] + extra;
}
else
{
result->lnum = regmatch->endpos[0].lnum;
col = regmatch->endpos[0].col;
off = spp->sp_offsets[idx];
}


if (result->lnum > syn_buf->b_ml.ml_line_count)
col = 0;
else if (off != 0)
{
base = ml_get_buf(syn_buf, result->lnum, FALSE);
p = base + col;
if (off > 0)
{
while (off-- > 0 && *p != NUL)
MB_PTR_ADV(p);
}
else if (off < 0)
{
while (off++ < 0 && base < p)
MB_PTR_BACK(base, p);
}
col = (int)(p - base);
}
result->col = col;
}





static void
syn_add_start_off(
lpos_T *result, 
regmmatch_T *regmatch, 
synpat_T *spp,
int idx,
int extra) 
{
int col;
int off;
char_u *base;
char_u *p;

if (spp->sp_off_flags & (1 << (idx + SPO_COUNT)))
{
result->lnum = regmatch->endpos[0].lnum;
col = regmatch->endpos[0].col;
off = spp->sp_offsets[idx] + extra;
}
else
{
result->lnum = regmatch->startpos[0].lnum;
col = regmatch->startpos[0].col;
off = spp->sp_offsets[idx];
}
if (result->lnum > syn_buf->b_ml.ml_line_count)
{

result->lnum = syn_buf->b_ml.ml_line_count;
col = (int)STRLEN(ml_get_buf(syn_buf, result->lnum, FALSE));
}
if (off != 0)
{
base = ml_get_buf(syn_buf, result->lnum, FALSE);
p = base + col;
if (off > 0)
{
while (off-- && *p != NUL)
MB_PTR_ADV(p);
}
else if (off < 0)
{
while (off++ && base < p)
MB_PTR_BACK(base, p);
}
col = (int)(p - base);
}
result->col = col;
}




static char_u *
syn_getcurline(void)
{
return ml_get_buf(syn_buf, current_lnum, FALSE);
}





static int
syn_regexec(
regmmatch_T *rmp,
linenr_T lnum,
colnr_T col,
syn_time_T *st UNUSED)
{
int r;
#if defined(FEAT_RELTIME)
int timed_out = FALSE;
#endif
#if defined(FEAT_PROFILE)
proftime_T pt;

if (syn_time_on)
profile_start(&pt);
#endif

if (rmp->regprog == NULL)



return FALSE;

rmp->rmm_maxcol = syn_buf->b_p_smc;
r = vim_regexec_multi(rmp, syn_win, syn_buf, lnum, col,
#if defined(FEAT_RELTIME)
syn_tm, &timed_out
#else
NULL, NULL
#endif
);

#if defined(FEAT_PROFILE)
if (syn_time_on)
{
profile_end(&pt);
profile_add(&st->total, &pt);
if (profile_cmp(&pt, &st->slowest) < 0)
st->slowest = pt;
++st->count;
if (r > 0)
++st->match;
}
#endif
#if defined(FEAT_RELTIME)
if (timed_out && !syn_win->w_s->b_syn_slow)
{
syn_win->w_s->b_syn_slow = TRUE;
msg(_("'redrawtime' exceeded, syntax highlighting disabled"));
}
#endif

if (r > 0)
{
rmp->startpos[0].lnum += lnum;
rmp->endpos[0].lnum += lnum;
return TRUE;
}
return FALSE;
}






static int
check_keyword_id(
char_u *line,
int startcol, 
int *endcolp, 
long *flagsp, 
short **next_listp, 
stateitem_T *cur_si, 
int *ccharp UNUSED) 
{
keyentry_T *kp;
char_u *kwp;
int round;
int kwlen;
char_u keyword[MAXKEYWLEN + 1]; 
hashtab_T *ht;
hashitem_T *hi;



kwp = line + startcol;
kwlen = 0;
do
{
if (has_mbyte)
kwlen += (*mb_ptr2len)(kwp + kwlen);
else
++kwlen;
}
while (vim_iswordp_buf(kwp + kwlen, syn_buf));

if (kwlen > MAXKEYWLEN)
return 0;





vim_strncpy(keyword, kwp, kwlen);






for (round = 1; round <= 2; ++round)
{
ht = round == 1 ? &syn_block->b_keywtab : &syn_block->b_keywtab_ic;
if (ht->ht_used == 0)
continue;
if (round == 2) 
(void)str_foldcase(kwp, kwlen, keyword, MAXKEYWLEN + 1);








hi = hash_find(ht, keyword);
if (!HASHITEM_EMPTY(hi))
for (kp = HI2KE(hi); kp != NULL; kp = kp->ke_next)
{
if (current_next_list != 0
? in_id_list(NULL, current_next_list, &kp->k_syn, 0)
: (cur_si == NULL
? !(kp->flags & HL_CONTAINED)
: in_id_list(cur_si, cur_si->si_cont_list,
&kp->k_syn, kp->flags & HL_CONTAINED)))
{
*endcolp = startcol + kwlen;
*flagsp = kp->flags;
*next_listp = kp->next_list;
#if defined(FEAT_CONCEAL)
*ccharp = kp->k_char;
#endif
return kp->k_syn.id;
}
}
}
return 0;
}




static void
syn_cmd_conceal(exarg_T *eap UNUSED, int syncing UNUSED)
{
#if defined(FEAT_CONCEAL)
char_u *arg = eap->arg;
char_u *next;

eap->nextcmd = find_nextcmd(arg);
if (eap->skip)
return;

next = skiptowhite(arg);
if (*arg == NUL)
{
if (curwin->w_s->b_syn_conceal)
msg(_("syntax conceal on"));
else
msg(_("syntax conceal off"));
}
else if (STRNICMP(arg, "on", 2) == 0 && next - arg == 2)
curwin->w_s->b_syn_conceal = TRUE;
else if (STRNICMP(arg, "off", 3) == 0 && next - arg == 3)
curwin->w_s->b_syn_conceal = FALSE;
else
semsg(_("E390: Illegal argument: %s"), arg);
#endif
}




static void
syn_cmd_case(exarg_T *eap, int syncing UNUSED)
{
char_u *arg = eap->arg;
char_u *next;

eap->nextcmd = find_nextcmd(arg);
if (eap->skip)
return;

next = skiptowhite(arg);
if (*arg == NUL)
{
if (curwin->w_s->b_syn_ic)
msg(_("syntax case ignore"));
else
msg(_("syntax case match"));
}
else if (STRNICMP(arg, "match", 5) == 0 && next - arg == 5)
curwin->w_s->b_syn_ic = FALSE;
else if (STRNICMP(arg, "ignore", 6) == 0 && next - arg == 6)
curwin->w_s->b_syn_ic = TRUE;
else
semsg(_("E390: Illegal argument: %s"), arg);
}




static void
syn_cmd_spell(exarg_T *eap, int syncing UNUSED)
{
char_u *arg = eap->arg;
char_u *next;

eap->nextcmd = find_nextcmd(arg);
if (eap->skip)
return;

next = skiptowhite(arg);
if (*arg == NUL)
{
if (curwin->w_s->b_syn_spell == SYNSPL_TOP)
msg(_("syntax spell toplevel"));
else if (curwin->w_s->b_syn_spell == SYNSPL_NOTOP)
msg(_("syntax spell notoplevel"));
else
msg(_("syntax spell default"));
}
else if (STRNICMP(arg, "toplevel", 8) == 0 && next - arg == 8)
curwin->w_s->b_syn_spell = SYNSPL_TOP;
else if (STRNICMP(arg, "notoplevel", 10) == 0 && next - arg == 10)
curwin->w_s->b_syn_spell = SYNSPL_NOTOP;
else if (STRNICMP(arg, "default", 7) == 0 && next - arg == 7)
curwin->w_s->b_syn_spell = SYNSPL_DEFAULT;
else
{
semsg(_("E390: Illegal argument: %s"), arg);
return;
}


redraw_win_later(curwin, NOT_VALID);
}




static void
syn_cmd_iskeyword(exarg_T *eap, int syncing UNUSED)
{
char_u *arg = eap->arg;
char_u save_chartab[32];
char_u *save_isk;

if (eap->skip)
return;

arg = skipwhite(arg);
if (*arg == NUL)
{
msg_puts("\n");
if (curwin->w_s->b_syn_isk != empty_option)
{
msg_puts(_("syntax iskeyword "));
msg_outtrans(curwin->w_s->b_syn_isk);
}
else
msg_outtrans((char_u *)_("syntax iskeyword not set"));
}
else
{
if (STRNICMP(arg, "clear", 5) == 0)
{
mch_memmove(curwin->w_s->b_syn_chartab, curbuf->b_chartab,
(size_t)32);
clear_string_option(&curwin->w_s->b_syn_isk);
}
else
{
mch_memmove(save_chartab, curbuf->b_chartab, (size_t)32);
save_isk = curbuf->b_p_isk;
curbuf->b_p_isk = vim_strsave(arg);

buf_init_chartab(curbuf, FALSE);
mch_memmove(curwin->w_s->b_syn_chartab, curbuf->b_chartab,
(size_t)32);
mch_memmove(curbuf->b_chartab, save_chartab, (size_t)32);
clear_string_option(&curwin->w_s->b_syn_isk);
curwin->w_s->b_syn_isk = curbuf->b_p_isk;
curbuf->b_p_isk = save_isk;
}
}
redraw_win_later(curwin, NOT_VALID);
}




void
syntax_clear(synblock_T *block)
{
int i;

block->b_syn_error = FALSE; 
#if defined(FEAT_RELTIME)
block->b_syn_slow = FALSE; 
#endif
block->b_syn_ic = FALSE; 
block->b_syn_spell = SYNSPL_DEFAULT; 
block->b_syn_containedin = FALSE;
#if defined(FEAT_CONCEAL)
block->b_syn_conceal = FALSE;
#endif


clear_keywtab(&block->b_keywtab);
clear_keywtab(&block->b_keywtab_ic);


for (i = block->b_syn_patterns.ga_len; --i >= 0; )
syn_clear_pattern(block, i);
ga_clear(&block->b_syn_patterns);


for (i = block->b_syn_clusters.ga_len; --i >= 0; )
syn_clear_cluster(block, i);
ga_clear(&block->b_syn_clusters);
block->b_spell_cluster_id = 0;
block->b_nospell_cluster_id = 0;

block->b_syn_sync_flags = 0;
block->b_syn_sync_minlines = 0;
block->b_syn_sync_maxlines = 0;
block->b_syn_sync_linebreaks = 0;

vim_regfree(block->b_syn_linecont_prog);
block->b_syn_linecont_prog = NULL;
VIM_CLEAR(block->b_syn_linecont_pat);
#if defined(FEAT_FOLDING)
block->b_syn_folditems = 0;
#endif
clear_string_option(&block->b_syn_isk);


syn_stack_free_all(block);
invalidate_current_state();


running_syn_inc_tag = 0;
}




void
reset_synblock(win_T *wp)
{
if (wp->w_s != &wp->w_buffer->b_s)
{
syntax_clear(wp->w_s);
vim_free(wp->w_s);
wp->w_s = &wp->w_buffer->b_s;
}
}




static void
syntax_sync_clear(void)
{
int i;


for (i = curwin->w_s->b_syn_patterns.ga_len; --i >= 0; )
if (SYN_ITEMS(curwin->w_s)[i].sp_syncing)
syn_remove_pattern(curwin->w_s, i);

curwin->w_s->b_syn_sync_flags = 0;
curwin->w_s->b_syn_sync_minlines = 0;
curwin->w_s->b_syn_sync_maxlines = 0;
curwin->w_s->b_syn_sync_linebreaks = 0;

vim_regfree(curwin->w_s->b_syn_linecont_prog);
curwin->w_s->b_syn_linecont_prog = NULL;
VIM_CLEAR(curwin->w_s->b_syn_linecont_pat);
clear_string_option(&curwin->w_s->b_syn_isk);

syn_stack_free_all(curwin->w_s); 
}




static void
syn_remove_pattern(
synblock_T *block,
int idx)
{
synpat_T *spp;

spp = &(SYN_ITEMS(block)[idx]);
#if defined(FEAT_FOLDING)
if (spp->sp_flags & HL_FOLD)
--block->b_syn_folditems;
#endif
syn_clear_pattern(block, idx);
mch_memmove(spp, spp + 1,
sizeof(synpat_T) * (block->b_syn_patterns.ga_len - idx - 1));
--block->b_syn_patterns.ga_len;
}





static void
syn_clear_pattern(synblock_T *block, int i)
{
vim_free(SYN_ITEMS(block)[i].sp_pattern);
vim_regfree(SYN_ITEMS(block)[i].sp_prog);

if (i == 0 || SYN_ITEMS(block)[i - 1].sp_type != SPTYPE_START)
{
vim_free(SYN_ITEMS(block)[i].sp_cont_list);
vim_free(SYN_ITEMS(block)[i].sp_next_list);
vim_free(SYN_ITEMS(block)[i].sp_syn.cont_in_list);
}
}




static void
syn_clear_cluster(synblock_T *block, int i)
{
vim_free(SYN_CLSTR(block)[i].scl_name);
vim_free(SYN_CLSTR(block)[i].scl_name_u);
vim_free(SYN_CLSTR(block)[i].scl_list);
}




static void
syn_cmd_clear(exarg_T *eap, int syncing)
{
char_u *arg = eap->arg;
char_u *arg_end;
int id;

eap->nextcmd = find_nextcmd(arg);
if (eap->skip)
return;







if (curwin->w_s->b_syn_topgrp != 0)
return;

if (ends_excmd(*arg))
{



if (syncing)
syntax_sync_clear();
else
{
syntax_clear(curwin->w_s);
if (curwin->w_s == &curwin->w_buffer->b_s)
do_unlet((char_u *)"b:current_syntax", TRUE);
do_unlet((char_u *)"w:current_syntax", TRUE);
}
}
else
{



while (!ends_excmd(*arg))
{
arg_end = skiptowhite(arg);
if (*arg == '@')
{
id = syn_scl_namen2id(arg + 1, (int)(arg_end - arg - 1));
if (id == 0)
{
semsg(_("E391: No such syntax cluster: %s"), arg);
break;
}
else
{





short scl_id = id - SYNID_CLUSTER;

VIM_CLEAR(SYN_CLSTR(curwin->w_s)[scl_id].scl_list);
}
}
else
{
id = syn_namen2id(arg, (int)(arg_end - arg));
if (id == 0)
{
semsg(_(e_nogroup), arg);
break;
}
else
syn_clear_one(id, syncing);
}
arg = skipwhite(arg_end);
}
}
redraw_curbuf_later(SOME_VALID);
syn_stack_free_all(curwin->w_s); 
}




static void
syn_clear_one(int id, int syncing)
{
synpat_T *spp;
int idx;


if (!syncing)
{
(void)syn_clear_keyword(id, &curwin->w_s->b_keywtab);
(void)syn_clear_keyword(id, &curwin->w_s->b_keywtab_ic);
}


for (idx = curwin->w_s->b_syn_patterns.ga_len; --idx >= 0; )
{
spp = &(SYN_ITEMS(curwin->w_s)[idx]);
if (spp->sp_syn.id != id || spp->sp_syncing != syncing)
continue;
syn_remove_pattern(curwin->w_s, idx);
}
}




static void
syn_cmd_on(exarg_T *eap, int syncing UNUSED)
{
syn_cmd_onoff(eap, "syntax");
}




static void
syn_cmd_enable(exarg_T *eap, int syncing UNUSED)
{
set_internal_string_var((char_u *)"syntax_cmd", (char_u *)"enable");
syn_cmd_onoff(eap, "syntax");
do_unlet((char_u *)"g:syntax_cmd", TRUE);
}





static void
syn_cmd_reset(exarg_T *eap, int syncing UNUSED)
{
eap->nextcmd = check_nextcmd(eap->arg);
if (!eap->skip)
{
set_internal_string_var((char_u *)"syntax_cmd", (char_u *)"reset");
do_cmdline_cmd((char_u *)"runtime! syntax/syncolor.vim");
do_unlet((char_u *)"g:syntax_cmd", TRUE);
}
}




static void
syn_cmd_manual(exarg_T *eap, int syncing UNUSED)
{
syn_cmd_onoff(eap, "manual");
}




static void
syn_cmd_off(exarg_T *eap, int syncing UNUSED)
{
syn_cmd_onoff(eap, "nosyntax");
}

static void
syn_cmd_onoff(exarg_T *eap, char *name)
{
char_u buf[100];

eap->nextcmd = check_nextcmd(eap->arg);
if (!eap->skip)
{
STRCPY(buf, "so ");
vim_snprintf((char *)buf + 3, sizeof(buf) - 3, SYNTAX_FNAME, name);
do_cmdline_cmd(buf);
}
}




static void
syn_cmd_list(
exarg_T *eap,
int syncing) 
{
char_u *arg = eap->arg;
int id;
char_u *arg_end;

eap->nextcmd = find_nextcmd(arg);
if (eap->skip)
return;

if (!syntax_present(curwin))
{
msg(_(msg_no_items));
return;
}

if (syncing)
{
if (curwin->w_s->b_syn_sync_flags & SF_CCOMMENT)
{
msg_puts(_("syncing on C-style comments"));
syn_lines_msg();
syn_match_msg();
return;
}
else if (!(curwin->w_s->b_syn_sync_flags & SF_MATCH))
{
if (curwin->w_s->b_syn_sync_minlines == 0)
msg_puts(_("no syncing"));
else
{
msg_puts(_("syncing starts "));
msg_outnum(curwin->w_s->b_syn_sync_minlines);
msg_puts(_(" lines before top line"));
syn_match_msg();
}
return;
}
msg_puts_title(_("\n--- Syntax sync items ---"));
if (curwin->w_s->b_syn_sync_minlines > 0
|| curwin->w_s->b_syn_sync_maxlines > 0
|| curwin->w_s->b_syn_sync_linebreaks > 0)
{
msg_puts(_("\nsyncing on items"));
syn_lines_msg();
syn_match_msg();
}
}
else
msg_puts_title(_("\n--- Syntax items ---"));
if (ends_excmd(*arg))
{



for (id = 1; id <= highlight_num_groups() && !got_int; ++id)
syn_list_one(id, syncing, FALSE);
for (id = 0; id < curwin->w_s->b_syn_clusters.ga_len && !got_int; ++id)
syn_list_cluster(id);
}
else
{



while (!ends_excmd(*arg) && !got_int)
{
arg_end = skiptowhite(arg);
if (*arg == '@')
{
id = syn_scl_namen2id(arg + 1, (int)(arg_end - arg - 1));
if (id == 0)
semsg(_("E392: No such syntax cluster: %s"), arg);
else
syn_list_cluster(id - SYNID_CLUSTER);
}
else
{
id = syn_namen2id(arg, (int)(arg_end - arg));
if (id == 0)
semsg(_(e_nogroup), arg);
else
syn_list_one(id, syncing, TRUE);
}
arg = skipwhite(arg_end);
}
}
eap->nextcmd = check_nextcmd(arg);
}

static void
syn_lines_msg(void)
{
if (curwin->w_s->b_syn_sync_maxlines > 0
|| curwin->w_s->b_syn_sync_minlines > 0)
{
msg_puts("; ");
if (curwin->w_s->b_syn_sync_minlines > 0)
{
msg_puts(_("minimal "));
msg_outnum(curwin->w_s->b_syn_sync_minlines);
if (curwin->w_s->b_syn_sync_maxlines)
msg_puts(", ");
}
if (curwin->w_s->b_syn_sync_maxlines > 0)
{
msg_puts(_("maximal "));
msg_outnum(curwin->w_s->b_syn_sync_maxlines);
}
msg_puts(_(" lines before top line"));
}
}

static void
syn_match_msg(void)
{
if (curwin->w_s->b_syn_sync_linebreaks > 0)
{
msg_puts(_("; match "));
msg_outnum(curwin->w_s->b_syn_sync_linebreaks);
msg_puts(_(" line breaks"));
}
}

static int last_matchgroup;

struct name_list
{
int flag;
char *name;
};

static void syn_list_flags(struct name_list *nl, int flags, int attr);




static void
syn_list_one(
int id,
int syncing, 
int link_only) 
{
int attr;
int idx;
int did_header = FALSE;
synpat_T *spp;
static struct name_list namelist1[] =
{
{HL_DISPLAY, "display"},
{HL_CONTAINED, "contained"},
{HL_ONELINE, "oneline"},
{HL_KEEPEND, "keepend"},
{HL_EXTEND, "extend"},
{HL_EXCLUDENL, "excludenl"},
{HL_TRANSP, "transparent"},
{HL_FOLD, "fold"},
#if defined(FEAT_CONCEAL)
{HL_CONCEAL, "conceal"},
{HL_CONCEALENDS, "concealends"},
#endif
{0, NULL}
};
static struct name_list namelist2[] =
{
{HL_SKIPWHITE, "skipwhite"},
{HL_SKIPNL, "skipnl"},
{HL_SKIPEMPTY, "skipempty"},
{0, NULL}
};

attr = HL_ATTR(HLF_D); 


if (!syncing)
{
did_header = syn_list_keywords(id, &curwin->w_s->b_keywtab, FALSE, attr);
did_header = syn_list_keywords(id, &curwin->w_s->b_keywtab_ic,
did_header, attr);
}


for (idx = 0; idx < curwin->w_s->b_syn_patterns.ga_len && !got_int; ++idx)
{
spp = &(SYN_ITEMS(curwin->w_s)[idx]);
if (spp->sp_syn.id != id || spp->sp_syncing != syncing)
continue;

(void)syn_list_header(did_header, 999, id);
did_header = TRUE;
last_matchgroup = 0;
if (spp->sp_type == SPTYPE_MATCH)
{
put_pattern("match", ' ', spp, attr);
msg_putchar(' ');
}
else if (spp->sp_type == SPTYPE_START)
{
while (SYN_ITEMS(curwin->w_s)[idx].sp_type == SPTYPE_START)
put_pattern("start", '=', &SYN_ITEMS(curwin->w_s)[idx++], attr);
if (SYN_ITEMS(curwin->w_s)[idx].sp_type == SPTYPE_SKIP)
put_pattern("skip", '=', &SYN_ITEMS(curwin->w_s)[idx++], attr);
while (idx < curwin->w_s->b_syn_patterns.ga_len
&& SYN_ITEMS(curwin->w_s)[idx].sp_type == SPTYPE_END)
put_pattern("end", '=', &SYN_ITEMS(curwin->w_s)[idx++], attr);
--idx;
msg_putchar(' ');
}
syn_list_flags(namelist1, spp->sp_flags, attr);

if (spp->sp_cont_list != NULL)
put_id_list((char_u *)"contains", spp->sp_cont_list, attr);

if (spp->sp_syn.cont_in_list != NULL)
put_id_list((char_u *)"containedin",
spp->sp_syn.cont_in_list, attr);

if (spp->sp_next_list != NULL)
{
put_id_list((char_u *)"nextgroup", spp->sp_next_list, attr);
syn_list_flags(namelist2, spp->sp_flags, attr);
}
if (spp->sp_flags & (HL_SYNC_HERE|HL_SYNC_THERE))
{
if (spp->sp_flags & HL_SYNC_HERE)
msg_puts_attr("grouphere", attr);
else
msg_puts_attr("groupthere", attr);
msg_putchar(' ');
if (spp->sp_sync_idx >= 0)
msg_outtrans(highlight_group_name(SYN_ITEMS(curwin->w_s)
[spp->sp_sync_idx].sp_syn.id - 1));
else
msg_puts("NONE");
msg_putchar(' ');
}
}


if (highlight_link_id(id - 1) && (did_header || link_only) && !got_int)
{
(void)syn_list_header(did_header, 999, id);
msg_puts_attr("links to", attr);
msg_putchar(' ');
msg_outtrans(highlight_group_name(highlight_link_id(id - 1) - 1));
}
}

static void
syn_list_flags(struct name_list *nlist, int flags, int attr)
{
int i;

for (i = 0; nlist[i].flag != 0; ++i)
if (flags & nlist[i].flag)
{
msg_puts_attr(nlist[i].name, attr);
msg_putchar(' ');
}
}




static void
syn_list_cluster(int id)
{
int endcol = 15;


msg_putchar('\n');
msg_outtrans(SYN_CLSTR(curwin->w_s)[id].scl_name);

if (msg_col >= endcol) 
endcol = msg_col + 1;
if (Columns <= endcol) 
endcol = Columns - 1;

msg_advance(endcol);
if (SYN_CLSTR(curwin->w_s)[id].scl_list != NULL)
{
put_id_list((char_u *)"cluster", SYN_CLSTR(curwin->w_s)[id].scl_list,
HL_ATTR(HLF_D));
}
else
{
msg_puts_attr("cluster", HL_ATTR(HLF_D));
msg_puts("=NONE");
}
}

static void
put_id_list(char_u *name, short *list, int attr)
{
short *p;

msg_puts_attr((char *)name, attr);
msg_putchar('=');
for (p = list; *p; ++p)
{
if (*p >= SYNID_ALLBUT && *p < SYNID_TOP)
{
if (p[1])
msg_puts("ALLBUT");
else
msg_puts("ALL");
}
else if (*p >= SYNID_TOP && *p < SYNID_CONTAINED)
{
msg_puts("TOP");
}
else if (*p >= SYNID_CONTAINED && *p < SYNID_CLUSTER)
{
msg_puts("CONTAINED");
}
else if (*p >= SYNID_CLUSTER)
{
short scl_id = *p - SYNID_CLUSTER;

msg_putchar('@');
msg_outtrans(SYN_CLSTR(curwin->w_s)[scl_id].scl_name);
}
else
msg_outtrans(highlight_group_name(*p - 1));
if (p[1])
msg_putchar(',');
}
msg_putchar(' ');
}

static void
put_pattern(
char *s,
int c,
synpat_T *spp,
int attr)
{
long n;
int mask;
int first;
static char *sepchars = "/+=-#@\"|'^&";
int i;


if (last_matchgroup != spp->sp_syn_match_id)
{
last_matchgroup = spp->sp_syn_match_id;
msg_puts_attr("matchgroup", attr);
msg_putchar('=');
if (last_matchgroup == 0)
msg_outtrans((char_u *)"NONE");
else
msg_outtrans(highlight_group_name(last_matchgroup - 1));
msg_putchar(' ');
}


msg_puts_attr(s, attr);
msg_putchar(c);


for (i = 0; vim_strchr(spp->sp_pattern, sepchars[i]) != NULL; )
if (sepchars[++i] == NUL)
{
i = 0; 
break;
}
msg_putchar(sepchars[i]);
msg_outtrans(spp->sp_pattern);
msg_putchar(sepchars[i]);


first = TRUE;
for (i = 0; i < SPO_COUNT; ++i)
{
mask = (1 << i);
if (spp->sp_off_flags & (mask + (mask << SPO_COUNT)))
{
if (!first)
msg_putchar(','); 
msg_puts(spo_name_tab[i]);
n = spp->sp_offsets[i];
if (i != SPO_LC_OFF)
{
if (spp->sp_off_flags & mask)
msg_putchar('s');
else
msg_putchar('e');
if (n > 0)
msg_putchar('+');
}
if (n || i == SPO_LC_OFF)
msg_outnum(n);
first = FALSE;
}
}
msg_putchar(' ');
}





static int
syn_list_keywords(
int id,
hashtab_T *ht,
int did_header, 
int attr)
{
int outlen;
hashitem_T *hi;
keyentry_T *kp;
int todo;
int prev_contained = 0;
short *prev_next_list = NULL;
short *prev_cont_in_list = NULL;
int prev_skipnl = 0;
int prev_skipwhite = 0;
int prev_skipempty = 0;





todo = (int)ht->ht_used;
for (hi = ht->ht_array; todo > 0 && !got_int; ++hi)
{
if (!HASHITEM_EMPTY(hi))
{
--todo;
for (kp = HI2KE(hi); kp != NULL && !got_int; kp = kp->ke_next)
{
if (kp->k_syn.id == id)
{
if (prev_contained != (kp->flags & HL_CONTAINED)
|| prev_skipnl != (kp->flags & HL_SKIPNL)
|| prev_skipwhite != (kp->flags & HL_SKIPWHITE)
|| prev_skipempty != (kp->flags & HL_SKIPEMPTY)
|| prev_cont_in_list != kp->k_syn.cont_in_list
|| prev_next_list != kp->next_list)
outlen = 9999;
else
outlen = (int)STRLEN(kp->keyword);

if (syn_list_header(did_header, outlen, id))
{
prev_contained = 0;
prev_next_list = NULL;
prev_cont_in_list = NULL;
prev_skipnl = 0;
prev_skipwhite = 0;
prev_skipempty = 0;
}
did_header = TRUE;
if (prev_contained != (kp->flags & HL_CONTAINED))
{
msg_puts_attr("contained", attr);
msg_putchar(' ');
prev_contained = (kp->flags & HL_CONTAINED);
}
if (kp->k_syn.cont_in_list != prev_cont_in_list)
{
put_id_list((char_u *)"containedin",
kp->k_syn.cont_in_list, attr);
msg_putchar(' ');
prev_cont_in_list = kp->k_syn.cont_in_list;
}
if (kp->next_list != prev_next_list)
{
put_id_list((char_u *)"nextgroup", kp->next_list, attr);
msg_putchar(' ');
prev_next_list = kp->next_list;
if (kp->flags & HL_SKIPNL)
{
msg_puts_attr("skipnl", attr);
msg_putchar(' ');
prev_skipnl = (kp->flags & HL_SKIPNL);
}
if (kp->flags & HL_SKIPWHITE)
{
msg_puts_attr("skipwhite", attr);
msg_putchar(' ');
prev_skipwhite = (kp->flags & HL_SKIPWHITE);
}
if (kp->flags & HL_SKIPEMPTY)
{
msg_puts_attr("skipempty", attr);
msg_putchar(' ');
prev_skipempty = (kp->flags & HL_SKIPEMPTY);
}
}
msg_outtrans(kp->keyword);
}
}
}
}

return did_header;
}

static void
syn_clear_keyword(int id, hashtab_T *ht)
{
hashitem_T *hi;
keyentry_T *kp;
keyentry_T *kp_prev;
keyentry_T *kp_next;
int todo;

hash_lock(ht);
todo = (int)ht->ht_used;
for (hi = ht->ht_array; todo > 0; ++hi)
{
if (!HASHITEM_EMPTY(hi))
{
--todo;
kp_prev = NULL;
for (kp = HI2KE(hi); kp != NULL; )
{
if (kp->k_syn.id == id)
{
kp_next = kp->ke_next;
if (kp_prev == NULL)
{
if (kp_next == NULL)
hash_remove(ht, hi);
else
hi->hi_key = KE2HIKEY(kp_next);
}
else
kp_prev->ke_next = kp_next;
vim_free(kp->next_list);
vim_free(kp->k_syn.cont_in_list);
vim_free(kp);
kp = kp_next;
}
else
{
kp_prev = kp;
kp = kp->ke_next;
}
}
}
}
hash_unlock(ht);
}




static void
clear_keywtab(hashtab_T *ht)
{
hashitem_T *hi;
int todo;
keyentry_T *kp;
keyentry_T *kp_next;

todo = (int)ht->ht_used;
for (hi = ht->ht_array; todo > 0; ++hi)
{
if (!HASHITEM_EMPTY(hi))
{
--todo;
for (kp = HI2KE(hi); kp != NULL; kp = kp_next)
{
kp_next = kp->ke_next;
vim_free(kp->next_list);
vim_free(kp->k_syn.cont_in_list);
vim_free(kp);
}
}
}
hash_clear(ht);
hash_init(ht);
}




static void
add_keyword(
char_u *name, 
int id, 
int flags, 
short *cont_in_list, 
short *next_list, 
int conceal_char)
{
keyentry_T *kp;
hashtab_T *ht;
hashitem_T *hi;
char_u *name_ic;
long_u hash;
char_u name_folded[MAXKEYWLEN + 1];

if (curwin->w_s->b_syn_ic)
name_ic = str_foldcase(name, (int)STRLEN(name),
name_folded, MAXKEYWLEN + 1);
else
name_ic = name;
kp = alloc(offsetof(keyentry_T, keyword) + STRLEN(name_ic) + 1);
if (kp == NULL)
return;
STRCPY(kp->keyword, name_ic);
kp->k_syn.id = id;
kp->k_syn.inc_tag = current_syn_inc_tag;
kp->flags = flags;
kp->k_char = conceal_char;
kp->k_syn.cont_in_list = copy_id_list(cont_in_list);
if (cont_in_list != NULL)
curwin->w_s->b_syn_containedin = TRUE;
kp->next_list = copy_id_list(next_list);

if (curwin->w_s->b_syn_ic)
ht = &curwin->w_s->b_keywtab_ic;
else
ht = &curwin->w_s->b_keywtab;

hash = hash_hash(kp->keyword);
hi = hash_lookup(ht, kp->keyword, hash);
if (HASHITEM_EMPTY(hi))
{

kp->ke_next = NULL;
hash_add_item(ht, hi, kp->keyword, hash);
}
else
{

kp->ke_next = HI2KE(hi);
hi->hi_key = KE2HIKEY(kp);
}
}






static char_u *
get_group_name(
char_u *arg, 
char_u **name_end) 
{
char_u *rest;

*name_end = skiptowhite(arg);
rest = skipwhite(*name_end);





if (ends_excmd(*arg) || *rest == NUL)
return NULL;
return rest;
}









static char_u *
get_syn_options(
char_u *arg, 
syn_opt_arg_T *opt, 
int *conceal_char UNUSED,
int skip) 
{
char_u *gname_start, *gname;
int syn_id;
int len;
char *p;
int i;
int fidx;
static struct flag
{
char *name;
int argtype;
int flags;
} flagtab[] = { {"cCoOnNtTaAiInNeEdD", 0, HL_CONTAINED},
{"oOnNeElLiInNeE", 0, HL_ONELINE},
{"kKeEeEpPeEnNdD", 0, HL_KEEPEND},
{"eExXtTeEnNdD", 0, HL_EXTEND},
{"eExXcClLuUdDeEnNlL", 0, HL_EXCLUDENL},
{"tTrRaAnNsSpPaArReEnNtT", 0, HL_TRANSP},
{"sSkKiIpPnNlL", 0, HL_SKIPNL},
{"sSkKiIpPwWhHiItTeE", 0, HL_SKIPWHITE},
{"sSkKiIpPeEmMpPtTyY", 0, HL_SKIPEMPTY},
{"gGrRoOuUpPhHeErReE", 0, HL_SYNC_HERE},
{"gGrRoOuUpPtThHeErReE", 0, HL_SYNC_THERE},
{"dDiIsSpPlLaAyY", 0, HL_DISPLAY},
{"fFoOlLdD", 0, HL_FOLD},
{"cCoOnNcCeEaAlL", 0, HL_CONCEAL},
{"cCoOnNcCeEaAlLeEnNdDsS", 0, HL_CONCEALENDS},
{"cCcChHaArR", 11, 0},
{"cCoOnNtTaAiInNsS", 1, 0},
{"cCoOnNtTaAiInNeEdDiInN", 2, 0},
{"nNeExXtTgGrRoOuUpP", 3, 0},
};
static char *first_letters = "cCoOkKeEtTsSgGdDfFnN";

if (arg == NULL) 
return NULL;

#if defined(FEAT_CONCEAL)
if (curwin->w_s->b_syn_conceal)
opt->flags |= HL_CONCEAL;
#endif

for (;;)
{





if (strchr(first_letters, *arg) == NULL)
break;

for (fidx = sizeof(flagtab) / sizeof(struct flag); --fidx >= 0; )
{
p = flagtab[fidx].name;
for (i = 0, len = 0; p[i] != NUL; i += 2, ++len)
if (arg[len] != p[i] && arg[len] != p[i + 1])
break;
if (p[i] == NUL && (VIM_ISWHITE(arg[len])
|| (flagtab[fidx].argtype > 0
? arg[len] == '='
: ends_excmd(arg[len]))))
{
if (opt->keyword
&& (flagtab[fidx].flags == HL_DISPLAY
|| flagtab[fidx].flags == HL_FOLD
|| flagtab[fidx].flags == HL_EXTEND))

fidx = -1;
break;
}
}
if (fidx < 0) 
break;

if (flagtab[fidx].argtype == 1)
{
if (!opt->has_cont_list)
{
emsg(_("E395: contains argument not accepted here"));
return NULL;
}
if (get_id_list(&arg, 8, &opt->cont_list, skip) == FAIL)
return NULL;
}
else if (flagtab[fidx].argtype == 2)
{
if (get_id_list(&arg, 11, &opt->cont_in_list, skip) == FAIL)
return NULL;
}
else if (flagtab[fidx].argtype == 3)
{
if (get_id_list(&arg, 9, &opt->next_list, skip) == FAIL)
return NULL;
}
else if (flagtab[fidx].argtype == 11 && arg[5] == '=')
{

if (has_mbyte)
{
#if defined(FEAT_CONCEAL)
*conceal_char = mb_ptr2char(arg + 6);
#endif
arg += mb_ptr2len(arg + 6) - 1;
}
else
{
#if defined(FEAT_CONCEAL)
*conceal_char = arg[6];
#else
;
#endif
}
#if defined(FEAT_CONCEAL)
if (!vim_isprintc_strict(*conceal_char))
{
emsg(_("E844: invalid cchar value"));
return NULL;
}
#endif
arg = skipwhite(arg + 7);
}
else
{
opt->flags |= flagtab[fidx].flags;
arg = skipwhite(arg + len);

if (flagtab[fidx].flags == HL_SYNC_HERE
|| flagtab[fidx].flags == HL_SYNC_THERE)
{
if (opt->sync_idx == NULL)
{
emsg(_("E393: group[t]here not accepted here"));
return NULL;
}
gname_start = arg;
arg = skiptowhite(arg);
if (gname_start == arg)
return NULL;
gname = vim_strnsave(gname_start, (int)(arg - gname_start));
if (gname == NULL)
return NULL;
if (STRCMP(gname, "NONE") == 0)
*opt->sync_idx = NONE_IDX;
else
{
syn_id = syn_name2id(gname);
for (i = curwin->w_s->b_syn_patterns.ga_len; --i >= 0; )
if (SYN_ITEMS(curwin->w_s)[i].sp_syn.id == syn_id
&& SYN_ITEMS(curwin->w_s)[i].sp_type == SPTYPE_START)
{
*opt->sync_idx = i;
break;
}
if (i < 0)
{
semsg(_("E394: Didn't find region item for %s"), gname);
vim_free(gname);
return NULL;
}
}

vim_free(gname);
arg = skipwhite(arg);
}
#if defined(FEAT_FOLDING)
else if (flagtab[fidx].flags == HL_FOLD
&& foldmethodIsSyntax(curwin))

foldUpdateAll(curwin);
#endif
}
}

return arg;
}






static void
syn_incl_toplevel(int id, int *flagsp)
{
if ((*flagsp & HL_CONTAINED) || curwin->w_s->b_syn_topgrp == 0)
return;
*flagsp |= HL_CONTAINED;
if (curwin->w_s->b_syn_topgrp >= SYNID_CLUSTER)
{

short *grp_list = ALLOC_MULT(short, 2);
int tlg_id = curwin->w_s->b_syn_topgrp - SYNID_CLUSTER;

if (grp_list != NULL)
{
grp_list[0] = id;
grp_list[1] = 0;
syn_combine_list(&SYN_CLSTR(curwin->w_s)[tlg_id].scl_list,
&grp_list, CLUSTER_ADD);
}
}
}




static void
syn_cmd_include(exarg_T *eap, int syncing UNUSED)
{
char_u *arg = eap->arg;
int sgl_id = 1;
char_u *group_name_end;
char_u *rest;
char *errormsg = NULL;
int prev_toplvl_grp;
int prev_syn_inc_tag;
int source = FALSE;

eap->nextcmd = find_nextcmd(arg);
if (eap->skip)
return;

if (arg[0] == '@')
{
++arg;
rest = get_group_name(arg, &group_name_end);
if (rest == NULL)
{
emsg(_("E397: Filename required"));
return;
}
sgl_id = syn_check_cluster(arg, (int)(group_name_end - arg));
if (sgl_id == 0)
return;

eap->arg = rest;
}





eap->argt |= (EX_XFILE | EX_NOSPC);
separate_nextcmd(eap);
if (*eap->arg == '<' || *eap->arg == '$' || mch_isFullName(eap->arg))
{



source = TRUE;
if (expand_filename(eap, syn_cmdlinep, &errormsg) == FAIL)
{
if (errormsg != NULL)
emsg(errormsg);
return;
}
}





if (running_syn_inc_tag >= MAX_SYN_INC_TAG)
{
emsg(_("E847: Too many syntax includes"));
return;
}
prev_syn_inc_tag = current_syn_inc_tag;
current_syn_inc_tag = ++running_syn_inc_tag;
prev_toplvl_grp = curwin->w_s->b_syn_topgrp;
curwin->w_s->b_syn_topgrp = sgl_id;
if (source ? do_source(eap->arg, FALSE, DOSO_NONE, NULL) == FAIL
: source_runtime(eap->arg, DIP_ALL) == FAIL)
semsg(_(e_notopen), eap->arg);
curwin->w_s->b_syn_topgrp = prev_toplvl_grp;
current_syn_inc_tag = prev_syn_inc_tag;
}




static void
syn_cmd_keyword(exarg_T *eap, int syncing UNUSED)
{
char_u *arg = eap->arg;
char_u *group_name_end;
int syn_id;
char_u *rest;
char_u *keyword_copy = NULL;
char_u *p;
char_u *kw;
syn_opt_arg_T syn_opt_arg;
int cnt;
int conceal_char = NUL;

rest = get_group_name(arg, &group_name_end);

if (rest != NULL)
{
if (eap->skip)
syn_id = -1;
else
syn_id = syn_check_group(arg, (int)(group_name_end - arg));
if (syn_id != 0)

keyword_copy = alloc(STRLEN(rest) + 1);
if (keyword_copy != NULL)
{
syn_opt_arg.flags = 0;
syn_opt_arg.keyword = TRUE;
syn_opt_arg.sync_idx = NULL;
syn_opt_arg.has_cont_list = FALSE;
syn_opt_arg.cont_in_list = NULL;
syn_opt_arg.next_list = NULL;






cnt = 0;
p = keyword_copy;
for ( ; rest != NULL && !ends_excmd(*rest); rest = skipwhite(rest))
{
rest = get_syn_options(rest, &syn_opt_arg, &conceal_char,
eap->skip);
if (rest == NULL || ends_excmd(*rest))
break;

while (*rest != NUL && !VIM_ISWHITE(*rest))
{
if (*rest == '\\' && rest[1] != NUL)
++rest;
*p++ = *rest++;
}
*p++ = NUL;
++cnt;
}

if (!eap->skip)
{

syn_incl_toplevel(syn_id, &syn_opt_arg.flags);




for (kw = keyword_copy; --cnt >= 0; kw += STRLEN(kw) + 1)
{
for (p = vim_strchr(kw, '['); ; )
{
if (p != NULL)
*p = NUL;
add_keyword(kw, syn_id, syn_opt_arg.flags,
syn_opt_arg.cont_in_list,
syn_opt_arg.next_list, conceal_char);
if (p == NULL)
break;
if (p[1] == NUL)
{
semsg(_("E789: Missing ']': %s"), kw);
goto error;
}
if (p[1] == ']')
{
if (p[2] != NUL)
{
semsg(_("E890: trailing char after ']': %s]%s"),
kw, &p[2]);
goto error;
}
kw = p + 1; 
break;
}
if (has_mbyte)
{
int l = (*mb_ptr2len)(p + 1);

mch_memmove(p, p + 1, l);
p += l;
}
else
{
p[0] = p[1];
++p;
}
}
}
}
error:
vim_free(keyword_copy);
vim_free(syn_opt_arg.cont_in_list);
vim_free(syn_opt_arg.next_list);
}
}

if (rest != NULL)
eap->nextcmd = check_nextcmd(rest);
else
semsg(_(e_invarg2), arg);

redraw_curbuf_later(SOME_VALID);
syn_stack_free_all(curwin->w_s); 
}






static void
syn_cmd_match(
exarg_T *eap,
int syncing) 
{
char_u *arg = eap->arg;
char_u *group_name_end;
char_u *rest;
synpat_T item; 
int syn_id;
int idx;
syn_opt_arg_T syn_opt_arg;
int sync_idx = 0;
int conceal_char = NUL;


rest = get_group_name(arg, &group_name_end);


syn_opt_arg.flags = 0;
syn_opt_arg.keyword = FALSE;
syn_opt_arg.sync_idx = syncing ? &sync_idx : NULL;
syn_opt_arg.has_cont_list = TRUE;
syn_opt_arg.cont_list = NULL;
syn_opt_arg.cont_in_list = NULL;
syn_opt_arg.next_list = NULL;
rest = get_syn_options(rest, &syn_opt_arg, &conceal_char, eap->skip);


init_syn_patterns();
vim_memset(&item, 0, sizeof(item));
rest = get_syn_pattern(rest, &item);
if (vim_regcomp_had_eol() && !(syn_opt_arg.flags & HL_EXCLUDENL))
syn_opt_arg.flags |= HL_HAS_EOL;


rest = get_syn_options(rest, &syn_opt_arg, &conceal_char, eap->skip);

if (rest != NULL) 
{



eap->nextcmd = check_nextcmd(rest);
if (!ends_excmd(*rest) || eap->skip)
rest = NULL;
else if (ga_grow(&curwin->w_s->b_syn_patterns, 1) != FAIL
&& (syn_id = syn_check_group(arg,
(int)(group_name_end - arg))) != 0)
{
syn_incl_toplevel(syn_id, &syn_opt_arg.flags);



idx = curwin->w_s->b_syn_patterns.ga_len;
SYN_ITEMS(curwin->w_s)[idx] = item;
SYN_ITEMS(curwin->w_s)[idx].sp_syncing = syncing;
SYN_ITEMS(curwin->w_s)[idx].sp_type = SPTYPE_MATCH;
SYN_ITEMS(curwin->w_s)[idx].sp_syn.id = syn_id;
SYN_ITEMS(curwin->w_s)[idx].sp_syn.inc_tag = current_syn_inc_tag;
SYN_ITEMS(curwin->w_s)[idx].sp_flags = syn_opt_arg.flags;
SYN_ITEMS(curwin->w_s)[idx].sp_sync_idx = sync_idx;
SYN_ITEMS(curwin->w_s)[idx].sp_cont_list = syn_opt_arg.cont_list;
SYN_ITEMS(curwin->w_s)[idx].sp_syn.cont_in_list =
syn_opt_arg.cont_in_list;
#if defined(FEAT_CONCEAL)
SYN_ITEMS(curwin->w_s)[idx].sp_cchar = conceal_char;
#endif
if (syn_opt_arg.cont_in_list != NULL)
curwin->w_s->b_syn_containedin = TRUE;
SYN_ITEMS(curwin->w_s)[idx].sp_next_list = syn_opt_arg.next_list;
++curwin->w_s->b_syn_patterns.ga_len;


if (syn_opt_arg.flags & (HL_SYNC_HERE|HL_SYNC_THERE))
curwin->w_s->b_syn_sync_flags |= SF_MATCH;
#if defined(FEAT_FOLDING)
if (syn_opt_arg.flags & HL_FOLD)
++curwin->w_s->b_syn_folditems;
#endif

redraw_curbuf_later(SOME_VALID);
syn_stack_free_all(curwin->w_s); 
return; 
}
}




vim_regfree(item.sp_prog);
vim_free(item.sp_pattern);
vim_free(syn_opt_arg.cont_list);
vim_free(syn_opt_arg.cont_in_list);
vim_free(syn_opt_arg.next_list);

if (rest == NULL)
semsg(_(e_invarg2), arg);
}





static void
syn_cmd_region(
exarg_T *eap,
int syncing) 
{
char_u *arg = eap->arg;
char_u *group_name_end;
char_u *rest; 
char_u *key_end;
char_u *key = NULL;
char_u *p;
int item;
#define ITEM_START 0
#define ITEM_SKIP 1
#define ITEM_END 2
#define ITEM_MATCHGROUP 3
struct pat_ptr
{
synpat_T *pp_synp; 
int pp_matchgroup_id; 
struct pat_ptr *pp_next; 
} *(pat_ptrs[3]);

struct pat_ptr *ppp;
struct pat_ptr *ppp_next;
int pat_count = 0; 
int syn_id;
int matchgroup_id = 0;
int not_enough = FALSE; 
int illegal = FALSE; 
int success = FALSE;
int idx;
syn_opt_arg_T syn_opt_arg;
int conceal_char = NUL;


rest = get_group_name(arg, &group_name_end);

pat_ptrs[0] = NULL;
pat_ptrs[1] = NULL;
pat_ptrs[2] = NULL;

init_syn_patterns();

syn_opt_arg.flags = 0;
syn_opt_arg.keyword = FALSE;
syn_opt_arg.sync_idx = NULL;
syn_opt_arg.has_cont_list = TRUE;
syn_opt_arg.cont_list = NULL;
syn_opt_arg.cont_in_list = NULL;
syn_opt_arg.next_list = NULL;




while (rest != NULL && !ends_excmd(*rest))
{

rest = get_syn_options(rest, &syn_opt_arg, &conceal_char, eap->skip);
if (rest == NULL || ends_excmd(*rest))
break;


key_end = rest;
while (*key_end && !VIM_ISWHITE(*key_end) && *key_end != '=')
++key_end;
vim_free(key);
key = vim_strnsave_up(rest, (int)(key_end - rest));
if (key == NULL) 
{
rest = NULL;
break;
}
if (STRCMP(key, "MATCHGROUP") == 0)
item = ITEM_MATCHGROUP;
else if (STRCMP(key, "START") == 0)
item = ITEM_START;
else if (STRCMP(key, "END") == 0)
item = ITEM_END;
else if (STRCMP(key, "SKIP") == 0)
{
if (pat_ptrs[ITEM_SKIP] != NULL) 
{
illegal = TRUE;
break;
}
item = ITEM_SKIP;
}
else
break;
rest = skipwhite(key_end);
if (*rest != '=')
{
rest = NULL;
semsg(_("E398: Missing '=': %s"), arg);
break;
}
rest = skipwhite(rest + 1);
if (*rest == NUL)
{
not_enough = TRUE;
break;
}

if (item == ITEM_MATCHGROUP)
{
p = skiptowhite(rest);
if ((p - rest == 4 && STRNCMP(rest, "NONE", 4) == 0) || eap->skip)
matchgroup_id = 0;
else
{
matchgroup_id = syn_check_group(rest, (int)(p - rest));
if (matchgroup_id == 0)
{
illegal = TRUE;
break;
}
}
rest = skipwhite(p);
}
else
{





ppp = ALLOC_ONE(struct pat_ptr);
if (ppp == NULL)
{
rest = NULL;
break;
}
ppp->pp_next = pat_ptrs[item];
pat_ptrs[item] = ppp;
ppp->pp_synp = ALLOC_CLEAR_ONE(synpat_T);
if (ppp->pp_synp == NULL)
{
rest = NULL;
break;
}





if (item == ITEM_START)
reg_do_extmatch = REX_SET;
else if (item == ITEM_SKIP || item == ITEM_END)
reg_do_extmatch = REX_USE;
rest = get_syn_pattern(rest, ppp->pp_synp);
reg_do_extmatch = 0;
if (item == ITEM_END && vim_regcomp_had_eol()
&& !(syn_opt_arg.flags & HL_EXCLUDENL))
ppp->pp_synp->sp_flags |= HL_HAS_EOL;
ppp->pp_matchgroup_id = matchgroup_id;
++pat_count;
}
}
vim_free(key);
if (illegal || not_enough)
rest = NULL;




if (rest != NULL && (pat_ptrs[ITEM_START] == NULL ||
pat_ptrs[ITEM_END] == NULL))
{
not_enough = TRUE;
rest = NULL;
}

if (rest != NULL)
{




eap->nextcmd = check_nextcmd(rest);
if (!ends_excmd(*rest) || eap->skip)
rest = NULL;
else if (ga_grow(&(curwin->w_s->b_syn_patterns), pat_count) != FAIL
&& (syn_id = syn_check_group(arg,
(int)(group_name_end - arg))) != 0)
{
syn_incl_toplevel(syn_id, &syn_opt_arg.flags);



idx = curwin->w_s->b_syn_patterns.ga_len;
for (item = ITEM_START; item <= ITEM_END; ++item)
{
for (ppp = pat_ptrs[item]; ppp != NULL; ppp = ppp->pp_next)
{
SYN_ITEMS(curwin->w_s)[idx] = *(ppp->pp_synp);
SYN_ITEMS(curwin->w_s)[idx].sp_syncing = syncing;
SYN_ITEMS(curwin->w_s)[idx].sp_type =
(item == ITEM_START) ? SPTYPE_START :
(item == ITEM_SKIP) ? SPTYPE_SKIP : SPTYPE_END;
SYN_ITEMS(curwin->w_s)[idx].sp_flags |= syn_opt_arg.flags;
SYN_ITEMS(curwin->w_s)[idx].sp_syn.id = syn_id;
SYN_ITEMS(curwin->w_s)[idx].sp_syn.inc_tag =
current_syn_inc_tag;
SYN_ITEMS(curwin->w_s)[idx].sp_syn_match_id =
ppp->pp_matchgroup_id;
#if defined(FEAT_CONCEAL)
SYN_ITEMS(curwin->w_s)[idx].sp_cchar = conceal_char;
#endif
if (item == ITEM_START)
{
SYN_ITEMS(curwin->w_s)[idx].sp_cont_list =
syn_opt_arg.cont_list;
SYN_ITEMS(curwin->w_s)[idx].sp_syn.cont_in_list =
syn_opt_arg.cont_in_list;
if (syn_opt_arg.cont_in_list != NULL)
curwin->w_s->b_syn_containedin = TRUE;
SYN_ITEMS(curwin->w_s)[idx].sp_next_list =
syn_opt_arg.next_list;
}
++curwin->w_s->b_syn_patterns.ga_len;
++idx;
#if defined(FEAT_FOLDING)
if (syn_opt_arg.flags & HL_FOLD)
++curwin->w_s->b_syn_folditems;
#endif
}
}

redraw_curbuf_later(SOME_VALID);
syn_stack_free_all(curwin->w_s); 
success = TRUE; 
}
}




for (item = ITEM_START; item <= ITEM_END; ++item)
for (ppp = pat_ptrs[item]; ppp != NULL; ppp = ppp_next)
{
if (!success && ppp->pp_synp != NULL)
{
vim_regfree(ppp->pp_synp->sp_prog);
vim_free(ppp->pp_synp->sp_pattern);
}
vim_free(ppp->pp_synp);
ppp_next = ppp->pp_next;
vim_free(ppp);
}

if (!success)
{
vim_free(syn_opt_arg.cont_list);
vim_free(syn_opt_arg.cont_in_list);
vim_free(syn_opt_arg.next_list);
if (not_enough)
semsg(_("E399: Not enough arguments: syntax region %s"), arg);
else if (illegal || rest == NULL)
semsg(_(e_invarg2), arg);
}
}




static int
syn_compare_stub(const void *v1, const void *v2)
{
const short *s1 = v1;
const short *s2 = v2;

return (*s1 > *s2 ? 1 : *s1 < *s2 ? -1 : 0);
}





static void
syn_combine_list(short **clstr1, short **clstr2, int list_op)
{
int count1 = 0;
int count2 = 0;
short *g1;
short *g2;
short *clstr = NULL;
int count;
int round;




if (*clstr2 == NULL)
return;
if (*clstr1 == NULL || list_op == CLUSTER_REPLACE)
{
if (list_op == CLUSTER_REPLACE)
vim_free(*clstr1);
if (list_op == CLUSTER_REPLACE || list_op == CLUSTER_ADD)
*clstr1 = *clstr2;
else
vim_free(*clstr2);
return;
}

for (g1 = *clstr1; *g1; g1++)
++count1;
for (g2 = *clstr2; *g2; g2++)
++count2;




qsort(*clstr1, (size_t)count1, sizeof(short), syn_compare_stub);
qsort(*clstr2, (size_t)count2, sizeof(short), syn_compare_stub);







for (round = 1; round <= 2; round++)
{
g1 = *clstr1;
g2 = *clstr2;
count = 0;




while (*g1 && *g2)
{



if (*g1 < *g2)
{
if (round == 2)
clstr[count] = *g1;
count++;
g1++;
continue;
}




if (list_op == CLUSTER_ADD)
{
if (round == 2)
clstr[count] = *g2;
count++;
}
if (*g1 == *g2)
g1++;
g2++;
}






for (; *g1; g1++, count++)
if (round == 2)
clstr[count] = *g1;
if (list_op == CLUSTER_ADD)
for (; *g2; g2++, count++)
if (round == 2)
clstr[count] = *g2;

if (round == 1)
{




if (count == 0)
{
clstr = NULL;
break;
}
clstr = ALLOC_MULT(short, count + 1);
if (clstr == NULL)
break;
clstr[count] = 0;
}
}




vim_free(*clstr1);
vim_free(*clstr2);
*clstr1 = clstr;
}





static int
syn_scl_name2id(char_u *name)
{
int i;
char_u *name_u;


name_u = vim_strsave_up(name);
if (name_u == NULL)
return 0;
for (i = curwin->w_s->b_syn_clusters.ga_len; --i >= 0; )
if (SYN_CLSTR(curwin->w_s)[i].scl_name_u != NULL
&& STRCMP(name_u, SYN_CLSTR(curwin->w_s)[i].scl_name_u) == 0)
break;
vim_free(name_u);
return (i < 0 ? 0 : i + SYNID_CLUSTER);
}




static int
syn_scl_namen2id(char_u *linep, int len)
{
char_u *name;
int id = 0;

name = vim_strnsave(linep, len);
if (name != NULL)
{
id = syn_scl_name2id(name);
vim_free(name);
}
return id;
}







static int
syn_check_cluster(char_u *pp, int len)
{
int id;
char_u *name;

name = vim_strnsave(pp, len);
if (name == NULL)
return 0;

id = syn_scl_name2id(name);
if (id == 0) 
id = syn_add_cluster(name);
else
vim_free(name);
return id;
}






static int
syn_add_cluster(char_u *name)
{
int len;




if (curwin->w_s->b_syn_clusters.ga_data == NULL)
{
curwin->w_s->b_syn_clusters.ga_itemsize = sizeof(syn_cluster_T);
curwin->w_s->b_syn_clusters.ga_growsize = 10;
}

len = curwin->w_s->b_syn_clusters.ga_len;
if (len >= MAX_CLUSTER_ID)
{
emsg(_("E848: Too many syntax clusters"));
vim_free(name);
return 0;
}




if (ga_grow(&curwin->w_s->b_syn_clusters, 1) == FAIL)
{
vim_free(name);
return 0;
}

vim_memset(&(SYN_CLSTR(curwin->w_s)[len]), 0, sizeof(syn_cluster_T));
SYN_CLSTR(curwin->w_s)[len].scl_name = name;
SYN_CLSTR(curwin->w_s)[len].scl_name_u = vim_strsave_up(name);
SYN_CLSTR(curwin->w_s)[len].scl_list = NULL;
++curwin->w_s->b_syn_clusters.ga_len;

if (STRICMP(name, "Spell") == 0)
curwin->w_s->b_spell_cluster_id = len + SYNID_CLUSTER;
if (STRICMP(name, "NoSpell") == 0)
curwin->w_s->b_nospell_cluster_id = len + SYNID_CLUSTER;

return len + SYNID_CLUSTER;
}





static void
syn_cmd_cluster(exarg_T *eap, int syncing UNUSED)
{
char_u *arg = eap->arg;
char_u *group_name_end;
char_u *rest;
int scl_id;
short *clstr_list;
int got_clstr = FALSE;
int opt_len;
int list_op;

eap->nextcmd = find_nextcmd(arg);
if (eap->skip)
return;

rest = get_group_name(arg, &group_name_end);

if (rest != NULL)
{
scl_id = syn_check_cluster(arg, (int)(group_name_end - arg));
if (scl_id == 0)
return;
scl_id -= SYNID_CLUSTER;

for (;;)
{
if (STRNICMP(rest, "add", 3) == 0
&& (VIM_ISWHITE(rest[3]) || rest[3] == '='))
{
opt_len = 3;
list_op = CLUSTER_ADD;
}
else if (STRNICMP(rest, "remove", 6) == 0
&& (VIM_ISWHITE(rest[6]) || rest[6] == '='))
{
opt_len = 6;
list_op = CLUSTER_SUBTRACT;
}
else if (STRNICMP(rest, "contains", 8) == 0
&& (VIM_ISWHITE(rest[8]) || rest[8] == '='))
{
opt_len = 8;
list_op = CLUSTER_REPLACE;
}
else
break;

clstr_list = NULL;
if (get_id_list(&rest, opt_len, &clstr_list, eap->skip) == FAIL)
{
semsg(_(e_invarg2), rest);
break;
}
if (scl_id >= 0)
syn_combine_list(&SYN_CLSTR(curwin->w_s)[scl_id].scl_list,
&clstr_list, list_op);
else
vim_free(clstr_list);
got_clstr = TRUE;
}

if (got_clstr)
{
redraw_curbuf_later(SOME_VALID);
syn_stack_free_all(curwin->w_s); 
}
}

if (!got_clstr)
emsg(_("E400: No cluster specified"));
if (rest == NULL || !ends_excmd(*rest))
semsg(_(e_invarg2), arg);
}




static void
init_syn_patterns(void)
{
curwin->w_s->b_syn_patterns.ga_itemsize = sizeof(synpat_T);
curwin->w_s->b_syn_patterns.ga_growsize = 10;
}






static char_u *
get_syn_pattern(char_u *arg, synpat_T *ci)
{
char_u *end;
int *p;
int idx;
char_u *cpo_save;


if (arg == NULL || arg[0] == NUL || arg[1] == NUL || arg[2] == NUL)
return NULL;

end = skip_regexp(arg + 1, *arg, TRUE);
if (*end != *arg) 
{
semsg(_("E401: Pattern delimiter not found: %s"), arg);
return NULL;
}

if ((ci->sp_pattern = vim_strnsave(arg + 1, (int)(end - arg - 1))) == NULL)
return NULL;


cpo_save = p_cpo;
p_cpo = (char_u *)"";
ci->sp_prog = vim_regcomp(ci->sp_pattern, RE_MAGIC);
p_cpo = cpo_save;

if (ci->sp_prog == NULL)
return NULL;
ci->sp_ic = curwin->w_s->b_syn_ic;
#if defined(FEAT_PROFILE)
syn_clear_time(&ci->sp_time);
#endif




++end;
do
{
for (idx = SPO_COUNT; --idx >= 0; )
if (STRNCMP(end, spo_name_tab[idx], 3) == 0)
break;
if (idx >= 0)
{
p = &(ci->sp_offsets[idx]);
if (idx != SPO_LC_OFF)
switch (end[3])
{
case 's': break;
case 'b': break;
case 'e': idx += SPO_COUNT; break;
default: idx = -1; break;
}
if (idx >= 0)
{
ci->sp_off_flags |= (1 << idx);
if (idx == SPO_LC_OFF) 
{
end += 3;
*p = getdigits(&end);


if (!(ci->sp_off_flags & (1 << SPO_MS_OFF)))
{
ci->sp_off_flags |= (1 << SPO_MS_OFF);
ci->sp_offsets[SPO_MS_OFF] = *p;
}
}
else 
{
end += 4;
if (*end == '+')
{
++end;
*p = getdigits(&end); 
}
else if (*end == '-')
{
++end;
*p = -getdigits(&end); 
}
}
if (*end != ',')
break;
++end;
}
}
} while (idx >= 0);

if (!ends_excmd(*end) && !VIM_ISWHITE(*end))
{
semsg(_("E402: Garbage after pattern: %s"), arg);
return NULL;
}
return skipwhite(end);
}




static void
syn_cmd_sync(exarg_T *eap, int syncing UNUSED)
{
char_u *arg_start = eap->arg;
char_u *arg_end;
char_u *key = NULL;
char_u *next_arg;
int illegal = FALSE;
int finished = FALSE;
long n;
char_u *cpo_save;

if (ends_excmd(*arg_start))
{
syn_cmd_list(eap, TRUE);
return;
}

while (!ends_excmd(*arg_start))
{
arg_end = skiptowhite(arg_start);
next_arg = skipwhite(arg_end);
vim_free(key);
key = vim_strnsave_up(arg_start, (int)(arg_end - arg_start));
if (STRCMP(key, "CCOMMENT") == 0)
{
if (!eap->skip)
curwin->w_s->b_syn_sync_flags |= SF_CCOMMENT;
if (!ends_excmd(*next_arg))
{
arg_end = skiptowhite(next_arg);
if (!eap->skip)
curwin->w_s->b_syn_sync_id = syn_check_group(next_arg,
(int)(arg_end - next_arg));
next_arg = skipwhite(arg_end);
}
else if (!eap->skip)
curwin->w_s->b_syn_sync_id = syn_name2id((char_u *)"Comment");
}
else if ( STRNCMP(key, "LINES", 5) == 0
|| STRNCMP(key, "MINLINES", 8) == 0
|| STRNCMP(key, "MAXLINES", 8) == 0
|| STRNCMP(key, "LINEBREAKS", 10) == 0)
{
if (key[4] == 'S')
arg_end = key + 6;
else if (key[0] == 'L')
arg_end = key + 11;
else
arg_end = key + 9;
if (arg_end[-1] != '=' || !VIM_ISDIGIT(*arg_end))
{
illegal = TRUE;
break;
}
n = getdigits(&arg_end);
if (!eap->skip)
{
if (key[4] == 'B')
curwin->w_s->b_syn_sync_linebreaks = n;
else if (key[1] == 'A')
curwin->w_s->b_syn_sync_maxlines = n;
else
curwin->w_s->b_syn_sync_minlines = n;
}
}
else if (STRCMP(key, "FROMSTART") == 0)
{
if (!eap->skip)
{
curwin->w_s->b_syn_sync_minlines = MAXLNUM;
curwin->w_s->b_syn_sync_maxlines = 0;
}
}
else if (STRCMP(key, "LINECONT") == 0)
{
if (*next_arg == NUL) 
{
illegal = TRUE;
break;
}
if (curwin->w_s->b_syn_linecont_pat != NULL)
{
emsg(_("E403: syntax sync: line continuations pattern specified twice"));
finished = TRUE;
break;
}
arg_end = skip_regexp(next_arg + 1, *next_arg, TRUE);
if (*arg_end != *next_arg) 
{
illegal = TRUE;
break;
}

if (!eap->skip)
{

if ((curwin->w_s->b_syn_linecont_pat = vim_strnsave(next_arg + 1,
(int)(arg_end - next_arg - 1))) == NULL)
{
finished = TRUE;
break;
}
curwin->w_s->b_syn_linecont_ic = curwin->w_s->b_syn_ic;


cpo_save = p_cpo;
p_cpo = (char_u *)"";
curwin->w_s->b_syn_linecont_prog =
vim_regcomp(curwin->w_s->b_syn_linecont_pat, RE_MAGIC);
p_cpo = cpo_save;
#if defined(FEAT_PROFILE)
syn_clear_time(&curwin->w_s->b_syn_linecont_time);
#endif

if (curwin->w_s->b_syn_linecont_prog == NULL)
{
VIM_CLEAR(curwin->w_s->b_syn_linecont_pat);
finished = TRUE;
break;
}
}
next_arg = skipwhite(arg_end + 1);
}
else
{
eap->arg = next_arg;
if (STRCMP(key, "MATCH") == 0)
syn_cmd_match(eap, TRUE);
else if (STRCMP(key, "REGION") == 0)
syn_cmd_region(eap, TRUE);
else if (STRCMP(key, "CLEAR") == 0)
syn_cmd_clear(eap, TRUE);
else
illegal = TRUE;
finished = TRUE;
break;
}
arg_start = next_arg;
}
vim_free(key);
if (illegal)
semsg(_("E404: Illegal arguments: %s"), arg_start);
else if (!finished)
{
eap->nextcmd = check_nextcmd(arg_start);
redraw_curbuf_later(SOME_VALID);
syn_stack_free_all(curwin->w_s); 
}
}








static int
get_id_list(
char_u **arg,
int keylen, 
short **list, 

int skip)
{
char_u *p = NULL;
char_u *end;
int round;
int count;
int total_count = 0;
short *retval = NULL;
char_u *name;
regmatch_T regmatch;
int id;
int i;
int failed = FALSE;








for (round = 1; round <= 2; ++round)
{



p = skipwhite(*arg + keylen);
if (*p != '=')
{
semsg(_("E405: Missing equal sign: %s"), *arg);
break;
}
p = skipwhite(p + 1);
if (ends_excmd(*p))
{
semsg(_("E406: Empty argument: %s"), *arg);
break;
}




count = 0;
while (!ends_excmd(*p))
{
for (end = p; *end && !VIM_ISWHITE(*end) && *end != ','; ++end)
;
name = alloc(end - p + 3); 
if (name == NULL)
{
failed = TRUE;
break;
}
vim_strncpy(name + 1, p, end - p);
if ( STRCMP(name + 1, "ALLBUT") == 0
|| STRCMP(name + 1, "ALL") == 0
|| STRCMP(name + 1, "TOP") == 0
|| STRCMP(name + 1, "CONTAINED") == 0)
{
if (TOUPPER_ASC(**arg) != 'C')
{
semsg(_("E407: %s not allowed here"), name + 1);
failed = TRUE;
vim_free(name);
break;
}
if (count != 0)
{
semsg(_("E408: %s must be first in contains list"),
name + 1);
failed = TRUE;
vim_free(name);
break;
}
if (name[1] == 'A')
id = SYNID_ALLBUT;
else if (name[1] == 'T')
id = SYNID_TOP;
else
id = SYNID_CONTAINED;
id += current_syn_inc_tag;
}
else if (name[1] == '@')
{
if (skip)
id = -1;
else
id = syn_check_cluster(name + 2, (int)(end - p - 1));
}
else
{



if (vim_strpbrk(name + 1, (char_u *)"\\.*^$~[") == NULL)
id = syn_check_group(name + 1, (int)(end - p));
else
{



*name = '^';
STRCAT(name, "$");
regmatch.regprog = vim_regcomp(name, RE_MAGIC);
if (regmatch.regprog == NULL)
{
failed = TRUE;
vim_free(name);
break;
}

regmatch.rm_ic = TRUE;
id = 0;
for (i = highlight_num_groups(); --i >= 0; )
{
if (vim_regexec(&regmatch, highlight_group_name(i),
(colnr_T)0))
{
if (round == 2)
{




if (count >= total_count)
{
vim_free(retval);
round = 1;
}
else
retval[count] = i + 1;
}
++count;
id = -1; 
}
}
vim_regfree(regmatch.regprog);
}
}
vim_free(name);
if (id == 0)
{
semsg(_("E409: Unknown group name: %s"), p);
failed = TRUE;
break;
}
if (id > 0)
{
if (round == 2)
{

if (count >= total_count)
{
vim_free(retval);
round = 1;
}
else
retval[count] = id;
}
++count;
}
p = skipwhite(end);
if (*p != ',')
break;
p = skipwhite(p + 1); 
}
if (failed)
break;
if (round == 1)
{
retval = ALLOC_MULT(short, count + 1);
if (retval == NULL)
break;
retval[count] = 0; 
total_count = count;
}
}

*arg = p;
if (failed || retval == NULL)
{
vim_free(retval);
return FAIL;
}

if (*list == NULL)
*list = retval;
else
vim_free(retval); 

return OK;
}




static short *
copy_id_list(short *list)
{
int len;
int count;
short *retval;

if (list == NULL)
return NULL;

for (count = 0; list[count]; ++count)
;
len = (count + 1) * sizeof(short);
retval = alloc(len);
if (retval != NULL)
mch_memmove(retval, list, (size_t)len);

return retval;
}








static int
in_id_list(
stateitem_T *cur_si, 
short *list, 
struct sp_syn *ssp, 
int contained) 
{
int retval;
short *scl_list;
short item;
short id = ssp->id;
static int depth = 0;
int r;


if (cur_si != NULL && ssp->cont_in_list != NULL
&& !(cur_si->si_flags & HL_MATCH))
{


while ((cur_si->si_flags & HL_TRANS_CONT)
&& cur_si > (stateitem_T *)(current_state.ga_data))
--cur_si;

if (cur_si->si_idx >= 0 && in_id_list(NULL, ssp->cont_in_list,
&(SYN_ITEMS(syn_block)[cur_si->si_idx].sp_syn),
SYN_ITEMS(syn_block)[cur_si->si_idx].sp_flags & HL_CONTAINED))
return TRUE;
}

if (list == NULL)
return FALSE;





if (list == ID_LIST_ALL)
return !contained;






item = *list;
if (item >= SYNID_ALLBUT && item < SYNID_CLUSTER)
{
if (item < SYNID_TOP)
{

if (item - SYNID_ALLBUT != ssp->inc_tag)
return FALSE;
}
else if (item < SYNID_CONTAINED)
{

if (item - SYNID_TOP != ssp->inc_tag || contained)
return FALSE;
}
else
{

if (item - SYNID_CONTAINED != ssp->inc_tag || !contained)
return FALSE;
}
item = *++list;
retval = FALSE;
}
else
retval = TRUE;




while (item != 0)
{
if (item == id)
return retval;
if (item >= SYNID_CLUSTER)
{
scl_list = SYN_CLSTR(syn_block)[item - SYNID_CLUSTER].scl_list;


if (scl_list != NULL && depth < 30)
{
++depth;
r = in_id_list(NULL, scl_list, ssp, contained);
--depth;
if (r)
return retval;
}
}
item = *++list;
}
return !retval;
}

struct subcommand
{
char *name; 
void (*func)(exarg_T *, int); 
};

static struct subcommand subcommands[] =
{
{"case", syn_cmd_case},
{"clear", syn_cmd_clear},
{"cluster", syn_cmd_cluster},
{"conceal", syn_cmd_conceal},
{"enable", syn_cmd_enable},
{"include", syn_cmd_include},
{"iskeyword", syn_cmd_iskeyword},
{"keyword", syn_cmd_keyword},
{"list", syn_cmd_list},
{"manual", syn_cmd_manual},
{"match", syn_cmd_match},
{"on", syn_cmd_on},
{"off", syn_cmd_off},
{"region", syn_cmd_region},
{"reset", syn_cmd_reset},
{"spell", syn_cmd_spell},
{"sync", syn_cmd_sync},
{"", syn_cmd_list},
{NULL, NULL}
};






void
ex_syntax(exarg_T *eap)
{
char_u *arg = eap->arg;
char_u *subcmd_end;
char_u *subcmd_name;
int i;

syn_cmdlinep = eap->cmdlinep;


for (subcmd_end = arg; ASCII_ISALPHA(*subcmd_end); ++subcmd_end)
;
subcmd_name = vim_strnsave(arg, (int)(subcmd_end - arg));
if (subcmd_name != NULL)
{
if (eap->skip) 
++emsg_skip;
for (i = 0; ; ++i)
{
if (subcommands[i].name == NULL)
{
semsg(_("E410: Invalid :syntax subcommand: %s"), subcmd_name);
break;
}
if (STRCMP(subcmd_name, (char_u *)subcommands[i].name) == 0)
{
eap->arg = skipwhite(subcmd_end);
(subcommands[i].func)(eap, FALSE);
break;
}
}
vim_free(subcmd_name);
if (eap->skip)
--emsg_skip;
}
}

void
ex_ownsyntax(exarg_T *eap)
{
char_u *old_value;
char_u *new_value;

if (curwin->w_s == &curwin->w_buffer->b_s)
{
curwin->w_s = ALLOC_ONE(synblock_T);
memset(curwin->w_s, 0, sizeof(synblock_T));
hash_init(&curwin->w_s->b_keywtab);
hash_init(&curwin->w_s->b_keywtab_ic);
#if defined(FEAT_SPELL)

curwin->w_p_spell = FALSE; 
clear_string_option(&curwin->w_s->b_p_spc);
clear_string_option(&curwin->w_s->b_p_spf);
clear_string_option(&curwin->w_s->b_p_spl);
#endif
clear_string_option(&curwin->w_s->b_syn_isk);
}


old_value = get_var_value((char_u *)"b:current_syntax");
if (old_value != NULL)
old_value = vim_strsave(old_value);



apply_autocmds(EVENT_SYNTAX, eap->arg, curbuf->b_fname, TRUE, curbuf);


new_value = get_var_value((char_u *)"b:current_syntax");
if (new_value != NULL)
set_internal_string_var((char_u *)"w:current_syntax", new_value);


if (old_value == NULL)
do_unlet((char_u *)"b:current_syntax", TRUE);
else
{
set_internal_string_var((char_u *)"b:current_syntax", old_value);
vim_free(old_value);
}
}

int
syntax_present(win_T *win)
{
return (win->w_s->b_syn_patterns.ga_len != 0
|| win->w_s->b_syn_clusters.ga_len != 0
|| win->w_s->b_keywtab.ht_used > 0
|| win->w_s->b_keywtab_ic.ht_used > 0);
}


static enum
{
EXP_SUBCMD, 
EXP_CASE, 
EXP_SPELL, 
EXP_SYNC 
} expand_what;





void
reset_expand_highlight(void)
{
include_link = include_default = include_none = 0;
}





void
set_context_in_echohl_cmd(expand_T *xp, char_u *arg)
{
xp->xp_context = EXPAND_HIGHLIGHT;
xp->xp_pattern = arg;
include_none = 1;
}




void
set_context_in_syntax_cmd(expand_T *xp, char_u *arg)
{
char_u *p;


xp->xp_context = EXPAND_SYNTAX;
expand_what = EXP_SUBCMD;
xp->xp_pattern = arg;
include_link = 0;
include_default = 0;


if (*arg != NUL)
{
p = skiptowhite(arg);
if (*p != NUL) 
{
xp->xp_pattern = skipwhite(p);
if (*skiptowhite(xp->xp_pattern) != NUL)
xp->xp_context = EXPAND_NOTHING;
else if (STRNICMP(arg, "case", p - arg) == 0)
expand_what = EXP_CASE;
else if (STRNICMP(arg, "spell", p - arg) == 0)
expand_what = EXP_SPELL;
else if (STRNICMP(arg, "sync", p - arg) == 0)
expand_what = EXP_SYNC;
else if ( STRNICMP(arg, "keyword", p - arg) == 0
|| STRNICMP(arg, "region", p - arg) == 0
|| STRNICMP(arg, "match", p - arg) == 0
|| STRNICMP(arg, "list", p - arg) == 0)
xp->xp_context = EXPAND_HIGHLIGHT;
else
xp->xp_context = EXPAND_NOTHING;
}
}
}





char_u *
get_syntax_name(expand_T *xp UNUSED, int idx)
{
switch (expand_what)
{
case EXP_SUBCMD:
return (char_u *)subcommands[idx].name;
case EXP_CASE:
{
static char *case_args[] = {"match", "ignore", NULL};
return (char_u *)case_args[idx];
}
case EXP_SPELL:
{
static char *spell_args[] =
{"toplevel", "notoplevel", "default", NULL};
return (char_u *)spell_args[idx];
}
case EXP_SYNC:
{
static char *sync_args[] =
{"ccomment", "clear", "fromstart",
"linebreaks=", "linecont", "lines=", "match",
"maxlines=", "minlines=", "region", NULL};
return (char_u *)sync_args[idx];
}
}
return NULL;
}





int
syn_get_id(
win_T *wp,
long lnum,
colnr_T col,
int trans, 
int *spellp, 
int keep_state) 
{


if (wp->w_buffer != syn_buf
|| lnum != current_lnum
|| col < current_col)
syntax_start(wp, lnum);
else if (wp->w_buffer == syn_buf
&& lnum == current_lnum
&& col > current_col)


next_match_idx = -1;

(void)get_syntax_attr(col, spellp, keep_state);

return (trans ? current_trans_id : current_id);
}

#if defined(FEAT_CONCEAL) || defined(PROTO)






int
get_syntax_info(int *seqnrp)
{
*seqnrp = current_seqnr;
return current_flags;
}




int
syn_get_sub_char(void)
{
return current_sub_char;
}
#endif

#if defined(FEAT_EVAL) || defined(PROTO)





int
syn_get_stack_item(int i)
{
if (i >= current_state.ga_len)
{


invalidate_current_state();
current_col = MAXCOL;
return -1;
}
return CUR_STATE(i).si_id;
}
#endif

#if defined(FEAT_FOLDING) || defined(PROTO)



int
syn_get_foldlevel(win_T *wp, long lnum)
{
int level = 0;
int i;


if (wp->w_s->b_syn_folditems != 0
&& !wp->w_s->b_syn_error
#if defined(SYN_TIME_LIMIT)
&& !wp->w_s->b_syn_slow
#endif
)
{
syntax_start(wp, lnum);

for (i = 0; i < current_state.ga_len; ++i)
if (CUR_STATE(i).si_flags & HL_FOLD)
++level;
}
if (level > wp->w_p_fdn)
{
level = wp->w_p_fdn;
if (level < 0)
level = 0;
}
return level;
}
#endif

#if defined(FEAT_PROFILE) || defined(PROTO)



void
ex_syntime(exarg_T *eap)
{
if (STRCMP(eap->arg, "on") == 0)
syn_time_on = TRUE;
else if (STRCMP(eap->arg, "off") == 0)
syn_time_on = FALSE;
else if (STRCMP(eap->arg, "clear") == 0)
syntime_clear();
else if (STRCMP(eap->arg, "report") == 0)
syntime_report();
else
semsg(_(e_invarg2), eap->arg);
}

static void
syn_clear_time(syn_time_T *st)
{
profile_zero(&st->total);
profile_zero(&st->slowest);
st->count = 0;
st->match = 0;
}




static void
syntime_clear(void)
{
int idx;
synpat_T *spp;

if (!syntax_present(curwin))
{
msg(_(msg_no_items));
return;
}
for (idx = 0; idx < curwin->w_s->b_syn_patterns.ga_len; ++idx)
{
spp = &(SYN_ITEMS(curwin->w_s)[idx]);
syn_clear_time(&spp->sp_time);
}
}





char_u *
get_syntime_arg(expand_T *xp UNUSED, int idx)
{
switch (idx)
{
case 0: return (char_u *)"on";
case 1: return (char_u *)"off";
case 2: return (char_u *)"clear";
case 3: return (char_u *)"report";
}
return NULL;
}

typedef struct
{
proftime_T total;
int count;
int match;
proftime_T slowest;
proftime_T average;
int id;
char_u *pattern;
} time_entry_T;

static int
syn_compare_syntime(const void *v1, const void *v2)
{
const time_entry_T *s1 = v1;
const time_entry_T *s2 = v2;

return profile_cmp(&s1->total, &s2->total);
}




static void
syntime_report(void)
{
int idx;
synpat_T *spp;
#if defined(FEAT_FLOAT)
proftime_T tm;
#endif
int len;
proftime_T total_total;
int total_count = 0;
garray_T ga;
time_entry_T *p;

if (!syntax_present(curwin))
{
msg(_(msg_no_items));
return;
}

ga_init2(&ga, sizeof(time_entry_T), 50);
profile_zero(&total_total);
for (idx = 0; idx < curwin->w_s->b_syn_patterns.ga_len; ++idx)
{
spp = &(SYN_ITEMS(curwin->w_s)[idx]);
if (spp->sp_time.count > 0)
{
(void)ga_grow(&ga, 1);
p = ((time_entry_T *)ga.ga_data) + ga.ga_len;
p->total = spp->sp_time.total;
profile_add(&total_total, &spp->sp_time.total);
p->count = spp->sp_time.count;
p->match = spp->sp_time.match;
total_count += spp->sp_time.count;
p->slowest = spp->sp_time.slowest;
#if defined(FEAT_FLOAT)
profile_divide(&spp->sp_time.total, spp->sp_time.count, &tm);
p->average = tm;
#endif
p->id = spp->sp_syn.id;
p->pattern = spp->sp_pattern;
++ga.ga_len;
}
}



if (ga.ga_len > 1)
qsort(ga.ga_data, (size_t)ga.ga_len, sizeof(time_entry_T),
syn_compare_syntime);

msg_puts_title(_(" TOTAL COUNT MATCH SLOWEST AVERAGE NAME PATTERN"));
msg_puts("\n");
for (idx = 0; idx < ga.ga_len && !got_int; ++idx)
{
p = ((time_entry_T *)ga.ga_data) + idx;

msg_puts(profile_msg(&p->total));
msg_puts(" "); 
msg_advance(13);
msg_outnum(p->count);
msg_puts(" ");
msg_advance(20);
msg_outnum(p->match);
msg_puts(" ");
msg_advance(26);
msg_puts(profile_msg(&p->slowest));
msg_puts(" ");
msg_advance(38);
#if defined(FEAT_FLOAT)
msg_puts(profile_msg(&p->average));
msg_puts(" ");
#endif
msg_advance(50);
msg_outtrans(highlight_group_name(p->id - 1));
msg_puts(" ");

msg_advance(69);
if (Columns < 80)
len = 20; 
else
len = Columns - 70;
if (len > (int)STRLEN(p->pattern))
len = (int)STRLEN(p->pattern);
msg_outtrans_len(p->pattern, len);
msg_puts("\n");
}
ga_clear(&ga);
if (!got_int)
{
msg_puts("\n");
msg_puts(profile_msg(&total_total));
msg_advance(13);
msg_outnum(total_count);
msg_puts("\n");
}
}
#endif

#endif 
