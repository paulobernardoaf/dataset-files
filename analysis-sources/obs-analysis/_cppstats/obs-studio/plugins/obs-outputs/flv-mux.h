#pragma once
#include <obs.h>
#define MILLISECOND_DEN 1000
static int32_t get_ms_time(struct encoder_packet *packet, int64_t val)
{
return (int32_t)(val * MILLISECOND_DEN / packet->timebase_den);
}
extern void write_file_info(FILE *file, int64_t duration_ms, int64_t size);
extern bool flv_meta_data(obs_output_t *context, uint8_t **output, size_t *size,
bool write_header, size_t audio_idx);
extern void flv_packet_mux(struct encoder_packet *packet, int32_t dts_offset,
uint8_t **output, size_t *size, bool is_header);
