























#include "libavutil/common.h"
#include "libavutil/mem.h"

#include "timefilter.h"

struct TimeFilter {


double cycle_time;
double feedback2_factor;
double feedback3_factor;
double clock_period;
int count;
};


static double qexpneg(double x)
{
return 1 - 1 / (1 + x * (1 + x / 2 * (1 + x / 3)));
}

TimeFilter *ff_timefilter_new(double time_base,
double period,
double bandwidth)
{
TimeFilter *self = av_mallocz(sizeof(TimeFilter));
double o = 2 * M_PI * bandwidth * period * time_base;

if (!self)
return NULL;

self->clock_period = time_base;
self->feedback2_factor = qexpneg(M_SQRT2 * o);
self->feedback3_factor = qexpneg(o * o) / period;
return self;
}

void ff_timefilter_destroy(TimeFilter *self)
{
av_freep(&self);
}

void ff_timefilter_reset(TimeFilter *self)
{
self->count = 0;
}

double ff_timefilter_update(TimeFilter *self, double system_time, double period)
{
self->count++;
if (self->count == 1) {
self->cycle_time = system_time;
} else {
double loop_error;
self->cycle_time += self->clock_period * period;
loop_error = system_time - self->cycle_time;

self->cycle_time += FFMAX(self->feedback2_factor, 1.0 / self->count) * loop_error;
self->clock_period += self->feedback3_factor * loop_error;
}
return self->cycle_time;
}

double ff_timefilter_eval(TimeFilter *self, double delta)
{
return self->cycle_time + self->clock_period * delta;
}
