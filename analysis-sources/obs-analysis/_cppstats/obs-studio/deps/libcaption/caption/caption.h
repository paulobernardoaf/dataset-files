#if defined(__cplusplus)
extern "C" {
#endif
#include "eia608.h"
#include "utf8.h"
#include "xds.h"
#if defined(_MSC_VER)
#if defined(_WIN64)
typedef signed long ssize_t;
#else
typedef signed int ssize_t;
#endif
#endif
typedef enum {
LIBCAPTION_ERROR = 0,
LIBCAPTION_OK = 1,
LIBCAPTION_READY = 2
} libcaption_stauts_t;
static inline libcaption_stauts_t libcaption_status_update(libcaption_stauts_t old_stat, libcaption_stauts_t new_stat)
{
return (LIBCAPTION_ERROR == old_stat || LIBCAPTION_ERROR == new_stat) ? LIBCAPTION_ERROR : (LIBCAPTION_READY == old_stat) ? LIBCAPTION_READY : new_stat;
}
#define SCREEN_ROWS 15
#define SCREEN_COLS 32
typedef struct {
unsigned int uln : 1; 
unsigned int sty : 3; 
utf8_char_t data[5]; 
} caption_frame_cell_t;
typedef struct {
caption_frame_cell_t cell[SCREEN_ROWS][SCREEN_COLS];
} caption_frame_buffer_t;
typedef struct {
unsigned int uln : 1; 
unsigned int sty : 3; 
unsigned int rup : 2; 
int8_t row, col;
uint16_t cc_data;
} caption_frame_state_t;
typedef struct {
double timestamp;
xds_t xds;
caption_frame_state_t state;
caption_frame_buffer_t front;
caption_frame_buffer_t back;
caption_frame_buffer_t* write;
libcaption_stauts_t status;
} caption_frame_t;
void caption_frame_init(caption_frame_t* frame);
static inline int caption_frame_popon(caption_frame_t* frame) { return (frame->write == &frame->back) ? 1 : 0; }
static inline int caption_frame_painton(caption_frame_t* frame) { return (frame->write == &frame->front) ? 1 : 0; }
const static int _caption_frame_rollup[] = { 0, 2, 3, 4 };
static inline int caption_frame_rollup(caption_frame_t* frame) { return _caption_frame_rollup[frame->state.rup]; }
static inline double caption_frame_timestamp(caption_frame_t* frame) { return frame->timestamp; }
int caption_frame_write_char(caption_frame_t* frame, int row, int col, eia608_style_t style, int underline, const utf8_char_t* c);
const utf8_char_t* caption_frame_read_char(caption_frame_t* frame, int row, int col, eia608_style_t* style, int* underline);
libcaption_stauts_t caption_frame_decode(caption_frame_t* frame, uint16_t cc_data, double timestamp);
int caption_frame_from_text(caption_frame_t* frame, const utf8_char_t* data);
#define CAPTION_FRAME_TEXT_BYTES (4 * ((SCREEN_COLS + 2) * SCREEN_ROWS) + 1)
size_t caption_frame_to_text(caption_frame_t* frame, utf8_char_t* data);
#define CAPTION_FRAME_DUMP_BUF_SIZE 8192
size_t caption_frame_dump_buffer(caption_frame_t* frame, utf8_char_t* buf);
void caption_frame_dump(caption_frame_t* frame);
#if defined(__cplusplus)
}
#endif
