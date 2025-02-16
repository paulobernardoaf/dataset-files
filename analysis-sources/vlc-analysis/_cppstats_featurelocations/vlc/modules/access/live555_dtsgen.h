


















#define DTSGEN_REORDER_MAX 4 
#define DTSGEN_HISTORY_COUNT (DTSGEN_REORDER_MAX + 2)


struct dtsgen_t
{
vlc_tick_t history[DTSGEN_HISTORY_COUNT];
vlc_tick_t ordereddts[DTSGEN_HISTORY_COUNT];
vlc_tick_t i_startingdts;
vlc_tick_t i_startingdiff;
unsigned reorderdepth;
unsigned count;
};

static int cmpvlctickp(const void *p1, const void *p2)
{
if(*((vlc_tick_t *)p1) >= *((vlc_tick_t *)p2))
return *((vlc_tick_t *)p1) > *((vlc_tick_t *)p2) ? 1 : 0;
else
return -1;
}

static void dtsgen_Init(struct dtsgen_t *d)
{
d->count = 0;
d->reorderdepth = 0;
}

static void dtsgen_Resync(struct dtsgen_t *d)
{
d->count = 0;
d->reorderdepth = 0;
}

#define dtsgen_Clean(d)













































static void dtsgen_AddNextPTS(struct dtsgen_t *d, vlc_tick_t i_pts)
{

if(d->count > 0 && d->count < DTSGEN_HISTORY_COUNT)
{
unsigned i;
if(d->count > (1 + d->reorderdepth))
i = d->count - (1 + d->reorderdepth);
else
i = 0;

for(; i < d->count; i++)
{
if(d->history[i] > i_pts)
{
if(d->reorderdepth < DTSGEN_REORDER_MAX)
d->reorderdepth++;
}
break;
}
}


if(d->count == DTSGEN_HISTORY_COUNT)
{
d->ordereddts[0] = i_pts; 
memmove(d->history, &d->history[1],
sizeof(d->history[0]) * (d->count - 1));
}
else
{
d->history[d->count] = i_pts;
d->ordereddts[d->count++] = i_pts;
}


qsort(&d->ordereddts, d->count, sizeof(d->ordereddts[0]), cmpvlctickp);
}

static vlc_tick_t dtsgen_GetDTS(struct dtsgen_t *d)
{
vlc_tick_t i_dts = VLC_TICK_INVALID;



if(d->count > DTSGEN_REORDER_MAX)
{
i_dts = d->ordereddts[d->count - d->reorderdepth - 1];
}


else if(d->count == 1)
{
d->i_startingdts =
i_dts = __MAX(d->history[0] - VLC_TICK_FROM_MS(150), VLC_TICK_0);
d->i_startingdiff = d->history[0] - i_dts;
}
else if(d->count > 1)
{
vlc_tick_t i_diff = d->ordereddts[d->count - 1] -
d->ordereddts[0];
i_diff = __MIN(d->i_startingdiff, i_diff);
d->i_startingdts += i_diff / DTSGEN_REORDER_MAX;
i_dts = d->i_startingdts;
}

return i_dts;
}

#if defined(DTSGEN_DEBUG)
static void dtsgen_Debug(vlc_object_t *p_demux, struct dtsgen_t *d,
vlc_tick_t dts, vlc_tick_t pts)
{
if(pts == VLC_TICK_INVALID)
return;
msg_Dbg(p_demux, "dtsgen %" PRId64 " / pts %" PRId64 " diff %" PRId64 ", "
"pkt count %u, reorder %u",
dts % (10 * CLOCK_FREQ),
pts % (10 * CLOCK_FREQ),
(pts - dts) % (10 * CLOCK_FREQ),
d->count, d->reorderdepth);
}
#else
#define dtsgen_Debug(a,b,c,d)
#endif
