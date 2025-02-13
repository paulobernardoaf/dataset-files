



































#include "../defs.h"
#include <string.h> 
#include <assert.h> 
#include "sha2.h"


















































#if !defined(BYTE_ORDER) || (BYTE_ORDER != LITTLE_ENDIAN && BYTE_ORDER != BIG_ENDIAN)
#error Define BYTE_ORDER to be equal to either LITTLE_ENDIAN or BIG_ENDIAN
#endif















#if defined(SHA2_USE_INTTYPES_H)

typedef uint8_t sha2_byte; 
typedef uint32_t sha2_word32; 
typedef uint64_t sha2_word64; 

#else 

typedef u_int8_t sha2_byte; 
typedef u_int32_t sha2_word32; 
typedef u_int64_t sha2_word64; 

#endif 




#define SHA256_SHORT_BLOCK_LENGTH (SHA256_BLOCK_LENGTH - 8)
#define SHA384_SHORT_BLOCK_LENGTH (SHA384_BLOCK_LENGTH - 16)
#define SHA512_SHORT_BLOCK_LENGTH (SHA512_BLOCK_LENGTH - 16)


#if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)) || defined(__GNUC__) || defined(_HPUX_SOURCE) || defined(__IBMC__)
#define ULL(number) number##ULL
#else
#define ULL(number) (uint64_t)(number)
#endif

#if BYTE_ORDER == LITTLE_ENDIAN
#define REVERSE32(w,x) { sha2_word32 tmp = (w); tmp = (tmp >> 16) | (tmp << 16); (x) = ((tmp & (sha2_word32)0xff00ff00UL) >> 8) | ((tmp & (sha2_word32)0x00ff00ffUL) << 8); }




#define REVERSE64(w,x) { sha2_word64 tmp = (w); tmp = (tmp >> 32) | (tmp << 32); tmp = ((tmp & ULL(0xff00ff00ff00ff00)) >> 8) | ((tmp & ULL(0x00ff00ff00ff00ff)) << 8); (x) = ((tmp & ULL(0xffff0000ffff0000)) >> 16) | ((tmp & ULL(0x0000ffff0000ffff)) << 16); }







#endif 






#define ADDINC128(w,n) { (w)[0] += (sha2_word64)(n); if ((w)[0] < (n)) { (w)[1]++; } }















#if !defined(SHA2_USE_MEMSET_MEMCPY) && !defined(SHA2_USE_BZERO_BCOPY)

#define SHA2_USE_MEMSET_MEMCPY 1
#endif
#if defined(SHA2_USE_MEMSET_MEMCPY) && defined(SHA2_USE_BZERO_BCOPY)

#error Define either SHA2_USE_MEMSET_MEMCPY or SHA2_USE_BZERO_BCOPY, not both!
#endif

#if defined(SHA2_USE_MEMSET_MEMCPY)
#define MEMSET_BZERO(p,l) memset((p), 0, (l))
#define MEMCPY_BCOPY(d,s,l) memcpy((d), (s), (l))
#endif
#if defined(SHA2_USE_BZERO_BCOPY)
#define MEMSET_BZERO(p,l) bzero((p), (l))
#define MEMCPY_BCOPY(d,s,l) bcopy((s), (d), (l))
#endif












#define R(b,x) ((x) >> (b))

#define S32(b,x) (((x) >> (b)) | ((x) << (32 - (b))))

#define S64(b,x) (((x) >> (b)) | ((x) << (64 - (b))))


#define Ch(x,y,z) (((x) & (y)) ^ ((~(x)) & (z)))
#define Maj(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))


#define Sigma0_256(x) (S32(2, (x)) ^ S32(13, (x)) ^ S32(22, (x)))
#define Sigma1_256(x) (S32(6, (x)) ^ S32(11, (x)) ^ S32(25, (x)))
#define sigma0_256(x) (S32(7, (x)) ^ S32(18, (x)) ^ R(3 , (x)))
#define sigma1_256(x) (S32(17, (x)) ^ S32(19, (x)) ^ R(10, (x)))


#define Sigma0_512(x) (S64(28, (x)) ^ S64(34, (x)) ^ S64(39, (x)))
#define Sigma1_512(x) (S64(14, (x)) ^ S64(18, (x)) ^ S64(41, (x)))
#define sigma0_512(x) (S64( 1, (x)) ^ S64( 8, (x)) ^ R( 7, (x)))
#define sigma1_512(x) (S64(19, (x)) ^ S64(61, (x)) ^ R( 6, (x)))






void SHA512_Last(SHA512_CTX*);
void SHA256_Transform(SHA256_CTX*, const sha2_word32*);
void SHA512_Transform(SHA512_CTX*, const sha2_word64*);




static const sha2_word32 K256[64] = {
0x428a2f98UL, 0x71374491UL, 0xb5c0fbcfUL, 0xe9b5dba5UL,
0x3956c25bUL, 0x59f111f1UL, 0x923f82a4UL, 0xab1c5ed5UL,
0xd807aa98UL, 0x12835b01UL, 0x243185beUL, 0x550c7dc3UL,
0x72be5d74UL, 0x80deb1feUL, 0x9bdc06a7UL, 0xc19bf174UL,
0xe49b69c1UL, 0xefbe4786UL, 0x0fc19dc6UL, 0x240ca1ccUL,
0x2de92c6fUL, 0x4a7484aaUL, 0x5cb0a9dcUL, 0x76f988daUL,
0x983e5152UL, 0xa831c66dUL, 0xb00327c8UL, 0xbf597fc7UL,
0xc6e00bf3UL, 0xd5a79147UL, 0x06ca6351UL, 0x14292967UL,
0x27b70a85UL, 0x2e1b2138UL, 0x4d2c6dfcUL, 0x53380d13UL,
0x650a7354UL, 0x766a0abbUL, 0x81c2c92eUL, 0x92722c85UL,
0xa2bfe8a1UL, 0xa81a664bUL, 0xc24b8b70UL, 0xc76c51a3UL,
0xd192e819UL, 0xd6990624UL, 0xf40e3585UL, 0x106aa070UL,
0x19a4c116UL, 0x1e376c08UL, 0x2748774cUL, 0x34b0bcb5UL,
0x391c0cb3UL, 0x4ed8aa4aUL, 0x5b9cca4fUL, 0x682e6ff3UL,
0x748f82eeUL, 0x78a5636fUL, 0x84c87814UL, 0x8cc70208UL,
0x90befffaUL, 0xa4506cebUL, 0xbef9a3f7UL, 0xc67178f2UL
};


static const sha2_word32 sha256_initial_hash_value[8] = {
0x6a09e667UL,
0xbb67ae85UL,
0x3c6ef372UL,
0xa54ff53aUL,
0x510e527fUL,
0x9b05688cUL,
0x1f83d9abUL,
0x5be0cd19UL
};


static const sha2_word64 K512[80] = {
ULL(0x428a2f98d728ae22), ULL(0x7137449123ef65cd),
ULL(0xb5c0fbcfec4d3b2f), ULL(0xe9b5dba58189dbbc),
ULL(0x3956c25bf348b538), ULL(0x59f111f1b605d019),
ULL(0x923f82a4af194f9b), ULL(0xab1c5ed5da6d8118),
ULL(0xd807aa98a3030242), ULL(0x12835b0145706fbe),
ULL(0x243185be4ee4b28c), ULL(0x550c7dc3d5ffb4e2),
ULL(0x72be5d74f27b896f), ULL(0x80deb1fe3b1696b1),
ULL(0x9bdc06a725c71235), ULL(0xc19bf174cf692694),
ULL(0xe49b69c19ef14ad2), ULL(0xefbe4786384f25e3),
ULL(0x0fc19dc68b8cd5b5), ULL(0x240ca1cc77ac9c65),
ULL(0x2de92c6f592b0275), ULL(0x4a7484aa6ea6e483),
ULL(0x5cb0a9dcbd41fbd4), ULL(0x76f988da831153b5),
ULL(0x983e5152ee66dfab), ULL(0xa831c66d2db43210),
ULL(0xb00327c898fb213f), ULL(0xbf597fc7beef0ee4),
ULL(0xc6e00bf33da88fc2), ULL(0xd5a79147930aa725),
ULL(0x06ca6351e003826f), ULL(0x142929670a0e6e70),
ULL(0x27b70a8546d22ffc), ULL(0x2e1b21385c26c926),
ULL(0x4d2c6dfc5ac42aed), ULL(0x53380d139d95b3df),
ULL(0x650a73548baf63de), ULL(0x766a0abb3c77b2a8),
ULL(0x81c2c92e47edaee6), ULL(0x92722c851482353b),
ULL(0xa2bfe8a14cf10364), ULL(0xa81a664bbc423001),
ULL(0xc24b8b70d0f89791), ULL(0xc76c51a30654be30),
ULL(0xd192e819d6ef5218), ULL(0xd69906245565a910),
ULL(0xf40e35855771202a), ULL(0x106aa07032bbd1b8),
ULL(0x19a4c116b8d2d0c8), ULL(0x1e376c085141ab53),
ULL(0x2748774cdf8eeb99), ULL(0x34b0bcb5e19b48a8),
ULL(0x391c0cb3c5c95a63), ULL(0x4ed8aa4ae3418acb),
ULL(0x5b9cca4f7763e373), ULL(0x682e6ff3d6b2b8a3),
ULL(0x748f82ee5defb2fc), ULL(0x78a5636f43172f60),
ULL(0x84c87814a1f0ab72), ULL(0x8cc702081a6439ec),
ULL(0x90befffa23631e28), ULL(0xa4506cebde82bde9),
ULL(0xbef9a3f7b2c67915), ULL(0xc67178f2e372532b),
ULL(0xca273eceea26619c), ULL(0xd186b8c721c0c207),
ULL(0xeada7dd6cde0eb1e), ULL(0xf57d4f7fee6ed178),
ULL(0x06f067aa72176fba), ULL(0x0a637dc5a2c898a6),
ULL(0x113f9804bef90dae), ULL(0x1b710b35131c471b),
ULL(0x28db77f523047d84), ULL(0x32caab7b40c72493),
ULL(0x3c9ebe0a15c9bebc), ULL(0x431d67c49c100d4c),
ULL(0x4cc5d4becb3e42b6), ULL(0x597f299cfc657e2a),
ULL(0x5fcb6fab3ad6faec), ULL(0x6c44198c4a475817)
};


static const sha2_word64 sha384_initial_hash_value[8] = {
ULL(0xcbbb9d5dc1059ed8),
ULL(0x629a292a367cd507),
ULL(0x9159015a3070dd17),
ULL(0x152fecd8f70e5939),
ULL(0x67332667ffc00b31),
ULL(0x8eb44a8768581511),
ULL(0xdb0c2e0d64f98fa7),
ULL(0x47b5481dbefa4fa4)
};


static const sha2_word64 sha512_initial_hash_value[8] = {
ULL(0x6a09e667f3bcc908),
ULL(0xbb67ae8584caa73b),
ULL(0x3c6ef372fe94f82b),
ULL(0xa54ff53a5f1d36f1),
ULL(0x510e527fade682d1),
ULL(0x9b05688c2b3e6c1f),
ULL(0x1f83d9abfb41bd6b),
ULL(0x5be0cd19137e2179)
};





static const char *sha2_hex_digits = "0123456789abcdef";



int SHA256_Init(SHA256_CTX* context) {
if (context == (SHA256_CTX*)0) {
return 0;
}
MEMCPY_BCOPY(context->state, sha256_initial_hash_value, SHA256_DIGEST_LENGTH);
MEMSET_BZERO(context->buffer, SHA256_BLOCK_LENGTH);
context->bitcount = 0;
return 1;
}

#if defined(SHA2_UNROLL_TRANSFORM)



#if BYTE_ORDER == LITTLE_ENDIAN

#define ROUND256_0_TO_15(a,b,c,d,e,f,g,h) REVERSE32(*data++, W256[j]); T1 = (h) + Sigma1_256(e) + Ch((e), (f), (g)) + K256[j] + W256[j]; (d) += T1; (h) = T1 + Sigma0_256(a) + Maj((a), (b), (c)); j++








#else 

#define ROUND256_0_TO_15(a,b,c,d,e,f,g,h) T1 = (h) + Sigma1_256(e) + Ch((e), (f), (g)) + K256[j] + (W256[j] = *data++); (d) += T1; (h) = T1 + Sigma0_256(a) + Maj((a), (b), (c)); j++






#endif 

#define ROUND256(a,b,c,d,e,f,g,h) s0 = W256[(j+1)&0x0f]; s0 = sigma0_256(s0); s1 = W256[(j+14)&0x0f]; s1 = sigma1_256(s1); T1 = (h) + Sigma1_256(e) + Ch((e), (f), (g)) + K256[j] + (W256[j&0x0f] += s1 + W256[(j+9)&0x0f] + s0); (d) += T1; (h) = T1 + Sigma0_256(a) + Maj((a), (b), (c)); j++










void SHA256_Transform(SHA256_CTX* context, const sha2_word32* data) {
sha2_word32 a, b, c, d, e, f, g, h, s0, s1;
sha2_word32 T1, *W256;
int j;

W256 = (sha2_word32*)context->buffer;


a = context->state[0];
b = context->state[1];
c = context->state[2];
d = context->state[3];
e = context->state[4];
f = context->state[5];
g = context->state[6];
h = context->state[7];

j = 0;
do {

ROUND256_0_TO_15(a,b,c,d,e,f,g,h);
ROUND256_0_TO_15(h,a,b,c,d,e,f,g);
ROUND256_0_TO_15(g,h,a,b,c,d,e,f);
ROUND256_0_TO_15(f,g,h,a,b,c,d,e);
ROUND256_0_TO_15(e,f,g,h,a,b,c,d);
ROUND256_0_TO_15(d,e,f,g,h,a,b,c);
ROUND256_0_TO_15(c,d,e,f,g,h,a,b);
ROUND256_0_TO_15(b,c,d,e,f,g,h,a);
} while (j < 16);


do {
ROUND256(a,b,c,d,e,f,g,h);
ROUND256(h,a,b,c,d,e,f,g);
ROUND256(g,h,a,b,c,d,e,f);
ROUND256(f,g,h,a,b,c,d,e);
ROUND256(e,f,g,h,a,b,c,d);
ROUND256(d,e,f,g,h,a,b,c);
ROUND256(c,d,e,f,g,h,a,b);
ROUND256(b,c,d,e,f,g,h,a);
} while (j < 64);


context->state[0] += a;
context->state[1] += b;
context->state[2] += c;
context->state[3] += d;
context->state[4] += e;
context->state[5] += f;
context->state[6] += g;
context->state[7] += h;


a = b = c = d = e = f = g = h = T1 = 0;
}

#else 

void SHA256_Transform(SHA256_CTX* context, const sha2_word32* data) {
sha2_word32 a, b, c, d, e, f, g, h, s0, s1;
sha2_word32 T1, T2, *W256;
int j;

W256 = (sha2_word32*)context->buffer;


a = context->state[0];
b = context->state[1];
c = context->state[2];
d = context->state[3];
e = context->state[4];
f = context->state[5];
g = context->state[6];
h = context->state[7];

j = 0;
do {
#if BYTE_ORDER == LITTLE_ENDIAN

REVERSE32(*data++,W256[j]);

T1 = h + Sigma1_256(e) + Ch(e, f, g) + K256[j] + W256[j];
#else 

T1 = h + Sigma1_256(e) + Ch(e, f, g) + K256[j] + (W256[j] = *data++);
#endif 
T2 = Sigma0_256(a) + Maj(a, b, c);
h = g;
g = f;
f = e;
e = d + T1;
d = c;
c = b;
b = a;
a = T1 + T2;

j++;
} while (j < 16);

do {

s0 = W256[(j+1)&0x0f];
s0 = sigma0_256(s0);
s1 = W256[(j+14)&0x0f];
s1 = sigma1_256(s1);


T1 = h + Sigma1_256(e) + Ch(e, f, g) + K256[j] +
(W256[j&0x0f] += s1 + W256[(j+9)&0x0f] + s0);
T2 = Sigma0_256(a) + Maj(a, b, c);
h = g;
g = f;
f = e;
e = d + T1;
d = c;
c = b;
b = a;
a = T1 + T2;

j++;
} while (j < 64);


context->state[0] += a;
context->state[1] += b;
context->state[2] += c;
context->state[3] += d;
context->state[4] += e;
context->state[5] += f;
context->state[6] += g;
context->state[7] += h;


a = b = c = d = e = f = g = h = T1 = T2 = 0;
}

#endif 

void SHA256_Update(SHA256_CTX* context, const sha2_byte *data, size_t len) {
unsigned int freespace, usedspace;

if (len == 0) {

return;
}


assert(context != (SHA256_CTX*)0 && data != (sha2_byte*)0);

usedspace = (unsigned int)((context->bitcount >> 3) % SHA256_BLOCK_LENGTH);
if (usedspace > 0) {

freespace = SHA256_BLOCK_LENGTH - usedspace;

if (len >= freespace) {

MEMCPY_BCOPY(&context->buffer[usedspace], data, freespace);
context->bitcount += freespace << 3;
len -= freespace;
data += freespace;
SHA256_Transform(context, (sha2_word32*)context->buffer);
} else {

MEMCPY_BCOPY(&context->buffer[usedspace], data, len);
context->bitcount += len << 3;

usedspace = freespace = 0;
return;
}
}
while (len >= SHA256_BLOCK_LENGTH) {

MEMCPY_BCOPY(context->buffer, data, SHA256_BLOCK_LENGTH);
SHA256_Transform(context, (sha2_word32*)context->buffer);
context->bitcount += SHA256_BLOCK_LENGTH << 3;
len -= SHA256_BLOCK_LENGTH;
data += SHA256_BLOCK_LENGTH;
}
if (len > 0) {

MEMCPY_BCOPY(context->buffer, data, len);
context->bitcount += len << 3;
}

usedspace = freespace = 0;
}

int SHA256_Final(sha2_byte digest[], SHA256_CTX* context) {
sha2_word32 *d = (sha2_word32*)digest;
unsigned int usedspace;


assert(context != (SHA256_CTX*)0);


if (digest != (sha2_byte*)0) {
usedspace = (unsigned int)((context->bitcount >> 3) % SHA256_BLOCK_LENGTH);
#if BYTE_ORDER == LITTLE_ENDIAN

REVERSE64(context->bitcount,context->bitcount);
#endif
if (usedspace > 0) {

context->buffer[usedspace++] = 0x80;

if (usedspace <= SHA256_SHORT_BLOCK_LENGTH) {

MEMSET_BZERO(&context->buffer[usedspace], SHA256_SHORT_BLOCK_LENGTH - usedspace);
} else {
if (usedspace < SHA256_BLOCK_LENGTH) {
MEMSET_BZERO(&context->buffer[usedspace], SHA256_BLOCK_LENGTH - usedspace);
}

SHA256_Transform(context, (sha2_word32*)context->buffer);


MEMSET_BZERO(context->buffer, SHA256_SHORT_BLOCK_LENGTH);
}
} else {

MEMSET_BZERO(context->buffer, SHA256_SHORT_BLOCK_LENGTH);


*context->buffer = 0x80;
}

MEMCPY_BCOPY(&context->buffer[SHA256_SHORT_BLOCK_LENGTH], &context->bitcount,
sizeof(sha2_word64));


SHA256_Transform(context, (sha2_word32*)context->buffer);

#if BYTE_ORDER == LITTLE_ENDIAN
{

int j;
for (j = 0; j < 8; j++) {
REVERSE32(context->state[j],context->state[j]);
*d++ = context->state[j];
}
}
#else
MEMCPY_BCOPY(d, context->state, SHA256_DIGEST_LENGTH);
#endif
}


MEMSET_BZERO(context, sizeof(*context));
usedspace = 0;
return 1;
}

char *SHA256_End(SHA256_CTX* context, char buffer[]) {
sha2_byte digest[SHA256_DIGEST_LENGTH], *d = digest;
int i;


assert(context != (SHA256_CTX*)0);

if (buffer != (char*)0) {
SHA256_Final(digest, context);
for (i = 0; i < SHA256_DIGEST_LENGTH; i++) {
*buffer++ = sha2_hex_digits[(*d & 0xf0) >> 4];
*buffer++ = sha2_hex_digits[*d & 0x0f];
d++;
}
*buffer = (char)0;
} else {
MEMSET_BZERO(context, sizeof(*context));
}
MEMSET_BZERO(digest, SHA256_DIGEST_LENGTH);
return buffer;
}

char* SHA256_Data(const sha2_byte* data, size_t len, char digest[SHA256_DIGEST_STRING_LENGTH]) {
SHA256_CTX context;

SHA256_Init(&context);
SHA256_Update(&context, data, len);
return SHA256_End(&context, digest);
}



int SHA512_Init(SHA512_CTX* context) {
if (context == (SHA512_CTX*)0) {
return 0;
}
MEMCPY_BCOPY(context->state, sha512_initial_hash_value, SHA512_DIGEST_LENGTH);
MEMSET_BZERO(context->buffer, SHA512_BLOCK_LENGTH);
context->bitcount[0] = context->bitcount[1] = 0;
return 1;
}

#if defined(SHA2_UNROLL_TRANSFORM)


#if BYTE_ORDER == LITTLE_ENDIAN

#define ROUND512_0_TO_15(a,b,c,d,e,f,g,h) REVERSE64(*data++, W512[j]); T1 = (h) + Sigma1_512(e) + Ch((e), (f), (g)) + K512[j] + W512[j]; (d) += T1, (h) = T1 + Sigma0_512(a) + Maj((a), (b), (c)), j++








#else 

#define ROUND512_0_TO_15(a,b,c,d,e,f,g,h) T1 = (h) + Sigma1_512(e) + Ch((e), (f), (g)) + K512[j] + (W512[j] = *data++); (d) += T1; (h) = T1 + Sigma0_512(a) + Maj((a), (b), (c)); j++






#endif 

#define ROUND512(a,b,c,d,e,f,g,h) s0 = W512[(j+1)&0x0f]; s0 = sigma0_512(s0); s1 = W512[(j+14)&0x0f]; s1 = sigma1_512(s1); T1 = (h) + Sigma1_512(e) + Ch((e), (f), (g)) + K512[j] + (W512[j&0x0f] += s1 + W512[(j+9)&0x0f] + s0); (d) += T1; (h) = T1 + Sigma0_512(a) + Maj((a), (b), (c)); j++










void SHA512_Transform(SHA512_CTX* context, const sha2_word64* data) {
sha2_word64 a, b, c, d, e, f, g, h, s0, s1;
sha2_word64 T1, *W512 = (sha2_word64*)context->buffer;
int j;


a = context->state[0];
b = context->state[1];
c = context->state[2];
d = context->state[3];
e = context->state[4];
f = context->state[5];
g = context->state[6];
h = context->state[7];

j = 0;
do {
ROUND512_0_TO_15(a,b,c,d,e,f,g,h);
ROUND512_0_TO_15(h,a,b,c,d,e,f,g);
ROUND512_0_TO_15(g,h,a,b,c,d,e,f);
ROUND512_0_TO_15(f,g,h,a,b,c,d,e);
ROUND512_0_TO_15(e,f,g,h,a,b,c,d);
ROUND512_0_TO_15(d,e,f,g,h,a,b,c);
ROUND512_0_TO_15(c,d,e,f,g,h,a,b);
ROUND512_0_TO_15(b,c,d,e,f,g,h,a);
} while (j < 16);


do {
ROUND512(a,b,c,d,e,f,g,h);
ROUND512(h,a,b,c,d,e,f,g);
ROUND512(g,h,a,b,c,d,e,f);
ROUND512(f,g,h,a,b,c,d,e);
ROUND512(e,f,g,h,a,b,c,d);
ROUND512(d,e,f,g,h,a,b,c);
ROUND512(c,d,e,f,g,h,a,b);
ROUND512(b,c,d,e,f,g,h,a);
} while (j < 80);


context->state[0] += a;
context->state[1] += b;
context->state[2] += c;
context->state[3] += d;
context->state[4] += e;
context->state[5] += f;
context->state[6] += g;
context->state[7] += h;


a = b = c = d = e = f = g = h = T1 = 0;
}

#else 

void SHA512_Transform(SHA512_CTX* context, const sha2_word64* data) {
sha2_word64 a, b, c, d, e, f, g, h, s0, s1;
sha2_word64 T1, T2, *W512 = (sha2_word64*)context->buffer;
int j;


a = context->state[0];
b = context->state[1];
c = context->state[2];
d = context->state[3];
e = context->state[4];
f = context->state[5];
g = context->state[6];
h = context->state[7];

j = 0;
do {
#if BYTE_ORDER == LITTLE_ENDIAN

REVERSE64(*data++, W512[j]);

T1 = h + Sigma1_512(e) + Ch(e, f, g) + K512[j] + W512[j];
#else 

T1 = h + Sigma1_512(e) + Ch(e, f, g) + K512[j] + (W512[j] = *data++);
#endif 
T2 = Sigma0_512(a) + Maj(a, b, c);
h = g;
g = f;
f = e;
e = d + T1;
d = c;
c = b;
b = a;
a = T1 + T2;

j++;
} while (j < 16);

do {

s0 = W512[(j+1)&0x0f];
s0 = sigma0_512(s0);
s1 = W512[(j+14)&0x0f];
s1 = sigma1_512(s1);


T1 = h + Sigma1_512(e) + Ch(e, f, g) + K512[j] +
(W512[j&0x0f] += s1 + W512[(j+9)&0x0f] + s0);
T2 = Sigma0_512(a) + Maj(a, b, c);
h = g;
g = f;
f = e;
e = d + T1;
d = c;
c = b;
b = a;
a = T1 + T2;

j++;
} while (j < 80);


context->state[0] += a;
context->state[1] += b;
context->state[2] += c;
context->state[3] += d;
context->state[4] += e;
context->state[5] += f;
context->state[6] += g;
context->state[7] += h;


a = b = c = d = e = f = g = h = T1 = T2 = 0;
}

#endif 

void SHA512_Update(SHA512_CTX* context, const sha2_byte *data, size_t len) {
unsigned int freespace, usedspace;

if (len == 0) {

return;
}


assert(context != (SHA512_CTX*)0 && data != (sha2_byte*)0);

usedspace = (unsigned int)((context->bitcount[0] >> 3) % SHA512_BLOCK_LENGTH);
if (usedspace > 0) {

freespace = SHA512_BLOCK_LENGTH - usedspace;

if (len >= freespace) {

MEMCPY_BCOPY(&context->buffer[usedspace], data, freespace);
ADDINC128(context->bitcount, freespace << 3);
len -= freespace;
data += freespace;
SHA512_Transform(context, (sha2_word64*)context->buffer);
} else {

MEMCPY_BCOPY(&context->buffer[usedspace], data, len);
ADDINC128(context->bitcount, len << 3);

usedspace = freespace = 0;
return;
}
}
while (len >= SHA512_BLOCK_LENGTH) {

MEMCPY_BCOPY(context->buffer, data, SHA512_BLOCK_LENGTH);
SHA512_Transform(context, (sha2_word64*)context->buffer);
ADDINC128(context->bitcount, SHA512_BLOCK_LENGTH << 3);
len -= SHA512_BLOCK_LENGTH;
data += SHA512_BLOCK_LENGTH;
}
if (len > 0) {

MEMCPY_BCOPY(context->buffer, data, len);
ADDINC128(context->bitcount, len << 3);
}

usedspace = freespace = 0;
}

void SHA512_Last(SHA512_CTX* context) {
unsigned int usedspace;

usedspace = (unsigned int)((context->bitcount[0] >> 3) % SHA512_BLOCK_LENGTH);
#if BYTE_ORDER == LITTLE_ENDIAN

REVERSE64(context->bitcount[0],context->bitcount[0]);
REVERSE64(context->bitcount[1],context->bitcount[1]);
#endif
if (usedspace > 0) {

context->buffer[usedspace++] = 0x80;

if (usedspace <= SHA512_SHORT_BLOCK_LENGTH) {

MEMSET_BZERO(&context->buffer[usedspace], SHA512_SHORT_BLOCK_LENGTH - usedspace);
} else {
if (usedspace < SHA512_BLOCK_LENGTH) {
MEMSET_BZERO(&context->buffer[usedspace], SHA512_BLOCK_LENGTH - usedspace);
}

SHA512_Transform(context, (sha2_word64*)context->buffer);


MEMSET_BZERO(context->buffer, SHA512_BLOCK_LENGTH - 2);
}
} else {

MEMSET_BZERO(context->buffer, SHA512_SHORT_BLOCK_LENGTH);


*context->buffer = 0x80;
}

MEMCPY_BCOPY(&context->buffer[SHA512_SHORT_BLOCK_LENGTH], &context->bitcount[1],
sizeof(sha2_word64));
MEMCPY_BCOPY(&context->buffer[SHA512_SHORT_BLOCK_LENGTH+8], &context->bitcount[0],
sizeof(sha2_word64));


SHA512_Transform(context, (sha2_word64*)context->buffer);
}

int SHA512_Final(sha2_byte digest[], SHA512_CTX* context) {
sha2_word64 *d = (sha2_word64*)digest;


assert(context != (SHA512_CTX*)0);


if (digest != (sha2_byte*)0) {
SHA512_Last(context);


#if BYTE_ORDER == LITTLE_ENDIAN
{

int j;
for (j = 0; j < 8; j++) {
REVERSE64(context->state[j],context->state[j]);
*d++ = context->state[j];
}
}
#else
MEMCPY_BCOPY(d, context->state, SHA512_DIGEST_LENGTH);
#endif
}


MEMSET_BZERO(context, sizeof(*context));
return 1;
}

char *SHA512_End(SHA512_CTX* context, char buffer[]) {
sha2_byte digest[SHA512_DIGEST_LENGTH], *d = digest;
int i;


assert(context != (SHA512_CTX*)0);

if (buffer != (char*)0) {
SHA512_Final(digest, context);
for (i = 0; i < SHA512_DIGEST_LENGTH; i++) {
*buffer++ = sha2_hex_digits[(*d & 0xf0) >> 4];
*buffer++ = sha2_hex_digits[*d & 0x0f];
d++;
}
*buffer = (char)0;
} else {
MEMSET_BZERO(context, sizeof(*context));
}
MEMSET_BZERO(digest, SHA512_DIGEST_LENGTH);
return buffer;
}

char* SHA512_Data(const sha2_byte* data, size_t len, char digest[SHA512_DIGEST_STRING_LENGTH]) {
SHA512_CTX context;

SHA512_Init(&context);
SHA512_Update(&context, data, len);
return SHA512_End(&context, digest);
}



int SHA384_Init(SHA384_CTX* context) {
if (context == (SHA384_CTX*)0) {
return 0;
}
MEMCPY_BCOPY(context->state, sha384_initial_hash_value, SHA512_DIGEST_LENGTH);
MEMSET_BZERO(context->buffer, SHA384_BLOCK_LENGTH);
context->bitcount[0] = context->bitcount[1] = 0;
return 1;
}

void SHA384_Update(SHA384_CTX* context, const sha2_byte* data, size_t len) {
SHA512_Update((SHA512_CTX*)context, data, len);
}

int SHA384_Final(sha2_byte digest[], SHA384_CTX* context) {
sha2_word64 *d = (sha2_word64*)digest;


assert(context != (SHA384_CTX*)0);


if (digest != (sha2_byte*)0) {
SHA512_Last((SHA512_CTX*)context);


#if BYTE_ORDER == LITTLE_ENDIAN
{

int j;
for (j = 0; j < 6; j++) {
REVERSE64(context->state[j],context->state[j]);
*d++ = context->state[j];
}
}
#else
MEMCPY_BCOPY(d, context->state, SHA384_DIGEST_LENGTH);
#endif
}


MEMSET_BZERO(context, sizeof(*context));
return 1;
}

char *SHA384_End(SHA384_CTX* context, char buffer[]) {
sha2_byte digest[SHA384_DIGEST_LENGTH], *d = digest;
int i;


assert(context != (SHA384_CTX*)0);

if (buffer != (char*)0) {
SHA384_Final(digest, context);
for (i = 0; i < SHA384_DIGEST_LENGTH; i++) {
*buffer++ = sha2_hex_digits[(*d & 0xf0) >> 4];
*buffer++ = sha2_hex_digits[*d & 0x0f];
d++;
}
*buffer = (char)0;
} else {
MEMSET_BZERO(context, sizeof(*context));
}
MEMSET_BZERO(digest, SHA384_DIGEST_LENGTH);
return buffer;
}

char* SHA384_Data(const sha2_byte* data, size_t len, char digest[SHA384_DIGEST_STRING_LENGTH]) {
SHA384_CTX context;

SHA384_Init(&context);
SHA384_Update(&context, data, len);
return SHA384_End(&context, digest);
}

