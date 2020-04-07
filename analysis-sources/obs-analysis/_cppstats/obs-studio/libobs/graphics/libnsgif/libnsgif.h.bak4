












#if !defined(_LIBNSGIF_H_)
#define _LIBNSGIF_H_

#include <stdbool.h>
#include <inttypes.h>

#if defined(__cplusplus)
extern "C"
{
#endif


typedef enum {
GIF_WORKING = 1,
GIF_OK = 0,
GIF_INSUFFICIENT_FRAME_DATA = -1,
GIF_FRAME_DATA_ERROR = -2,
GIF_INSUFFICIENT_DATA = -3,
GIF_DATA_ERROR = -4,
GIF_INSUFFICIENT_MEMORY = -5,
GIF_FRAME_NO_DISPLAY = -6,
GIF_END_OF_FRAME = -7
} gif_result;



#define GIF_MAX_LZW 12



typedef struct gif_frame {
bool display; 
unsigned int frame_delay; 


unsigned int frame_pointer; 
bool virgin; 
bool opaque; 
bool redraw_required; 
unsigned char disposal_method; 
bool transparency; 
unsigned char transparency_index; 
unsigned int redraw_x; 
unsigned int redraw_y; 
unsigned int redraw_width; 
unsigned int redraw_height; 
} gif_frame;



typedef void* (*gif_bitmap_cb_create)(int width, int height);
typedef void (*gif_bitmap_cb_destroy)(void *bitmap);
typedef unsigned char* (*gif_bitmap_cb_get_buffer)(void *bitmap);
typedef void (*gif_bitmap_cb_set_opaque)(void *bitmap, bool opaque);
typedef bool (*gif_bitmap_cb_test_opaque)(void *bitmap);
typedef void (*gif_bitmap_cb_modified)(void *bitmap);



typedef struct gif_bitmap_callback_vt {
gif_bitmap_cb_create bitmap_create; 
gif_bitmap_cb_destroy bitmap_destroy; 
gif_bitmap_cb_get_buffer bitmap_get_buffer; 


gif_bitmap_cb_set_opaque bitmap_set_opaque; 
gif_bitmap_cb_test_opaque bitmap_test_opaque; 
gif_bitmap_cb_modified bitmap_modified; 
} gif_bitmap_callback_vt;



typedef struct gif_animation {
gif_bitmap_callback_vt bitmap_callbacks; 
unsigned char *gif_data; 
unsigned int width; 
unsigned int height; 
unsigned int frame_count; 
unsigned int frame_count_partial; 
gif_frame *frames; 
int decoded_frame; 
void *frame_image; 
int loop_count; 
gif_result current_error; 


unsigned int buffer_position; 
unsigned int buffer_size; 
unsigned int frame_holders; 
unsigned int background_index; 
unsigned int aspect_ratio; 
unsigned int colour_table_size; 
bool global_colours; 
unsigned int *global_colour_table; 
unsigned int *local_colour_table; 





unsigned char buf[4];
unsigned char *direct;

int table[2][(1 << GIF_MAX_LZW)];
unsigned char stack[(1 << GIF_MAX_LZW) * 2];
unsigned char *stack_pointer;
int code_size, set_code_size;
int max_code, max_code_size;
int clear_code, end_code;
int curbit, lastbit, last_byte;
int firstcode, oldcode;
bool zero_data_block;
bool get_done;



bool clear_image;
} gif_animation;

void gif_create(gif_animation *gif, gif_bitmap_callback_vt *bitmap_callbacks);
gif_result gif_initialise(gif_animation *gif, size_t size, unsigned char *data);
gif_result gif_decode_frame(gif_animation *gif, unsigned int frame);
void gif_finalise(gif_animation *gif);

#if defined(__cplusplus)
};
#endif

#endif
