#define HOST_CHARSET_UNKNOWN 0
#define HOST_CHARSET_ASCII 1
#define HOST_CHARSET_EBCDIC 2
#if '\n' == 0x0A && ' ' == 0x20 && '0' == 0x30 && 'A' == 0x41 && 'a' == 0x61 && '!' == 0x21
#define HOST_CHARSET HOST_CHARSET_ASCII
#else
#if '\n' == 0x15 && ' ' == 0x40 && '0' == 0xF0 && 'A' == 0xC1 && 'a' == 0x81 && '!' == 0x5A
#define HOST_CHARSET HOST_CHARSET_EBCDIC
#else
#define HOST_CHARSET HOST_CHARSET_UNKNOWN
#endif
#endif
enum {
_sch_isblank = 0x0001, 
_sch_iscntrl = 0x0002, 
_sch_isdigit = 0x0004, 
_sch_islower = 0x0008, 
_sch_isprint = 0x0010, 
_sch_ispunct = 0x0020, 
_sch_isspace = 0x0040, 
_sch_isupper = 0x0080, 
_sch_isxdigit = 0x0100, 
_sch_isidst = 0x0200, 
_sch_isvsp = 0x0400, 
_sch_isnvsp = 0x0800, 
_sch_isalpha = _sch_isupper|_sch_islower, 
_sch_isalnum = _sch_isalpha|_sch_isdigit, 
_sch_isidnum = _sch_isidst|_sch_isdigit, 
_sch_isgraph = _sch_isalnum|_sch_ispunct, 
_sch_iscppsp = _sch_isvsp|_sch_isnvsp, 
_sch_isbasic = _sch_isprint|_sch_iscppsp 
};
extern const unsigned short _sch_istable[256];
#define _sch_test(c, bit) (_sch_istable[(c) & 0xff] & (unsigned short)(bit))
#define ISALPHA(c) _sch_test(c, _sch_isalpha)
#define ISALNUM(c) _sch_test(c, _sch_isalnum)
#define ISBLANK(c) _sch_test(c, _sch_isblank)
#define ISCNTRL(c) _sch_test(c, _sch_iscntrl)
#define ISDIGIT(c) _sch_test(c, _sch_isdigit)
#define ISGRAPH(c) _sch_test(c, _sch_isgraph)
#define ISLOWER(c) _sch_test(c, _sch_islower)
#define ISPRINT(c) _sch_test(c, _sch_isprint)
#define ISPUNCT(c) _sch_test(c, _sch_ispunct)
#define ISSPACE(c) _sch_test(c, _sch_isspace)
#define ISUPPER(c) _sch_test(c, _sch_isupper)
#define ISXDIGIT(c) _sch_test(c, _sch_isxdigit)
#define ISIDNUM(c) _sch_test(c, _sch_isidnum)
#define ISIDST(c) _sch_test(c, _sch_isidst)
#define IS_ISOBASIC(c) _sch_test(c, _sch_isbasic)
#define IS_VSPACE(c) _sch_test(c, _sch_isvsp)
#define IS_NVSPACE(c) _sch_test(c, _sch_isnvsp)
#define IS_SPACE_OR_NUL(c) _sch_test(c, _sch_iscppsp)
extern const unsigned char _sch_toupper[256];
extern const unsigned char _sch_tolower[256];
#define TOUPPER(c) _sch_toupper[(c) & 0xff]
#define TOLOWER(c) _sch_tolower[(c) & 0xff]
#include <ctype.h>
#undef isalpha
#define isalpha(c) do_not_use_isalpha_with_safe_ctype
#undef isalnum
#define isalnum(c) do_not_use_isalnum_with_safe_ctype
#undef iscntrl
#define iscntrl(c) do_not_use_iscntrl_with_safe_ctype
#undef isdigit
#define isdigit(c) do_not_use_isdigit_with_safe_ctype
#undef isgraph
#define isgraph(c) do_not_use_isgraph_with_safe_ctype
#undef islower
#define islower(c) do_not_use_islower_with_safe_ctype
#undef isprint
#define isprint(c) do_not_use_isprint_with_safe_ctype
#undef ispunct
#define ispunct(c) do_not_use_ispunct_with_safe_ctype
#undef isspace
#define isspace(c) do_not_use_isspace_with_safe_ctype
#undef isupper
#define isupper(c) do_not_use_isupper_with_safe_ctype
#undef isxdigit
#define isxdigit(c) do_not_use_isxdigit_with_safe_ctype
#undef toupper
#define toupper(c) do_not_use_toupper_with_safe_ctype
#undef tolower
#define tolower(c) do_not_use_tolower_with_safe_ctype
