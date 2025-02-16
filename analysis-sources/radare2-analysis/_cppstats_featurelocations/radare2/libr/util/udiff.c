

#include <r_diff.h>


#define USE_SYSTEM_DIFF 1


R_API RDiff *r_diff_new_from(ut64 off_a, ut64 off_b) {
RDiff *d = R_NEW0 (RDiff);
if (d) {
d->delta = 1;
d->user = NULL;
d->off_a = off_a;
d->off_b = off_b;
}
return d;
}

R_API RDiff *r_diff_new() {
return r_diff_new_from (0, 0);
}

R_API RDiff *r_diff_free(RDiff *d) {
free (d);
return NULL;
}

R_API int r_diff_set_callback(RDiff *d, RDiffCallback callback, void *user) {
d->callback = callback;
d->user = user;
return 1;
}

R_API int r_diff_set_delta(RDiff *d, int delta) {
d->delta = delta;
return 1;
}

typedef struct {
RDiff *d;
char *str;
} RDiffUser;

#if USE_SYSTEM_DIFF
R_API char *r_diff_buffers_to_string(RDiff *d, const ut8 *a, int la, const ut8 *b, int lb) {
return r_diff_buffers_unified (d, a, la, b, lb);
}

#else

static int tostring(RDiff *d, void *user, RDiffOp *op) {
RDiffUser *u = (RDiffUser*)user;
if (op->a_len > 0) {
char *a_str = r_str_ndup ((const char *)op->a_buf + op->a_off, op->a_len);
u->str = r_str_appendf (u->str, "+(%s)", a_str);
#if 0
char *bufasm = r_str_prefix_all (a_str, "- ");
u->str = r_str_appendf (u->str, "-(%s)", bufasm);
free (bufasm);
#endif
free (a_str);
}
if (op->b_len > 0) {
char *b_str = r_str_ndup ((const char *)op->b_buf + op->b_off, op->b_len);
u->str = r_str_appendf (u->str, "+(%s)", b_str);
#if 0
char *bufasm = r_str_prefix_all (b_str, "+ ");
u->str = r_str_appendf (u->str, "+(%s)", bufasm);
free (bufasm);
#endif
free (b_str);
}
if (op->a_len == op->b_len) {
char *b_str = r_str_ndup ((const char *)op->a_buf + op->a_off, op->a_len);

u->str = r_str_appendf (u->str, "%s", b_str);

free (b_str);
}
return 1;
}

R_API char *r_diff_buffers_to_string(RDiff *d, const ut8 *a, int la, const ut8 *b, int lb) {

void *c = d->callback;
void *u = d->user;
RDiffUser du = {d, strdup ("")};
d->callback = &tostring;
d->user = &du;
r_diff_buffers_static (d, a, la, b, lb);
d->callback = c;
d->user = u;
return du.str;
}
#endif

#define diffHit() {const size_t i_hit = i - hit;int ra = la - i_hit;int rb = lb - i_hit;struct r_diff_op_t o = {.a_off = d->off_a+i-hit, .a_buf = a+i-hit, .a_len = R_MIN (hit, ra),.b_off = d->off_b+i-hit, .b_buf = b+i-hit, .b_len = R_MIN (hit, rb)};d->callback (d, d->user, &o);}










R_API int r_diff_buffers_static(RDiff *d, const ut8 *a, int la, const ut8 *b, int lb) {
int i, len;
int hit = 0;
la = R_ABS (la);
lb = R_ABS (lb);
if (la != lb) {
len = R_MIN (la, lb);
eprintf ("Buffer truncated to %d byte(s) (%d not compared)\n", len, R_ABS(lb-la));
} else {
len = la;
}
for (i = 0; i < len; i++) {
if (a[i] != b[i]) {
hit++;
} else {
if (hit > 0) {
diffHit ();
hit = 0;
}
}
}
if (hit > 0) {
diffHit ();
}
return 0;
}


R_API char *r_diff_buffers_unified(RDiff *d, const ut8 *a, int la, const ut8 *b, int lb) {
r_file_dump (".a", a, la, 0);
r_file_dump (".b", b, lb, 0);
#if 0
if (r_mem_is_printable (a, R_MIN (5, la))) {
r_file_dump (".a", a, la, 0);
r_file_dump (".b", b, lb, 0);
} else {
r_file_hexdump (".a", a, la, 0);
r_file_hexdump (".b", b, lb, 0);
}
#endif
char* err = NULL;
char* out = NULL;
int out_len;
(void)r_sys_cmd_str_full ("diff -u .a .b", NULL, &out, &out_len, &err);
r_file_rm (".a");
r_file_rm (".b");
free (err);
return out;
}

R_API int r_diff_buffers(RDiff *d, const ut8 *a, ut32 la, const ut8 *b, ut32 lb) {
return d->delta
? r_diff_buffers_delta (d, a, la, b, lb)
: r_diff_buffers_static (d, a, la, b, lb);
}

R_API bool r_diff_buffers_distance_levenstein(RDiff *d, const ut8 *a, ut32 la, const ut8 *b, ut32 lb, ut32 *distance, double *similarity) {
const bool verbose = d? d->verbose: false;











int i, j;
const ut8 *aBufPtr;
const ut8 *bBufPtr;
ut32 aLen;
ut32 bLen;


int *temp;
int *v0, *v1;







int start = 0;
int stop = 0;
int smallest;
int colMin = 0;
int extendStop = 0;
int extendStart = 0;


int cost = 0;



ut64 loops = 0;
















if (la < lb) {
aBufPtr = b;
bBufPtr = a;
aLen = lb;
bLen = la;
} else {
aBufPtr = a;
bBufPtr = b;
aLen = la;
bLen = lb;
}
stop = bLen;



if (!aBufPtr || !bBufPtr || aLen < 1 || bLen < 1) {
return false;
}


if (aLen == bLen && !memcmp (aBufPtr, bBufPtr, aLen)) {
if (distance) {
*distance = 0;
}
if (similarity) {
*similarity = 1.0;
}
return true;
}



v0 = (int*) calloc ((bLen + 3), sizeof (int));
if (!v0) {
eprintf ("Error: cannot allocate %i bytes.", bLen + 3);
return false;
}

v1 = (int*) calloc ((bLen + 3), sizeof (int));
if (!v1) {
eprintf ("Error: cannot allocate %i bytes", 2 * (bLen + 3));
free (v0);
return false;
}



for (i = 0; i < bLen + 1 ; i++) {
v0[i] = i;
v1[i] = i + 1;
}


for (i = 0; i < aLen; i++) {





stop = R_MIN ((i + extendStop + 2), bLen);






if (start > bLen) {
break;
}
v1[start] = v0[start] + 1;


colMin = aLen;


for (j = start; j <= stop; j++) {
loops++;


cost = (aBufPtr[i] == bBufPtr[j]) ? 0 : 1;
smallest = R_MIN ((v1[j] + 1), (v0[j + 1] + 1));
smallest = R_MIN (smallest, (v0[j] + cost));



if (j + 2 > bLen + 3) {
break;
}
v1[j + 1] = smallest;
v1[j + 2] = smallest + 1;


colMin = R_MIN ((colMin), (smallest));

}






start = i + 1 - colMin - extendStart;






if (!cost && aBufPtr[i] != bBufPtr[j - 2]) {
extendStop ++;
}





if (i + 1 < aLen && start < bLen && aBufPtr[i + 1] == bBufPtr[start]) {
start --;
extendStart ++;
}

temp = v0;
v0 = v1;
v1 = temp;


if (verbose && i % 10000==0) {
eprintf ("\rProcessing %d of %d\r", i, aLen);
}
}

if (verbose) {
eprintf ("\rProcessing %d of %d (loops=%"PFMT64d")\n", i, aLen,loops);
}
if (distance) {


*distance = v0[stop];
}
if (similarity) {
double diff = (double) (v0[stop]) / (double) (R_MAX (aLen, bLen));
*similarity = (double)1 - diff;
}
free (v0);
free (v1);
return true;
}



R_API bool r_diff_buffers_distance_myers(RDiff *diff, const ut8 *a, ut32 la, const ut8 *b, ut32 lb, ut32 *distance, double *similarity) {
const bool verbose = diff ? diff->verbose: false;
if (!a || !b) {
return false;
}
const ut32 length = la + lb;
const ut8 *ea = a + la, *eb = b + lb;

for (; a < ea && b < eb && *a == *b; a++, b++) {}

for (; a < ea && b < eb && ea[-1] == eb[-1]; ea--, eb--) {}
la = ea - a;
lb = eb - b;
ut32 *v0, *v;
st64 m = (st64)la + lb, di = 0, low, high, i, x, y;
if (m + 2 > SIZE_MAX / sizeof (st64) || !(v0 = malloc ((m + 2) * sizeof (ut32)))) {
return false;
}
v = v0 + lb;
v[1] = 0;
for (di = 0; di <= m; di++) {
low = -di + 2 * R_MAX (0, di - (st64)lb);
high = di - 2 * R_MAX (0, di - (st64)la);
for (i = low; i <= high; i += 2) {
x = i == -di || (i != di && v[i-1] < v[i+1]) ? v[i+1] : v[i-1] + 1;
y = x - i;
while (x < la && y < lb && a[x] == b[y]) {
x++;
y++;
}
v[i] = x;
if (x == la && y == lb) {
goto out;
}
}
if (verbose && di % 10000 == 0) {
eprintf ("\rProcessing dist %" PFMT64d " of max %" PFMT64d "\r", di, m);
}
}

out:
if (verbose) {
eprintf ("\n");
}
free (v0);

if (distance) {
*distance = di;
}
if (similarity) {
*similarity = length ? 1.0 - (double)di / length : 1.0;
}
return true;
}

R_API bool r_diff_buffers_distance_original(RDiff *diff, const ut8 *a, ut32 la, const ut8 *b, ut32 lb, ut32 *distance, double *similarity) {
if (!a || !b) {
return false;
}

const bool verbose = diff ? diff->verbose : false;
const ut32 length = R_MAX (la, lb);
const ut8 *ea = a + la, *eb = b + lb, *t;
ut32 *d, i, j;

for (; a < ea && b < eb && *a == *b; a++, b++) {}

for (; a < ea && b < eb && ea[-1] == eb[-1]; ea--, eb--) {}
la = ea - a;
lb = eb - b;
if (la < lb) {
i = la;
la = lb;
lb = i;
t = a;
a = b;
b = t;
}

if (sizeof (ut32) > SIZE_MAX / (lb + 1) || !(d = malloc ((lb + 1) * sizeof (ut32)))) {
return false;
}
for (i = 0; i <= lb; i++) {
d[i] = i;
}
for (i = 0; i < la; i++) {
ut32 ul = d[0];
d[0] = i + 1;
for (j = 0; j < lb; j++) {
ut32 u = d[j + 1];
d[j + 1] = a[i] == b[j] ? ul : R_MIN (ul, R_MIN (d[j], u)) + 1;
ul = u;
}
if (verbose && i % 10000 == 0) {
eprintf ("\rProcessing %" PFMT32u " of %" PFMT32u "\r", i, la);
}
}

if (verbose) {
eprintf ("\n");
}
if (distance) {
*distance = d[lb];
}
if (similarity) {
*similarity = length ? 1.0 - (double)d[lb] / length : 1.0;
}
free (d);
return true;
}

R_API bool r_diff_buffers_distance(RDiff *d, const ut8 *a, ut32 la, const ut8 *b, ut32 lb, ut32 *distance, double *similarity) {
if (d) {
switch (d->type) {
case 'm':
return r_diff_buffers_distance_myers (d, a, la, b, lb, distance, similarity);
case 'l':
return r_diff_buffers_distance_levenstein (d, a, la, b, lb, distance, similarity);
default:
break;
}
}
return r_diff_buffers_distance_original (d, a, la, b, lb, distance, similarity);
}
