#include <r_types.h>
#if defined(__cplusplus)
extern "C" {
#endif
R_API char* r_debruijn_pattern(int size, int start, const char* charset);
R_API int r_debruijn_offset(ut64 value, bool is_big_endian);
#if defined(__cplusplus)
}
#endif
