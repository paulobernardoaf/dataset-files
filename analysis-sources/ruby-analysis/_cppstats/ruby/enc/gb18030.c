#include "regenc.h"
#if 1
#define DEBUG_GB18030(arg)
#else
#define DEBUG_GB18030(arg) printf arg
#endif
enum {
C1, 
C2, 
C4, 
CM 
};
static const char GB18030_MAP[] = {
C1, C1, C1, C1, C1, C1, C1, C1, C1, C1, C1, C1, C1, C1, C1, C1,
C1, C1, C1, C1, C1, C1, C1, C1, C1, C1, C1, C1, C1, C1, C1, C1,
C1, C1, C1, C1, C1, C1, C1, C1, C1, C1, C1, C1, C1, C1, C1, C1,
C4, C4, C4, C4, C4, C4, C4, C4, C4, C4, C1, C1, C1, C1, C1, C1,
C2, C2, C2, C2, C2, C2, C2, C2, C2, C2, C2, C2, C2, C2, C2, C2,
C2, C2, C2, C2, C2, C2, C2, C2, C2, C2, C2, C2, C2, C2, C2, C2,
C2, C2, C2, C2, C2, C2, C2, C2, C2, C2, C2, C2, C2, C2, C2, C2,
C2, C2, C2, C2, C2, C2, C2, C2, C2, C2, C2, C2, C2, C2, C2, C1,
C2, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM,
CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM,
CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM,
CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM,
CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM,
CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM,
CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM,
CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, CM, C1
};
typedef enum { FAILURE = -2, ACCEPT = -1, S0 = 0, S1, S2, S3 } state_t;
#define A ACCEPT
#define F FAILURE
static const signed char trans[][0x100] = {
{ 
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
F, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, F
},
{ 
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, F, F, F, F, F, F,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, F,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, F
},
{ 
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, F
},
{ 
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
A, A, A, A, A, A, A, A, A, A, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F
}
};
#undef A
#undef F
static int
gb18030_mbc_enc_len(const UChar* p, const UChar* e, OnigEncoding enc ARG_UNUSED)
{
int firstbyte = *p++;
state_t s = trans[0][firstbyte];
#define RETURN(n) return s == ACCEPT ? ONIGENC_CONSTRUCT_MBCLEN_CHARFOUND(n) : ONIGENC_CONSTRUCT_MBCLEN_INVALID()
if (s < 0) RETURN(1);
if (p == e) return ONIGENC_CONSTRUCT_MBCLEN_NEEDMORE(2-1);
s = trans[s][*p++];
if (s < 0) RETURN(2);
if (p == e) return ONIGENC_CONSTRUCT_MBCLEN_NEEDMORE(4-2);
s = trans[s][*p++];
if (s < 0) RETURN(3);
if (p == e) return ONIGENC_CONSTRUCT_MBCLEN_NEEDMORE(4-3);
s = trans[s][*p++];
RETURN(4);
#undef RETURN
}
static OnigCodePoint
gb18030_mbc_to_code(const UChar* p, const UChar* end, OnigEncoding enc)
{
int c, i, len;
OnigCodePoint n;
len = enclen(enc, p, end);
n = (OnigCodePoint )(*p++);
if (len == 1) return n;
for (i = 1; i < len; i++) {
if (p >= end) break;
c = *p++;
n <<= 8; n += c;
}
return n;
}
static int
gb18030_code_to_mbc(OnigCodePoint code, UChar *buf, OnigEncoding enc)
{
return onigenc_mb4_code_to_mbc(enc, code, buf);
}
static int
gb18030_mbc_case_fold(OnigCaseFoldType flag, const UChar** pp, const UChar* end,
UChar* lower, OnigEncoding enc)
{
return onigenc_mbn_mbc_case_fold(enc, flag,
pp, end, lower);
}
#if 0
static int
gb18030_is_mbc_ambiguous(OnigCaseFoldType flag,
const UChar** pp, const UChar* end, OnigEncoding enc)
{
return onigenc_mbn_is_mbc_ambiguous(enc, flag, pp, end);
}
#endif
static int
gb18030_is_code_ctype(OnigCodePoint code, unsigned int ctype, OnigEncoding enc)
{
return onigenc_mb4_is_code_ctype(enc, code, ctype);
}
enum state {
S_START,
S_one_C2,
S_one_C4,
S_one_CM,
S_odd_CM_one_CX,
S_even_CM_one_CX,
S_one_CMC4,
S_odd_CMC4,
S_one_C4_odd_CMC4,
S_even_CMC4,
S_one_C4_even_CMC4,
S_odd_CM_odd_CMC4,
S_even_CM_odd_CMC4,
S_odd_CM_even_CMC4,
S_even_CM_even_CMC4,
S_odd_C4CM,
S_one_CM_odd_C4CM,
S_even_C4CM,
S_one_CM_even_C4CM,
S_even_CM_odd_C4CM,
S_odd_CM_odd_C4CM,
S_even_CM_even_C4CM,
S_odd_CM_even_C4CM
};
static UChar*
gb18030_left_adjust_char_head(const UChar* start, const UChar* s, const UChar* end, OnigEncoding enc)
{
const UChar *p;
enum state state = S_START;
DEBUG_GB18030(("----------------\n"));
for (p = s; p >= start; p--) {
DEBUG_GB18030(("state %d --(%02x)-->\n", state, *p));
switch (state) {
case S_START:
switch (GB18030_MAP[*p]) {
case C1:
return (UChar *)s;
case C2:
state = S_one_C2; 
break;
case C4:
state = S_one_C4; 
break;
case CM:
state = S_one_CM; 
break;
}
break;
case S_one_C2: 
switch (GB18030_MAP[*p]) {
case C1:
case C2:
case C4:
return (UChar *)s;
case CM:
state = S_odd_CM_one_CX; 
break;
}
break;
case S_one_C4: 
switch (GB18030_MAP[*p]) {
case C1:
case C2:
case C4:
return (UChar *)s;
case CM:
state = S_one_CMC4;
break;
}
break;
case S_one_CM: 
switch (GB18030_MAP[*p]) {
case C1:
case C2:
return (UChar *)s;
case C4:
state = S_odd_C4CM;
break;
case CM:
state = S_odd_CM_one_CX; 
break;
}
break;
case S_odd_CM_one_CX: 
switch (GB18030_MAP[*p]) {
case C1:
case C2:
case C4:
return (UChar *)(s - 1);
case CM:
state = S_even_CM_one_CX;
break;
}
break;
case S_even_CM_one_CX: 
switch (GB18030_MAP[*p]) {
case C1:
case C2:
case C4:
return (UChar *)s;
case CM:
state = S_odd_CM_one_CX;
break;
}
break;
case S_one_CMC4: 
switch (GB18030_MAP[*p]) {
case C1:
case C2:
return (UChar *)(s - 1);
case C4:
state = S_one_C4_odd_CMC4; 
break;
case CM:
state = S_even_CM_one_CX; 
break;
}
break;
case S_odd_CMC4: 
switch (GB18030_MAP[*p]) {
case C1:
case C2:
return (UChar *)(s - 1);
case C4:
state = S_one_C4_odd_CMC4;
break;
case CM:
state = S_odd_CM_odd_CMC4;
break;
}
break;
case S_one_C4_odd_CMC4: 
switch (GB18030_MAP[*p]) {
case C1:
case C2:
case C4:
return (UChar *)(s - 1);
case CM:
state = S_even_CMC4; 
break;
}
break;
case S_even_CMC4: 
switch (GB18030_MAP[*p]) {
case C1:
case C2:
return (UChar *)(s - 3);
case C4:
state = S_one_C4_even_CMC4;
break;
case CM:
state = S_odd_CM_even_CMC4;
break;
}
break;
case S_one_C4_even_CMC4: 
switch (GB18030_MAP[*p]) {
case C1:
case C2:
case C4:
return (UChar *)(s - 3);
case CM:
state = S_odd_CMC4;
break;
}
break;
case S_odd_CM_odd_CMC4: 
switch (GB18030_MAP[*p]) {
case C1:
case C2:
case C4:
return (UChar *)(s - 3);
case CM:
state = S_even_CM_odd_CMC4;
break;
}
break;
case S_even_CM_odd_CMC4: 
switch (GB18030_MAP[*p]) {
case C1:
case C2:
case C4:
return (UChar *)(s - 1);
case CM:
state = S_odd_CM_odd_CMC4;
break;
}
break;
case S_odd_CM_even_CMC4: 
switch (GB18030_MAP[*p]) {
case C1:
case C2:
case C4:
return (UChar *)(s - 1);
case CM:
state = S_even_CM_even_CMC4;
break;
}
break;
case S_even_CM_even_CMC4: 
switch (GB18030_MAP[*p]) {
case C1:
case C2:
case C4:
return (UChar *)(s - 3);
case CM:
state = S_odd_CM_even_CMC4;
break;
}
break;
case S_odd_C4CM: 
switch (GB18030_MAP[*p]) {
case C1:
case C2:
case C4:
return (UChar *)s;
case CM:
state = S_one_CM_odd_C4CM; 
break;
}
break;
case S_one_CM_odd_C4CM: 
switch (GB18030_MAP[*p]) {
case C1:
case C2:
return (UChar *)(s - 2); 
case C4:
state = S_even_C4CM;
break;
case CM:
state = S_even_CM_odd_C4CM;
break;
}
break;
case S_even_C4CM: 
switch (GB18030_MAP[*p]) {
case C1:
case C2:
case C4:
return (UChar *)(s - 2); 
case CM:
state = S_one_CM_even_C4CM;
break;
}
break;
case S_one_CM_even_C4CM: 
switch (GB18030_MAP[*p]) {
case C1:
case C2:
return (UChar *)(s - 0); 
case C4:
state = S_odd_C4CM;
break;
case CM:
state = S_even_CM_even_C4CM;
break;
}
break;
case S_even_CM_odd_C4CM: 
switch (GB18030_MAP[*p]) {
case C1:
case C2:
case C4:
return (UChar *)(s - 0); 
case CM:
state = S_odd_CM_odd_C4CM;
break;
}
break;
case S_odd_CM_odd_C4CM: 
switch (GB18030_MAP[*p]) {
case C1:
case C2:
case C4:
return (UChar *)(s - 2); 
case CM:
state = S_even_CM_odd_C4CM;
break;
}
break;
case S_even_CM_even_C4CM: 
switch (GB18030_MAP[*p]) {
case C1:
case C2:
case C4:
return (UChar *)(s - 2); 
case CM:
state = S_odd_CM_even_C4CM;
break;
}
break;
case S_odd_CM_even_C4CM: 
switch (GB18030_MAP[*p]) {
case C1:
case C2:
case C4:
return (UChar *)(s - 0); 
case CM:
state = S_even_CM_even_C4CM;
break;
}
break;
}
}
DEBUG_GB18030(("state %d\n", state));
switch (state) {
case S_START: return (UChar *)(s - 0);
case S_one_C2: return (UChar *)(s - 0);
case S_one_C4: return (UChar *)(s - 0);
case S_one_CM: return (UChar *)(s - 0);
case S_odd_CM_one_CX: return (UChar *)(s - 1);
case S_even_CM_one_CX: return (UChar *)(s - 0);
case S_one_CMC4: return (UChar *)(s - 1);
case S_odd_CMC4: return (UChar *)(s - 1);
case S_one_C4_odd_CMC4: return (UChar *)(s - 1);
case S_even_CMC4: return (UChar *)(s - 3);
case S_one_C4_even_CMC4: return (UChar *)(s - 3);
case S_odd_CM_odd_CMC4: return (UChar *)(s - 3);
case S_even_CM_odd_CMC4: return (UChar *)(s - 1);
case S_odd_CM_even_CMC4: return (UChar *)(s - 1);
case S_even_CM_even_CMC4: return (UChar *)(s - 3);
case S_odd_C4CM: return (UChar *)(s - 0);
case S_one_CM_odd_C4CM: return (UChar *)(s - 2);
case S_even_C4CM: return (UChar *)(s - 2);
case S_one_CM_even_C4CM: return (UChar *)(s - 0);
case S_even_CM_odd_C4CM: return (UChar *)(s - 0);
case S_odd_CM_odd_C4CM: return (UChar *)(s - 2);
case S_even_CM_even_C4CM: return (UChar *)(s - 2);
case S_odd_CM_even_C4CM: return (UChar *)(s - 0);
}
return (UChar* )s; 
}
static int
gb18030_is_allowed_reverse_match(const UChar* s, const UChar* end ARG_UNUSED, OnigEncoding enc ARG_UNUSED)
{
return GB18030_MAP[*s] == C1 ? TRUE : FALSE;
}
OnigEncodingDefine(gb18030, GB18030) = {
gb18030_mbc_enc_len,
"GB18030", 
4, 
1, 
onigenc_is_mbc_newline_0x0a,
gb18030_mbc_to_code,
onigenc_mb4_code_to_mbclen,
gb18030_code_to_mbc,
gb18030_mbc_case_fold,
onigenc_ascii_apply_all_case_fold,
onigenc_ascii_get_case_fold_codes_by_str,
onigenc_minimum_property_name_to_ctype,
gb18030_is_code_ctype,
onigenc_not_support_get_ctype_code_range,
gb18030_left_adjust_char_head,
gb18030_is_allowed_reverse_match,
onigenc_ascii_only_case_map,
0,
ONIGENC_FLAG_NONE,
};
