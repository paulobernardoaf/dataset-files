










#include "ruby/config.h"

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <time.h>

#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#if defined(HAVE_FCNTL_H)
#include <fcntl.h>
#endif

#if defined(HAVE_SYS_TIME_H)
#include <sys/time.h>
#endif

#if defined(HAVE_SYSCALL_H)
#include <syscall.h>
#elif defined HAVE_SYS_SYSCALL_H
#include <sys/syscall.h>
#endif

#if defined(_WIN32)
#include <winsock2.h>
#include <windows.h>
#include <wincrypt.h>
#endif

#if defined(__OpenBSD__)

#include <sys/param.h>
#endif

#if defined HAVE_GETRANDOM
#include <sys/random.h>
#elif defined __linux__ && defined __NR_getrandom
#include <linux/random.h>
#endif

#include "internal.h"
#include "internal/compilers.h"
#include "internal/error.h"
#include "internal/numeric.h"
#include "internal/random.h"
#include "internal/sanitizers.h"
#include "ruby_atomic.h"

typedef int int_must_be_32bit_at_least[sizeof(int) * CHAR_BIT < 32 ? -1 : 1];

#include "missing/mt19937.c"


static double int_pair_to_real_exclusive(uint32_t a, uint32_t b);
static double
genrand_real(struct MT *mt)
{

unsigned int a = genrand_int32(mt), b = genrand_int32(mt);
return int_pair_to_real_exclusive(a, b);
}

static double
int_pair_to_real_exclusive(uint32_t a, uint32_t b)
{
a >>= 5;
b >>= 6;
return(a*67108864.0+b)*(1.0/9007199254740992.0);
}


static double int_pair_to_real_inclusive(uint32_t a, uint32_t b);
#if 0
static double
genrand_real2(struct MT *mt)
{

uint32_t a = genrand_int32(mt), b = genrand_int32(mt);
return int_pair_to_real_inclusive(a, b);
}
#endif



#undef N
#undef M

typedef struct {
VALUE seed;
struct MT mt;
} rb_random_t;

#define DEFAULT_SEED_CNT 4

static rb_random_t default_rand;

static VALUE rand_init(struct MT *mt, VALUE vseed);
static VALUE random_seed(VALUE);

static rb_random_t *
rand_start(rb_random_t *r)
{
struct MT *mt = &r->mt;
if (!genrand_initialized(mt)) {
r->seed = rand_init(mt, random_seed(Qundef));
}
return r;
}

static struct MT *
default_mt(void)
{
return &rand_start(&default_rand)->mt;
}

unsigned int
rb_genrand_int32(void)
{
struct MT *mt = default_mt();
return genrand_int32(mt);
}

double
rb_genrand_real(void)
{
struct MT *mt = default_mt();
return genrand_real(mt);
}

#define SIZEOF_INT32 (31/CHAR_BIT + 1)

static double
int_pair_to_real_inclusive(uint32_t a, uint32_t b)
{
double r;
enum {dig = 53};
enum {dig_u = dig-32, dig_r64 = 64-dig, bmask = ~(~0u<<(dig_r64))};
#if defined HAVE_UINT128_T
const uint128_t m = ((uint128_t)1 << dig) | 1;
uint128_t x = ((uint128_t)a << 32) | b;
r = (double)(uint64_t)((x * m) >> 64);
#elif defined HAVE_UINT64_T && !MSC_VERSION_BEFORE(1300)
uint64_t x = ((uint64_t)a << dig_u) +
(((uint64_t)b + (a >> dig_u)) >> dig_r64);
r = (double)x;
#else

b = (b >> dig_r64) + (((a >> dig_u) + (b & bmask)) >> dig_r64);
r = (double)a * (1 << dig_u) + b;
#endif
return ldexp(r, -dig);
}

VALUE rb_cRandom;
#define id_minus '-'
#define id_plus '+'
static ID id_rand, id_bytes;
NORETURN(static void domain_error(void));


static void
random_mark(void *ptr)
{
rb_gc_mark(((rb_random_t *)ptr)->seed);
}

static void
random_free(void *ptr)
{
if (ptr != &default_rand)
xfree(ptr);
}

static size_t
random_memsize(const void *ptr)
{
return sizeof(rb_random_t);
}

static const rb_data_type_t random_mt_type = {
"random/MT",
{
random_mark,
random_free,
random_memsize,
},
0, 0, RUBY_TYPED_FREE_IMMEDIATELY
};

static rb_random_t *
get_rnd(VALUE obj)
{
rb_random_t *ptr;
TypedData_Get_Struct(obj, rb_random_t, &random_mt_type, ptr);
return rand_start(ptr);
}

static rb_random_t *
try_get_rnd(VALUE obj)
{
if (obj == rb_cRandom) {
return rand_start(&default_rand);
}
if (!rb_typeddata_is_kind_of(obj, &random_mt_type)) return NULL;
return rand_start(DATA_PTR(obj));
}


static VALUE
random_alloc(VALUE klass)
{
rb_random_t *rnd;
VALUE obj = TypedData_Make_Struct(klass, rb_random_t, &random_mt_type, rnd);
rnd->seed = INT2FIX(0);
return obj;
}

static VALUE
rand_init(struct MT *mt, VALUE seed)
{
uint32_t buf0[SIZEOF_LONG / SIZEOF_INT32 * 4], *buf = buf0;
size_t len;
int sign;

len = rb_absint_numwords(seed, 32, NULL);
if (len > numberof(buf0))
buf = ALLOC_N(uint32_t, len);
sign = rb_integer_pack(seed, buf, len, sizeof(uint32_t), 0,
INTEGER_PACK_LSWORD_FIRST|INTEGER_PACK_NATIVE_BYTE_ORDER);
if (sign < 0)
sign = -sign;
if (len == 0) {
buf[0] = 0;
len = 1;
}
if (len <= 1) {
init_genrand(mt, buf[0]);
}
else {
if (sign != 2 && buf[len-1] == 1) 
len--;
init_by_array(mt, buf, (int)len);
}
explicit_bzero(buf, len * sizeof(*buf));
if (buf != buf0) xfree(buf);
return seed;
}










static VALUE
random_init(int argc, VALUE *argv, VALUE obj)
{
VALUE vseed;
rb_random_t *rnd = get_rnd(obj);

if (rb_check_arity(argc, 0, 1) == 0) {
rb_check_frozen(obj);
vseed = random_seed(obj);
}
else {
vseed = argv[0];
rb_check_copyable(obj, vseed);
vseed = rb_to_int(vseed);
}
rnd->seed = rand_init(&rnd->mt, vseed);
return obj;
}

#define DEFAULT_SEED_LEN (DEFAULT_SEED_CNT * (int)sizeof(int32_t))

#if defined(S_ISCHR) && !defined(DOSISH)
#define USE_DEV_URANDOM 1
#else
#define USE_DEV_URANDOM 0
#endif

#if USE_DEV_URANDOM
static int
fill_random_bytes_urandom(void *seed, size_t size)
{





int fd = rb_cloexec_open("/dev/urandom",
#if defined(O_NONBLOCK)
O_NONBLOCK|
#endif
#if defined(O_NOCTTY)
O_NOCTTY|
#endif
O_RDONLY, 0);
struct stat statbuf;
ssize_t ret = 0;
size_t offset = 0;

if (fd < 0) return -1;
rb_update_max_fd(fd);
if (fstat(fd, &statbuf) == 0 && S_ISCHR(statbuf.st_mode)) {
do {
ret = read(fd, ((char*)seed) + offset, size - offset);
if (ret < 0) {
close(fd);
return -1;
}
offset += (size_t)ret;
} while (offset < size);
}
close(fd);
return 0;
}
#else
#define fill_random_bytes_urandom(seed, size) -1
#endif

#if ! defined HAVE_GETRANDOM && defined __linux__ && defined __NR_getrandom
#if !defined(GRND_NONBLOCK)
#define GRND_NONBLOCK 0x0001 
#endif
#define getrandom(ptr, size, flags) (ssize_t)syscall(__NR_getrandom, (ptr), (size), (flags))

#define HAVE_GETRANDOM 1
#endif

#if 0
#elif defined MAC_OS_X_VERSION_10_7 && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_7
#include <Security/Security.h>

static int
fill_random_bytes_syscall(void *seed, size_t size, int unused)
{
int status = SecRandomCopyBytes(kSecRandomDefault, size, seed);

if (status != errSecSuccess) {
#if 0
CFStringRef s = SecCopyErrorMessageString(status, NULL);
const char *m = s ? CFStringGetCStringPtr(s, kCFStringEncodingUTF8) : NULL;
fprintf(stderr, "SecRandomCopyBytes failed: %d: %s\n", status,
m ? m : "unknown");
if (s) CFRelease(s);
#endif
return -1;
}
return 0;
}
#elif defined(HAVE_ARC4RANDOM_BUF)
static int
fill_random_bytes_syscall(void *buf, size_t size, int unused)
{
#if (defined(__OpenBSD__) && OpenBSD >= 201411) || (defined(__NetBSD__) && __NetBSD_Version__ >= 700000000) || (defined(__FreeBSD__) && __FreeBSD_version >= 1200079)


arc4random_buf(buf, size);
return 0;
#else
return -1;
#endif
}
#elif defined(_WIN32)
static void
release_crypt(void *p)
{
HCRYPTPROV prov = (HCRYPTPROV)ATOMIC_PTR_EXCHANGE(*(HCRYPTPROV *)p, INVALID_HANDLE_VALUE);
if (prov && prov != (HCRYPTPROV)INVALID_HANDLE_VALUE) {
CryptReleaseContext(prov, 0);
}
}

static int
fill_random_bytes_syscall(void *seed, size_t size, int unused)
{
static HCRYPTPROV perm_prov;
HCRYPTPROV prov = perm_prov, old_prov;
if (!prov) {
if (!CryptAcquireContext(&prov, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
prov = (HCRYPTPROV)INVALID_HANDLE_VALUE;
}
old_prov = (HCRYPTPROV)ATOMIC_PTR_CAS(perm_prov, 0, prov);
if (LIKELY(!old_prov)) { 
if (prov != (HCRYPTPROV)INVALID_HANDLE_VALUE) {
rb_gc_register_mark_object(Data_Wrap_Struct(0, 0, release_crypt, &perm_prov));
}
}
else { 
if (prov != (HCRYPTPROV)INVALID_HANDLE_VALUE) {
CryptReleaseContext(prov, 0);
}
prov = old_prov;
}
}
if (prov == (HCRYPTPROV)INVALID_HANDLE_VALUE) return -1;
CryptGenRandom(prov, size, seed);
return 0;
}
#elif defined HAVE_GETRANDOM
static int
fill_random_bytes_syscall(void *seed, size_t size, int need_secure)
{
static rb_atomic_t try_syscall = 1;
if (try_syscall) {
size_t offset = 0;
int flags = 0;
if (!need_secure)
flags = GRND_NONBLOCK;
do {
errno = 0;
ssize_t ret = getrandom(((char*)seed) + offset, size - offset, flags);
if (ret == -1) {
ATOMIC_SET(try_syscall, 0);
return -1;
}
offset += (size_t)ret;
} while (offset < size);
return 0;
}
return -1;
}
#else
#define fill_random_bytes_syscall(seed, size, need_secure) -1
#endif

int
ruby_fill_random_bytes(void *seed, size_t size, int need_secure)
{
int ret = fill_random_bytes_syscall(seed, size, need_secure);
if (ret == 0) return ret;
return fill_random_bytes_urandom(seed, size);
}

#define fill_random_bytes ruby_fill_random_bytes


static void
fill_random_seed(uint32_t *seed, size_t cnt)
{
static int n = 0;
#if defined HAVE_CLOCK_GETTIME
struct timespec tv;
#elif defined HAVE_GETTIMEOFDAY
struct timeval tv;
#endif
size_t len = cnt * sizeof(*seed);

memset(seed, 0, len);

fill_random_bytes(seed, len, FALSE);

#if defined HAVE_CLOCK_GETTIME
clock_gettime(CLOCK_REALTIME, &tv);
seed[0] ^= tv.tv_nsec;
#elif defined HAVE_GETTIMEOFDAY
gettimeofday(&tv, 0);
seed[0] ^= tv.tv_usec;
#endif
seed[1] ^= (uint32_t)tv.tv_sec;
#if SIZEOF_TIME_T > SIZEOF_INT
seed[0] ^= (uint32_t)((time_t)tv.tv_sec >> SIZEOF_INT * CHAR_BIT);
#endif
seed[2] ^= getpid() ^ (n++ << 16);
seed[3] ^= (uint32_t)(VALUE)&seed;
#if SIZEOF_VOIDP > SIZEOF_INT
seed[2] ^= (uint32_t)((VALUE)&seed >> SIZEOF_INT * CHAR_BIT);
#endif
}

static VALUE
make_seed_value(uint32_t *ptr, size_t len)
{
VALUE seed;

if (ptr[len-1] <= 1) {

ptr[len++] = 1;
}

seed = rb_integer_unpack(ptr, len, sizeof(uint32_t), 0,
INTEGER_PACK_LSWORD_FIRST|INTEGER_PACK_NATIVE_BYTE_ORDER);

return seed;
}

#define with_random_seed(size, add) for (uint32_t seedbuf[(size)+(add)], loop = (fill_random_seed(seedbuf, (size)), 1); loop; explicit_bzero(seedbuf, (size)*sizeof(seedbuf[0])), loop = 0)











static VALUE
random_seed(VALUE _)
{
VALUE v;
with_random_seed(DEFAULT_SEED_CNT, 1) {
v = make_seed_value(seedbuf, DEFAULT_SEED_CNT);
}
return v;
}

















static VALUE
random_raw_seed(VALUE self, VALUE size)
{
long n = NUM2ULONG(size);
VALUE buf = rb_str_new(0, n);
if (n == 0) return buf;
if (fill_random_bytes(RSTRING_PTR(buf), n, TRUE))
rb_raise(rb_eRuntimeError, "failed to get urandom");
return buf;
}















static VALUE
random_get_seed(VALUE obj)
{
return get_rnd(obj)->seed;
}


static VALUE
random_copy(VALUE obj, VALUE orig)
{
rb_random_t *rnd1, *rnd2;
struct MT *mt;

if (!OBJ_INIT_COPY(obj, orig)) return obj;

rnd1 = get_rnd(obj);
rnd2 = get_rnd(orig);
mt = &rnd1->mt;

*rnd1 = *rnd2;
mt->next = mt->state + numberof(mt->state) - mt->left + 1;
return obj;
}

static VALUE
mt_state(const struct MT *mt)
{
return rb_integer_unpack(mt->state, numberof(mt->state),
sizeof(*mt->state), 0,
INTEGER_PACK_LSWORD_FIRST|INTEGER_PACK_NATIVE_BYTE_ORDER);
}


static VALUE
random_state(VALUE obj)
{
rb_random_t *rnd = get_rnd(obj);
return mt_state(&rnd->mt);
}


static VALUE
random_s_state(VALUE klass)
{
return mt_state(&default_rand.mt);
}


static VALUE
random_left(VALUE obj)
{
rb_random_t *rnd = get_rnd(obj);
return INT2FIX(rnd->mt.left);
}


static VALUE
random_s_left(VALUE klass)
{
return INT2FIX(default_rand.mt.left);
}


static VALUE
random_dump(VALUE obj)
{
rb_random_t *rnd = get_rnd(obj);
VALUE dump = rb_ary_new2(3);

rb_ary_push(dump, mt_state(&rnd->mt));
rb_ary_push(dump, INT2FIX(rnd->mt.left));
rb_ary_push(dump, rnd->seed);

return dump;
}


static VALUE
random_load(VALUE obj, VALUE dump)
{
rb_random_t *rnd = get_rnd(obj);
struct MT *mt = &rnd->mt;
VALUE state, left = INT2FIX(1), seed = INT2FIX(0);
unsigned long x;

rb_check_copyable(obj, dump);
Check_Type(dump, T_ARRAY);
switch (RARRAY_LEN(dump)) {
case 3:
seed = RARRAY_AREF(dump, 2);
case 2:
left = RARRAY_AREF(dump, 1);
case 1:
state = RARRAY_AREF(dump, 0);
break;
default:
rb_raise(rb_eArgError, "wrong dump data");
}
rb_integer_pack(state, mt->state, numberof(mt->state),
sizeof(*mt->state), 0,
INTEGER_PACK_LSWORD_FIRST|INTEGER_PACK_NATIVE_BYTE_ORDER);
x = NUM2ULONG(left);
if (x > numberof(mt->state)) {
rb_raise(rb_eArgError, "wrong value");
}
mt->left = (unsigned int)x;
mt->next = mt->state + numberof(mt->state) - x + 1;
rnd->seed = rb_to_int(seed);

return obj;
}
























static VALUE
rb_f_srand(int argc, VALUE *argv, VALUE obj)
{
VALUE seed, old;
rb_random_t *r = &default_rand;

if (rb_check_arity(argc, 0, 1) == 0) {
seed = random_seed(obj);
}
else {
seed = rb_to_int(argv[0]);
}
old = r->seed;
r->seed = rand_init(&r->mt, seed);

return old;
}

static unsigned long
make_mask(unsigned long x)
{
x = x | x >> 1;
x = x | x >> 2;
x = x | x >> 4;
x = x | x >> 8;
x = x | x >> 16;
#if 4 < SIZEOF_LONG
x = x | x >> 32;
#endif
return x;
}

static unsigned long
limited_rand(struct MT *mt, unsigned long limit)
{

unsigned long val, mask;

if (!limit) return 0;
mask = make_mask(limit);

#if 4 < SIZEOF_LONG
if (0xffffffff < limit) {
int i;
retry:
val = 0;
for (i = SIZEOF_LONG/SIZEOF_INT32-1; 0 <= i; i--) {
if ((mask >> (i * 32)) & 0xffffffff) {
val |= (unsigned long)genrand_int32(mt) << (i * 32);
val &= mask;
if (limit < val)
goto retry;
}
}
return val;
}
#endif

do {
val = genrand_int32(mt) & mask;
} while (limit < val);
return val;
}

static VALUE
limited_big_rand(struct MT *mt, VALUE limit)
{


uint32_t mask;
long i;
int boundary;

size_t len;
uint32_t *tmp, *lim_array, *rnd_array;
VALUE vtmp;
VALUE val;

len = rb_absint_numwords(limit, 32, NULL);
tmp = ALLOCV_N(uint32_t, vtmp, len*2);
lim_array = tmp;
rnd_array = tmp + len;
rb_integer_pack(limit, lim_array, len, sizeof(uint32_t), 0,
INTEGER_PACK_LSWORD_FIRST|INTEGER_PACK_NATIVE_BYTE_ORDER);

retry:
mask = 0;
boundary = 1;
for (i = len-1; 0 <= i; i--) {
uint32_t rnd;
uint32_t lim = lim_array[i];
mask = mask ? 0xffffffff : (uint32_t)make_mask(lim);
if (mask) {
rnd = genrand_int32(mt) & mask;
if (boundary) {
if (lim < rnd)
goto retry;
if (rnd < lim)
boundary = 0;
}
}
else {
rnd = 0;
}
rnd_array[i] = rnd;
}
val = rb_integer_unpack(rnd_array, len, sizeof(uint32_t), 0,
INTEGER_PACK_LSWORD_FIRST|INTEGER_PACK_NATIVE_BYTE_ORDER);
ALLOCV_END(vtmp);

return val;
}







unsigned long
rb_genrand_ulong_limited(unsigned long limit)
{
return limited_rand(default_mt(), limit);
}

static VALUE
obj_random_bytes(VALUE obj, void *p, long n)
{
VALUE len = LONG2NUM(n);
VALUE v = rb_funcallv_public(obj, id_bytes, 1, &len);
long l;
Check_Type(v, T_STRING);
l = RSTRING_LEN(v);
if (l < n)
rb_raise(rb_eRangeError, "random data too short %ld", l);
else if (l > n)
rb_raise(rb_eRangeError, "random data too long %ld", l);
if (p) memcpy(p, RSTRING_PTR(v), n);
return v;
}

static unsigned int
random_int32(rb_random_t *rnd)
{
return genrand_int32(&rnd->mt);
}

unsigned int
rb_random_int32(VALUE obj)
{
rb_random_t *rnd = try_get_rnd(obj);
if (!rnd) {
uint32_t x;
obj_random_bytes(obj, &x, sizeof(x));
return (unsigned int)x;
}
return random_int32(rnd);
}

static double
random_real(VALUE obj, rb_random_t *rnd, int excl)
{
uint32_t a, b;

if (!rnd) {
uint32_t x[2] = {0, 0};
obj_random_bytes(obj, x, sizeof(x));
a = x[0];
b = x[1];
}
else {
a = random_int32(rnd);
b = random_int32(rnd);
}
if (excl) {
return int_pair_to_real_exclusive(a, b);
}
else {
return int_pair_to_real_inclusive(a, b);
}
}

double
rb_random_real(VALUE obj)
{
rb_random_t *rnd = try_get_rnd(obj);
if (!rnd) {
VALUE v = rb_funcallv(obj, id_rand, 0, 0);
double d = NUM2DBL(v);
if (d < 0.0) {
rb_raise(rb_eRangeError, "random number too small %g", d);
}
else if (d >= 1.0) {
rb_raise(rb_eRangeError, "random number too big %g", d);
}
return d;
}
return genrand_real(&rnd->mt);
}

static inline VALUE
ulong_to_num_plus_1(unsigned long n)
{
#if HAVE_LONG_LONG
return ULL2NUM((LONG_LONG)n+1);
#else
if (n >= ULONG_MAX) {
return rb_big_plus(ULONG2NUM(n), INT2FIX(1));
}
return ULONG2NUM(n+1);
#endif
}

static unsigned long
random_ulong_limited(VALUE obj, rb_random_t *rnd, unsigned long limit)
{
if (!limit) return 0;
if (!rnd) {
const int w = sizeof(limit) * CHAR_BIT - nlz_long(limit);
const int n = w > 32 ? sizeof(unsigned long) : sizeof(uint32_t);
const unsigned long mask = ~(~0UL << w);
const unsigned long full =
(size_t)n >= sizeof(unsigned long) ? ~0UL :
~(~0UL << n * CHAR_BIT);
unsigned long val, bits = 0, rest = 0;
do {
if (mask & ~rest) {
union {uint32_t u32; unsigned long ul;} buf;
obj_random_bytes(obj, &buf, n);
rest = full;
bits = (n == sizeof(uint32_t)) ? buf.u32 : buf.ul;
}
val = bits;
bits >>= w;
rest >>= w;
val &= mask;
} while (limit < val);
return val;
}
return limited_rand(&rnd->mt, limit);
}

unsigned long
rb_random_ulong_limited(VALUE obj, unsigned long limit)
{
rb_random_t *rnd = try_get_rnd(obj);
if (!rnd) {
VALUE lim = ulong_to_num_plus_1(limit);
VALUE v = rb_to_int(rb_funcallv_public(obj, id_rand, 1, &lim));
unsigned long r = NUM2ULONG(v);
if (rb_num_negative_p(v)) {
rb_raise(rb_eRangeError, "random number too small %ld", r);
}
if (r > limit) {
rb_raise(rb_eRangeError, "random number too big %ld", r);
}
return r;
}
return limited_rand(&rnd->mt, limit);
}

static VALUE
random_ulong_limited_big(VALUE obj, rb_random_t *rnd, VALUE vmax)
{
if (!rnd) {
VALUE v, vtmp;
size_t i, nlz, len = rb_absint_numwords(vmax, 32, &nlz);
uint32_t *tmp = ALLOCV_N(uint32_t, vtmp, len * 2);
uint32_t mask = (uint32_t)~0 >> nlz;
uint32_t *lim_array = tmp;
uint32_t *rnd_array = tmp + len;
int flag = INTEGER_PACK_MSWORD_FIRST|INTEGER_PACK_NATIVE_BYTE_ORDER;
rb_integer_pack(vmax, lim_array, len, sizeof(uint32_t), 0, flag);

retry:
obj_random_bytes(obj, rnd_array, len * sizeof(uint32_t));
rnd_array[0] &= mask;
for (i = 0; i < len; ++i) {
if (lim_array[i] < rnd_array[i])
goto retry;
if (rnd_array[i] < lim_array[i])
break;
}
v = rb_integer_unpack(rnd_array, len, sizeof(uint32_t), 0, flag);
ALLOCV_END(vtmp);
return v;
}
return limited_big_rand(&rnd->mt, vmax);
}

static VALUE genrand_bytes(rb_random_t *rnd, long n);









static VALUE
random_bytes(VALUE obj, VALUE len)
{
return genrand_bytes(get_rnd(obj), NUM2LONG(rb_to_int(len)));
}

static VALUE
genrand_bytes(rb_random_t *rnd, long n)
{
VALUE bytes;
char *ptr;
unsigned int r, i;

bytes = rb_str_new(0, n);
ptr = RSTRING_PTR(bytes);
for (; n >= SIZEOF_INT32; n -= SIZEOF_INT32) {
r = genrand_int32(&rnd->mt);
i = SIZEOF_INT32;
do {
*ptr++ = (char)r;
r >>= CHAR_BIT;
} while (--i);
}
if (n > 0) {
r = genrand_int32(&rnd->mt);
do {
*ptr++ = (char)r;
r >>= CHAR_BIT;
} while (--n);
}
return bytes;
}

VALUE
rb_random_bytes(VALUE obj, long n)
{
rb_random_t *rnd = try_get_rnd(obj);
if (!rnd) {
return obj_random_bytes(obj, NULL, n);
}
return genrand_bytes(rnd, n);
}







static VALUE
random_s_bytes(VALUE obj, VALUE len)
{
rb_random_t *rnd = rand_start(&default_rand);
return genrand_bytes(rnd, NUM2LONG(rb_to_int(len)));
}

static VALUE
range_values(VALUE vmax, VALUE *begp, VALUE *endp, int *exclp)
{
VALUE beg, end;

if (!rb_range_values(vmax, &beg, &end, exclp)) return Qfalse;
if (begp) *begp = beg;
if (NIL_P(beg)) return Qnil;
if (endp) *endp = end;
if (NIL_P(end)) return Qnil;
return rb_check_funcall_default(end, id_minus, 1, begp, Qfalse);
}

static VALUE
rand_int(VALUE obj, rb_random_t *rnd, VALUE vmax, int restrictive)
{

unsigned long r;

if (FIXNUM_P(vmax)) {
long max = FIX2LONG(vmax);
if (!max) return Qnil;
if (max < 0) {
if (restrictive) return Qnil;
max = -max;
}
r = random_ulong_limited(obj, rnd, (unsigned long)max - 1);
return ULONG2NUM(r);
}
else {
VALUE ret;
if (rb_bigzero_p(vmax)) return Qnil;
if (!BIGNUM_SIGN(vmax)) {
if (restrictive) return Qnil;
vmax = rb_big_uminus(vmax);
}
vmax = rb_big_minus(vmax, INT2FIX(1));
if (FIXNUM_P(vmax)) {
long max = FIX2LONG(vmax);
if (max == -1) return Qnil;
r = random_ulong_limited(obj, rnd, max);
return LONG2NUM(r);
}
ret = random_ulong_limited_big(obj, rnd, vmax);
RB_GC_GUARD(vmax);
return ret;
}
}

static void
domain_error(void)
{
VALUE error = INT2FIX(EDOM);
rb_exc_raise(rb_class_new_instance(1, &error, rb_eSystemCallError));
}

NORETURN(static void invalid_argument(VALUE));
static void
invalid_argument(VALUE arg0)
{
rb_raise(rb_eArgError, "invalid argument - %"PRIsVALUE, arg0);
}

static VALUE
check_random_number(VALUE v, const VALUE *argv)
{
switch (v) {
case Qfalse:
(void)NUM2LONG(argv[0]);
break;
case Qnil:
invalid_argument(argv[0]);
}
return v;
}

static inline double
float_value(VALUE v)
{
double x = RFLOAT_VALUE(v);
if (isinf(x) || isnan(x)) {
domain_error();
}
return x;
}

static inline VALUE
rand_range(VALUE obj, rb_random_t* rnd, VALUE range)
{
VALUE beg = Qundef, end = Qundef, vmax, v;
int excl = 0;

if ((v = vmax = range_values(range, &beg, &end, &excl)) == Qfalse)
return Qfalse;
if (NIL_P(v)) domain_error();
if (!RB_TYPE_P(vmax, T_FLOAT) && (v = rb_check_to_int(vmax), !NIL_P(v))) {
long max;
vmax = v;
v = Qnil;
if (FIXNUM_P(vmax)) {
fixnum:
if ((max = FIX2LONG(vmax) - excl) >= 0) {
unsigned long r = random_ulong_limited(obj, rnd, (unsigned long)max);
v = ULONG2NUM(r);
}
}
else if (BUILTIN_TYPE(vmax) == T_BIGNUM && BIGNUM_SIGN(vmax) && !rb_bigzero_p(vmax)) {
vmax = excl ? rb_big_minus(vmax, INT2FIX(1)) : rb_big_norm(vmax);
if (FIXNUM_P(vmax)) {
excl = 0;
goto fixnum;
}
v = random_ulong_limited_big(obj, rnd, vmax);
}
}
else if (v = rb_check_to_float(vmax), !NIL_P(v)) {
int scale = 1;
double max = RFLOAT_VALUE(v), mid = 0.5, r;
if (isinf(max)) {
double min = float_value(rb_to_float(beg)) / 2.0;
max = float_value(rb_to_float(end)) / 2.0;
scale = 2;
mid = max + min;
max -= min;
}
else if (isnan(max)) {
domain_error();
}
v = Qnil;
if (max > 0.0) {
r = random_real(obj, rnd, excl);
if (scale > 1) {
return rb_float_new(+(+(+(r - 0.5) * max) * scale) + mid);
}
v = rb_float_new(r * max);
}
else if (max == 0.0 && !excl) {
v = rb_float_new(0.0);
}
}

if (FIXNUM_P(beg) && FIXNUM_P(v)) {
long x = FIX2LONG(beg) + FIX2LONG(v);
return LONG2NUM(x);
}
switch (TYPE(v)) {
case T_NIL:
break;
case T_BIGNUM:
return rb_big_plus(v, beg);
case T_FLOAT: {
VALUE f = rb_check_to_float(beg);
if (!NIL_P(f)) {
return DBL2NUM(RFLOAT_VALUE(v) + RFLOAT_VALUE(f));
}
}
default:
return rb_funcallv(beg, id_plus, 1, &v);
}

return v;
}

static VALUE rand_random(int argc, VALUE *argv, VALUE obj, rb_random_t *rnd);






























static VALUE
random_rand(int argc, VALUE *argv, VALUE obj)
{
VALUE v = rand_random(argc, argv, obj, get_rnd(obj));
check_random_number(v, argv);
return v;
}

static VALUE
rand_random(int argc, VALUE *argv, VALUE obj, rb_random_t *rnd)
{
VALUE vmax, v;

if (rb_check_arity(argc, 0, 1) == 0) {
return rb_float_new(random_real(obj, rnd, TRUE));
}
vmax = argv[0];
if (NIL_P(vmax)) return Qnil;
if (!RB_TYPE_P(vmax, T_FLOAT)) {
v = rb_check_to_int(vmax);
if (!NIL_P(v)) return rand_int(obj, rnd, v, 1);
}
v = rb_check_to_float(vmax);
if (!NIL_P(v)) {
const double max = float_value(v);
if (max < 0.0) {
return Qnil;
}
else {
double r = random_real(obj, rnd, TRUE);
if (max > 0.0) r *= max;
return rb_float_new(r);
}
}
return rand_range(obj, rnd, vmax);
}











static VALUE
rand_random_number(int argc, VALUE *argv, VALUE obj)
{
rb_random_t *rnd = try_get_rnd(obj);
VALUE v = rand_random(argc, argv, obj, rnd);
if (NIL_P(v)) v = rand_random(0, 0, obj, rnd);
else if (!v) invalid_argument(argv[0]);
return v;
}

























static VALUE
random_equal(VALUE self, VALUE other)
{
rb_random_t *r1, *r2;
if (rb_obj_class(self) != rb_obj_class(other)) return Qfalse;
r1 = get_rnd(self);
r2 = get_rnd(other);
if (memcmp(r1->mt.state, r2->mt.state, sizeof(r1->mt.state))) return Qfalse;
if ((r1->mt.next - r1->mt.state) != (r2->mt.next - r2->mt.state)) return Qfalse;
if (r1->mt.left != r2->mt.left) return Qfalse;
return rb_equal(r1->seed, r2->seed);
}
































static VALUE
rb_f_rand(int argc, VALUE *argv, VALUE obj)
{
VALUE vmax;
rb_random_t *rnd = rand_start(&default_rand);

if (rb_check_arity(argc, 0, 1) && !NIL_P(vmax = argv[0])) {
VALUE v = rand_range(Qnil, rnd, vmax);
if (v != Qfalse) return v;
vmax = rb_to_int(vmax);
if (vmax != INT2FIX(0)) {
v = rand_int(Qnil, rnd, vmax, 0);
if (!NIL_P(v)) return v;
}
}
return DBL2NUM(genrand_real(&rnd->mt));
}









static VALUE
random_s_rand(int argc, VALUE *argv, VALUE obj)
{
VALUE v = rand_random(argc, argv, Qnil, rand_start(&default_rand));
check_random_number(v, argv);
return v;
}

#define SIP_HASH_STREAMING 0
#define sip_hash13 ruby_sip_hash13
#if !defined _WIN32 && !defined BYTE_ORDER
#if defined(WORDS_BIGENDIAN)
#define BYTE_ORDER BIG_ENDIAN
#else
#define BYTE_ORDER LITTLE_ENDIAN
#endif
#if !defined(LITTLE_ENDIAN)
#define LITTLE_ENDIAN 1234
#endif
#if !defined(BIG_ENDIAN)
#define BIG_ENDIAN 4321
#endif
#endif
#include "siphash.c"

typedef struct {
st_index_t hash;
uint8_t sip[16];
} hash_salt_t;

static union {
hash_salt_t key;
uint32_t u32[type_roomof(hash_salt_t, uint32_t)];
} hash_salt;

static void
init_hash_salt(struct MT *mt)
{
int i;

for (i = 0; i < numberof(hash_salt.u32); ++i)
hash_salt.u32[i] = genrand_int32(mt);
}

NO_SANITIZE("unsigned-integer-overflow", extern st_index_t rb_hash_start(st_index_t h));
st_index_t
rb_hash_start(st_index_t h)
{
return st_hash_start(hash_salt.key.hash + h);
}

st_index_t
rb_memhash(const void *ptr, long len)
{
sip_uint64_t h = sip_hash13(hash_salt.key.sip, ptr, len);
#if defined(HAVE_UINT64_T)
return (st_index_t)h;
#else
return (st_index_t)(h.u32[0] ^ h.u32[1]);
#endif
}



void
Init_RandomSeedCore(void)
{
if (!fill_random_bytes(&hash_salt, sizeof(hash_salt), FALSE)) return;








struct MT mt;

with_random_seed(DEFAULT_SEED_CNT, 0) {
init_by_array(&mt, seedbuf, DEFAULT_SEED_CNT);
}

init_hash_salt(&mt);
explicit_bzero(&mt, sizeof(mt));
}


static VALUE
Init_Random_default(VALUE klass)
{
rb_random_t *r = &default_rand;
struct MT *mt = &r->mt;
VALUE v = TypedData_Wrap_Struct(klass, &random_mt_type, r);

rb_gc_register_mark_object(v);
with_random_seed(DEFAULT_SEED_CNT, 1) {
init_by_array(mt, seedbuf, DEFAULT_SEED_CNT);
r->seed = make_seed_value(seedbuf, DEFAULT_SEED_CNT);
}

return v;
}

void
rb_reset_random_seed(void)
{
rb_random_t *r = &default_rand;
uninit_genrand(&r->mt);
r->seed = INT2FIX(0);
}

























void
InitVM_Random(void)
{
rb_define_global_function("srand", rb_f_srand, -1);
rb_define_global_function("rand", rb_f_rand, -1);

rb_cRandom = rb_define_class("Random", rb_cObject);
rb_define_alloc_func(rb_cRandom, random_alloc);
rb_define_method(rb_cRandom, "initialize", random_init, -1);
rb_define_method(rb_cRandom, "rand", random_rand, -1);
rb_define_method(rb_cRandom, "bytes", random_bytes, 1);
rb_define_method(rb_cRandom, "seed", random_get_seed, 0);
rb_define_method(rb_cRandom, "initialize_copy", random_copy, 1);
rb_define_private_method(rb_cRandom, "marshal_dump", random_dump, 0);
rb_define_private_method(rb_cRandom, "marshal_load", random_load, 1);
rb_define_private_method(rb_cRandom, "state", random_state, 0);
rb_define_private_method(rb_cRandom, "left", random_left, 0);
rb_define_method(rb_cRandom, "==", random_equal, 1);

{

VALUE rand_default = Init_Random_default(rb_cRandom);


rb_define_const(rb_cRandom, "DEFAULT", rand_default);
}

rb_define_singleton_method(rb_cRandom, "srand", rb_f_srand, -1);
rb_define_singleton_method(rb_cRandom, "rand", random_s_rand, -1);
rb_define_singleton_method(rb_cRandom, "bytes", random_s_bytes, 1);
rb_define_singleton_method(rb_cRandom, "new_seed", random_seed, 0);
rb_define_singleton_method(rb_cRandom, "urandom", random_raw_seed, 1);
rb_define_private_method(CLASS_OF(rb_cRandom), "state", random_s_state, 0);
rb_define_private_method(CLASS_OF(rb_cRandom), "left", random_s_left, 0);

{

VALUE m = rb_define_module_under(rb_cRandom, "Formatter");
rb_include_module(rb_cRandom, m);
rb_extend_object(rb_cRandom, m);
rb_define_method(m, "random_number", rand_random_number, -1);
rb_define_method(m, "rand", rand_random_number, -1);
}
}

#undef rb_intern
void
Init_Random(void)
{
id_rand = rb_intern("rand");
id_bytes = rb_intern("bytes");

InitVM(Random);
}
