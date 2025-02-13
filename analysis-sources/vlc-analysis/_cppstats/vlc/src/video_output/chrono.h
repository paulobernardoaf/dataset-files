#include <assert.h>
typedef struct {
int shift;
vlc_tick_t avg;
vlc_tick_t avg_initial;
int shift_var;
vlc_tick_t var;
vlc_tick_t start;
} vout_chrono_t;
static inline void vout_chrono_Init(vout_chrono_t *chrono, int shift, vlc_tick_t avg_initial)
{
chrono->shift = shift;
chrono->avg_initial =
chrono->avg = avg_initial;
chrono->shift_var = shift+1;
chrono->var = avg_initial / 2;
chrono->start = VLC_TICK_INVALID;
}
static inline void vout_chrono_Clean(vout_chrono_t *chrono)
{
VLC_UNUSED(chrono);
}
static inline void vout_chrono_Start(vout_chrono_t *chrono)
{
chrono->start = vlc_tick_now();
}
static inline vlc_tick_t vout_chrono_GetHigh(vout_chrono_t *chrono)
{
return chrono->avg + 2 * chrono->var;
}
static inline vlc_tick_t vout_chrono_GetLow(vout_chrono_t *chrono)
{
return __MAX(chrono->avg - 2 * chrono->var, 0);
}
static inline void vout_chrono_Stop(vout_chrono_t *chrono)
{
assert(chrono->start != VLC_TICK_INVALID);
const vlc_tick_t duration = vlc_tick_now() - chrono->start;
const vlc_tick_t var = llabs( duration - chrono->avg );
if( duration < vout_chrono_GetHigh( chrono ) )
chrono->avg = (((1 << chrono->shift) - 1) * chrono->avg + duration) >> chrono->shift;
chrono->var = (((1 << chrono->shift_var) - 1) * chrono->var + var) >> chrono->shift_var;
chrono->start = VLC_TICK_INVALID;
}
static inline void vout_chrono_Reset(vout_chrono_t *chrono)
{
vout_chrono_t ch = *chrono;
vout_chrono_Clean(chrono);
vout_chrono_Init(chrono, ch.shift, ch.avg_initial);
}
