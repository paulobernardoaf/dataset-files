#include <stddef.h>
#include <stdint.h>
int av_ac3_parse_header(const uint8_t *buf, size_t size,
uint8_t *bitstream_id, uint16_t *frame_size);
