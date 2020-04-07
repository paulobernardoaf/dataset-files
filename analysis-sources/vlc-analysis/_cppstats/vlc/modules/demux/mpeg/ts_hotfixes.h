#define MIN_PAT_INTERVAL VLC_TICK_FROM_SEC(1) 
void ProbePES( demux_t *p_demux, ts_pid_t *pid, const uint8_t *p_pesstart, size_t i_data, bool b_adaptfield );
void MissingPATPMTFixup( demux_t *p_demux );
