



















#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <assert.h>
#include <stdlib.h>
#include <windows.h>
#include <vlc_common.h>

struct vlc_timer
{
HANDLE handle;
void (*func) (void *);
void *data;
};

static void CALLBACK vlc_timer_do (void *val, BOOLEAN timeout)
{
struct vlc_timer *timer = val;

assert (timeout);
timer->func (timer->data);
}

int vlc_timer_create (vlc_timer_t *id, void (*func) (void *), void *data)
{
struct vlc_timer *timer = malloc (sizeof (*timer));

if (timer == NULL)
return ENOMEM;
timer->func = func;
timer->data = data;
timer->handle = INVALID_HANDLE_VALUE;
*id = timer;
return 0;
}

void vlc_timer_destroy (vlc_timer_t timer)
{
if (timer->handle != INVALID_HANDLE_VALUE)
DeleteTimerQueueTimer (NULL, timer->handle, INVALID_HANDLE_VALUE);
free (timer);
}

void vlc_timer_schedule (vlc_timer_t timer, bool absolute,
vlc_tick_t value, vlc_tick_t interval)
{
if (timer->handle != INVALID_HANDLE_VALUE)
{
DeleteTimerQueueTimer (NULL, timer->handle, INVALID_HANDLE_VALUE);
timer->handle = INVALID_HANDLE_VALUE;
}
if (value == VLC_TIMER_DISARM)
return; 

if (absolute)
{
value -= vlc_tick_now ();
if (value < 0)
value = 0;
}

DWORD val = MS_FROM_VLC_TICK(value);
DWORD interv = MS_FROM_VLC_TICK(interval);
if (val == 0 && value != 0)
val = 1; 
if (interv == 0 && interval != 0)
interv = 1; 

if (!CreateTimerQueueTimer(&timer->handle, NULL, vlc_timer_do, timer,
val, interv, WT_EXECUTEDEFAULT))
abort ();
}

unsigned vlc_timer_getoverrun (vlc_timer_t timer)
{
(void)timer;
return 0;
}
