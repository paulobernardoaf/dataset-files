






















#if !defined(LIBCAPTION_MPEG_H)
#define LIBCAPTION_MPEG_H
#if defined(__cplusplus)
extern "C" {
#endif

#include "caption.h"
#include "cea708.h"
#include "scc.h"
#include <float.h>
#include <stddef.h>

#define STREAM_TYPE_H262 0x02
#define STREAM_TYPE_H264 0x1B
#define STREAM_TYPE_H265 0x24
#define H262_SEI_PACKET 0xB2
#define H264_SEI_PACKET 0x06
#define H265_SEI_PACKET 0x27 
#define MAX_NALU_SIZE (6 * 1024 * 1024)
#define MAX_REFRENCE_FRAMES 64
typedef struct {
size_t size;
uint8_t data[MAX_NALU_SIZE + 1];
double dts, cts;
libcaption_stauts_t status;


size_t front;
size_t latent;
cea708_t cea708[MAX_REFRENCE_FRAMES];
} mpeg_bitstream_t;

void mpeg_bitstream_init(mpeg_bitstream_t* packet);





size_t mpeg_bitstream_parse(mpeg_bitstream_t* packet, caption_frame_t* frame, const uint8_t* data, size_t size, unsigned stream_type, double dts, double cts);



static inline libcaption_stauts_t mpeg_bitstream_status(mpeg_bitstream_t* packet) { return packet->status; }





size_t mpeg_bitstream_flush(mpeg_bitstream_t* packet, caption_frame_t* frame);

typedef enum {
sei_type_buffering_period = 0,
sei_type_pic_timing = 1,
sei_type_pan_scan_rect = 2,
sei_type_filler_payload = 3,
sei_type_user_data_registered_itu_t_t35 = 4,
sei_type_user_data_unregistered = 5,
sei_type_recovery_point = 6,
sei_type_dec_ref_pic_marking_repetition = 7,
sei_type_spare_pic = 8,
sei_type_scene_info = 9,
sei_type_sub_seq_info = 10,
sei_type_sub_seq_layer_characteristics = 11,
sei_type_sub_seq_characteristics = 12,
sei_type_full_frame_freeze = 13,
sei_type_full_frame_freeze_release = 14,
sei_type_full_frame_snapshot = 15,
sei_type_progressive_refinement_segment_start = 16,
sei_type_progressive_refinement_segment_end = 17,
sei_type_motion_constrained_slice_group_set = 18,
sei_type_film_grain_characteristics = 19,
sei_type_deblocking_filter_display_preference = 20,
sei_type_stereo_video_info = 21,
} sei_msgtype_t;

typedef struct _sei_message_t {
size_t size;
sei_msgtype_t type;
struct _sei_message_t* next;
} sei_message_t;

typedef struct {
double timestamp;
sei_message_t* head;
sei_message_t* tail;
} sei_t;




void sei_init(sei_t* sei, double timestamp);



void sei_free(sei_t* sei);



void sei_cat(sei_t* to, sei_t* from, int itu_t_t35);



void sei_message_append(sei_t* sei, sei_message_t* msg);



libcaption_stauts_t sei_parse(sei_t* sei, const uint8_t* data, size_t size, double timestamp);



static inline sei_message_t* sei_message_head(sei_t* sei) { return sei->head; }



static inline sei_message_t* sei_message_tail(sei_t* sei) { return sei->tail; }



sei_message_t* sei_message_next(sei_message_t* msg);



sei_msgtype_t sei_message_type(sei_message_t* msg);



size_t sei_message_size(sei_message_t* msg);



uint8_t* sei_message_data(sei_message_t* msg);



sei_message_t* sei_message_new(sei_msgtype_t type, uint8_t* data, size_t size);



static inline sei_message_t* sei_message_copy(sei_message_t* msg)
{
return sei_message_new(sei_message_type(msg), sei_message_data(msg), sei_message_size(msg));
}







void sei_message_free(sei_message_t* msg);




size_t sei_render_size(sei_t* sei);



size_t sei_render(sei_t* sei, uint8_t* data);



void sei_dump(sei_t* sei);



void sei_dump_messages(sei_message_t* head, double timestamp);




libcaption_stauts_t sei_from_scc(sei_t* sei, const scc_t* scc);



libcaption_stauts_t sei_from_caption_frame(sei_t* sei, caption_frame_t* frame);



libcaption_stauts_t sei_from_caption_clear(sei_t* sei);



libcaption_stauts_t sei_to_caption_frame(sei_t* sei, caption_frame_t* frame);

#if defined(__cplusplus)
}
#endif
#endif
