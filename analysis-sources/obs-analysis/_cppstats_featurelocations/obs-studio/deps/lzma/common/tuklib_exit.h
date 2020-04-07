












#if !defined(TUKLIB_EXIT_H)
#define TUKLIB_EXIT_H

#include "tuklib_common.h"
TUKLIB_DECLS_BEGIN

#define tuklib_exit TUKLIB_SYMBOL(tuklib_exit)
extern void tuklib_exit(int status, int err_status, int show_error)
tuklib_attr_noreturn;

TUKLIB_DECLS_END
#endif
