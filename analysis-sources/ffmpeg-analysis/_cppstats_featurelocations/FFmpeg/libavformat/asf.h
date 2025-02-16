



















#if !defined(AVFORMAT_ASF_H)
#define AVFORMAT_ASF_H

#include <stdint.h>
#include "avformat.h"
#include "metadata.h"
#include "riff.h"

typedef enum ASFDataType {
ASF_UNICODE = 0,
ASF_BYTE_ARRAY = 1,
ASF_BOOL = 2,
ASF_DWORD = 3,
ASF_QWORD = 4,
ASF_WORD = 5,
ASF_GUID = 6,
}ASFDataType;

typedef struct ASFMainHeader {
ff_asf_guid guid; 
uint64_t file_size; 

uint64_t create_time; 

uint64_t play_time; 

uint64_t send_time; 

uint32_t preroll; 

uint32_t ignore; 
uint32_t flags; 


uint32_t min_pktsize; 

uint32_t max_pktsize; 

uint32_t max_bitrate; 


} ASFMainHeader;


typedef struct ASFIndex {
uint32_t packet_number;
uint16_t packet_count;
uint64_t send_time;
uint64_t offset;
} ASFIndex;

extern const ff_asf_guid ff_asf_header;
extern const ff_asf_guid ff_asf_file_header;
extern const ff_asf_guid ff_asf_stream_header;
extern const ff_asf_guid ff_asf_ext_stream_header;
extern const ff_asf_guid ff_asf_audio_stream;
extern const ff_asf_guid ff_asf_audio_conceal_none;
extern const ff_asf_guid ff_asf_audio_conceal_spread;
extern const ff_asf_guid ff_asf_video_stream;
extern const ff_asf_guid ff_asf_jfif_media;
extern const ff_asf_guid ff_asf_video_conceal_none;
extern const ff_asf_guid ff_asf_command_stream;
extern const ff_asf_guid ff_asf_comment_header;
extern const ff_asf_guid ff_asf_codec_comment_header;
extern const ff_asf_guid ff_asf_codec_comment1_header;
extern const ff_asf_guid ff_asf_data_header;
extern const ff_asf_guid ff_asf_head1_guid;
extern const ff_asf_guid ff_asf_head2_guid;
extern const ff_asf_guid ff_asf_extended_content_header;
extern const ff_asf_guid ff_asf_simple_index_header;
extern const ff_asf_guid ff_asf_ext_stream_embed_stream_header;
extern const ff_asf_guid ff_asf_ext_stream_audio_stream;
extern const ff_asf_guid ff_asf_metadata_header;
extern const ff_asf_guid ff_asf_metadata_library_header;
extern const ff_asf_guid ff_asf_marker_header;
extern const ff_asf_guid ff_asf_reserved_4;
extern const ff_asf_guid ff_asf_my_guid;
extern const ff_asf_guid ff_asf_language_guid;
extern const ff_asf_guid ff_asf_content_encryption;
extern const ff_asf_guid ff_asf_ext_content_encryption;
extern const ff_asf_guid ff_asf_digital_signature;
extern const ff_asf_guid ff_asf_extended_stream_properties_object;
extern const ff_asf_guid ff_asf_group_mutual_exclusion_object;
extern const ff_asf_guid ff_asf_mutex_language;

extern const AVMetadataConv ff_asf_metadata_conv[];

#define ASF_PACKET_FLAG_ERROR_CORRECTION_PRESENT 0x80 


















#define ASF_PPI_FLAG_MULTIPLE_PAYLOADS_PRESENT 1

#define ASF_PPI_FLAG_SEQUENCE_FIELD_IS_BYTE 0x02 
#define ASF_PPI_FLAG_SEQUENCE_FIELD_IS_WORD 0x04 
#define ASF_PPI_FLAG_SEQUENCE_FIELD_IS_DWORD 0x06 
#define ASF_PPI_MASK_SEQUENCE_FIELD_SIZE 0x06 

#define ASF_PPI_FLAG_PADDING_LENGTH_FIELD_IS_BYTE 0x08 
#define ASF_PPI_FLAG_PADDING_LENGTH_FIELD_IS_WORD 0x10 
#define ASF_PPI_FLAG_PADDING_LENGTH_FIELD_IS_DWORD 0x18 
#define ASF_PPI_MASK_PADDING_LENGTH_FIELD_SIZE 0x18 

#define ASF_PPI_FLAG_PACKET_LENGTH_FIELD_IS_BYTE 0x20 
#define ASF_PPI_FLAG_PACKET_LENGTH_FIELD_IS_WORD 0x40 
#define ASF_PPI_FLAG_PACKET_LENGTH_FIELD_IS_DWORD 0x60 
#define ASF_PPI_MASK_PACKET_LENGTH_FIELD_SIZE 0x60 


#define ASF_PL_FLAG_REPLICATED_DATA_LENGTH_FIELD_IS_BYTE 0x01 
#define ASF_PL_FLAG_REPLICATED_DATA_LENGTH_FIELD_IS_WORD 0x02 
#define ASF_PL_FLAG_REPLICATED_DATA_LENGTH_FIELD_IS_DWORD 0x03 
#define ASF_PL_MASK_REPLICATED_DATA_LENGTH_FIELD_SIZE 0x03 

#define ASF_PL_FLAG_OFFSET_INTO_MEDIA_OBJECT_LENGTH_FIELD_IS_BYTE 0x04 
#define ASF_PL_FLAG_OFFSET_INTO_MEDIA_OBJECT_LENGTH_FIELD_IS_WORD 0x08 
#define ASF_PL_FLAG_OFFSET_INTO_MEDIA_OBJECT_LENGTH_FIELD_IS_DWORD 0x0c 
#define ASF_PL_MASK_OFFSET_INTO_MEDIA_OBJECT_LENGTH_FIELD_SIZE 0x0c 

#define ASF_PL_FLAG_MEDIA_OBJECT_NUMBER_LENGTH_FIELD_IS_BYTE 0x10 
#define ASF_PL_FLAG_MEDIA_OBJECT_NUMBER_LENGTH_FIELD_IS_WORD 0x20 
#define ASF_PL_FLAG_MEDIA_OBJECT_NUMBER_LENGTH_FIELD_IS_DWORD 0x30 
#define ASF_PL_MASK_MEDIA_OBJECT_NUMBER_LENGTH_FIELD_SIZE 0x30 

#define ASF_PL_FLAG_STREAM_NUMBER_LENGTH_FIELD_IS_BYTE 0x40 
#define ASF_PL_MASK_STREAM_NUMBER_LENGTH_FIELD_SIZE 0xc0 

#define ASF_PL_FLAG_PAYLOAD_LENGTH_FIELD_IS_BYTE 0x40 
#define ASF_PL_FLAG_PAYLOAD_LENGTH_FIELD_IS_WORD 0x80 
#define ASF_PL_MASK_PAYLOAD_LENGTH_FIELD_SIZE 0xc0 

#define ASF_PL_FLAG_KEY_FRAME 0x80 

#endif 
