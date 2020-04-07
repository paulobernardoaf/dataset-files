#if defined(__cplusplus)
extern "C" {
#if 0
} 
#endif
#endif
#if defined(RUBY)
#include "ruby/oniguruma.h"
#else
#include "oniguruma.h"
#endif
RUBY_SYMBOL_EXPORT_BEGIN
#if !defined(ONIG_RUBY_M17N)
ONIG_EXTERN OnigEncoding OnigEncDefaultCharEncoding;
#define mbclen(p,e,enc) rb_enc_mbclen((p),(e),(enc))
#endif 
RUBY_SYMBOL_EXPORT_END
#if defined(__cplusplus)
#if 0
{ 
#endif
} 
#endif
