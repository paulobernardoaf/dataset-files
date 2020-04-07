






















#define PES_PROGRAM_STREAM_MAP 0xbc
#define PES_PRIVATE_STREAM_1 0xbd
#define PES_PADDING 0xbe
#define PES_PRIVATE_STREAM_2 0xbf
#define PES_ECM 0xb0
#define PES_EMM 0xb1
#define PES_PROGRAM_STREAM_DIRECTORY 0xff
#define PES_DSMCC_STREAM 0xf2
#define PES_ITU_T_H222_1_TYPE_E_STREAM 0xf8
#define PES_EXTENDED_STREAM_ID 0xfd

#define PES_PAYLOAD_SIZE_MAX 65500

void EStoPES ( block_t **pp_pes,
const es_format_t *p_fmt, int i_stream_id,
int b_mpeg2, int b_data_alignment, int i_header_size,
int i_max_pes_size, vlc_tick_t ts_offset );
