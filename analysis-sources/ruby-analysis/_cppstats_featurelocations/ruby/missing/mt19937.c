


















































#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfU 
#define UMASK 0x80000000U 
#define LMASK 0x7fffffffU 
#define MIXBITS(u,v) ( ((u) & UMASK) | ((v) & LMASK) )
#define TWIST(u,v) ((MIXBITS((u),(v)) >> 1) ^ ((v)&1U ? MATRIX_A : 0U))

enum {MT_MAX_STATE = N};

struct MT {

uint32_t state[N]; 
uint32_t *next;
int left;
};

#define genrand_initialized(mt) ((mt)->next != 0)
#define uninit_genrand(mt) ((mt)->next = 0)

NO_SANITIZE("unsigned-integer-overflow", static void init_genrand(struct MT *mt, unsigned int s));
NO_SANITIZE("unsigned-integer-overflow", static void init_by_array(struct MT *mt, const uint32_t init_key[], int key_length));


static void
init_genrand(struct MT *mt, unsigned int s)
{
int j;
mt->state[0] = s & 0xffffffffU;
for (j=1; j<N; j++) {
mt->state[j] = (1812433253U * (mt->state[j-1] ^ (mt->state[j-1] >> 30)) + j);




mt->state[j] &= 0xffffffff; 
}
mt->left = 1;
mt->next = mt->state + N;
}





static void
init_by_array(struct MT *mt, const uint32_t init_key[], int key_length)
{
int i, j, k;
init_genrand(mt, 19650218U);
i=1; j=0;
k = (N>key_length ? N : key_length);
for (; k; k--) {
mt->state[i] = (mt->state[i] ^ ((mt->state[i-1] ^ (mt->state[i-1] >> 30)) * 1664525U))
+ init_key[j] + j; 
mt->state[i] &= 0xffffffffU; 
i++; j++;
if (i>=N) { mt->state[0] = mt->state[N-1]; i=1; }
if (j>=key_length) j=0;
}
for (k=N-1; k; k--) {
mt->state[i] = (mt->state[i] ^ ((mt->state[i-1] ^ (mt->state[i-1] >> 30)) * 1566083941U))
- i; 
mt->state[i] &= 0xffffffffU; 
i++;
if (i>=N) { mt->state[0] = mt->state[N-1]; i=1; }
}

mt->state[0] = 0x80000000U; 
}

static void
next_state(struct MT *mt)
{
uint32_t *p = mt->state;
int j;

mt->left = N;
mt->next = mt->state;

for (j=N-M+1; --j; p++)
*p = p[M] ^ TWIST(p[0], p[1]);

for (j=M; --j; p++)
*p = p[M-N] ^ TWIST(p[0], p[1]);

*p = p[M-N] ^ TWIST(p[0], mt->state[0]);
}


static unsigned int
genrand_int32(struct MT *mt)
{

unsigned int y;

if (--mt->left <= 0) next_state(mt);
y = *mt->next++;


y ^= (y >> 11);
y ^= (y << 7) & 0x9d2c5680;
y ^= (y << 15) & 0xefc60000;
y ^= (y >> 18);

return y;
}
