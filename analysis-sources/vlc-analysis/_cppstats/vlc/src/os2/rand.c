#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_rand.h>
void vlc_rand_bytes (void *buf, size_t len)
{
QWORD qwTime;
uint8_t *p_buf = (uint8_t *)buf;
while (len > 0)
{
DosTmrQueryTime( &qwTime );
*p_buf++ = ( uint8_t )( qwTime.ulLo * rand());
len--;
}
}
