#if defined(__cplusplus)
extern "C" {
#if 0
} 
#endif
#endif
#include "ruby/defines.h"
#if defined(RUBY_EXTCONF_H)
#include RUBY_EXTCONF_H
#endif
#if !defined(_)
#if defined(__cplusplus)
#if !defined(HAVE_PROTOTYPES)
#define HAVE_PROTOTYPES 1
#endif
#if !defined(HAVE_STDARG_PROTOTYPES)
#define HAVE_STDARG_PROTOTYPES 1
#endif
#endif
#if defined(HAVE_PROTOTYPES)
#define _(args) args
#else
#define _(args) ()
#endif
#if defined(HAVE_STDARG_PROTOTYPES)
#define __(args) args
#else
#define __(args) ()
#endif
#endif
RUBY_SYMBOL_EXPORT_BEGIN
#define DECIMAL_SIZE_OF_BITS(n) (((n) * 3010 + 9998) / 9999)
#define scan_oct(s,l,e) ((int)ruby_scan_oct((s),(l),(e)))
unsigned long ruby_scan_oct(const char *, size_t, size_t *);
#define scan_hex(s,l,e) ((int)ruby_scan_hex((s),(l),(e)))
unsigned long ruby_scan_hex(const char *, size_t, size_t *);
#if defined(HAVE_GNU_QSORT_R)
#define ruby_qsort qsort_r
#else
void ruby_qsort(void *, const size_t, const size_t,
int (*)(const void *, const void *, void *), void *);
#endif
void ruby_setenv(const char *, const char *);
void ruby_unsetenv(const char *);
char *ruby_strdup(const char *);
#undef strdup
#define strdup(s) ruby_strdup(s)
char *ruby_getcwd(void);
double ruby_strtod(const char *, char **);
#undef strtod
#define strtod(s,e) ruby_strtod((s),(e))
void ruby_each_words(const char *, void (*)(const char*, int, void*), void *);
RUBY_SYMBOL_EXPORT_END
#if defined(__cplusplus)
#if 0
{ 
#endif
} 
#endif
