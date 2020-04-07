





















#include "xinclude.h"

#define XDL_MAX_COST_MIN 256
#define XDL_HEUR_MIN_COST 256
#define XDL_LINE_MAX (long)((1UL << (CHAR_BIT * sizeof(long) - 1)) - 1)
#define XDL_SNAKE_CNT 20
#define XDL_K_HEUR 4

typedef struct s_xdpsplit {
long i1, i2;
int min_lo, min_hi;
} xdpsplit_t;










static long xdl_split(unsigned long const *ha1, long off1, long lim1,
unsigned long const *ha2, long off2, long lim2,
long *kvdf, long *kvdb, int need_min, xdpsplit_t *spl,
xdalgoenv_t *xenv) {
long dmin = off1 - lim2, dmax = lim1 - off2;
long fmid = off1 - off2, bmid = lim1 - lim2;
long odd = (fmid - bmid) & 1;
long fmin = fmid, fmax = fmid;
long bmin = bmid, bmax = bmid;
long ec, d, i1, i2, prev1, best, dd, v, k;




kvdf[fmid] = off1;
kvdb[bmid] = lim1;

for (ec = 1;; ec++) {
int got_snake = 0;








if (fmin > dmin)
kvdf[--fmin - 1] = -1;
else
++fmin;
if (fmax < dmax)
kvdf[++fmax + 1] = -1;
else
--fmax;

for (d = fmax; d >= fmin; d -= 2) {
if (kvdf[d - 1] >= kvdf[d + 1])
i1 = kvdf[d - 1] + 1;
else
i1 = kvdf[d + 1];
prev1 = i1;
i2 = i1 - d;
for (; i1 < lim1 && i2 < lim2 && ha1[i1] == ha2[i2]; i1++, i2++);
if (i1 - prev1 > xenv->snake_cnt)
got_snake = 1;
kvdf[d] = i1;
if (odd && bmin <= d && d <= bmax && kvdb[d] <= i1) {
spl->i1 = i1;
spl->i2 = i2;
spl->min_lo = spl->min_hi = 1;
return ec;
}
}








if (bmin > dmin)
kvdb[--bmin - 1] = XDL_LINE_MAX;
else
++bmin;
if (bmax < dmax)
kvdb[++bmax + 1] = XDL_LINE_MAX;
else
--bmax;

for (d = bmax; d >= bmin; d -= 2) {
if (kvdb[d - 1] < kvdb[d + 1])
i1 = kvdb[d - 1];
else
i1 = kvdb[d + 1] - 1;
prev1 = i1;
i2 = i1 - d;
for (; i1 > off1 && i2 > off2 && ha1[i1 - 1] == ha2[i2 - 1]; i1--, i2--);
if (prev1 - i1 > xenv->snake_cnt)
got_snake = 1;
kvdb[d] = i1;
if (!odd && fmin <= d && d <= fmax && i1 <= kvdf[d]) {
spl->i1 = i1;
spl->i2 = i2;
spl->min_lo = spl->min_hi = 1;
return ec;
}
}

if (need_min)
continue;











if (got_snake && ec > xenv->heur_min) {
for (best = 0, d = fmax; d >= fmin; d -= 2) {
dd = d > fmid ? d - fmid: fmid - d;
i1 = kvdf[d];
i2 = i1 - d;
v = (i1 - off1) + (i2 - off2) - dd;

if (v > XDL_K_HEUR * ec && v > best &&
off1 + xenv->snake_cnt <= i1 && i1 < lim1 &&
off2 + xenv->snake_cnt <= i2 && i2 < lim2) {
for (k = 1; ha1[i1 - k] == ha2[i2 - k]; k++)
if (k == xenv->snake_cnt) {
best = v;
spl->i1 = i1;
spl->i2 = i2;
break;
}
}
}
if (best > 0) {
spl->min_lo = 1;
spl->min_hi = 0;
return ec;
}

for (best = 0, d = bmax; d >= bmin; d -= 2) {
dd = d > bmid ? d - bmid: bmid - d;
i1 = kvdb[d];
i2 = i1 - d;
v = (lim1 - i1) + (lim2 - i2) - dd;

if (v > XDL_K_HEUR * ec && v > best &&
off1 < i1 && i1 <= lim1 - xenv->snake_cnt &&
off2 < i2 && i2 <= lim2 - xenv->snake_cnt) {
for (k = 0; ha1[i1 + k] == ha2[i2 + k]; k++)
if (k == xenv->snake_cnt - 1) {
best = v;
spl->i1 = i1;
spl->i2 = i2;
break;
}
}
}
if (best > 0) {
spl->min_lo = 0;
spl->min_hi = 1;
return ec;
}
}





if (ec >= xenv->mxcost) {
long fbest, fbest1, bbest, bbest1;

fbest = fbest1 = -1;
for (d = fmax; d >= fmin; d -= 2) {
i1 = XDL_MIN(kvdf[d], lim1);
i2 = i1 - d;
if (lim2 < i2)
i1 = lim2 + d, i2 = lim2;
if (fbest < i1 + i2) {
fbest = i1 + i2;
fbest1 = i1;
}
}

bbest = bbest1 = XDL_LINE_MAX;
for (d = bmax; d >= bmin; d -= 2) {
i1 = XDL_MAX(off1, kvdb[d]);
i2 = i1 - d;
if (i2 < off2)
i1 = off2 + d, i2 = off2;
if (i1 + i2 < bbest) {
bbest = i1 + i2;
bbest1 = i1;
}
}

if ((lim1 + lim2) - bbest < fbest - (off1 + off2)) {
spl->i1 = fbest1;
spl->i2 = fbest - fbest1;
spl->min_lo = 1;
spl->min_hi = 0;
} else {
spl->i1 = bbest1;
spl->i2 = bbest - bbest1;
spl->min_lo = 0;
spl->min_hi = 1;
}
return ec;
}
}
}







int xdl_recs_cmp(diffdata_t *dd1, long off1, long lim1,
diffdata_t *dd2, long off2, long lim2,
long *kvdf, long *kvdb, int need_min, xdalgoenv_t *xenv) {
unsigned long const *ha1 = dd1->ha, *ha2 = dd2->ha;




for (; off1 < lim1 && off2 < lim2 && ha1[off1] == ha2[off2]; off1++, off2++);
for (; off1 < lim1 && off2 < lim2 && ha1[lim1 - 1] == ha2[lim2 - 1]; lim1--, lim2--);





if (off1 == lim1) {
char *rchg2 = dd2->rchg;
long *rindex2 = dd2->rindex;

for (; off2 < lim2; off2++)
rchg2[rindex2[off2]] = 1;
} else if (off2 == lim2) {
char *rchg1 = dd1->rchg;
long *rindex1 = dd1->rindex;

for (; off1 < lim1; off1++)
rchg1[rindex1[off1]] = 1;
} else {
xdpsplit_t spl;
spl.i1 = spl.i2 = 0;




if (xdl_split(ha1, off1, lim1, ha2, off2, lim2, kvdf, kvdb,
need_min, &spl, xenv) < 0) {

return -1;
}




if (xdl_recs_cmp(dd1, off1, spl.i1, dd2, off2, spl.i2,
kvdf, kvdb, spl.min_lo, xenv) < 0 ||
xdl_recs_cmp(dd1, spl.i1, lim1, dd2, spl.i2, lim2,
kvdf, kvdb, spl.min_hi, xenv) < 0) {

return -1;
}
}

return 0;
}


int xdl_do_diff(mmfile_t *mf1, mmfile_t *mf2, xpparam_t const *xpp,
xdfenv_t *xe) {
long ndiags;
long *kvd, *kvdf, *kvdb;
xdalgoenv_t xenv;
diffdata_t dd1, dd2;

if (XDF_DIFF_ALG(xpp->flags) == XDF_PATIENCE_DIFF)
return xdl_do_patience_diff(mf1, mf2, xpp, xe);

if (XDF_DIFF_ALG(xpp->flags) == XDF_HISTOGRAM_DIFF)
return xdl_do_histogram_diff(mf1, mf2, xpp, xe);

if (xdl_prepare_env(mf1, mf2, xpp, xe) < 0) {

return -1;
}





ndiags = xe->xdf1.nreff + xe->xdf2.nreff + 3;
if (!(kvd = (long *) xdl_malloc((2 * ndiags + 2) * sizeof(long)))) {

xdl_free_env(xe);
return -1;
}
kvdf = kvd;
kvdb = kvdf + ndiags;
kvdf += xe->xdf2.nreff + 1;
kvdb += xe->xdf2.nreff + 1;

xenv.mxcost = xdl_bogosqrt(ndiags);
if (xenv.mxcost < XDL_MAX_COST_MIN)
xenv.mxcost = XDL_MAX_COST_MIN;
xenv.snake_cnt = XDL_SNAKE_CNT;
xenv.heur_min = XDL_HEUR_MIN_COST;

dd1.nrec = xe->xdf1.nreff;
dd1.ha = xe->xdf1.ha;
dd1.rchg = xe->xdf1.rchg;
dd1.rindex = xe->xdf1.rindex;
dd2.nrec = xe->xdf2.nreff;
dd2.ha = xe->xdf2.ha;
dd2.rchg = xe->xdf2.rchg;
dd2.rindex = xe->xdf2.rindex;

if (xdl_recs_cmp(&dd1, 0, dd1.nrec, &dd2, 0, dd2.nrec,
kvdf, kvdb, (xpp->flags & XDF_NEED_MINIMAL) != 0, &xenv) < 0) {

xdl_free(kvd);
xdl_free_env(xe);
return -1;
}

xdl_free(kvd);

return 0;
}


static xdchange_t *xdl_add_change(xdchange_t *xscr, long i1, long i2, long chg1, long chg2) {
xdchange_t *xch;

if (!(xch = (xdchange_t *) xdl_malloc(sizeof(xdchange_t))))
return NULL;

xch->next = xscr;
xch->i1 = i1;
xch->i2 = i2;
xch->chg1 = chg1;
xch->chg2 = chg2;
xch->ignore = 0;

return xch;
}


static int recs_match(xrecord_t *rec1, xrecord_t *rec2, long flags)
{
return (rec1->ha == rec2->ha &&
xdl_recmatch(rec1->ptr, rec1->size,
rec2->ptr, rec2->size,
flags));
}







#define MAX_INDENT 200






static int xget_indent(xrecord_t *rec)
{
long i;
int ret = 0;

for (i = 0; i < rec->size; i++) {
char c = rec->ptr[i];

if (!XDL_ISSPACE(c))
return ret;
else if (c == ' ')
ret += 1;
else if (c == '\t')
ret += 8 - ret % 8;


if (ret >= MAX_INDENT)
return MAX_INDENT;
}


return -1;
}






#define MAX_BLANKS 20


struct split_measurement {



int end_of_file;





int indent;




int pre_blank;





int pre_indent;




int post_blank;





int post_indent;
};

struct split_score {

int effective_indent;


int penalty;
};




static void measure_split(const xdfile_t *xdf, long split,
struct split_measurement *m)
{
long i;

if (split >= xdf->nrec) {
m->end_of_file = 1;
m->indent = -1;
} else {
m->end_of_file = 0;
m->indent = xget_indent(xdf->recs[split]);
}

m->pre_blank = 0;
m->pre_indent = -1;
for (i = split - 1; i >= 0; i--) {
m->pre_indent = xget_indent(xdf->recs[i]);
if (m->pre_indent != -1)
break;
m->pre_blank += 1;
if (m->pre_blank == MAX_BLANKS) {
m->pre_indent = 0;
break;
}
}

m->post_blank = 0;
m->post_indent = -1;
for (i = split + 1; i < xdf->nrec; i++) {
m->post_indent = xget_indent(xdf->recs[i]);
if (m->post_indent != -1)
break;
m->post_blank += 1;
if (m->post_blank == MAX_BLANKS) {
m->post_indent = 0;
break;
}
}
}














#define START_OF_FILE_PENALTY 1


#define END_OF_FILE_PENALTY 21


#define TOTAL_BLANK_WEIGHT (-30)


#define POST_BLANK_WEIGHT 6




#define RELATIVE_INDENT_PENALTY (-4)
#define RELATIVE_INDENT_WITH_BLANK_PENALTY 10





#define RELATIVE_OUTDENT_PENALTY 24
#define RELATIVE_OUTDENT_WITH_BLANK_PENALTY 17





#define RELATIVE_DEDENT_PENALTY 23
#define RELATIVE_DEDENT_WITH_BLANK_PENALTY 17







#define INDENT_WEIGHT 60




#define INDENT_HEURISTIC_MAX_SLIDING 100











static void score_add_split(const struct split_measurement *m, struct split_score *s)
{




int post_blank, total_blank, indent, any_blanks;

if (m->pre_indent == -1 && m->pre_blank == 0)
s->penalty += START_OF_FILE_PENALTY;

if (m->end_of_file)
s->penalty += END_OF_FILE_PENALTY;





post_blank = (m->indent == -1) ? 1 + m->post_blank : 0;
total_blank = m->pre_blank + post_blank;


s->penalty += TOTAL_BLANK_WEIGHT * total_blank;
s->penalty += POST_BLANK_WEIGHT * post_blank;

if (m->indent != -1)
indent = m->indent;
else
indent = m->post_indent;

any_blanks = (total_blank != 0);


s->effective_indent += indent;

if (indent == -1) {

} else if (m->pre_indent == -1) {

} else if (indent > m->pre_indent) {



s->penalty += any_blanks ?
RELATIVE_INDENT_WITH_BLANK_PENALTY :
RELATIVE_INDENT_PENALTY;
} else if (indent == m->pre_indent) {




} else {







if (m->post_indent != -1 && m->post_indent > indent) {




s->penalty += any_blanks ?
RELATIVE_OUTDENT_WITH_BLANK_PENALTY :
RELATIVE_OUTDENT_PENALTY;
} else {



s->penalty += any_blanks ?
RELATIVE_DEDENT_WITH_BLANK_PENALTY :
RELATIVE_DEDENT_PENALTY;
}
}
}

static int score_cmp(struct split_score *s1, struct split_score *s2)
{

int cmp_indents = ((s1->effective_indent > s2->effective_indent) -
(s1->effective_indent < s2->effective_indent));

return INDENT_WEIGHT * cmp_indents + (s1->penalty - s2->penalty);
}
















struct xdlgroup {




long start;





long end;
};




static void group_init(xdfile_t *xdf, struct xdlgroup *g)
{
g->start = g->end = 0;
while (xdf->rchg[g->end])
g->end++;
}





static inline int group_next(xdfile_t *xdf, struct xdlgroup *g)
{
if (g->end == xdf->nrec)
return -1;

g->start = g->end + 1;
for (g->end = g->start; xdf->rchg[g->end]; g->end++)
;

return 0;
}





static inline int group_previous(xdfile_t *xdf, struct xdlgroup *g)
{
if (g->start == 0)
return -1;

g->end = g->start - 1;
for (g->start = g->end; xdf->rchg[g->start - 1]; g->start--)
;

return 0;
}






static int group_slide_down(xdfile_t *xdf, struct xdlgroup *g, long flags)
{
if (g->end < xdf->nrec &&
recs_match(xdf->recs[g->start], xdf->recs[g->end], flags)) {
xdf->rchg[g->start++] = 0;
xdf->rchg[g->end++] = 1;

while (xdf->rchg[g->end])
g->end++;

return 0;
} else {
return -1;
}
}






static int group_slide_up(xdfile_t *xdf, struct xdlgroup *g, long flags)
{
if (g->start > 0 &&
recs_match(xdf->recs[g->start - 1], xdf->recs[g->end - 1], flags)) {
xdf->rchg[--g->start] = 1;
xdf->rchg[--g->end] = 0;

while (xdf->rchg[g->start - 1])
g->start--;

return 0;
} else {
return -1;
}
}

static void xdl_bug(const char *msg)
{
fprintf(stderr, "BUG: %s\n", msg);
exit(1);
}






int xdl_change_compact(xdfile_t *xdf, xdfile_t *xdfo, long flags) {
struct xdlgroup g, go;
long earliest_end, end_matching_other;
long groupsize;

group_init(xdf, &g);
group_init(xdfo, &go);

while (1) {

if (g.end == g.start)
goto next;





do {
groupsize = g.end - g.start;







end_matching_other = -1;


while (!group_slide_up(xdf, &g, flags))
if (group_previous(xdfo, &go))
xdl_bug("group sync broken sliding up");





earliest_end = g.end;

if (go.end > go.start)
end_matching_other = g.end;


while (1) {
if (group_slide_down(xdf, &g, flags))
break;
if (group_next(xdfo, &go))
xdl_bug("group sync broken sliding down");

if (go.end > go.start)
end_matching_other = g.end;
}
} while (groupsize != g.end - g.start);









if (g.end == earliest_end) {

} else if (end_matching_other != -1) {





while (go.end == go.start) {
if (group_slide_up(xdf, &g, flags))
xdl_bug("match disappeared");
if (group_previous(xdfo, &go))
xdl_bug("group sync broken sliding to match");
}
} else if (flags & XDF_INDENT_HEURISTIC) {











long shift, best_shift = -1;
struct split_score best_score;

shift = earliest_end;
if (g.end - groupsize - 1 > shift)
shift = g.end - groupsize - 1;
if (g.end - INDENT_HEURISTIC_MAX_SLIDING > shift)
shift = g.end - INDENT_HEURISTIC_MAX_SLIDING;
for (; shift <= g.end; shift++) {
struct split_measurement m;
struct split_score score = {0, 0};

measure_split(xdf, shift, &m);
score_add_split(&m, &score);
measure_split(xdf, shift - groupsize, &m);
score_add_split(&m, &score);
if (best_shift == -1 ||
score_cmp(&score, &best_score) <= 0) {
best_score.effective_indent = score.effective_indent;
best_score.penalty = score.penalty;
best_shift = shift;
}
}

while (g.end > best_shift) {
if (group_slide_up(xdf, &g, flags))
xdl_bug("best shift unreached");
if (group_previous(xdfo, &go))
xdl_bug("group sync broken sliding to blank line");
}
}

next:

if (group_next(xdf, &g))
break;
if (group_next(xdfo, &go))
xdl_bug("group sync broken moving to next group");
}

if (!group_next(xdfo, &go))
xdl_bug("group sync broken at end of file");

return 0;
}


int xdl_build_script(xdfenv_t *xe, xdchange_t **xscr) {
xdchange_t *cscr = NULL, *xch;
char *rchg1 = xe->xdf1.rchg, *rchg2 = xe->xdf2.rchg;
long i1, i2, l1, l2;




for (i1 = xe->xdf1.nrec, i2 = xe->xdf2.nrec; i1 >= 0 || i2 >= 0; i1--, i2--)
if (rchg1[i1 - 1] || rchg2[i2 - 1]) {
for (l1 = i1; rchg1[i1 - 1]; i1--);
for (l2 = i2; rchg2[i2 - 1]; i2--);

if (!(xch = xdl_add_change(cscr, i1, i2, l1 - i1, l2 - i2))) {
xdl_free_script(cscr);
return -1;
}
cscr = xch;
}

*xscr = cscr;

return 0;
}


void xdl_free_script(xdchange_t *xscr) {
xdchange_t *xch;

while ((xch = xscr) != NULL) {
xscr = xscr->next;
xdl_free(xch);
}
}

static int xdl_call_hunk_func(xdfenv_t *xe UNUSED, xdchange_t *xscr, xdemitcb_t *ecb,
xdemitconf_t const *xecfg)
{
xdchange_t *xch, *xche;

for (xch = xscr; xch; xch = xche->next) {
xche = xdl_get_hunk(&xch, xecfg);
if (!xch)
break;
if (xecfg->hunk_func(xch->i1, xche->i1 + xche->chg1 - xch->i1,
xch->i2, xche->i2 + xche->chg2 - xch->i2,
ecb->priv) < 0)
return -1;
}
return 0;
}

static void xdl_mark_ignorable(xdchange_t *xscr, xdfenv_t *xe, long flags)
{
xdchange_t *xch;

for (xch = xscr; xch; xch = xch->next) {
int ignore = 1;
xrecord_t **rec;
long i;

rec = &xe->xdf1.recs[xch->i1];
for (i = 0; i < xch->chg1 && ignore; i++)
ignore = xdl_blankline(rec[i]->ptr, rec[i]->size, flags);

rec = &xe->xdf2.recs[xch->i2];
for (i = 0; i < xch->chg2 && ignore; i++)
ignore = xdl_blankline(rec[i]->ptr, rec[i]->size, flags);

xch->ignore = ignore;
}
}

int xdl_diff(mmfile_t *mf1, mmfile_t *mf2, xpparam_t const *xpp,
xdemitconf_t const *xecfg, xdemitcb_t *ecb) {
xdchange_t *xscr;
xdfenv_t xe;
emit_func_t ef = xecfg->hunk_func ? xdl_call_hunk_func : xdl_emit_diff;

if (xdl_do_diff(mf1, mf2, xpp, &xe) < 0) {

return -1;
}
if (xdl_change_compact(&xe.xdf1, &xe.xdf2, xpp->flags) < 0 ||
xdl_change_compact(&xe.xdf2, &xe.xdf1, xpp->flags) < 0 ||
xdl_build_script(&xe, &xscr) < 0) {

xdl_free_env(&xe);
return -1;
}
if (xscr) {
if (xpp->flags & XDF_IGNORE_BLANK_LINES)
xdl_mark_ignorable(xscr, &xe, xpp->flags);

if (ef(&xe, xscr, ecb, xecfg) < 0) {

xdl_free_script(xscr);
xdl_free_env(&xe);
return -1;
}
xdl_free_script(xscr);
}
xdl_free_env(&xe);

return 0;
}
