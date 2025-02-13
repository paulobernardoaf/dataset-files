#if defined(__cplusplus)
extern "C" {
#endif
#include "caption.h"
#include "eia608.h"
enum VTT_BLOCK_TYPE {
VTT_REGION = 0,
VTT_STYLE = 1,
VTT_NOTE = 2,
VTT_CUE = 3
};
typedef struct _vtt_block_t {
struct _vtt_block_t* next;
enum VTT_BLOCK_TYPE type;
double timestamp;
double duration; 
char* cue_settings;
char* cue_id;
size_t text_size;
char* block_text;
} vtt_block_t;
typedef struct _vtt_t {
vtt_block_t* region_head;
vtt_block_t* region_tail;
vtt_block_t* style_head;
vtt_block_t* style_tail;
vtt_block_t* cue_head;
vtt_block_t* cue_tail;
} vtt_t;
vtt_t* vtt_new();
void vtt_free(vtt_t* vtt);
vtt_block_t* vtt_block_new(vtt_t* vtt, const utf8_char_t* data, size_t size, enum VTT_BLOCK_TYPE type);
void vtt_cue_free_head(vtt_t* vtt);
void vtt_style_free_head(vtt_t* vtt);
void vtt_region_free_head(vtt_t* vtt);
vtt_t* vtt_parse(const utf8_char_t* data, size_t size);
vtt_t* _vtt_parse(const utf8_char_t* data, size_t size, int srt_mode);
static inline vtt_block_t* vtt_cue_next(vtt_block_t* block) { return block->next; }
static inline utf8_char_t* vtt_block_data(vtt_block_t* block) { return (utf8_char_t*)(block) + sizeof(vtt_block_t); }
static inline void vtt_crack_time(double tt, int* hh, int* mm, int* ss, int* ms)
{
(*ms) = (int)((int64_t)(tt * 1000) % 1000);
(*ss) = (int)((int64_t)(tt) % 60);
(*mm) = (int)((int64_t)(tt / (60)) % 60);
(*hh) = (int)((int64_t)(tt / (60 * 60)));
}
int vtt_cue_to_caption_frame(vtt_block_t* cue, caption_frame_t* frame);
vtt_block_t* vtt_cue_from_caption_frame(caption_frame_t* frame, vtt_t* vtt);
void vtt_dump(vtt_t* vtt);
#if defined(__cplusplus)
}
#endif
