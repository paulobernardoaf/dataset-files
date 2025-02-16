#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_filter.h>
#include <vlc_subpicture.h>
#include <vlc_es.h>
#include <stdlib.h>
#define SUBSDELAY_HELP N_("Change subtitle delay")
#define MODE_TEXT N_( "Delay calculation mode" )
#define MODE_LONGTEXT N_( "Absolute delay - add absolute delay to each subtitle. " "Relative to source delay - multiply subtitle delay. " "Relative to source content - determine subtitle delay from its content (text)." )
#define FACTOR_TEXT N_( "Calculation factor" )
#define FACTOR_LONGTEXT N_( "Calculation factor. " "In Absolute delay mode the factor represents seconds.")
#define OVERLAP_TEXT N_( "Maximum overlapping subtitles" )
#define OVERLAP_LONGTEXT N_( "Maximum number of subtitles allowed at the same time." )
#define MIN_ALPHA_TEXT N_( "Minimum alpha value" )
#define MIN_ALPHA_LONGTEXT N_( "Alpha value of the earliest subtitle, where 0 is fully transparent and 255 is fully opaque." )
#define MIN_STOPS_INTERVAL_TEXT N_( "Interval between two disappearances" )
#define MIN_STOPS_INTERVAL_LONGTEXT N_( "Minimum time (in milliseconds) that subtitle should stay after its predecessor has disappeared " "(subtitle delay will be extended to meet this requirement)." )
#define MIN_STOP_START_INTERVAL_TEXT N_( "Interval between disappearance and appearance" )
#define MIN_STOP_START_INTERVAL_LONGTEXT N_( "Minimum time (in milliseconds) between subtitle disappearance and newer subtitle appearance " "(earlier subtitle delay will be extended to fill the gap)." )
#define MIN_START_STOP_INTERVAL_TEXT N_( "Interval between appearance and disappearance" )
#define MIN_START_STOP_INTERVAL_LONGTEXT N_( "Minimum time (in milliseconds) that subtitle should stay after newer subtitle has appeared " "(earlier subtitle delay will be shortened to avoid the overlap)." )
static const int pi_mode_values[] = { 0, 1, 2 };
static const char * const ppsz_mode_descriptions[] = { N_( "Absolute delay" ), N_( "Relative to source delay" ), N_(
"Relative to source content" ) };
#define CFG_PREFIX "subsdelay-"
#define CFG_MODE CFG_PREFIX "mode"
#define CFG_FACTOR CFG_PREFIX "factor"
#define CFG_OVERLAP CFG_PREFIX "overlap"
#define CFG_MIN_ALPHA CFG_PREFIX "min-alpha"
#define CFG_MIN_STOPS_INTERVAL CFG_PREFIX "min-stops"
#define CFG_MIN_STOP_START_INTERVAL CFG_PREFIX "min-stop-start"
#define CFG_MIN_START_STOP_INTERVAL CFG_PREFIX "min-start-stop"
#define SUBSDELAY_MAX_ENTRIES 16
#define SUBSDELAY_MODE_ABSOLUTE 0
#define SUBSDELAY_MODE_RELATIVE_SOURCE_DELAY 1
#define SUBSDELAY_MODE_RELATIVE_SOURCE_CONTENT 2
typedef struct subsdelay_heap_entry_t subsdelay_heap_entry_t;
struct subsdelay_heap_entry_t
{
subpicture_t *p_subpic; 
subpicture_t *p_source; 
filter_t *p_filter; 
subsdelay_heap_entry_t *p_next; 
bool b_update_stop; 
bool b_update_ephemer; 
bool b_update_position; 
bool b_check_empty; 
vlc_tick_t i_new_stop; 
int i_last_region_x;
int i_last_region_y;
int i_last_region_align;
bool b_last_region_saved;
};
typedef struct
{
vlc_mutex_t lock; 
subsdelay_heap_entry_t *p_list[SUBSDELAY_MAX_ENTRIES]; 
subsdelay_heap_entry_t *p_head; 
int i_count; 
} subsdelay_heap_t;
typedef struct
{
int i_mode; 
float f_factor; 
int i_overlap; 
int i_min_alpha; 
vlc_tick_t i_min_stops_interval;
vlc_tick_t i_min_stop_start_interval;
vlc_tick_t i_min_start_stop_interval;
subsdelay_heap_t heap; 
} filter_sys_t;
static int SubsdelayCreate( vlc_object_t * );
static void SubsdelayDestroy( vlc_object_t * );
static subpicture_t * SubsdelayFilter( filter_t *p_filter, subpicture_t* p_subpic );
static int SubsdelayCallback( vlc_object_t *p_this, char const *psz_var, vlc_value_t oldval, vlc_value_t newval,
void *p_data );
static void SubsdelayEnforceDelayRules( filter_t *p_filter );
static vlc_tick_t SubsdelayEstimateDelay( filter_t *p_filter, subsdelay_heap_entry_t *p_entry );
static void SubsdelayRecalculateDelays( filter_t *p_filter );
static int SubsdelayCalculateAlpha( filter_t *p_filter, int i_overlapping, int i_source_alpha );
static int SubsdelayGetTextRank( char *psz_text );
static bool SubsdelayIsTextEmpty( const text_segment_t* p_segment );
static int SubpicValidateWrapper( subpicture_t *p_subpic, bool has_src_changed, const video_format_t *p_fmt_src,
bool has_dst_changed, const video_format_t *p_fmt_dst, vlc_tick_t i_ts );
static void SubpicUpdateWrapper( subpicture_t *p_subpic, const video_format_t *p_fmt_src,
const video_format_t *p_fmt_dst, vlc_tick_t i_ts );
static void SubpicDestroyWrapper( subpicture_t *p_subpic );
static void SubpicLocalUpdate( subpicture_t* p_subpic, vlc_tick_t i_ts );
static bool SubpicIsEmpty( subpicture_t* p_subpic );
static subpicture_t *SubpicClone( subpicture_t *p_source, subpicture_updater_t *updater );
static void SubpicDestroyClone( subpicture_t *p_subpic );
static void SubsdelayHeapInit( subsdelay_heap_t *p_heap );
static void SubsdelayHeapDestroy( subsdelay_heap_t *p_heap );
static subsdelay_heap_entry_t *SubsdelayHeapPush( subsdelay_heap_t *p_heap, subpicture_t *p_subpic, filter_t *p_filter );
static void SubsdelayHeapRemove( subsdelay_heap_t *p_heap, subsdelay_heap_entry_t *p_entry );
static void SubsdelayRebuildList( subsdelay_heap_t *p_heap );
static void SubsdelayHeapLock( subsdelay_heap_t *p_heap );
static void SubsdelayHeapUnlock( subsdelay_heap_t *p_heap );
static subsdelay_heap_entry_t * SubsdelayEntryCreate( subpicture_t *p_subpic, filter_t *p_filter );
static void SubsdelayEntryDestroy( subsdelay_heap_entry_t *p_entry );
static int SubsdelayHeapCountOverlap( subsdelay_heap_t *p_heap, subsdelay_heap_entry_t *p_entry, vlc_tick_t i_date );
static void SubsdelayEntryNewStopValueUpdated( subsdelay_heap_entry_t *p_entry );
vlc_module_begin()
set_shortname( N_("Subsdelay") )
set_description( N_("Subtitle delay") )
set_help( SUBSDELAY_HELP )
set_capability( "sub filter", 0 )
set_callbacks( SubsdelayCreate, SubsdelayDestroy )
set_category( CAT_VIDEO )
set_subcategory( SUBCAT_VIDEO_SUBPIC )
add_integer( CFG_MODE, 1, MODE_TEXT, MODE_LONGTEXT, false )
change_integer_list( pi_mode_values, ppsz_mode_descriptions )
add_float_with_range( CFG_FACTOR, 2, 0, 20, FACTOR_TEXT, FACTOR_LONGTEXT, false )
add_integer_with_range( CFG_OVERLAP, 3, 1, 4, OVERLAP_TEXT, OVERLAP_LONGTEXT, false )
add_integer_with_range( CFG_MIN_ALPHA, 70, 0, 255, MIN_ALPHA_TEXT, MIN_ALPHA_LONGTEXT, false )
set_section( N_("Overlap fix"), NULL )
add_integer( CFG_MIN_STOPS_INTERVAL, 1000, MIN_STOPS_INTERVAL_TEXT, MIN_STOPS_INTERVAL_LONGTEXT, false )
add_integer( CFG_MIN_START_STOP_INTERVAL, 1000, MIN_START_STOP_INTERVAL_TEXT,
MIN_START_STOP_INTERVAL_LONGTEXT, false )
add_integer( CFG_MIN_STOP_START_INTERVAL, 1000, MIN_STOP_START_INTERVAL_TEXT,
MIN_STOP_START_INTERVAL_LONGTEXT, false )
vlc_module_end ()
static const char * const ppsz_filter_options[] = { "mode", "factor", "overlap", NULL };
static int SubsdelayCreate( vlc_object_t *p_this )
{
filter_t *p_filter = (filter_t *) p_this;
filter_sys_t *p_sys;
p_sys = (filter_sys_t*) malloc( sizeof(filter_sys_t) );
if( !p_sys )
{
return VLC_ENOMEM;
}
p_sys->i_mode = var_CreateGetIntegerCommand( p_filter, CFG_MODE );
var_AddCallback( p_filter, CFG_MODE, SubsdelayCallback, p_sys );
p_sys->f_factor = var_CreateGetFloatCommand( p_filter, CFG_FACTOR );
var_AddCallback( p_filter, CFG_FACTOR, SubsdelayCallback, p_sys );
p_sys->i_overlap = var_CreateGetIntegerCommand( p_filter, CFG_OVERLAP );
var_AddCallback( p_filter, CFG_OVERLAP, SubsdelayCallback, p_sys );
p_sys->i_min_alpha = var_CreateGetIntegerCommand( p_filter, CFG_MIN_ALPHA );
var_AddCallback( p_filter, CFG_MIN_ALPHA, SubsdelayCallback, p_sys );
p_sys->i_min_stops_interval
= VLC_TICK_FROM_MS( var_CreateGetIntegerCommand( p_filter, CFG_MIN_STOPS_INTERVAL ) );
var_AddCallback( p_filter, CFG_MIN_STOPS_INTERVAL, SubsdelayCallback, p_sys );
p_sys->i_min_stop_start_interval
= VLC_TICK_FROM_MS( var_CreateGetIntegerCommand( p_filter, CFG_MIN_STOP_START_INTERVAL ) );
var_AddCallback( p_filter, CFG_MIN_STOP_START_INTERVAL, SubsdelayCallback, p_sys );
p_sys->i_min_start_stop_interval
= VLC_TICK_FROM_MS( var_CreateGetIntegerCommand( p_filter, CFG_MIN_START_STOP_INTERVAL ) );
var_AddCallback( p_filter, CFG_MIN_START_STOP_INTERVAL, SubsdelayCallback, p_sys );
p_filter->p_sys = p_sys;
p_filter->pf_sub_filter = SubsdelayFilter;
config_ChainParse( p_filter, CFG_PREFIX, ppsz_filter_options, p_filter->p_cfg );
SubsdelayHeapInit( &p_sys->heap );
return VLC_SUCCESS;
}
static void SubsdelayDestroy( vlc_object_t *p_this )
{
filter_t *p_filter = (filter_t *) p_this;
filter_sys_t *p_sys = p_filter->p_sys;
SubsdelayHeapDestroy( &p_sys->heap );
var_DelCallback( p_filter, CFG_MODE, SubsdelayCallback, p_sys );
var_Destroy( p_filter, CFG_MODE );
var_DelCallback( p_filter, CFG_FACTOR, SubsdelayCallback, p_sys );
var_Destroy( p_filter, CFG_FACTOR );
var_DelCallback( p_filter, CFG_OVERLAP, SubsdelayCallback, p_sys );
var_Destroy( p_filter, CFG_OVERLAP );
var_DelCallback( p_filter, CFG_MIN_ALPHA, SubsdelayCallback, p_sys );
var_Destroy( p_filter, CFG_MIN_ALPHA );
var_DelCallback( p_filter, CFG_MIN_STOPS_INTERVAL, SubsdelayCallback, p_sys );
var_Destroy( p_filter, CFG_MIN_STOPS_INTERVAL );
var_DelCallback( p_filter, CFG_MIN_STOP_START_INTERVAL, SubsdelayCallback, p_sys );
var_Destroy( p_filter, CFG_MIN_STOP_START_INTERVAL );
var_DelCallback( p_filter, CFG_MIN_START_STOP_INTERVAL, SubsdelayCallback, p_sys );
var_Destroy( p_filter, CFG_MIN_START_STOP_INTERVAL );
free( p_sys );
}
static subpicture_t * SubsdelayFilter( filter_t *p_filter, subpicture_t* p_subpic )
{
subsdelay_heap_t *p_heap;
subsdelay_heap_entry_t *p_entry;
if( !p_subpic->b_subtitle )
{
return p_subpic;
}
if( SubpicIsEmpty( p_subpic ) )
{
subpicture_Delete( p_subpic );
return NULL;
}
filter_sys_t *p_sys = p_filter->p_sys;
p_heap = &p_sys->heap;
SubsdelayHeapLock( p_heap );
p_entry = SubsdelayHeapPush( p_heap, p_subpic, p_filter );
if( !p_entry )
{
SubsdelayHeapUnlock( p_heap );
msg_Err(p_filter, "Can't add subpicture to the heap");
return p_subpic;
}
p_subpic = p_entry->p_subpic; 
if( p_subpic->b_ephemer )
{
p_subpic->i_stop = p_subpic->i_start + VLC_TICK_FROM_SEC(20); 
p_subpic->b_ephemer = false;
}
SubsdelayEnforceDelayRules( p_filter );
SubsdelayHeapUnlock( p_heap );
return p_subpic;
}
static int SubsdelayCallback( vlc_object_t *p_this, char const *psz_var, vlc_value_t oldval, vlc_value_t newval,
void *p_data )
{
filter_sys_t *p_sys = (filter_sys_t *) p_data;
VLC_UNUSED( oldval );
SubsdelayHeapLock( &p_sys->heap );
if( !strcmp( psz_var, CFG_MODE ) )
{
p_sys->i_mode = newval.i_int;
}
else if( !strcmp( psz_var, CFG_FACTOR ) )
{
p_sys->f_factor = newval.f_float;
}
else if( !strcmp( psz_var, CFG_OVERLAP ) )
{
p_sys->i_overlap = newval.i_int;
}
else if( !strcmp( psz_var, CFG_MIN_ALPHA ) )
{
p_sys->i_min_alpha = newval.i_int;
}
else if( !strcmp( psz_var, CFG_MIN_STOPS_INTERVAL ) )
{
p_sys->i_min_stops_interval = VLC_TICK_FROM_MS( newval.i_int );
}
else if( !strcmp( psz_var, CFG_MIN_STOP_START_INTERVAL ) )
{
p_sys->i_min_stop_start_interval = VLC_TICK_FROM_MS( newval.i_int );
}
else if( !strcmp( psz_var, CFG_MIN_START_STOP_INTERVAL ) )
{
p_sys->i_min_start_stop_interval = VLC_TICK_FROM_MS( newval.i_int );
}
else
{
vlc_assert_unreachable();
}
SubsdelayRecalculateDelays( (filter_t *) p_this );
SubsdelayHeapUnlock( &p_sys->heap );
return VLC_SUCCESS;
}
static void SubsdelayHeapInit( subsdelay_heap_t *p_heap )
{
p_heap->i_count = 0;
p_heap->p_head = NULL;
vlc_mutex_init( &p_heap->lock );
}
static void SubsdelayHeapDestroy( subsdelay_heap_t *p_heap )
{
SubsdelayHeapLock( p_heap );
for( subsdelay_heap_entry_t *p_entry = p_heap->p_head;
p_entry != NULL; p_entry = p_entry->p_next )
{
p_entry->p_subpic->i_stop = p_entry->p_source->i_stop;
p_entry->p_filter = NULL;
}
SubsdelayHeapUnlock( p_heap );
}
static subsdelay_heap_entry_t *SubsdelayHeapPush( subsdelay_heap_t *p_heap, subpicture_t *p_subpic, filter_t *p_filter )
{
subsdelay_heap_entry_t *p_last, *p_new_entry;
if( p_heap->i_count >= SUBSDELAY_MAX_ENTRIES )
{
return NULL; 
}
p_new_entry = SubsdelayEntryCreate( p_subpic, p_filter );
if( !p_new_entry )
{
return NULL;
}
p_last = NULL;
for( subsdelay_heap_entry_t *p_entry = p_heap->p_head; p_entry != NULL;
p_entry = p_entry->p_next )
{
if( p_entry->p_source->i_start > p_subpic->i_start )
{
break;
}
p_last = p_entry;
}
if( p_last )
{
p_new_entry->p_next = p_last->p_next;
p_last->p_next = p_new_entry;
if( p_last->b_update_ephemer )
{
p_last->p_source->i_stop = p_new_entry->p_source->i_start;
p_last->b_update_ephemer = false;
}
}
else
{
p_new_entry->p_next = p_heap->p_head;
p_heap->p_head = p_new_entry;
}
SubsdelayRebuildList( p_heap );
return p_new_entry;
}
static void SubsdelayHeapRemove( subsdelay_heap_t *p_heap, subsdelay_heap_entry_t *p_entry )
{
subsdelay_heap_entry_t *p_prev;
p_prev = NULL;
for( subsdelay_heap_entry_t *p_curr = p_heap->p_head; p_curr != NULL;
p_curr = p_curr->p_next )
{
if( p_curr == p_entry )
{
break;
}
p_prev = p_curr;
}
if( p_prev )
{
p_prev->p_next = p_entry->p_next;
}
else
{
p_heap->p_head = p_entry->p_next;
}
p_entry->p_filter = NULL;
SubsdelayRebuildList( p_heap );
}
static void SubsdelayRebuildList( subsdelay_heap_t *p_heap )
{
int i_index;
i_index = 0;
for( subsdelay_heap_entry_t *p_curr = p_heap->p_head; p_curr != NULL;
p_curr = p_curr->p_next )
{
p_heap->p_list[i_index] = p_curr;
i_index++;
}
p_heap->i_count = i_index;
}
static void SubsdelayHeapLock( subsdelay_heap_t *p_heap )
{
vlc_mutex_lock( &p_heap->lock );
}
static void SubsdelayHeapUnlock( subsdelay_heap_t *p_heap )
{
vlc_mutex_unlock( &p_heap->lock );
}
static subsdelay_heap_entry_t * SubsdelayEntryCreate( subpicture_t *p_source, filter_t *p_filter )
{
subsdelay_heap_entry_t *p_entry;
subpicture_t *p_new_subpic;
subpicture_updater_t updater;
p_entry = (subsdelay_heap_entry_t *) malloc( sizeof( subsdelay_heap_entry_t ) );
if( !p_entry )
{
return NULL;
}
updater.p_sys = p_entry;
updater.pf_validate = SubpicValidateWrapper;
updater.pf_update = SubpicUpdateWrapper;
updater.pf_destroy = SubpicDestroyWrapper;
p_new_subpic = SubpicClone( p_source, &updater );
if( !p_new_subpic )
{
free( p_entry );
return NULL;
}
p_entry->p_subpic = p_new_subpic;
p_entry->p_source = p_source;
p_entry->p_filter = p_filter;
p_entry->p_next = NULL;
p_entry->b_update_stop = true;
p_entry->b_update_ephemer = p_source->b_ephemer;
p_entry->b_update_position = true;
p_entry->b_check_empty = true;
p_entry->i_new_stop = p_source->i_stop;
p_entry->b_last_region_saved = false;
p_entry->i_last_region_x = 0;
p_entry->i_last_region_y = 0;
p_entry->i_last_region_align = 0;
return p_entry;
}
static void SubsdelayEntryDestroy( subsdelay_heap_entry_t *p_entry )
{
SubpicDestroyClone( p_entry->p_source );
free( p_entry );
}
static int SubsdelayHeapCountOverlap( subsdelay_heap_t *p_heap, subsdelay_heap_entry_t *p_entry, vlc_tick_t i_date )
{
int i_overlaps;
VLC_UNUSED( p_heap );
i_overlaps = 0;
for( subsdelay_heap_entry_t *p_curr = p_entry->p_next; p_curr != NULL;
p_curr = p_curr->p_next )
{
if( p_curr->p_source->i_start > i_date )
{
break;
}
if( !p_curr->b_check_empty ) 
{
i_overlaps++;
}
}
return i_overlaps;
}
static void SubsdelayEntryNewStopValueUpdated( subsdelay_heap_entry_t *p_entry )
{
if( !p_entry->b_update_stop )
{
p_entry->p_subpic->i_stop = p_entry->i_new_stop - VLC_TICK_FROM_MS(100); 
}
}
static void SubsdelayEnforceDelayRules( filter_t *p_filter )
{
subsdelay_heap_entry_t ** p_list;
int i_count, i_overlap;
vlc_tick_t i_offset;
vlc_tick_t i_min_stops_interval;
vlc_tick_t i_min_stop_start_interval;
vlc_tick_t i_min_start_stop_interval;
filter_sys_t *p_sys = p_filter->p_sys;
p_list = p_sys->heap.p_list;
i_count = p_sys->heap.i_count;
i_overlap = p_sys->i_overlap;
i_min_stops_interval = p_sys->i_min_stops_interval;
i_min_stop_start_interval = p_sys->i_min_stop_start_interval;
i_min_start_stop_interval = p_sys->i_min_start_stop_interval;
for( int i = 0; i < i_count - 1; i++ )
{
p_list[i + 1]->i_new_stop = __MAX( p_list[i + 1]->i_new_stop,
p_list[i]->i_new_stop + i_min_stops_interval );
}
for( int i = 0; i < i_count; i++ )
{
for( int j = i + 1; j < __MIN( i_count, i + 1 + i_overlap ); j++ )
{
i_offset = p_list[j]->p_source->i_start - p_list[i]->i_new_stop;
if( i_offset <= 0 )
{
continue;
}
if( i_offset < i_min_stop_start_interval )
{
p_list[i]->i_new_stop = p_list[j]->p_source->i_start;
}
break;
}
}
for( int i = 0; i < i_count; i++ )
{
for( int j = i + 1; j < __MIN( i_count, i + 1 + i_overlap ); j++ )
{
i_offset = p_list[i]->i_new_stop - p_list[j]->p_source->i_start;
if( i_offset <= 0 )
{
break;
}
if( i_offset < i_min_start_stop_interval )
{
p_list[i]->i_new_stop = p_list[j]->p_source->i_start;
break;
}
}
}
for( int i = 0; i < i_count - i_overlap; i++ )
{
p_list[i]->i_new_stop = __MIN(p_list[i]->i_new_stop, p_list[i + i_overlap]->p_source->i_start);
}
for( int i = 0; i < i_count; i++ )
{
SubsdelayEntryNewStopValueUpdated( p_list[i] );
}
}
static void SubsdelayRecalculateDelays( filter_t *p_filter )
{
filter_sys_t *p_sys = p_filter->p_sys;
for( subsdelay_heap_entry_t *p_curr = p_sys->heap.p_head;
p_curr != NULL; p_curr = p_curr->p_next )
{
if( !p_curr->b_update_ephemer )
{
p_curr->i_new_stop = p_curr->p_source->i_start + SubsdelayEstimateDelay( p_filter, p_curr );
p_curr->b_update_stop = false;
}
}
SubsdelayEnforceDelayRules( p_filter );
}
static int SubpicValidateWrapper( subpicture_t *p_subpic, bool has_src_changed, const video_format_t *p_fmt_src,
bool has_dst_changed, const video_format_t *p_fmt_dst, vlc_tick_t i_ts )
{
subsdelay_heap_entry_t *p_entry;
vlc_tick_t i_new_ts;
int i_result = VLC_SUCCESS;
p_entry = p_subpic->updater.p_sys;
if( !p_entry )
{
return VLC_SUCCESS;
}
if( p_entry->p_source->updater.pf_validate )
{
i_new_ts = p_entry->p_source->i_start +
( (double)( p_entry->p_source->i_stop - p_entry->p_source->i_start ) * ( i_ts - p_entry->p_source->i_start ) ) /
( p_entry->i_new_stop - p_entry->p_source->i_start );
i_result = p_entry->p_source->updater.pf_validate( p_entry->p_source, has_src_changed, p_fmt_src,
has_dst_changed, p_fmt_dst, i_new_ts );
}
p_entry->b_last_region_saved = false;
if( p_subpic->p_region )
{
p_entry->i_last_region_x = p_subpic->p_region->i_x;
p_entry->i_last_region_y = p_subpic->p_region->i_y;
p_entry->i_last_region_align = p_subpic->p_region->i_align;
p_entry->b_last_region_saved = true;
}
if( !i_result )
{
SubpicLocalUpdate( p_subpic, i_ts );
}
return i_result;
}
static void SubpicUpdateWrapper( subpicture_t *p_subpic, const video_format_t *p_fmt_src,
const video_format_t *p_fmt_dst, vlc_tick_t i_ts )
{
subsdelay_heap_entry_t *p_entry;
vlc_tick_t i_new_ts;
p_entry = p_subpic->updater.p_sys;
if( !p_entry )
{
return;
}
if( p_entry->p_source->updater.pf_update )
{
i_new_ts = p_entry->p_source->i_start +
( (double)( p_entry->p_source->i_stop - p_entry->p_source->i_start ) * ( i_ts - p_entry->p_source->i_start ) ) /
( p_entry->i_new_stop - p_entry->p_source->i_start );
p_entry->p_source->p_region = p_entry->p_subpic->p_region;
p_entry->p_source->updater.pf_update( p_entry->p_source, p_fmt_src, p_fmt_dst, i_new_ts );
p_entry->p_subpic->p_region = p_entry->p_source->p_region;
}
SubpicLocalUpdate( p_subpic, i_ts );
}
static void SubpicDestroyWrapper( subpicture_t *p_subpic )
{
subsdelay_heap_entry_t *p_entry;
subsdelay_heap_t *p_heap;
p_entry = p_subpic->updater.p_sys;
if( !p_entry )
{
return;
}
if( p_entry->p_filter )
{
filter_sys_t *p_sys = p_entry->p_filter->p_sys;
p_heap = &p_sys->heap;
SubsdelayHeapLock( p_heap );
SubsdelayHeapRemove( p_heap, p_entry );
SubsdelayHeapUnlock( p_heap );
}
SubsdelayEntryDestroy( p_entry );
}
static void SubpicLocalUpdate( subpicture_t* p_subpic, vlc_tick_t i_ts )
{
subsdelay_heap_entry_t *p_entry;
subsdelay_heap_t *p_heap;
filter_t *p_filter;
int i_overlapping;
p_entry = p_subpic->updater.p_sys;
if( !p_entry || !p_entry->p_filter )
{
return;
}
p_filter = p_entry->p_filter;
filter_sys_t *p_sys = p_filter->p_sys;
p_heap = &p_sys->heap;
SubsdelayHeapLock( p_heap );
if( p_entry->b_check_empty && p_subpic->p_region )
{
if( SubsdelayIsTextEmpty( p_subpic->p_region->p_text ) )
{
p_subpic->b_ephemer = false;
p_subpic->i_stop = p_subpic->i_start;
SubsdelayHeapRemove( p_heap, p_entry );
SubsdelayHeapUnlock( p_heap );
return;
}
p_entry->b_check_empty = false;
}
if( p_entry->b_update_stop && !p_entry->b_update_ephemer )
{
p_entry->i_new_stop = p_entry->p_source->i_start + SubsdelayEstimateDelay( p_filter, p_entry );
p_entry->b_update_stop = false;
SubsdelayEnforceDelayRules( p_filter );
}
i_overlapping = SubsdelayHeapCountOverlap( p_heap, p_entry, i_ts );
p_subpic->i_alpha = SubsdelayCalculateAlpha( p_filter, i_overlapping, p_entry->p_source->i_alpha );
if( p_entry->b_update_position )
{
p_subpic->b_absolute = false;
if( p_subpic->p_region )
{
p_subpic->p_region->i_x = 0;
p_subpic->p_region->i_y = 10;
p_subpic->p_region->i_align = ( p_subpic->p_region->i_align & ( ~SUBPICTURE_ALIGN_MASK ) )
| SUBPICTURE_ALIGN_BOTTOM;
}
p_entry->b_update_position = false;
}
else if( p_entry->b_last_region_saved )
{
p_subpic->b_absolute = true;
if( p_subpic->p_region )
{
p_subpic->p_region->i_x = p_entry->i_last_region_x;
p_subpic->p_region->i_y = p_entry->i_last_region_y;
p_subpic->p_region->i_align = p_entry->i_last_region_align;
}
}
SubsdelayHeapUnlock( p_heap );
}
static bool SubpicIsEmpty( subpicture_t* p_subpic )
{
return ( p_subpic->p_region && ( SubsdelayIsTextEmpty( p_subpic->p_region->p_text ) ) );
}
static subpicture_t *SubpicClone( subpicture_t *p_source, subpicture_updater_t *updater )
{
subpicture_t *p_subpic;
subpicture_updater_t subpic_updater;
subpicture_private_t *p_subpic_private;
p_subpic = subpicture_New( updater );
if( !p_subpic )
{
return NULL;
}
subpic_updater = p_subpic->updater;
p_subpic_private = p_subpic->p_private;
memcpy( p_subpic, p_source, sizeof( subpicture_t ) );
p_subpic->updater = subpic_updater;
p_subpic->p_private = p_subpic_private;
return p_subpic;
}
static void SubpicDestroyClone( subpicture_t *p_subpic )
{
p_subpic->p_region = NULL; 
subpicture_Delete( p_subpic );
}
static vlc_tick_t SubsdelayEstimateDelay( filter_t *p_filter, subsdelay_heap_entry_t *p_entry )
{
int i_mode;
int i_rank;
filter_sys_t *p_sys = p_filter->p_sys;
i_mode = p_sys->i_mode;
if( i_mode == SUBSDELAY_MODE_ABSOLUTE )
{
return ( p_entry->p_source->i_stop - p_entry->p_source->i_start + vlc_tick_from_sec( p_sys->f_factor ) );
}
if( i_mode == SUBSDELAY_MODE_RELATIVE_SOURCE_CONTENT )
{
if( p_entry->p_subpic && p_entry->p_subpic->p_region && ( p_entry->p_subpic->p_region->p_text ) )
{
i_rank = SubsdelayGetTextRank( p_entry->p_subpic->p_region->p_text->psz_text );
return vlc_tick_from_sec( p_sys->f_factor * i_rank );
}
i_mode = SUBSDELAY_MODE_RELATIVE_SOURCE_DELAY;
}
if( likely(i_mode == SUBSDELAY_MODE_RELATIVE_SOURCE_DELAY) )
{
return (vlc_tick_t)( p_sys->f_factor * ( p_entry->p_source->i_stop - p_entry->p_source->i_start ) );
}
return VLC_TICK_FROM_SEC(10); 
}
static int SubsdelayCalculateAlpha( filter_t *p_filter, int i_overlapping, int i_source_alpha )
{
int i_new_alpha;
int i_min_alpha;
filter_sys_t *p_sys = p_filter->p_sys;
i_min_alpha = p_sys->i_min_alpha;
if( i_overlapping > p_sys->i_overlap - 1)
{
i_overlapping = p_sys->i_overlap - 1;
}
switch ( p_sys->i_overlap )
{
case 1:
i_new_alpha = 255;
break;
case 2:
i_new_alpha = 255 - i_overlapping * ( 255 - i_min_alpha );
break;
case 3:
i_new_alpha = 255 - i_overlapping * ( 255 - i_min_alpha ) / 2;
break;
default:
i_new_alpha = 255 - i_overlapping * ( 255 - i_min_alpha ) / 3;
break;
}
return ( i_source_alpha * i_new_alpha ) / 255;
}
static int SubsdelayGetWordRank( int i_length )
{
static const int p_rank[20] = { 300, 300, 300, 330, 363, 399, 438, 481, 529, 581,
639, 702, 772, 849, 933, 1026, 1128, 1240, 1364, 1500 };
if( i_length < 1 )
{
return 0;
}
if( i_length > 20 )
{
i_length = 20;
}
return p_rank[i_length - 1];
}
static int SubsdelayGetTextRank( char *psz_text )
{
bool b_skip_esc;
bool b_skip_tag;
char c;
int i, i_word_length, i_rank;
i = 0;
i_word_length = 0;
i_rank = 0;
b_skip_esc = false;
b_skip_tag = false;
while ( psz_text[i] != '\0' )
{
c = psz_text[i];
i++;
if( c == '\\' && !b_skip_esc )
{
b_skip_esc = true;
continue;
}
if( psz_text[i] == '<' )
{
b_skip_tag = true;
continue;
}
if( !b_skip_esc && !b_skip_tag )
{
if( c == ' ' || c == ',' || c == '.' || c == '-' || c == '?' || c == '!' ) 
{
if( i_word_length > 0 )
{
i_rank += SubsdelayGetWordRank( i_word_length );
}
i_word_length = 0;
}
else
{
i_word_length++;
}
}
b_skip_esc = false;
if( c == '>' )
{
b_skip_tag = false;
}
}
if( i_word_length > 0 )
{
i_rank += SubsdelayGetWordRank( i_word_length );
}
return i_rank;
}
static bool SubsdelayIsTextEmpty( const text_segment_t *p_segment )
{
while ( p_segment )
{
if ( strlen( p_segment->psz_text ) > 0 )
{
size_t offset = strspn( p_segment->psz_text, " " );
if ( p_segment->psz_text[offset] )
return false;
}
p_segment = p_segment->p_next;
}
return true;
}
