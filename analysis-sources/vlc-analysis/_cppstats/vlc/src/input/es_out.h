#include <vlc_common.h>
enum es_out_mode_e
{
ES_OUT_MODE_NONE, 
ES_OUT_MODE_ALL, 
ES_OUT_MODE_AUTO, 
ES_OUT_MODE_PARTIAL,
ES_OUT_MODE_END 
};
enum es_out_query_private_e
{
ES_OUT_PRIV_SET_MODE, 
ES_OUT_PRIV_SET_ES, 
ES_OUT_PRIV_UNSET_ES, 
ES_OUT_PRIV_RESTART_ES, 
ES_OUT_PRIV_GET_WAKE_UP, 
ES_OUT_PRIV_SET_ES_LIST, 
ES_OUT_PRIV_SET_ES_CAT_IDS, 
ES_OUT_PRIV_STOP_ALL_ES, 
ES_OUT_PRIV_START_ALL_ES, 
ES_OUT_PRIV_GET_BUFFERING, 
ES_OUT_PRIV_SET_ES_DELAY, 
ES_OUT_PRIV_SET_DELAY, 
ES_OUT_PRIV_SET_RECORD_STATE, 
ES_OUT_PRIV_SET_PAUSE_STATE, 
ES_OUT_PRIV_SET_RATE, 
ES_OUT_PRIV_SET_FRAME_NEXT, 
ES_OUT_PRIV_SET_TIMES, 
ES_OUT_PRIV_SET_JITTER, 
ES_OUT_PRIV_GET_GROUP_FORCED, 
ES_OUT_PRIV_SET_EOS, 
ES_OUT_PRIV_SET_VBI_PAGE, 
ES_OUT_PRIV_SET_VBI_TRANSPARENCY 
};
static inline int es_out_vaPrivControl( es_out_t *out, int query, va_list args )
{
vlc_assert( out->cbs->priv_control );
return out->cbs->priv_control( out, query, args );
}
static inline int es_out_PrivControl( es_out_t *out, int query, ... )
{
va_list args;
va_start( args, query );
int result = es_out_vaPrivControl( out, query, args );
va_end( args );
return result;
}
static inline void es_out_SetMode( es_out_t *p_out, int i_mode )
{
int i_ret = es_out_PrivControl( p_out, ES_OUT_PRIV_SET_MODE, i_mode );
assert( !i_ret );
}
static inline int es_out_SetEs( es_out_t *p_out, vlc_es_id_t *id )
{
return es_out_PrivControl( p_out, ES_OUT_PRIV_SET_ES, id );
}
static inline int es_out_UnsetEs( es_out_t *p_out, vlc_es_id_t *id )
{
return es_out_PrivControl( p_out, ES_OUT_PRIV_UNSET_ES, id );
}
static inline int es_out_RestartEs( es_out_t *p_out, vlc_es_id_t *id )
{
return es_out_PrivControl( p_out, ES_OUT_PRIV_RESTART_ES, id );
}
static inline vlc_tick_t es_out_GetWakeup( es_out_t *p_out )
{
vlc_tick_t i_wu;
int i_ret = es_out_PrivControl( p_out, ES_OUT_PRIV_GET_WAKE_UP, &i_wu );
assert( !i_ret );
return i_wu;
}
static inline int es_out_SetEsList( es_out_t *p_out,
enum es_format_category_e cat,
vlc_es_id_t **ids )
{
return es_out_PrivControl( p_out, ES_OUT_PRIV_SET_ES_LIST, cat, ids );
}
static inline void es_out_SetEsCatIds( es_out_t *p_out,
enum es_format_category_e cat,
const char *str_ids )
{
int ret = es_out_PrivControl( p_out, ES_OUT_PRIV_SET_ES_CAT_IDS,
cat, str_ids );
assert( ret == VLC_SUCCESS );
}
static inline int es_out_StopAllEs( es_out_t *p_out, vlc_es_id_t ***context )
{
return es_out_PrivControl( p_out, ES_OUT_PRIV_STOP_ALL_ES, context );
}
static inline int es_out_StartAllEs( es_out_t *p_out, vlc_es_id_t **context )
{
return es_out_PrivControl( p_out, ES_OUT_PRIV_START_ALL_ES, context );
}
static inline bool es_out_GetBuffering( es_out_t *p_out )
{
bool b;
int i_ret = es_out_PrivControl( p_out, ES_OUT_PRIV_GET_BUFFERING, &b );
assert( !i_ret );
return b;
}
static inline bool es_out_GetEmpty( es_out_t *p_out )
{
bool b;
int i_ret = es_out_Control( p_out, ES_OUT_GET_EMPTY, &b );
assert( !i_ret );
return b;
}
static inline void es_out_SetEsDelay( es_out_t *p_out, vlc_es_id_t *es, vlc_tick_t i_delay )
{
int i_ret = es_out_PrivControl( p_out, ES_OUT_PRIV_SET_ES_DELAY, es, i_delay );
assert( !i_ret );
}
static inline void es_out_SetDelay( es_out_t *p_out, int i_cat, vlc_tick_t i_delay )
{
int i_ret = es_out_PrivControl( p_out, ES_OUT_PRIV_SET_DELAY, i_cat, i_delay );
assert( !i_ret );
}
static inline int es_out_SetRecordState( es_out_t *p_out, bool b_record )
{
return es_out_PrivControl( p_out, ES_OUT_PRIV_SET_RECORD_STATE, b_record );
}
static inline int es_out_SetPauseState( es_out_t *p_out, bool b_source_paused, bool b_paused, vlc_tick_t i_date )
{
return es_out_PrivControl( p_out, ES_OUT_PRIV_SET_PAUSE_STATE, b_source_paused, b_paused, i_date );
}
static inline int es_out_SetRate( es_out_t *p_out, float source_rate, float rate )
{
return es_out_PrivControl( p_out, ES_OUT_PRIV_SET_RATE, source_rate, rate );
}
static inline int es_out_SetFrameNext( es_out_t *p_out )
{
return es_out_PrivControl( p_out, ES_OUT_PRIV_SET_FRAME_NEXT );
}
static inline void es_out_SetTimes( es_out_t *p_out, double f_position,
vlc_tick_t i_time, vlc_tick_t i_normal_time,
vlc_tick_t i_length )
{
int i_ret = es_out_PrivControl( p_out, ES_OUT_PRIV_SET_TIMES, f_position, i_time,
i_normal_time, i_length );
assert( !i_ret );
}
static inline void es_out_SetJitter( es_out_t *p_out,
vlc_tick_t i_pts_delay, vlc_tick_t i_pts_jitter, int i_cr_average )
{
int i_ret = es_out_PrivControl( p_out, ES_OUT_PRIV_SET_JITTER,
i_pts_delay, i_pts_jitter, i_cr_average );
assert( !i_ret );
}
static inline int es_out_GetGroupForced( es_out_t *p_out )
{
int i_group;
int i_ret = es_out_PrivControl( p_out, ES_OUT_PRIV_GET_GROUP_FORCED, &i_group );
assert( !i_ret );
return i_group;
}
static inline void es_out_Eos( es_out_t *p_out )
{
int i_ret = es_out_PrivControl( p_out, ES_OUT_PRIV_SET_EOS );
assert( !i_ret );
}
static inline int es_out_SetVbiPage( es_out_t *p_out, vlc_es_id_t *id,
unsigned page )
{
return es_out_PrivControl( p_out, ES_OUT_PRIV_SET_VBI_PAGE, id, page );
}
static inline int es_out_SetVbiTransparency( es_out_t *p_out, vlc_es_id_t *id,
bool enabled )
{
return es_out_PrivControl( p_out, ES_OUT_PRIV_SET_VBI_TRANSPARENCY, id,
enabled );
}
es_out_t *input_EsOutNew( input_thread_t *, input_source_t *main_source, float rate );
es_out_t *input_EsOutTimeshiftNew( input_thread_t *, es_out_t *, float i_rate );
es_out_t *input_EsOutSourceNew(es_out_t *master_out, input_source_t *in);
es_out_id_t *vlc_es_id_get_out(vlc_es_id_t *id);
const input_source_t *vlc_es_id_GetSource(vlc_es_id_t *id);
