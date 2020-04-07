

















#if !defined(VLC_TS_SL_H)
#define VLC_TS_SL_H

typedef struct es_mpeg4_descriptor_t es_mpeg4_descriptor_t;
typedef struct decoder_config_descriptor_t decoder_config_descriptor_t;

const es_mpeg4_descriptor_t * GetMPEG4DescByEsId( const ts_pmt_t *pmt, uint16_t i_es_id );
void SLPackets_Section_Handler( demux_t *p_demux,
const uint8_t *, size_t,
const uint8_t *, size_t,
void * );
bool SetupISO14496LogicalStream( demux_t *, const decoder_config_descriptor_t *,
es_format_t * );

ts_stream_processor_t *SL_stream_processor_New( ts_stream_t * );

#endif
