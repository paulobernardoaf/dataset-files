











#include "tuklib_mbstr.h"

#if defined(HAVE_MBRTOWC) && defined(HAVE_WCWIDTH)
#include <wchar.h>
#endif


extern size_t
tuklib_mbstr_width(const char *str, size_t *bytes)
{
const size_t len = strlen(str);
if (bytes != NULL)
*bytes = len;

#if !(defined(HAVE_MBRTOWC) && defined(HAVE_WCWIDTH))


return len;

#else
mbstate_t state;
memset(&state, 0, sizeof(state));

size_t width = 0;
size_t i = 0;



while (i < len) {
wchar_t wc;
const size_t ret = mbrtowc(&wc, str + i, len - i, &state);
if (ret < 1 || ret > len)
return (size_t)-1;

i += ret;

const int wc_width = wcwidth(wc);
if (wc_width < 0)
return (size_t)-1;

width += wc_width;
}




if (!mbsinit(&state))
return (size_t)-1;

return width;
#endif
}
