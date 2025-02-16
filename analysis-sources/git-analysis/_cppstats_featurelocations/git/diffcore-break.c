


#include "cache.h"
#include "diff.h"
#include "diffcore.h"

static int should_break(struct repository *r,
struct diff_filespec *src,
struct diff_filespec *dst,
int break_score,
int *merge_score_p)
{



































unsigned long delta_size, max_size;
unsigned long src_copied, literal_added, src_removed;

*merge_score_p = 0; 



if (S_ISREG(src->mode) != S_ISREG(dst->mode)) {
*merge_score_p = (int)MAX_SCORE;
return 1; 
}

if (src->oid_valid && dst->oid_valid &&
oideq(&src->oid, &dst->oid))
return 0; 

if (diff_populate_filespec(r, src, 0) ||
diff_populate_filespec(r, dst, 0))
return 0; 

max_size = ((src->size > dst->size) ? src->size : dst->size);
if (max_size < MINIMUM_BREAK_SIZE)
return 0; 

if (!src->size)
return 0; 

if (diffcore_count_changes(r, src, dst,
&src->cnt_data, &dst->cnt_data,
&src_copied, &literal_added))
return 0;


if (src->size < src_copied)
src_copied = src->size;
if (dst->size < literal_added + src_copied) {
if (src_copied < dst->size)
literal_added = dst->size - src_copied;
else
literal_added = 0;
}
src_removed = src->size - src_copied;






*merge_score_p = (int)(src_removed * MAX_SCORE / src->size);
if (*merge_score_p > break_score)
return 1;




delta_size = src_removed + literal_added;
if (delta_size * MAX_SCORE / max_size < break_score)
return 0;




if ((src->size * break_score < src_removed * MAX_SCORE) &&
(literal_added * 20 < src_removed) &&
(literal_added * 20 < src_copied))
return 0;

return 1;
}

void diffcore_break(struct repository *r, int break_score)
{
struct diff_queue_struct *q = &diff_queued_diff;
struct diff_queue_struct outq;
































int merge_score;
int i;




merge_score = (break_score >> 16) & 0xFFFF;
break_score = (break_score & 0xFFFF);

if (!break_score)
break_score = DEFAULT_BREAK_SCORE;
if (!merge_score)
merge_score = DEFAULT_MERGE_SCORE;

DIFF_QUEUE_CLEAR(&outq);

for (i = 0; i < q->nr; i++) {
struct diff_filepair *p = q->queue[i];
int score;





if (DIFF_FILE_VALID(p->one) && DIFF_FILE_VALID(p->two) &&
object_type(p->one->mode) == OBJ_BLOB &&
object_type(p->two->mode) == OBJ_BLOB &&
!strcmp(p->one->path, p->two->path)) {
if (should_break(r, p->one, p->two,
break_score, &score)) {

struct diff_filespec *null_one, *null_two;
struct diff_filepair *dp;







if (score < merge_score)
score = 0;


null_one = alloc_filespec(p->one->path);
dp = diff_queue(&outq, p->one, null_one);
dp->score = score;
dp->broken_pair = 1;


null_two = alloc_filespec(p->two->path);
dp = diff_queue(&outq, null_two, p->two);
dp->score = score;
dp->broken_pair = 1;

diff_free_filespec_blob(p->one);
diff_free_filespec_blob(p->two);
free(p); 


continue;
}
}
diff_free_filespec_data(p->one);
diff_free_filespec_data(p->two);
diff_q(&outq, p);
}
free(q->queue);
*q = outq;

return;
}

static void merge_broken(struct diff_filepair *p,
struct diff_filepair *pp,
struct diff_queue_struct *outq)
{

struct diff_filepair *c = p, *d = pp, *dp;
if (DIFF_FILE_VALID(p->one)) {

d = p; c = pp;
}

if (!DIFF_FILE_VALID(d->one))
die("internal error in merge #1");
if (DIFF_FILE_VALID(d->two))
die("internal error in merge #2");
if (DIFF_FILE_VALID(c->one))
die("internal error in merge #3");
if (!DIFF_FILE_VALID(c->two))
die("internal error in merge #4");

dp = diff_queue(outq, d->one, c->two);
dp->score = p->score;






d->one->rename_used++;
diff_free_filespec_data(d->two);
diff_free_filespec_data(c->one);
free(d);
free(c);
}

void diffcore_merge_broken(void)
{
struct diff_queue_struct *q = &diff_queued_diff;
struct diff_queue_struct outq;
int i, j;

DIFF_QUEUE_CLEAR(&outq);

for (i = 0; i < q->nr; i++) {
struct diff_filepair *p = q->queue[i];
if (!p)

continue;
else if (p->broken_pair &&
!strcmp(p->one->path, p->two->path)) {



for (j = i + 1; j < q->nr; j++) {
struct diff_filepair *pp = q->queue[j];
if (pp->broken_pair &&
!strcmp(pp->one->path, pp->two->path) &&
!strcmp(p->one->path, pp->two->path)) {

merge_broken(p, pp, &outq);
q->queue[j] = NULL;
goto next;
}
}



diff_q(&outq, p);
}
else
diff_q(&outq, p);
next:;
}
free(q->queue);
*q = outq;

return;
}
