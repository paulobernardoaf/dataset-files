



















#if !defined(AVFORMAT_MMS_H)
#define AVFORMAT_MMS_H

#include "url.h"

typedef struct MMSStream {
int id;
}MMSStream;

typedef struct MMSContext {
URLContext *mms_hd; 
MMSStream *streams;



uint8_t *write_out_ptr; 
uint8_t out_buffer[512]; 




uint8_t in_buffer[65536]; 
uint8_t *read_in_ptr; 
int remaining_in_len; 




uint8_t *asf_header; 
int asf_header_size; 
int header_parsed; 
int asf_packet_len;
int asf_header_read_size;


int stream_num; 
unsigned int nb_streams_allocated; 
} MMSContext;

int ff_mms_asf_header_parser(MMSContext * mms);
int ff_mms_read_data(MMSContext *mms, uint8_t *buf, const int size);
int ff_mms_read_header(MMSContext * mms, uint8_t * buf, const int size);

#endif 
