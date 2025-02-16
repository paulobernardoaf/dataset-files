#include <stdint.h>
typedef struct {
int version;
int channels; 
int preskip;
uint32_t input_sample_rate;
int gain; 
uint8_t channel_mapping;
int nb_streams;
int nb_coupled;
unsigned char stream_map[255];
} OpusHeader;
int opus_header_parse(const unsigned char *header, int len, OpusHeader *h);
void opus_prepare_header(unsigned channels, unsigned rate, OpusHeader *header);
int opus_write_header(uint8_t **p_extra, int *i_extra, OpusHeader *header, const char *vendor);
