




















#if !defined(AVFORMAT_RDT_H)
#define AVFORMAT_RDT_H

#include <stdint.h>
#include "avformat.h"
#include "rtpdec.h"

typedef struct RDTDemuxContext RDTDemuxContext;











RDTDemuxContext *ff_rdt_parse_open(AVFormatContext *ic,
int first_stream_of_set_idx,
void *priv_data,
const RTPDynamicProtocolHandler *handler);
void ff_rdt_parse_close(RDTDemuxContext *s);













void ff_rdt_calc_response_and_checksum(char response[41], char chksum[9],
const char *challenge);









void ff_rdt_subscribe_rule(char *cmd, int size,
int stream_nr, int rule_nr);













int ff_rdt_parse_header(const uint8_t *buf, int len,
int *pset_id, int *pseq_no, int *pstream_id,
int *pis_keyframe, uint32_t *ptimestamp);





int ff_rdt_parse_packet(RDTDemuxContext *s, AVPacket *pkt,
uint8_t **buf, int len);









void ff_real_parse_sdp_a_line(AVFormatContext *s, int stream_index,
const char *buf);

#endif 
