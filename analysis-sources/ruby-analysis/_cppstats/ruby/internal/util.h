#include "ruby/config.h"
#include <stddef.h> 
#if defined(HAVE_SYS_TYPES_H)
#include <sys/types.h> 
#endif 
char *ruby_dtoa(double d_, int mode, int ndigits, int *decpt, int *sign, char **rve);
char *ruby_hdtoa(double d, const char *xdigs, int ndigits, int *decpt, int *sign, char **rve);
RUBY_SYMBOL_EXPORT_BEGIN
extern const signed char ruby_digit36_to_number_table[];
extern const char ruby_hexdigits[];
extern unsigned long ruby_scan_digits(const char *str, ssize_t len, int base, size_t *retlen, int *overflow);
RUBY_SYMBOL_EXPORT_END
