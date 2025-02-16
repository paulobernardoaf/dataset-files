

















#if !defined(VLC_TS_STREAMS_PRIVATE_H)
#define VLC_TS_STREAMS_PRIVATE_H

typedef struct dvbpsi_s dvbpsi_t;
typedef struct ts_sections_processor_t ts_sections_processor_t;

#include "mpeg4_iod.h"
#include "timestamps.h"

#include <vlc_common.h>
#include <vlc_es.h>

struct ts_pat_t
{
int i_version;
int i_ts_id;
bool b_generated;
dvbpsi_t *handle;
DECL_ARRAY(ts_pid_t *) programs;

};

struct ts_pmt_t
{
dvbpsi_t *handle;
int i_version;
int i_number;
int i_pid_pcr;
bool b_selected;

od_descriptor_t *iod;
od_descriptors_t od;

DECL_ARRAY(ts_pid_t *) e_streams;


ts_pid_t *p_atsc_si_basepid;

ts_pid_t *p_si_sdt_pid;

struct
{
stime_t i_current;
stime_t i_first; 

stime_t i_first_dts;
stime_t i_pcroffset;
bool b_disable; 
bool b_fix_done;
} pcr;

struct
{
time_t i_event_start;
time_t i_event_length;
} eit;

stime_t i_last_dts;
uint64_t i_last_dts_byte;


struct
{
int i_logo_id;
int i_download_id;
} arib;
};

struct ts_es_t
{
ts_pmt_t *p_program;
es_format_t fmt;
es_out_id_t *id;
uint16_t i_sl_es_id;
int i_next_block_flags;
ts_es_t *p_extraes; 
ts_es_t *p_next; 

uint8_t b_interlaced;

struct
{
uint8_t i_service_id;
uint32_t i_format;
} metadata;
};

typedef enum
{
TS_TRANSPORT_PES,
TS_TRANSPORT_SECTIONS,
TS_TRANSPORT_IGNORE
} ts_transport_type_t;

struct ts_stream_t
{
ts_es_t *p_es;

uint8_t i_stream_type;

ts_transport_type_t transport;

struct
{
size_t i_data_size;
size_t i_gathered;
block_t *p_data;
block_t **pp_last;
uint8_t saved[5];
size_t i_saved;
} gather;

bool b_always_receive;
bool b_broken_PUSI_conformance;
ts_sections_processor_t *p_sections_proc;
ts_stream_processor_t *p_proc;

struct
{
block_t *p_head;
block_t **pp_last;
} prepcr;

stime_t i_last_dts;
};

typedef struct ts_si_context_t ts_si_context_t;

struct ts_si_t
{
dvbpsi_t *handle;
int i_version;

ts_pid_t *eitpid;
ts_pid_t *tdtpid;
ts_pid_t *cdtpid;
};

typedef struct ts_psip_context_t ts_psip_context_t;

struct ts_psip_t
{
dvbpsi_t *handle;
int i_version;
ts_es_t *p_eas_es;
ts_psip_context_t *p_ctx;



DECL_ARRAY(ts_pid_t *) eit;

};

#endif
