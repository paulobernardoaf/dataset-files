#include "vim.h"
#if defined(FEAT_EVAL)
static void set_vv_searchforward(void);
static int first_submatch(regmmatch_T *rp);
#endif
static int check_linecomment(char_u *line);
static int cls(void);
static int skip_chars(int, int);
#if defined(FEAT_FIND_ID)
static void show_pat_in_path(char_u *, int,
int, int, FILE *, linenr_T *, long);
#endif
static void search_stat(int dirc, pos_T *pos, int show_top_bot_msg, char_u *msgbuf, int recompute);
static spat_T spats[2] =
{
{NULL, TRUE, FALSE, {'/', 0, 0, 0L}}, 
{NULL, TRUE, FALSE, {'/', 0, 0, 0L}} 
};
static int last_idx = 0; 
static char_u lastc[2] = {NUL, NUL}; 
static int lastcdir = FORWARD; 
static int last_t_cmd = TRUE; 
static char_u lastc_bytes[MB_MAXBYTES + 1];
static int lastc_bytelen = 1; 
static spat_T saved_spats[2];
#if defined(FEAT_SEARCH_EXTRA)
static int saved_spats_last_idx = 0;
static int saved_spats_no_hlsearch = 0;
#endif
static char_u *mr_pattern = NULL; 
#if defined(FEAT_RIGHTLEFT)
static int mr_pattern_alloced = FALSE; 
#endif
#if defined(FEAT_FIND_ID)
typedef struct SearchedFile
{
FILE *fp; 
char_u *name; 
linenr_T lnum; 
int matched; 
} SearchedFile;
#endif
int
search_regcomp(
char_u *pat,
int pat_save,
int pat_use,
int options,
regmmatch_T *regmatch) 
{
int magic;
int i;
rc_did_emsg = FALSE;
magic = p_magic;
if (pat == NULL || *pat == NUL)
{
if (pat_use == RE_LAST)
i = last_idx;
else
i = pat_use;
if (spats[i].pat == NULL) 
{
if (pat_use == RE_SUBST)
emsg(_(e_nopresub));
else
emsg(_(e_noprevre));
rc_did_emsg = TRUE;
return FAIL;
}
pat = spats[i].pat;
magic = spats[i].magic;
no_smartcase = spats[i].no_scs;
}
else if (options & SEARCH_HIS) 
add_to_history(HIST_SEARCH, pat, TRUE, NUL);
#if defined(FEAT_RIGHTLEFT)
if (mr_pattern_alloced)
{
vim_free(mr_pattern);
mr_pattern_alloced = FALSE;
}
if (curwin->w_p_rl && *curwin->w_p_rlc == 's')
{
char_u *rev_pattern;
rev_pattern = reverse_text(pat);
if (rev_pattern == NULL)
mr_pattern = pat; 
else
{
mr_pattern = rev_pattern;
mr_pattern_alloced = TRUE;
}
}
else
#endif
mr_pattern = pat;
if (!(options & SEARCH_KEEP) && !cmdmod.keeppatterns)
{
if (pat_save == RE_SEARCH || pat_save == RE_BOTH)
save_re_pat(RE_SEARCH, pat, magic);
if (pat_save == RE_SUBST || pat_save == RE_BOTH)
save_re_pat(RE_SUBST, pat, magic);
}
regmatch->rmm_ic = ignorecase(pat);
regmatch->rmm_maxcol = 0;
regmatch->regprog = vim_regcomp(pat, magic ? RE_MAGIC : 0);
if (regmatch->regprog == NULL)
return FAIL;
return OK;
}
char_u *
get_search_pat(void)
{
return mr_pattern;
}
#if defined(FEAT_RIGHTLEFT) || defined(PROTO)
char_u *
reverse_text(char_u *s)
{
unsigned len;
unsigned s_i, rev_i;
char_u *rev;
len = (unsigned)STRLEN(s);
rev = alloc(len + 1);
if (rev != NULL)
{
rev_i = len;
for (s_i = 0; s_i < len; ++s_i)
{
if (has_mbyte)
{
int mb_len;
mb_len = (*mb_ptr2len)(s + s_i);
rev_i -= mb_len;
mch_memmove(rev + rev_i, s + s_i, mb_len);
s_i += mb_len - 1;
}
else
rev[--rev_i] = s[s_i];
}
rev[len] = NUL;
}
return rev;
}
#endif
void
save_re_pat(int idx, char_u *pat, int magic)
{
if (spats[idx].pat != pat)
{
vim_free(spats[idx].pat);
spats[idx].pat = vim_strsave(pat);
spats[idx].magic = magic;
spats[idx].no_scs = no_smartcase;
last_idx = idx;
#if defined(FEAT_SEARCH_EXTRA)
if (p_hls)
redraw_all_later(SOME_VALID);
set_no_hlsearch(FALSE);
#endif
}
}
static int save_level = 0;
void
save_search_patterns(void)
{
if (save_level++ == 0)
{
saved_spats[0] = spats[0];
if (spats[0].pat != NULL)
saved_spats[0].pat = vim_strsave(spats[0].pat);
saved_spats[1] = spats[1];
if (spats[1].pat != NULL)
saved_spats[1].pat = vim_strsave(spats[1].pat);
#if defined(FEAT_SEARCH_EXTRA)
saved_spats_last_idx = last_idx;
saved_spats_no_hlsearch = no_hlsearch;
#endif
}
}
void
restore_search_patterns(void)
{
if (--save_level == 0)
{
vim_free(spats[0].pat);
spats[0] = saved_spats[0];
#if defined(FEAT_EVAL)
set_vv_searchforward();
#endif
vim_free(spats[1].pat);
spats[1] = saved_spats[1];
#if defined(FEAT_SEARCH_EXTRA)
last_idx = saved_spats_last_idx;
set_no_hlsearch(saved_spats_no_hlsearch);
#endif
}
}
#if defined(EXITFREE) || defined(PROTO)
void
free_search_patterns(void)
{
vim_free(spats[0].pat);
vim_free(spats[1].pat);
#if defined(FEAT_RIGHTLEFT)
if (mr_pattern_alloced)
{
vim_free(mr_pattern);
mr_pattern_alloced = FALSE;
mr_pattern = NULL;
}
#endif
}
#endif
#if defined(FEAT_SEARCH_EXTRA)
static spat_T saved_last_search_spat;
static int did_save_last_search_spat = 0;
static int saved_last_idx = 0;
static int saved_no_hlsearch = 0;
void
save_last_search_pattern(void)
{
if (did_save_last_search_spat != 0)
iemsg("did_save_last_search_spat is not zero");
else
++did_save_last_search_spat;
saved_last_search_spat = spats[RE_SEARCH];
if (spats[RE_SEARCH].pat != NULL)
saved_last_search_spat.pat = vim_strsave(spats[RE_SEARCH].pat);
saved_last_idx = last_idx;
saved_no_hlsearch = no_hlsearch;
}
void
restore_last_search_pattern(void)
{
if (did_save_last_search_spat != 1)
{
iemsg("did_save_last_search_spat is not one");
return;
}
--did_save_last_search_spat;
vim_free(spats[RE_SEARCH].pat);
spats[RE_SEARCH] = saved_last_search_spat;
saved_last_search_spat.pat = NULL;
#if defined(FEAT_EVAL)
set_vv_searchforward();
#endif
last_idx = saved_last_idx;
set_no_hlsearch(saved_no_hlsearch);
}
char_u *
last_search_pattern(void)
{
return spats[RE_SEARCH].pat;
}
#endif
int
ignorecase(char_u *pat)
{
return ignorecase_opt(pat, p_ic, p_scs);
}
int
ignorecase_opt(char_u *pat, int ic_in, int scs)
{
int ic = ic_in;
if (ic && !no_smartcase && scs
&& !(ctrl_x_mode_not_default() && curbuf->b_p_inf))
ic = !pat_has_uppercase(pat);
no_smartcase = FALSE;
return ic;
}
int
pat_has_uppercase(char_u *pat)
{
char_u *p = pat;
while (*p != NUL)
{
int l;
if (has_mbyte && (l = (*mb_ptr2len)(p)) > 1)
{
if (enc_utf8 && utf_isupper(utf_ptr2char(p)))
return TRUE;
p += l;
}
else if (*p == '\\')
{
if (p[1] == '_' && p[2] != NUL) 
p += 3;
else if (p[1] == '%' && p[2] != NUL) 
p += 3;
else if (p[1] != NUL) 
p += 2;
else
p += 1;
}
else if (MB_ISUPPER(*p))
return TRUE;
else
++p;
}
return FALSE;
}
#if defined(FEAT_EVAL) || defined(PROTO)
char_u *
last_csearch(void)
{
return lastc_bytes;
}
int
last_csearch_forward(void)
{
return lastcdir == FORWARD;
}
int
last_csearch_until(void)
{
return last_t_cmd == TRUE;
}
void
set_last_csearch(int c, char_u *s UNUSED, int len UNUSED)
{
*lastc = c;
lastc_bytelen = len;
if (len)
memcpy(lastc_bytes, s, len);
else
vim_memset(lastc_bytes, 0, sizeof(lastc_bytes));
}
#endif
void
set_csearch_direction(int cdir)
{
lastcdir = cdir;
}
void
set_csearch_until(int t_cmd)
{
last_t_cmd = t_cmd;
}
char_u *
last_search_pat(void)
{
return spats[last_idx].pat;
}
void
reset_search_dir(void)
{
spats[0].off.dir = '/';
#if defined(FEAT_EVAL)
set_vv_searchforward();
#endif
}
#if defined(FEAT_EVAL) || defined(FEAT_VIMINFO)
void
set_last_search_pat(
char_u *s,
int idx,
int magic,
int setlast)
{
vim_free(spats[idx].pat);
if (*s == NUL)
spats[idx].pat = NULL;
else
spats[idx].pat = vim_strsave(s);
spats[idx].magic = magic;
spats[idx].no_scs = FALSE;
spats[idx].off.dir = '/';
#if defined(FEAT_EVAL)
set_vv_searchforward();
#endif
spats[idx].off.line = FALSE;
spats[idx].off.end = FALSE;
spats[idx].off.off = 0;
if (setlast)
last_idx = idx;
if (save_level)
{
vim_free(saved_spats[idx].pat);
saved_spats[idx] = spats[0];
if (spats[idx].pat == NULL)
saved_spats[idx].pat = NULL;
else
saved_spats[idx].pat = vim_strsave(spats[idx].pat);
#if defined(FEAT_SEARCH_EXTRA)
saved_spats_last_idx = last_idx;
#endif
}
#if defined(FEAT_SEARCH_EXTRA)
if (p_hls && idx == last_idx && !no_hlsearch)
redraw_all_later(SOME_VALID);
#endif
}
#endif
#if defined(FEAT_SEARCH_EXTRA)
void
last_pat_prog(regmmatch_T *regmatch)
{
if (spats[last_idx].pat == NULL)
{
regmatch->regprog = NULL;
return;
}
++emsg_off; 
(void)search_regcomp((char_u *)"", 0, last_idx, SEARCH_KEEP, regmatch);
--emsg_off;
}
#endif
int
searchit(
win_T *win, 
buf_T *buf,
pos_T *pos,
pos_T *end_pos, 
int dir,
char_u *pat,
long count,
int options,
int pat_use, 
searchit_arg_T *extra_arg) 
{
int found;
linenr_T lnum; 
colnr_T col;
regmmatch_T regmatch;
char_u *ptr;
colnr_T matchcol;
lpos_T endpos;
lpos_T matchpos;
int loop;
pos_T start_pos;
int at_first_line;
int extra_col;
int start_char_len;
int match_ok;
long nmatched;
int submatch = 0;
int first_match = TRUE;
int called_emsg_before = called_emsg;
#if defined(FEAT_SEARCH_EXTRA)
int break_loop = FALSE;
#endif
linenr_T stop_lnum = 0; 
#if defined(FEAT_RELTIME)
proftime_T *tm = NULL; 
int *timed_out = NULL; 
#endif
if (extra_arg != NULL)
{
stop_lnum = extra_arg->sa_stop_lnum;
#if defined(FEAT_RELTIME)
tm = extra_arg->sa_tm;
timed_out = &extra_arg->sa_timed_out;
#endif
}
if (search_regcomp(pat, RE_SEARCH, pat_use,
(options & (SEARCH_HIS + SEARCH_KEEP)), &regmatch) == FAIL)
{
if ((options & SEARCH_MSG) && !rc_did_emsg)
semsg(_("E383: Invalid search string: %s"), mr_pattern);
return FAIL;
}
do 
{
if (pos->col == MAXCOL)
start_char_len = 0;
else if (has_mbyte
&& pos->lnum >= 1 && pos->lnum <= buf->b_ml.ml_line_count
&& pos->col < MAXCOL - 2)
{
ptr = ml_get_buf(buf, pos->lnum, FALSE);
if ((int)STRLEN(ptr) <= pos->col)
start_char_len = 1;
else
start_char_len = (*mb_ptr2len)(ptr + pos->col);
}
else
start_char_len = 1;
if (dir == FORWARD)
{
if (options & SEARCH_START)
extra_col = 0;
else
extra_col = start_char_len;
}
else
{
if (options & SEARCH_START)
extra_col = start_char_len;
else
extra_col = 0;
}
start_pos = *pos; 
found = 0; 
at_first_line = TRUE; 
if (pos->lnum == 0) 
{
pos->lnum = 1;
pos->col = 0;
at_first_line = FALSE; 
}
if (dir == BACKWARD && start_pos.col == 0
&& (options & SEARCH_START) == 0)
{
lnum = pos->lnum - 1;
at_first_line = FALSE;
}
else
lnum = pos->lnum;
for (loop = 0; loop <= 1; ++loop) 
{
for ( ; lnum > 0 && lnum <= buf->b_ml.ml_line_count;
lnum += dir, at_first_line = FALSE)
{
if (stop_lnum != 0 && (dir == FORWARD
? lnum > stop_lnum : lnum < stop_lnum))
break;
#if defined(FEAT_RELTIME)
if (tm != NULL && profile_passed_limit(tm))
break;
#endif
col = at_first_line && (options & SEARCH_COL) ? pos->col
: (colnr_T)0;
nmatched = vim_regexec_multi(&regmatch, win, buf,
lnum, col,
#if defined(FEAT_RELTIME)
tm, timed_out
#else
NULL, NULL
#endif
);
if (called_emsg > called_emsg_before
#if defined(FEAT_RELTIME)
|| (timed_out != NULL && *timed_out)
#endif
)
break;
if (nmatched > 0)
{
matchpos = regmatch.startpos[0];
endpos = regmatch.endpos[0];
#if defined(FEAT_EVAL)
submatch = first_submatch(&regmatch);
#endif
if (lnum + matchpos.lnum > buf->b_ml.ml_line_count)
ptr = (char_u *)"";
else
ptr = ml_get_buf(buf, lnum + matchpos.lnum, FALSE);
if (dir == FORWARD && at_first_line)
{
match_ok = TRUE;
while (matchpos.lnum == 0
&& ((options & SEARCH_END) && first_match
? (nmatched == 1
&& (int)endpos.col - 1
< (int)start_pos.col + extra_col)
: ((int)matchpos.col
- (ptr[matchpos.col] == NUL)
< (int)start_pos.col + extra_col)))
{
if (vim_strchr(p_cpo, CPO_SEARCH) != NULL)
{
if (nmatched > 1)
{
match_ok = FALSE;
break;
}
matchcol = endpos.col;
if (matchcol == matchpos.col
&& ptr[matchcol] != NUL)
{
if (has_mbyte)
matchcol +=
(*mb_ptr2len)(ptr + matchcol);
else
++matchcol;
}
}
else
{
matchcol = matchpos.col;
if (ptr[matchcol] != NUL)
{
if (has_mbyte)
matchcol += (*mb_ptr2len)(ptr
+ matchcol);
else
++matchcol;
}
}
if (matchcol == 0 && (options & SEARCH_START))
break;
if (ptr[matchcol] == NUL
|| (nmatched = vim_regexec_multi(&regmatch,
win, buf, lnum + matchpos.lnum,
matchcol,
#if defined(FEAT_RELTIME)
tm, timed_out
#else
NULL, NULL
#endif
)) == 0)
{
match_ok = FALSE;
break;
}
matchpos = regmatch.startpos[0];
endpos = regmatch.endpos[0];
#if defined(FEAT_EVAL)
submatch = first_submatch(&regmatch);
#endif
ptr = ml_get_buf(buf, lnum + matchpos.lnum, FALSE);
}
if (!match_ok)
continue;
}
if (dir == BACKWARD)
{
match_ok = FALSE;
for (;;)
{
if (loop
|| ((options & SEARCH_END)
? (lnum + regmatch.endpos[0].lnum
< start_pos.lnum
|| (lnum + regmatch.endpos[0].lnum
== start_pos.lnum
&& (int)regmatch.endpos[0].col - 1
< (int)start_pos.col
+ extra_col))
: (lnum + regmatch.startpos[0].lnum
< start_pos.lnum
|| (lnum + regmatch.startpos[0].lnum
== start_pos.lnum
&& (int)regmatch.startpos[0].col
< (int)start_pos.col
+ extra_col))))
{
match_ok = TRUE;
matchpos = regmatch.startpos[0];
endpos = regmatch.endpos[0];
#if defined(FEAT_EVAL)
submatch = first_submatch(&regmatch);
#endif
}
else
break;
if (vim_strchr(p_cpo, CPO_SEARCH) != NULL)
{
if (nmatched > 1)
break;
matchcol = endpos.col;
if (matchcol == matchpos.col
&& ptr[matchcol] != NUL)
{
if (has_mbyte)
matchcol +=
(*mb_ptr2len)(ptr + matchcol);
else
++matchcol;
}
}
else
{
if (matchpos.lnum > 0)
break;
matchcol = matchpos.col;
if (ptr[matchcol] != NUL)
{
if (has_mbyte)
matchcol +=
(*mb_ptr2len)(ptr + matchcol);
else
++matchcol;
}
}
if (ptr[matchcol] == NUL
|| (nmatched = vim_regexec_multi(&regmatch,
win, buf, lnum + matchpos.lnum,
matchcol,
#if defined(FEAT_RELTIME)
tm, timed_out
#else
NULL, NULL
#endif
)) == 0)
{
#if defined(FEAT_RELTIME)
if (timed_out != NULL && *timed_out)
match_ok = FALSE;
#endif
break;
}
ptr = ml_get_buf(buf, lnum + matchpos.lnum, FALSE);
}
if (!match_ok)
continue;
}
if ((options & SEARCH_END) && !(options & SEARCH_NOOF)
&& !(matchpos.lnum == endpos.lnum
&& matchpos.col == endpos.col))
{
pos->lnum = lnum + endpos.lnum;
pos->col = endpos.col;
if (endpos.col == 0)
{
if (pos->lnum > 1) 
{
--pos->lnum;
pos->col = (colnr_T)STRLEN(ml_get_buf(buf,
pos->lnum, FALSE));
}
}
else
{
--pos->col;
if (has_mbyte
&& pos->lnum <= buf->b_ml.ml_line_count)
{
ptr = ml_get_buf(buf, pos->lnum, FALSE);
pos->col -= (*mb_head_off)(ptr, ptr + pos->col);
}
}
if (end_pos != NULL)
{
end_pos->lnum = lnum + matchpos.lnum;
end_pos->col = matchpos.col;
}
}
else
{
pos->lnum = lnum + matchpos.lnum;
pos->col = matchpos.col;
if (end_pos != NULL)
{
end_pos->lnum = lnum + endpos.lnum;
end_pos->col = endpos.col;
}
}
pos->coladd = 0;
if (end_pos != NULL)
end_pos->coladd = 0;
found = 1;
first_match = FALSE;
search_match_lines = endpos.lnum - matchpos.lnum;
search_match_endcol = endpos.col;
break;
}
line_breakcheck(); 
if (got_int)
break;
#if defined(FEAT_SEARCH_EXTRA)
if ((options & SEARCH_PEEK)
&& ((lnum - pos->lnum) & 0x3f) == 0
&& char_avail())
{
break_loop = TRUE;
break;
}
#endif
if (loop && lnum == start_pos.lnum)
break; 
}
at_first_line = FALSE;
if (!p_ws || stop_lnum != 0 || got_int
|| called_emsg > called_emsg_before
#if defined(FEAT_RELTIME)
|| (timed_out != NULL && *timed_out)
#endif
#if defined(FEAT_SEARCH_EXTRA)
|| break_loop
#endif
|| found || loop)
break;
if (dir == BACKWARD) 
lnum = buf->b_ml.ml_line_count;
else
lnum = 1;
if (!shortmess(SHM_SEARCH) && (options & SEARCH_MSG))
give_warning((char_u *)_(dir == BACKWARD
? top_bot_msg : bot_top_msg), TRUE);
if (extra_arg != NULL)
extra_arg->sa_wrapped = TRUE;
}
if (got_int || called_emsg > called_emsg_before
#if defined(FEAT_RELTIME)
|| (timed_out != NULL && *timed_out)
#endif
#if defined(FEAT_SEARCH_EXTRA)
|| break_loop
#endif
)
break;
}
while (--count > 0 && found); 
vim_regfree(regmatch.regprog);
if (!found) 
{
if (got_int)
emsg(_(e_interr));
else if ((options & SEARCH_MSG) == SEARCH_MSG)
{
if (p_ws)
semsg(_(e_patnotf2), mr_pattern);
else if (lnum == 0)
semsg(_("E384: search hit TOP without match for: %s"),
mr_pattern);
else
semsg(_("E385: search hit BOTTOM without match for: %s"),
mr_pattern);
}
return FAIL;
}
if (pos->lnum > buf->b_ml.ml_line_count)
{
pos->lnum = buf->b_ml.ml_line_count;
pos->col = (int)STRLEN(ml_get_buf(buf, pos->lnum, FALSE));
if (pos->col > 0)
--pos->col;
}
return submatch + 1;
}
#if defined(FEAT_EVAL)
void
set_search_direction(int cdir)
{
spats[0].off.dir = cdir;
}
static void
set_vv_searchforward(void)
{
set_vim_var_nr(VV_SEARCHFORWARD, (long)(spats[0].off.dir == '/'));
}
static int
first_submatch(regmmatch_T *rp)
{
int submatch;
for (submatch = 1; ; ++submatch)
{
if (rp->startpos[submatch].lnum >= 0)
break;
if (submatch == 9)
{
submatch = 0;
break;
}
}
return submatch;
}
#endif
int
do_search(
oparg_T *oap, 
int dirc, 
int search_delim, 
char_u *pat,
long count,
int options,
searchit_arg_T *sia) 
{
pos_T pos; 
char_u *searchstr;
soffset_T old_off;
int retval; 
char_u *p;
long c;
char_u *dircp;
char_u *strcopy = NULL;
char_u *ps;
char_u *msgbuf = NULL;
size_t len;
int has_offset = FALSE;
#define SEARCH_STAT_BUF_LEN 12
if (spats[0].off.line && vim_strchr(p_cpo, CPO_LINEOFF) != NULL)
{
spats[0].off.line = FALSE;
spats[0].off.off = 0;
}
old_off = spats[0].off;
pos = curwin->w_cursor; 
if (dirc == 0)
dirc = spats[0].off.dir;
else
{
spats[0].off.dir = dirc;
#if defined(FEAT_EVAL)
set_vv_searchforward();
#endif
}
if (options & SEARCH_REV)
{
#if defined(MSWIN)
dirc = (dirc == '/') ? '?' : '/';
#else
if (dirc == '/')
dirc = '?';
else
dirc = '/';
#endif
}
#if defined(FEAT_FOLDING)
if (dirc == '/')
{
if (hasFolding(pos.lnum, NULL, &pos.lnum))
pos.col = MAXCOL - 2; 
}
else
{
if (hasFolding(pos.lnum, &pos.lnum, NULL))
pos.col = 0;
}
#endif
#if defined(FEAT_SEARCH_EXTRA)
if (no_hlsearch && !(options & SEARCH_KEEP))
{
redraw_all_later(SOME_VALID);
set_no_hlsearch(FALSE);
}
#endif
for (;;)
{
int show_top_bot_msg = FALSE;
searchstr = pat;
dircp = NULL;
if (pat == NULL || *pat == NUL || *pat == search_delim)
{
if (spats[RE_SEARCH].pat == NULL) 
{
searchstr = spats[RE_SUBST].pat;
if (searchstr == NULL)
{
emsg(_(e_noprevre));
retval = 0;
goto end_do_search;
}
}
else
{
searchstr = (char_u *)"";
}
}
if (pat != NULL && *pat != NUL) 
{
ps = strcopy;
p = skip_regexp_ex(pat, search_delim, (int)p_magic, &strcopy, NULL);
if (strcopy != ps)
{
searchcmdlen += (int)(STRLEN(pat) - STRLEN(strcopy));
pat = strcopy;
searchstr = strcopy;
}
if (*p == search_delim)
{
dircp = p; 
*p++ = NUL;
}
spats[0].off.line = FALSE;
spats[0].off.end = FALSE;
spats[0].off.off = 0;
if (*p == '+' || *p == '-' || VIM_ISDIGIT(*p))
spats[0].off.line = TRUE;
else if ((options & SEARCH_OPT) &&
(*p == 'e' || *p == 's' || *p == 'b'))
{
if (*p == 'e') 
spats[0].off.end = SEARCH_END;
++p;
}
if (VIM_ISDIGIT(*p) || *p == '+' || *p == '-') 
{
if (VIM_ISDIGIT(*p) || VIM_ISDIGIT(*(p + 1)))
spats[0].off.off = atol((char *)p);
else if (*p == '-') 
spats[0].off.off = -1;
else 
spats[0].off.off = 1;
++p;
while (VIM_ISDIGIT(*p)) 
++p;
}
searchcmdlen += (int)(p - pat);
pat = p; 
}
if ((options & SEARCH_ECHO) && messaging() &&
!msg_silent &&
(!cmd_silent || !shortmess(SHM_SEARCHCOUNT)))
{
char_u *trunc;
char_u off_buf[40];
size_t off_len = 0;
msg_start();
if (!cmd_silent &&
(spats[0].off.line || spats[0].off.end || spats[0].off.off))
{
p = off_buf;
*p++ = dirc;
if (spats[0].off.end)
*p++ = 'e';
else if (!spats[0].off.line)
*p++ = 's';
if (spats[0].off.off > 0 || spats[0].off.line)
*p++ = '+';
*p = NUL;
if (spats[0].off.off != 0 || spats[0].off.line)
sprintf((char *)p, "%ld", spats[0].off.off);
off_len = STRLEN(off_buf);
}
if (*searchstr == NUL)
p = spats[0].pat;
else
p = searchstr;
if (!shortmess(SHM_SEARCHCOUNT) || cmd_silent)
{
if (msg_scrolled != 0 && !cmd_silent)
len = (int)(Rows - msg_row) * Columns - 1;
else
len = (int)(Rows - msg_row - 1) * Columns + sc_col - 1;
if (len < STRLEN(p) + off_len + SEARCH_STAT_BUF_LEN + 3)
len = STRLEN(p) + off_len + SEARCH_STAT_BUF_LEN + 3;
}
else
len = STRLEN(p) + off_len + 3;
msgbuf = alloc(len);
if (msgbuf != NULL)
{
vim_memset(msgbuf, ' ', len);
msgbuf[len - 1] = NUL;
if (!cmd_silent)
{
msgbuf[0] = dirc;
if (enc_utf8 && utf_iscomposing(utf_ptr2char(p)))
{
msgbuf[1] = ' ';
mch_memmove(msgbuf + 2, p, STRLEN(p));
}
else
mch_memmove(msgbuf + 1, p, STRLEN(p));
if (off_len > 0)
mch_memmove(msgbuf + STRLEN(p) + 1, off_buf, off_len);
trunc = msg_strtrunc(msgbuf, TRUE);
if (trunc != NULL)
{
vim_free(msgbuf);
msgbuf = trunc;
}
#if defined(FEAT_RIGHTLEFT)
if (curwin->w_p_rl && *curwin->w_p_rlc == 's')
{
char_u *r;
size_t pat_len;
r = reverse_text(msgbuf);
if (r != NULL)
{
vim_free(msgbuf);
msgbuf = r;
while (*r != NUL && *r == ' ')
r++;
pat_len = msgbuf + STRLEN(msgbuf) - r;
mch_memmove(msgbuf, r, pat_len);
if ((size_t)(r - msgbuf) >= pat_len)
vim_memset(r, ' ', pat_len);
else
vim_memset(msgbuf + pat_len, ' ', r - msgbuf);
}
}
#endif
msg_outtrans(msgbuf);
msg_clr_eos();
msg_check();
gotocmdline(FALSE);
out_flush();
msg_nowait = TRUE; 
}
}
}
if (!spats[0].off.line && spats[0].off.off && pos.col < MAXCOL - 2)
{
if (spats[0].off.off > 0)
{
for (c = spats[0].off.off; c; --c)
if (decl(&pos) == -1)
break;
if (c) 
{
pos.lnum = 0; 
pos.col = MAXCOL;
}
}
else
{
for (c = spats[0].off.off; c; ++c)
if (incl(&pos) == -1)
break;
if (c) 
{
pos.lnum = curbuf->b_ml.ml_line_count + 1;
pos.col = 0;
}
}
}
c = searchit(curwin, curbuf, &pos, NULL,
dirc == '/' ? FORWARD : BACKWARD,
searchstr, count, spats[0].off.end + (options &
(SEARCH_KEEP + SEARCH_PEEK + SEARCH_HIS
+ SEARCH_MSG + SEARCH_START
+ ((pat != NULL && *pat == ';') ? 0 : SEARCH_NOOF))),
RE_LAST, sia);
if (dircp != NULL)
*dircp = search_delim; 
if (!shortmess(SHM_SEARCH)
&& ((dirc == '/' && LT_POS(pos, curwin->w_cursor))
|| (dirc == '?' && LT_POS(curwin->w_cursor, pos))))
show_top_bot_msg = TRUE;
if (c == FAIL)
{
retval = 0;
goto end_do_search;
}
if (spats[0].off.end && oap != NULL)
oap->inclusive = TRUE; 
retval = 1; 
if (!(options & SEARCH_NOOF) || (pat != NULL && *pat == ';'))
{
pos_T org_pos = pos;
if (spats[0].off.line) 
{
c = pos.lnum + spats[0].off.off;
if (c < 1)
pos.lnum = 1;
else if (c > curbuf->b_ml.ml_line_count)
pos.lnum = curbuf->b_ml.ml_line_count;
else
pos.lnum = c;
pos.col = 0;
retval = 2; 
}
else if (pos.col < MAXCOL - 2) 
{
c = spats[0].off.off;
if (c > 0)
{
while (c-- > 0)
if (incl(&pos) == -1)
break;
}
else
{
while (c++ < 0)
if (decl(&pos) == -1)
break;
}
}
if (!EQUAL_POS(pos, org_pos))
has_offset = TRUE;
}
if ((options & SEARCH_ECHO)
&& messaging()
&& !msg_silent
&& c != FAIL
&& !shortmess(SHM_SEARCHCOUNT)
&& msgbuf != NULL)
search_stat(dirc, &pos, show_top_bot_msg, msgbuf,
(count != 1 || has_offset));
if (!(options & SEARCH_OPT) || pat == NULL || *pat != ';')
break;
dirc = *++pat;
search_delim = dirc;
if (dirc != '?' && dirc != '/')
{
retval = 0;
emsg(_("E386: Expected '?' or '/' after ';'"));
goto end_do_search;
}
++pat;
}
if (options & SEARCH_MARK)
setpcmark();
curwin->w_cursor = pos;
curwin->w_set_curswant = TRUE;
end_do_search:
if ((options & SEARCH_KEEP) || cmdmod.keeppatterns)
spats[0].off = old_off;
vim_free(strcopy);
vim_free(msgbuf);
return retval;
}
int
search_for_exact_line(
buf_T *buf,
pos_T *pos,
int dir,
char_u *pat)
{
linenr_T start = 0;
char_u *ptr;
char_u *p;
if (buf->b_ml.ml_line_count == 0)
return FAIL;
for (;;)
{
pos->lnum += dir;
if (pos->lnum < 1)
{
if (p_ws)
{
pos->lnum = buf->b_ml.ml_line_count;
if (!shortmess(SHM_SEARCH))
give_warning((char_u *)_(top_bot_msg), TRUE);
}
else
{
pos->lnum = 1;
break;
}
}
else if (pos->lnum > buf->b_ml.ml_line_count)
{
if (p_ws)
{
pos->lnum = 1;
if (!shortmess(SHM_SEARCH))
give_warning((char_u *)_(bot_top_msg), TRUE);
}
else
{
pos->lnum = 1;
break;
}
}
if (pos->lnum == start)
break;
if (start == 0)
start = pos->lnum;
ptr = ml_get_buf(buf, pos->lnum, FALSE);
p = skipwhite(ptr);
pos->col = (colnr_T) (p - ptr);
if ((compl_cont_status & CONT_ADDING)
&& !(compl_cont_status & CONT_SOL))
{
if ((p_ic ? MB_STRICMP(p, pat) : STRCMP(p, pat)) == 0)
return OK;
}
else if (*p != NUL) 
{ 
if ((p_ic ? MB_STRNICMP(p, pat, compl_length)
: STRNCMP(p, pat, compl_length)) == 0)
return OK;
}
}
return FAIL;
}
int
searchc(cmdarg_T *cap, int t_cmd)
{
int c = cap->nchar; 
int dir = cap->arg; 
long count = cap->count1; 
int col;
char_u *p;
int len;
int stop = TRUE;
if (c != NUL) 
{
if (!KeyStuffed) 
{
*lastc = c;
set_csearch_direction(dir);
set_csearch_until(t_cmd);
lastc_bytelen = (*mb_char2bytes)(c, lastc_bytes);
if (cap->ncharC1 != 0)
{
lastc_bytelen += (*mb_char2bytes)(cap->ncharC1,
lastc_bytes + lastc_bytelen);
if (cap->ncharC2 != 0)
lastc_bytelen += (*mb_char2bytes)(cap->ncharC2,
lastc_bytes + lastc_bytelen);
}
}
}
else 
{
if (*lastc == NUL && lastc_bytelen == 1)
return FAIL;
if (dir) 
dir = -lastcdir;
else
dir = lastcdir;
t_cmd = last_t_cmd;
c = *lastc;
if (vim_strchr(p_cpo, CPO_SCOLON) == NULL && count == 1 && t_cmd)
stop = FALSE;
}
if (dir == BACKWARD)
cap->oap->inclusive = FALSE;
else
cap->oap->inclusive = TRUE;
p = ml_get_curline();
col = curwin->w_cursor.col;
len = (int)STRLEN(p);
while (count--)
{
if (has_mbyte)
{
for (;;)
{
if (dir > 0)
{
col += (*mb_ptr2len)(p + col);
if (col >= len)
return FAIL;
}
else
{
if (col == 0)
return FAIL;
col -= (*mb_head_off)(p, p + col - 1) + 1;
}
if (lastc_bytelen == 1)
{
if (p[col] == c && stop)
break;
}
else if (STRNCMP(p + col, lastc_bytes, lastc_bytelen) == 0
&& stop)
break;
stop = TRUE;
}
}
else
{
for (;;)
{
if ((col += dir) < 0 || col >= len)
return FAIL;
if (p[col] == c && stop)
break;
stop = TRUE;
}
}
}
if (t_cmd)
{
col -= dir;
if (has_mbyte)
{
if (dir < 0)
col += lastc_bytelen - 1;
else
col -= (*mb_head_off)(p, p + col);
}
}
curwin->w_cursor.col = col;
return OK;
}
pos_T *
findmatch(oparg_T *oap, int initc)
{
return findmatchlimit(oap, initc, 0, 0);
}
static int
check_prevcol(
char_u *linep,
int col,
int ch,
int *prevcol)
{
--col;
if (col > 0 && has_mbyte)
col -= (*mb_head_off)(linep, linep + col);
if (prevcol)
*prevcol = col;
return (col >= 0 && linep[col] == ch) ? TRUE : FALSE;
}
static int
find_rawstring_end(char_u *linep, pos_T *startpos, pos_T *endpos)
{
char_u *p;
char_u *delim_copy;
size_t delim_len;
linenr_T lnum;
int found = FALSE;
for (p = linep + startpos->col + 1; *p && *p != '('; ++p)
;
delim_len = (p - linep) - startpos->col - 1;
delim_copy = vim_strnsave(linep + startpos->col + 1, (int)delim_len);
if (delim_copy == NULL)
return FALSE;
for (lnum = startpos->lnum; lnum <= endpos->lnum; ++lnum)
{
char_u *line = ml_get(lnum);
for (p = line + (lnum == startpos->lnum
? startpos->col + 1 : 0); *p; ++p)
{
if (lnum == endpos->lnum && (colnr_T)(p - line) >= endpos->col)
break;
if (*p == ')' && p[delim_len + 1] == '"'
&& STRNCMP(delim_copy, p + 1, delim_len) == 0)
{
found = TRUE;
break;
}
}
if (found)
break;
}
vim_free(delim_copy);
return found;
}
static void
find_mps_values(
int *initc,
int *findc,
int *backwards,
int switchit)
{
char_u *ptr;
ptr = curbuf->b_p_mps;
while (*ptr != NUL)
{
if (has_mbyte)
{
char_u *prev;
if (mb_ptr2char(ptr) == *initc)
{
if (switchit)
{
*findc = *initc;
*initc = mb_ptr2char(ptr + mb_ptr2len(ptr) + 1);
*backwards = TRUE;
}
else
{
*findc = mb_ptr2char(ptr + mb_ptr2len(ptr) + 1);
*backwards = FALSE;
}
return;
}
prev = ptr;
ptr += mb_ptr2len(ptr) + 1;
if (mb_ptr2char(ptr) == *initc)
{
if (switchit)
{
*findc = *initc;
*initc = mb_ptr2char(prev);
*backwards = FALSE;
}
else
{
*findc = mb_ptr2char(prev);
*backwards = TRUE;
}
return;
}
ptr += mb_ptr2len(ptr);
}
else
{
if (*ptr == *initc)
{
if (switchit)
{
*backwards = TRUE;
*findc = *initc;
*initc = ptr[2];
}
else
{
*backwards = FALSE;
*findc = ptr[2];
}
return;
}
ptr += 2;
if (*ptr == *initc)
{
if (switchit)
{
*backwards = FALSE;
*findc = *initc;
*initc = ptr[-2];
}
else
{
*backwards = TRUE;
*findc = ptr[-2];
}
return;
}
++ptr;
}
if (*ptr == ',')
++ptr;
}
}
pos_T *
findmatchlimit(
oparg_T *oap,
int initc,
int flags,
int maxtravel)
{
static pos_T pos; 
int findc = 0; 
int c;
int count = 0; 
int backwards = FALSE; 
int raw_string = FALSE; 
int inquote = FALSE; 
char_u *linep; 
char_u *ptr;
int do_quotes; 
int at_start; 
int hash_dir = 0; 
int comment_dir = 0; 
pos_T match_pos; 
int start_in_quotes; 
int traveled = 0; 
int ignore_cend = FALSE; 
int cpo_match; 
int cpo_bsl; 
int match_escaped = 0; 
int dir; 
int comment_col = MAXCOL; 
#if defined(FEAT_LISP)
int lispcomm = FALSE; 
int lisp = curbuf->b_p_lisp; 
#endif
pos = curwin->w_cursor;
pos.coladd = 0;
linep = ml_get(pos.lnum);
cpo_match = (vim_strchr(p_cpo, CPO_MATCH) != NULL);
cpo_bsl = (vim_strchr(p_cpo, CPO_MATCHBSL) != NULL);
if (flags & FM_BACKWARD)
dir = BACKWARD;
else if (flags & FM_FORWARD)
dir = FORWARD;
else
dir = 0;
if (initc == '/' || initc == '*' || initc == 'R')
{
comment_dir = dir;
if (initc == '/')
ignore_cend = TRUE;
backwards = (dir == FORWARD) ? FALSE : TRUE;
raw_string = (initc == 'R');
initc = NUL;
}
else if (initc != '#' && initc != NUL)
{
find_mps_values(&initc, &findc, &backwards, TRUE);
if (findc == NUL)
return NULL;
}
else
{
if (initc == '#')
{
hash_dir = dir;
}
else
{
if (!cpo_match)
{
ptr = skipwhite(linep);
if (*ptr == '#' && pos.col <= (colnr_T)(ptr - linep))
{
ptr = skipwhite(ptr + 1);
if ( STRNCMP(ptr, "if", 2) == 0
|| STRNCMP(ptr, "endif", 5) == 0
|| STRNCMP(ptr, "el", 2) == 0)
hash_dir = 1;
}
else if (linep[pos.col] == '/')
{
if (linep[pos.col + 1] == '*')
{
comment_dir = FORWARD;
backwards = FALSE;
pos.col++;
}
else if (pos.col > 0 && linep[pos.col - 1] == '*')
{
comment_dir = BACKWARD;
backwards = TRUE;
pos.col--;
}
}
else if (linep[pos.col] == '*')
{
if (linep[pos.col + 1] == '/')
{
comment_dir = BACKWARD;
backwards = TRUE;
}
else if (pos.col > 0 && linep[pos.col - 1] == '/')
{
comment_dir = FORWARD;
backwards = FALSE;
}
}
}
if (!hash_dir && !comment_dir)
{
if (linep[pos.col] == NUL && pos.col)
--pos.col;
for (;;)
{
initc = PTR2CHAR(linep + pos.col);
if (initc == NUL)
break;
find_mps_values(&initc, &findc, &backwards, FALSE);
if (findc)
break;
pos.col += mb_ptr2len(linep + pos.col);
}
if (!findc)
{
if (!cpo_match && *skipwhite(linep) == '#')
hash_dir = 1;
else
return NULL;
}
else if (!cpo_bsl)
{
int col, bslcnt = 0;
for (col = pos.col; check_prevcol(linep, col, '\\', &col);)
bslcnt++;
match_escaped = (bslcnt & 1);
}
}
}
if (hash_dir)
{
if (oap != NULL)
oap->motion_type = MLINE; 
if (initc != '#')
{
ptr = skipwhite(skipwhite(linep) + 1);
if (STRNCMP(ptr, "if", 2) == 0 || STRNCMP(ptr, "el", 2) == 0)
hash_dir = 1;
else if (STRNCMP(ptr, "endif", 5) == 0)
hash_dir = -1;
else
return NULL;
}
pos.col = 0;
while (!got_int)
{
if (hash_dir > 0)
{
if (pos.lnum == curbuf->b_ml.ml_line_count)
break;
}
else if (pos.lnum == 1)
break;
pos.lnum += hash_dir;
linep = ml_get(pos.lnum);
line_breakcheck(); 
ptr = skipwhite(linep);
if (*ptr != '#')
continue;
pos.col = (colnr_T) (ptr - linep);
ptr = skipwhite(ptr + 1);
if (hash_dir > 0)
{
if (STRNCMP(ptr, "if", 2) == 0)
count++;
else if (STRNCMP(ptr, "el", 2) == 0)
{
if (count == 0)
return &pos;
}
else if (STRNCMP(ptr, "endif", 5) == 0)
{
if (count == 0)
return &pos;
count--;
}
}
else
{
if (STRNCMP(ptr, "if", 2) == 0)
{
if (count == 0)
return &pos;
count--;
}
else if (initc == '#' && STRNCMP(ptr, "el", 2) == 0)
{
if (count == 0)
return &pos;
}
else if (STRNCMP(ptr, "endif", 5) == 0)
count++;
}
}
return NULL;
}
}
#if defined(FEAT_RIGHTLEFT)
if (curwin->w_p_rl && vim_strchr((char_u *)"()[]{}<>", initc) != NULL)
backwards = !backwards;
#endif
do_quotes = -1;
start_in_quotes = MAYBE;
CLEAR_POS(&match_pos);
if ((backwards && comment_dir)
#if defined(FEAT_LISP)
|| lisp
#endif
)
comment_col = check_linecomment(linep);
#if defined(FEAT_LISP)
if (lisp && comment_col != MAXCOL && pos.col > (colnr_T)comment_col)
lispcomm = TRUE; 
#endif
while (!got_int)
{
if (backwards)
{
#if defined(FEAT_LISP)
if (lispcomm && pos.col < (colnr_T)comment_col)
break;
#endif
if (pos.col == 0) 
{
if (pos.lnum == 1) 
break;
--pos.lnum;
if (maxtravel > 0 && ++traveled > maxtravel)
break;
linep = ml_get(pos.lnum);
pos.col = (colnr_T)STRLEN(linep); 
do_quotes = -1;
line_breakcheck();
if (comment_dir
#if defined(FEAT_LISP)
|| lisp
#endif
)
comment_col = check_linecomment(linep);
#if defined(FEAT_LISP)
if (lisp && comment_col != MAXCOL)
pos.col = comment_col;
#endif
}
else
{
--pos.col;
if (has_mbyte)
pos.col -= (*mb_head_off)(linep, linep + pos.col);
}
}
else 
{
if (linep[pos.col] == NUL
#if defined(FEAT_LISP)
|| (lisp && comment_col != MAXCOL
&& pos.col == (colnr_T)comment_col)
#endif
)
{
if (pos.lnum == curbuf->b_ml.ml_line_count 
#if defined(FEAT_LISP)
|| lispcomm
#endif
)
break;
++pos.lnum;
if (maxtravel && traveled++ > maxtravel)
break;
linep = ml_get(pos.lnum);
pos.col = 0;
do_quotes = -1;
line_breakcheck();
#if defined(FEAT_LISP)
if (lisp) 
comment_col = check_linecomment(linep);
#endif
}
else
{
if (has_mbyte)
pos.col += (*mb_ptr2len)(linep + pos.col);
else
++pos.col;
}
}
if (pos.col == 0 && (flags & FM_BLOCKSTOP) &&
(linep[0] == '{' || linep[0] == '}'))
{
if (linep[0] == findc && count == 0) 
return &pos;
break; 
}
if (comment_dir)
{
if (comment_dir == FORWARD)
{
if (linep[pos.col] == '*' && linep[pos.col + 1] == '/')
{
pos.col++;
return &pos;
}
}
else 
{
if (pos.col == 0)
continue;
else if (raw_string)
{
if (linep[pos.col - 1] == 'R'
&& linep[pos.col] == '"'
&& vim_strchr(linep + pos.col + 1, '(') != NULL)
{
if (!find_rawstring_end(linep, &pos,
count > 0 ? &match_pos : &curwin->w_cursor))
{
count++;
match_pos = pos;
match_pos.col--;
}
linep = ml_get(pos.lnum); 
}
}
else if ( linep[pos.col - 1] == '/'
&& linep[pos.col] == '*'
&& (pos.col == 1 || linep[pos.col - 2] != '*')
&& (int)pos.col < comment_col)
{
count++;
match_pos = pos;
match_pos.col--;
}
else if (linep[pos.col - 1] == '*' && linep[pos.col] == '/')
{
if (count > 0)
pos = match_pos;
else if (pos.col > 1 && linep[pos.col - 2] == '/'
&& (int)pos.col <= comment_col)
pos.col -= 2;
else if (ignore_cend)
continue;
else
return NULL;
return &pos;
}
}
continue;
}
if (cpo_match)
do_quotes = 0;
else if (do_quotes == -1)
{
at_start = do_quotes;
for (ptr = linep; *ptr; ++ptr)
{
if (ptr == linep + pos.col + backwards)
at_start = (do_quotes & 1);
if (*ptr == '"'
&& (ptr == linep || ptr[-1] != '\'' || ptr[1] != '\''))
++do_quotes;
if (*ptr == '\\' && ptr[1] != NUL)
++ptr;
}
do_quotes &= 1; 
if (!do_quotes)
{
inquote = FALSE;
if (ptr[-1] == '\\')
{
do_quotes = 1;
if (start_in_quotes == MAYBE)
{
inquote = TRUE;
start_in_quotes = TRUE;
}
else if (backwards)
inquote = TRUE;
}
if (pos.lnum > 1)
{
ptr = ml_get(pos.lnum - 1);
if (*ptr && *(ptr + STRLEN(ptr) - 1) == '\\')
{
do_quotes = 1;
if (start_in_quotes == MAYBE)
{
inquote = at_start;
if (inquote)
start_in_quotes = TRUE;
}
else if (!backwards)
inquote = TRUE;
}
linep = ml_get(pos.lnum);
}
}
}
if (start_in_quotes == MAYBE)
start_in_quotes = FALSE;
c = PTR2CHAR(linep + pos.col);
switch (c)
{
case NUL:
if (pos.col == 0 || linep[pos.col - 1] != '\\')
{
inquote = FALSE;
start_in_quotes = FALSE;
}
break;
case '"':
if (do_quotes)
{
int col;
for (col = pos.col - 1; col >= 0; --col)
if (linep[col] != '\\')
break;
if ((((int)pos.col - 1 - col) & 1) == 0)
{
inquote = !inquote;
start_in_quotes = FALSE;
}
}
break;
case '\'':
if (!cpo_match && initc != '\'' && findc != '\'')
{
if (backwards)
{
if (pos.col > 1)
{
if (linep[pos.col - 2] == '\'')
{
pos.col -= 2;
break;
}
else if (linep[pos.col - 2] == '\\' &&
pos.col > 2 && linep[pos.col - 3] == '\'')
{
pos.col -= 3;
break;
}
}
}
else if (linep[pos.col + 1]) 
{
if (linep[pos.col + 1] == '\\' &&
linep[pos.col + 2] && linep[pos.col + 3] == '\'')
{
pos.col += 3;
break;
}
else if (linep[pos.col + 2] == '\'')
{
pos.col += 2;
break;
}
}
}
default:
#if defined(FEAT_LISP)
if (curbuf->b_p_lisp
&& vim_strchr((char_u *)"(){}[]", c) != NULL
&& pos.col > 1
&& check_prevcol(linep, pos.col, '\\', NULL)
&& check_prevcol(linep, pos.col - 1, '#', NULL))
break;
#endif
if ((!inquote || start_in_quotes == TRUE)
&& (c == initc || c == findc))
{
int col, bslcnt = 0;
if (!cpo_bsl)
{
for (col = pos.col; check_prevcol(linep, col, '\\', &col);)
bslcnt++;
}
if (cpo_bsl || (bslcnt & 1) == match_escaped)
{
if (c == initc)
count++;
else
{
if (count == 0)
return &pos;
count--;
}
}
}
}
}
if (comment_dir == BACKWARD && count > 0)
{
pos = match_pos;
return &pos;
}
return (pos_T *)NULL; 
}
static int
check_linecomment(char_u *line)
{
char_u *p;
p = line;
#if defined(FEAT_LISP)
if (curbuf->b_p_lisp)
{
if (vim_strchr(p, ';') != NULL) 
{
int in_str = FALSE; 
p = line; 
while ((p = vim_strpbrk(p, (char_u *)"\";")) != NULL)
{
if (*p == '"')
{
if (in_str)
{
if (*(p - 1) != '\\') 
in_str = FALSE;
}
else if (p == line || ((p - line) >= 2
&& *(p - 1) != '\\' && *(p - 2) != '#'))
in_str = TRUE;
}
else if (!in_str && ((p - line) < 2
|| (*(p - 1) != '\\' && *(p - 2) != '#')))
break; 
++p;
}
}
else
p = NULL;
}
else
#endif
while ((p = vim_strchr(p, '/')) != NULL)
{
if (p[1] == '/' && (p == line || p[-1] != '*' || p[2] != '*'))
break;
++p;
}
if (p == NULL)
return MAXCOL;
return (int)(p - line);
}
void
showmatch(
int c) 
{
pos_T *lpos, save_cursor;
pos_T mpos;
colnr_T vcol;
long save_so;
long save_siso;
#if defined(CURSOR_SHAPE)
int save_state;
#endif
colnr_T save_dollar_vcol;
char_u *p;
long *so = curwin->w_p_so >= 0 ? &curwin->w_p_so : &p_so;
long *siso = curwin->w_p_siso >= 0 ? &curwin->w_p_siso : &p_siso;
for (p = curbuf->b_p_mps; *p != NUL; ++p)
{
#if defined(FEAT_RIGHTLEFT)
if (PTR2CHAR(p) == c && (curwin->w_p_rl ^ p_ri))
break;
#endif
p += mb_ptr2len(p) + 1;
if (PTR2CHAR(p) == c
#if defined(FEAT_RIGHTLEFT)
&& !(curwin->w_p_rl ^ p_ri)
#endif
)
break;
p += mb_ptr2len(p);
if (*p == NUL)
return;
}
if ((lpos = findmatch(NULL, NUL)) == NULL) 
vim_beep(BO_MATCH);
else if (lpos->lnum >= curwin->w_topline && lpos->lnum < curwin->w_botline)
{
if (!curwin->w_p_wrap)
getvcol(curwin, lpos, NULL, &vcol, NULL);
if (curwin->w_p_wrap || (vcol >= curwin->w_leftcol
&& vcol < curwin->w_leftcol + curwin->w_width))
{
mpos = *lpos; 
save_cursor = curwin->w_cursor;
save_so = *so;
save_siso = *siso;
if (dollar_vcol >= 0 && dollar_vcol == curwin->w_virtcol)
dollar_vcol = -1;
++curwin->w_virtcol; 
update_screen(VALID); 
save_dollar_vcol = dollar_vcol;
#if defined(CURSOR_SHAPE)
save_state = State;
State = SHOWMATCH;
ui_cursor_shape(); 
#endif
curwin->w_cursor = mpos; 
*so = 0; 
*siso = 0; 
showruler(FALSE);
setcursor();
cursor_on(); 
out_flush_cursor(TRUE, FALSE);
dollar_vcol = save_dollar_vcol;
if (vim_strchr(p_cpo, CPO_SHOWMATCH) != NULL)
ui_delay(p_mat * 100L + 8, TRUE);
else if (!char_avail())
ui_delay(p_mat * 100L + 9, FALSE);
curwin->w_cursor = save_cursor; 
*so = save_so;
*siso = save_siso;
#if defined(CURSOR_SHAPE)
State = save_state;
ui_cursor_shape(); 
#endif
}
}
}
int
findsent(int dir, long count)
{
pos_T pos, tpos;
int c;
int (*func)(pos_T *);
int startlnum;
int noskip = FALSE; 
int cpo_J;
int found_dot;
pos = curwin->w_cursor;
if (dir == FORWARD)
func = incl;
else
func = decl;
while (count--)
{
if (gchar_pos(&pos) == NUL)
{
do
if ((*func)(&pos) == -1)
break;
while (gchar_pos(&pos) == NUL);
if (dir == FORWARD)
goto found;
}
else if (dir == FORWARD && pos.col == 0 &&
startPS(pos.lnum, NUL, FALSE))
{
if (pos.lnum == curbuf->b_ml.ml_line_count)
return FAIL;
++pos.lnum;
goto found;
}
else if (dir == BACKWARD)
decl(&pos);
found_dot = FALSE;
while (c = gchar_pos(&pos), VIM_ISWHITE(c)
|| vim_strchr((char_u *)".!?)]\"'", c) != NULL)
{
tpos = pos;
if (decl(&tpos) == -1 || (LINEEMPTY(tpos.lnum) && dir == FORWARD))
break;
if (found_dot)
break;
if (vim_strchr((char_u *) ".!?", c) != NULL)
found_dot = TRUE;
if (vim_strchr((char_u *) ")]\"'", c) != NULL
&& vim_strchr((char_u *) ".!?)]\"'", gchar_pos(&tpos)) == NULL)
break;
decl(&pos);
}
startlnum = pos.lnum;
cpo_J = vim_strchr(p_cpo, CPO_ENDOFSENT) != NULL;
for (;;) 
{
c = gchar_pos(&pos);
if (c == NUL || (pos.col == 0 && startPS(pos.lnum, NUL, FALSE)))
{
if (dir == BACKWARD && pos.lnum != startlnum)
++pos.lnum;
break;
}
if (c == '.' || c == '!' || c == '?')
{
tpos = pos;
do
if ((c = inc(&tpos)) == -1)
break;
while (vim_strchr((char_u *)")]\"'", c = gchar_pos(&tpos))
!= NULL);
if (c == -1 || (!cpo_J && (c == ' ' || c == '\t')) || c == NUL
|| (cpo_J && (c == ' ' && inc(&tpos) >= 0
&& gchar_pos(&tpos) == ' ')))
{
pos = tpos;
if (gchar_pos(&pos) == NUL) 
inc(&pos);
break;
}
}
if ((*func)(&pos) == -1)
{
if (count)
return FAIL;
noskip = TRUE;
break;
}
}
found:
while (!noskip && ((c = gchar_pos(&pos)) == ' ' || c == '\t'))
if (incl(&pos) == -1)
break;
}
setpcmark();
curwin->w_cursor = pos;
return OK;
}
int
findpar(
int *pincl, 
int dir,
long count,
int what,
int both)
{
linenr_T curr;
int did_skip; 
int first; 
int posix = (vim_strchr(p_cpo, CPO_PARA) != NULL);
#if defined(FEAT_FOLDING)
linenr_T fold_first; 
linenr_T fold_last; 
int fold_skipped; 
#endif
curr = curwin->w_cursor.lnum;
while (count--)
{
did_skip = FALSE;
for (first = TRUE; ; first = FALSE)
{
if (*ml_get(curr) != NUL)
did_skip = TRUE;
#if defined(FEAT_FOLDING)
fold_skipped = FALSE;
if (first && hasFolding(curr, &fold_first, &fold_last))
{
curr = ((dir > 0) ? fold_last : fold_first) + dir;
fold_skipped = TRUE;
}
#endif
if (!first && did_skip && (startPS(curr, what, both)
|| (posix && what == NUL && *ml_get(curr) == '{')))
break;
#if defined(FEAT_FOLDING)
if (fold_skipped)
curr -= dir;
#endif
if ((curr += dir) < 1 || curr > curbuf->b_ml.ml_line_count)
{
if (count)
return FALSE;
curr -= dir;
break;
}
}
}
setpcmark();
if (both && *ml_get(curr) == '}') 
++curr;
curwin->w_cursor.lnum = curr;
if (curr == curbuf->b_ml.ml_line_count && what != '}')
{
char_u *line = ml_get(curr);
if ((curwin->w_cursor.col = (colnr_T)STRLEN(line)) != 0)
{
--curwin->w_cursor.col;
curwin->w_cursor.col -=
(*mb_head_off)(line, line + curwin->w_cursor.col);
*pincl = TRUE;
}
}
else
curwin->w_cursor.col = 0;
return TRUE;
}
static int
inmacro(char_u *opt, char_u *s)
{
char_u *macro;
for (macro = opt; macro[0]; ++macro)
{
if ( (macro[0] == s[0]
|| (macro[0] == ' '
&& (s[0] == NUL || s[0] == ' ')))
&& (macro[1] == s[1]
|| ((macro[1] == NUL || macro[1] == ' ')
&& (s[0] == NUL || s[1] == NUL || s[1] == ' '))))
break;
++macro;
if (macro[0] == NUL)
break;
}
return (macro[0] != NUL);
}
int
startPS(linenr_T lnum, int para, int both)
{
char_u *s;
s = ml_get(lnum);
if (*s == para || *s == '\f' || (both && *s == '}'))
return TRUE;
if (*s == '.' && (inmacro(p_sections, s + 1) ||
(!para && inmacro(p_para, s + 1))))
return TRUE;
return FALSE;
}
static int cls_bigword; 
static int
cls(void)
{
int c;
c = gchar_cursor();
if (c == ' ' || c == '\t' || c == NUL)
return 0;
if (enc_dbcs != 0 && c > 0xFF)
{
if (enc_dbcs == DBCS_KOR && cls_bigword)
return 1;
return dbcs_class(((unsigned)c >> 8), (c & 0xFF));
}
if (enc_utf8)
{
c = utf_class(c);
if (c != 0 && cls_bigword)
return 1;
return c;
}
if (cls_bigword)
return 1;
if (vim_iswordc(c))
return 2;
return 1;
}
int
fwd_word(
long count,
int bigword, 
int eol)
{
int sclass; 
int i;
int last_line;
curwin->w_cursor.coladd = 0;
cls_bigword = bigword;
while (--count >= 0)
{
#if defined(FEAT_FOLDING)
if (hasFolding(curwin->w_cursor.lnum, NULL, &curwin->w_cursor.lnum))
coladvance((colnr_T)MAXCOL);
#endif
sclass = cls();
last_line = (curwin->w_cursor.lnum == curbuf->b_ml.ml_line_count);
i = inc_cursor();
if (i == -1 || (i >= 1 && last_line)) 
return FAIL;
if (i >= 1 && eol && count == 0) 
return OK;
if (sclass != 0)
while (cls() == sclass)
{
i = inc_cursor();
if (i == -1 || (i >= 1 && eol && count == 0))
return OK;
}
while (cls() == 0)
{
if (curwin->w_cursor.col == 0 && *ml_get_curline() == NUL)
break;
i = inc_cursor();
if (i == -1 || (i >= 1 && eol && count == 0))
return OK;
}
}
return OK;
}
int
bck_word(long count, int bigword, int stop)
{
int sclass; 
curwin->w_cursor.coladd = 0;
cls_bigword = bigword;
while (--count >= 0)
{
#if defined(FEAT_FOLDING)
if (hasFolding(curwin->w_cursor.lnum, &curwin->w_cursor.lnum, NULL))
curwin->w_cursor.col = 0;
#endif
sclass = cls();
if (dec_cursor() == -1) 
return FAIL;
if (!stop || sclass == cls() || sclass == 0)
{
while (cls() == 0)
{
if (curwin->w_cursor.col == 0
&& LINEEMPTY(curwin->w_cursor.lnum))
goto finished;
if (dec_cursor() == -1) 
return OK;
}
if (skip_chars(cls(), BACKWARD))
return OK;
}
inc_cursor(); 
finished:
stop = FALSE;
}
return OK;
}
int
end_word(
long count,
int bigword,
int stop,
int empty)
{
int sclass; 
curwin->w_cursor.coladd = 0;
cls_bigword = bigword;
while (--count >= 0)
{
#if defined(FEAT_FOLDING)
if (hasFolding(curwin->w_cursor.lnum, NULL, &curwin->w_cursor.lnum))
coladvance((colnr_T)MAXCOL);
#endif
sclass = cls();
if (inc_cursor() == -1)
return FAIL;
if (cls() == sclass && sclass != 0)
{
if (skip_chars(sclass, FORWARD))
return FAIL;
}
else if (!stop || sclass == 0)
{
while (cls() == 0)
{
if (empty && curwin->w_cursor.col == 0
&& LINEEMPTY(curwin->w_cursor.lnum))
goto finished;
if (inc_cursor() == -1) 
return FAIL;
}
if (skip_chars(cls(), FORWARD))
return FAIL;
}
dec_cursor(); 
finished:
stop = FALSE; 
}
return OK;
}
int
bckend_word(
long count,
int bigword, 
int eol) 
{
int sclass; 
int i;
curwin->w_cursor.coladd = 0;
cls_bigword = bigword;
while (--count >= 0)
{
sclass = cls();
if ((i = dec_cursor()) == -1)
return FAIL;
if (eol && i == 1)
return OK;
if (sclass != 0)
{
while (cls() == sclass)
if ((i = dec_cursor()) == -1 || (eol && i == 1))
return OK;
}
while (cls() == 0)
{
if (curwin->w_cursor.col == 0 && LINEEMPTY(curwin->w_cursor.lnum))
break;
if ((i = dec_cursor()) == -1 || (eol && i == 1))
return OK;
}
}
return OK;
}
static int
skip_chars(int cclass, int dir)
{
while (cls() == cclass)
if ((dir == FORWARD ? inc_cursor() : dec_cursor()) == -1)
return TRUE;
return FALSE;
}
#if defined(FEAT_TEXTOBJ)
static void
back_in_line(void)
{
int sclass; 
sclass = cls();
for (;;)
{
if (curwin->w_cursor.col == 0) 
break;
dec_cursor();
if (cls() != sclass) 
{
inc_cursor();
break;
}
}
}
static void
find_first_blank(pos_T *posp)
{
int c;
while (decl(posp) != -1)
{
c = gchar_pos(posp);
if (!VIM_ISWHITE(c))
{
incl(posp);
break;
}
}
}
static void
findsent_forward(
long count,
int at_start_sent) 
{
while (count--)
{
findsent(FORWARD, 1L);
if (at_start_sent)
find_first_blank(&curwin->w_cursor);
if (count == 0 || at_start_sent)
decl(&curwin->w_cursor);
at_start_sent = !at_start_sent;
}
}
int
current_word(
oparg_T *oap,
long count,
int include, 
int bigword) 
{
pos_T start_pos;
pos_T pos;
int inclusive = TRUE;
int include_white = FALSE;
cls_bigword = bigword;
CLEAR_POS(&start_pos);
if (VIsual_active && *p_sel == 'e' && LT_POS(VIsual, curwin->w_cursor))
dec_cursor();
if (!VIsual_active || EQUAL_POS(curwin->w_cursor, VIsual))
{
back_in_line();
start_pos = curwin->w_cursor;
if ((cls() == 0) == include)
{
if (end_word(1L, bigword, TRUE, TRUE) == FAIL)
return FAIL;
}
else
{
fwd_word(1L, bigword, TRUE);
if (curwin->w_cursor.col == 0)
decl(&curwin->w_cursor);
else
oneleft();
if (include)
include_white = TRUE;
}
if (VIsual_active)
{
VIsual = start_pos;
redraw_curbuf_later(INVERTED); 
}
else
{
oap->start = start_pos;
oap->motion_type = MCHAR;
}
--count;
}
while (count > 0)
{
inclusive = TRUE;
if (VIsual_active && LT_POS(curwin->w_cursor, VIsual))
{
if (decl(&curwin->w_cursor) == -1)
return FAIL;
if (include != (cls() != 0))
{
if (bck_word(1L, bigword, TRUE) == FAIL)
return FAIL;
}
else
{
if (bckend_word(1L, bigword, TRUE) == FAIL)
return FAIL;
(void)incl(&curwin->w_cursor);
}
}
else
{
if (incl(&curwin->w_cursor) == -1)
return FAIL;
if (include != (cls() == 0))
{
if (fwd_word(1L, bigword, TRUE) == FAIL && count > 1)
return FAIL;
if (oneleft() == FAIL)
inclusive = FALSE;
}
else
{
if (end_word(1L, bigword, TRUE, TRUE) == FAIL)
return FAIL;
}
}
--count;
}
if (include_white && (cls() != 0
|| (curwin->w_cursor.col == 0 && !inclusive)))
{
pos = curwin->w_cursor; 
curwin->w_cursor = start_pos;
if (oneleft() == OK)
{
back_in_line();
if (cls() == 0 && curwin->w_cursor.col > 0)
{
if (VIsual_active)
VIsual = curwin->w_cursor;
else
oap->start = curwin->w_cursor;
}
}
curwin->w_cursor = pos; 
}
if (VIsual_active)
{
if (*p_sel == 'e' && inclusive && LTOREQ_POS(VIsual, curwin->w_cursor))
inc_cursor();
if (VIsual_mode == 'V')
{
VIsual_mode = 'v';
redraw_cmdline = TRUE; 
}
}
else
oap->inclusive = inclusive;
return OK;
}
int
current_sent(oparg_T *oap, long count, int include)
{
pos_T start_pos;
pos_T pos;
int start_blank;
int c;
int at_start_sent;
long ncount;
start_pos = curwin->w_cursor;
pos = start_pos;
findsent(FORWARD, 1L); 
if (VIsual_active && !EQUAL_POS(start_pos, VIsual))
{
extend:
if (LT_POS(start_pos, VIsual))
{
at_start_sent = TRUE;
decl(&pos);
while (LT_POS(pos, curwin->w_cursor))
{
c = gchar_pos(&pos);
if (!VIM_ISWHITE(c))
{
at_start_sent = FALSE;
break;
}
incl(&pos);
}
if (!at_start_sent)
{
findsent(BACKWARD, 1L);
if (EQUAL_POS(curwin->w_cursor, start_pos))
at_start_sent = TRUE; 
else
findsent(FORWARD, 1L);
}
if (include) 
count *= 2;
while (count--)
{
if (at_start_sent)
find_first_blank(&curwin->w_cursor);
c = gchar_cursor();
if (!at_start_sent || (!include && !VIM_ISWHITE(c)))
findsent(BACKWARD, 1L);
at_start_sent = !at_start_sent;
}
}
else
{
incl(&pos);
at_start_sent = TRUE;
if (!EQUAL_POS(pos, curwin->w_cursor))
{
at_start_sent = FALSE;
while (LT_POS(pos, curwin->w_cursor))
{
c = gchar_pos(&pos);
if (!VIM_ISWHITE(c))
{
at_start_sent = TRUE;
break;
}
incl(&pos);
}
if (at_start_sent) 
findsent(BACKWARD, 1L);
else 
curwin->w_cursor = start_pos;
}
if (include) 
count *= 2;
findsent_forward(count, at_start_sent);
if (*p_sel == 'e')
++curwin->w_cursor.col;
}
return OK;
}
while (c = gchar_pos(&pos), VIM_ISWHITE(c)) 
incl(&pos);
if (EQUAL_POS(pos, curwin->w_cursor))
{
start_blank = TRUE;
find_first_blank(&start_pos); 
}
else
{
start_blank = FALSE;
findsent(BACKWARD, 1L);
start_pos = curwin->w_cursor;
}
if (include)
ncount = count * 2;
else
{
ncount = count;
if (start_blank)
--ncount;
}
if (ncount > 0)
findsent_forward(ncount, TRUE);
else
decl(&curwin->w_cursor);
if (include)
{
if (start_blank)
{
find_first_blank(&curwin->w_cursor);
c = gchar_pos(&curwin->w_cursor); 
if (VIM_ISWHITE(c))
decl(&curwin->w_cursor);
}
else if (c = gchar_cursor(), !VIM_ISWHITE(c))
find_first_blank(&start_pos);
}
if (VIsual_active)
{
if (EQUAL_POS(start_pos, curwin->w_cursor))
goto extend;
if (*p_sel == 'e')
++curwin->w_cursor.col;
VIsual = start_pos;
VIsual_mode = 'v';
redraw_cmdline = TRUE; 
redraw_curbuf_later(INVERTED); 
}
else
{
if (incl(&curwin->w_cursor) == -1)
oap->inclusive = TRUE;
else
oap->inclusive = FALSE;
oap->start = start_pos;
oap->motion_type = MCHAR;
}
return OK;
}
int
current_block(
oparg_T *oap,
long count,
int include, 
int what, 
int other) 
{
pos_T old_pos;
pos_T *pos = NULL;
pos_T start_pos;
pos_T *end_pos;
pos_T old_start, old_end;
char_u *save_cpo;
int sol = FALSE; 
old_pos = curwin->w_cursor;
old_end = curwin->w_cursor; 
old_start = old_end;
if (!VIsual_active || EQUAL_POS(VIsual, curwin->w_cursor))
{
setpcmark();
if (what == '{') 
while (inindent(1))
if (inc_cursor() != 0)
break;
if (gchar_cursor() == what)
++curwin->w_cursor.col;
}
else if (LT_POS(VIsual, curwin->w_cursor))
{
old_start = VIsual;
curwin->w_cursor = VIsual; 
}
else
old_end = VIsual;
save_cpo = p_cpo;
p_cpo = (char_u *)(vim_strchr(p_cpo, CPO_MATCHBSL) != NULL ? "%M" : "%");
while (count-- > 0)
{
if ((pos = findmatch(NULL, what)) == NULL)
break;
curwin->w_cursor = *pos;
start_pos = *pos; 
}
p_cpo = save_cpo;
if (pos == NULL || (end_pos = findmatch(NULL, other)) == NULL)
{
curwin->w_cursor = old_pos;
return FAIL;
}
curwin->w_cursor = *end_pos;
while (!include)
{
incl(&start_pos);
sol = (curwin->w_cursor.col == 0);
decl(&curwin->w_cursor);
while (inindent(1))
{
sol = TRUE;
if (decl(&curwin->w_cursor) != 0)
break;
}
if (!LT_POS(start_pos, old_start) && !LT_POS(old_end, curwin->w_cursor)
&& VIsual_active)
{
curwin->w_cursor = old_start;
decl(&curwin->w_cursor);
if ((pos = findmatch(NULL, what)) == NULL)
{
curwin->w_cursor = old_pos;
return FAIL;
}
start_pos = *pos;
curwin->w_cursor = *pos;
if ((end_pos = findmatch(NULL, other)) == NULL)
{
curwin->w_cursor = old_pos;
return FAIL;
}
curwin->w_cursor = *end_pos;
}
else
break;
}
if (VIsual_active)
{
if (*p_sel == 'e')
inc(&curwin->w_cursor);
if (sol && gchar_cursor() != NUL)
inc(&curwin->w_cursor); 
VIsual = start_pos;
VIsual_mode = 'v';
redraw_curbuf_later(INVERTED); 
showmode();
}
else
{
oap->start = start_pos;
oap->motion_type = MCHAR;
oap->inclusive = FALSE;
if (sol)
incl(&curwin->w_cursor);
else if (LTOREQ_POS(start_pos, curwin->w_cursor))
oap->inclusive = TRUE;
else
curwin->w_cursor = start_pos;
}
return OK;
}
static int
in_html_tag(
int end_tag)
{
char_u *line = ml_get_curline();
char_u *p;
int c;
int lc = NUL;
pos_T pos;
if (enc_dbcs)
{
char_u *lp = NULL;
for (p = line; p < line + curwin->w_cursor.col; MB_PTR_ADV(p))
if (*p == '>' || *p == '<')
{
lc = *p;
lp = p;
}
if (*p != '<') 
{
if (lc != '<')
return FALSE;
p = lp;
}
}
else
{
for (p = line + curwin->w_cursor.col; p > line; )
{
if (*p == '<') 
break;
MB_PTR_BACK(line, p);
if (*p == '>') 
break;
}
if (*p != '<')
return FALSE;
}
pos.lnum = curwin->w_cursor.lnum;
pos.col = (colnr_T)(p - line);
MB_PTR_ADV(p);
if (end_tag)
return *p == '/';
if (*p == '/')
return FALSE;
for (;;)
{
if (inc(&pos) < 0)
return FALSE;
c = *ml_get_pos(&pos);
if (c == '>')
break;
lc = c;
}
return lc != '/';
}
int
current_tagblock(
oparg_T *oap,
long count_arg,
int include) 
{
long count = count_arg;
long n;
pos_T old_pos;
pos_T start_pos;
pos_T end_pos;
pos_T old_start, old_end;
char_u *spat, *epat;
char_u *p;
char_u *cp;
int len;
int r;
int do_include = include;
int save_p_ws = p_ws;
int retval = FAIL;
int is_inclusive = TRUE;
p_ws = FALSE;
old_pos = curwin->w_cursor;
old_end = curwin->w_cursor; 
old_start = old_end;
if (!VIsual_active || *p_sel == 'e')
decl(&old_end); 
if (!VIsual_active || EQUAL_POS(VIsual, curwin->w_cursor))
{
setpcmark();
while (inindent(1))
if (inc_cursor() != 0)
break;
if (in_html_tag(FALSE))
{
while (*ml_get_cursor() != '>')
if (inc_cursor() < 0)
break;
}
else if (in_html_tag(TRUE))
{
while (*ml_get_cursor() != '<')
if (dec_cursor() < 0)
break;
dec_cursor();
old_end = curwin->w_cursor;
}
}
else if (LT_POS(VIsual, curwin->w_cursor))
{
old_start = VIsual;
curwin->w_cursor = VIsual; 
}
else
old_end = VIsual;
again:
for (n = 0; n < count; ++n)
{
if (do_searchpair((char_u *)"<[^ \t>/!]\\+\\%(\\_s\\_[^>]\\{-}[^/]>\\|$\\|\\_s\\=>\\)",
(char_u *)"",
(char_u *)"</[^>]*>", BACKWARD, NULL, 0,
NULL, (linenr_T)0, 0L) <= 0)
{
curwin->w_cursor = old_pos;
goto theend;
}
}
start_pos = curwin->w_cursor;
inc_cursor();
p = ml_get_cursor();
for (cp = p; *cp != NUL && *cp != '>' && !VIM_ISWHITE(*cp); MB_PTR_ADV(cp))
;
len = (int)(cp - p);
if (len == 0)
{
curwin->w_cursor = old_pos;
goto theend;
}
spat = alloc(len + 31);
epat = alloc(len + 9);
if (spat == NULL || epat == NULL)
{
vim_free(spat);
vim_free(epat);
curwin->w_cursor = old_pos;
goto theend;
}
sprintf((char *)spat, "<%.*s\\>\\%%(\\s\\_[^>]\\{-}[^/]>\\|>\\)\\c", len, p);
sprintf((char *)epat, "</%.*s>\\c", len, p);
r = do_searchpair(spat, (char_u *)"", epat, FORWARD, NULL,
0, NULL, (linenr_T)0, 0L);
vim_free(spat);
vim_free(epat);
if (r < 1 || LT_POS(curwin->w_cursor, old_end))
{
count = 1;
curwin->w_cursor = start_pos;
goto again;
}
if (do_include)
{
while (*ml_get_cursor() != '>')
if (inc_cursor() < 0)
break;
}
else
{
char_u *c = ml_get_cursor();
if (*c == '<' && !VIsual_active && curwin->w_cursor.col == 0)
is_inclusive = FALSE;
else if (*c == '<')
dec_cursor();
}
end_pos = curwin->w_cursor;
if (!do_include)
{
curwin->w_cursor = start_pos;
while (inc_cursor() >= 0)
if (*ml_get_cursor() == '>')
{
inc_cursor();
start_pos = curwin->w_cursor;
break;
}
curwin->w_cursor = end_pos;
if (VIsual_active && EQUAL_POS(start_pos, old_start)
&& EQUAL_POS(end_pos, old_end))
{
do_include = TRUE;
curwin->w_cursor = old_start;
count = count_arg;
goto again;
}
}
if (VIsual_active)
{
if (LT_POS(end_pos, start_pos))
curwin->w_cursor = start_pos;
else if (*p_sel == 'e')
inc_cursor();
VIsual = start_pos;
VIsual_mode = 'v';
redraw_curbuf_later(INVERTED); 
showmode();
}
else
{
oap->start = start_pos;
oap->motion_type = MCHAR;
if (LT_POS(end_pos, start_pos))
{
curwin->w_cursor = start_pos;
oap->inclusive = FALSE;
}
else
oap->inclusive = is_inclusive;
}
retval = OK;
theend:
p_ws = save_p_ws;
return retval;
}
int
current_par(
oparg_T *oap,
long count,
int include, 
int type) 
{
linenr_T start_lnum;
linenr_T end_lnum;
int white_in_front;
int dir;
int start_is_white;
int prev_start_is_white;
int retval = OK;
int do_white = FALSE;
int t;
int i;
if (type == 'S') 
return FAIL;
start_lnum = curwin->w_cursor.lnum;
if (VIsual_active && start_lnum != VIsual.lnum)
{
extend:
if (start_lnum < VIsual.lnum)
dir = BACKWARD;
else
dir = FORWARD;
for (i = count; --i >= 0; )
{
if (start_lnum ==
(dir == BACKWARD ? 1 : curbuf->b_ml.ml_line_count))
{
retval = FAIL;
break;
}
prev_start_is_white = -1;
for (t = 0; t < 2; ++t)
{
start_lnum += dir;
start_is_white = linewhite(start_lnum);
if (prev_start_is_white == start_is_white)
{
start_lnum -= dir;
break;
}
for (;;)
{
if (start_lnum == (dir == BACKWARD
? 1 : curbuf->b_ml.ml_line_count))
break;
if (start_is_white != linewhite(start_lnum + dir)
|| (!start_is_white
&& startPS(start_lnum + (dir > 0
? 1 : 0), 0, 0)))
break;
start_lnum += dir;
}
if (!include)
break;
if (start_lnum == (dir == BACKWARD
? 1 : curbuf->b_ml.ml_line_count))
break;
prev_start_is_white = start_is_white;
}
}
curwin->w_cursor.lnum = start_lnum;
curwin->w_cursor.col = 0;
return retval;
}
white_in_front = linewhite(start_lnum);
while (start_lnum > 1)
{
if (white_in_front) 
{
if (!linewhite(start_lnum - 1))
break;
}
else 
{
if (linewhite(start_lnum - 1) || startPS(start_lnum, 0, 0))
break;
}
--start_lnum;
}
end_lnum = start_lnum;
while (end_lnum <= curbuf->b_ml.ml_line_count && linewhite(end_lnum))
++end_lnum;
--end_lnum;
i = count;
if (!include && white_in_front)
--i;
while (i--)
{
if (end_lnum == curbuf->b_ml.ml_line_count)
return FAIL;
if (!include)
do_white = linewhite(end_lnum + 1);
if (include || !do_white)
{
++end_lnum;
while (end_lnum < curbuf->b_ml.ml_line_count
&& !linewhite(end_lnum + 1)
&& !startPS(end_lnum + 1, 0, 0))
++end_lnum;
}
if (i == 0 && white_in_front && include)
break;
if (include || do_white)
while (end_lnum < curbuf->b_ml.ml_line_count
&& linewhite(end_lnum + 1))
++end_lnum;
}
if (!white_in_front && !linewhite(end_lnum) && include)
while (start_lnum > 1 && linewhite(start_lnum - 1))
--start_lnum;
if (VIsual_active)
{
if (VIsual_mode == 'V' && start_lnum == curwin->w_cursor.lnum)
goto extend;
if (VIsual.lnum != start_lnum)
{
VIsual.lnum = start_lnum;
VIsual.col = 0;
}
VIsual_mode = 'V';
redraw_curbuf_later(INVERTED); 
showmode();
}
else
{
oap->start.lnum = start_lnum;
oap->start.col = 0;
oap->motion_type = MLINE;
}
curwin->w_cursor.lnum = end_lnum;
curwin->w_cursor.col = 0;
return OK;
}
static int
find_next_quote(
char_u *line,
int col,
int quotechar,
char_u *escape) 
{
int c;
for (;;)
{
c = line[col];
if (c == NUL)
return -1;
else if (escape != NULL && vim_strchr(escape, c))
++col;
else if (c == quotechar)
break;
if (has_mbyte)
col += (*mb_ptr2len)(line + col);
else
++col;
}
return col;
}
static int
find_prev_quote(
char_u *line,
int col_start,
int quotechar,
char_u *escape) 
{
int n;
while (col_start > 0)
{
--col_start;
col_start -= (*mb_head_off)(line, line + col_start);
n = 0;
if (escape != NULL)
while (col_start - n > 0 && vim_strchr(escape,
line[col_start - n - 1]) != NULL)
++n;
if (n & 1)
col_start -= n; 
else if (line[col_start] == quotechar)
break;
}
return col_start;
}
int
current_quote(
oparg_T *oap,
long count,
int include, 
int quotechar) 
{
char_u *line = ml_get_curline();
int col_end;
int col_start = curwin->w_cursor.col;
int inclusive = FALSE;
int vis_empty = TRUE; 
int vis_bef_curs = FALSE; 
int did_exclusive_adj = FALSE; 
int inside_quotes = FALSE; 
int selected_quote = FALSE; 
int i;
int restore_vis_bef = FALSE; 
if (VIsual_active)
{
if (VIsual.lnum != curwin->w_cursor.lnum)
return FALSE;
vis_bef_curs = LT_POS(VIsual, curwin->w_cursor);
vis_empty = EQUAL_POS(VIsual, curwin->w_cursor);
if (*p_sel == 'e')
{
if (vis_bef_curs)
{
dec_cursor();
did_exclusive_adj = TRUE;
}
else if (!vis_empty)
{
dec(&VIsual);
did_exclusive_adj = TRUE;
}
vis_empty = EQUAL_POS(VIsual, curwin->w_cursor);
if (!vis_bef_curs && !vis_empty)
{
pos_T t = curwin->w_cursor;
curwin->w_cursor = VIsual;
VIsual = t;
vis_bef_curs = TRUE;
restore_vis_bef = TRUE;
}
}
}
if (!vis_empty)
{
if (vis_bef_curs)
{
inside_quotes = VIsual.col > 0
&& line[VIsual.col - 1] == quotechar
&& line[curwin->w_cursor.col] != NUL
&& line[curwin->w_cursor.col + 1] == quotechar;
i = VIsual.col;
col_end = curwin->w_cursor.col;
}
else
{
inside_quotes = curwin->w_cursor.col > 0
&& line[curwin->w_cursor.col - 1] == quotechar
&& line[VIsual.col] != NUL
&& line[VIsual.col + 1] == quotechar;
i = curwin->w_cursor.col;
col_end = VIsual.col;
}
while (i <= col_end)
if (line[i++] == quotechar)
{
selected_quote = TRUE;
break;
}
}
if (!vis_empty && line[col_start] == quotechar)
{
if (vis_bef_curs)
{
col_start = find_next_quote(line, col_start + 1, quotechar, NULL);
if (col_start < 0)
goto abort_search;
col_end = find_next_quote(line, col_start + 1, quotechar,
curbuf->b_p_qe);
if (col_end < 0)
{
col_end = col_start;
col_start = curwin->w_cursor.col;
}
}
else
{
col_end = find_prev_quote(line, col_start, quotechar, NULL);
if (line[col_end] != quotechar)
goto abort_search;
col_start = find_prev_quote(line, col_end, quotechar,
curbuf->b_p_qe);
if (line[col_start] != quotechar)
{
col_start = col_end;
col_end = curwin->w_cursor.col;
}
}
}
else
if (line[col_start] == quotechar || !vis_empty)
{
int first_col = col_start;
if (!vis_empty)
{
if (vis_bef_curs)
first_col = find_next_quote(line, col_start, quotechar, NULL);
else
first_col = find_prev_quote(line, col_start, quotechar, NULL);
}
col_start = 0;
for (;;)
{
col_start = find_next_quote(line, col_start, quotechar, NULL);
if (col_start < 0 || col_start > first_col)
goto abort_search;
col_end = find_next_quote(line, col_start + 1, quotechar,
curbuf->b_p_qe);
if (col_end < 0)
goto abort_search;
if (col_start <= first_col && first_col <= col_end)
break;
col_start = col_end + 1;
}
}
else
{
col_start = find_prev_quote(line, col_start, quotechar, curbuf->b_p_qe);
if (line[col_start] != quotechar)
{
col_start = find_next_quote(line, col_start, quotechar, NULL);
if (col_start < 0)
goto abort_search;
}
col_end = find_next_quote(line, col_start + 1, quotechar,
curbuf->b_p_qe);
if (col_end < 0)
goto abort_search;
}
if (include)
{
if (VIM_ISWHITE(line[col_end + 1]))
while (VIM_ISWHITE(line[col_end + 1]))
++col_end;
else
while (col_start > 0 && VIM_ISWHITE(line[col_start - 1]))
--col_start;
}
if (!include && count < 2 && (vis_empty || !inside_quotes))
++col_start;
curwin->w_cursor.col = col_start;
if (VIsual_active)
{
if (vis_empty
|| (vis_bef_curs
&& !selected_quote
&& (inside_quotes
|| (line[VIsual.col] != quotechar
&& (VIsual.col == 0
|| line[VIsual.col - 1] != quotechar)))))
{
VIsual = curwin->w_cursor;
redraw_curbuf_later(INVERTED);
}
}
else
{
oap->start = curwin->w_cursor;
oap->motion_type = MCHAR;
}
curwin->w_cursor.col = col_end;
if ((include || count > 1 
|| (!vis_empty && inside_quotes)
) && inc_cursor() == 2)
inclusive = TRUE;
if (VIsual_active)
{
if (vis_empty || vis_bef_curs)
{
if (*p_sel != 'e')
dec_cursor();
}
else
{
if (inside_quotes
|| (!selected_quote
&& line[VIsual.col] != quotechar
&& (line[VIsual.col] == NUL
|| line[VIsual.col + 1] != quotechar)))
{
dec_cursor();
VIsual = curwin->w_cursor;
}
curwin->w_cursor.col = col_start;
}
if (VIsual_mode == 'V')
{
VIsual_mode = 'v';
redraw_cmdline = TRUE; 
}
}
else
{
oap->inclusive = inclusive;
}
return OK;
abort_search:
if (VIsual_active && *p_sel == 'e')
{
if (did_exclusive_adj)
inc_cursor();
if (restore_vis_bef)
{
pos_T t = curwin->w_cursor;
curwin->w_cursor = VIsual;
VIsual = t;
}
}
return FALSE;
}
#endif 
static int
is_zero_width(char_u *pattern, int move, pos_T *cur, int direction)
{
regmmatch_T regmatch;
int nmatched = 0;
int result = -1;
pos_T pos;
int called_emsg_before = called_emsg;
int flag = 0;
if (pattern == NULL)
pattern = spats[last_idx].pat;
if (search_regcomp(pattern, RE_SEARCH, RE_SEARCH,
SEARCH_KEEP, &regmatch) == FAIL)
return -1;
regmatch.startpos[0].col = -1;
if (move)
{
CLEAR_POS(&pos);
}
else
{
pos = *cur;
flag = SEARCH_START;
}
if (searchit(curwin, curbuf, &pos, NULL, direction, pattern, 1,
SEARCH_KEEP + flag, RE_SEARCH, NULL) != FAIL)
{
do
{
regmatch.startpos[0].col++;
nmatched = vim_regexec_multi(&regmatch, curwin, curbuf,
pos.lnum, regmatch.startpos[0].col, NULL, NULL);
if (nmatched != 0)
break;
} while (direction == FORWARD ? regmatch.startpos[0].col < pos.col
: regmatch.startpos[0].col > pos.col);
if (called_emsg == called_emsg_before)
{
result = (nmatched != 0
&& regmatch.startpos[0].lnum == regmatch.endpos[0].lnum
&& regmatch.startpos[0].col == regmatch.endpos[0].col);
}
}
vim_regfree(regmatch.regprog);
return result;
}
int
current_search(
long count,
int forward) 
{
pos_T start_pos; 
pos_T end_pos; 
pos_T orig_pos; 
pos_T pos; 
int i;
int dir;
int result; 
char_u old_p_ws = p_ws;
int flags = 0;
pos_T save_VIsual = VIsual;
int zero_width;
if (VIsual_active && *p_sel == 'e' && LT_POS(VIsual, curwin->w_cursor))
dec_cursor();
orig_pos = pos = curwin->w_cursor;
if (VIsual_active)
{
if (forward)
incl(&pos);
else
decl(&pos);
}
zero_width = is_zero_width(spats[last_idx].pat, TRUE, &curwin->w_cursor,
FORWARD);
if (zero_width == -1)
return FAIL; 
for (i = 0; i < 2; i++)
{
if (forward)
dir = i;
else
dir = !i;
flags = 0;
if (!dir && !zero_width)
flags = SEARCH_END;
end_pos = pos;
if (i == 0)
p_ws = FALSE;
result = searchit(curwin, curbuf, &pos, &end_pos,
(dir ? FORWARD : BACKWARD),
spats[last_idx].pat, (long) (i ? count : 1),
SEARCH_KEEP | flags, RE_SEARCH, NULL);
p_ws = old_p_ws;
if (i == 1 && !result) 
{
curwin->w_cursor = orig_pos;
if (VIsual_active)
VIsual = save_VIsual;
return FAIL;
}
else if (i == 0 && !result)
{
if (forward)
{
CLEAR_POS(&pos);
}
else
{
pos.lnum = curwin->w_buffer->b_ml.ml_line_count;
pos.col = (colnr_T)STRLEN(
ml_get(curwin->w_buffer->b_ml.ml_line_count));
}
}
}
start_pos = pos;
if (!VIsual_active)
VIsual = start_pos;
curwin->w_cursor = end_pos;
if (LT_POS(VIsual, end_pos) && forward)
dec_cursor();
else if (VIsual_active && LT_POS(curwin->w_cursor, VIsual))
curwin->w_cursor = pos; 
VIsual_active = TRUE;
VIsual_mode = 'v';
if (*p_sel == 'e')
{
if (forward && LTOREQ_POS(VIsual, curwin->w_cursor))
inc_cursor();
else if (!forward && LTOREQ_POS(curwin->w_cursor, VIsual))
inc(&VIsual);
}
#if defined(FEAT_FOLDING)
if (fdo_flags & FDO_SEARCH && KeyTyped)
foldOpenCursor();
#endif
may_start_select('c');
setmouse();
#if defined(FEAT_CLIPBOARD)
clip_star.vmode = NUL;
#endif
redraw_curbuf_later(INVERTED);
showmode();
return OK;
}
#if defined(FEAT_LISP) || defined(FEAT_CINDENT) || defined(FEAT_TEXTOBJ) || defined(PROTO)
int
linewhite(linenr_T lnum)
{
char_u *p;
p = skipwhite(ml_get(lnum));
return (*p == NUL);
}
#endif
static void
search_stat(
int dirc,
pos_T *pos,
int show_top_bot_msg,
char_u *msgbuf,
int recompute)
{
int save_ws = p_ws;
int wraparound = FALSE;
pos_T p = (*pos);
static pos_T lastpos = {0, 0, 0};
static int cur = 0;
static int cnt = 0;
static int chgtick = 0;
static char_u *lastpat = NULL;
static buf_T *lbuf = NULL;
#if defined(FEAT_RELTIME)
proftime_T start;
#endif
#define OUT_OF_TIME 999
wraparound = ((dirc == '?' && LT_POS(lastpos, p))
|| (dirc == '/' && LT_POS(p, lastpos)));
if (!(chgtick == CHANGEDTICK(curbuf)
&& MB_STRNICMP(lastpat, spats[last_idx].pat, STRLEN(lastpat)) == 0
&& STRLEN(lastpat) == STRLEN(spats[last_idx].pat)
&& EQUAL_POS(lastpos, curwin->w_cursor)
&& lbuf == curbuf) || wraparound || cur < 0 || cur > 99 || recompute)
{
cur = 0;
cnt = 0;
CLEAR_POS(&lastpos);
lbuf = curbuf;
}
if (EQUAL_POS(lastpos, curwin->w_cursor) && !wraparound
&& (dirc == '/' ? cur < cnt : cur > 0))
cur += dirc == '/' ? 1 : -1;
else
{
p_ws = FALSE;
#if defined(FEAT_RELTIME)
profile_setlimit(20L, &start);
#endif
while (!got_int && searchit(curwin, curbuf, &lastpos, NULL,
FORWARD, NULL, 1, SEARCH_KEEP, RE_LAST, NULL) != FAIL)
{
#if defined(FEAT_RELTIME)
if (profile_passed_limit(&start))
{
cnt = OUT_OF_TIME;
cur = OUT_OF_TIME;
break;
}
#endif
cnt++;
if (LTOREQ_POS(lastpos, p))
cur++;
fast_breakcheck();
if (cnt > 99)
break;
}
if (got_int)
cur = -1; 
}
if (cur > 0)
{
char t[SEARCH_STAT_BUF_LEN] = "";
size_t len;
#if defined(FEAT_RIGHTLEFT)
if (curwin->w_p_rl && *curwin->w_p_rlc == 's')
{
if (cur == OUT_OF_TIME)
vim_snprintf(t, SEARCH_STAT_BUF_LEN, "[?/??]");
else if (cnt > 99 && cur > 99)
vim_snprintf(t, SEARCH_STAT_BUF_LEN, "[>99/>99]");
else if (cnt > 99)
vim_snprintf(t, SEARCH_STAT_BUF_LEN, "[>99/%d]", cur);
else
vim_snprintf(t, SEARCH_STAT_BUF_LEN, "[%d/%d]", cnt, cur);
}
else
#endif
{
if (cur == OUT_OF_TIME)
vim_snprintf(t, SEARCH_STAT_BUF_LEN, "[?/??]");
else if (cnt > 99 && cur > 99)
vim_snprintf(t, SEARCH_STAT_BUF_LEN, "[>99/>99]");
else if (cnt > 99)
vim_snprintf(t, SEARCH_STAT_BUF_LEN, "[%d/>99]", cur);
else
vim_snprintf(t, SEARCH_STAT_BUF_LEN, "[%d/%d]", cur, cnt);
}
len = STRLEN(t);
if (show_top_bot_msg && len + 2 < SEARCH_STAT_BUF_LEN)
{
mch_memmove(t + 2, t, len);
t[0] = 'W';
t[1] = ' ';
len += 2;
}
mch_memmove(msgbuf + STRLEN(msgbuf) - len, t, len);
if (dirc == '?' && cur == 100)
cur = -1;
vim_free(lastpat);
lastpat = vim_strsave(spats[last_idx].pat);
chgtick = CHANGEDTICK(curbuf);
lbuf = curbuf;
lastpos = p;
msg_hist_off = TRUE;
give_warning(msgbuf, FALSE);
msg_hist_off = FALSE;
}
p_ws = save_ws;
}
#if defined(FEAT_FIND_ID) || defined(PROTO)
void
find_pattern_in_path(
char_u *ptr, 
int dir UNUSED, 
int len, 
int whole, 
int skip_comments, 
int type, 
long count,
int action, 
linenr_T start_lnum, 
linenr_T end_lnum) 
{
SearchedFile *files; 
SearchedFile *bigger; 
int max_path_depth = 50;
long match_count = 1;
char_u *pat;
char_u *new_fname;
char_u *curr_fname = curbuf->b_fname;
char_u *prev_fname = NULL;
linenr_T lnum;
int depth;
int depth_displayed; 
int old_files;
int already_searched;
char_u *file_line;
char_u *line;
char_u *p;
char_u save_char;
int define_matched;
regmatch_T regmatch;
regmatch_T incl_regmatch;
regmatch_T def_regmatch;
int matched = FALSE;
int did_show = FALSE;
int found = FALSE;
int i;
char_u *already = NULL;
char_u *startp = NULL;
char_u *inc_opt = NULL;
#if defined(FEAT_QUICKFIX)
win_T *curwin_save = NULL;
#endif
regmatch.regprog = NULL;
incl_regmatch.regprog = NULL;
def_regmatch.regprog = NULL;
file_line = alloc(LSIZE);
if (file_line == NULL)
return;
if (type != CHECK_PATH && type != FIND_DEFINE
&& !(compl_cont_status & CONT_SOL))
{
pat = alloc(len + 5);
if (pat == NULL)
goto fpip_end;
sprintf((char *)pat, whole ? "\\<%.*s\\>" : "%.*s", len, ptr);
regmatch.rm_ic = ignorecase(pat);
regmatch.regprog = vim_regcomp(pat, p_magic ? RE_MAGIC : 0);
vim_free(pat);
if (regmatch.regprog == NULL)
goto fpip_end;
}
inc_opt = (*curbuf->b_p_inc == NUL) ? p_inc : curbuf->b_p_inc;
if (*inc_opt != NUL)
{
incl_regmatch.regprog = vim_regcomp(inc_opt, p_magic ? RE_MAGIC : 0);
if (incl_regmatch.regprog == NULL)
goto fpip_end;
incl_regmatch.rm_ic = FALSE; 
}
if (type == FIND_DEFINE && (*curbuf->b_p_def != NUL || *p_def != NUL))
{
def_regmatch.regprog = vim_regcomp(*curbuf->b_p_def == NUL
? p_def : curbuf->b_p_def, p_magic ? RE_MAGIC : 0);
if (def_regmatch.regprog == NULL)
goto fpip_end;
def_regmatch.rm_ic = FALSE; 
}
files = lalloc_clear(max_path_depth * sizeof(SearchedFile), TRUE);
if (files == NULL)
goto fpip_end;
old_files = max_path_depth;
depth = depth_displayed = -1;
lnum = start_lnum;
if (end_lnum > curbuf->b_ml.ml_line_count)
end_lnum = curbuf->b_ml.ml_line_count;
if (lnum > end_lnum) 
lnum = end_lnum;
line = ml_get(lnum);
for (;;)
{
if (incl_regmatch.regprog != NULL
&& vim_regexec(&incl_regmatch, line, (colnr_T)0))
{
char_u *p_fname = (curr_fname == curbuf->b_fname)
? curbuf->b_ffname : curr_fname;
if (inc_opt != NULL && strstr((char *)inc_opt, "\\zs") != NULL)
new_fname = find_file_name_in_path(incl_regmatch.startp[0],
(int)(incl_regmatch.endp[0] - incl_regmatch.startp[0]),
FNAME_EXP|FNAME_INCL|FNAME_REL, 1L, p_fname);
else
new_fname = file_name_in_line(incl_regmatch.endp[0], 0,
FNAME_EXP|FNAME_INCL|FNAME_REL, 1L, p_fname, NULL);
already_searched = FALSE;
if (new_fname != NULL)
{
for (i = 0;; i++)
{
if (i == depth + 1)
i = old_files;
if (i == max_path_depth)
break;
if (fullpathcmp(new_fname, files[i].name, TRUE, TRUE)
& FPC_SAME)
{
if (type != CHECK_PATH &&
action == ACTION_SHOW_ALL && files[i].matched)
{
msg_putchar('\n'); 
if (!got_int) 
{
msg_home_replace_hl(new_fname);
msg_puts(_(" (includes previously listed match)"));
prev_fname = NULL;
}
}
VIM_CLEAR(new_fname);
already_searched = TRUE;
break;
}
}
}
if (type == CHECK_PATH && (action == ACTION_SHOW_ALL
|| (new_fname == NULL && !already_searched)))
{
if (did_show)
msg_putchar('\n'); 
else
{
gotocmdline(TRUE); 
msg_puts_title(_("--- Included files "));
if (action != ACTION_SHOW_ALL)
msg_puts_title(_("not found "));
msg_puts_title(_("in path ---\n"));
}
did_show = TRUE;
while (depth_displayed < depth && !got_int)
{
++depth_displayed;
for (i = 0; i < depth_displayed; i++)
msg_puts(" ");
msg_home_replace(files[depth_displayed].name);
msg_puts(" -->\n");
}
if (!got_int) 
{
for (i = 0; i <= depth_displayed; i++)
msg_puts(" ");
if (new_fname != NULL)
{
msg_outtrans_attr(new_fname, HL_ATTR(HLF_D));
}
else
{
if (inc_opt != NULL
&& strstr((char *)inc_opt, "\\zs") != NULL)
{
p = incl_regmatch.startp[0];
i = (int)(incl_regmatch.endp[0]
- incl_regmatch.startp[0]);
}
else
{
for (p = incl_regmatch.endp[0];
*p && !vim_isfilec(*p); p++)
;
for (i = 0; vim_isfilec(p[i]); i++)
;
}
if (i == 0)
{
p = incl_regmatch.endp[0];
i = (int)STRLEN(p);
}
else if (p > line)
{
if (p[-1] == '"' || p[-1] == '<')
{
--p;
++i;
}
if (p[i] == '"' || p[i] == '>')
++i;
}
save_char = p[i];
p[i] = NUL;
msg_outtrans_attr(p, HL_ATTR(HLF_D));
p[i] = save_char;
}
if (new_fname == NULL && action == ACTION_SHOW_ALL)
{
if (already_searched)
msg_puts(_(" (Already listed)"));
else
msg_puts(_(" NOT FOUND"));
}
}
out_flush(); 
}
if (new_fname != NULL)
{
if (depth + 1 == old_files)
{
bigger = ALLOC_MULT(SearchedFile, max_path_depth * 2);
if (bigger != NULL)
{
for (i = 0; i <= depth; i++)
bigger[i] = files[i];
for (i = depth + 1; i < old_files + max_path_depth; i++)
{
bigger[i].fp = NULL;
bigger[i].name = NULL;
bigger[i].lnum = 0;
bigger[i].matched = FALSE;
}
for (i = old_files; i < max_path_depth; i++)
bigger[i + max_path_depth] = files[i];
old_files += max_path_depth;
max_path_depth *= 2;
vim_free(files);
files = bigger;
}
}
if ((files[depth + 1].fp = mch_fopen((char *)new_fname, "r"))
== NULL)
vim_free(new_fname);
else
{
if (++depth == old_files)
{
vim_free(files[old_files].name);
++old_files;
}
files[depth].name = curr_fname = new_fname;
files[depth].lnum = 0;
files[depth].matched = FALSE;
if (action == ACTION_EXPAND)
{
msg_hist_off = TRUE; 
vim_snprintf((char*)IObuff, IOSIZE,
_("Scanning included file: %s"),
(char *)new_fname);
msg_trunc_attr((char *)IObuff, TRUE, HL_ATTR(HLF_R));
}
else if (p_verbose >= 5)
{
verbose_enter();
smsg(_("Searching included file %s"),
(char *)new_fname);
verbose_leave();
}
}
}
}
else
{
p = line;
search_line:
define_matched = FALSE;
if (def_regmatch.regprog != NULL
&& vim_regexec(&def_regmatch, line, (colnr_T)0))
{
p = def_regmatch.endp[0];
while (*p && !vim_iswordc(*p))
p++;
define_matched = TRUE;
}
if (def_regmatch.regprog == NULL || define_matched)
{
if (define_matched || (compl_cont_status & CONT_SOL))
{
startp = skipwhite(p);
if (p_ic)
matched = !MB_STRNICMP(startp, ptr, len);
else
matched = !STRNCMP(startp, ptr, len);
if (matched && define_matched && whole
&& vim_iswordc(startp[len]))
matched = FALSE;
}
else if (regmatch.regprog != NULL
&& vim_regexec(&regmatch, line, (colnr_T)(p - line)))
{
matched = TRUE;
startp = regmatch.startp[0];
if (!define_matched && skip_comments)
{
if ((*line != '#' ||
STRNCMP(skipwhite(line + 1), "define", 6) != 0)
&& get_leader_len(line, NULL, FALSE, TRUE))
matched = FALSE;
p = skipwhite(line);
if (matched
|| (p[0] == '/' && p[1] == '*') || p[0] == '*')
for (p = line; *p && p < startp; ++p)
{
if (matched
&& p[0] == '/'
&& (p[1] == '*' || p[1] == '/'))
{
matched = FALSE;
if (p[1] == '/')
break;
++p;
}
else if (!matched && p[0] == '*' && p[1] == '/')
{
matched = TRUE;
++p;
}
}
}
}
}
}
if (matched)
{
if (action == ACTION_EXPAND)
{
int cont_s_ipos = FALSE;
int add_r;
char_u *aux;
if (depth == -1 && lnum == curwin->w_cursor.lnum)
break;
found = TRUE;
aux = p = startp;
if (compl_cont_status & CONT_ADDING)
{
p += compl_length;
if (vim_iswordp(p))
goto exit_matched;
p = find_word_start(p);
}
p = find_word_end(p);
i = (int)(p - aux);
if ((compl_cont_status & CONT_ADDING) && i == compl_length)
{
STRNCPY(IObuff, aux, i);
if (depth < 0)
{
if (lnum >= end_lnum)
goto exit_matched;
line = ml_get(++lnum);
}
else if (vim_fgets(line = file_line,
LSIZE, files[depth].fp))
goto exit_matched;
already = aux = p = skipwhite(line);
p = find_word_start(p);
p = find_word_end(p);
if (p > aux)
{
if (*aux != ')' && IObuff[i-1] != TAB)
{
if (IObuff[i-1] != ' ')
IObuff[i++] = ' ';
if (p_js
&& (IObuff[i-2] == '.'
|| (vim_strchr(p_cpo, CPO_JOINSP) == NULL
&& (IObuff[i-2] == '?'
|| IObuff[i-2] == '!'))))
IObuff[i++] = ' ';
}
if (p - aux >= IOSIZE - i)
p = aux + IOSIZE - i - 1;
STRNCPY(IObuff + i, aux, p - aux);
i += (int)(p - aux);
cont_s_ipos = TRUE;
}
IObuff[i] = NUL;
aux = IObuff;
if (i == compl_length)
goto exit_matched;
}
add_r = ins_compl_add_infercase(aux, i, p_ic,
curr_fname == curbuf->b_fname ? NULL : curr_fname,
dir, cont_s_ipos);
if (add_r == OK)
dir = FORWARD;
else if (add_r == FAIL)
break;
}
else if (action == ACTION_SHOW_ALL)
{
found = TRUE;
if (!did_show)
gotocmdline(TRUE); 
if (curr_fname != prev_fname)
{
if (did_show)
msg_putchar('\n'); 
if (!got_int) 
msg_home_replace_hl(curr_fname);
prev_fname = curr_fname;
}
did_show = TRUE;
if (!got_int)
show_pat_in_path(line, type, TRUE, action,
(depth == -1) ? NULL : files[depth].fp,
(depth == -1) ? &lnum : &files[depth].lnum,
match_count++);
for (i = 0; i <= depth; ++i)
files[i].matched = TRUE;
}
else if (--count <= 0)
{
found = TRUE;
if (depth == -1 && lnum == curwin->w_cursor.lnum
#if defined(FEAT_QUICKFIX)
&& g_do_tagpreview == 0
#endif
)
emsg(_("E387: Match is on current line"));
else if (action == ACTION_SHOW)
{
show_pat_in_path(line, type, did_show, action,
(depth == -1) ? NULL : files[depth].fp,
(depth == -1) ? &lnum : &files[depth].lnum, 1L);
did_show = TRUE;
}
else
{
#if defined(FEAT_GUI)
need_mouse_correct = TRUE;
#endif
#if defined(FEAT_QUICKFIX)
if (g_do_tagpreview != 0)
{
curwin_save = curwin;
prepare_tagpreview(TRUE, TRUE, FALSE);
}
#endif
if (action == ACTION_SPLIT)
{
if (win_split(0, 0) == FAIL)
break;
RESET_BINDING(curwin);
}
if (depth == -1)
{
#if defined(FEAT_QUICKFIX)
if (g_do_tagpreview != 0)
{
if (!GETFILE_SUCCESS(getfile(
curwin_save->w_buffer->b_fnum, NULL,
NULL, TRUE, lnum, FALSE)))
break; 
}
else
#endif
setpcmark();
curwin->w_cursor.lnum = lnum;
check_cursor();
}
else
{
if (!GETFILE_SUCCESS(getfile(
0, files[depth].name, NULL, TRUE,
files[depth].lnum, FALSE)))
break; 
curwin->w_cursor.lnum = files[depth].lnum;
}
}
if (action != ACTION_SHOW)
{
curwin->w_cursor.col = (colnr_T)(startp - line);
curwin->w_set_curswant = TRUE;
}
#if defined(FEAT_QUICKFIX)
if (g_do_tagpreview != 0
&& curwin != curwin_save && win_valid(curwin_save))
{
validate_cursor();
redraw_later(VALID);
win_enter(curwin_save, TRUE);
}
#if defined(FEAT_PROP_POPUP)
else if (WIN_IS_POPUP(curwin))
win_enter(firstwin, TRUE);
#endif
#endif
break;
}
exit_matched:
matched = FALSE;
if (def_regmatch.regprog == NULL
&& action == ACTION_EXPAND
&& !(compl_cont_status & CONT_SOL)
&& *startp != NUL
&& *(p = startp + mb_ptr2len(startp)) != NUL)
goto search_line;
}
line_breakcheck();
if (action == ACTION_EXPAND)
ins_compl_check_keys(30, FALSE);
if (got_int || ins_compl_interrupted())
break;
while (depth >= 0 && !already
&& vim_fgets(line = file_line, LSIZE, files[depth].fp))
{
fclose(files[depth].fp);
--old_files;
files[old_files].name = files[depth].name;
files[old_files].matched = files[depth].matched;
--depth;
curr_fname = (depth == -1) ? curbuf->b_fname
: files[depth].name;
if (depth < depth_displayed)
depth_displayed = depth;
}
if (depth >= 0) 
{
files[depth].lnum++;
i = (int)STRLEN(line);
if (i > 0 && line[i - 1] == '\n')
line[--i] = NUL;
if (i > 0 && line[i - 1] == '\r')
line[--i] = NUL;
}
else if (!already)
{
if (++lnum > end_lnum)
break;
line = ml_get(lnum);
}
already = NULL;
}
for (i = 0; i <= depth; i++)
{
fclose(files[i].fp);
vim_free(files[i].name);
}
for (i = old_files; i < max_path_depth; i++)
vim_free(files[i].name);
vim_free(files);
if (type == CHECK_PATH)
{
if (!did_show)
{
if (action != ACTION_SHOW_ALL)
msg(_("All included files were found"));
else
msg(_("No included files"));
}
}
else if (!found && action != ACTION_EXPAND)
{
if (got_int || ins_compl_interrupted())
emsg(_(e_interr));
else if (type == FIND_DEFINE)
emsg(_("E388: Couldn't find definition"));
else
emsg(_("E389: Couldn't find pattern"));
}
if (action == ACTION_SHOW || action == ACTION_SHOW_ALL)
msg_end();
fpip_end:
vim_free(file_line);
vim_regfree(regmatch.regprog);
vim_regfree(incl_regmatch.regprog);
vim_regfree(def_regmatch.regprog);
}
static void
show_pat_in_path(
char_u *line,
int type,
int did_show,
int action,
FILE *fp,
linenr_T *lnum,
long count)
{
char_u *p;
if (did_show)
msg_putchar('\n'); 
else if (!msg_silent)
gotocmdline(TRUE); 
if (got_int) 
return;
for (;;)
{
p = line + STRLEN(line) - 1;
if (fp != NULL)
{
if (p >= line && *p == '\n')
--p;
if (p >= line && *p == '\r')
--p;
*(p + 1) = NUL;
}
if (action == ACTION_SHOW_ALL)
{
sprintf((char *)IObuff, "%3ld: ", count); 
msg_puts((char *)IObuff);
sprintf((char *)IObuff, "%4ld", *lnum); 
msg_puts_attr((char *)IObuff, HL_ATTR(HLF_N));
msg_puts(" ");
}
msg_prt_line(line, FALSE);
out_flush(); 
if (got_int || type != FIND_DEFINE || p < line || *p != '\\')
break;
if (fp != NULL)
{
if (vim_fgets(line, LSIZE, fp)) 
break;
++*lnum;
}
else
{
if (++*lnum > curbuf->b_ml.ml_line_count)
break;
line = ml_get(*lnum);
}
msg_putchar('\n');
}
}
#endif
#if defined(FEAT_VIMINFO)
spat_T *
get_spat(int idx)
{
return &spats[idx];
}
int
get_spat_last_idx(void)
{
return last_idx;
}
#endif
