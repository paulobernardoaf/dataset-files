#if !defined(RUBY) && (defined(RUBY_EXPORT) || defined(ONIG_ENC_REGISTER))
#define RUBY
#endif
#if defined(RUBY)
#if !defined(ONIGMO_REGINT_H)
#if !defined(RUBY_EXTERN)
#include "ruby/config.h"
#include "ruby/defines.h"
#endif
#endif
#else 
#if !defined(PACKAGE)
#include "config.h"
#endif
#endif 
#if defined(ONIG_ESCAPE_UCHAR_COLLISION)
#undef ONIG_ESCAPE_UCHAR_COLLISION
#endif
#if defined(RUBY)
#include "ruby/onigmo.h"
#else
#include "onigmo.h"
#endif
RUBY_SYMBOL_EXPORT_BEGIN
typedef struct {
OnigCodePoint from;
OnigCodePoint to;
} OnigPairCaseFoldCodes;
#if !defined(NULL)
#define NULL ((void* )0)
#endif
#if !defined(TRUE)
#define TRUE 1
#endif
#if !defined(FALSE)
#define FALSE 0
#endif
#if !defined(ARG_UNUSED)
#if defined(__GNUC__)
#define ARG_UNUSED __attribute__ ((unused))
#else
#define ARG_UNUSED
#endif
#endif
#define ONIG_IS_NULL(p) (((void*)(p)) == (void*)0)
#define ONIG_IS_NOT_NULL(p) (((void*)(p)) != (void*)0)
#define ONIG_CHECK_NULL_RETURN(p) if (ONIG_IS_NULL(p)) return NULL
#define ONIG_CHECK_NULL_RETURN_VAL(p,val) if (ONIG_IS_NULL(p)) return (val)
#define enclen(enc,p,e) ((enc->max_enc_len == enc->min_enc_len) ? enc->min_enc_len : ONIGENC_MBC_ENC_LEN(enc,p,e))
#define BIT_CTYPE_NEWLINE (1<< ONIGENC_CTYPE_NEWLINE)
#define BIT_CTYPE_ALPHA (1<< ONIGENC_CTYPE_ALPHA)
#define BIT_CTYPE_BLANK (1<< ONIGENC_CTYPE_BLANK)
#define BIT_CTYPE_CNTRL (1<< ONIGENC_CTYPE_CNTRL)
#define BIT_CTYPE_DIGIT (1<< ONIGENC_CTYPE_DIGIT)
#define BIT_CTYPE_GRAPH (1<< ONIGENC_CTYPE_GRAPH)
#define BIT_CTYPE_LOWER (1<< ONIGENC_CTYPE_LOWER)
#define BIT_CTYPE_PRINT (1<< ONIGENC_CTYPE_PRINT)
#define BIT_CTYPE_PUNCT (1<< ONIGENC_CTYPE_PUNCT)
#define BIT_CTYPE_SPACE (1<< ONIGENC_CTYPE_SPACE)
#define BIT_CTYPE_UPPER (1<< ONIGENC_CTYPE_UPPER)
#define BIT_CTYPE_XDIGIT (1<< ONIGENC_CTYPE_XDIGIT)
#define BIT_CTYPE_WORD (1<< ONIGENC_CTYPE_WORD)
#define BIT_CTYPE_ALNUM (1<< ONIGENC_CTYPE_ALNUM)
#define BIT_CTYPE_ASCII (1<< ONIGENC_CTYPE_ASCII)
#define CTYPE_TO_BIT(ctype) (1<<(ctype))
#define CTYPE_IS_WORD_GRAPH_PRINT(ctype) ((ctype) == ONIGENC_CTYPE_WORD || (ctype) == ONIGENC_CTYPE_GRAPH ||(ctype) == ONIGENC_CTYPE_PRINT)
typedef struct {
short int len;
const UChar name[6];
int ctype;
} PosixBracketEntryType;
#define POSIX_BRACKET_ENTRY_INIT(name, ctype) {(short int )(sizeof(name) - 1), name, (ctype)}
#if !defined(numberof)
#define numberof(array) (int )(sizeof(array) / sizeof((array)[0]))
#endif
#define USE_CRNL_AS_LINE_TERMINATOR
#define USE_UNICODE_PROPERTIES
#define USE_UNICODE_AGE_PROPERTIES
#define ONIG_ENCODING_INIT_DEFAULT ONIG_ENCODING_ASCII
ONIG_EXTERN int onigenc_ascii_apply_all_case_fold(OnigCaseFoldType flag, OnigApplyAllCaseFoldFunc f, void* arg, OnigEncoding enc);
ONIG_EXTERN int onigenc_ascii_get_case_fold_codes_by_str(OnigCaseFoldType flag, const OnigUChar* p, const OnigUChar* end, OnigCaseFoldCodeItem items[], OnigEncoding enc);
ONIG_EXTERN int onigenc_apply_all_case_fold_with_map(int map_size, const OnigPairCaseFoldCodes map[], int ess_tsett_flag, OnigCaseFoldType flag, OnigApplyAllCaseFoldFunc f, void* arg);
ONIG_EXTERN int onigenc_get_case_fold_codes_by_str_with_map(int map_size, const OnigPairCaseFoldCodes map[], int ess_tsett_flag, OnigCaseFoldType flag, const OnigUChar* p, const OnigUChar* end, OnigCaseFoldCodeItem items[]);
ONIG_EXTERN int onigenc_not_support_get_ctype_code_range(OnigCtype ctype, OnigCodePoint* sb_out, const OnigCodePoint* ranges[], OnigEncoding enc);
ONIG_EXTERN int onigenc_is_mbc_newline_0x0a(const UChar* p, const UChar* end, OnigEncoding enc);
ONIG_EXTERN int onigenc_single_byte_ascii_only_case_map(OnigCaseFoldType* flagP, const OnigUChar** pp, const OnigUChar* end, OnigUChar* to, OnigUChar* to_end, const struct OnigEncodingTypeST* enc);
ONIG_EXTERN int onigenc_ascii_mbc_case_fold(OnigCaseFoldType flag, const UChar** p, const UChar* end, UChar* lower, OnigEncoding enc);
ONIG_EXTERN int onigenc_single_byte_mbc_enc_len(const UChar* p, const UChar* e, OnigEncoding enc);
ONIG_EXTERN OnigCodePoint onigenc_single_byte_mbc_to_code(const UChar* p, const UChar* end, OnigEncoding enc);
ONIG_EXTERN int onigenc_single_byte_code_to_mbclen(OnigCodePoint code, OnigEncoding enc);
ONIG_EXTERN int onigenc_single_byte_code_to_mbc(OnigCodePoint code, UChar *buf, OnigEncoding enc);
ONIG_EXTERN UChar* onigenc_single_byte_left_adjust_char_head(const UChar* start, const UChar* s, const OnigUChar* end, OnigEncoding enc);
ONIG_EXTERN int onigenc_always_true_is_allowed_reverse_match(const UChar* s, const UChar* end, OnigEncoding enc);
ONIG_EXTERN int onigenc_always_false_is_allowed_reverse_match(const UChar* s, const UChar* end, OnigEncoding enc);
ONIG_EXTERN int onigenc_ascii_is_code_ctype(OnigCodePoint code, unsigned int ctype, OnigEncoding enc);
ONIG_EXTERN OnigCodePoint onigenc_mbn_mbc_to_code(OnigEncoding enc, const UChar* p, const UChar* end);
ONIG_EXTERN int onigenc_mbn_mbc_case_fold(OnigEncoding enc, OnigCaseFoldType flag, const UChar** p, const UChar* end, UChar* lower);
ONIG_EXTERN int onigenc_mb2_code_to_mbclen(OnigCodePoint code, OnigEncoding enc);
ONIG_EXTERN int onigenc_mb2_code_to_mbc(OnigEncoding enc, OnigCodePoint code, UChar *buf);
ONIG_EXTERN int onigenc_minimum_property_name_to_ctype(OnigEncoding enc, const UChar* p, const UChar* end);
ONIG_EXTERN int onigenc_unicode_property_name_to_ctype(OnigEncoding enc, const UChar* p, const UChar* end);
ONIG_EXTERN int onigenc_mb2_is_code_ctype(OnigEncoding enc, OnigCodePoint code, unsigned int ctype);
ONIG_EXTERN int onigenc_mb4_code_to_mbclen(OnigCodePoint code, OnigEncoding enc);
ONIG_EXTERN int onigenc_mb4_code_to_mbc(OnigEncoding enc, OnigCodePoint code, UChar *buf);
ONIG_EXTERN int onigenc_mb4_is_code_ctype(OnigEncoding enc, OnigCodePoint code, unsigned int ctype);
ONIG_EXTERN int onigenc_unicode_case_map(OnigCaseFoldType* flagP, const OnigUChar** pp, const OnigUChar* end, OnigUChar* to, OnigUChar* to_end, const struct OnigEncodingTypeST* enc);
ONIG_EXTERN int onigenc_unicode_is_code_ctype(OnigCodePoint code, unsigned int ctype, OnigEncoding enc);
ONIG_EXTERN int onigenc_utf16_32_get_ctype_code_range(OnigCtype ctype, OnigCodePoint *sb_out, const OnigCodePoint* ranges[], OnigEncoding enc);
ONIG_EXTERN int onigenc_unicode_ctype_code_range(int ctype, const OnigCodePoint* ranges[]);
ONIG_EXTERN int onigenc_unicode_get_case_fold_codes_by_str(OnigEncoding enc, OnigCaseFoldType flag, const OnigUChar* p, const OnigUChar* end, OnigCaseFoldCodeItem items[]);
ONIG_EXTERN int onigenc_unicode_mbc_case_fold(OnigEncoding enc, OnigCaseFoldType flag, const UChar** pp, const UChar* end, UChar* fold);
ONIG_EXTERN int onigenc_unicode_apply_all_case_fold(OnigCaseFoldType flag, OnigApplyAllCaseFoldFunc f, void* arg, OnigEncoding enc);
#define UTF16_IS_SURROGATE_FIRST(c) (((c) & 0xfc) == 0xd8)
#define UTF16_IS_SURROGATE_SECOND(c) (((c) & 0xfc) == 0xdc)
#define UTF16_IS_SURROGATE(c) (((c) & 0xf8) == 0xd8)
#define UNICODE_VALID_CODEPOINT_P(c) ( ((c) <= 0x10ffff) && !((c) < 0x10000 && UTF16_IS_SURROGATE((c) >> 8)))
#define ONIGENC_ISO_8859_1_TO_LOWER_CASE(c) OnigEncISO_8859_1_ToLowerCaseTable[c]
#define ONIGENC_ISO_8859_1_TO_UPPER_CASE(c) OnigEncISO_8859_1_ToUpperCaseTable[c]
ONIG_EXTERN const UChar OnigEncISO_8859_1_ToLowerCaseTable[];
ONIG_EXTERN const UChar OnigEncISO_8859_1_ToUpperCaseTable[];
ONIG_EXTERN int
onigenc_with_ascii_strncmp(OnigEncoding enc, const UChar* p, const UChar* end, const UChar* sascii , int n);
ONIG_EXTERN int
onigenc_with_ascii_strnicmp(OnigEncoding enc, const UChar* p, const UChar* end, const UChar* sascii , int n);
ONIG_EXTERN UChar*
onigenc_step(OnigEncoding enc, const UChar* p, const UChar* end, int n);
extern int onig_is_in_code_range(const UChar* p, OnigCodePoint code);
ONIG_EXTERN OnigEncoding OnigEncDefaultCharEncoding;
ONIG_EXTERN const UChar OnigEncAsciiToLowerCaseTable[];
ONIG_EXTERN const UChar OnigEncAsciiToUpperCaseTable[];
ONIG_EXTERN const unsigned short OnigEncAsciiCtypeTable[];
#define ONIGENC_IS_ASCII_CODE(code) ((code) < 0x80)
#define ONIGENC_ASCII_CODE_TO_LOWER_CASE(c) OnigEncAsciiToLowerCaseTable[c]
#define ONIGENC_ASCII_CODE_TO_UPPER_CASE(c) OnigEncAsciiToUpperCaseTable[c]
#define ONIGENC_IS_ASCII_CODE_CTYPE(code,ctype) ((OnigEncAsciiCtypeTable[code] & CTYPE_TO_BIT(ctype)) != 0)
#define ONIGENC_IS_ASCII_CODE_CASE_AMBIG(code) (ONIGENC_IS_ASCII_CODE_CTYPE(code, ONIGENC_CTYPE_UPPER) ||ONIGENC_IS_ASCII_CODE_CTYPE(code, ONIGENC_CTYPE_LOWER))
#define ONIGENC_IS_IN_RANGE(code, from, to) ((OnigCodePoint )((code) - (from)) <= (OnigCodePoint )((to) - (from)))
#if defined(ONIG_ENC_REGISTER)
extern int ONIG_ENC_REGISTER(const char *, OnigEncoding);
#define OnigEncodingName(n) encoding_##n
#define OnigEncodingDeclare(n) static const OnigEncodingType OnigEncodingName(n)
#define OnigEncodingDefine(f,n) OnigEncodingDeclare(n); void Init_##f(void) { ONIG_ENC_REGISTER(OnigEncodingName(n).name, &OnigEncodingName(n)); } OnigEncodingDeclare(n)
#else
#define OnigEncodingName(n) OnigEncoding##n
#define OnigEncodingDeclare(n) const OnigEncodingType OnigEncodingName(n)
#define OnigEncodingDefine(f,n) OnigEncodingDeclare(n)
#endif
#define ENC_REPLICATE(name, orig)
#define ENC_ALIAS(name, orig)
#define ENC_DUMMY(name)
RUBY_SYMBOL_EXPORT_END
