#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <stdatomic.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <vlc_common.h>
struct vlc_timer
{
vlc_thread_t thread;
vlc_cond_t reschedule;
vlc_mutex_t lock;
void (*func) (void *);
void *data;
vlc_tick_t value, interval;
bool live;
atomic_uint overruns;
};
static void *vlc_timer_thread (void *data)
{
struct vlc_timer *timer = data;
vlc_mutex_lock (&timer->lock);
while (timer->live)
{
if (timer->value == 0)
{
assert(timer->interval == 0);
vlc_cond_wait (&timer->reschedule, &timer->lock);
continue;
}
if (timer->interval != 0)
{
vlc_tick_t now = vlc_tick_now();
if (now > timer->value)
{ 
unsigned misses = (now - timer->value) / timer->interval;
timer->value += misses * timer->interval;
assert(timer->value <= now);
atomic_fetch_add_explicit(&timer->overruns, misses,
memory_order_relaxed);
}
}
vlc_tick_t value = timer->value;
if (vlc_cond_timedwait(&timer->reschedule, &timer->lock, value) == 0)
continue;
if (likely(timer->value <= value))
{
timer->value += timer->interval; 
if (timer->interval == 0)
timer->value = 0; 
}
vlc_mutex_unlock (&timer->lock);
timer->func (timer->data);
vlc_mutex_lock (&timer->lock);
}
vlc_mutex_unlock (&timer->lock);
return NULL;
}
int vlc_timer_create (vlc_timer_t *id, void (*func) (void *), void *data)
{
struct vlc_timer *timer = malloc (sizeof (*timer));
if (unlikely(timer == NULL))
return ENOMEM;
vlc_mutex_init (&timer->lock);
vlc_cond_init (&timer->reschedule);
assert (func);
timer->func = func;
timer->data = data;
timer->value = 0;
timer->interval = 0;
timer->live = true;
atomic_init(&timer->overruns, 0);
if (vlc_clone (&timer->thread, vlc_timer_thread, timer,
VLC_THREAD_PRIORITY_INPUT))
{
free (timer);
return ENOMEM;
}
*id = timer;
return 0;
}
void vlc_timer_destroy (vlc_timer_t timer)
{
vlc_mutex_lock(&timer->lock);
timer->live = false;
vlc_cond_signal(&timer->reschedule);
vlc_mutex_unlock(&timer->lock);
vlc_join (timer->thread, NULL);
free (timer);
}
void vlc_timer_schedule (vlc_timer_t timer, bool absolute,
vlc_tick_t value, vlc_tick_t interval)
{
if (value == VLC_TIMER_DISARM)
interval = 0;
else
if (!absolute)
value += vlc_tick_now();
vlc_mutex_lock (&timer->lock);
timer->value = value;
timer->interval = interval;
vlc_cond_signal (&timer->reschedule);
vlc_mutex_unlock (&timer->lock);
}
unsigned vlc_timer_getoverrun (vlc_timer_t timer)
{
return atomic_exchange_explicit (&timer->overruns, 0,
memory_order_relaxed);
}
