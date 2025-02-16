





























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include <assert.h>

#include <time.h>
#include <stdlib.h>

char *secstotimestr( char *psz_buffer, int32_t i_seconds )
{
if( unlikely(i_seconds < 0) )
{
secstotimestr( psz_buffer + 1, -i_seconds );
*psz_buffer = '-';
return psz_buffer;
}

div_t d;

d = div( i_seconds, 60 );
i_seconds = d.rem;
d = div( d.quot, 60 );

if( d.quot )
snprintf( psz_buffer, MSTRTIME_MAX_SIZE, "%u:%02u:%02u",
d.quot, d.rem, i_seconds );
else
snprintf( psz_buffer, MSTRTIME_MAX_SIZE, "%02u:%02u",
d.rem, i_seconds );
return psz_buffer;
}





void date_Init( date_t *p_date, uint32_t i_divider_n, uint32_t i_divider_d )
{
p_date->date = VLC_TICK_INVALID;
p_date->i_divider_num = i_divider_n;
p_date->i_divider_den = i_divider_d;
p_date->i_remainder = 0;
}

void date_Change( date_t *p_date, uint32_t i_divider_n, uint32_t i_divider_d )
{
assert( p_date->i_divider_num != 0 );

p_date->i_remainder = p_date->i_remainder * i_divider_n / p_date->i_divider_num;
p_date->i_divider_num = i_divider_n;
p_date->i_divider_den = i_divider_d;
}

vlc_tick_t date_Increment( date_t *p_date, uint32_t i_nb_samples )
{
if(unlikely(p_date->date == VLC_TICK_INVALID))
return VLC_TICK_INVALID;
assert( p_date->i_divider_num != 0 );
vlc_tick_t i_dividend = i_nb_samples * CLOCK_FREQ * p_date->i_divider_den;
lldiv_t d = lldiv( i_dividend, p_date->i_divider_num );

p_date->date += d.quot;
p_date->i_remainder += (int)d.rem;

if( p_date->i_remainder >= p_date->i_divider_num )
{

assert( p_date->i_remainder < 2*p_date->i_divider_num);
p_date->date += 1;
p_date->i_remainder -= p_date->i_divider_num;
}

return p_date->date;
}

vlc_tick_t date_Decrement( date_t *p_date, uint32_t i_nb_samples )
{
if(unlikely(p_date->date == VLC_TICK_INVALID))
return VLC_TICK_INVALID;
vlc_tick_t i_dividend = (vlc_tick_t)i_nb_samples * CLOCK_FREQ * p_date->i_divider_den;
p_date->date -= i_dividend / p_date->i_divider_num;
unsigned i_rem_adjust = i_dividend % p_date->i_divider_num;

if( p_date->i_remainder < i_rem_adjust )
{

assert( p_date->i_remainder < p_date->i_divider_num);
p_date->date -= 1;
p_date->i_remainder += p_date->i_divider_num;
}

p_date->i_remainder -= i_rem_adjust;

return p_date->date;
}

uint64_t NTPtime64(void)
{
struct timespec ts;

timespec_get(&ts, TIME_UTC);


uint64_t t = (uint64_t)(ts.tv_nsec) << 32;
t /= 1000000000;




t |= ((UINT64_C(70) * 365 + 17) * 24 * 60 * 60 + ts.tv_sec) << 32;
return t;
}

struct timespec timespec_from_vlc_tick (vlc_tick_t date)
{
lldiv_t d = lldiv (date, CLOCK_FREQ);
struct timespec ts = { d.quot, NS_FROM_VLC_TICK( d.rem ) };

return ts;
}
