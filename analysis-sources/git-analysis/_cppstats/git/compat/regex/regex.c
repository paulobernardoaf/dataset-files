#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#if defined(__cplusplus)
#error "This is C code, use a C compiler"
#endif
#if defined(_LIBC)
#define regfree(preg) __regfree (preg)
#define regexec(pr, st, nm, pm, ef) __regexec (pr, st, nm, pm, ef)
#define regcomp(preg, pattern, cflags) __regcomp (preg, pattern, cflags)
#define regerror(errcode, preg, errbuf, errbuf_size) __regerror(errcode, preg, errbuf, errbuf_size)
#define re_set_registers(bu, re, nu, st, en) __re_set_registers (bu, re, nu, st, en)
#define re_match_2(bufp, string1, size1, string2, size2, pos, regs, stop) __re_match_2 (bufp, string1, size1, string2, size2, pos, regs, stop)
#define re_match(bufp, string, size, pos, regs) __re_match (bufp, string, size, pos, regs)
#define re_search(bufp, string, size, startpos, range, regs) __re_search (bufp, string, size, startpos, range, regs)
#define re_compile_pattern(pattern, length, bufp) __re_compile_pattern (pattern, length, bufp)
#define re_set_syntax(syntax) __re_set_syntax (syntax)
#define re_search_2(bufp, st1, s1, st2, s2, startpos, range, regs, stop) __re_search_2 (bufp, st1, s1, st2, s2, startpos, range, regs, stop)
#define re_compile_fastmap(bufp) __re_compile_fastmap (bufp)
#include "../locale/localeinfo.h"
#endif
#if defined (_MSC_VER)
#include <stdio.h> 
#endif
#include <limits.h>
#include <stdint.h>
#if defined(GAWK)
#undef alloca
#define alloca alloca_is_bad_you_should_never_use_it
#endif
#include <regex.h>
#include "regex_internal.h"
#include "regex_internal.c"
#if defined(GAWK)
#define bool int
#define true (1)
#define false (0)
#endif
#include "regcomp.c"
#include "regexec.c"
#if _LIBC
#include <shlib-compat.h>
#if SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_3)
link_warning (re_max_failures, "the 're_max_failures' variable is obsolete and will go away.")
int re_max_failures = 2000;
#endif
#endif
