#include <vlc_common.h>
typedef struct
{
double value; 
int count; 
int range; 
} average_t;
void AvgInit(average_t *, int range);
void AvgClean(average_t *);
void AvgReset(average_t *);
void AvgUpdate(average_t *, double value);
double AvgGet(average_t *);
void AvgRescale(average_t *, int range);
typedef struct
{
vlc_tick_t system;
vlc_tick_t stream;
} clock_point_t;
static inline clock_point_t clock_point_Create(vlc_tick_t system, vlc_tick_t stream)
{
return (clock_point_t) { .system = system, .stream = stream };
}
