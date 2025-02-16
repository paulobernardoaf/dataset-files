struct filter_t;
struct picture_t;
struct plane_t;
typedef enum { CC_ALTLINE, CC_UPCONVERT, CC_SOURCE_TOP, CC_SOURCE_BOTTOM,
CC_MERGE } compose_chroma_t;
void ComposeFrame( filter_t *p_filter,
picture_t *p_outpic,
picture_t *p_inpic_top, picture_t *p_inpic_bottom,
compose_chroma_t i_output_chroma, bool swapped_uv_conversion );
int EstimateNumBlocksWithMotion( const picture_t* p_prev,
const picture_t* p_curr,
int *pi_top, int *pi_bot);
int CalculateInterlaceScore( const picture_t* p_pic_top,
const picture_t* p_pic_bot );
