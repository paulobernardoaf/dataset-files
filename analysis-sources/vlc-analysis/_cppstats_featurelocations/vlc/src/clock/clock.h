


















#if !defined(VLC_CLOCK_H)
#define VLC_CLOCK_H

enum vlc_clock_master_source
{
VLC_CLOCK_MASTER_AUDIO = 0,
VLC_CLOCK_MASTER_MONOTONIC,
VLC_CLOCK_MASTER_DEFAULT = VLC_CLOCK_MASTER_AUDIO,
};

typedef struct vlc_clock_main_t vlc_clock_main_t;
typedef struct vlc_clock_t vlc_clock_t;




struct vlc_clock_cbs
{














void (*on_update)(vlc_tick_t system_ts, vlc_tick_t ts, double rate,
unsigned frame_rate, unsigned frame_rate_base,
void *data);
};




vlc_clock_main_t *vlc_clock_main_New(void);




void vlc_clock_main_Delete(vlc_clock_main_t *main_clock);




void vlc_clock_main_Abort(vlc_clock_main_t *main_clock);




void vlc_clock_main_Reset(vlc_clock_main_t *main_clock);

void vlc_clock_main_SetFirstPcr(vlc_clock_main_t *main_clock,
vlc_tick_t system_now, vlc_tick_t ts);
void vlc_clock_main_SetInputDejitter(vlc_clock_main_t *main_clock,
vlc_tick_t delay);






void vlc_clock_main_SetDejitter(vlc_clock_main_t *main_clock, vlc_tick_t dejitter);





void vlc_clock_main_ChangePause(vlc_clock_main_t *clock, vlc_tick_t system_now,
bool paused);





void vlc_clock_main_SetMaster(vlc_clock_main_t *main_clock, vlc_clock_t *clock);






vlc_clock_t *vlc_clock_main_CreateMaster(vlc_clock_main_t *main_clock,
const struct vlc_clock_cbs *cbs,
void *cbs_data);






vlc_clock_t *vlc_clock_main_CreateSlave(vlc_clock_main_t *main_clock,
enum es_format_category_e cat,
const struct vlc_clock_cbs *cbs,
void *cbs_data);






vlc_clock_t *vlc_clock_CreateSlave(const vlc_clock_t *clock,
enum es_format_category_e cat);




void vlc_clock_Delete(vlc_clock_t *clock);








vlc_tick_t vlc_clock_Update(vlc_clock_t *clock, vlc_tick_t system_now,
vlc_tick_t ts, double rate);







vlc_tick_t vlc_clock_UpdateVideo(vlc_clock_t *clock, vlc_tick_t system_now,
vlc_tick_t ts, double rate,
unsigned frame_rate, unsigned frame_rate_base);




void vlc_clock_Reset(vlc_clock_t *clock);







vlc_tick_t vlc_clock_SetDelay(vlc_clock_t *clock, vlc_tick_t ts_delay);




int vlc_clock_Wait(vlc_clock_t *clock, vlc_tick_t system_now, vlc_tick_t ts,
double rate, vlc_tick_t max_duration);





vlc_tick_t vlc_clock_ConvertToSystem(vlc_clock_t *clock, vlc_tick_t system_now,
vlc_tick_t ts, double rate);




void vlc_clock_ConvertArrayToSystem(vlc_clock_t *clock, vlc_tick_t system_now,
vlc_tick_t *ts_array, size_t ts_count,
double rate);

#endif 
