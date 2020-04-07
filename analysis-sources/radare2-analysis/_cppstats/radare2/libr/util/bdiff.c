#include <r_util.h>
#include <r_diff.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
struct line {
int h, len, n, e;
const char *l;
};
struct pos {
int pos, len;
};
struct hunk {
int a1, a2, b1, b2;
};
struct hunklist {
struct hunk *base, *head;
};
static int splitlines(const char *a, int len, struct line **lr) {
int h, i;
const char *p, *b = a;
const char * const plast = a + len - 1;
struct line *l;
if (!a) {
eprintf ("null pointer received\n");
return 0;
}
i = 1; 
for (p = a; p < a + len; p++) {
if (*p == '\n' || p == plast) {
i++;
}
}
*lr = l = (struct line *)malloc(sizeof(struct line) * i);
if (!l) {
return -1;
}
h = 0;
for (p = a; p < a + len; p++) {
h = (h * 1664525) + *p + 1013904223;
if (*p == '\n' || p == plast) {
l->h = h;
h = 0;
l->len = p - b + 1;
l->l = b;
l->n = INT_MAX;
l++;
b = p + 1;
}
}
l->h = l->len = 0;
l->l = a + len;
return i - 1;
}
inline static int cmp(struct line *a, struct line *b) {
return a->h != b->h || a->len != b->len || memcmp(a->l, b->l, a->len);
}
static int equatelines(struct line *a, int an, struct line *b, int bn) {
int i, j, buckets = 1, t, scale;
struct pos *h = NULL;
while (buckets < bn + 1) {
buckets *= 2;
}
for (scale = 4; scale; scale /= 2) {
h = (struct pos *)malloc(scale * buckets * sizeof(struct pos));
if (h) {
break;
}
}
if (!h) {
return 0;
}
buckets = buckets * scale - 1;
for (i = 0; i <= buckets; i++) {
h[i].pos = INT_MAX;
h[i].len = 0;
}
for (i = bn - 1; i >= 0; i--) {
for (j = b[i].h & buckets; h[j].pos != INT_MAX;
j = (j + 1) & buckets) {
if (!cmp (b + i, b + h[j].pos)) {
break;
}
}
b[i].n = h[j].pos;
b[i].e = j;
h[j].pos = i;
h[j].len++; 
}
t = (bn >= 4000) ? bn / 1000 : bn + 1;
for (i = 0; i < an; i++) {
for (j = a[i].h & buckets; h[j].pos != INT_MAX;
j = (j + 1) & buckets) {
if (!cmp (a + i, b + h[j].pos)) {
break;
}
}
a[i].e = j; 
if (h[j].len <= t) {
a[i].n = h[j].pos; 
} else {
a[i].n = INT_MAX; 
}
}
free(h);
return 1;
}
static int longest_match(struct line *a, struct line *b, struct pos *pos,
int a1, int a2, int b1, int b2, int *omi, int *omj)
{
int mi = a1, mj = b1, mk = 0, mb = 0, i, j, k;
for (i = a1; i < a2; i++) {
for (j = a[i].n; j < b1; j = b[j].n) {
;
}
for (; j < b2; j = b[j].n) {
if (i > a1 && j > b1 && pos[j - 1].pos == i - 1) {
k = pos[j - 1].len + 1;
} else {
k = 1;
}
pos[j].pos = i;
pos[j].len = k;
if (k > mk) {
mi = i;
mj = j;
mk = k;
}
}
}
if (mk) {
mi = mi - mk + 1;
mj = mj - mk + 1;
}
while (mi - mb > a1 && mj - mb > b1 &&
a[mi - mb - 1].e == b[mj - mb - 1].e) {
mb++;
}
while (mi + mk < a2 && mj + mk < b2 &&
a[mi + mk].e == b[mj + mk].e) {
mk++;
}
*omi = mi - mb;
*omj = mj - mb;
return mk + mb;
}
static void recurse(struct line *a, struct line *b, struct pos *pos,
int a1, int a2, int b1, int b2, struct hunklist *l)
{
int i, j, k;
k = longest_match(a, b, pos, a1, a2, b1, b2, &i, &j);
if (!k) {
return;
}
recurse(a, b, pos, a1, i, b1, j, l);
l->head->a1 = i;
l->head->a2 = i + k;
l->head->b1 = j;
l->head->b2 = j + k;
l->head++;
recurse(a, b, pos, i + k, a2, j + k, b2, l);
}
static struct hunklist diff(struct line *a, int an, struct line *b, int bn)
{
struct hunklist l;
struct hunk *curr;
struct pos *pos;
int t;
t = equatelines(a, an, b, bn);
pos = (struct pos *)calloc(bn ? bn : 1, sizeof(struct pos));
l.head = l.base = (struct hunk *)malloc(sizeof(struct hunk) *
((an<bn ? an:bn) + 1));
if (pos && l.base && t) {
recurse(a, b, pos, 0, an, 0, bn, &l);
l.head->a1 = l.head->a2 = an;
l.head->b1 = l.head->b2 = bn;
l.head++;
}
free(pos);
for (curr = l.base; curr != l.head; curr++) {
struct hunk *next = curr+1;
int shift = 0;
if (next == l.head) {
break;
}
if (curr->a2 == next->a1) {
while (curr->a2 + shift < an && curr->b2 + shift < bn && !cmp (a + curr->a2 + shift, b + curr->b2 + shift)) {
shift++;
}
} else if (curr->b2 == next->b1) {
while (curr->b2 + shift < bn && curr->a2 + shift < an && !cmp (b + curr->b2 + shift, a + curr->a2 + shift)) {
shift++;
}
}
if (!shift) {
continue;
}
curr->b2 += shift;
next->b1 += shift;
curr->a2 += shift;
next->a1 += shift;
}
return l;
}
R_API int r_diff_buffers_delta(RDiff *d, const ut8 *sa, int la, const ut8 *sb, int lb) {
RDiffOp dop;
struct line *al = NULL;
struct line *bl = NULL;
struct hunklist l = { NULL, NULL };
struct hunk *h;
int an, bn, offa, rlen, offb, len = 0;
int hits = -1;
an = splitlines ((const char *)sa, la, &al);
if (an<0) {
free (al);
return -1;
}
bn = splitlines ((const char *)sb, lb, &bl);
if (bn<0) {
free (al);
free (bl);
return -1;
}
if (!al || !bl) {
eprintf ("bindiff_buffers: Out of memory.\n");
goto beach;
}
l = diff (al, an, bl, bn);
if (!l.head) {
eprintf ("bindiff_buffers: Out of memory.\n");
goto beach;
}
hits = la = lb = 0;
for (h = l.base; h != l.head; h++) {
if (h->a1 != la || h->b1 != lb) {
len = bl[h->b1].l - bl[lb].l;
offa = al[la].l - al->l;
offb = al[h->a1].l - al->l;
rlen = offb-offa;
if (d->callback) {
dop.a_off = offa;
dop.a_buf = (ut8 *)al[la].l;
dop.a_len = rlen;
dop.b_off = offa; 
dop.b_buf = (ut8 *)bl[lb].l;
dop.b_len = len;
if (!d->callback (d, d->user, &dop)) {
break;
}
}
#if 0 
if (rlen > 0) {
printf ("r-%d @ 0x%"PFMT64x"\n", rlen, (ut64)offa);
}
printf ("e file.write=true\n"); 
printf ("wx ");
for(i=0;i<len;i++)
printf ("%02x", bl[lb].l[i]);
printf (" @ 0x%"PFMT64x"\n", (ut64)offa);
rb += 12 + len;
#endif
}
la = h->a2;
lb = h->b2;
}
beach:
free (al);
free (bl);
free (l.base);
return hits;
}
