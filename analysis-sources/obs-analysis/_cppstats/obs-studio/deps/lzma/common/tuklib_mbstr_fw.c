#include "tuklib_mbstr.h"
extern int
tuklib_mbstr_fw(const char *str, int columns_min)
{
size_t len;
const size_t width = tuklib_mbstr_width(str, &len);
if (width == (size_t)-1)
return -1;
if (width > (size_t)columns_min)
return 0;
if (width < (size_t)columns_min)
len += (size_t)columns_min - width;
return len;
}
