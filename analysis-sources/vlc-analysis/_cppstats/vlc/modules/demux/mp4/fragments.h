#include <vlc_common.h>
#include "libmp4.h"
typedef struct mp4_fragments_index_t
{
uint64_t *pi_pos;
stime_t *p_times; 
unsigned i_entries;
stime_t i_last_time; 
unsigned i_tracks;
} mp4_fragments_index_t;
void MP4_Fragments_Index_Delete( mp4_fragments_index_t *p_index );
mp4_fragments_index_t * MP4_Fragments_Index_New( unsigned i_tracks, unsigned i_num );
stime_t MP4_Fragment_Index_GetTrackStartTime( mp4_fragments_index_t *p_index,
unsigned i_track_index, uint64_t i_moof_pos );
stime_t MP4_Fragment_Index_GetTrackDuration( mp4_fragments_index_t *p_index, unsigned i_track_index );
bool MP4_Fragments_Index_Lookup( mp4_fragments_index_t *p_index,
stime_t *pi_time, uint64_t *pi_pos, unsigned i_track_index );
#if defined(MP4_VERBOSE)
void MP4_Fragments_Index_Dump( vlc_object_t *p_obj, const mp4_fragments_index_t *p_index,
uint32_t i_movie_timescale );
#endif
