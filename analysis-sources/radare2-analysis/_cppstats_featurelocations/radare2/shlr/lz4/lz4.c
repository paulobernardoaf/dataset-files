










































#define ACCELERATION_DEFAULT 1


















#if !defined(LZ4_FORCE_MEMORY_ACCESS)
#if defined(__GNUC__) && ( defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6ZK__) || defined(__ARM_ARCH_6T2__) )
#define LZ4_FORCE_MEMORY_ACCESS 2
#elif defined(__INTEL_COMPILER) || defined(__GNUC__)
#define LZ4_FORCE_MEMORY_ACCESS 1
#endif
#endif





#if defined(_MSC_VER) && defined(_WIN32_WCE) 
#define LZ4_FORCE_SW_BITCOUNT
#endif

#include "lz4.h"




#if defined(_MSC_VER)
#include <intrin.h>
#pragma warning(disable : 4127) 
#pragma warning(disable : 4293) 
#endif 

#if !defined(LZ4_FORCE_INLINE)
#if defined(_MSC_VER)
#define LZ4_FORCE_INLINE static __forceinline
#else
#if defined (__cplusplus) || defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L 
#if defined(__GNUC__)
#define LZ4_FORCE_INLINE static inline __attribute__((always_inline))
#else
#define LZ4_FORCE_INLINE static inline
#endif
#else
#define LZ4_FORCE_INLINE static
#endif 
#endif 
#endif 















#if defined(__PPC64__) && defined(__LITTLE_ENDIAN__) && defined(__GNUC__)
#define LZ4_FORCE_O2_GCC_PPC64LE __attribute__((optimize("O2")))
#define LZ4_FORCE_O2_INLINE_GCC_PPC64LE __attribute__((optimize("O2"))) LZ4_FORCE_INLINE
#else
#define LZ4_FORCE_O2_GCC_PPC64LE
#define LZ4_FORCE_O2_INLINE_GCC_PPC64LE static
#endif

#if (defined(__GNUC__) && (__GNUC__ >= 3)) || (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 800)) || defined(__clang__)
#define expect(expr,value) (__builtin_expect ((expr),(value)) )
#else
#define expect(expr,value) (expr)
#endif

#define likely(expr) expect((expr) != 0, 1)
#define unlikely(expr) expect((expr) != 0, 0)


#include <stdlib.h>
#include <string.h>

typedef size_t reg_t;




static unsigned LZ4_isLittleEndian(void)
{
const union { ut32 u; ut8 c[4]; } one = { 1 }; 
return one.c[0];
}


#if defined(LZ4_FORCE_MEMORY_ACCESS) && (LZ4_FORCE_MEMORY_ACCESS==2)


static ut16 LZ4_read16(const void* memPtr) { return *(const ut16*) memPtr; }
static ut32 LZ4_read32(const void* memPtr) { return *(const ut32*) memPtr; }
static reg_t LZ4_read_ARCH(const void* memPtr) { return *(const reg_t*) memPtr; }

static void LZ4_write16(void* memPtr, ut16 value) { *(ut16*)memPtr = value; }
static void LZ4_write32(void* memPtr, ut32 value) { *(ut32*)memPtr = value; }

#elif defined(LZ4_FORCE_MEMORY_ACCESS) && (LZ4_FORCE_MEMORY_ACCESS==1)



typedef union { ut16 u16; ut32 u32; reg_t uArch; } __attribute__((packed)) unalign;

static ut16 LZ4_read16(const void* ptr) { return ((const unalign*)ptr)->u16; }
static ut32 LZ4_read32(const void* ptr) { return ((const unalign*)ptr)->u32; }
static reg_t LZ4_read_ARCH(const void* ptr) { return ((const unalign*)ptr)->uArch; }

static void LZ4_write16(void* memPtr, ut16 value) { ((unalign*)memPtr)->u16 = value; }
static void LZ4_write32(void* memPtr, ut32 value) { ((unalign*)memPtr)->u32 = value; }

#else 

static ut16 LZ4_read16(const void* memPtr) {
ut16 val;
memcpy (&val, memPtr, sizeof(val));
return val;
}

static ut32 LZ4_read32(const void* memPtr) {
ut32 val;
memcpy(&val, memPtr, sizeof(val));
return val;
}

static reg_t LZ4_read_ARCH(const void* memPtr) {
reg_t val; memcpy(&val, memPtr, sizeof(val)); return val;
}

static void LZ4_write16(void* memPtr, ut16 value) {
memcpy(memPtr, &value, sizeof(value));
}

static void LZ4_write32(void* memPtr, ut32 value) {
memcpy(memPtr, &value, sizeof(value));
}

#endif 

static ut16 LZ4_readLE16(const void* memPtr) {
if (LZ4_isLittleEndian()) {
return LZ4_read16(memPtr);
}
const ut8* p = (const ut8*)memPtr;
return (ut16)((ut16)p[0] + (p[1]<<8));
}

static void LZ4_writeLE16(void* memPtr, ut16 value) {
if (LZ4_isLittleEndian ()) {
LZ4_write16 (memPtr, value);
} else {
ut8* p = (ut8*)memPtr;
p[0] = (ut8) value;
p[1] = (ut8)(value>>8);
}
}

static void LZ4_copy8(void* dst, const void* src) {
memcpy (dst,src,8);
}


LZ4_FORCE_O2_INLINE_GCC_PPC64LE
void LZ4_wildCopy(void* dstPtr, const void* srcPtr, void* dstEnd) {
ut8* d = (ut8*)dstPtr;
const ut8* s = (const ut8*)srcPtr;
ut8* const e = (ut8*)dstEnd;

do {
LZ4_copy8(d,s); d+=8; s+=8;
} while (d<e);
}




#define MINMATCH 4

#define WILDCOPYLENGTH 8
#define LASTLITERALS 5
#define MFLIMIT (WILDCOPYLENGTH+MINMATCH)
static const int LZ4_minLength = (MFLIMIT+1);

#define KB *(1 <<10)
#define MB *(1 <<20)
#define GB *(1U<<30)

#define MAXD_LOG 16
#define MAX_DISTANCE ((1 << MAXD_LOG) - 1)

#define ML_BITS 4
#define ML_MASK ((1U<<ML_BITS)-1)
#define RUN_BITS (8-ML_BITS)
#define RUN_MASK ((1U<<RUN_BITS)-1)




static unsigned LZ4_NbCommonBytes (register reg_t val) {
if (LZ4_isLittleEndian()) {
if (sizeof(val)==8) {
#if defined(_MSC_VER) && defined(_WIN64) && !defined(LZ4_FORCE_SW_BITCOUNT)
unsigned long r = 0;
_BitScanForward64( &r, (ut64)val );
return (int)(r>>3);
#elif (defined(__clang__) || (defined(__GNUC__) && (__GNUC__>=3))) && !defined(LZ4_FORCE_SW_BITCOUNT)
return (__builtin_ctzll((ut64)val) >> 3);
#else
static const int DeBruijnBytePos[64] = { 0, 0, 0, 0, 0, 1, 1, 2, 0, 3, 1, 3, 1, 4, 2, 7, 0, 2, 3, 6, 1, 5, 3, 5, 1, 3, 4, 4, 2, 5, 6, 7, 7, 0, 1, 2, 3, 3, 4, 6, 2, 6, 5, 5, 3, 4, 5, 6, 7, 1, 2, 4, 6, 4, 4, 5, 7, 2, 6, 5, 7, 6, 7, 7 };
return DeBruijnBytePos[((ut64)((val & -(long long)val) * 0x0218A392CDABBD3FULL)) >> 58];
#endif
} else {
#if defined(_MSC_VER) && !defined(LZ4_FORCE_SW_BITCOUNT)
unsigned long r;
_BitScanForward( &r, (ut32)val );
return (int)(r>>3);
#elif (defined(__clang__) || (defined(__GNUC__) && (__GNUC__>=3))) && !defined(LZ4_FORCE_SW_BITCOUNT)
return (__builtin_ctz((ut32)val) >> 3);
#else
static const int DeBruijnBytePos[32] = { 0, 0, 3, 0, 3, 1, 3, 0, 3, 2, 2, 1, 3, 2, 0, 1, 3, 3, 1, 2, 2, 2, 2, 0, 3, 1, 2, 0, 1, 0, 1, 1 };
return DeBruijnBytePos[((ut32)((val & -(st32)val) * 0x077CB531U)) >> 27];
#endif
}
} else {
if (sizeof(val)==8) { 
#if defined(_MSC_VER) && defined(_WIN64) && !defined(LZ4_FORCE_SW_BITCOUNT)
unsigned long r = 0;
_BitScanReverse64( &r, val );
return (unsigned)(r>>3);
#elif (defined(__clang__) || (defined(__GNUC__) && (__GNUC__>=3))) && !defined(LZ4_FORCE_SW_BITCOUNT)
return (__builtin_clzll((ut64)val) >> 3);
#else
static const ut32 by32 = sizeof(val)*4; 


unsigned r;
if (!(val>>by32)) { r=4; } else { r=0; val>>=by32; }
if (!(val>>16)) { r+=2; val>>=8; } else { val>>=24; }
r += (!val);
return r;
#endif
} else {
#if defined(_MSC_VER) && !defined(LZ4_FORCE_SW_BITCOUNT)
unsigned long r = 0;
_BitScanReverse( &r, (unsigned long)val );
return (unsigned)(r>>3);
#elif (defined(__clang__) || (defined(__GNUC__) && (__GNUC__>=3))) && !defined(LZ4_FORCE_SW_BITCOUNT)
return (__builtin_clz((ut32)val) >> 3);
#else
unsigned r;
if (!(val>>16)) { r=2; val>>=8; } else { r=0; val>>=24; }
r += (!val);
return r;
#endif
}
}
}

#define STEPSIZE sizeof(reg_t)
static unsigned LZ4_count(const ut8* pIn, const ut8* pMatch, const ut8* pInLimit) {
const ut8* const pStart = pIn;

while (likely(pIn<pInLimit-(STEPSIZE-1))) {
reg_t const diff = LZ4_read_ARCH(pMatch) ^ LZ4_read_ARCH(pIn);
if (!diff) { pIn+=STEPSIZE; pMatch+=STEPSIZE; continue; }
pIn += LZ4_NbCommonBytes(diff);
return (unsigned)(pIn - pStart);
}

if ((STEPSIZE==8) && (pIn<(pInLimit-3)) && (LZ4_read32(pMatch) == LZ4_read32(pIn))) { pIn+=4; pMatch+=4; }
if ((pIn<(pInLimit-1)) && (LZ4_read16(pMatch) == LZ4_read16(pIn))) { pIn+=2; pMatch+=2; }
if ((pIn<pInLimit) && (*pMatch == *pIn)) pIn++;
return (unsigned)(pIn - pStart);
}


#if !defined(LZ4_COMMONDEFS_ONLY)



static const int LZ4_64Klimit = ((64 KB) + (MFLIMIT-1));
static const ut32 LZ4_skipTrigger = 6; 





typedef enum { notLimited = 0, limitedOutput = 1 } limitedOutput_directive;
typedef enum { byPtr, byut32, byut16 } tableType_t;

typedef enum { noDict = 0, withPrefix64k, usingExtDict } dict_directive;
typedef enum { noDictIssue = 0, dictSmall } dictIssue_directive;

typedef enum { endOnOutputSize = 0, endOnInputSize = 1 } endCondition_directive;
typedef enum { full = 0, partial = 1 } earlyEnd_directive;





int LZ4_versionNumber (void) { return LZ4_VERSION_NUMBER; }
const char* LZ4_versionString(void) { return LZ4_VERSION_STRING; }
int LZ4_compressBound(int isize) { return LZ4_COMPRESSBOUND(isize); }
int LZ4_sizeofState() { return LZ4_STREAMSIZE; }




static ut32 LZ4_hash4(ut32 sequence, tableType_t const tableType) {
if (tableType == byut16) {
return ((sequence * 2654435761U) >> ((MINMATCH*8)-(LZ4_HASHLOG+1)));
}
return ((sequence * 2654435761U) >> ((MINMATCH*8)-LZ4_HASHLOG));
}

static ut32 LZ4_hash5(ut64 sequence, tableType_t const tableType) {
static const ut64 prime5bytes = 889523592379ULL;
static const ut64 prime8bytes = 11400714785074694791ULL;
const ut32 hashLog = (tableType == byut16) ? LZ4_HASHLOG+1 : LZ4_HASHLOG;
if (LZ4_isLittleEndian()) {
return (ut32)(((sequence << 24) * prime5bytes) >> (64 - hashLog));
}
return (ut32)(((sequence >> 24) * prime8bytes) >> (64 - hashLog));
}

LZ4_FORCE_INLINE ut32 LZ4_hashPosition(const void* const p, tableType_t const tableType) {
if ((sizeof(reg_t)==8) && (tableType != byut16)) return LZ4_hash5(LZ4_read_ARCH(p), tableType);
return LZ4_hash4(LZ4_read32(p), tableType);
}

static void LZ4_putPositionOnHash(const ut8* p, ut32 h, void* tableBase, tableType_t const tableType, const ut8* srcBase) {
switch (tableType) {
case byPtr: { const ut8** hashTable = (const ut8**)tableBase; hashTable[h] = p; return; }
case byut32: { ut32* hashTable = (ut32*) tableBase; hashTable[h] = (ut32)(p-srcBase); return; }
case byut16: { ut16* hashTable = (ut16*) tableBase; hashTable[h] = (ut16)(p-srcBase); return; }
}
}

LZ4_FORCE_INLINE void LZ4_putPosition(const ut8* p, void* tableBase, tableType_t tableType, const ut8* srcBase) {
ut32 const h = LZ4_hashPosition(p, tableType);
LZ4_putPositionOnHash(p, h, tableBase, tableType, srcBase);
}

static const ut8* LZ4_getPositionOnHash(ut32 h, void* tableBase, tableType_t tableType, const ut8* srcBase) {
if (tableType == byPtr) { const ut8** hashTable = (const ut8**) tableBase; return hashTable[h]; }
if (tableType == byut32) { const ut32* const hashTable = (ut32*) tableBase; return hashTable[h] + srcBase; }
{ const ut16* const hashTable = (ut16*) tableBase; return hashTable[h] + srcBase; } 
}

LZ4_FORCE_INLINE const ut8* LZ4_getPosition(const ut8* p, void* tableBase, tableType_t tableType, const ut8* srcBase) {
ut32 const h = LZ4_hashPosition(p, tableType);
return LZ4_getPositionOnHash(h, tableBase, tableType, srcBase);
}



LZ4_FORCE_INLINE int LZ4_compress_generic(
LZ4_stream_t_internal* const cctx,
const char* const source,
char* const dest,
const int inputSize,
const int maxOutputSize,
const limitedOutput_directive outputLimited,
const tableType_t tableType,
const dict_directive dict,
const dictIssue_directive dictIssue,
const ut32 acceleration)
{
const ut8* ip = (const ut8*) source;
const ut8* base;
const ut8* lowLimit;
const ut8* const lowRefLimit = ip - cctx->dictSize;
const ut8* const dictionary = cctx->dictionary;
const ut8* const dictEnd = dictionary + cctx->dictSize;
const ptrdiff_t dictDelta = dictEnd - (const ut8*)source;
const ut8* anchor = (const ut8*) source;
const ut8* const iend = ip + inputSize;
const ut8* const mflimit = iend - MFLIMIT;
const ut8* const matchlimit = iend - LASTLITERALS;

ut8* op = (ut8*) dest;
ut8* const olimit = op + maxOutputSize;

ut32 forwardH;


if ((ut32)inputSize > (ut32)LZ4_MAX_INPUT_SIZE) return 0; 
switch(dict) {
case withPrefix64k:
base = (const ut8*)source - cctx->currentOffset;
lowLimit = (const ut8*)source - cctx->dictSize;
break;
case usingExtDict:
base = (const ut8*)source - cctx->currentOffset;
lowLimit = (const ut8*)source;
break;
case noDict:
default:
base = (const ut8*)source;
lowLimit = (const ut8*)source;
break;
}
if ((tableType == byut16) && (inputSize>=LZ4_64Klimit)) return 0; 
if (inputSize<LZ4_minLength) goto _last_literals; 


LZ4_putPosition(ip, cctx->hashTable, tableType, base);
ip++; forwardH = LZ4_hashPosition(ip, tableType);


for ( ; ; ) {
ptrdiff_t refDelta = 0;
const ut8* match;
ut8* token;


{ const ut8* forwardIp = ip;
unsigned step = 1;
unsigned searchMatchNb = acceleration << LZ4_skipTrigger;
do {
ut32 const h = forwardH;
ip = forwardIp;
forwardIp += step;
step = (searchMatchNb++ >> LZ4_skipTrigger);

if (unlikely(forwardIp > mflimit)) goto _last_literals;

match = LZ4_getPositionOnHash(h, cctx->hashTable, tableType, base);
if (dict==usingExtDict) {
if (match < (const ut8*)source) {
refDelta = dictDelta;
lowLimit = dictionary;
} else {
refDelta = 0;
lowLimit = (const ut8*)source;
}
}
forwardH = LZ4_hashPosition(forwardIp, tableType);
LZ4_putPositionOnHash(ip, h, cctx->hashTable, tableType, base);

} while ( ((dictIssue==dictSmall) ? (match < lowRefLimit) : 0)
|| ((tableType==byut16) ? 0 : (match + MAX_DISTANCE < ip))
|| (LZ4_read32(match+refDelta) != LZ4_read32(ip)) );
}


while (((ip>anchor) & (match+refDelta > lowLimit)) && (unlikely(ip[-1]==match[refDelta-1]))) { ip--; match--; }


{ unsigned const litLength = (unsigned)(ip - anchor);
token = op++;
if ((outputLimited) && 
(unlikely(op + litLength + (2 + 1 + LASTLITERALS) + (litLength/255) > olimit)))
return 0;
if (litLength >= RUN_MASK) {
int len = (int)litLength-RUN_MASK;
*token = (RUN_MASK<<ML_BITS);
for(; len >= 255 ; len-=255) *op++ = 255;
*op++ = (ut8)len;
}
else *token = (ut8)(litLength<<ML_BITS);


LZ4_wildCopy(op, anchor, op+litLength);
op+=litLength;
}

_next_match:

LZ4_writeLE16 (op, (ut16)(ip-match));
op += 2;


unsigned matchCode;

if ((dict==usingExtDict) && (lowLimit==dictionary)) {
const ut8* limit;
match += refDelta;
limit = ip + (dictEnd-match);
if (limit > matchlimit) limit = matchlimit;
matchCode = LZ4_count(ip+MINMATCH, match+MINMATCH, limit);
ip += MINMATCH + matchCode;
if (ip==limit) {
unsigned const more = LZ4_count(ip, (const ut8*)source, matchlimit);
matchCode += more;
ip += more;
}
} else {
matchCode = LZ4_count(ip+MINMATCH, match+MINMATCH, matchlimit);
ip += MINMATCH + matchCode;
}


if (outputLimited && (unlikely(op + (1 + LASTLITERALS) + (matchCode>>8) > olimit)) ) {
return 0;
}
if (matchCode >= ML_MASK) {
*token += ML_MASK;
matchCode -= ML_MASK;
LZ4_write32(op, 0xFFFFFFFF);
while (matchCode >= 4*255) {
op+=4;
LZ4_write32(op, 0xFFFFFFFF);
matchCode -= 4*255;
}
op += matchCode / 255;
*op++ = (ut8)(matchCode % 255);
} else {
*token += (ut8)(matchCode);
}

anchor = ip;


if (ip > mflimit) {
break;
}


LZ4_putPosition(ip-2, cctx->hashTable, tableType, base);


match = LZ4_getPosition(ip, cctx->hashTable, tableType, base);
if (dict == usingExtDict) {
if (match < (const ut8*)source) {
refDelta = dictDelta;
lowLimit = dictionary;
} else {
refDelta = 0;
lowLimit = (const ut8*)source;
}
}
LZ4_putPosition(ip, cctx->hashTable, tableType, base);
if ( ((dictIssue==dictSmall) ? (match>=lowRefLimit) : 1)
&& (match+MAX_DISTANCE>=ip)
&& (LZ4_read32(match+refDelta)==LZ4_read32(ip)) )
{ token=op++; *token=0; goto _next_match; }


forwardH = LZ4_hashPosition(++ip, tableType);
}

_last_literals:

{ size_t const lastRun = (size_t)(iend - anchor);
if ( (outputLimited) && 
((op - (ut8*)dest) + lastRun + 1 + ((lastRun+255-RUN_MASK)/255) > (ut32)maxOutputSize) )
return 0;
if (lastRun >= RUN_MASK) {
size_t accumulator = lastRun - RUN_MASK;
*op++ = RUN_MASK << ML_BITS;
for(; accumulator >= 255 ; accumulator-=255) *op++ = 255;
*op++ = (ut8) accumulator;
} else {
*op++ = (ut8)(lastRun<<ML_BITS);
}
memcpy(op, anchor, lastRun);
op += lastRun;
}


return (int) (((char*)op)-dest);
}

int LZ4_compress_fast_extState(void* state, const char* source, char* dest, int inputSize, int maxOutputSize, int acceleration) {
LZ4_stream_t_internal* ctx = &((LZ4_stream_t*)state)->internal_donotuse;
LZ4_resetStream ((LZ4_stream_t*)state);
if (acceleration < 1) {
acceleration = ACCELERATION_DEFAULT;
}
if (maxOutputSize >= LZ4_compressBound(inputSize)) {
if (inputSize < LZ4_64Klimit) {
return LZ4_compress_generic(ctx, source, dest, inputSize, 0, notLimited, byut16, noDict, noDictIssue, acceleration);
}
return LZ4_compress_generic(ctx, source, dest, inputSize, 0, notLimited, (sizeof(void*)==8) ? byut32 : byPtr, noDict, noDictIssue, acceleration);
}
if (inputSize < LZ4_64Klimit) {
return LZ4_compress_generic(ctx, source, dest, inputSize, maxOutputSize, limitedOutput, byut16, noDict, noDictIssue, acceleration);
}
return LZ4_compress_generic(ctx, source, dest, inputSize, maxOutputSize, limitedOutput, (sizeof(void*)==8) ? byut32 : byPtr, noDict, noDictIssue, acceleration);
}

int LZ4_compress_fast(const char* source, char* dest, int inputSize, int maxOutputSize, int acceleration) {
LZ4_stream_t ctx;
return LZ4_compress_fast_extState (&ctx, source, dest, inputSize, maxOutputSize, acceleration);
}

int LZ4_compress_default(const char* source, char* dest, int inputSize, int maxOutputSize) {
return LZ4_compress_fast(source, dest, inputSize, maxOutputSize, 1);
}



int LZ4_compress_fast_force(const char* source, char* dest, int inputSize, int maxOutputSize, int acceleration) {
LZ4_stream_t ctx;
LZ4_resetStream(&ctx);

if (inputSize < LZ4_64Klimit) {
return LZ4_compress_generic(&ctx.internal_donotuse, source, dest, inputSize, maxOutputSize, limitedOutput, byut16, noDict, noDictIssue, acceleration);
}
return LZ4_compress_generic(&ctx.internal_donotuse, source, dest, inputSize, maxOutputSize, limitedOutput, sizeof(void*)==8 ? byut32 : byPtr, noDict, noDictIssue, acceleration);
}





static int LZ4_compress_destSize_generic(
LZ4_stream_t_internal* const ctx,
const char* const src,
char* const dst,
int* const srcSizePtr,
const int targetDstSize,
const tableType_t tableType)
{
const ut8* ip = (const ut8*) src;
const ut8* base = (const ut8*) src;
const ut8* lowLimit = (const ut8*) src;
const ut8* anchor = ip;
const ut8* const iend = ip + *srcSizePtr;
const ut8* const mflimit = iend - MFLIMIT;
const ut8* const matchlimit = iend - LASTLITERALS;

ut8* op = (ut8*) dst;
ut8* const oend = op + targetDstSize;
ut8* const oMaxLit = op + targetDstSize - 2 - 8 - 1 ;
ut8* const oMaxMatch = op + targetDstSize - (LASTLITERALS + 1 );
ut8* const oMaxSeq = oMaxLit - 1 ;

ut32 forwardH;



if (targetDstSize < 1) return 0; 
if ((ut32)*srcSizePtr > (ut32)LZ4_MAX_INPUT_SIZE) return 0; 
if ((tableType == byut16) && (*srcSizePtr>=LZ4_64Klimit)) return 0; 
if (*srcSizePtr<LZ4_minLength) goto _last_literals; 


*srcSizePtr = 0;
LZ4_putPosition(ip, ctx->hashTable, tableType, base);
ip++; forwardH = LZ4_hashPosition(ip, tableType);


for ( ; ; ) {
const ut8* match;
ut8* token;


{ const ut8* forwardIp = ip;
unsigned step = 1;
unsigned searchMatchNb = 1 << LZ4_skipTrigger;

do {
ut32 h = forwardH;
ip = forwardIp;
forwardIp += step;
step = (searchMatchNb++ >> LZ4_skipTrigger);

if (unlikely(forwardIp > mflimit)) goto _last_literals;

match = LZ4_getPositionOnHash(h, ctx->hashTable, tableType, base);
forwardH = LZ4_hashPosition(forwardIp, tableType);
LZ4_putPositionOnHash(ip, h, ctx->hashTable, tableType, base);

} while ( ((tableType==byut16) ? 0 : (match + MAX_DISTANCE < ip))
|| (LZ4_read32(match) != LZ4_read32(ip)) );
}


while ((ip>anchor) && (match > lowLimit) && (unlikely(ip[-1]==match[-1]))) { ip--; match--; }


{ unsigned litLength = (unsigned)(ip - anchor);
token = op++;
if (op + ((litLength+240)/255) + litLength > oMaxLit) {

op--;
goto _last_literals;
}
if (litLength>=RUN_MASK) {
unsigned len = litLength - RUN_MASK;
*token=(RUN_MASK<<ML_BITS);
for(; len >= 255 ; len-=255) *op++ = 255;
*op++ = (ut8)len;
}
else *token = (ut8)(litLength<<ML_BITS);


LZ4_wildCopy(op, anchor, op+litLength);
op += litLength;
}

_next_match:

LZ4_writeLE16(op, (ut16)(ip-match)); op+=2;


{ size_t matchLength = LZ4_count(ip+MINMATCH, match+MINMATCH, matchlimit);

if (op + ((matchLength+240)/255) > oMaxMatch) {

matchLength = (15-1) + (oMaxMatch-op) * 255;
}
ip += MINMATCH + matchLength;

if (matchLength>=ML_MASK) {
*token += ML_MASK;
matchLength -= ML_MASK;
while (matchLength >= 255) { matchLength-=255; *op++ = 255; }
*op++ = (ut8)matchLength;
}
else *token += (ut8)(matchLength);
}

anchor = ip;


if (ip > mflimit) break;
if (op > oMaxSeq) break;


LZ4_putPosition(ip-2, ctx->hashTable, tableType, base);


match = LZ4_getPosition(ip, ctx->hashTable, tableType, base);
LZ4_putPosition(ip, ctx->hashTable, tableType, base);
if ( (match+MAX_DISTANCE>=ip)
&& (LZ4_read32(match)==LZ4_read32(ip)) )
{ token=op++; *token=0; goto _next_match; }


forwardH = LZ4_hashPosition(++ip, tableType);
}

_last_literals:

{ size_t lastRunSize = (size_t)(iend - anchor);
if (op + 1 + ((lastRunSize+240)/255) + lastRunSize > oend) {

lastRunSize = (oend-op) - 1;
lastRunSize -= (lastRunSize+240)/255;
}
ip = anchor + lastRunSize;

if (lastRunSize >= RUN_MASK) {
size_t accumulator = lastRunSize - RUN_MASK;
*op++ = RUN_MASK << ML_BITS;
for(; accumulator >= 255 ; accumulator-=255) *op++ = 255;
*op++ = (ut8) accumulator;
} else {
*op++ = (ut8)(lastRunSize<<ML_BITS);
}
memcpy(op, anchor, lastRunSize);
op += lastRunSize;
}


*srcSizePtr = (int) (((const char*)ip)-src);
return (int) (((char*)op)-dst);
}

static int LZ4_compress_destSize_extState (LZ4_stream_t* state, const char* src, char* dst, int* srcSizePtr, int targetDstSize) {
LZ4_resetStream (state);
if (targetDstSize >= LZ4_compressBound(*srcSizePtr)) { 
return LZ4_compress_fast_extState(state, src, dst, *srcSizePtr, targetDstSize, 1);
}
if (*srcSizePtr < LZ4_64Klimit) {
return LZ4_compress_destSize_generic(&state->internal_donotuse, src, dst, srcSizePtr, targetDstSize, byut16);
}
return LZ4_compress_destSize_generic(&state->internal_donotuse, src, dst, srcSizePtr, targetDstSize, sizeof(void*)==8 ? byut32 : byPtr);
}

int LZ4_compress_destSize(const char* src, char* dst, int* srcSizePtr, int targetDstSize) {
LZ4_stream_t ctx;
return LZ4_compress_destSize_extState(&ctx, src, dst, srcSizePtr, targetDstSize);
}





#define LZ4_STATIC_ASSERT(c) { enum { LZ4_static_assert = 1/(int)(!!(c)) }; } 
LZ4_stream_t* LZ4_createStream(void) {
LZ4_stream_t* lz4s = (LZ4_stream_t*)calloc(8, LZ4_STREAMSIZE_U64);
LZ4_STATIC_ASSERT (LZ4_STREAMSIZE >= sizeof(LZ4_stream_t_internal)); 
LZ4_resetStream(lz4s);
return lz4s;
}

void LZ4_resetStream (LZ4_stream_t* LZ4_stream) {
memset (LZ4_stream, 0, sizeof(LZ4_stream_t));
}

void LZ4_freeStream (LZ4_stream_t* LZ4_stream) {
free (LZ4_stream);
}

#define HASH_UNIT sizeof(reg_t)
int LZ4_loadDict (LZ4_stream_t* LZ4_dict, const char* dictionary, int dictSize) {
LZ4_stream_t_internal* dict = &LZ4_dict->internal_donotuse;
const ut8* p = (const ut8*)dictionary;
const ut8* const dictEnd = p + dictSize;
const ut8* base;

if ((dict->initCheck) || (dict->currentOffset > 1 GB)) { 
LZ4_resetStream(LZ4_dict);
}

if (dictSize < (int)HASH_UNIT) {
dict->dictionary = NULL;
dict->dictSize = 0;
return 0;
}

if ((dictEnd - p) > 64 KB) p = dictEnd - 64 KB;
dict->currentOffset += 64 KB;
base = p - dict->currentOffset;
dict->dictionary = p;
dict->dictSize = (ut32)(dictEnd - p);
dict->currentOffset += dict->dictSize;

while (p <= dictEnd-HASH_UNIT) {
LZ4_putPosition(p, dict->hashTable, byut32, base);
p+=3;
}

return dict->dictSize;
}

static void LZ4_renormDictT(LZ4_stream_t_internal* LZ4_dict, const ut8* src) {
if ((LZ4_dict->currentOffset > 0x80000000) || ((size_t)LZ4_dict->currentOffset > (size_t)src)) { 

ut32 const delta = LZ4_dict->currentOffset - 64 KB;
const ut8* dictEnd = LZ4_dict->dictionary + LZ4_dict->dictSize;
int i;
for (i=0; i<LZ4_HASH_SIZE_U32; i++) {
if (LZ4_dict->hashTable[i] < delta) LZ4_dict->hashTable[i]=0;
else LZ4_dict->hashTable[i] -= delta;
}
LZ4_dict->currentOffset = 64 KB;
if (LZ4_dict->dictSize > 64 KB) LZ4_dict->dictSize = 64 KB;
LZ4_dict->dictionary = dictEnd - LZ4_dict->dictSize;
}
}

int LZ4_compress_fast_continue (LZ4_stream_t* LZ4_stream, const char* source, char* dest, int inputSize, int maxOutputSize, int acceleration) {
LZ4_stream_t_internal* streamPtr = &LZ4_stream->internal_donotuse;
const ut8* const dictEnd = streamPtr->dictionary + streamPtr->dictSize;
const ut8* smallest = (const ut8*) source;

if (streamPtr->initCheck) {
return 0; 
}
if ((streamPtr->dictSize>0) && (smallest>dictEnd)) {
smallest = dictEnd;
}
LZ4_renormDictT (streamPtr, smallest);
if (acceleration < 1) {
acceleration = ACCELERATION_DEFAULT;
}

const ut8* sourceEnd = (const ut8*) source + inputSize;
if ((sourceEnd > streamPtr->dictionary) && (sourceEnd < dictEnd)) {
streamPtr->dictSize = (ut32)(dictEnd - sourceEnd);
if (streamPtr->dictSize > 64 KB) streamPtr->dictSize = 64 KB;
if (streamPtr->dictSize < 4) streamPtr->dictSize = 0;
streamPtr->dictionary = dictEnd - streamPtr->dictSize;
}


if (dictEnd == (const ut8*)source) {
int result;
if ((streamPtr->dictSize < 64 KB) && (streamPtr->dictSize < streamPtr->currentOffset))
result = LZ4_compress_generic(streamPtr, source, dest, inputSize, maxOutputSize, limitedOutput, byut32, withPrefix64k, dictSmall, acceleration);
else
result = LZ4_compress_generic(streamPtr, source, dest, inputSize, maxOutputSize, limitedOutput, byut32, withPrefix64k, noDictIssue, acceleration);
streamPtr->dictSize += (ut32)inputSize;
streamPtr->currentOffset += (ut32)inputSize;
return result;
}


int result = ((streamPtr->dictSize < 64 KB) && (streamPtr->dictSize < streamPtr->currentOffset))
? LZ4_compress_generic(streamPtr, source, dest, inputSize, maxOutputSize, limitedOutput, byut32, usingExtDict, dictSmall, acceleration)
: LZ4_compress_generic(streamPtr, source, dest, inputSize, maxOutputSize, limitedOutput, byut32, usingExtDict, noDictIssue, acceleration);
streamPtr->dictionary = (const ut8*)source;
streamPtr->dictSize = (ut32)inputSize;
streamPtr->currentOffset += (ut32)inputSize;
return result;
}


int LZ4_compress_forceExtDict (LZ4_stream_t* LZ4_dict, const char* source, char* dest, int inputSize) {
LZ4_stream_t_internal* streamPtr = &LZ4_dict->internal_donotuse;
int result;
const ut8* const dictEnd = streamPtr->dictionary + streamPtr->dictSize;

const ut8* smallest = dictEnd;
if (smallest > (const ut8*) source) smallest = (const ut8*) source;
LZ4_renormDictT(streamPtr, smallest);

result = LZ4_compress_generic(streamPtr, source, dest, inputSize, 0, notLimited, byut32, usingExtDict, noDictIssue, 1);

streamPtr->dictionary = (const ut8*)source;
streamPtr->dictSize = (ut32)inputSize;
streamPtr->currentOffset += (ut32)inputSize;

return result;
}








int LZ4_saveDict (LZ4_stream_t* LZ4_dict, char* safeBuffer, int dictSize) {
LZ4_stream_t_internal* const dict = &LZ4_dict->internal_donotuse;
const ut8* const previousDictEnd = dict->dictionary + dict->dictSize;

if ((ut32)dictSize > 64 KB) dictSize = 64 KB; 
if ((ut32)dictSize > dict->dictSize) dictSize = dict->dictSize;

memmove (safeBuffer, previousDictEnd - dictSize, dictSize);

dict->dictionary = (const ut8*)safeBuffer;
dict->dictSize = (ut32)dictSize;

return dictSize;
}










LZ4_FORCE_O2_GCC_PPC64LE
LZ4_FORCE_INLINE int LZ4_decompress_generic(
const char* const src,
char* const dst,
int srcSize,
int outputSize, 

int endOnInput, 
int partialDecoding, 
int targetOutputSize, 
int dict, 
const ut8* const lowPrefix, 
const ut8* const dictStart, 
const size_t dictSize 
)
{
const ut8* ip = (const ut8*) src;
const ut8* const iend = ip + srcSize;

ut8* op = (ut8*) dst;
ut8* const oend = op + outputSize;
ut8* cpy;
ut8* oexit = op + targetOutputSize;

const ut8* const dictEnd = (const ut8*)dictStart + dictSize;
const unsigned dec32table[] = {0, 1, 2, 1, 4, 4, 4, 4};
const int dec64table[] = {0, 0, 0, -1, 0, 1, 2, 3};

const int safeDecode = (endOnInput==endOnInputSize);
const int checkOffset = ((safeDecode) && (dictSize < (int)(64 KB)));



if ((partialDecoding) && (oexit > oend-MFLIMIT)) oexit = oend-MFLIMIT; 
if ((endOnInput) && (unlikely(outputSize==0))) return ((srcSize==1) && (*ip==0)) ? 0 : -1; 
if ((!endOnInput) && (unlikely(outputSize==0))) return (*ip==0?1:-1);


for (;;) {
size_t length;
const ut8* match;
size_t offset;


unsigned const token = *ip++;
if ((length=(token>>ML_BITS)) == RUN_MASK) {
unsigned s;
do {
s = *ip++;
length += s;
} while ( likely(endOnInput ? ip<iend-RUN_MASK : 1) & (s==255) );
if ((safeDecode) && unlikely((size_t)(op)+length<(size_t)(op))) goto _output_error; 
if ((safeDecode) && unlikely((size_t)(ip)+length<(size_t)(ip))) goto _output_error; 
}


cpy = op+length;
if ( ((endOnInput) && ((cpy>(partialDecoding?oexit:oend-MFLIMIT)) || (ip+length>iend-(2+1+LASTLITERALS))) )
|| ((!endOnInput) && (cpy>oend-WILDCOPYLENGTH)) )
{
if (partialDecoding) {
if (cpy > oend) goto _output_error; 
if ((endOnInput) && (ip+length > iend)) goto _output_error; 
} else {
if ((!endOnInput) && (cpy != oend)) goto _output_error; 
if ((endOnInput) && ((ip+length != iend) || (cpy > oend))) goto _output_error; 
}
memcpy(op, ip, length);
ip += length;
op += length;
break; 
}
LZ4_wildCopy(op, ip, cpy);
ip += length; op = cpy;


offset = LZ4_readLE16(ip); ip+=2;
match = op - offset;
if ((checkOffset) && (unlikely(match + dictSize < lowPrefix))) goto _output_error; 
LZ4_write32(op, (ut32)offset); 


length = token & ML_MASK;
if (length == ML_MASK) {
unsigned s;
do {
s = *ip++;
if ((endOnInput) && (ip > iend-LASTLITERALS)) goto _output_error;
length += s;
} while (s==255);
if ((safeDecode) && unlikely((size_t)(op)+length<(size_t)op)) goto _output_error; 
}
length += MINMATCH;


if ((dict==usingExtDict) && (match < lowPrefix)) {
if (unlikely(op+length > oend-LASTLITERALS)) goto _output_error; 

if (length <= (size_t)(lowPrefix-match)) {

memmove(op, dictEnd - (lowPrefix-match), length);
op += length;
} else {

size_t const copySize = (size_t)(lowPrefix-match);
size_t const restSize = length - copySize;
memcpy(op, dictEnd - copySize, copySize);
op += copySize;
if (restSize > (size_t)(op-lowPrefix)) { 
ut8* const endOfMatch = op + restSize;
const ut8* copyFrom = lowPrefix;
while (op < endOfMatch) *op++ = *copyFrom++;
} else {
memcpy(op, lowPrefix, restSize);
op += restSize;
} }
continue;
}


cpy = op + length;
if (unlikely(offset<8)) {
const int dec64 = dec64table[offset];
op[0] = match[0];
op[1] = match[1];
op[2] = match[2];
op[3] = match[3];
match += dec32table[offset];
memcpy(op+4, match, 4);
match -= dec64;
} else { LZ4_copy8(op, match); match+=8; }
op += 8;

if (unlikely(cpy>oend-12)) {
ut8* const oCopyLimit = oend-(WILDCOPYLENGTH-1);
if (cpy > oend-LASTLITERALS) goto _output_error; 
if (op < oCopyLimit) {
LZ4_wildCopy(op, match, oCopyLimit);
match += oCopyLimit - op;
op = oCopyLimit;
}
while (op<cpy) *op++ = *match++;
} else {
LZ4_copy8(op, match);
if (length>16) LZ4_wildCopy(op+8, match+8, cpy);
}
op = cpy; 
}


if (endOnInput) {
return (int) (((char*)op)-dst); 
}
return (int) (((const char*)ip)-src); 


_output_error:
return (int) (-(((const char*)ip)-src))-1;
}


LZ4_FORCE_O2_GCC_PPC64LE
int LZ4_decompress_safe(const char* source, char* dest, int compressedSize, int maxDecompressedSize)
{
return LZ4_decompress_generic(source, dest, compressedSize, maxDecompressedSize, endOnInputSize, full, 0, noDict, (ut8*)dest, NULL, 0);
}

LZ4_FORCE_O2_GCC_PPC64LE
int LZ4_decompress_safe_partial(const char* source, char* dest, int compressedSize, int targetOutputSize, int maxDecompressedSize)
{
return LZ4_decompress_generic(source, dest, compressedSize, maxDecompressedSize, endOnInputSize, partial, targetOutputSize, noDict, (ut8*)dest, NULL, 0);
}

LZ4_FORCE_O2_GCC_PPC64LE
int LZ4_decompress_fast(const char* source, char* dest, int originalSize) {
return LZ4_decompress_generic(source, dest, 0, originalSize, endOnOutputSize, full, 0, withPrefix64k, (ut8*)(dest - 64 KB), NULL, 64 KB);
}




LZ4_streamDecode_t* LZ4_createStreamDecode(void) {
return R_NEW0 (LZ4_streamDecode_t);
}

void LZ4_freeStreamDecode (LZ4_streamDecode_t* LZ4_stream) {
free (LZ4_stream);
}








int LZ4_setStreamDecode (LZ4_streamDecode_t* LZ4_streamDecode, const char* dictionary, int dictSize) {
LZ4_streamDecode_t_internal* lz4sd = &LZ4_streamDecode->internal_donotuse;
lz4sd->prefixSize = (size_t) dictSize;
lz4sd->prefixEnd = (const ut8*) dictionary + dictSize;
lz4sd->externalDict = NULL;
lz4sd->extDictSize = 0;
return 1;
}








LZ4_FORCE_O2_GCC_PPC64LE
int LZ4_decompress_safe_continue (LZ4_streamDecode_t* LZ4_streamDecode, const char* source, char* dest, int compressedSize, int maxOutputSize) {
LZ4_streamDecode_t_internal* lz4sd = &LZ4_streamDecode->internal_donotuse;
int result;

if (lz4sd->prefixEnd == (ut8*)dest) {
result = LZ4_decompress_generic(source, dest, compressedSize, maxOutputSize,
endOnInputSize, full, 0,
usingExtDict, lz4sd->prefixEnd - lz4sd->prefixSize, lz4sd->externalDict, lz4sd->extDictSize);
if (result <= 0) return result;
lz4sd->prefixSize += result;
lz4sd->prefixEnd += result;
} else {
lz4sd->extDictSize = lz4sd->prefixSize;
lz4sd->externalDict = lz4sd->prefixEnd - lz4sd->extDictSize;
result = LZ4_decompress_generic(source, dest, compressedSize, maxOutputSize,
endOnInputSize, full, 0,
usingExtDict, (ut8*)dest, lz4sd->externalDict, lz4sd->extDictSize);
if (result <= 0) return result;
lz4sd->prefixSize = result;
lz4sd->prefixEnd = (ut8*)dest + result;
}

return result;
}

LZ4_FORCE_O2_GCC_PPC64LE
int LZ4_decompress_fast_continue (LZ4_streamDecode_t* LZ4_streamDecode, const char* source, char* dest, int originalSize) {
LZ4_streamDecode_t_internal* lz4sd = &LZ4_streamDecode->internal_donotuse;
int result;

if (lz4sd->prefixEnd == (ut8*)dest) {
result = LZ4_decompress_generic(source, dest, 0, originalSize,
endOnOutputSize, full, 0,
usingExtDict, lz4sd->prefixEnd - lz4sd->prefixSize, lz4sd->externalDict, lz4sd->extDictSize);
if (result <= 0) return result;
lz4sd->prefixSize += originalSize;
lz4sd->prefixEnd += originalSize;
} else {
lz4sd->extDictSize = lz4sd->prefixSize;
lz4sd->externalDict = lz4sd->prefixEnd - lz4sd->extDictSize;
result = LZ4_decompress_generic(source, dest, 0, originalSize,
endOnOutputSize, full, 0,
usingExtDict, (ut8*)dest, lz4sd->externalDict, lz4sd->extDictSize);
if (result <= 0) return result;
lz4sd->prefixSize = originalSize;
lz4sd->prefixEnd = (ut8*)dest + originalSize;
}
return result;
}









LZ4_FORCE_O2_GCC_PPC64LE
LZ4_FORCE_INLINE int LZ4_decompress_usingDict_generic(const char* source, char* dest, int compressedSize, int maxOutputSize, int safe, const char* dictStart, int dictSize) {
if (dictSize==0) {
return LZ4_decompress_generic(source, dest, compressedSize, maxOutputSize, safe, full, 0, noDict, (ut8*)dest, NULL, 0);
}
if (dictStart+dictSize == dest) {
if (dictSize >= (int)(64 KB - 1))
return LZ4_decompress_generic(source, dest, compressedSize, maxOutputSize, safe, full, 0, withPrefix64k, (ut8*)dest-64 KB, NULL, 0);
return LZ4_decompress_generic(source, dest, compressedSize, maxOutputSize, safe, full, 0, noDict, (ut8*)dest-dictSize, NULL, 0);
}
return LZ4_decompress_generic(source, dest, compressedSize, maxOutputSize, safe, full, 0, usingExtDict, (ut8*)dest, (const ut8*)dictStart, dictSize);
}

LZ4_FORCE_O2_GCC_PPC64LE
int LZ4_decompress_safe_usingDict(const char* source, char* dest, int compressedSize, int maxOutputSize, const char* dictStart, int dictSize) {
return LZ4_decompress_usingDict_generic(source, dest, compressedSize, maxOutputSize, 1, dictStart, dictSize);
}

LZ4_FORCE_O2_GCC_PPC64LE
int LZ4_decompress_fast_usingDict(const char* source, char* dest, int originalSize, const char* dictStart, int dictSize) {
return LZ4_decompress_usingDict_generic(source, dest, 0, originalSize, 0, dictStart, dictSize);
}


LZ4_FORCE_O2_GCC_PPC64LE
int LZ4_decompress_safe_forceExtDict(const char* source, char* dest, int compressedSize, int maxOutputSize, const char* dictStart, int dictSize) {
return LZ4_decompress_generic(source, dest, compressedSize, maxOutputSize, endOnInputSize, full, 0, usingExtDict, (ut8*)dest, (const ut8*)dictStart, dictSize);
}






int LZ4_compress_limitedOutput(const char* source, char* dest, int inputSize, int maxOutputSize) { return LZ4_compress_default(source, dest, inputSize, maxOutputSize); }
int LZ4_compress(const char* source, char* dest, int inputSize) { return LZ4_compress_default(source, dest, inputSize, LZ4_compressBound(inputSize)); }
int LZ4_compress_limitedOutput_withState (void* state, const char* src, char* dst, int srcSize, int dstSize) { return LZ4_compress_fast_extState(state, src, dst, srcSize, dstSize, 1); }
int LZ4_compress_withState (void* state, const char* src, char* dst, int srcSize) { return LZ4_compress_fast_extState(state, src, dst, srcSize, LZ4_compressBound(srcSize), 1); }
int LZ4_compress_limitedOutput_continue (LZ4_stream_t* LZ4_stream, const char* src, char* dst, int srcSize, int maxDstSize) { return LZ4_compress_fast_continue(LZ4_stream, src, dst, srcSize, maxDstSize, 1); }
int LZ4_compress_continue (LZ4_stream_t* LZ4_stream, const char* source, char* dest, int inputSize) { return LZ4_compress_fast_continue(LZ4_stream, source, dest, inputSize, LZ4_compressBound(inputSize), 1); }







int LZ4_uncompress (const char* source, char* dest, int outputSize) { return LZ4_decompress_fast(source, dest, outputSize); }
int LZ4_uncompress_unknownOutputSize (const char* source, char* dest, int isize, int maxOutputSize) { return LZ4_decompress_safe(source, dest, isize, maxOutputSize); }




int LZ4_sizeofStreamState() { return LZ4_STREAMSIZE; }

static void LZ4_init(LZ4_stream_t* lz4ds, ut8* base) {
memset (lz4ds, 0, sizeof(LZ4_stream_t));
lz4ds->internal_donotuse.bufferStart = base;
}

int LZ4_resetStreamState(void* state, char* inputBuffer) {
if ((((size_t)state) & 3) != 0) return 1; 
LZ4_init((LZ4_stream_t*)state, (ut8*)inputBuffer);
return 0;
}

void* LZ4_create (char* inputBuffer) {
LZ4_stream_t* lz4ds = (LZ4_stream_t*)calloc(8, sizeof(LZ4_stream_t));
LZ4_init (lz4ds, (ut8*)inputBuffer);
return lz4ds;
}

char* LZ4_slideInputBuffer (void* LZ4_Data) {
LZ4_stream_t_internal* ctx = &((LZ4_stream_t*)LZ4_Data)->internal_donotuse;
int dictSize = LZ4_saveDict((LZ4_stream_t*)LZ4_Data, (char*)ctx->bufferStart, 64 KB);
return (char*)(ctx->bufferStart + dictSize);
}



int LZ4_decompress_safe_withPrefix64k(const char* source, char* dest, int compressedSize, int maxOutputSize) {
return LZ4_decompress_generic(source, dest, compressedSize, maxOutputSize, endOnInputSize, full, 0, withPrefix64k, (ut8*)dest - 64 KB, NULL, 64 KB);
}

int LZ4_decompress_fast_withPrefix64k(const char* source, char* dest, int originalSize) {
return LZ4_decompress_generic(source, dest, 0, originalSize, endOnOutputSize, full, 0, withPrefix64k, (ut8*)dest - 64 KB, NULL, 64 KB);
}

#endif 
