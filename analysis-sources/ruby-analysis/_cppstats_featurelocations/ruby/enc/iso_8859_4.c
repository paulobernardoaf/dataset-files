




























#include "regenc.h"
#include "iso_8859.h"

#define ENC_ISO_8859_4_TO_LOWER_CASE(c) EncISO_8859_4_ToLowerCaseTable[c]
#define ENC_IS_ISO_8859_4_CTYPE(code,ctype) ((EncISO_8859_4_CtypeTable[code] & CTYPE_TO_BIT(ctype)) != 0)


static const UChar EncISO_8859_4_ToLowerCaseTable[256] = {
'\000', '\001', '\002', '\003', '\004', '\005', '\006', '\007',
'\010', '\011', '\012', '\013', '\014', '\015', '\016', '\017',
'\020', '\021', '\022', '\023', '\024', '\025', '\026', '\027',
'\030', '\031', '\032', '\033', '\034', '\035', '\036', '\037',
'\040', '\041', '\042', '\043', '\044', '\045', '\046', '\047',
'\050', '\051', '\052', '\053', '\054', '\055', '\056', '\057',
'\060', '\061', '\062', '\063', '\064', '\065', '\066', '\067',
'\070', '\071', '\072', '\073', '\074', '\075', '\076', '\077',
'\100', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
'\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
'\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
'\170', '\171', '\172', '\133', '\134', '\135', '\136', '\137',
'\140', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
'\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
'\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
'\170', '\171', '\172', '\173', '\174', '\175', '\176', '\177',
'\200', '\201', '\202', '\203', '\204', '\205', '\206', '\207',
'\210', '\211', '\212', '\213', '\214', '\215', '\216', '\217',
'\220', '\221', '\222', '\223', '\224', '\225', '\226', '\227',
'\230', '\231', '\232', '\233', '\234', '\235', '\236', '\237',
'\240', '\261', '\242', '\263', '\244', '\265', '\266', '\247',
'\250', '\271', '\272', '\273', '\274', '\255', '\276', '\257',
'\260', '\261', '\262', '\263', '\264', '\265', '\266', '\267',
'\270', '\271', '\272', '\273', '\274', '\277', '\276', '\277',
'\340', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
'\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
'\360', '\361', '\362', '\363', '\364', '\365', '\366', '\327',
'\370', '\371', '\372', '\373', '\374', '\375', '\376', '\337',
'\340', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
'\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
'\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
'\370', '\371', '\372', '\373', '\374', '\375', '\376', '\377'
};

static const unsigned short EncISO_8859_4_CtypeTable[256] = {
0x4008, 0x4008, 0x4008, 0x4008, 0x4008, 0x4008, 0x4008, 0x4008,
0x4008, 0x420c, 0x4209, 0x4208, 0x4208, 0x4208, 0x4008, 0x4008,
0x4008, 0x4008, 0x4008, 0x4008, 0x4008, 0x4008, 0x4008, 0x4008,
0x4008, 0x4008, 0x4008, 0x4008, 0x4008, 0x4008, 0x4008, 0x4008,
0x4284, 0x41a0, 0x41a0, 0x41a0, 0x41a0, 0x41a0, 0x41a0, 0x41a0,
0x41a0, 0x41a0, 0x41a0, 0x41a0, 0x41a0, 0x41a0, 0x41a0, 0x41a0,
0x78b0, 0x78b0, 0x78b0, 0x78b0, 0x78b0, 0x78b0, 0x78b0, 0x78b0,
0x78b0, 0x78b0, 0x41a0, 0x41a0, 0x41a0, 0x41a0, 0x41a0, 0x41a0,
0x41a0, 0x7ca2, 0x7ca2, 0x7ca2, 0x7ca2, 0x7ca2, 0x7ca2, 0x74a2,
0x74a2, 0x74a2, 0x74a2, 0x74a2, 0x74a2, 0x74a2, 0x74a2, 0x74a2,
0x74a2, 0x74a2, 0x74a2, 0x74a2, 0x74a2, 0x74a2, 0x74a2, 0x74a2,
0x74a2, 0x74a2, 0x74a2, 0x41a0, 0x41a0, 0x41a0, 0x41a0, 0x51a0,
0x41a0, 0x78e2, 0x78e2, 0x78e2, 0x78e2, 0x78e2, 0x78e2, 0x70e2,
0x70e2, 0x70e2, 0x70e2, 0x70e2, 0x70e2, 0x70e2, 0x70e2, 0x70e2,
0x70e2, 0x70e2, 0x70e2, 0x70e2, 0x70e2, 0x70e2, 0x70e2, 0x70e2,
0x70e2, 0x70e2, 0x70e2, 0x41a0, 0x41a0, 0x41a0, 0x41a0, 0x4008,
0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008,
0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008,
0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008,
0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008,
0x0284, 0x34a2, 0x30e2, 0x34a2, 0x00a0, 0x34a2, 0x34a2, 0x00a0,
0x00a0, 0x34a2, 0x34a2, 0x34a2, 0x34a2, 0x01a0, 0x34a2, 0x00a0,
0x00a0, 0x30e2, 0x00a0, 0x30e2, 0x00a0, 0x30e2, 0x30e2, 0x00a0,
0x00a0, 0x30e2, 0x30e2, 0x30e2, 0x30e2, 0x34a2, 0x30e2, 0x30e2,
0x34a2, 0x34a2, 0x34a2, 0x34a2, 0x34a2, 0x34a2, 0x34a2, 0x34a2,
0x34a2, 0x34a2, 0x34a2, 0x34a2, 0x34a2, 0x34a2, 0x34a2, 0x34a2,
0x34a2, 0x34a2, 0x34a2, 0x34a2, 0x34a2, 0x34a2, 0x34a2, 0x00a0,
0x34a2, 0x34a2, 0x34a2, 0x34a2, 0x34a2, 0x34a2, 0x34a2, 0x30e2,
0x30e2, 0x30e2, 0x30e2, 0x30e2, 0x30e2, 0x30e2, 0x30e2, 0x30e2,
0x30e2, 0x30e2, 0x30e2, 0x30e2, 0x30e2, 0x30e2, 0x30e2, 0x30e2,
0x30e2, 0x30e2, 0x30e2, 0x30e2, 0x30e2, 0x30e2, 0x30e2, 0x00a0,
0x30e2, 0x30e2, 0x30e2, 0x30e2, 0x30e2, 0x30e2, 0x30e2, 0x00a0
};

static int
mbc_case_fold(OnigCaseFoldType flag,
const UChar** pp, const UChar* end ARG_UNUSED, UChar* lower,
OnigEncoding enc ARG_UNUSED)
{
const UChar* p = *pp;

if (*p == SHARP_s && (flag & INTERNAL_ONIGENC_CASE_FOLD_MULTI_CHAR) != 0) {
*lower++ = 's';
*lower = 's';
(*pp)++;
return 2;
}

*lower = ENC_ISO_8859_4_TO_LOWER_CASE(*p);
(*pp)++;
return 1; 
}

#if 0
static int
is_mbc_ambiguous(OnigCaseFoldType flag, const UChar** pp, const UChar* end)
{
int v;
const UChar* p = *pp;

if (*p == SHARP_s && (flag & INTERNAL_ONIGENC_CASE_FOLD_MULTI_CHAR) != 0) {
(*pp)++;
return TRUE;
}

(*pp)++;
v = (EncISO_8859_4_CtypeTable[*p] & (BIT_CTYPE_UPPER | BIT_CTYPE_LOWER));
if ((v | BIT_CTYPE_LOWER) != 0) {
if (*p == 0xa2)
return FALSE;
else
return TRUE;
}

return (v != 0 ? TRUE : FALSE);
}
#endif

static int
is_code_ctype(OnigCodePoint code, unsigned int ctype, OnigEncoding enc ARG_UNUSED)
{
if (code < 256)
return ENC_IS_ISO_8859_4_CTYPE(code, ctype);
else
return FALSE;
}

static const OnigPairCaseFoldCodes CaseFoldMap[] = {
{ 0xa1, 0xb1 },
{ 0xa3, 0xb3 },
{ 0xa5, 0xb5 },
{ 0xa6, 0xb6 },
{ 0xa9, 0xb9 },
{ 0xaa, 0xba },
{ 0xab, 0xbb },
{ 0xac, 0xbc },
{ 0xae, 0xbe },
{ 0xbd, 0xbf },

{ 0xc0, 0xe0 },
{ 0xc1, 0xe1 },
{ 0xc2, 0xe2 },
{ 0xc3, 0xe3 },
{ 0xc4, 0xe4 },
{ 0xc5, 0xe5 },
{ 0xc6, 0xe6 },
{ 0xc7, 0xe7 },
{ 0xc8, 0xe8 },
{ 0xc9, 0xe9 },
{ 0xca, 0xea },
{ 0xcb, 0xeb },
{ 0xcc, 0xec },
{ 0xcd, 0xed },
{ 0xce, 0xee },
{ 0xcf, 0xef },

{ 0xd0, 0xf0 },
{ 0xd1, 0xf1 },
{ 0xd2, 0xf2 },
{ 0xd3, 0xf3 },
{ 0xd4, 0xf4 },
{ 0xd5, 0xf5 },
{ 0xd6, 0xf6 },
{ 0xd8, 0xf8 },
{ 0xd9, 0xf9 },
{ 0xda, 0xfa },
{ 0xdb, 0xfb },
{ 0xdc, 0xfc },
{ 0xdd, 0xfd },
{ 0xde, 0xfe }
};

static int
apply_all_case_fold(OnigCaseFoldType flag,
OnigApplyAllCaseFoldFunc f, void* arg,
OnigEncoding enc ARG_UNUSED)
{
return onigenc_apply_all_case_fold_with_map(
numberof(CaseFoldMap), CaseFoldMap, 1,
flag, f, arg);
}

static int
get_case_fold_codes_by_str(OnigCaseFoldType flag,
const OnigUChar* p, const OnigUChar* end,
OnigCaseFoldCodeItem items[],
OnigEncoding enc ARG_UNUSED)
{
return onigenc_get_case_fold_codes_by_str_with_map(
numberof(CaseFoldMap), CaseFoldMap, 1,
flag, p, end, items);
}

static int
case_map(OnigCaseFoldType* flagP, const OnigUChar** pp,
const OnigUChar* end, OnigUChar* to, OnigUChar* to_end,
const struct OnigEncodingTypeST* enc)
{
OnigCodePoint code;
OnigUChar *to_start = to;
OnigCaseFoldType flags = *flagP;

while (*pp < end && to < to_end) {
code = *(*pp)++;
if (code == SHARP_s) {
if (flags & ONIGENC_CASE_UPCASE) {
flags |= ONIGENC_CASE_MODIFIED;
*to++ = 'S';
code = (flags & ONIGENC_CASE_TITLECASE) ? 's' : 'S';
}
else if (flags & ONIGENC_CASE_FOLD) {
flags |= ONIGENC_CASE_MODIFIED;
*to++ = 's';
code = 's';
}
}
else if ((EncISO_8859_4_CtypeTable[code] & BIT_CTYPE_UPPER)
&& (flags & (ONIGENC_CASE_DOWNCASE | ONIGENC_CASE_FOLD))) {
flags |= ONIGENC_CASE_MODIFIED;
code = ENC_ISO_8859_4_TO_LOWER_CASE(code);
}
else if (code == 0xA2)
;
else if ((EncISO_8859_4_CtypeTable[code]&BIT_CTYPE_LOWER)
&& (flags & ONIGENC_CASE_UPCASE)) {
flags |= ONIGENC_CASE_MODIFIED;
if (code >= 0xA0 && code <= 0xBF) {
if (code == 0xBF)
code -= 0x02;
else
code -= 0x10;
}
else
code -= 0x20;
}
*to++ = code;
if (flags & ONIGENC_CASE_TITLECASE) 
flags ^= (ONIGENC_CASE_UPCASE | ONIGENC_CASE_DOWNCASE | ONIGENC_CASE_TITLECASE);
}
*flagP = flags;
return (int )(to - to_start);
}

OnigEncodingDefine(iso_8859_4, ISO_8859_4) = {
onigenc_single_byte_mbc_enc_len,
"ISO-8859-4", 
1, 
1, 
onigenc_is_mbc_newline_0x0a,
onigenc_single_byte_mbc_to_code,
onigenc_single_byte_code_to_mbclen,
onigenc_single_byte_code_to_mbc,
mbc_case_fold,
apply_all_case_fold,
get_case_fold_codes_by_str,
onigenc_minimum_property_name_to_ctype,
is_code_ctype,
onigenc_not_support_get_ctype_code_range,
onigenc_single_byte_left_adjust_char_head,
onigenc_always_true_is_allowed_reverse_match,
case_map,
0,
ONIGENC_FLAG_NONE,
};
ENC_ALIAS("ISO8859-4", "ISO-8859-4")
