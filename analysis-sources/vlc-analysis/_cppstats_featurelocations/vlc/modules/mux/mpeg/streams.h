


















#if !defined(VLC_MPEG_STREAMS_H_)
#define VLC_MPEG_STREAMS_H_

typedef struct
{
uint16_t i_pid;

uint8_t i_stream_type;
uint8_t i_continuity_counter;
bool b_discontinuity;

} tsmux_stream_t;

typedef struct
{
int i_stream_id; 


int i_es_id;


size_t i_langs;
uint8_t *lang;
} pesmux_stream_t;

#endif
