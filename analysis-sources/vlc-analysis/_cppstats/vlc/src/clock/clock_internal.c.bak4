






















#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "clock_internal.h"




void AvgInit(average_t *avg, int range)
{
avg->range = range;
AvgReset(avg);
}

void AvgClean(average_t * avg)
{
VLC_UNUSED(avg);
}

void AvgReset(average_t *avg)
{
avg->value = 0.0f;
avg->count = 0;
}

void AvgUpdate(average_t *avg, double value)
{
const int new_value_weight = 1;
int average_weight;
int divider;
if (avg->count < avg->range)
{
average_weight = avg->count++;
divider = avg->count;
}
else
{
average_weight = avg->range - 1;
divider = avg->range;
}

const double tmp = average_weight * avg->value + new_value_weight * value;
avg->value = tmp / divider;
}

double AvgGet(average_t *avg)
{
return avg->value;
}

void AvgRescale(average_t *avg, int range)
{
const double tmp = avg->value * avg->range;

avg->range = range;
avg->value = tmp / avg->range;
}
