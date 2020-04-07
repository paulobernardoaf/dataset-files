#include "ts_pid_fwd.h"
#define MIN_ES_PID 4 
#define MAX_ES_PID 8190
#include "ts_streams.h"
typedef struct demux_sys_t demux_sys_t;
typedef enum
{
TYPE_FREE = 0,
TYPE_CAT,
TYPE_PAT,
TYPE_PMT,
TYPE_STREAM,
TYPE_SI,
TYPE_PSIP,
} ts_pid_type_t;
enum
{
FLAGS_NONE = 0,
FLAG_SEEN = 1,
FLAG_SCRAMBLED = 2,
FLAG_FILTERED = 4
};
#define SEEN(x) ((x)->i_flags & FLAG_SEEN)
#define SCRAMBLED(x) ((x).i_flags & FLAG_SCRAMBLED)
#define PREVPKTKEEPBYTES 16
struct ts_pid_t
{
uint16_t i_pid;
uint8_t i_flags;
uint8_t i_cc; 
uint8_t i_dup; 
uint8_t type;
uint8_t prevpktbytes[PREVPKTKEEPBYTES];
uint16_t i_refcount;
union
{
ts_pat_t *p_pat;
ts_pmt_t *p_pmt;
ts_stream_t *p_stream;
ts_si_t *p_si;
ts_psip_t *p_psip;
} u;
struct
{
vlc_fourcc_t i_fourcc;
vlc_fourcc_t i_original_fourcc;
int i_cat;
int i_pcr_count;
uint8_t i_stream_id;
} probed;
};
struct ts_pid_list_t
{
ts_pid_t pat;
ts_pid_t dummy;
ts_pid_t base_si;
ts_pid_t **pp_all;
int i_all;
int i_all_alloc;
uint16_t i_last_pid;
ts_pid_t *p_last;
};
void ts_pid_list_Init( ts_pid_list_t * );
void ts_pid_list_Release( demux_t *, ts_pid_list_t * );
ts_pid_t * ts_pid_Get( ts_pid_list_t *, uint16_t i_pid );
typedef struct
{
int i_pos;
} ts_pid_next_context_t;
#define ts_pid_NextContextInitValue { 0 }
ts_pid_t * ts_pid_Next( ts_pid_list_t *, ts_pid_next_context_t * );
#define GetPID(p_sys, i_pid) ts_pid_Get((&(p_sys)->pids), i_pid)
int UpdateHWFilter( demux_sys_t *, ts_pid_t * );
int SetPIDFilter( demux_sys_t *, ts_pid_t *, bool b_selected );
bool PIDSetup( demux_t *p_demux, ts_pid_type_t i_type, ts_pid_t *pid, ts_pid_t *p_parent );
void PIDRelease( demux_t *p_demux, ts_pid_t *pid );
