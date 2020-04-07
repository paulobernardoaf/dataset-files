#include "tuklib_common.h"
TUKLIB_DECLS_BEGIN
#define tuklib_mbstr_width TUKLIB_SYMBOL(tuklib_mbstr_width)
extern size_t tuklib_mbstr_width(const char *str, size_t *bytes);
#define tuklib_mbstr_fw TUKLIB_SYMBOL(tuklib_mbstr_fw)
extern int tuklib_mbstr_fw(const char *str, int columns_min);
TUKLIB_DECLS_END
