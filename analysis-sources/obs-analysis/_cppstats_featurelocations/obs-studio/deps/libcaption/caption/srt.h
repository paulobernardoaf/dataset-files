






















#if !defined(LIBCAPTION_SRT_H)
#define LIBCAPTION_SRT_H
#if defined(__cplusplus)
extern "C" {
#endif

#include "caption.h"
#include "eia608.h"
#include "vtt.h"


typedef vtt_t srt_t;
typedef vtt_block_t srt_cue_t;




srt_t* srt_new();



srt_t* srt_free_head(srt_t* head);




srt_t* srt_parse(const utf8_char_t* data, size_t size);



void srt_free(srt_t* srt);




static inline vtt_block_t* srt_next(vtt_block_t* srt) { return srt->next; }




static inline utf8_char_t* srt_cue_data(srt_cue_t* cue) { return vtt_block_data(cue); }




static inline srt_cue_t* srt_cue_from_caption_frame(caption_frame_t* frame, srt_t* srt) { return vtt_cue_from_caption_frame(frame, srt); };




static inline void srt_cue_free_head(srt_t* srt) { vtt_cue_free_head(srt); };




static inline srt_cue_t* srt_cue_new(srt_t* srt, const utf8_char_t* data, size_t size) { return vtt_block_new(srt, data, size, VTT_CUE); };




static inline int srt_cue_to_caption_frame(srt_cue_t* cue, caption_frame_t* frame) { return vtt_cue_to_caption_frame(cue, frame); };

void srt_dump(srt_t* srt);



void vtt_dump(srt_t* srt);

#if defined(__cplusplus)
}
#endif
#endif
